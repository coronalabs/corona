//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "IPhoneBuild.h"

#import <AppKit/NSApplication.h>
#import <AppKit/NSNibLoading.h>
#import <AppKit/NSOpenPanel.h>

#include "Rtt_Event.h"

@class NSNotification;

// -------------------------

#import "AppDelegate.h"
#include "Rtt_MacPlatform.h"
#include "Rtt_MacSimulator.h"
#include "Rtt_PlatformAppPackager.h"
#include "Rtt_PlatformPlayer.h"
#include "Rtt_PlatformSimulator.h"
#include "Rtt_Runtime.h"

// -------------------------

@implementation IPhoneBuild

@synthesize appName;
@synthesize appVersion;
@synthesize username;
@synthesize password;

-(id)init
{
	self = [super init];
	if ( self )
	{
		fBuildWindow = nil;
		fSigningIdentities = nil;
		appName = nil;
		appVersion = nil;
		username = nil;
		password = nil;
	}
	return self;
}

-(IBAction)openForBuild:(id)sender
{
	using namespace Rtt;

	NSApplication* application = [NSApplication sharedApplication];
	AppDelegate* appDelegate = (AppDelegate*)[application delegate];
	NSString* appPath = [self showOpenPanel];

	if ( appPath )
	{
		if ( ! fBuildWindow )
		{
			[NSBundle loadNibNamed:@"DeviceBuild" owner:self];
			Rtt_ASSERT( fSigningIdentities );

			// Initialize contents of signing identities
		}

		if ( fBuildWindow )
		{
			if ( NSRunAbortedResponse != [application runModalForWindow:fBuildWindow] )
			{
				MacPlatformServices services( fSimulator->GetPlayer()->GetPlatform() );
				PlatformAppPackager packager( services );

				const char* name = [appName UTF8String];
				const char* version = [appVersion UTF8String];

#if 1
				name = "Animation1";
				version = "1.0";
#endif

				AppPackagerParams params = 
				{
					name,
					version,
					"iPhone Distribution: ansca, inc",
					"/Users/wluh/Library/MobileDevice/Provisioning Profiles/AdHoc.mobileprovision",
					[appPath UTF8String], // "/Volumes/rtt/apps/SampleCode/Animation1",
					"/Users/wluh/tmp"
				};

				const char* usr = [username UTF8String];
				const char* pwd = [password UTF8String];

#if 1
				usr = "testuser";
				pwd = "abc123";
#endif

				packager.Build( params, usr, pwd );
			}
		}
	}
}

-(IBAction)build:(id)sender
{
	[[NSApplication sharedApplication] stopModal];
}

-(IBAction)cancelBuild:(id)sender
{
	[[NSApplication sharedApplication] abortModal];
}


@end
