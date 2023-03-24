//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_ApplePlatform.h"
#include "Core/Rtt_Allocator.h"
#include "Core/Rtt_String.h"
#include "Rtt_AppleTimer.h"
#include "Rtt_PlatformSurface.h"
#include "Rtt_PreferenceCollection.h"
#include "Core/Rtt_FileSystem.h"

#if !defined( Rtt_NO_GUI )
	#include "Rtt_GPUStream.h"
	#include "Rtt_LuaContext.h"
	#include "Rtt_LuaLibSystem.h"
	#include "Rtt_LuaResource.h"
	#include "Rtt_Runtime.h"

	#include "Rtt_AppleBitmap.h"
	#include "Rtt_AppleFont.h"
	#include "Rtt_AppleAudioPlayer.h"
	#include "Rtt_AppleAudioRecorder.h"
	#include "Rtt_AppleEventSound.h"
	#include "Rtt_AppleReachability.h"
#endif

#include "Rtt_Lua.h"
#include "Rtt_LuaAux.h"

#import <Foundation/Foundation.h>
#import "CoronaLuaObjC.h"
#include <sys/xattr.h> // Extended attributes (for iCloud backups, etc)

// #include <AudioToolbox/AudioServices.h>

#include <string.h>

#if !defined( Rtt_NO_GUI )
	// This is a hack to hold on to symbols required for running plugins
	// Because some plugins are dynamically linked, linker wouldn't know which symbols app uses
	// and will strip this plugin interfaces as unused

	#include "CoronaLuaObjCHelper.h"
	#include "CoronaEvent.h"
	#include "Corona/CoronaGraphics.h"
	#include "Corona/CoronaMemory.h"

	volatile void* fakeVariableToPreventSymbolStripping[] __attribute__((used)) = {
		(void*)CoronaEventNameKey,
		(void*)CoronaEventProviderKey,
		(void*)CoronaEventPhaseKey,
		(void*)CoronaEventTypeKey,
		(void*)CoronaEventResponseKey,
		(void*)CoronaEventIsErrorKey,
		(void*)CoronaEventErrorCodeKey,
		(void*)CoronaEventDataKey,
		(void*)CoronaEventAdsRequestName,
		(void*)CoronaEventGameNetworkName,
		(void*)CoronaEventPopupName,
		
		(void*)CoronaLuaCreateDictionary,
		(void*)CoronaLuaPushValue,
		
		(void*)CoronaExternalPushTexture,
		(void*)CoronaExternalGetUserData,
		(void*)CoronaExternalFormatBPP,

		(void*)CoronaMemoryCreateInterface,
		(void*)CoronaMemoryBindLookupSlot,
		(void*)CoronaMemoryReleaseLookupSlot,
		(void*)CoronaMemoryPushLookupEncoding,
		(void*)CoronaMemoryAcquireInterface
	};

#endif

// ----------------------------------------------------------------------------

@interface CustomMutableURLRequest : NSMutableURLRequest
{
@private
	NSTimeInterval fTimeout;
}

@end

@implementation CustomMutableURLRequest

- (id)init
{
	self = [super init];
	if ( self )
	{
		fTimeout = [super timeoutInterval];
	}
	return self;
}

- (NSTimeInterval)timeout
{
	return fTimeout;
}

- (void)setTimeoutInterval:(NSTimeInterval)seconds
{
	fTimeout = seconds;
	[super setTimeoutInterval:seconds];
}

@end

// ----------------------------------------------------------------------------

@class CustomURLConnection;

typedef void (^ReceivedDataHandler)(NSData*);

@interface CustomURLConnection : NSURLConnection
{
	NSString *fUrl;
	Rtt::LuaResource *fResource;
	ReceivedDataHandler fBlock;
	NSMutableData *fData;
	NSString *fFilePath;
	NSURLResponse *fResponse;
	NSTimeInterval fTimeout; // TODO: Remove when iOS workaround for Radar 10412199 is removed
	id fDelegate; // TODO: Remove when iOS workaround for Radar 10412199 is removed
}
@property (nonatomic, readonly, getter=url) NSString *fUrl;
@property (nonatomic, readonly, getter=resource) Rtt::LuaResource *fResource;
@property (nonatomic, readonly, getter=block) ReceivedDataHandler fBlock;
@property (nonatomic, readwrite, assign, getter=data, setter=setData:) NSMutableData *fData;
@property (nonatomic, readonly, getter=filePath) NSString *fFilePath;
@property (nonatomic, readwrite, retain, getter=response, setter=setResponse:) NSURLResponse *fResponse;

-(id)initWithRequest:(NSURLRequest *)request delegate:(id)delegate filePath:(NSString*)path resource:(Rtt::LuaResource*)resource;

-(id)initWithRequest:(NSURLRequest *)request delegate:(id)delegate block:(ReceivedDataHandler)block;

@end

@implementation CustomURLConnection

@synthesize fUrl;
@synthesize fResource;
@synthesize fBlock;
@synthesize fData;
@synthesize fFilePath;
@synthesize fResponse;

// Workaround for Apple Radar: 10412199
// See also:
// http://stackoverflow.com/questions/2736967/nsmutableurlrequest-not-obeying-my-timeoutinterval
// https://devforums.apple.com/message/108292#108292
#ifdef Rtt_IPHONE_ENV
- (void)setTimeoutForRequest:(NSURLRequest*)request delegate:(id)delegate
{
	// On iOS, the timeout is not honored (i.e. it's always 240 sec) 
	// when the request has a body
	if ( [request HTTPBody] )
	{
		if ( [request isKindOfClass:[CustomMutableURLRequest class]] )
		{
			CustomMutableURLRequest *r = (CustomMutableURLRequest *)request;
			fTimeout = [r timeout];
			if ( fTimeout > 0 )
			{
				[self performSelector:@selector(didTimeout:) withObject:delegate afterDelay:fTimeout];
			}
		}
	}
}

- (void)didTimeout:(id)delegate
{
	[self cancel];

	NSError *error = [NSError errorWithDomain:NSURLErrorDomain code:-1001 userInfo:[NSDictionary dictionaryWithObject:@"The request timed out." forKey:NSLocalizedDescriptionKey]];
	[delegate connection:self didFailWithError:error];
}

- (void)cancelTimeout
{
	// Workaround for Apple Radar: 10412199
	[NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(didTimeout:) object:fDelegate];
}

#endif

-(id)initWithRequest:(NSURLRequest *)request delegate:(id)delegate filePath:(NSString*)path resource:(Rtt::LuaResource*)resource
{
	self = [super initWithRequest:request delegate:delegate];
	if ( self )
	{
		// This used to give precedence to "path" if it was set but that made the error messages obtuse, save the actual url instead
		fUrl = ([[request URL] absoluteString] ? [[[request URL] absoluteString] retain] : [path retain]);
		fResource = resource;
		fBlock = NULL;
		fData = [[NSMutableData alloc] initWithCapacity:0];
		fFilePath = [path retain];
		fResponse = nil;
// Workaround for Apple Radar: 10412199
#ifdef Rtt_IPHONE_ENV
		fDelegate = delegate;
		[self setTimeoutForRequest:request delegate:delegate];
#endif
	}
	return self;
}

-(id)initWithRequest:(NSURLRequest *)request delegate:(id)delegate block:(ReceivedDataHandler)block
{
	self = [super initWithRequest:request delegate:delegate];
	if ( self )
	{
		fResource = NULL;
		fBlock = [block copy];
		fData = [[NSMutableData alloc] initWithCapacity:0];
		fFilePath = nil;
// Workaround for Apple Radar: 10412199
#ifdef Rtt_IPHONE_ENV
		fDelegate = delegate;
		[self setTimeoutForRequest:request delegate:delegate];
#endif
	}
	return self;
}

-(void)dealloc
{
#ifndef Rtt_NO_GUI
	// Dealloc Corona objects on the main thread
	if ( [NSThread isMainThread] )
	{
		Rtt_DELETE( fResource );
	}
	else
	{
		Rtt::LuaResource *resource = fResource;

		// Prevent deadlock by queue'ing on main thread
		dispatch_async( dispatch_get_main_queue(),
			^(void)
			{
				Rtt_DELETE( resource );
			}
		);
	}
#endif // Rtt_NO_GUI

#ifdef Rtt_IPHONE_ENV
	[self cancelTimeout];
#endif

	[fResponse release];
	[fBlock release];
	[fData release];
	[fFilePath release];
	[fUrl release];

	[super dealloc];
}

@end

// ----------------------------------------------------------------------------

@interface Rtt_ConnectionDelegate : NSObject
{
}

-(void)connection:(NSURLConnection *)connection didFailWithError:(NSError*)error;
-(void)connectionDidFinishLoading:(NSURLConnection *)connection;

@end


@implementation Rtt_ConnectionDelegate

-(void)connection:(NSURLConnection *)connection didFailWithError:(NSError*)error
{
	Rtt_TRACE( ( "ERROR: Connection failed. %s %s\n",
          [[error localizedDescription] UTF8String],
          [[[error userInfo] objectForKey:NSURLErrorFailingURLStringErrorKey] UTF8String] ) );
	[connection release];
}

-(void)connectionDidFinishLoading:(NSURLConnection *)connection
{
	[connection release];
}

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
	#ifdef Rtt_DEBUG
		NSLog( @"%@", response );
	#endif
}

@end

// ----------------------------------------------------------------------------

@interface CustomConnectionDelegate : NSObject
{
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data;
- (void)connectionDidFinishLoading:(NSURLConnection *)connection;
- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response;
- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError*)error;

@end

@implementation CustomConnectionDelegate

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
	// This method is called incrementally as the server sends data; we must concatenate the data to assemble the response

	CustomURLConnection *customConnection = (CustomURLConnection *)connection;
		
	[customConnection.data appendData:data];
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection
{
	// This method is called when the response is complete and no further data is coming
	
	using namespace Rtt;
	
	CustomURLConnection *customConnection = (CustomURLConnection *)connection;
	LuaResource *resource = customConnection.resource;
	ReceivedDataHandler block = customConnection.block;
	NSMutableData *data = customConnection.data;
	NSString *filePath = customConnection.filePath;

	if ( resource )
	{
		const char *responseString = NULL;
		if ( filePath )
		{
			[data writeToFile:filePath atomically:YES];
			responseString = [filePath UTF8String];
		}
		else
		{
			NSString *responseNSString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];	
			responseString = [responseNSString UTF8String];
			[responseNSString autorelease];
		}

#ifdef Rtt_NO_GUI
#else
		// Trap status codes
		NSInteger statusCode = -1;
		NSURLResponse *response = customConnection.response;
		if ([response respondsToSelector:@selector(statusCode)])
		{
			statusCode = [((NSHTTPURLResponse *)response) statusCode];
			
			// Report the error response in the Simulator (if it's not about our analytics)
			if (statusCode >= 400 && strstr([customConnection.url UTF8String], "http://stats.coronalabs.com/analytics/") == NULL)
			{
				// Report the unsuccessful response in the Simulator pour l'aidez le debugging
				Rtt_TRACE_SIM( ( "Warning: Network request got %ld response: %s", statusCode, [customConnection.url UTF8String] ) );
			}
		}

		NetworkRequestEvent e( [customConnection.url UTF8String], responseString, (int) statusCode );
		e.DispatchEvent( * resource );
#endif
	}
	else if ( block )
	{
		block( customConnection.data );
	}
}

- (void)connection:(NSURLConnection *)connection dispatchError:(NSString *)s
{
	using namespace Rtt;
	
	CustomURLConnection *customConnection = (CustomURLConnection *)connection;
#ifndef Rtt_NO_GUI
	LuaResource *resource = customConnection.resource;
#endif

	const char *errorMessage = [s UTF8String];

	// Report the error in the Simulator (if it's not about our analytics which sometimes times out)
	if (errorMessage != NULL && strstr(errorMessage, "http://stats.coronalabs.com/analytics/") == NULL)
	{
		Rtt_TRACE_SIM( ( "Error on network request: %s: %s", [customConnection.url UTF8String], errorMessage ) );
	}
	
#ifndef Rtt_NO_GUI
	if ( resource )
	{
		NetworkRequestEvent e( [customConnection.url UTF8String], errorMessage );
		e.DispatchEvent( * resource );
	}
#endif
}

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response 
{
	CustomURLConnection *customConnection = (CustomURLConnection *)connection;

#ifdef Rtt_IPHONE_ENV
	// Workaround for Apple Radar: 10412199
	[customConnection cancelTimeout];
#endif

	// It can be called multiple times, for example in the case of a
	// redirect, so each time we reset the data.
	[customConnection.data setLength:0];

	customConnection.response = response;
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error 
{
#ifdef Rtt_IPHONE_ENV
	CustomURLConnection *customConnection = (CustomURLConnection *)connection;

	// Workaround for Apple Radar: 10412199
	[customConnection cancelTimeout];
#endif

	NSString *s = [error localizedDescription];
	[self connection:connection dispatchError:s];
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

id
ApplePlatform::ToValue( lua_State *L, int index )
{
	return [CoronaLuaObjC toValue:L forLuaValue:index];
}

NSMutableDictionary*
ApplePlatform::CreateDictionary( lua_State *L, int t )
{
	return [CoronaLuaObjC toDictionary:L forTable:t];
}

void
ApplePlatform::CopyDictionary( lua_State *L, int index, NSDictionary *src )
{
	[CoronaLuaObjC copyEntries:L fromDictionary:src toTable:index];
}

int
ApplePlatform::CreateAndPushTable( lua_State *L, NSDictionary *src )
{
	return [CoronaLuaObjC pushTable:L forValue:src];
}

bool
ApplePlatform::CreateAndPushArray( lua_State *L, NSArray* src )
{
	return [CoronaLuaObjC pushArray:L forValue:src];
}

bool
ApplePlatform::Push( lua_State *L, id value )
{
	return [CoronaLuaObjC pushLuaValue:L forValue:value];
}

// ----------------------------------------------------------------------------

ApplePlatform::ApplePlatform()
:	fAllocator( Rtt_AllocatorCreate() ),
	fResourcePath( nil ),
	fProjectResourcePath( nil ),
	fSkinResourcePath( nil ),
	fAudioPlayer( NULL ),
	fHttpPostDelegate( nil ),
	fCustomConnectionDelegate( nil ),
	fCrypto(),
	fClass( NULL )
{
}

ApplePlatform::~ApplePlatform()
{
	[fHttpPostDelegate release];
	[fCustomConnectionDelegate release];
    [fProjectResourcePath release];
    [fSkinResourcePath release];

#ifndef Rtt_NO_GUI
	Rtt_DELETE( fAudioPlayer );
#endif // Rtt_NO_GUI
	[fResourcePath release];
	Rtt_AllocatorDestroy( fAllocator );
}

void
ApplePlatform::SetResourceDirectory( NSString *resourcePath )
{
    Rtt_ASSERT( ! fResourcePath );
    
    if ( resourcePath )
    {
        fResourcePath = [resourcePath copy];
    }
}

void
ApplePlatform::SetProjectResourceDirectory( NSString *projectResourcePath )
{
    if ( projectResourcePath )
    {
        [fProjectResourcePath release];
        fProjectResourcePath = [projectResourcePath copy];
    }
}

void
ApplePlatform::SetSkinResourceDirectory( NSString *skinResourcePath )
{
    if ( skinResourcePath )
    {
        [fSkinResourcePath release];
        fSkinResourcePath = [skinResourcePath copy];
    }
}

Rtt_Allocator&
ApplePlatform::GetAllocator() const
{
	return * fAllocator;
}

RenderingStream*
ApplePlatform::CreateRenderingStream( bool antialias ) const
{
#ifdef OLD_GRAPHICS

#ifdef Rtt_NO_GUI
	return NULL;
#else
	RenderingStream* result = NULL;
	if ( antialias )
	{	
		result = Rtt_NEW( fAllocator, GPUMultisampleStream( fAllocator ) );
	}
	else
	{
		result = Rtt_NEW( fAllocator, GPUStream( fAllocator ) );
	}

	result->SetProperty( RenderingStream::kFlipHorizontalAxis, true );
	return result;
#endif

#else
	return NULL;
#endif
}

PlatformSurface*
ApplePlatform::CreateOffscreenSurface( const PlatformSurface& parent ) const
{
#ifdef Rtt_NO_GUI
	return NULL;
#else
	OffscreenGPUSurface *result = Rtt_NEW( Allocator(), OffscreenGPUSurface( parent ) );

	if ( result && ! result->IsValid() )
	{
		Rtt_DELETE( result );
		result = NULL;
	}

	return result;
#endif
}

PlatformTimer*
ApplePlatform::CreateTimerWithCallback( MCallback& callback ) const
{
	return Rtt_NEW( fAllocator, AppleTimer( callback ) );
}

PlatformBitmap*
ApplePlatform::CreateBitmap( const char *path, bool convertToGrayscale ) const
{
#ifdef Rtt_NO_GUI
	return NULL;
#else
//	const char* path = PathForFile( filename, MPlatform::kResourceDir, false );
	return path ? Rtt_NEW( & GetAllocator(), AppleFileBitmap( path, convertToGrayscale ) ) : NULL;
#endif
}

PlatformBitmap*
ApplePlatform::CreateBitmapMask( const char str[], const PlatformFont& font, Real w, Real h, const char alignment[], Real& baselineOffset ) const
{
#ifdef Rtt_NO_GUI
	return NULL;
#else
	return Rtt_NEW( & GetAllocator(), AppleTextBitmap( str, font, w, h, alignment, baselineOffset ) );
#endif
}

const MCrypto&
ApplePlatform::GetCrypto() const
{
	return fCrypto;
}

void
ApplePlatform::GetPreference( Category category, Rtt::String * value ) const
{
	const char *result = NULL;

	if ( kUILanguage == category )
	{
		NSUserDefaults* defs = [NSUserDefaults standardUserDefaults];
		NSArray* languages = [defs objectForKey:@"AppleLanguages"];
		NSString* preferredLang = [languages objectAtIndex:0];
		result = [preferredLang UTF8String];
	}
	else
	{
		NSLocale *locale = [NSLocale autoupdatingCurrentLocale];

		NSString *key = nil;
		switch ( category )
		{
			case kLocaleIdentifier:
				key = NSLocaleIdentifier;
				break;
			case kLocaleLanguage:
				key = NSLocaleLanguageCode;
				break;
			case kLocaleCountry:
				key = NSLocaleCountryCode;
				break;
			default:
				Rtt_ASSERT_NOT_REACHED();
				break;
		}

		if ( key )
		{
			result = [[locale objectForKey:key] UTF8String];
		}
	}

	value->Set( result );
}

static Rtt::OperationResult
CreateFailureResultForUnknownPreferenceCategory( const char* categoryName )
{
	std::string message;
	message = "Category name '";
	message += categoryName ? categoryName : "";
	message += "' is not supported on this platform.";
	return Rtt::OperationResult::FailedWith( message.c_str() );
}
	
Preference::ReadValueResult
ApplePlatform::GetPreference( const char* categoryName, const char* keyName ) const
{
	// Do not continue if category name is not "app".
	if (Rtt_StringCompare(categoryName, Preference::kCategoryNameApp) != 0)
	{
		Rtt::OperationResult result = CreateFailureResultForUnknownPreferenceCategory(categoryName);
		return Preference::ReadValueResult::FailedWith(result.GetUtf8MessageAsSharedPointer());
	}
	
	// Fetch the preferences accessing interface.
	NSUserDefaults* userDefaultsPointer = [NSUserDefaults standardUserDefaults];
	if (!userDefaultsPointer)
	{
		return Preference::ReadValueResult::FailedWith("Failed to access NSUserDefaults.");
	}
	
	// Convert the given preference key string to an NSString.
	NSString* nsKeyName = [NSString stringWithExternalString:keyName];
	if (!nsKeyName)
	{
		return Preference::ReadValueResult::FailedWith("Preference key name cannot be nil.");
	}
	
	// Fetch the preference value.
	id objectValue = [userDefaultsPointer objectForKey:nsKeyName];
	
	// Do not continue if the preference was not found.
	if (nil == objectValue)
	{
		return Preference::ReadValueResult::kPreferenceNotFound;
	}
	
	// Preference was found. Return a success result if the value type is supported by Corona.
	if ([objectValue isKindOfClass:[NSString class]])
	{
		return Preference::ReadValueResult::SucceededWith(PreferenceValue([objectValue UTF8String]));
	}
	else if ([objectValue isKindOfClass:[NSURL class]])
	{
		return Preference::ReadValueResult::SucceededWith(PreferenceValue([[objectValue absoluteString] UTF8String]));
	}
	else if ([objectValue isKindOfClass:[NSData class]])
	{
		Rtt::SharedConstStdStringPtr sharedStringPointer =
				Rtt_MakeSharedConstStdStringPtr((const char*)[objectValue bytes], [objectValue length]);
		return Preference::ReadValueResult::SucceededWith(sharedStringPointer);
	}
	else if ([objectValue isKindOfClass:[NSNumber class]])
	{
		const char* typeName = [objectValue objCType];
		if (Rtt_StringCompare(typeName, @encode(BOOL)) == 0)
		{
			bool boolValue = [objectValue boolValue] != NO;
			return Preference::ReadValueResult::SucceededWith(PreferenceValue(boolValue));
		}
		else if (Rtt_StringCompare(typeName, @encode(float)) == 0)
		{
			return Preference::ReadValueResult::SucceededWith(PreferenceValue([objectValue floatValue]));
		}
		else if (Rtt_StringCompare(typeName, @encode(double)) == 0)
		{
			return Preference::ReadValueResult::SucceededWith(PreferenceValue([objectValue doubleValue]));
		}
		else if (Rtt_StringCompare(typeName, @encode(char)) == 0)
		{
			return Preference::ReadValueResult::SucceededWith(PreferenceValue([objectValue charValue]));
		}
		else if (Rtt_StringCompare(typeName, @encode(short)) == 0)
		{
			return Preference::ReadValueResult::SucceededWith(PreferenceValue([objectValue shortValue]));
		}
		else if (Rtt_StringCompare(typeName, @encode(int)) == 0)
		{
			return Preference::ReadValueResult::SucceededWith(PreferenceValue([objectValue intValue]));
		}
		else if (Rtt_StringCompare(typeName, @encode(long)) == 0)
		{
#ifdef __LP64__
			return Preference::ReadValueResult::SucceededWith(PreferenceValue([objectValue longLongValue]));
#else
			return Preference::ReadValueResult::SucceededWith(PreferenceValue([objectValue intValue]));
#endif
		}
		else if (Rtt_StringCompare(typeName, @encode(long long)) == 0)
		{
			return Preference::ReadValueResult::SucceededWith(PreferenceValue([objectValue longLongValue]));
		}
		else if (Rtt_StringCompare(typeName, @encode(unsigned char)) == 0)
		{
			return Preference::ReadValueResult::SucceededWith(PreferenceValue([objectValue unsignedCharValue]));
		}
		else if (Rtt_StringCompare(typeName, @encode(unsigned short)) == 0)
		{
			return Preference::ReadValueResult::SucceededWith(PreferenceValue([objectValue unsignedShortValue]));
		}
		else if (Rtt_StringCompare(typeName, @encode(unsigned int)) == 0)
		{
			return Preference::ReadValueResult::SucceededWith(PreferenceValue([objectValue unsignedIntValue]));
		}
		else if (Rtt_StringCompare(typeName, @encode(unsigned long)) == 0)
		{
#ifdef __LP64__
			return Preference::ReadValueResult::SucceededWith(PreferenceValue([objectValue unsignedLongLongValue]));
#else
			return Preference::ReadValueResult::SucceededWith(PreferenceValue([objectValue unsignedIntValue]));
#endif
		}
		else if (Rtt_StringCompare(typeName, @encode(unsigned long long)) == 0)
		{
			return Preference::ReadValueResult::SucceededWith(PreferenceValue([objectValue unsignedLongLongValue]));
		}
	}
	
	// Preference value type is not supported. Return a failure result.
	const char* className = NULL;
	NSString *nsClassName = NSStringFromClass([objectValue class]);
	if (nsClassName)
	{
		className = [nsClassName UTF8String];
	}
	if (!className)
	{
		className = "unknown";
	}
	std::string errorMessage;
	errorMessage = "Preference value was found, but it cannot be returned because it is of unsupported type: ";
	errorMessage += className;
	return Preference::ReadValueResult::FailedWith(errorMessage.c_str());
}

OperationResult
ApplePlatform::SetPreferences( const char* categoryName, const PreferenceCollection& collection ) const
{
	// Do not continue if category name is not "app".
	if (Rtt_StringCompare(categoryName, Preference::kCategoryNameApp) != 0)
	{
		return CreateFailureResultForUnknownPreferenceCategory(categoryName);
	}
	
	// Fetch the preferences accessing interface.
	NSUserDefaults* userDefaultsPointer = [NSUserDefaults standardUserDefaults];
	if (!userDefaultsPointer)
	{
		return OperationResult::FailedWith("Failed to access NSUserDefaults.");
	}
	
	// Write the collection of preferences to user defaults.
	int writeCount = 0;
	for (int index = 0; index < collection.GetCount(); index++)
	{
		// Fetch the next preference.
		Preference* preferencePointer = collection.GetByIndex(index);
		if (!preferencePointer)
		{
			continue;
		}
		
		// Convert the given preference key string to an NSString.
		NSString* nsKeyName = [NSString stringWithExternalString:preferencePointer->GetKeyName()];
		if (!nsKeyName)
		{
			continue;
		}
		
		// Write the preference value to user defaults.
		PreferenceValue preferenceValue = preferencePointer->GetValue();
		switch (preferenceValue.GetType())
		{
			case PreferenceValue::kTypeBoolean:
			{
				BOOL boolValue = preferenceValue.ToBoolean().GetValue() ? YES : NO;
				[userDefaultsPointer setBool:boolValue forKey:nsKeyName];
				writeCount++;
				break;
			}
			case PreferenceValue::kTypeSignedInt8:
			{
				NSNumber* numberValue = [NSNumber numberWithChar:preferenceValue.ToSignedInt8().GetValue()];
				if (numberValue)
				{
					[userDefaultsPointer setObject:numberValue forKey:nsKeyName];
					writeCount++;
				}
				break;
			}
			case PreferenceValue::kTypeSignedInt16:
			{
				NSNumber* numberValue = [NSNumber numberWithShort:preferenceValue.ToSignedInt16().GetValue()];
				if (numberValue)
				{
					[userDefaultsPointer setObject:numberValue forKey:nsKeyName];
					writeCount++;
				}
				break;
			}
			case PreferenceValue::kTypeSignedInt32:
			{
				NSNumber* numberValue = [NSNumber numberWithInt:preferenceValue.ToSignedInt32().GetValue()];
				if (numberValue)
				{
					[userDefaultsPointer setObject:numberValue forKey:nsKeyName];
					writeCount++;
				}
				break;
			}
			case PreferenceValue::kTypeSignedInt64:
			{
				NSNumber* numberValue = [NSNumber numberWithLongLong:preferenceValue.ToSignedInt64().GetValue()];
				if (numberValue)
				{
					[userDefaultsPointer setObject:numberValue forKey:nsKeyName];
					writeCount++;
				}
				break;
			}
			case PreferenceValue::kTypeUnsignedInt8:
			{
				NSNumber* numberValue = [NSNumber numberWithUnsignedChar:preferenceValue.ToUnsignedInt8().GetValue()];
				if (numberValue)
				{
					[userDefaultsPointer setObject:numberValue forKey:nsKeyName];
					writeCount++;
				}
				break;
			}
			case PreferenceValue::kTypeUnsignedInt16:
			{
				NSNumber* numberValue = [NSNumber numberWithUnsignedShort:preferenceValue.ToUnsignedInt16().GetValue()];
				if (numberValue)
				{
					[userDefaultsPointer setObject:numberValue forKey:nsKeyName];
					writeCount++;
				}
				break;
			}
			case PreferenceValue::kTypeUnsignedInt32:
			{
				NSNumber* numberValue = [NSNumber numberWithUnsignedInt:preferenceValue.ToUnsignedInt32().GetValue()];
				if (numberValue)
				{
					[userDefaultsPointer setObject:numberValue forKey:nsKeyName];
					writeCount++;
				}
				break;
			}
			case PreferenceValue::kTypeUnsignedInt64:
			{
				NSNumber* numberValue = [NSNumber numberWithUnsignedLongLong:preferenceValue.ToUnsignedInt64().GetValue()];
				if (numberValue)
				{
					[userDefaultsPointer setObject:numberValue forKey:nsKeyName];
					writeCount++;
				}
				break;
			}
			case PreferenceValue::kTypeFloatSingle:
			{
				[userDefaultsPointer setFloat:preferenceValue.ToFloatSingle().GetValue() forKey:nsKeyName];
				writeCount++;
				break;
			}
			case PreferenceValue::kTypeFloatDouble:
			{
				[userDefaultsPointer setDouble:preferenceValue.ToFloatDouble().GetValue() forKey:nsKeyName];
				writeCount++;
				break;
			}
			case PreferenceValue::kTypeString:
			default:
			{
				Rtt::SharedConstStdStringPtr sharedStringPointer = preferenceValue.ToString().GetValue();
				if (sharedStringPointer.NotNull() && !sharedStringPointer->empty())
				{
					auto nullCharacterIndex = sharedStringPointer->find('\0');
					if ((nullCharacterIndex != std::string::npos) && ((nullCharacterIndex + 1) < sharedStringPointer->length()))
					{
						// The string contains at least 1 embedded null character.
						// Write it as a byte array so that the string won't get truncated.
						NSData* nsDataValue = [NSData dataWithBytes:sharedStringPointer->c_str() length:sharedStringPointer->length()];
						if (nsDataValue)
						{
							[userDefaultsPointer setObject:nsDataValue forKey:nsKeyName];
							writeCount++;
						}
					}
					else
					{
						// Insert the value as a null terminated string.
						// This is preferred since it'll be human readable.
						NSString* nsStringValue = [NSString stringWithExternalString:sharedStringPointer->c_str()];
						if (!nsStringValue)
						{
							nsStringValue = @"";
						}
						[userDefaultsPointer setObject:nsStringValue forKey:nsKeyName];
						writeCount++;
					}
				}
				else
				{
					[userDefaultsPointer setObject:@"" forKey:nsKeyName];
					writeCount++;
				}
				break;
			}
		}
	}
	
	// Save the above preference changes to storage.
	const BOOL wasSynchronized = [userDefaultsPointer synchronize];
	if (wasSynchronized == NO)
	{
		return OperationResult::FailedWith("Failed to save preferences to storage.");
	}
	
	// Return a failure if at least 1 preference has failed to be written to storage.
	if (writeCount < collection.GetCount())
	{
		return OperationResult::FailedWith("Failed to write all preferences to storage.");
	}
	
	// Success! All preferences were successfully saved to storage.
	return OperationResult::kSucceeded;
}

OperationResult
ApplePlatform::DeletePreferences( const char* categoryName, const char** keyNameArray, U32 keyNameCount ) const
{
	// Do not continue if category name is not "app".
	if (Rtt_StringCompare(categoryName, Preference::kCategoryNameApp) != 0)
	{
		return CreateFailureResultForUnknownPreferenceCategory(categoryName);
	}
	
	// Validate key name array.
	if (!keyNameArray || (keyNameCount < 1))
	{
		return OperationResult::FailedWith("Preference key name array cannot be null or empty.");
	}
	
	// Fetch the preferences accessing interface.
	NSUserDefaults* userDefaultsPointer = [NSUserDefaults standardUserDefaults];
	if (!userDefaultsPointer)
	{
		return OperationResult::FailedWith("Failed to access NSUserDefaults.");
	}
	
	// Delete the given preferences from user defaults.
	for (U32 index = 0; index < keyNameCount; index++)
	{
		NSString* nsKeyName = [NSString stringWithExternalString:keyNameArray[index]];
		if (nsKeyName)
		{
			[userDefaultsPointer removeObjectForKey:nsKeyName];
		}
	}
	
	// Commit the above preference deletions to storage.
	const BOOL wasSynchronized = [userDefaultsPointer synchronize];
	if (wasSynchronized == NO)
	{
		return OperationResult::FailedWith("Failed to delete preferences from storage.");
	}
	
	// We've successfully deleted the given preferences.
	return OperationResult::kSucceeded;
}

// ----------------------------------------------------------------------------
	
PlatformEventSound *
ApplePlatform::CreateEventSound( const ResourceHandle<lua_State> &handle, const char * filePath ) const
{
#ifdef Rtt_NO_GUI
	return NULL;
#else
	AppleEventSound * eventSound = Rtt_NEW( GetAllocator(), AppleEventSound( handle ) );
	
	if ( ! eventSound->Load( filePath ) )
	{
		Rtt_DELETE( eventSound );
		eventSound = NULL;
	}
	
	return eventSound;
#endif
}

void
ApplePlatform::ReleaseEventSound( PlatformEventSound * sound ) const
{
#ifdef Rtt_NO_GUI
#else
	sound->ReleaseOnComplete();
#endif
}

void
ApplePlatform::PlayEventSound( PlatformEventSound * sound ) const
{
#ifdef Rtt_NO_GUI
#else
	sound->Play();
#endif
}


// ----------------------------------------------------------------------------
	
PlatformAudioRecorder * 
ApplePlatform::CreateAudioRecorder( const ResourceHandle<lua_State> & handle, const char * filePath ) const
{
#ifdef Rtt_NO_GUI
	return NULL;
#else
	AppleAudioRecorder * recorder = Rtt_NEW( GetAllocator(), AppleAudioRecorder( handle, GetAllocator(), filePath ) );
	
	return recorder;
#endif
}


// ----------------------------------------------------------------------------
	
PlatformAudioPlayer*
ApplePlatform::GetAudioPlayer( const ResourceHandle<lua_State> & handle ) const
{
#ifdef Rtt_NO_GUI
	return NULL;
#else
	if ( ! fAudioPlayer )
	{
		fAudioPlayer = Rtt_NEW( fAllocator, AppleAudioPlayer( handle ) );
	}

	return fAudioPlayer;
#endif
}

// ----------------------------------------------------------------------------

int 
ApplePlatform::GetStatusBarHeight() const
{
	return 20;
}
    
int
ApplePlatform::GetTopStatusBarHeightPixels() const
{
    return 0;
}
int
ApplePlatform::GetBottomStatusBarHeightPixels() const
{
    return 0;
}

// ----------------------------------------------------------------------------

Real
ApplePlatform::GetStandardFontSize() const
{
#ifdef Rtt_NO_GUI
	return Rtt_REAL_0;
#else
	static Real sSize = -1;

	if ( sSize < 0.f )
	{
		#if defined( Rtt_IPHONE_ENV ) || defined( Rtt_MAC_ENV )
			sSize = [AppleFont systemFontSize];
		#else
			AppleFont *font = [AppleFont preferredFontForTextStyle:UIFontTextStyleBody];
			sSize = [font pointSize];
		#endif
	}
	
	return sSize;
#endif
}

PlatformFont*
ApplePlatform::CreateFont( PlatformFont::SystemFont fontType, Rtt_Real size ) const
{
#ifdef Rtt_NO_GUI
	return NULL;
#else
	return Rtt_NEW( fAllocator, AppleFontWrapper( fontType, size ) );
#endif
}

PlatformFont*
ApplePlatform::CreateFont( const char *fontName, Rtt_Real size ) const
{
#ifdef Rtt_NO_GUI
	return NULL;
#else
	PlatformFont* result = NULL;

	if ( fontName )
	{
		result = Rtt_NEW( fAllocator, AppleFontWrapper( fontName, size, GetResourceDirectory() ) );
	}

	return result;
#endif
}

// ----------------------------------------------------------------------------

bool
ApplePlatform::CanShowPopup( const char *name ) const
{
	return false;
}

bool
ApplePlatform::ShowPopup( lua_State *L, const char *name, int optionsIndex ) const
{
	return false;
}

bool
ApplePlatform::HidePopup( const char *name ) const
{
	return false;
}

// ----------------------------------------------------------------------------

void*
ApplePlatform::CreateAndScheduleNotification( lua_State *L, int index ) const
{
	return NULL;
}

void
ApplePlatform::ReleaseNotification( void *notificationId ) const
{
}

void
ApplePlatform::CancelNotification( void *notificationId ) const
{
}

// ----------------------------------------------------------------------------

void
ApplePlatform::FlurryInit( const char * applicationKey ) const
{
}

void
ApplePlatform::FlurryEvent( const char * eventId ) const
{
}

// ----------------------------------------------------------------------------

void
ApplePlatform::SetNativeProperty( lua_State *L, const char *key, int valueIndex ) const
{
}

int
ApplePlatform::PushNativeProperty( lua_State *L, const char *key ) const
{
	return 0;
}

int
ApplePlatform::PushSystemInfo( lua_State *L, const char *key ) const
{
	// Validate.
	if ( !L )
	{
		return 0;
	}

	// Push the requested system information to Lua.
	int pushedValues = 0;
	if ( Rtt_StringCompare( key, "isoCountryCode" ) == 0 )
	{
		NSLocale *locale = [NSLocale autoupdatingCurrentLocale];
		lua_pushstring( L, [[locale objectForKey:NSLocaleCountryCode] UTF8String] );
		pushedValues = 1;
	}
	else if ( Rtt_StringCompare( key, "isoLanguageCode" ) == 0 )
	{
		NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
		NSArray *preferredLanguages = [userDefaults objectForKey:@"AppleLanguages"];
		lua_pushstring( L, [[preferredLanguages objectAtIndex:0] UTF8String] );
		pushedValues = 1;
	}
	else if ( Rtt_StringCompare( key, "bundleID" ) == 0 )
	{
#ifdef Rtt_AUTHORING_SIMULATOR
		const char * bundleId = "com.coronalabs.NewApp";
#else
		const char * bundleId = [[[NSBundle mainBundle] bundleIdentifier] UTF8String];
#endif
		lua_pushstring( L, bundleId );
		pushedValues = 1;
	}
	else
	{
		// Push nil if given a key that is unknown on this platform.
		lua_pushnil( L );
		pushedValues = 1;
	}

	// Return the number of values pushed into Lua.
	return pushedValues;
}

// ----------------------------------------------------------------------------

void
ApplePlatform::HttpRequest( NSString *url, NSString *method, NSDictionary *params, NSString *body, void (^block)( NSData* ) ) const
{
	NSURL *nsUrl = [NSURL URLWithString:url];
	NSMutableURLRequest *request = [[[NSMutableURLRequest alloc] initWithURL:nsUrl] autorelease];
	[request setHTTPMethod:method];

	if ( params )
	{
		for ( id k in params )
		{
			id v = [params valueForKey:k];
			[request addValue:k forHTTPHeaderField:v];
		}
	}

	if ( body )
	{
		[request setHTTPBody:[body dataUsingEncoding:NSUTF8StringEncoding]];
	}

	if ( ! fCustomConnectionDelegate )
	{
		fCustomConnectionDelegate = [[CustomConnectionDelegate alloc] init];
	}

	CustomURLConnection *customConnection =
		[[CustomURLConnection alloc] initWithRequest:request delegate:fCustomConnectionDelegate block:block];
	[customConnection start];
	[customConnection autorelease];
}

void
ApplePlatform::NetworkBaseRequest( lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex, NSString *path ) const
{
	NSURL *nsUrl = [NSURL URLWithString:[NSString stringWithExternalString:url]];

	CustomMutableURLRequest *request = [[[CustomMutableURLRequest alloc] initWithURL:nsUrl] autorelease];

	NSString *methodString = [NSString stringWithExternalString:method];
	[request setHTTPMethod:methodString];

	[request setTimeoutInterval:30]; // Default to 30 seconds

	if ( paramsIndex > 0 )
	{
		Rtt_ASSERT( lua_istable( L, paramsIndex ) );

		lua_getfield( L, paramsIndex, "headers" );
		if ( lua_istable( L, -1 ) )
		{
			lua_pushnil( L );
			while ( lua_next( L, -2 ) != 0 )
			{
				const char * key = lua_tostring( L, -2 );
				const char * value = lua_tostring( L, -1 );
				
				NSString *nsKey = [NSString stringWithExternalString:key];
				NSString *nsValue = [NSString stringWithExternalString:value];
				
				[request addValue:nsValue forHTTPHeaderField:nsKey];
				
				lua_pop( L, 1 );
			}		
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, paramsIndex, "body" );
		if ( lua_isstring( L, -1 ) )
		{
			const char * body = lua_tostring( L, -1 );
			[request setHTTPBody:[[NSString stringWithExternalString:body] dataUsingEncoding:NSUTF8StringEncoding]];
		}
		lua_pop( L, 1 );

		lua_getfield( L, paramsIndex, "timeout" );
		if ( lua_isnumber( L, -1 ) )
		{
			NSTimeInterval timeoutInSeconds = lua_tonumber( L, -1 );

			// Ensure timeout is non-negative
			timeoutInSeconds = Max( timeoutInSeconds, 0.0 ); 

			[request setTimeoutInterval:timeoutInSeconds];
		}
		lua_pop( L, 1 );
	}

	if ( ! fCustomConnectionDelegate )
	{
		fCustomConnectionDelegate = [[CustomConnectionDelegate alloc] init];
	}

	CustomURLConnection *customConnection =
		[[CustomURLConnection alloc] initWithRequest:request delegate:fCustomConnectionDelegate filePath:path resource:listener];
	[customConnection start];
	[customConnection autorelease];
}

void
ApplePlatform::NetworkRequest( lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex ) const
{
	NetworkBaseRequest( L, url, method, listener, paramsIndex, NULL );
}

void
ApplePlatform::NetworkDownload( lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex, const char *filename, MPlatform::Directory baseDir ) const
{
#if defined( Rtt_NO_GUI ) && defined( Rtt_ALLOCATOR_SYSTEM )
	String filePath;
#else
	Runtime& runtime = * LuaContext::GetRuntime( L );
	String filePath( runtime.GetAllocator() );
#endif
	PathForFile( filename, baseDir, MPlatform::kDefaultPathFlags, filePath );
	NSString *path = [NSString stringWithExternalString:filePath.GetString()];
	NetworkBaseRequest( L, url, method, listener, paramsIndex, path );
}

PlatformReachability*
ApplePlatform::NewReachability( const ResourceHandle<lua_State>& handle, PlatformReachability::PlatformReachabilityType type, const char* address ) const
{
#ifdef Rtt_NO_GUI
	return NULL;
#else
	PlatformReachability* result = NULL;

	result = Rtt_NEW( fAllocator, AppleReachability( handle, type, address ) );
//	result = new Rtt::AppleReachability( handle, type, address );

	return result;
#endif
}

bool
ApplePlatform::SupportsNetworkStatus() const
{
	return true;
}
	
// ----------------------------------------------------------------------------

void
ApplePlatform::RaiseError( MPlatform::Error e, const char* reason ) const
{
	const char kNull[] = "(null)";

	if ( ! reason ) { reason = kNull; }
	Rtt_TRACE( ( "MPlatformFactory error(%d): %s\n", e, kNull ) );
}

bool
ApplePlatform::ValidateAssetFile(const char *assetFilename, const int assetSize) const
{
	NSString *pathStr = PathForSystemResourceFile(assetFilename);

	// For now we just check the size which is good enough for image files;
	// perhaps check an MD5 of the first 1KB in the future
	return (assetFilename != NULL &&
			([[NSFileManager defaultManager] attributesOfItemAtPath:pathStr error:nil].fileSize == (unsigned long long)assetSize));
}

bool
ApplePlatform::FileExists( const char * path ) const
{
	NSString * s = [NSString stringWithExternalString:path];
	
	return ( path && [[NSFileManager defaultManager] fileExistsAtPath:s] );
}

NSString*
ApplePlatform::PathForDir( const char *folderName, NSString *baseDir, bool createIfNotExist ) const
{
	NSString *result = nil;

	NSFileManager *fileMgr = [NSFileManager defaultManager];
	if ( fileMgr )
	{
		NSString *path = PathForFile( folderName, baseDir );
		if ( path )
		{
			BOOL isDir = NO;
			if ( ! [fileMgr fileExistsAtPath:path isDirectory:&isDir] )
			{
				if ( createIfNotExist )
				{
					// Create folder
					if ( [fileMgr createDirectoryAtPath:path
							withIntermediateDirectories:YES
											 attributes:nil
												  error:nil] )
					{
						result = path;
					}
				}
			}
			else if ( Rtt_VERIFY( isDir ) )
			{
				// File exists at path and is also a dir
				result = path;
			}
			else
			{
				Rtt_TRACE_SIM( ( "ERROR: Non-directory file already exists at %s\n.", [result UTF8String] ) );
				result = nil;
			}
		}
	}

	return result;
}

// Returns 'path' if file exists at 'path'. Otherwise, returns 'nil'.
static NSString *
VerifyFileExists( NSString *path )
{
	if ( path && ! [[NSFileManager defaultManager] fileExistsAtPath:path] )
	{
		path = nil;
	}

	return path;
}

void 
ApplePlatform::PathForFile( const char* filename, Directory baseDir, U32 flags, String & result ) const
{
	NSString* path = nil;
	bool testExistence = flags & MPlatform::kTestFileExists;

	result.Set( NULL );
	
	switch( baseDir )
	{
		case MPlatform::kResourceDir:
			path = PathForResourceFile( filename );
			path = VerifyFileExists( path );
			if ( ! path )
			{
				// The file may be not be in the developer's project because it is
				// a Corona resource file that will be available in the final device build.
				// For example, image files used by the widget library.
				path = PathForCoronaResourceFile( filename );
				path = VerifyFileExists( path );
			}
			if ( ! path )
			{
				// Lastly, check if the file exists in a plugin that has been linked with the app
				path = PathForPluginsFile( filename );
				path = VerifyFileExists( path );
			}
			Rtt_WARN_SIM(
				testExistence || ! filename || path,
				( "WARNING: Cannot create path for resource file '%s'. File does not exist.\n", filename ) );
			break;

		case MPlatform::kSystemResourceDir:
			path = PathForSystemResourceFile( filename );
			break;
            
		case MPlatform::kProjectResourceDir:
			path = PathForProjectResourceFile( filename );
			break;
            
		case MPlatform::kSkinResourceDir:
			path = PathForSkinResourceFile( filename );
			break;
            
		case MPlatform::kTmpDir:
			path = PathForTmpFile( filename );
			break;

		case MPlatform::kCachesDir:
			path = PathForCachesFile( filename );
			break;

		case MPlatform::kSystemCachesDir:
			path = PathForSystemCachesFile( filename );
			break;

		case MPlatform::kPluginsDir:
			path = PathForPluginsFile( filename );
			break;

		case MPlatform::kApplicationSupportDir:
			path = PathForApplicationSupportFile( filename );
			break;

		case MPlatform::kDocumentsDir:
		default:
			path = PathForDocumentsFile( filename );
			break;
	}

	if ( testExistence && path && ! [[NSFileManager defaultManager] fileExistsAtPath:path] )
	{
		path = nil;
	}

	result.SetSrc( [path UTF8String] );
}

NSString*
ApplePlatform::PathForResourceFile( const char* filename ) const
{
	NSString* result = NULL;

	if ( fResourcePath )
	{
		NSString* path = fResourcePath;

		if ( filename )
		{
			path = [NSString stringWithFormat:@"%@/%@", path,
					[[NSFileManager defaultManager] stringWithFileSystemRepresentation:filename length:strlen(filename)]];
		}

		result = path;
	}
	else
	{
		result = PathForSystemResourceFile( filename );
	}

	return result;
}

NSString*
ApplePlatform::PathForSystemResourceFile( const char* filename ) const
{
    NSString* result = NULL;
    
    NSBundle* bundle = ( ! fClass ? [NSBundle mainBundle] : [NSBundle bundleForClass:fClass] );
    if ( ! Rtt_VERIFY( bundle ) )
    {
        bundle = [NSBundle mainBundle];
    }

#ifdef Rtt_AUTHORING_SIMULATOR
    NSString* resourcePath = [bundle resourcePath];
#else
    NSString* resourcePath = fResourcePath;

    // Handle the case where fResourcePath hasn't been set yet with a sensible default
    if (resourcePath == nil)
    {
        resourcePath = [bundle resourcePath];
    }
#endif

    if ( filename )
    {
		NSString *nsFilename = [NSString stringWithExternalString:filename];

		result = [resourcePath stringByAppendingPathComponent:nsFilename];
		// Since we hand-crafted the path, we might want to verify it actually exists.
		// This check is run by the caller so we could disable this check if we wanted.
		if (! [[NSFileManager defaultManager] fileExistsAtPath:result] )
		{
			// Fallback to recommended resource lookup. This is required because bundle
			// files does not have to be in same folder (for example On Demand Resources).
#ifdef Rtt_AUTHORING_SIMULATOR
			result = [bundle pathForResource:nsFilename ofType:nil];
#else
			if (fResourcePath == nil)
			{
				result = [bundle pathForResource:nsFilename ofType:nil];
			}
#endif
		}
    }
    else
    {
        result = resourcePath;
    }
    return result;
}

NSString*
ApplePlatform::PathForProjectResourceFile( const char* filename ) const
{
    NSString* result = NULL;
    
    if ( fProjectResourcePath == nil )
    {
        Rtt_TRACE_SIM( ( "WARNING: simulator.setProjectResourceDirectory() must be called before using system.ProjectResourceDirectory" ) );
        
        return nil;
    }
    
    if ( filename )
    {
        result = [fProjectResourcePath stringByAppendingPathComponent:[NSString stringWithExternalString:filename]];
    }
    else
    {
        result = fProjectResourcePath;
    }
    
    return result;
}

NSString*
ApplePlatform::PathForSkinResourceFile( const char* filename ) const
{
    NSString* result = NULL;
    
    if ( fSkinResourcePath == nil )
    {
        Rtt_TRACE_SIM( ( "WARNING: simulator.setSkinResourceDirectory() must be called before using system.SkinResourceDirectory" ) );
        
        return nil;
    }
    
    if ( filename )
    {
        result = [fSkinResourcePath stringByAppendingPathComponent:[NSString stringWithExternalString:filename]];
    }
    else
    {
        result = fSkinResourcePath;
    }
    
    return result;
}
    
void
ApplePlatform::SetProjectResourceDirectory( const char* filename )
{
    [fProjectResourcePath release];
    fProjectResourcePath = [[NSString stringWithExternalString:filename] copy];
}

void
ApplePlatform::SetSkinResourceDirectory( const char* filename )
{
    [fSkinResourcePath release];
    fSkinResourcePath = [[NSString stringWithExternalString:filename] copy];
}

NSString *
ApplePlatform::PathForCoronaResourceFile( const char* filename ) const
{
	NSString *result = nil;

	if ( filename )
	{
#if defined(Rtt_CORONA_CARDS_ENV) && defined(Rtt_MAC_ENV)
		NSString *coronaCardsResourceDir = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"../Frameworks/CoronaCards.framework/Resources/"];

		result = [NSString stringWithFormat:@"%@/%@", coronaCardsResourceDir,
				  [[NSFileManager defaultManager] stringWithFileSystemRepresentation:filename length:strlen(filename)]];
#elif defined( Rtt_TVOS_ENV )
		NSString *coronaResourceDir = [[[NSBundle mainBundle] privateFrameworksPath] stringByAppendingPathComponent:@"CoronaCards.framework"];
		result = [coronaResourceDir stringByAppendingPathComponent:[[NSFileManager defaultManager] stringWithFileSystemRepresentation:filename length:strlen(filename)]];
#else
		// On iOS/Mac, Corona resource files are located in the CoronaResources.bundle
		NSBundle *mainBundle = [NSBundle mainBundle];
		NSString *bundlePath = [mainBundle pathForResource:@"CoronaResources" ofType:@"bundle"];

		NSBundle *bundle = [NSBundle bundleWithPath:bundlePath];
		NSString *baseDir = [bundle resourcePath];

		if ( baseDir )
		{
			result = [NSString stringWithFormat:@"%@/%@", baseDir,
					[[NSFileManager defaultManager] stringWithFileSystemRepresentation:filename length:strlen(filename)]];
		}
#endif // Rtt_CORONA_CARDS_ENV && Rtt_MAC_ENV
	}

	return result;
}

static NSString *
GetDirectory( NSSearchPathDirectory directory )
{
	NSArray *paths = NSSearchPathForDirectoriesInDomains( directory, NSUserDomainMask, YES );
	return [paths objectAtIndex:0];
}

NSString*
ApplePlatform::PathForDocumentsFile( const char* filename ) const
{
	NSString *dir = GetDirectory( NSDocumentDirectory );
	return PathForFile( filename, dir );
}

NSString*
ApplePlatform::PathForApplicationSupportFile( const char* filename ) const
{
	NSString *dir = GetDirectory( NSApplicationSupportDirectory );
	BOOL isDir = NO;

	// Sometimes the default application support directory doesn't exist (on iOS, but ensuring
	// it exists everywhere is fine)
	if (! [[NSFileManager defaultManager] fileExistsAtPath:dir isDirectory:&isDir])
	{
		// Create folder
		NSError *error = nil;
		if (! [[NSFileManager defaultManager] createDirectoryAtPath:dir
				withIntermediateDirectories:YES
								 attributes:nil
									  error:&error])
		{
			Rtt_LogException("ERROR: failed to create application support directory: %s (%s)",
							 [dir UTF8String], [[error localizedDescription] UTF8String]);
		}
	}

	return PathForFile( filename, dir );
}

NSString*
ApplePlatform::CachesParentDir() const
{
	return GetDirectory( NSCachesDirectory );
}

NSString*
ApplePlatform::PathForCachesFile( const char* filename ) const
{
	static const char kDirName[] = "Caches";

	NSString *root = CachesParentDir();
	NSString *dir = PathForDir( kDirName, root, true );
	return PathForFile( filename, dir );
}

NSString*
ApplePlatform::PathForSystemCachesFile( const char* filename ) const
{
	static const char kDirName[] = ".system";

	NSString *root = CachesParentDir();
	NSString *dir = PathForDir( kDirName, root, true );
	return PathForFile( filename, dir );
}

NSString*
ApplePlatform::PathForTmpFile( const char* filename ) const
{
	return PathForFile( filename, NSTemporaryDirectory() );
}

NSString *
ApplePlatform::PathForPluginsFile( const char *filename ) const
{
	return nil;
}

NSString*
ApplePlatform::PathForFile( const char* filename, NSString* baseDir ) const
{
	NSString* result;

	if ( filename )
	{
		// Decomposing any Unicode characters makes the filename compatible with the APFS filesystem on iOS 10.3
		NSString *filenameStr = [[NSString stringWithExternalString:filename] decomposedStringWithCompatibilityMapping];
		result = [baseDir stringByAppendingPathComponent:filenameStr];
	}
	else
	{
		result = baseDir;
	}

	return result;
}

	
// Not tested. Can't find documentation. Keys and values are guessed from Google searches and iTunes Music Library.xml.
/*
bool
ApplePlatform::SetSyncTimeMachine( NSString* filepath, bool value, NSString** errorstr ) const
{
	if ( nil == filepath || [filepath length] == 0 )
	{
		return false;
	}
	int result;
	
	// This is a complete guess
	if ( value )
	{
		const char* setvalue = "com.apple.backupd";
		result = setxattr( [filepath fileSystemRepresentation], "com.apple.metadata:com_apple_backup_excludeItem", &value, sizeof(value), 0, 0);
	}
	else
	{
		const char* setvalue = "";
		result = setxattr( [filepath fileSystemRepresentation], "com.apple.metadata:com_apple_backup_excludeItem", &value, sizeof(value), 0, 0);
		
	}
	if ( 0 != result && nil != errorstr )
	{
		*errorstr = [NSString stringWithFormat:@"errno %d: %s", errno, strerror(errno)];
	}
	return ( 0 == result );
}
*/

bool
ApplePlatform::SetSynciCloud( NSString* filepath, bool value, NSString** errorstr ) const
{
	if ( nil == filepath || [filepath length] == 0 )
	{
		return false;
	}
	
	// Our notion of the boolean is backwards compared to Apple.
	// Apple is 'DoNotBackup=true'
	bool donotbackup = ! value;
	
	int result = setxattr( [filepath fileSystemRepresentation], "com.apple.MobileBackup", &donotbackup, sizeof(donotbackup), 0, 0);
	if ( 0 != result && nil != errorstr )
	{
		*errorstr = [NSString stringWithFormat:@"errno %d: %s", errno, strerror(errno)];
	}
	return ( 0 == result );
}


bool
ApplePlatform::GetSynciCloud( NSString* filepath, bool& value, NSString** errorstr ) const
{
	if ( nil == filepath || [filepath length] == 0 )
	{
		return false;
	}
	
	bool donotbackup;
	
	ssize_t result = getxattr( [filepath fileSystemRepresentation], "com.apple.MobileBackup", &donotbackup, sizeof(donotbackup), 0, 0);
	if ( result <= 0 )
	{
		// Special case for 'Attribute not found'
		// This means no attributes have been set at all.
		// Instead of passing an error, just pretend it is 'DoNotBackup=true'
		if ( ENOATTR == errno )
		{
			result = sizeof(donotbackup);
			donotbackup = false;
			errno = 0;
		}
		else if ( nil != errorstr )
		{
			*errorstr = [NSString stringWithFormat:@"errno %d: %s", errno, strerror(errno)];
		}
	}
	
	// Our notion of the boolean is backwards compared to Apple.
	// Apple is 'DoNotBackup=true'
	value = ! donotbackup;
	
    return (result > 0);
}


int
ApplePlatform::SetSync( lua_State* L ) const
{
	int numret = 0;
#ifdef Rtt_NO_GUI
#else
	const char* filename = luaL_checkstring( L, 1 );
	MPlatform::Directory basedir = MPlatform::kDocumentsDir;
	int paramindex = 2;
	if ( lua_type( L, 2 ) == LUA_TLIGHTUSERDATA )
	{
		void* p = lua_touserdata( L, 2 );
		basedir = (MPlatform::Directory)EnumForUserdata(
			LuaLibSystem::Directories(),
			p,
			MPlatform::kNumDirs,
			MPlatform::kDocumentsDir );
		paramindex = 3;
	}
	
	String fullpath( &GetAllocator() );
	// Get the full path after we parse for the baseDir parameter
	PathForFile( filename, basedir, MPlatform::kDefaultPathFlags, fullpath );
	
	if ( lua_type( L, paramindex ) == LUA_TTABLE )
	{
		lua_getfield( L, paramindex, "iCloudBackup");
		if ( lua_isboolean( L, -1 ) )
		{
			bool value = lua_toboolean( L, -1 );
			lua_pop( L, 1 ); // pop iCloudBackup
			NSString* nsfilepath = [NSString stringWithExternalString:fullpath.GetString()];
			NSString* errorstr = nil;
			bool retflag = SetSynciCloud( nsfilepath, value, &errorstr );
			lua_pushboolean( L, retflag );
			if ( ! retflag )
			{
				lua_pushstring( L, [errorstr UTF8String] );
			}
			else
			{
				lua_pushnil( L );
			}
			numret = 2;
		}
		else
		{
			lua_pop( L, 1 ); // pop iCloudBackup
		}
		
		// do other keys here

	}
#endif
	return numret;
}

int
ApplePlatform::GetSync( lua_State* L ) const
{
	int numret = 0;

#ifdef Rtt_NO_GUI
#else
	const char* filename = luaL_checkstring( L, 1 );
	MPlatform::Directory basedir = MPlatform::kDocumentsDir;
	int paramindex = 2;
	if ( lua_type( L, 2 ) == LUA_TLIGHTUSERDATA )
	{
		void* p = lua_touserdata( L, 2 );
		basedir = (MPlatform::Directory)EnumForUserdata(
			LuaLibSystem::Directories(),
			p,
			MPlatform::kNumDirs,
			MPlatform::kDocumentsDir );
		paramindex = 3;
	}
	
	String fullpath( &GetAllocator() );
	// Get the full path after we parse for the baseDir parameter
	PathForFile( filename, basedir, MPlatform::kDefaultPathFlags, fullpath );
	
	if ( lua_type( L, paramindex ) == LUA_TTABLE )
	{
		lua_getfield( L, paramindex, "key");
		if ( lua_isstring( L, -1 ) && 0 == strcmp( "iCloudBackup", lua_tostring( L, -1) ) )
		{
			bool value;
			lua_pop( L, 1 ); // pop iCloudBackup
			NSString* nsfilepath = [NSString stringWithExternalString:fullpath.GetString()];
			NSString* errorstr = nil;
			bool retflag = GetSynciCloud( nsfilepath, value, &errorstr );
			lua_pushboolean( L, value );
			if ( ! retflag )
			{
				lua_pushstring( L, [errorstr UTF8String] );
			}
			else
			{
				lua_pushnil( L );
			}
			numret = 2;
		}
		else
		{
			lua_pop( L, 1 ); // pop iCloudBackup
		}
		
		// do other keys here

	}
#endif

	return numret;
}

void
ApplePlatform::BeginRuntime( const Runtime& runtime ) const
{
}

void
ApplePlatform::EndRuntime( const Runtime& runtime ) const
{
}

PlatformExitCallback*
ApplePlatform::GetExitCallback()
{
	return NULL;
}

bool
ApplePlatform::RequestSystem( lua_State *L, const char *actionName, int optionsIndex ) const
{
	// Validate.
	if ( !actionName )
	{
		return false;
	}

	// Execute the requested operation.
	if ( Rtt_StringCompare( actionName, "validateResourceFile" ) == 0 )
	{
		const char *assetFilename = NULL;
		int assetFileSize = -1;

		if ( LUA_TTABLE == lua_type(L, optionsIndex) )
		{
			lua_getfield( L, -1, "filename" );
			if ((assetFilename = lua_tostring( L, -1 )) == NULL)
			{
				return false;
			}
			lua_pop( L, 1 );

			lua_getfield( L, -1, "size" );
			if (lua_type( L, -1 ) == LUA_TNUMBER)
			{
				assetFileSize = luaL_checkreal( L, -1 );
			}
			lua_pop( L, 1 );
		}

		if (assetFilename != NULL && assetFileSize >= 0)
		{
			return ValidateAssetFile(assetFilename, assetFileSize);
		}
		else
		{
			return false;
		}
	}

	return false;
}

void
ApplePlatform::SetBundleClass( Class newValue )
{
	fClass = newValue;
}

/*
PlatformDictionaryWrapper*
ApplePlatform::CreateDictionary( const KeyValuePair* pairs, int numPairs )
{
	PlatformDictionaryWrapper* result = NULL;

	if ( numPairs > 0 )
	{
		NSMutableArray* keys = [[NSMutableArray alloc] initWithCapacity:numPairs];
		NSMutableArray* values = [[NSMutableArray alloc] initWithCapacity:numPairs];
		for ( int i = 0; i < numPairs; i++ )
		{
			NSString* k = [[NSString alloc] initWithUTF8String:pairs[i].key];
			[keys addObject:k];
			[k release];

			NSString* v = [[NSString alloc] initWithUTF8String:pairs[i].value];
			[values addObject:v];
			[v release];
		}

		NSDictionary* dictionary = [[NSDictionary alloc] initWithObjects:values forKeys:keys];
		result = Rtt_NEW( fAllocator, AppleDictionaryWrapper( dictionary ) );
		[dictionary release];

		[values release];
		[keys release];
	}

	return result;
}
*/

#ifdef Rtt_AUTHORING_SIMULATOR
void
ApplePlatform::SetCursorForRect(const char *cursorName, int x, int y, int width, int height) const
{
    // Only the Mac does anything
}
#endif // Rtt_AUTHORING_SIMULATOR

void
ApplePlatform::Suspend( ) const
{
}

void
ApplePlatform::Resume( ) const
{
}
	
FontMetricsMap
ApplePlatform::GetFontMetrics( const PlatformFont & font ) const
{
#if !defined( Rtt_NO_GUI )
	AppleFont *f = (AppleFont*)font.NativeObject();
	return {
		{ "ascent" ,    f.ascender},
		{ "descent" ,   f.descender},
		{ "leading",    f.leading },
		{ "height" ,    f.ascender - f.descender }
	};
#else
	return FontMetricsMap();
#endif
	
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

