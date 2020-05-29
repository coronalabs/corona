//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "PluginView.h"
#import <Foundation/NSString.h>
#import <Foundation/NSURL.h>
#import <WebKit/WebKit.h>

#include <zlib.h>

#include "Rtt_DeviceOrientation.h"
#include "Rtt_MacPlatform.h"
#include "Rtt_Runtime.h"

// Corona Executes Lua
static const NSString *kExtension = @"cel";

@implementation PluginView

+ (NSView *)plugInViewWithArguments:(NSDictionary *)arguments
{
	NSDictionary *attributes = [arguments valueForKey:WebPlugInAttributesKey];
	NSNumber *width = [attributes valueForKey:@"width"];
	NSNumber *height = [attributes valueForKey:@"height"];

	CGFloat w = [width floatValue];
	CGFloat h = [height floatValue];
	NSRect rect = { { 0.f, 0.f }, { w, h } };

	PluginView *plugin = [[[self alloc] initWithFrame:rect] autorelease];
	[plugin setArguments:arguments];
	return plugin;
}

- (id)initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if ( self )
	{
		fArguments = nil;
		fReceivedData = nil;
	}

	return self;
}

- (void)dealloc
{
	[fReceivedData release];
	[fArguments release];

	[super dealloc];
}

- (void)setArguments:(NSDictionary *)arguments
{
	if ( fArguments != arguments )
	{
		[fArguments release];
		fArguments = [arguments retain];
	}
}

// WebPlugIn protocol
// ----------------------------------------------------------------------------

- (void)webPlugInInitialize
{
	using namespace Rtt;

	Rtt_ASSERT( ! self.platform );

	MacPlatform *platform = new MacPlatform();
	if ( platform )
	{
		GLView *view = self.glView;

		platform->Initialize( view );
		platform->SetBundleClass( [PluginView class] );
		self.platform = platform;

		NSURL *baseUrl = [fArguments valueForKey:WebPlugInBaseURLKey];
		NSString *basePath = [baseUrl path];
		bool isCelFile = [[basePath pathExtension] isEqualToString:kExtension];
		bool isLocal = [baseUrl isFileURL];

		if ( isLocal )
		{
			// Application url
			if ( ! isCelFile )
			{
				NSString *resourcePath = [basePath stringByDeletingLastPathComponent];
				platform->SetResourcePath( [resourcePath UTF8String] );
			}
			else
			{
				Rtt_ASSERT_NOT_IMPLEMENTED();
			}
		}
		else
		{
			NSURLRequest *request = [NSURLRequest requestWithURL:baseUrl
													 cachePolicy:NSURLRequestUseProtocolCachePolicy
												 timeoutInterval:60.0];

			// Create the connection with the request and start loading the data
			NSURLConnection *connection= [[NSURLConnection alloc] initWithRequest:request delegate:self];
			if ( connection )
			{
				Rtt_ASSERT( ! fReceivedData );

				// Create the NSMutableData to hold the received data.
				// receivedData is an instance variable declared elsewhere.
				fReceivedData = [NSMutableData new];
			}
			else
			{
				// Inform the user that the connection failed.
			}

			// Network url --- need to download
			Rtt_ASSERT_NOT_IMPLEMENTED();
		}

		[view setDelegate:self];
	}
}

- (void)webPlugInStart
{
	 // No-op
	 // We must wait until OpenGL Context is ready
}

- (void)didPrepareOpenGLContext:(id)sender
{
	using namespace Rtt;

	Rtt_ASSERT( ! self.runtime );

	MPlatform *platform = self.platform;
	if ( Rtt_VERIFY( platform ) )
	{
		// Launch
		Runtime *runtime = new Runtime( * platform );
		if ( Rtt_VERIFY( runtime ) )
		{
			self.runtime = runtime;
			self.glView.runtime = runtime;

			if (  Runtime::kSuccess == runtime->LoadApplication( Runtime::kWebPluginLaunchOption, DeviceOrientation::kUpright ) )
			{
				runtime->BeginRunLoop();
			}
		}
	}
}

- (void)webPlugInStop
{
}

- (void)webPlugInDestroy
{
	// TODO: Delete uncompressed files
}

- (void)webPlugInSetIsSelected:(BOOL)isSelected
{
}

// NSURLConnection delegate methods
// ----------------------------------------------------------------------------

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection
{
	NSString *tmp = NSTemporaryDirectory();

	// TODO: Uncompress data and write to files	
	Rtt_ASSERT_NOT_IMPLEMENTED();
}

// Scripting Support
// ----------------------------------------------------------------------------

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)selector
{
/*
	if (selector == @selector(play) || selector == @selector(pause))
	{
		return NO;
	}
*/
	return YES;
}
 
+ (BOOL)isKeyExcludedFromWebScript:(const char *)property
{
/*
	if (strcmp(property,"muted") == 0)
	{
		return NO;
	}
*/
	return YES;
}
 
- (id)objectForWebScript
{
	return self;
}

/* 
- (void)play
{
    [self start:self];
}
 
- (void)pause
{
    [self stop:self];
}
*/

// ----------------------------------------------------------------------------

@end
