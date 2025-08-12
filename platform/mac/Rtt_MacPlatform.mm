//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_MacPlatform.h"

#include "CoronaLua.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_LuaResource.h"
#include "Rtt_LuaContext.h"
#include "Rtt_MacFBConnect.h"
#include "Rtt_MacImageProvider.h"
#include "Rtt_MacMapViewObject.h"
#include "Rtt_MacTextFieldObject.h"
#include "Rtt_MacTextBoxObject.h"
#include "Rtt_MacVideoPlayer.h"
#include "Rtt_MacViewSurface.h"
#include "Rtt_MacVideoObject.h"
#include "Rtt_MacVideoProvider.h"
#include "Rtt_MacWebPopup.h"
#include "Rtt_MacWebViewObject.h"
#include "Rtt_MacActivityIndicator.h"
#include "Rtt_MacFont.h"
#include "Rtt_PlatformInAppStore.h"
#include "Rtt_AppleInAppStore.h"
#include "Rtt_PlatformPlayer.h"
#include "Rtt_PlatformSimulator.h"
#include "Rtt_PreferenceCollection.h"
#include "Rtt_RenderingStream.h"

//#include "Rtt_AppleDictionaryWrapper.h"
#include "Rtt_MacSimulator.h"
#include "Rtt_MacExitCallback.h"
#import "AppDelegate.h"
#import "GLView.h"
#if Rtt_AUTHORING_SIMULATOR
#import "TextEditorSupport.h"
#endif

#import <AppKit/NSAlert.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSFontManager.h>
#import <AppKit/NSOpenGL.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSWorkspace.h>
#import <Foundation/NSString.h>
#import <SystemConfiguration/SystemConfiguration.h>
#import <netinet/in.h>
#import <IOKit/pwr_mgt/IOPMLib.h>

//#import <CoreFoundation/CoreFoundation.h>
//#import <Security/Security.h>
//#import <CoreServices/CoreServices.h>

#if (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5)
	#import <Foundation/NSFileManager.h>
	#import <Foundation/NSPathUtilities.h>
	#import <Foundation/NSURL.h>
	#include <CoreFoundation/CoreFoundation.h>
	#include <ApplicationServices/ApplicationServices.h>
#endif

#include "Rtt_Lua.h"

#if !defined( Rtt_PROJECTOR )
	#include "Rtt_JavaHost.h"
#endif

#include <pthread.h>
#include <sstream>
#include <sys/sysctl.h>

// ----------------------------------------------------------------------------

NSString* const kOpenFolderPath = @"openFolderPath";
NSString* const kBuildFolderPath = @"buildFolderPath";
NSString* const kKeyStoreFolderPath = @"keyStoreFolderPath";
NSString* const kKeyStoreFolderPathAndFile = @"keyStoreFolderPathAndFile";
NSString* const kDstFolderPath = @"dstFolderPath";
NSString* const kImageFolderPath = @"imageFolderPath";
NSString* const kVideoFolderPath = @"videoFolderPath";
NSString* const kDidAgreeToLicense = @"didAgreeToLicense";
NSString* const kUserPreferenceUsersCurrentSelectedSkin = @"skin";
NSString* const kUserPreferenceCustomBuildID = @"userPreferenceCustomBuildID";
NSString* const kUserPreferenceDoNotUseSkinnedWindows = @"doNotUseSkinnedWindows"; // deprecated
NSString* const kUserPreferenceScaleFactorForSkin = @"scaleFactorForSkin";
NSString* const kUserPreferenceLastIOSCertificate = @"lastIOSCertificate";
NSString* const kUserPreferenceLastTVOSCertificate = @"lastTVOSCertificate";
NSString* const kUserPreferenceLastOSXCertificate = @"lastOSXCertificate";
NSString* const kUserPreferenceLastAndroidKeyAlias = @"lastAndroidKeyAlias";
NSString* const kUserPreferenceLastAndroidKeystore = @"lastAndroidKeystore";
NSString* const kUserPreferenceLastAndroidTargetStore = @"lastAndroidTargetStore";


@interface CustomAlert : NSAlert
{
	Rtt::LuaResource *fResource;
}
@property (nonatomic, readwrite, assign) Rtt::LuaResource *fResource;

-(void)endSheetAndClose;

@end


@implementation CustomAlert

@synthesize fResource;

-(void)dealloc
{
	Rtt_DELETE( fResource );

	[super dealloc];
}

-(void)endSheetAndClose
{
	NSWindow *window = [self window];
	[NSApp endSheet:window];
	[window orderOut:nil];
	[window close];
}

@end

// ----------------------------------------------------------------------------

@interface AlertDelegate : NSObject
{
	NSMutableDictionary *fWrappers;
}

@property (nonatomic, readonly, getter=wrappers) NSMutableDictionary *fWrappers;

-(CustomAlert*)objectForKey:(void*)key;
-(void)setObject:(CustomAlert*)value forKey:(void*)key;
-(void)removeObjectForKey:(void*)key;

@end

@implementation AlertDelegate

@synthesize fWrappers;

-(id)init
{
	self = [super init];
	if ( self )
	{
		fWrappers = [[NSMutableDictionary alloc] init];
	}
	return self;
}

-(void)dealloc
{
	if ( [fWrappers count] > 0 )
	{
		NSArray *wrappers = [fWrappers allValues];
		for( CustomAlert *alert in wrappers )
		{
			// Remove alert from fWrappers so that the alert's modal delegate does nothing
			[self removeObjectForKey:alert];

			// TODO: fResource cannot delete itself b/c its Lua context is bogus
			// by the time it's destructor is called.
			// For now, force a memory leak.
			alert.fResource = NULL;

			[alert endSheetAndClose];
		}
	}

	[fWrappers release];

	[super dealloc];
}

-(CustomAlert*)objectForKey:(void*)key
{
	char buf[sizeof(void*)<<2]; // buffer large enough to hold a hex value of ptr
	sprintf( buf, "%lx", (size_t) key );

	NSString *k = [[NSString alloc] initWithUTF8String:buf];
	id object = [fWrappers objectForKey:k];
	CustomAlert *result = Rtt_VERIFY( ! object || [object isKindOfClass:[CustomAlert class]] ) ? object : nil;
	[k release];

	return result;
}

-(void)setObject:(CustomAlert*)value forKey:(void*)key
{
	char buf[sizeof(void*)<<2]; // buffer large enough to hold a hex value of ptr
	sprintf( buf, "%lx", (size_t) key );

	NSString *k = [[NSString alloc] initWithUTF8String:buf];
	Rtt_ASSERT( nil == [fWrappers objectForKey:k] );
	[fWrappers setValue:value forKey:k];
	[k release];
}

-(void)removeObjectForKey:(void*)key
{
	char buf[sizeof(void*)<<2]; // buffer large enough to hold a hex value of ptr
	sprintf( buf, "%lx", (size_t) key );

	NSString *k = [[NSString alloc] initWithUTF8String:buf];
	Rtt_ASSERT( [fWrappers objectForKey:k] && [[fWrappers objectForKey:k] isKindOfClass:[CustomAlert class]] );
	[fWrappers removeObjectForKey:k];
	[k release];
}

-(void)alertDidEnd:(NSAlert *)alertView returnCode:(NSInteger)returnCode cancelled:(BOOL)cancelled
{
    // This is the recommended way to remove the alert from the screen (see Cocoa docs for beginSheetModalForWindow)
    [[alertView window] orderOut:nil];
    
	if ( Rtt_VERIFY( [alertView isKindOfClass:[CustomAlert class]] ) )
	{
		CustomAlert *alert = (CustomAlert*)alertView;

		if ( [self objectForKey:alert] )
		{
			Rtt::LuaResource *resource = alert.fResource;

			// Remove alert from dictionary before calling Lua callback
			[self removeObjectForKey:alert];

			if ( resource )
			{
				int buttonIndex = (int) returnCode - NSAlertFirstButtonReturn;
				Rtt::LuaLibNative::AlertComplete( * resource, buttonIndex, cancelled );
			}
		}
	}
}

-(void)alertDidEnd:(NSAlert *)alertView returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
	[self alertDidEnd:alertView returnCode:returnCode cancelled:NO];
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static bool
MacOpenURL( const char *url )
{
	bool result = false;

	if ( url )
	{
        NSString *urlString = [NSString stringWithExternalString:url];
        NSURL *urlPlatform = nil;
        NSString *kFileURLPrefix = @"file:";

        if ([urlString hasPrefix:kFileURLPrefix])
        {
            // For reasons that are not obvious, "file:" URLs have to be created differently
            urlString = [urlString stringByReplacingCharactersInRange:NSMakeRange(0, [kFileURLPrefix length]) withString:@""];
            urlPlatform = [NSURL fileURLWithPath:urlString];
        }
        else
        {
            urlPlatform = [NSURL URLWithString:urlString];
        }

        if (urlPlatform != nil)
        {
            result = [[NSWorkspace sharedWorkspace] openURL:urlPlatform];
        }
        else
        {
            // If we can't make a URL out of our input, try opening it as a file
            result = [[NSWorkspace sharedWorkspace] openFile:urlString];
        }
 	}

	return result;
}

// ----------------------------------------------------------------------------

static NSString*
GetUniqueName( NSString *seed, NSString *prefix )
{
	MCrypto::Algorithm algorithm = MCrypto::kMD5Algorithm;

	AppleCrypto crypto;
	U8 digest[MCrypto::kMaxDigestSize];
	size_t digestLen = crypto.GetDigestLength( algorithm );
	Rtt_ASSERT( digestLen < MCrypto::kMaxDigestSize );

	const char *dataStr = [seed UTF8String]; Rtt_ASSERT( dataStr && strlen( dataStr ) > 0 );
	
	Rtt::Data<const char> data( dataStr, (int) strlen( dataStr ) );
	
	crypto.CalculateDigest( algorithm, data, digest );

	// TODO: Add hyphens for slightly better human-readable names
	// e.g. 1BCE5E1D-712F-4F4B-B20A-9E99D9636159
	char *hex = (char*)calloc( sizeof( char ), digestLen*2 + 1 );
	for ( unsigned int i = 0; i < digestLen; i++ )
	{
		char *p = hex;
		p += sprintf( hex + 2*i, "%02x", digest[i] );
	}
	NSString *result = [[NSString stringWithExternalString:hex] uppercaseString];
	free( hex );

	if ( prefix )
	{
		result = [NSString stringWithFormat:@"%@-%@", prefix, result];
	}

	return result;
}

static NSString*
GetLibraryPath( NSString *folderName )
{
    // Note that "folderName" is "AppName-MD5" and is only appropriate for use in the Simulator or CoronaShell
	NSError *error = nil;
	NSURL *appSupportURL = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:&error];
	NSString *appSupportPath = [appSupportURL path];

#if Rtt_AUTHORING_SIMULATOR
	NSString *coronaSupportPath = [appSupportPath stringByAppendingPathComponent:@"Corona Simulator"];

	return [coronaSupportPath stringByAppendingPathComponent:folderName];
#else
	if ([[appSupportPath lastPathComponent] isEqualToString:@"Application Support"])
	{
		// We're not sandboxed, create an app appropriate path

		NSString *appName = [[[NSBundle mainBundle] infoDictionary] objectForKey:(NSString *)kCFBundleNameKey];
        
        if ([appName isEqualTo:@"CoronaShell"])
        {
            // Put app sandboxes in the same place as the Simulator if we're running as CoronaShell
            
            NSString *coronaSupportPath = [appSupportPath stringByAppendingPathComponent:@"Corona Simulator"];
            
            return [coronaSupportPath stringByAppendingPathComponent:folderName];
        }
        else
        {
            // We're a native OS X app
            return [appSupportPath stringByAppendingPathComponent:appName];
        }
	}
	else
	{
		// We're sandboxed, we can use the path we were given by macOS
		return appSupportPath;
	}
#endif  // Rtt_AUTHORING_SIMULATOR
}

MacPlatform::MacPlatform(CoronaView *view)
:	Super(),
	fView( nil ),
	fSandboxPath( nil ),
	fDevice( GetAllocator(), view ),
	fMutexCount( 0 ),
	fDelegate( [[AlertDelegate alloc] init] ),
	fWebPopup( NULL ),
	fVideoPlayer( NULL ),
	fActivityIndicator( NULL ),
#if Rtt_AUTHORING_SIMULATOR
	fFBConnect( NULL ),
#endif // Rtt_AUTHORING_SIMULATOR
	fStoreProvider( NULL ),
	fExitCallback( NULL ),
	fAssertionID(kIOPMNullAssertionID)
{
	NSString *path = [[NSBundle mainBundle] resourcePath];
	fSandboxPath = [GetLibraryPath( GetUniqueName( path, [path lastPathComponent] ) ) retain];

	pthread_mutex_init( & fMutex, NULL );
}

MacPlatform::~MacPlatform()
{
	Rtt_ASSERT( 0 == fMutexCount );

	Rtt_DELETE( fExitCallback );
	Rtt_DELETE( fStoreProvider );
#if Rtt_AUTHORING_SIMULATOR
	Rtt_DELETE( fFBConnect );
#endif // Rtt_AUTHORING_SIMULATOR
	Rtt_DELETE( fActivityIndicator );
	Rtt_DELETE( fVideoPlayer ) ;
	Rtt_DELETE( fWebPopup );
	[fDelegate release];
	pthread_mutex_destroy( & fMutex );

	[fSandboxPath release];

	// Deactivate any fonts that were activated earlier
	MacFont::ActivateFontsForApplication(fResourcePath, false);

	[fView release];

	if (fAssertionID != kIOPMNullAssertionID)
	{
		IOPMAssertionRelease(fAssertionID);
		fAssertionID = kIOPMNullAssertionID;
	}
}

void
MacPlatform::Initialize( GLView* pView )
{
	Rtt_ASSERT( ! fView );

	fView = [pView retain];

#ifndef Rtt_AUTHORING_SIMULATOR
	// For CoronaCards we supply the Mac model name for the "model"
	NSString *modelStr = nil;
	size_t len = 0;
	sysctlbyname("hw.model", NULL, &len, NULL, 0);

	if (len > 0)
	{
		char *model = (char *)malloc((len + 1)*sizeof(char));
		sysctlbyname("hw.model", model, &len, NULL, 0);
		modelStr = [NSString stringWithExternalString:model];
		free(model);
	}

	if (modelStr != nil)
	{
		GetMacDevice().SetModel( modelStr );
	}
	
	// We supply "Apple" for the manufacturer name
	GetMacDevice().SetManufacturer( @"Apple" );
	
#endif
}

static const char kDocumentsDirName[] = "Documents";
static const char kApplicationSupportDirName[] = "Application Support";
static const char kTmpDirName[] = "tmp";

void
MacPlatform::SetResourcePath( const char resourcePath[] )
{
	Rtt_ASSERT( ! GetResourceDirectory() );

	if ( resourcePath )
	{
		NSString *path = [NSString stringWithExternalString:resourcePath];
		Super::SetResourceDirectory( path );

		[fSandboxPath release];
		NSString *folderName = GetUniqueName( path, [path lastPathComponent] );
		fSandboxPath = [GetLibraryPath( folderName ) retain];

		MacFont::ActivateFontsForApplication(path);

		// We want to create folders as soon as we know the sandbox path.
		// Previously, we created them on-the-fly. We'll continue to do that in 
		// case the user deletes the folder from underneath us.
		PathForDir( kDocumentsDirName, fSandboxPath, true );
		PathForDir( kTmpDirName, fSandboxPath, true );
		PathForDir( kApplicationSupportDirName, fSandboxPath, true );
	}
}

MPlatformDevice&
MacPlatform::GetDevice() const
{
	return const_cast< MacConsoleDevice& >( fDevice );
}

PlatformSurface*
MacPlatform::CreateScreenSurface() const
{
	return Rtt_NEW( Allocator(), MacViewSurface( fView ) );
}

// TODO: Consolidate this with MacPlatform::SaveBitmap(). Lots of duplicate code.
CGImageRef
MacPlatform::CreateMacImage( Rtt_Allocator* pAllocator, PlatformBitmap& bitmap )
{
	const void* buffer = bitmap.GetBits( pAllocator );
	size_t w = bitmap.Width();
	size_t h = bitmap.Height();
	size_t bytesPerPixel = PlatformBitmap::BytesPerPixel( bitmap.GetFormat() );
//	size_t bitsPerPixel = (bytesPerPixel << 3);
	size_t bytesPerRow = w*bytesPerPixel;
	size_t numBytes = h*bytesPerRow;
//	const size_t kBitsPerComponent = 8;

	CGDataProviderRef dataProvider = CGDataProviderCreateWithData( NULL, buffer, numBytes, NULL );
	CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
	CGImageRef imageRef = CGImageCreate(
		w, h, 8, 32, w*4,
		colorspace, kCGBitmapByteOrderDefault, dataProvider,
		NULL, true, kCGRenderingIntentDefault);

	Rtt_ASSERT( w == CGImageGetWidth( imageRef ) );
	Rtt_ASSERT( h == CGImageGetHeight( imageRef ) );

	void* pixels = calloc( bytesPerRow, h );
	CGContextRef context = CGBitmapContextCreate(
		pixels, w, h, 8, bytesPerRow,
		CGImageGetColorSpace( imageRef ), kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Big );
	CGContextDrawImage( context, CGRectMake( 0.0, 0.0, w, h ), imageRef );
	CGImageRef bitmapImageRef = CGBitmapContextCreateImage(context);

	CGContextRelease( context );
	free( pixels );

	CGImageRelease( imageRef );
	CGColorSpaceRelease( colorspace );
	CGDataProviderRelease( dataProvider );

	return bitmapImageRef;
}

bool
MacPlatform::SaveImageToPhotoLibrary(const char* filePath) const
{
    NSFileManager* fileMgr = [NSFileManager defaultManager];
    NSString* filebase = @"Picture";
    bool bDone = false;
    for ( int i = 1; false == bDone && i < 1000; i++ )
    {
        NSString* relativePath = [NSString stringWithFormat:@"Desktop/%@ %d.jpg", filebase, i];
        NSString* path = [NSHomeDirectory() stringByAppendingPathComponent:relativePath];
        
        NSString* source = [NSString stringWithExternalString:filePath];
        if ( ! [fileMgr fileExistsAtPath:path] )
        {
            Rtt_TRACE_SIM( ( "NOTE: Simulator does not actually add images to the photo library. Image is being saved as: \"%s\"\n", [path UTF8String] ) );
            NSError* error = nil;
            [fileMgr copyItemAtPath:source toPath:path error:&error];
            bDone = true;
        }
    }

    return true;
}

bool
MacPlatform::SaveBitmap( PlatformBitmap* bitmap, NSURL* url ) const
{
	const void* buffer = bitmap->GetBits( & GetAllocator() );
	size_t w = bitmap->Width();
	size_t h = bitmap->Height();
	size_t bytesPerPixel = PlatformBitmap::BytesPerPixel( bitmap->GetFormat() );
//	size_t bitsPerPixel = (bytesPerPixel << 3);
	size_t bytesPerRow = w*bytesPerPixel;
	size_t numBytes = h*bytesPerRow;
//	const size_t kBitsPerComponent = 8;

	// TODO: Should this depend on bitmap->GetFormat()
	Rtt_ASSERT( bitmap->GetFormat() == PlatformBitmap::kBGRA );
	// Bug 4921: I don't understand this, but changing kCGImageAlphaPremultipliedFirst to kCGImageAlphaNoneSkipFirst fixes the white box issue.

	//Preserve previous jpeg save functionality (black background)
	//If we're saving to jpeg, set the src info and dest info to the same
    CGBitmapInfo srcBitmapInfo = kCGImageAlphaNoneSkipFirst;
    CGBitmapInfo destBitMapInfo = kCGImageAlphaNoneSkipFirst;
    
	
	//If we're saving to png with alpha support then set the src info and dest
	//info to handle alpha at the source and alpha destination
    bool enablePngAlphaSave = false;
    NSString *lowercase = [[url absoluteString] lowercaseString];
    if ( [lowercase hasSuffix:@"png"] )
    {
        enablePngAlphaSave = true;
		
		//This is the correct order to save with alpha channel, but it looks like some display objects
		//like polylines are working differently in the alpha channel in which case we don't want
		//kCGImageAlphaNoneSkipFirst may be the correct choice
		srcBitmapInfo = CGBitmapInfo(kCGBitmapByteOrderDefault | kCGImageAlphaFirst);
		destBitMapInfo = kCGImageAlphaPremultipliedLast;
    }
	
    
	CGDataProviderRef dataProvider = CGDataProviderCreateWithData( NULL, buffer, numBytes, NULL );
	CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
	CGImageRef imageRef = CGImageCreate(
                                        w, h, 8, 32, w*bytesPerPixel,
                                        colorspace, srcBitmapInfo, dataProvider,
                                        NULL, true, kCGRenderingIntentDefault);
    
	Rtt_ASSERT( w == CGImageGetWidth( imageRef ) );
	Rtt_ASSERT( h == CGImageGetHeight( imageRef ) );
    
    
    
	void* pixels = calloc( bytesPerRow, h );
	CGContextRef context = CGBitmapContextCreate(
                                                 pixels, w, h, 8, bytesPerRow,
                                                 colorspace, destBitMapInfo );
	CGContextDrawImage( context, CGRectMake( 0.0, 0.0, w, h ), imageRef );
	CGImageRef bitmapImageRef = CGBitmapContextCreateImage(context);
    
    CGImageDestinationRef imageDest = NULL;
    if (enablePngAlphaSave)
    {
        imageDest = CGImageDestinationCreateWithURL( (CFURLRef)url, kUTTypePNG, 1, NULL );
    }
    else
    {
        imageDest = CGImageDestinationCreateWithURL( (CFURLRef)url, kUTTypeJPEG, 1, NULL );
    }
    
	if ( imageDest )
	{
		CGImageDestinationAddImage( imageDest, bitmapImageRef, nil );
		CGImageDestinationFinalize( imageDest );
		CFRelease( imageDest );
	}

	CGImageRelease( bitmapImageRef );
	CGContextRelease( context );
	free( pixels );

	CGImageRelease( imageRef );
	CGColorSpaceRelease( colorspace );
	CGDataProviderRelease( dataProvider );

	return true;
}

bool
MacPlatform::SaveBitmap( PlatformBitmap* bitmap, const char* filePath, float jpegQuality ) const
{
	return SaveBitmap( bitmap, [NSURL fileURLWithPath:[NSString stringWithExternalString:filePath]] );
}

bool
MacPlatform::AddBitmapToPhotoLibrary( PlatformBitmap* bitmap ) const
{
	NSFileManager* fileMgr = [NSFileManager defaultManager];
	NSURL* url = nil;
	NSString* filebase = @"Picture";
	for ( int i = 1; nil == url; i++ )
	{
		NSString* relativePath = [NSString stringWithFormat:@"Pictures/%@ %d.jpg", filebase, i];
		NSString* path = [NSHomeDirectory() stringByAppendingPathComponent:relativePath];
		if ( ! [fileMgr fileExistsAtPath:path] )
		{
			Rtt_TRACE_SIM( ( "NOTE: screen captured to: \"~/%s\"\n", [relativePath UTF8String] ) );
			url = [NSURL fileURLWithPath:path];
		}
	}

	return SaveBitmap( bitmap, url );
}

void
MacPlatform::GetPreference( Category category, Rtt::String * value ) const
{
	// TODO: Move this into MacGUIPlatform b/c it's simulator-specific

#ifdef Rtt_AUTHORING_SIMULATOR
    const char *result = NULL;
	MacSimulator *simulator = ((AppDelegate*)[NSApp delegate]).simulator;
	NSDictionary *properties = ( simulator ? simulator->GetProperties() : nil );
	switch( category )
	{
		case MPlatform::kDefaultStatusBarFile:
			result = [[properties valueForKey:@"statusBarDefaultFile"] UTF8String];
			break;
		case MPlatform::kDarkStatusBarFile:
			result = [[properties valueForKey:@"statusBarBlackFile"] UTF8String];
			break;
		case MPlatform::kTranslucentStatusBarFile:
			result = [[properties valueForKey:@"statusBarTranslucentFile"] UTF8String];
			break;
		case MPlatform::kLightTransparentStatusBarFile:
			result = [[properties valueForKey:@"statusBarLightTransparentFile"] UTF8String];
			break;
		case MPlatform::kDarkTransparentStatusBarFile:
			result = [[properties valueForKey:@"statusBarDarkTransparentFile"] UTF8String];
			break;
		case MPlatform::kScreenDressingFile:
			result = [[properties valueForKey:@"screenDressingFile"] UTF8String];
			break;
		case MPlatform::kSubscription:
			result = [[properties valueForKey:@"subscription"] UTF8String];
			break;
		default:
			Super::GetPreference( category, value );
			return;
	}
    
    value->Set( result );
#else
    Super::GetPreference( category, value );
#endif // Rtt_AUTHORING_SIMULATOR
}

#ifdef Rtt_AUTHORING_SIMULATOR
ValueResult<Rtt::SharedConstStdStringPtr>
MacPlatform::GetSimulatedAppPreferenceKeyFor(const char* keyName) const
{
	Rtt::SharedConstStdStringPtr sharedStringPointer;
	
	// Generate a unique preference name using a hash of the simulated project's directory and the given key name.
	// We do this because the simulated app's preference will be stored into the Corona Simulator's plist file.
	// Format:  "appPreferences/<ProjectDirectoryHash>/<keyName>"
	AppDelegate* appDelegatePointer = (AppDelegate*)[[NSApplication sharedApplication] delegate];
	NSString* resourceDirectoryPath = PathForResourceFile(NULL);
	NSString* rootKeyName =
			[appDelegatePointer getAppSpecificPreferenceKeyName:@"appPreferences" withProjectPath: resourceDirectoryPath];
	if (rootKeyName)
	{
		std::stringstream stringStream;
		stringStream << [rootKeyName UTF8String];
		stringStream << '/';
		if (keyName)
		{
			stringStream << keyName;
		}
		sharedStringPointer = Rtt_MakeSharedConstStdStringPtr(stringStream.str());
	}
	if (sharedStringPointer.IsNull() || sharedStringPointer->empty())
	{
		const char kMessage[] = "Failed to generate sandboxed preference key name for simulated app.";
		return ValueResult<Rtt::SharedConstStdStringPtr>::FailedWith(kMessage);
	}
	return ValueResult<Rtt::SharedConstStdStringPtr>::SucceededWith(sharedStringPointer);
}
#endif
	
Preference::ReadValueResult
MacPlatform::GetPreference( const char* categoryName, const char* keyName ) const
{
#ifdef Rtt_AUTHORING_SIMULATOR
	if (Rtt_StringCompare(categoryName, Preference::kCategoryNameApp) == 0)
	{
		// We're accessing a simulated app's preference.
		// Generate a unique sandboxed preference key to be accessed from the Corona Simulator's plist file.
		ValueResult<Rtt::SharedConstStdStringPtr> simulatedKeyResult = GetSimulatedAppPreferenceKeyFor(keyName);
		if (simulatedKeyResult.HasFailed())
		{
			return Preference::ReadValueResult::FailedWith(simulatedKeyResult.GetUtf8MessageAsSharedPointer());
		}
		return Super::GetPreference(Preference::kCategoryNameApp, simulatedKeyResult.GetValue()->c_str());
	}
	else if (Rtt_StringCompare(categoryName, Preference::kCategoryNameSimulator) == 0)
	{
		// We're accessing a Corona Simulator preference.
		// Since this is a Corona Simulator app, we can access the preference directly.
		return Super::GetPreference(Preference::kCategoryNameApp, keyName);
	}
#endif
	return Super::GetPreference(categoryName, keyName);
}
	
OperationResult
MacPlatform::SetPreferences( const char* categoryName, const PreferenceCollection& collection ) const
{
#ifdef Rtt_AUTHORING_SIMULATOR
	if (Rtt_StringCompare(categoryName, Preference::kCategoryNameApp) == 0)
	{
		// We're accessing a simulated app's preferences.
		// Make a copy of the given collection, generating unique sandboxed keys under the simulator's plist file.
		PreferenceCollection simulatedKeyCollection;
		for (int index = 0; index < collection.GetCount(); index++)
		{
			Preference* preferencePointer = collection.GetByIndex(index);
			if (!preferencePointer)
			{
				continue;
			}
			const char* keyName = preferencePointer->GetKeyName();
			ValueResult<Rtt::SharedConstStdStringPtr> simulatedKeyResult = GetSimulatedAppPreferenceKeyFor(keyName);
			if (simulatedKeyResult.HasFailed())
			{
				return OperationResult::FailedWith(simulatedKeyResult.GetUtf8MessageAsSharedPointer());
			}
			simulatedKeyCollection.Add(simulatedKeyResult.GetValue(), preferencePointer->GetValue());
		}
		return Super::SetPreferences(Preference::kCategoryNameApp, simulatedKeyCollection);
	}
	else if (Rtt_StringCompare(categoryName, Preference::kCategoryNameSimulator) == 0)
	{
		// We're accessing the Corona Simulator's preferences.
		// Since this is a Corona Simulator app, we can access these preferences directly.
		return Super::SetPreferences(Preference::kCategoryNameApp, collection);
	}
#endif
	return Super::SetPreferences(categoryName, collection);
}

OperationResult
MacPlatform::DeletePreferences( const char* categoryName, const char** keyNameArray, U32 keyNameCount ) const
{
	// Validate.
	if (!keyNameArray || (keyNameCount < 1))
	{
		return OperationResult::FailedWith("Preference key name array cannot be null or empty.");
	}
	
#ifdef Rtt_AUTHORING_SIMULATOR
	if (Rtt_StringCompare(categoryName, Preference::kCategoryNameApp) == 0)
	{
		// We're deleting a simulated app's preferences.
		// Generate a new array of unique sandboxed preference key names to be deleted from the simulator's plist file.
		// Note: The "sharedSimulatedKeyNamePointers" collection stores the generated strings.
		//       The "simulatedKeyNamePointers" is passed as a string array to the Super::DeletePreferences() method
		//       and provides the raw string pointers contained by the "sharedSimulatedKeyNamePointers" collection.
		std::vector<Rtt::SharedConstStdStringPtr> sharedSimulatedKeyNamePointers;
		std::vector<const char*> simulatedKeyNamePointers;
		for (U32 index = 0; index < keyNameCount; index++)
		{
			ValueResult<Rtt::SharedConstStdStringPtr> simulatedKeyResult =
					GetSimulatedAppPreferenceKeyFor(keyNameArray[index]);
			if (simulatedKeyResult.HasFailed())
			{
				return OperationResult::FailedWith(simulatedKeyResult.GetUtf8MessageAsSharedPointer());
			}
			sharedSimulatedKeyNamePointers.push_back(simulatedKeyResult.GetValue());
			simulatedKeyNamePointers.push_back(simulatedKeyResult.GetValue()->c_str());
		}
		return Super::DeletePreferences(
				Preference::kCategoryNameApp, &simulatedKeyNamePointers.front(), (U32)simulatedKeyNamePointers.size());
	}
	else if (Rtt_StringCompare(categoryName, Preference::kCategoryNameSimulator) == 0)
	{
		// We're deleting the Corona Simulator's preferences.
		// Since this is a Corona Simulator app, we can delete these preferences directly.
		return Super::DeletePreferences(Preference::kCategoryNameApp, keyNameArray, keyNameCount);
	}
#endif
	return Super::DeletePreferences(categoryName, keyNameArray, keyNameCount);
}
	
bool
MacPlatform::OpenURL( const char* url ) const
{
	return MacOpenURL( url );
}
	
int
MacPlatform::CanOpenURL( const char *url ) const
{
	int result = -1;
	if ( url && LSCopyDefaultApplicationURLForURL != NULL )
	{
		NSString *urlString = [NSString stringWithExternalString:url];
		NSURL *urlPlatform = [NSURL URLWithString:urlString];
		if (urlPlatform != nil)
		{
			CFURLRef appUrl = LSCopyDefaultApplicationURLForURL((CFURLRef)urlPlatform, kLSRolesViewer, NULL);
			if (appUrl != nil)
			{
				result = 1;
				CFRelease(appUrl);
			}
			else
			{
				result = 0;
			}
		}
	}
	return result;
}
    
PlatformVideoPlayer *
MacPlatform::GetVideoPlayer( const ResourceHandle<lua_State> & handle ) const
{
	if ( ! fVideoPlayer )
	{
		fVideoPlayer = Rtt_NEW( Allocator(), MacVideoPlayer( handle ) );
	}
	return fVideoPlayer;
}

PlatformImageProvider *
MacPlatform::GetImageProvider( const ResourceHandle<lua_State> & handle ) const
{
	return Rtt_NEW( Allocator(), MacImageProvider( handle ) );
}

PlatformVideoProvider *
MacPlatform::GetVideoProvider( const ResourceHandle<lua_State> & handle ) const
{
	return Rtt_NEW( Allocator(), MacVideoProvider( handle ) );
}

PlatformStoreProvider*
MacPlatform::GetStoreProvider( const ResourceHandle<lua_State>& handle ) const
{
#ifdef Rtt_AUTHORING_SIMULATOR
	// Because the simulator doesn't run under the user's bundle identifier, I don't think it is possible to comminicate with the Store.
	// Don't bother to create the store provider and let this function return NULL.
	return NULL;
#else
	if (!fStoreProvider)
	{
		fStoreProvider = Rtt_NEW( fAllocator, AppleStoreProvider( handle ) );
	}
	return fStoreProvider;
#endif // Rtt_AUTHORING_SIMULATOR
}

void
MacPlatform::SetStatusBarMode( StatusBarMode newValue ) const
{
#ifdef Rtt_AUTHORING_SIMULATOR
	MacSimulator *simulator = ((AppDelegate*)[NSApp delegate]).simulator;
	if(simulator)
	{
		simulator->SetStatusBarMode(newValue);
	}
#endif
}

MPlatform::StatusBarMode
MacPlatform::GetStatusBarMode() const
{
	// MacApp now hits this code
//	Rtt_ASSERT_NOT_REACHED();
	return MPlatform::kHiddenStatusBar;
}

void
MacPlatform::SetIdleTimer( bool enabled ) const
{
	if (enabled)
	{
		// Release any previously made assertion
		if (fAssertionID != kIOPMNullAssertionID)
		{
			IOPMAssertionRelease(fAssertionID);
			fAssertionID = kIOPMNullAssertionID;
		}
	}
	else
	{
#if Rtt_AUTHORING_SIMULATOR
		Rtt_TRACE_SIM( ( "WARNING: disabling the idle timer can reduce battery life on the device\n" ) );
#endif

		//  NOTE: IOPMAssertionCreateWithName limits the string to 128 characters.
		CFStringRef reasonForActivity= CFSTR("CoronaSDK app running");
		IOPMAssertionID assertionID;

		IOReturn result = IOPMAssertionCreateWithName(kIOPMAssertPreventUserIdleDisplaySleep,
									kIOPMAssertionLevelOn, reasonForActivity, &assertionID);

		if (result == kIOReturnSuccess)
		{
			// Release any previously made assertion
			if (fAssertionID != kIOPMNullAssertionID)
			{
				IOPMAssertionRelease(fAssertionID);
			}

			fAssertionID = assertionID;
		}
		else
		{
#if Rtt_AUTHORING_SIMULATOR
			Rtt_TRACE_SIM( ( "ERROR: unable to disable idle timer\n" ) );
#endif
		}
	}
}

bool
MacPlatform::GetIdleTimer() const
{
	return true;
}

NativeAlertRef
MacPlatform::ShowNativeAlert(
	const char *title,
	const char *msg,
	const char **buttonLabels,
	U32 numButtons,
	LuaResource* resource ) const
{
	CustomAlert *alert = nil;
	int numAlerts = (int) [fDelegate.wrappers count];
	Rtt_WARN_SIM( numAlerts <= 0, ( "WARNING: The simulator does not support simultaneous alerts via native.showAlert(). If an alert is shown, subsequent calls will be ignored.\n" ) );

	if ( 0 == numAlerts )
	{
		NSString *t = [[NSString alloc] initWithUTF8String:title];
		NSString *m = [[NSString alloc] initWithUTF8String:msg];

		alert = [[CustomAlert alloc] init];
		[alert setMessageText:t];
		[alert setInformativeText:m];

		for ( U32 i = 0; i < numButtons; i++ )
		{
			const char *label = buttonLabels[i]; Rtt_ASSERT( label );
			NSString *l = [[NSString alloc] initWithUTF8String:label];
			[alert addButtonWithTitle:l];
			[l release];
		}

		alert.fResource = resource;
		[fDelegate setObject:alert forKey:alert];

		[alert beginSheetModalForWindow:[fView window]
						  modalDelegate:fDelegate
						 didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
							contextInfo:nil];
		[alert release];

		[m release];
		[t release];
	}

	return alert;
}

void
MacPlatform::CancelNativeAlert( NativeAlertRef alert, S32 index ) const
{
	CustomAlert *alertView = [fDelegate objectForKey:alert];
	if ( alertView )
	{
		NSInteger returnCode = NSAlertFirstButtonReturn + index;
		[fDelegate alertDidEnd:alertView returnCode:returnCode cancelled:YES];
		[alertView endSheetAndClose];
	}
}

void
MacPlatform::SetActivityIndicator( bool visible ) const
{
	if ( ! fActivityIndicator )
	{
		fActivityIndicator = Rtt_NEW( & GetAllocator(), MacActivityIndicator );
	}

	if ( visible )
	{
		fActivityIndicator->ShowActivityIndicator();
	}
	else
	{
		fActivityIndicator->HideActivityIndicator();
	}
}

void
MacPlatform::Suspend() const
{
	// suspend MacWebPopup *fWebPopup;
	// suspend  MacVideoPlayer *fVideoPlayer;
	if (fVideoPlayer != NULL)
	{
		fVideoPlayer->SetSuspended(true);
	}

	// suspend  MacActivityIndicator* fActivityIndicator;
}

void
MacPlatform::Resume() const
{
	// resume MacWebPopup *fWebPopup;

	// resume  MacVideoPlayer *fVideoPlayer;
	if (fVideoPlayer != NULL)
	{
		fVideoPlayer->SetSuspended(false);
	}

	// resume  MacActivityIndicator* fActivityIndicator;
}

PlatformWebPopup*
MacPlatform::GetWebPopup() const
{
	if ( ! fWebPopup )
	{
		fWebPopup = Rtt_NEW( & GetAllocator(), MacWebPopup );
	}

	return fWebPopup;
}

PlatformDisplayObject*
MacPlatform::CreateNativeTextBox( const Rect& bounds ) const
{
	return Rtt_NEW( & GetAllocator(), MacTextBoxObject( bounds ) );
}

PlatformDisplayObject*
MacPlatform::CreateNativeTextField( const Rect& bounds ) const
{
	return Rtt_NEW( & GetAllocator(), MacTextFieldObject( bounds ) );
}

void
MacPlatform::SetKeyboardFocus( PlatformDisplayObject *object ) const
{
	if ( object )
	{
		// Verify that this is actually a text field or text box
		const LuaProxyVTable *vtable = & object->ProxyVTable();
		
		if ( & PlatformDisplayObject::GetTextFieldObjectProxyVTable() == vtable
			|| & PlatformDisplayObject::GetTextBoxObjectProxyVTable() == vtable )
		{
			((MacDisplayObject*)object)->SetFocus();
		}
	}
	else
	{
		// There is no object specified. What do we do? There isn't actually a keyboard on the screen, possibly just a focus ring around some object.
		// set the first responder back to the GLView?
		[[fView window] performSelector:@selector(makeFirstResponder:) withObject:fView afterDelay:0.0];
	}
}

PlatformDisplayObject *
MacPlatform::CreateNativeMapView( const Rect& bounds ) const
{
#if Rtt_AUTHORING_SIMULATOR
	Rtt_TRACE_SIM( ( "WARNING: Map views are not supported in the simulator. Please build for device.\n" ) );
#endif // Rtt_AUTHORING_SIMULATOR

	return Rtt_NEW( & GetAllocator(), MacMapViewObject( bounds ) );
}

PlatformDisplayObject *
MacPlatform::CreateNativeWebView( const Rect& bounds ) const
{
	return Rtt_NEW( & GetAllocator(), MacWebViewObject( bounds ) );
}

PlatformDisplayObject*
MacPlatform::CreateNativeVideo( const Rect& bounds ) const
{
	return Rtt_NEW( & GetAllocator(), MacVideoObject( bounds ) );
}

S32
MacPlatform::GetFontNames( lua_State *L, int index ) const
{
	S32 numFonts = 0;

	NSArray *names = [[NSFontManager sharedFontManager] availableFonts];
	for ( NSString *fontName in names )
	{
		lua_pushstring( L, [fontName UTF8String] );
		lua_rawseti( L, index, ++numFonts );
	}

	return numFonts;
}


void
MacPlatform::SetTapDelay( Rtt_Real delay ) const
{
#ifdef Rtt_AUTHORING_SIMULATOR
	AppDelegate *delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
	delegate.simulator->GetScreenView().tapDelay = delay;
#else
	Rtt_ASSERT_NOT_IMPLEMENTED();
#endif
}

Rtt_Real
MacPlatform::GetTapDelay() const
{
#ifdef Rtt_AUTHORING_SIMULATOR
	AppDelegate *delegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
	return delegate.simulator->GetScreenView().tapDelay;
#else
	Rtt_ASSERT_NOT_IMPLEMENTED();
	return Rtt_REAL_0;
#endif
}

int
MacPlatform::PushSystemInfo( lua_State *L, const char *key ) const
{
	// Validate.
	if (!L)
	{
		return 0;
	}

	// Push the requested system information to Lua.
	int pushedValues = 0;
	if (Rtt_StringCompare(key, "appName") == 0)
	{
		// Fetch the application's name.
		const char *applicationName = "";
#if Rtt_AUTHORING_SIMULATOR
		NSString *value = [GetResourceDirectory() lastPathComponent];
#else
		NSString *value = (NSString*)[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];
#endif
		if (value)
		{
			applicationName = [value UTF8String];
		}
		lua_pushstring(L, applicationName);
		pushedValues = 1;
	}
	else if (Rtt_StringCompare(key, "appVersionString") == 0)
	{
		// Return an empty version string since it is unknown by the simulator.
		lua_pushstring(L, "");
		pushedValues = 1;
	}
	else if ( Rtt_StringCompare( key, "darkMode" ) == 0 )
	{
		BOOL res = NO;
		if (@available(macOS 10.14, *)) {
			res = [fView.effectiveAppearance bestMatchFromAppearancesWithNames:@[NSAppearanceNameAqua, NSAppearanceNameDarkAqua]] == NSAppearanceNameDarkAqua;
		}
		lua_pushboolean(L, res);
		pushedValues = 1;
	}
    else if ( Rtt_StringCompare( key, "reduceMotion" ) == 0 )
    {
        BOOL res = [[NSWorkspace sharedWorkspace] accessibilityDisplayShouldReduceMotion];
        lua_pushboolean(L, res);
        pushedValues = 1;
    }
    else if ( Rtt_StringCompare( key, "reduceTransparency" ) == 0 )
    {
        BOOL res = [[NSWorkspace sharedWorkspace] accessibilityDisplayShouldReduceTransparency];
        lua_pushboolean(L, res);
        pushedValues = 1;
    }
    else if ( Rtt_StringCompare( key, "differentiateWithoutColor" ) == 0 )
    {
        BOOL res = [[NSWorkspace sharedWorkspace] accessibilityDisplayShouldDifferentiateWithoutColor];
        lua_pushboolean(L, res);
        pushedValues = 1;
    }
	else
	{
		// Attempt to fetch the requested system info from the base class.
		pushedValues = Super::PushSystemInfo(L, key);
	}

	// Return the number of values pushed into Lua.
	return pushedValues;
}

// ----------------------------------------------------------------------------

PlatformFBConnect*
MacPlatform::GetFBConnect() const
{
#if Rtt_AUTHORING_SIMULATOR
	if ( ! fFBConnect )
	{
		fFBConnect = Rtt_NEW( Allocator(), MacFBConnect );
	}

	Rtt_TRACE_SIM( ( "WARNING: Facebook Connect is not supported in the simulator. Please build for device.\n" ) );

	return fFBConnect;
#else
    return NULL;
#endif // Rtt_AUTHORING_SIMULATOR
}

BOOL
MacPlatform::IsFilenameCaseCorrect(const char *filename, NSString *path) const
{
#ifdef Rtt_AUTHORING_SIMULATOR
	// Enforce case sensitivity of filenames
	BOOL isDir = NO;
	if ([path length] > 0 &&
		[[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDir] && !isDir )
	{
		// Check for case-sensitive completions that match path. If none
		// found that match, then pretend the file at "path" does not exist.
		NSString *dummy = nil;
		NSArray *matches = nil;
		NSUInteger numMatches = [path completePathIntoString:&dummy
											   caseSensitive:YES
											matchesIntoArray:& matches
												 filterTypes:nil];

		// [NSString completePathIntoString:] returns paths which are encoding differently to the paths it receives
		// so we force decomposition to make comparison possible
		if ( 0 == numMatches || ! [matches containsObject:[path decomposedStringWithCanonicalMapping]] )
		{
			Runtime *runtime = [fView runtime];
			lua_State *L = NULL;
			if (runtime != NULL)
			{
				L = runtime->VMContext().L();
			}

			// Unicode filenames survive the embedding in the message better if we do it using NSString rather than printf
			NSString *msg = [NSString stringWithFormat:@"the file '%@' could not be found at case-sensitive path '%@'",
							 [[NSFileManager defaultManager] stringWithFileSystemRepresentation:filename length:strlen(filename)], path];

			// Report existing but different case path:
			numMatches = [path completePathIntoString:&dummy
							   caseSensitive:NO
									 matchesIntoArray:& matches
										  filterTypes:nil];
			if (numMatches > 0)
			{
				msg = [msg stringByAppendingString:[NSString stringWithFormat:@"\n         found filename with different case '%s'", [[matches objectAtIndex:0] UTF8String]]];
			}

			CoronaLuaWarning(L, "%s", [msg UTF8String] );

			return YES;  // if we want to fail the request we should return NO here (YES matches Windows behavior)
		}
	}
#endif

	return YES;
}

NSString*
MacPlatform::PathForResourceFile( const char* filename ) const
{
	NSString* result = Super::PathForResourceFile( filename );

	if (! IsFilenameCaseCorrect(filename, result))
	{
		return nil;
	}
	else
	{
		return result;
	}
}
    
NSString*
MacPlatform::PathForProjectResourceFile( const char* filename ) const
{
    return Super::PathForProjectResourceFile(filename);
}

void
MacPlatform::SetProjectResourceDirectory( const char* filename )
{
    Super::SetProjectResourceDirectory(filename);
}

NSString*
MacPlatform::PathForSkinResourceFile( const char* filename ) const
{
    return Super::PathForProjectResourceFile(filename);
}

void
MacPlatform::SetSkinResourceDirectory( const char* filename )
{
    Super::SetSkinResourceDirectory(filename);
}

NSString*
MacPlatform::PathForDocumentsFile( const char* filename ) const
{
	NSString *result = nil;

	if ( fSandboxPath )
	{
		NSString *baseDir = PathForDir( kDocumentsDirName, fSandboxPath, true );
		result = Super::PathForFile( filename, baseDir );
	}
	else
	{
		result = Super::PathForDocumentsFile( filename );
	}

	if (! IsFilenameCaseCorrect(filename, result))
	{
		return nil;
	}
	else
	{
		return result;
	}
}

NSString*
MacPlatform::PathForApplicationSupportFile( const char* filename ) const
{
	NSString *result = nil;

	if ( fSandboxPath )
	{
		NSString *baseDir = PathForDir( kApplicationSupportDirName, fSandboxPath, true );
		result = Super::PathForFile( filename, baseDir );
	}
	else
	{
		result = Super::PathForApplicationSupportFile( filename );
	}

	if (! IsFilenameCaseCorrect(filename, result))
	{
		return nil;
	}
	else
	{
		return result;
	}
}

NSString*
MacPlatform::CachesParentDir() const
{
	NSString *result = ( fSandboxPath ? fSandboxPath : Super::CachesParentDir() );
	return result;
}

NSString*
MacPlatform::PathForFile( const char* filename, NSString* baseDir ) const
{
	NSString* result;

	if ( filename )
	{
		result = [baseDir stringByAppendingPathComponent:[NSString stringWithExternalString:filename]];
	}
	else
	{
		result = baseDir;
	}

	if (! IsFilenameCaseCorrect(filename, result))
	{
		return nil;
	}
	else
	{
		return result;
	}
}

NSString*
MacPlatform::PathForTmpFile( const char* filename ) const
{
	NSString *result = nil;

	if ( fSandboxPath )
	{
		NSString *baseDir = PathForDir( kTmpDirName, fSandboxPath, true );
		result = Super::PathForFile( filename, baseDir );
	}
	else
	{
		result = Super::PathForTmpFile( filename );
	}

	if (! IsFilenameCaseCorrect(filename, result))
	{
		return nil;
	}
	else
	{
		return result;
	}
}

static NSString* kPluginsDirKey = @"pluginsDirectory";

NSString *
MacPlatform::PathForPluginsFile( const char *filename ) const
{
	// Look for custom plugins dir location
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	NSString *baseDir = [defaults stringForKey:kPluginsDirKey];
	
	if ( ! baseDir )
	{
		// Default is user's Application Support dir (i.e. '~/Library/Application Support')
		NSArray *paths = NSSearchPathForDirectoriesInDomains(
							NSApplicationSupportDirectory, NSUserDomainMask, YES );

		baseDir = [paths objectAtIndex:0];
		if ( baseDir )
		{
			// TODO: Figure out to group by daily build
			// NSString *dailyBuildNumber = [NSString stringWithExternalString:Rtt_STRING_BUILD];

			// TODO: Or use a TTL scheme defined by server, i.e. for a given daily build/plugin,
			// the server will tell you the mod date. If the local plugin predates it, then we
			// grab the new plugin.

			// '~/Library/Application Support/Corona/Simulator/Plugins/'
			NSString *appName = [[[NSBundle mainBundle] infoDictionary] objectForKey:(NSString *)kCFBundleNameKey];

			if ([appName isEqualTo:@"Corona Simulator"] || [appName isEqualTo:@"CoronaShell"])
			{
				// Corona Simulator and CoronaShell find plugins in the same place
				baseDir = [baseDir stringByAppendingPathComponent:@"Corona"];
				baseDir = [baseDir stringByAppendingPathComponent:@"Simulator"];
				baseDir = PathForDir( "Plugins", baseDir, true );
			}
			else
			{
				// CoronaSDK for OS X apps find their plugins in the "Plugins" subdirectory of the app's bundle directory which
                // a location approved by Apple for executable files
				// (they should *not* look in the same place as the Simulator because that would make it too easy to create apps that
				// will only run on the developer's computer and no where else)
                baseDir = [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent:@"Contents/Plugins"];
 			}
		}
	}

	NSString *result = baseDir;

	if ( filename && baseDir )
	{
		result = Super::PathForFile( filename, baseDir );
	}

	return result;
}

void
MacPlatform::BeginRuntime( const Runtime& runtime ) const
{
	// TODO: This is not thread-safe.
	// It is only re-entrant safe.
	if ( 0 == fMutexCount++ )
	{
		pthread_mutex_lock( & fMutex );

		// calls setNeedDisplay
		[fView update];

		// When async, we do not guarantee the context is current
		// unless we're in the Runtime::Render() call.
		if ( runtime.IsProperty( Runtime::kRenderAsync ) )
		{
			[[fView openGLContext] makeCurrentContext];
		}
	}
}

void
MacPlatform::EndRuntime( const Runtime& runtime ) const
{
	if ( 0 == --fMutexCount )
	{
		//[fView unbindRenderFBO];
		//[fView unlockOpenGLContext];
		
		pthread_mutex_unlock( & fMutex );
	}
}

PlatformExitCallback*
MacPlatform::GetExitCallback()
{
	// Assumption: allowLuaExit can't be redefined after the application starts.
	// This saves the trouble of needing to figure out when and where to set the callback.
	if ( NULL == fExitCallback )
	{
		fExitCallback =  Rtt_NEW( & fPlatform.GetAllocator(), MacExitCallback );
	}
	return fExitCallback;
}

bool
MacPlatform::RequestSystem( lua_State *L, const char *actionName, int optionsIndex ) const
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
	else if ( Rtt_StringCompare( actionName, "exitApplication" ) == 0 )
	{
		// Exit/close the app gracefully.
#ifndef Rtt_AUTHORING_SIMULATOR
		CoronaLog("native.requestExit() called - application will close");

		[[NSApplication sharedApplication] performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
		return true;
#endif
	}

	// Return false to indicate that the requested action is not support or unknown.
	return false;
}

void
MacPlatform::RuntimeErrorNotification( const char *errorType, const char *message, const char *stacktrace ) const
{
	@autoreleasepool
	{
		if (message != NULL)
		{
			AppDelegate *delegate = (AppDelegate*)[NSApp delegate];
			NSString *nsMsg = [NSString stringWithExternalString:message];
			NSString *appPath = @"";

			if ([delegate respondsToSelector:@selector(notifyRuntimeError:)])
			{
				[delegate notifyRuntimeError:nsMsg];
			}

			if ([delegate respondsToSelector:@selector(toggleSuspendResume:)])
			{
				[delegate toggleSuspendResume:nil];
			}

			if ([delegate respondsToSelector:@selector(fAppPath)])
			{
				appPath = (delegate.fAppPath == nil) ? @"" : [delegate.fAppPath stringByAppendingString:@"/"];;
			}

			char filename[BUFSIZ];
			char errorMsg[BUFSIZ];
			int linenum = 0;
			NSAlert *alert = [[NSAlert alloc] init];

			if (stacktrace != NULL  && strlen(stacktrace) > 0)
			{
				NSTextView *accessory = [[NSTextView alloc] initWithFrame:NSMakeRect(0,0,600,15)];
				// NSFont *font = [NSFont systemFontOfSize:[NSFont systemFontSize]];
				NSFont *font = [NSFont userFixedPitchFontOfSize:6];
				NSDictionary *textAttributes = [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName];
				NSString *stacktraceStr = [NSString stringWithExternalString:stacktrace];
				stacktraceStr = [stacktraceStr stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];  // trim any leading or trailing newlines
				// elide the full path of the project in the stacktrace
				[accessory insertText:[[NSAttributedString alloc] initWithString:[stacktraceStr stringByReplacingOccurrencesOfString:appPath withString:@""]
																	  attributes:textAttributes]];
				[accessory setEditable:NO];
				[accessory setDrawsBackground:NO];

				[alert setAccessoryView:accessory];
			}

			NSString *msgText = nil;

			// Parse error message to get filename and linenumber
			int count = sscanf(message, "%[^:]:%d: %[^\n]", filename, &linenum, errorMsg);
			// NSLog(@"file: %s\nlinenum: %d\nerrorMsg: %s", filename, linenum, errorMsg);

			if (count != 3)
			{
				// Rtt_TRACE_SIM( ( "RuntimeErrorNotification: Failed to parse error message: %s", message));

				strncpy(errorMsg, message, BUFSIZ);
				errorMsg[0] = toupper(errorMsg[0]);
				msgText = [NSString stringWithExternalString:errorMsg];
			}
			else
			{
				errorMsg[0] = toupper(errorMsg[0]);
				msgText = [NSString stringWithExternalString:errorMsg];
#ifndef Rtt_AUTHORING_SIMULATOR
				// This is needed for CoronaCards
				msgText = [msgText stringByAppendingString:[NSString stringWithFormat:@"\n\n(%s:%d)", filename, linenum]];
#endif
			}

			NSInteger firstButton = -1;
			NSInteger secondButton = NSAlertFirstButtonReturn;
			NSInteger thirdButton = NSAlertSecondButtonReturn;

			if (strcasecmp(errorType, "Syntax error") != 0)
			{
				// Only show a continue button is this isn't a "Syntax error"
				[alert addButtonWithTitle:@"Continue"];

				firstButton = NSAlertFirstButtonReturn;
				secondButton = NSAlertSecondButtonReturn;
				thirdButton = NSAlertThirdButtonReturn;
			}

			if ([delegate respondsToSelector:@selector(launchSimulator:)])
			{
				[alert addButtonWithTitle:@"Relaunch"];
			}
			else if ([delegate respondsToSelector:@selector(terminate:)])
			{
				[alert addButtonWithTitle:@"Quit"];
			}

#ifdef Rtt_AUTHORING_SIMULATOR
			const char *platformName = "Corona Simulator";
#else
			const char *platformName = "CoronaSDK";
#endif

			[alert setMessageText:[NSString stringWithFormat:@"%s %s", platformName, errorType]];

#ifdef Rtt_AUTHORING_SIMULATOR
			if (strlen(filename) > 0)
			{
				msgText = [msgText stringByAppendingFormat:@"\n\nFile: %@\n", [[NSString stringWithExternalString:filename] stringByReplacingOccurrencesOfString:appPath withString:@""]];

				if (linenum > 0)
				{
					msgText = [msgText stringByAppendingFormat:@"Line: %d\n", linenum];

					[alert addButtonWithTitle:@"Edit Code"];
				}
			}
#endif // Rtt_AUTHORING_SIMULATOR

			[alert setInformativeText:msgText];
			// This is the most reliable style
			[alert setAlertStyle:NSCriticalAlertStyle];

#if Rtt_AUTHORING_SIMULATOR
			// We have to access "filename" outside the block
			NSString *filenameStr = [NSString stringWithExternalString:filename];
#endif // Rtt_AUTHORING_SIMULATOR
			NSString *errorTypeStr = [NSString stringWithExternalString:errorType];

			// Common code to handle button presses in either a sheet or a modal alert
			void (^responseHandler) (NSModalResponse) = ^ void (NSModalResponse result)
			{
				if (result == firstButton)
				{
					if ([errorTypeStr caseInsensitiveCompare:@"Syntax error"] == NSOrderedSame &&
						[delegate respondsToSelector:@selector(terminate:)])
					{
						[delegate performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
					}
				}
				else if (result == secondButton)
				{
					if ([delegate respondsToSelector:@selector(launchSimulator:)])
					{
						[delegate performSelector:@selector(launchSimulator:) withObject:nil afterDelay:0.0];
					}
					else if ([delegate respondsToSelector:@selector(terminate:)])
					{
						[delegate performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
					}
				}
#if Rtt_AUTHORING_SIMULATOR
				else if (result == thirdButton)
				{
					TextEditorSupport_LaunchTextEditorWithFile(filenameStr, linenum);
				}
#endif // Rtt_AUTHORING_SIMULATOR

				[NSApp stopModalWithCode:result];

				[alert release];
			};

			NSWindow *simWindow = [delegate currentWindow];

			if (simWindow == nil)
			{
				// check for RuntimeErrorNotification not called from Runtime destructor
				if ([fView runtime])
				{
					// This typically happens for errors in build.settings
					NSModalResponse response = [alert runModal];
				
					responseHandler(response);
				}
			}
			else
			{
				[alert beginSheetModalForWindow:simWindow completionHandler:responseHandler];
			}
		}
	}
}

#ifdef Rtt_AUTHORING_SIMULATOR
void
MacPlatform::SetCursorForRect(const char *cursorName, int x, int y, int width, int height) const
{
    NSRect r = NSMakeRect( x, y, width, height );

    [fView setCursor:cursorName forRect:r];
}
#endif // Rtt_AUTHORING_SIMULATOR

void
MacPlatform::SetNativeProperty(lua_State *L, const char *key, int valueIndex) const
{
	// Rtt_TRACE_SIM(("MacPlatform::SetNativeProperty: key %s, valueIndex %d\n", key, valueIndex ));

    // Validate.
    if (!L || !key)
    {
        return;
    }

	NSWindow *window = [fView window];

    // Set the requested native property.
    if (Rtt_StringCompare(key, "windowTitleText") == 0)
    {
		// Update the main window's title bar text
		if (window != nil && (lua_type(L, valueIndex) == LUA_TSTRING))
        {
            [window setTitle:[NSString stringWithExternalString:lua_tostring(L, valueIndex)]];
        }
    }
    else if (Rtt_StringCompare(key, "windowSize") == 0)
    {
#ifdef Rtt_AUTHORING_SIMULATOR

		Rtt_Log("Note: native.setProperty(\"windowSize\", mode) does not work in the Simulator");
#else
		if (window != nil && (lua_type(L, valueIndex) == LUA_TTABLE))
		{
			NSRect frame = [window frame];
			NSSize size = frame.size;
			
			lua_getfield( L, -1, "width");
			if (lua_type( L, -1 ) == LUA_TNUMBER) 
			{
			    size.width = lua_tointeger( L, -1 );
			}
			lua_pop( L, 1 );
			
			lua_getfield( L, -1, "height");
			if (lua_type( L, -1 ) == LUA_TNUMBER) 
			{
			    size.height = lua_tointeger( L, -1 );
			}
			lua_pop( L, 1 );
			
			[fView setFrameSize:NSMakeSize(size.width, size.height)];
			[window setContentSize:(NSSize)size];
		}
#endif
	}
    else if (Rtt_StringCompare(key, "windowMode") == 0)
    {
#ifdef Rtt_AUTHORING_SIMULATOR

		Rtt_Log("Note: native.setProperty(\"windowMode\", mode) does not work in the Simulator");

#else

        // Change the window mode to normal, minimized, maximized, fullscreen, etc.
        if (window != nil && (lua_type(L, valueIndex) == LUA_TSTRING))
        {
            // Fetch the requested window mode from Lua
            auto modeName = lua_tostring(L, valueIndex);
			BOOL isFullScreen = (([window styleMask] & NSFullScreenWindowMask) == NSFullScreenWindowMask);

			// Implement the "windowMode" setting.  Not all of the behaviors are exactly the same as those on
			// Windows (e.g. you can't go from "fullscreen" to "minimized").  I suspect fixing this is a tar pit.
			if (Rtt_StringCompareNoCase(modeName, "normal") == 0)
			{
				// OS X doesn't have the concept of "normal" windows so we just undo any
				// special presentations that have been applied previously
				if ([window isZoomed])
				{
					[window performZoom:nil]; // toggle zoomed state
				}

				if ([window isMiniaturized])
				{
					[window deminiaturize:nil];
				}

				if (isFullScreen)
				{
					[window toggleFullScreen:nil];
				}
			}
			else if (Rtt_StringCompareNoCase(modeName, "minimized") == 0)
			{
				[window performMiniaturize:nil];
			}
			else if (Rtt_StringCompareNoCase(modeName, "maximized") == 0)
			{
				[window performZoom:nil];
			}
			else if (Rtt_StringCompareNoCase(modeName, "fullscreen") == 0)
			{
				if (! isFullScreen)
				{
					[window toggleFullScreen:nil];
				}
			}
            else
            {
				if (Rtt_StringIsEmpty(modeName))
				{
					CoronaLuaWarning(L,"native.setProperty(\"windowMode\", mode) was given an empty mode");
				}
				else
				{
					CoronaLuaWarning(L, "native.setProperty(\"windowMode\", mode) was given an invalid mode name: %s", modeName);
				}

                return;
            }
        }
		else
		{
			CoronaLuaWarning(L, "native.setProperty(\"windowMode\", mode) expected a string parameter but got a %s", lua_typename(L, lua_type(L, valueIndex)));
		}
#endif // Rtt_AUTHORING_SIMULATOR
    }
    else if (Rtt_StringCompare(key, "mouseCursorVisible") == 0)
	{
		if (lua_type(L, valueIndex) == LUA_TBOOLEAN)
		{
			int cursorVisible = lua_toboolean(L, valueIndex);

			Rtt_ASSERT(fView != nil);

			if (cursorVisible)
			{
				[fView showCursor];
			}
			else
			{
				[fView hideCursor];
			}
		}
		else
		{
			CoronaLuaWarning(L, "native.setProperty(\"mouseCursorVisible\", mode) expected a boolean parameter but got a %s", lua_typename(L, lua_type(L, valueIndex)));
		}
	}
	else if (Rtt_StringCompare(key, "mouseCursor") == 0)
	{
		if (lua_type(L, valueIndex) == LUA_TSTRING)
		{
			auto requestedStyle = lua_tostring(L, valueIndex);
			NSRect cursorRect = NSMakeRect(0, 0, fView.frame.size.width, fView.frame.size.height);
			const char* validStyles[] = {
				"arrow", "closedHand", "openHand", "pointingHand", "crosshair",
				"notAllowed", "beam", "resizeRight", "resizeLeft",
				"resizeLeftRight", "resizeUp", "resizeDown", "resizeUpDown",
				"disappearingItem", "beamHorizontal", "dragLink", "dragCopy", "contextMenu",
				NULL
			};

			for (unsigned long i = 0; i < sizeof(validStyles) / sizeof(const char*); i++)
			{
				if (Rtt_StringCompare(requestedStyle, validStyles[i]) == 0)
				{
					[fView setCursor:requestedStyle forRect:cursorRect];
					break;
				}
			}
		}
		else
		{
			CoronaLuaWarning(L, "native.setProperty(\"mouseCursor\", cursor) expected a string parameter but got a %s", lua_typename(L, lua_type(L, valueIndex)));
		}
	}
	else if (Rtt_StringCompare(key, "preferredScreenEdgesDeferringSystemGestures")==0 ||
			 Rtt_StringCompare(key, "prefersHomeIndicatorAutoHidden")==0 ||
			 Rtt_StringCompare(key, "androidSystemUiVisibility")==0 ||
			 Rtt_StringCompare(key, "navigationBarColor")==0 ||
			 Rtt_StringCompare(key, "applicationIconBadgeNumber")==0 ||
			 Rtt_StringCompare(key, "applicationSupportsShakeToEdit")==0 ||
			 Rtt_StringCompare(key, "networkActivityIndicatorVisible")==0 ||
			 Rtt_StringCompare(key, "prefersHomeIndicatorAutoHidden")==0 ||
			 Rtt_StringCompare(key, "windowMode")==0 ||
			 Rtt_StringCompare(key, "windowTitleText")==0 ||
			 Rtt_StringCompare(key, "mouseCursorVisible")==0 ||
			 Rtt_StringCompare(key, "preferredScreenEdgesDeferringSystemGestures")==0)
	{
		Rtt_Log("Note: native.setProperty(\"%s\", value) does not work in the Simulator", key);
	}
	else
	{
		CoronaLuaWarning(L, "native.setProperty() was given an invalid selector: %s", key);
	}
}

int
MacPlatform::PushNativeProperty( lua_State *L, const char *key ) const
{
	// Rtt_TRACE_SIM(("MacPlatform::PushNativeProperty: key %s\n", key ));

	int pushedValues = 0;
	NSWindow *window = [fView window];

	// Set the requested native property.
	if (Rtt_StringCompare(key, "windowTitleText") == 0)
	{
		// Update the main window's title bar text
		if (window != nil)
		{
			lua_pushstring(L, [[window title] UTF8String]);
		}
		else
		{
			lua_pushstring(L, "");
		}

		pushedValues = 1;
	}
	else if (Rtt_StringCompare(key, "windowMode") == 0)
	{
		BOOL isFullScreen = (([window styleMask] & NSFullScreenWindowMask) == NSFullScreenWindowMask);
		const char *result = NULL;

		// This seems to be deterministic but it might be necessary to remember the last
		// mode we were given and return that for 100% consistency
		if ([window isZoomed])
		{
			result = "maximized";
		}
		else if ([window isMiniaturized])
		{
			result = "minimized";
		}
		else if (isFullScreen)
		{
			result = "fullscreen";
		}
		else
		{
			result = "normal";
		}

		lua_pushstring(L, result);
		pushedValues = 1;
	}
	else if (Rtt_StringCompare(key, "mouseCursorVisible") == 0)
	{
		Rtt_ASSERT(fView != nil);
		lua_pushboolean(L, (! fView.cursorHidden));
		pushedValues = 1;
	}
	else
	{
		if (Rtt_StringIsEmpty(key))
		{
			CoronaLuaWarning(L,"native.getProperty() was given a nil or empty string key");
		}
		else
		{
			CoronaLuaWarning(L,"native.getProperty() was given unknown key: %s", key);
		}
	}

	return pushedValues;
}

/*
void
MacPlatform::NetworkRequest( lua_State *L, const char *url, const char *method, LuaResource *listener, int paramsIndex ) const
{	
}
*/

void MacPlatform::GetSafeAreaInsetsPixels(Rtt_Real &top, Rtt_Real &left, Rtt_Real &bottom, Rtt_Real &right) const
{
#ifdef Rtt_AUTHORING_SIMULATOR
	MacSimulator *simulator = ((AppDelegate*)[NSApp delegate]).simulator;
	NSDictionary *properties = (simulator != nil ? simulator->GetProperties() : nil);

	float statusBarMult = (simulator && simulator->GetStatusBarMode()==kHiddenStatusBar)?0:1;

	if (DeviceOrientation::IsSideways(GetDevice().GetOrientation()))
	{
		top = [[properties valueForKey:@"safeLandscapeScreenInsetTop"] floatValue];
		left = [[properties valueForKey:@"safeLandscapeScreenInsetLeft"] floatValue];
		bottom = [[properties valueForKey:@"safeLandscapeScreenInsetBottom"] floatValue];
		right = [[properties valueForKey:@"safeLandscapeScreenInsetRight"] floatValue];

		top += statusBarMult * [[properties valueForKey:@"safeLandscapeScreenInsetStatusBar"] floatValue];
	}
	else
	{
		top = [[properties valueForKey:@"safeScreenInsetTop"] floatValue];
		left = [[properties valueForKey:@"safeScreenInsetLeft"] floatValue];
		bottom = [[properties valueForKey:@"safeScreenInsetBottom"] floatValue];
		right = [[properties valueForKey:@"safeScreenInsetRight"] floatValue];

		top += statusBarMult * [[properties valueForKey:@"safeScreenInsetStatusBar"] floatValue];
	}

#else

	top = left = bottom = right = 0;

#endif
}

// ----------------------------------------------------------------------------

// TODO: Move this to a separate file
#if !defined( Rtt_WEB_PLUGIN )

MacGUIPlatform::MacGUIPlatform( PlatformSimulator& simulator )
:	Super(NULL),
	fMacDevice(  GetAllocator(), simulator ),
	fAdaptiveWidth( PlatformSurface::kUninitializedVirtualLength ),
	fAdaptiveHeight( PlatformSurface::kUninitializedVirtualLength )
{
}

MPlatformDevice&
MacGUIPlatform::GetDevice() const
{
	return const_cast< MacDevice& >( fMacDevice );
}

void
MacGUIPlatform::SetStatusBarMode( StatusBarMode newValue ) const
{
	fMacDevice.GetSimulator().SetStatusBarMode( newValue );
}

MPlatform::StatusBarMode
MacGUIPlatform::GetStatusBarMode() const
{
	return fMacDevice.GetSimulator().GetStatusBarMode();
}

Real
MacGUIPlatform::GetStandardFontSize() const
{
	const MacSimulator &simulator = (const MacSimulator&)fMacDevice.GetSimulator();
	Real fontSize = Rtt_FloatToReal( [[simulator.GetProperties() valueForKey:@"defaultFontSize"] floatValue] );
	if (fontSize < Rtt_REAL_1 )
	{
		fontSize = Super::GetStandardFontSize();
	}
	return fontSize;
}

bool
MacGUIPlatform::RequestSystem( lua_State *L, const char *actionName, int optionsIndex ) const
{
	// Validate.
	if ( !actionName )
	{
		return false;
	}
	
	// Execute the requested operation.
	if ( Rtt_StringCompare( actionName, "exitApplication" ) == 0 )
	{
		// We're simulating a device via the Corona Simulator.
		// Request the simulator to terminate the Corona runtime, if supported by the simulated platform.
		const MacSimulator &simulator = (const MacSimulator&)fMacDevice.GetSimulator();
		if ( [[simulator.GetProperties() valueForKey:@"supportsExitRequests"] boolValue] )
		{
			AppDelegate* appdelegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
			CoronaLog("native.requestExit() called - application will close");
			[appdelegate performSelectorOnMainThread:@selector(close:) withObject:nil waitUntilDone:NO];
		}
		else
		{
			CoronaLuaError(L, "native.requestExit() is not supported on device \"%s\".", GetDevice().GetModel());
		}
		return true;
	}
	
	// The given action is not exclusive to the Corona Simulator. Let the base code handle it.
	return Super::RequestSystem( L, actionName, optionsIndex );
}

PlatformSurface*
MacGUIPlatform::CreateScreenSurface() const
{
	return Rtt_NEW( Allocator(), MacViewSurface( GetView(), fAdaptiveWidth, fAdaptiveHeight ) );
}

// ----------------------------------------------------------------------------


MacAppPlatform::MacAppPlatform( DeviceOrientation::Type orientation )
:	Super(NULL),
	fMacAppDevice(  GetAllocator(), orientation )
{
}

MPlatformDevice&
MacAppPlatform::GetDevice() const
{
	return const_cast< MacAppDevice& >( fMacAppDevice );
}

// ----------------------------------------------------------------------------
	
#ifdef Rtt_AUTHORING_SIMULATOR

// ----------------------------------------------------------------------------	

MacPlatformServices::MacPlatformServices( const MPlatform& platform )
:	fPlatform( platform )
{
}

const MPlatform&
MacPlatformServices::Platform() const
{
	return fPlatform;
}

#define Rtt_CORONA_DOMAIN "com.coronalabs.Corona_Simulator"
static const char kCoronaDomainUTF8[] = Rtt_CORONA_DOMAIN;
#undef Rtt_CORONA_DOMAIN

void
MacPlatformServices::GetPreference( const char *key, Rtt::String * value ) const
{
	const char *result = NULL;

	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	NSString *v = [defaults stringForKey:[NSString stringWithExternalString:key]];
	
	if( nil != v)
	{
		result = [v UTF8String];
	}

	value->Set( result );
}

void
MacPlatformServices::SetPreference( const char *key, const char *value ) const
{
	if ( Rtt_VERIFY( key ) )
	{
		NSString *v = ( value ? [[NSString alloc] initWithUTF8String:value] : nil );
		
		[[NSUserDefaults standardUserDefaults] setObject:v forKey:[NSString stringWithExternalString:key]];
		
		[v release];
	}
}

// These "*LibraryPreference" APIs are deprecated and just call the normal preference APIs
void
MacPlatformServices::GetLibraryPreference( const char *key, Rtt::String * value ) const
{
	GetPreference(key, value);
}

void 
MacPlatformServices::SetLibraryPreference( const char *key, const char *value ) const
{
	SetPreference(key, value);
}

void
MacPlatformServices::GetSecurePreference( const char *key, Rtt::String * value ) const
{
	UInt32 pwdLen = 0;
	void *pwd = NULL;

	NSString *tmp = nil;
	if ( noErr == SecKeychainFindGenericPassword(
							NULL, (UInt32) strlen( kCoronaDomainUTF8 ), kCoronaDomainUTF8,
							(UInt32) strlen( key ), key, & pwdLen, & pwd, NULL ) )
	{
		tmp = [[NSString alloc] initWithBytes:pwd length:pwdLen encoding:NSUTF8StringEncoding];
		(void)Rtt_VERIFY( noErr == SecKeychainItemFreeContent( NULL, pwd ) );
	}

	value->Set( [tmp UTF8String] );
	[tmp release];
}

bool
MacPlatformServices::SetSecurePreference( const char *key, const char *value ) const
{
	bool result = false;

	SecKeychainItemRef item = NULL;

	UInt32 keyLen = (UInt32) strlen( key );

	if ( errSecItemNotFound == SecKeychainFindGenericPassword(
							NULL, (UInt32) strlen( kCoronaDomainUTF8 ), kCoronaDomainUTF8,
							keyLen, key, NULL, NULL, & item ) )
	{
		// No item found, so add the item (provided value is not NULL)
		if ( value )
		{
			// Add password
			result = ( noErr == SecKeychainAddGenericPassword(
									NULL, (UInt32) strlen( kCoronaDomainUTF8 ), kCoronaDomainUTF8,
									keyLen, key, (UInt32) strlen( value ), value, NULL ) );
		}
		else
		{
			result = true;
		}
	}
	else
	{
		Rtt_ASSERT( item );
		if ( ! value )
		{
			// Remove password
			result = ( noErr == SecKeychainItemDelete( item ) );
		}
		else
		{
			Rtt_ASSERT( item );

			// Modify password
			result = ( noErr == SecKeychainItemModifyAttributesAndData( item, NULL, (UInt32) strlen( value ), value ) );
		}

		CFRelease( item );
	}

	return result;
}

static SCNetworkReachabilityRef
CreateWithAddress( const struct sockaddr_in* hostAddress )
{
	SCNetworkReachabilityRef result =
		SCNetworkReachabilityCreateWithAddress( NULL, (const struct sockaddr*)hostAddress );

	return result;
}

static bool
IsHostReachable( SCNetworkReachabilityRef reachability )
{
	Rtt_ASSERT( reachability );

	bool result = false;

	SCNetworkConnectionFlags flags;
	if ( SCNetworkReachabilityGetFlags( reachability, & flags ) )
	{
		if ( (flags & kSCNetworkFlagsReachable ) == 0 )
		{
			// if target host is not reachable
			// return NotReachable;
			goto exit_gracefully;
		}
		
		if ( (flags & kSCNetworkFlagsConnectionRequired) == 0 )
		{
			// if target host is reachable and no connection is required
			//  then we'll assume (for now) that your on Wi-Fi
			// retVal = ReachableViaWiFi;
			result = true;
			goto exit_gracefully;
		}
		
		
		if ( (flags & kSCNetworkFlagsConnectionAutomatic) != 0 )
		{
			// ... and the connection is on-demand (or on-traffic) if the
			//     calling application is using the CFSocketStream or higher APIs
			if ((flags & kSCNetworkFlagsInterventionRequired) == 0)
			{
				// ... and no [user] intervention is needed
				// retVal = ReachableViaWiFi;
				result = true;
				goto exit_gracefully;
			}
		}
		/*
		if ((flags & kSCNetworkReachabilityFlagsIsWWAN) == kSCNetworkReachabilityFlagsIsWWAN)
		{
			// ... but WWAN connections are OK if the calling application
			//     is using the CFNetwork (CFSocketStream?) APIs.
			// retVal = ReachableViaWWAN;
			result = true;
			goto exit_gracefully;
		}
		*/
	}

exit_gracefully:
	return result;
}

bool
MacPlatformServices::IsInternetAvailable() const
{
	bool result = false;

	struct sockaddr_in zeroAddress;
	bzero(&zeroAddress, sizeof(zeroAddress));
	zeroAddress.sin_len = sizeof(zeroAddress);
	zeroAddress.sin_family = AF_INET;

	SCNetworkReachabilityRef reachability = CreateWithAddress( & zeroAddress );
	if ( reachability )
	{
		result = IsHostReachable( reachability );
		CFRelease( reachability );
	}


	return result;
}

bool
MacPlatformServices::IsLocalWifiAvailable() const
{
	bool result = false;

	struct sockaddr_in localWifiAddress;
	bzero(&localWifiAddress, sizeof(localWifiAddress));
	localWifiAddress.sin_len = sizeof(localWifiAddress);
	localWifiAddress.sin_family = AF_INET;
	// IN_LINKLOCALNETNUM is defined in <netinet/in.h> as 169.254.0.0
	localWifiAddress.sin_addr.s_addr = htonl(IN_LINKLOCALNETNUM);

	SCNetworkReachabilityRef reachability = CreateWithAddress( & localWifiAddress );
	if ( reachability )
	{
		SCNetworkConnectionFlags flags;
		if ( SCNetworkReachabilityGetFlags( reachability, & flags ) )
		{
			result = (flags & kSCNetworkFlagsReachable) && (flags & kSCNetworkFlagsIsDirect);
		}

		CFRelease( reachability );
	}

	return result;
}

void
MacPlatformServices::Terminate() const
{
	NSApplication *application = [NSApplication sharedApplication];
	[application terminate:application];
}

void
MacPlatformServices::Sleep( int milliseconds ) const
{
	usleep( milliseconds*1000 );
}

// ----------------------------------------------------------------------------

#endif // Rtt_AUTHORING_SIMULATOR
	
// ----------------------------------------------------------------------------

Rtt_EXPORT CGSize
Rtt_GetScreenSize()
{
	CGSize result;
	result.width = NSZeroSize.width;
	result.height = NSZeroSize.height;

#ifdef Rtt_AUTHORING_SIMULATOR
	// TODO: Generalize this for current window
	MacSimulator *simulator = (MacSimulator *)[(AppDelegate *)[NSApp delegate] simulator];
	if ( simulator )
	{
		// Fetch the bounds from the screen size of the actual device (not the view size)
		result.width = simulator->GetScreenWidth();
		result.height = simulator->GetScreenHeight();
	}
#endif // Rtt_AUTHORING_SIMULATOR

	return result;
}

#endif // Rtt_WEB_PLUGIN

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

