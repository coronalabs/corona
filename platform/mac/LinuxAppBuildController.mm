//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "LinuxAppBuildController.h"

#import "AppleSigningIdentityController.h"
#import "ValidationSupportMacUI.h"
#import "ValidationToolOutputViewController.h"

#import "Rtt_LinuxAppPackager.h"
#import "Rtt_MacPlatform.h"

#include "Rtt_Authorization.h"
#include "Rtt_AuthorizationTicket.h"
#include "Rtt_WebServicesSession.h"

#import "Rtt_MacConsolePlatform.h"
#import "TextEditorSupport.h"

// ----------------------------------------------------------------------------

using namespace Rtt;

// ----------------------------------------------------------------------------

@implementation LinuxAppBuildController

- (id)initWithWindowNibName:(NSString*)nibFile projectPath:(NSString *)projPath authorizer:(const Rtt::Authorization *)authorizer;
{
	self = [super initWithWindowNibName:nibFile projectPath:projPath authorizer:authorizer];

	if ( self )
	{
		platformName = @"linux";
		platformTitle = @"Linux";
	}

	return self;
}

- (void)dealloc
{
	[super dealloc];
}

- (BOOL)validateProject
{
	BOOL result = [super validateProject];

	if ( result )
	{
		// Run file validation tools so we can alert the users of any problems before we submit to the server.
		NSWindow *buildWindow = [self window];

		ValidationSupportMacUI *validator = [[ValidationSupportMacUI alloc] initWithParentWindow:buildWindow];
		{
			result = [validator runWebAppNameValidation:[self appName]];
		}
		[validator release];

		if ( ! result )
		{
			[self logEvent:@"build-bungled" key:@"reason" value:@"invalid-web-project"];
		}
	}

	return result;
}

- (BOOL)isStoreBuild
{
	return YES;
}

- (BOOL)isDeveloperBuild
{
	return NO;
}

- (Rtt::PlatformAppPackager*)createAppPackager:(Rtt::MPlatformServices*)services
{
	using namespace Rtt;

	LinuxAppPackager *result = new LinuxAppPackager( * services );
	return result;
}

- (BOOL)isTargetingXcodeSimulator
{
	return [[[fBuildFor selectedItem] title] isEqualToString:@"Xcode Simulator"];
}

- (void)willShowAlert:(NSAlert*)alert
{
	using namespace Rtt;

	NSString *validationMessage = nil;

	// Add a accessory view to the alertview if there are validation warnings to display them.
	if( (nil != validationMessage ) && ( ! [validationMessage isEqualToString:@""] ) )
	{
		[alert setMessageText:@"Your application built but failed to pass Apple's validation tests."];

		[alert setInformativeText:@"Your application cannot be uploaded to the App Store until it passes Apple's validation tests, though you may install it directly to provisioned devices."];

		ValidationToolOutputViewController* validationToolViewController = [[ValidationToolOutputViewController alloc] initWithNibName:@"ValidationToolOutput" bundle:nil];
		[validationToolViewController autorelease];
		[validationToolViewController setValidationMessage:validationMessage];
		[alert setAccessoryView:validationToolViewController.view];


	}
	else
	{
//		// If a distribution build for a device, then add a button for the Application Loader
//		// FIXME: Should test for Ad-hoc vs store
//		if ( ! [self isTargetingXcodeSimulator] && [self isStoreBuild] )
//		{
//			[alert addButtonWithTitle:@"Upload to App Store..."];
//		}
	}
}

- (BOOL)verifyBuildTools:(id)sender
{
	return YES;
}

- (BOOL)shouldRemoveWhitespaceFromPackageName
{
	return YES;
}

- (NSString*)appExtension
{
	return @"linuxapp";
}

- (void)alertDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void  *)contextInfo
{
//	if ( NSAlertThirdButtonReturn == returnCode )
//	{
//		NSString *dstApp = [self appPackagePath];
//		NSString* appuploadertool = [sdkRoot stringByAppendingPathComponent:@"/Applications/Utilities/Application Loader.app"];
//
//
//		NSFileManager* filemanager = [NSFileManager defaultManager];
//
//		NSString* zipfile = nil;
//
//		if( [filemanager fileExistsAtPath:dstApp] )
//		{
//			zipfile = [[dstApp stringByDeletingPathExtension] stringByAppendingPathExtension:@"zip"];
//
//			if( ! [filemanager fileExistsAtPath:zipfile] )
//			{
//				zipfile = nil;
//			}
//		}
//		[[NSWorkspace sharedWorkspace] openFile:zipfile withApplication:appuploadertool];
//	}

	[super alertDidEnd:alert returnCode:returnCode contextInfo:contextInfo];
}

- (BOOL)buildFormComplete
{
	BOOL result = [super buildFormComplete];
	if ( result )
	{
	}

	return result;
}

- (Rtt::TargetDevice::Platform)targetPlatform
{
	return Rtt::TargetDevice::kIPhonePlatform;
}


- (IBAction)build:(id)sender
{
	MacConsolePlatform platform;
	MacPlatformServices *services = new MacPlatformServices( platform );
	WebServicesSession *session = new WebServicesSession( *services );
	
	if ( ! [self verifyBuildTools:sender] )
	{
		return;
	}
	
	if ( ! [self validateProject] )
	{
		return;
	}
	
	if ( ! [self shouldInitiateBuild:sender] )
	{
		// Let them fix whatever the issue is and come back
		return;
	}
	
	[self setProgressBarLabel:@"Building for LINUX…"];

	__block NSString *message = nil;
	__block BOOL loginSuccessful = NO;
	

	// Login to the authorization server
	void (^login)() = ^()
	{
		loginSuccessful = [self loginSession:session services:services ticket:[appDelegate ticket] message:&message];
	};
	
	[self runLengthyOperationForWindow:[self window] delayProgressWindowBy:0 allowStop:NO withBlock:login];
	
	if (! loginSuccessful)
	{
		[self logEvent:@"build-bungled" key:@"reason" value:@"cannot-login"];
		[self showError:@"Cannot Login To Build Server" message:message helpURL:nil parentWindow:[self window]];
		return;
	}
	
	const char* name = [self.appName UTF8String];
	const char* versionname = NULL;
	
	// The number formatter for Android seems to be causing the value to be returned to be a NSNumber instead of NSString.
	// We must react accordingly.
	if( [self.appVersion isKindOfClass:[NSNumber class]] )
	{
		versionname = [[(NSNumber*)self.appVersion stringValue] UTF8String];
	}
	else
	{
		// Assume NSString?
		versionname = [self.appVersion UTF8String];
	}
	
	const char* dstDir = [self.dstPath UTF8String];

	Rtt::String resourcesDir;
	const char * srcDir = [self.projectPath UTF8String];
	
	platform.PathForFile( NULL, Rtt::MPlatform::kResourceDir, Rtt::MPlatform::kDefaultPathFlags, resourcesDir );
	
	LinuxAppPackager *packager = new LinuxAppPackager( *services);
	
	bool isvalidsettings = packager->ReadBuildSettings( srcDir );
	
	// Abort build if the build.settings is corrupt
	if ( ! isvalidsettings )
	{
		[self logEvent:@"build-bungled" key:@"reason" value:@"bad-build-settings"];
		
		NSString *buildSettingsError = [NSString stringWithExternalString:packager->GetErrorMesg()];
		
		Rtt_DELETE( packager );
		packager = NULL;
		
		[self showModalSheet:@"Error in build.settings" message:[NSString stringWithFormat:@"There is an error in `build.settings`:\n\n`%@`\n\nCorrect and retry the build.", buildSettingsError] buttonLabels:@[@"Dismiss", @"Edit build.settings" ] alertStyle:NSCriticalAlertStyle helpURL:nil parentWindow:[self window] completionHandler: ^(NSModalResponse returnCode)
		 {
			 if (returnCode == NSAlertSecondButtonReturn)
			 {
				 TextEditorSupport_LaunchTextEditorWithFile([self.projectPath stringByAppendingPathComponent:@"build.settings"], 0);
			 }
			 
			 [NSApp stopModalWithCode:returnCode];
		 }];
		
		return;
	}

	const Rtt::AuthorizationTicket *ticket = [appDelegate ticket];
	NSString *username = [NSString stringWithExternalString:ticket->GetUsername()];
	const char* identity = [username UTF8String];
	
	bool useStandartResources = (fUseStandartResources.state == NSOnState);
	
	int targetVersion = Rtt::TargetDevice::kLinux;
	const char * customBuildId = packager->GetCustomBuildId();
	LinuxAppPackagerParams * params = new LinuxAppPackagerParams(name, versionname, identity, NULL, srcDir, dstDir,	NULL, TargetDevice::kLinuxPlatform, targetVersion, TargetDevice::kLinux, customBuildId, NULL, "bundleId", true, NULL, useStandartResources );
	
	NSString *kBuildSettings = @"build.settings";
	params->SetBuildSettingsPath( [[self.projectPath stringByAppendingPathComponent:kBuildSettings] UTF8String]);

	
	// Some IDEs will terminate us quite abruptly so make sure we're on disk before starting a long operation
	[[NSUserDefaults standardUserDefaults] synchronize];
	
	// Do the actual build
	__block size_t code = WebServicesSession::kNoError;
	

	void (^performBuild)() = ^()
	{
		NSString* tmpDirBase = NSTemporaryDirectory();
		code = packager->Build( params, *session, [tmpDirBase UTF8String] );
	};
	
	[self runLengthyOperationForWindow:[self window] delayProgressWindowBy:0 allowStop:YES withBlock:performBuild];
	
	if (appDelegate.stopRequested)
	{
		[self logEvent:@"build-stopped"];
		
		Rtt_Log("WARNING: Build stopped by request");
		[self showMessage:@"Build Stopped" message:@"Build stopped by request" helpURL:nil parentWindow:[self window]];
	}
	else if (code == WebServicesSession::kNoError)
	{
		[self logEvent:@"build-succeeded"];
		
		[appDelegate notifyWithTitle:@"Corona Simulator"
										 description:[NSString stringWithFormat:@"LINUX build of \"%@\" complete", self.appName]
												iconData:nil];
		
		NSRunningApplication *app = [[NSRunningApplication runningApplicationsWithBundleIdentifier:@"com.coronalabs.CoronaLiveServer"] firstObject];
		if(!app) {
			NSString *liveServerPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"Corona Live Server.app"];
			app = [[NSWorkspace sharedWorkspace] launchApplicationAtURL:[NSURL fileURLWithPath:liveServerPath] options:(NSWorkspaceLaunchAndHide|NSWorkspaceLaunchWithoutActivation) configuration:@{} error:nil];
		}
		NSString *dstHtmlPath = [self.dstPath stringByAppendingPathComponent:self.appName];
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			if(!app.finishedLaunching) {
				while (![[[NSRunningApplication runningApplicationsWithBundleIdentifier:@"com.coronalabs.CoronaLiveServer"] firstObject] isFinishedLaunching]) {
					[NSThread sleepForTimeInterval:0.01];
				}
			}

			[[NSDistributedNotificationCenter defaultCenter] postNotificationName:@"linuxProjectBuilt" object:nil userInfo:@{@"root":dstHtmlPath} deliverImmediately:YES];
		});

		// Do nothing
		[self logEvent:@"build-post-action" key:@"post-action" value:@"do-nothing"];
	}
	else
	{
		[self logEvent:@"build-failed" key:@"reason" value:[NSString stringWithFormat:@"[%ld] %s", code, params->GetBuildMessage()]];
		
		NSString *msg = @"";
		
		[appDelegate notifyWithTitle:@"Corona Simulator"
										 description:[NSString stringWithFormat:@"Error building \"%@\" for Linux", self.appName]
												iconData:nil];
		
		// Display the error
		if (params->GetBuildMessage() != NULL)
		{
			msg = [NSString stringWithFormat:@"%s\n\n", params->GetBuildMessage()];
		}
		
		msg = [msg stringByAppendingFormat:@"Error code: %ld", code];
		
		[self showError:@"Build Failed" message:msg helpURL:nil parentWindow:[self window]];
	}
	
	[self closeBuild:self];
	[self saveBuildPreferences];
}

- (void) restoreBuildPreferences
{
	[super restoreBuildPreferences];

	NSString *def = [[NSUserDefaults standardUserDefaults] stringForKey:@"linuxIncludeResources"] ?: @"YES";
	BOOL useRes = [[appDelegate restoreAppSpecificPreference:@"linuxIncludeResources" defaultValue:def] isEqualToString:@"YES"];
	[fUseStandartResources setState:useRes?NSOnState:NSOffState];
}

- (void) saveBuildPreferences
{
	[super saveBuildPreferences];
	
	NSString * res = fUseStandartResources.state == NSOnState?@"YES":@"NO";
	[appDelegate saveAppSpecificPreference:@"linuxIncludeResources" value:res];
	[[NSUserDefaults standardUserDefaults] setObject:res forKey:@"linuxIncludeResources"];
}

@end

