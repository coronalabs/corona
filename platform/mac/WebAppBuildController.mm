//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "WebAppBuildController.h"

#import "AppleSigningIdentityController.h"
#import "ValidationSupportMacUI.h"
#import "ValidationToolOutputViewController.h"

#import "Rtt_WebAppPackager.h"
#import "Rtt_MacPlatform.h"

#import "Rtt_MacConsolePlatform.h"
#import "TextEditorSupport.h"

// ----------------------------------------------------------------------------

using namespace Rtt;

// ----------------------------------------------------------------------------

@implementation WebAppBuildController

- (id)initWithWindowNibName:(NSString*)nibFile projectPath:(NSString *)projPath;
{
	self = [super initWithWindowNibName:nibFile projectPath:projPath];

	if ( self )
	{
		platformName = @"web";
		platformTitle = @"Web";
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

	WebAppPackager *result = new WebAppPackager( * services );
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
	return @"webapp";
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
	
	[self setProgressBarLabel:@"Building for HTML5…"];
	
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
	
	WebAppPackager *packager = new WebAppPackager( *services);
	
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
	
	bool useStandartResources = (fUseStandartResources.state == NSOnState);
	bool createFBInstantArchive = (fCreateFBInstantArchive.state == NSOnState);

	int targetVersion = Rtt::TargetDevice::kWeb1_0;
	const char * customBuildId = packager->GetCustomBuildId();
	WebAppPackagerParams * params = new WebAppPackagerParams(name, versionname, NULL, NULL, srcDir, dstDir,	NULL, TargetDevice::kWebPlatform, targetVersion, TargetDevice::kWebGenericBrowser, customBuildId, NULL, "bundleId", true, useStandartResources, NULL, createFBInstantArchive );
	
	NSString *kBuildSettings = @"build.settings";
	params->SetBuildSettingsPath( [[self.projectPath stringByAppendingPathComponent:kBuildSettings] UTF8String]);

	
	// Some IDEs will terminate us quite abruptly so make sure we're on disk before starting a long operation
	[[NSUserDefaults standardUserDefaults] synchronize];
	
	// Do the actual build
	__block size_t code = PlatformAppPackager::kNoError;
	

	void (^performBuild)() = ^()
	{
		NSString* tmpDirBase = NSTemporaryDirectory();
		code = packager->Build( params, [tmpDirBase UTF8String] );
	};
	
	[self runLengthyOperationForWindow:[self window] delayProgressWindowBy:0 allowStop:YES withBlock:performBuild];
	
	if (appDelegate.stopRequested)
	{
		[self logEvent:@"build-stopped"];
		
		Rtt_Log("WARNING: Build stopped by request");
		[self showMessage:@"Build Stopped" message:@"Build stopped by request" helpURL:nil parentWindow:[self window]];
	}
	else if (code == PlatformAppPackager::kNoError)
	{
		[self logEvent:@"build-succeeded"];
		
		[appDelegate notifyWithTitle:@"Corona Simulator"
										 description:[NSString stringWithFormat:@"HTML5 build of \"%@\" complete", self.appName]
												iconData:nil];
		
	//	if (shouldShowApplication)
	//	{
			// Reveal built app in Finder
	//		[self logEvent:@"build-post-action" key:@"post-action" value:@"show-app"];
			
	//		NSString *message = [NSString stringWithFormat:@"Showing built HTML5 app *%@* in Finder", self.appName];
			
	//		[[NSWorkspace sharedWorkspace] selectFile:[self appBundleFile] inFileViewerRootedAtPath:@""];
			
	//		[self showMessage:@"Build Complete" message:message helpURL:nil parentWindow:[self window]];
			
	//		[self closeBuild:self];
		NSRunningApplication *app = [[NSRunningApplication runningApplicationsWithBundleIdentifier:@"com.coronalabs.CoronaLiveServer"] firstObject];
		if(!app) {
			NSString *liveServerPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"Corona Live Server.app"];
			app = [[NSWorkspace sharedWorkspace] launchApplicationAtURL:[NSURL fileURLWithPath:liveServerPath] options:(NSWorkspaceLaunchAndHide|NSWorkspaceLaunchWithoutActivation) configuration:@{} error:nil];
		}
		NSString *dstHtmlPath = [self.dstPath stringByAppendingPathComponent:[self.appName stringByAppendingString:@".html5"]];
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			if(!app.finishedLaunching) {
				while (![[[NSRunningApplication runningApplicationsWithBundleIdentifier:@"com.coronalabs.CoronaLiveServer"] firstObject] isFinishedLaunching]) {
					[NSThread sleepForTimeInterval:0.01];
				}
			}

			[[NSDistributedNotificationCenter defaultCenter] postNotificationName:@"html5ProjectBuilt" object:nil userInfo:@{@"root":dstHtmlPath} deliverImmediately:YES];
		});


	//	}
	//	else
	//	{
			// Do nothing
			[self logEvent:@"build-post-action" key:@"post-action" value:@"do-nothing"];
			
		//	[self closeBuild:self];
	//	}
	}
	else
	{
		[self logEvent:@"build-failed" key:@"reason" value:[NSString stringWithFormat:@"[%ld] %s", code, params->GetBuildMessage()]];
		
		NSString *msg = @"";
		
		[appDelegate notifyWithTitle:@"Corona Simulator"
										 description:[NSString stringWithFormat:@"Error building \"%@\" for HTML5", self.appName]
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

	NSString *def = [[NSUserDefaults standardUserDefaults] stringForKey:@"webIncludeResources"] ?: @"YES";
	BOOL useRes = [[appDelegate restoreAppSpecificPreference:@"webIncludeResources" defaultValue:def] isEqualToString:@"YES"];
	[fUseStandartResources setState:useRes?NSOnState:NSOffState];
	
	NSString *def2 = [[NSUserDefaults standardUserDefaults] stringForKey:@"createFBInstantArchive"] ?: @"YES";
	BOOL createFBInstantArchive = [[appDelegate restoreAppSpecificPreference:@"createFBInstantArchive" defaultValue:def2] isEqualToString:@"YES"];
	[fCreateFBInstantArchive setState:createFBInstantArchive?NSOnState:NSOffState];
}

- (void) saveBuildPreferences
{
	[super saveBuildPreferences];

	NSString * res = fUseStandartResources.state == NSOnState?@"YES":@"NO";
	[appDelegate saveAppSpecificPreference:@"webIncludeResources" value:res];
	[[NSUserDefaults standardUserDefaults] setObject:res forKey:@"webIncludeResources"];

	NSString * res2 = fCreateFBInstantArchive.state == NSOnState?@"YES":@"NO";
	[appDelegate saveAppSpecificPreference:@"createFBInstantArchive" value:res2];
	[[NSUserDefaults standardUserDefaults] setObject:res2 forKey:@"createFBInstantArchive"];
}

@end

