//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_MacConsolePlatform.h"
#include "Rtt_Assert.h"
#include "Rtt_MacPlatform.h"
#include "Rtt_MacSimulatorServices.h"
#include "Rtt_MacSimulator.h"
#include "Rtt_ProjectSettings.h"

#import "TVOSAppBuildController.h"

#import "ValidationSupportMacUI.h"
#import "ValidationToolOutputViewController.h"
#import "XcodeToolHelper.h"
#import "NSAlert-OAExtensions.h"
#import "TextEditorSupport.h"
#import "SDKList.h"
#import "AppleSigningIdentityController.h"

using namespace Rtt;

static NSString *kMacAppStoreXcodeURL = @"macappstore://itunes.apple.com/us/app/xcode/id497799835";
static NSString *kDailyBuildsURL = @"https://github.com/coronalabs/corona/releases";
static NSString *kValueNotSet = @"not set";
//static NSString *kValueYes = @"YES";
//static NSString *kValueNo = @"NO";
static NSString *kiPhoneDistributionIdentityTag = @"iPhone Distribution";
static NSString *kValueNone = @"None";

// ----------------------------------------------------------------------------

@interface TVOSSimulatorMenuItem : NSMenuItem

@property (nonatomic, readwrite, copy) NSString *fullTitle;

@end

@implementation TVOSSimulatorMenuItem

-(id)initWithFullTitle:(NSString*)full_title title:(NSString *)short_title
{
	self = [super initWithTitle:short_title action:NULL keyEquivalent:@""];

	if ( self )
	{
		_fullTitle = [full_title retain];
	}

	return self;
}

-(void)dealloc
{
	[_fullTitle release];
	[super dealloc];
}

@end

// ----------------------------------------------------------------------------

@implementation TVOSAppBuildController

- (id)initWithWindowNibName:(NSString*)nibFile projectPath:(NSString *)projPath;
{
	self = [super initWithWindowNibName:nibFile projectPath:projPath];

	if ( self )
	{
		platformName = @"tvos";
		platformTitle = @"tvOS";
	}

	return self;
}

- (void)dealloc
{
	[super dealloc];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
}

- (void) showWindow:(id)sender;
{
    [super showWindow:sender];

/*
    // Temporary manual enabling of "Enable Monetization" checkbox
    BOOL showMonetizationCheckbox  = NO;
    NSString *showMonetizationCheckboxStr = [[NSUserDefaults standardUserDefaults] stringForKey:@"debugMonetizationPlugin"];

    if (showMonetizationCheckboxStr != nil)
    {
        showMonetizationCheckbox = [showMonetizationCheckboxStr boolValue];
    }

    if (! showMonetizationCheckbox)
    {
        [fEnableMonetization setState:NSOffState];
        [fEnableMonetization setHidden:YES];
    }
    else
    {
        // Restore the app specific setting for "Enable Monetization"
        BOOL enableMonetization = [[appDelegate restoreAppSpecificPreference:@"enableMonetization" defaultValue:kValueNotSet] isEqualToString:kValueYes];
        [fEnableMonetization setState:(enableMonetization ? NSOnState : NSOffState)];
        [fEnableMonetization setHidden:NO];
    }
*/

	if ([availableSimulatorsPopup numberOfItems] == 0)
	{
		NSDictionary *availableSimulators = [XcodeToolHelper loadSupportedIOSSimulatorDevices:@"tvOS"];

		if ([availableSimulators count] > 0)
		{
			[[availableSimulatorsPopup menu] removeAllItems];
			[[availableSimulatorsPopup menu] setAutoenablesItems:NO];

			// NSLog(@"availableSimulators: %@", availableSimulators);

			NSString *lastIOSSimulator = [appDelegate restoreAppSpecificPreference:@"lastTVOSSimulator" defaultValue:kValueNotSet];
			NSArray *sortedOSKeys = [[availableSimulators allKeys] sortedArrayUsingSelector:@selector(localizedStandardCompare:)];

			for (NSString *os in sortedOSKeys)
			{
				NSMenuItem *newTitle = [[NSMenuItem alloc] init];
				NSString *prettyOsName = os;
				NSRange prefixRange = [prettyOsName rangeOfString:@"com.apple.CoreSimulator.SimRuntime."];
				if(prefixRange.location == 0)
				{
					prettyOsName = [prettyOsName stringByReplacingCharactersInRange:prefixRange withString:@""];
					NSRange firstDash = [prettyOsName rangeOfString:@"-"];
					if(firstDash.location != NSNotFound)
					{
						prettyOsName = [prettyOsName stringByReplacingCharactersInRange:firstDash withString:@" "];
					}
					prettyOsName = [prettyOsName stringByReplacingOccurrencesOfString:@"-" withString:@"."];
				}
				[newTitle setTitle:prettyOsName];
				[newTitle setEnabled:NO];
				[[availableSimulatorsPopup menu] addItem:newTitle];

				NSDictionary *devices = [availableSimulators objectForKey:os];
				NSArray *sortedDeviceKeys = [[devices allKeys] sortedArrayUsingSelector:@selector(localizedStandardCompare:)];

				for (NSString *device in sortedDeviceKeys)
				{
					TVOSSimulatorMenuItem *newItem = [[TVOSSimulatorMenuItem alloc] initWithFullTitle:[NSString stringWithFormat:@"%@ / %@", device, prettyOsName]
																								title:[NSString stringWithFormat:@"      %@", device]];

					[newItem setEnabled:YES];
					[newItem setRepresentedObject:[devices objectForKey:device]];
					[newItem setAction:@selector(didSelectSimulator:)];
					[newItem setTarget:self];

					[[availableSimulatorsPopup menu] addItem:newItem];

					// This will pick the last "Apple TV" device on the latest iOS as the default (presumably the newest device)
					// (this will generally be overridden by a remembered setting)
					if ([device hasPrefix:@"Apple TV"])
					{
						[availableSimulatorsPopup selectItem:newItem];
					}

					if ([lastIOSSimulator isEqualToString:[devices objectForKey:device]])
					{
						currentTVOSSimulatorItem = newItem;
					}
				}
			}

			if (currentTVOSSimulatorItem != nil)
			{
				[availableSimulatorsPopup selectItem:currentTVOSSimulatorItem];
			}
			else if (! [lastIOSSimulator isEqualToString:kValueNotSet] && [lastIOSSimulator length] > 0 && currentTVOSSimulatorItem == nil)
			{
				// We have a remembered tvOS Simulator but it's not in the menu (happens frequently with Xcode updates)
				// (we'll fall through and use the new default)
				Rtt_Log("WARNING: previously used tvOS Simulator (%s) not available", [lastIOSSimulator UTF8String]);
			}

			// Make the tvOS Simulator popup display the correct string
			[self didSelectSimulator:[availableSimulatorsPopup selectedItem]];
		}
		else
		{
			// No tvOS Simulators configured in Xcode
			NSMenuItem *newTitle = [[NSMenuItem alloc] init];
			[newTitle setTitle:@"no devices found"];
			[newTitle setEnabled:NO];
			[[availableSimulatorsPopup menu] addItem:newTitle];

			// If the "Open in Xcode" option is chosen, choose "Do nothing"
			if ([postBuildRadioGroup selectedRow] == 1)
			{
				[postBuildRadioGroup selectCellAtRow:4 column:0];
			}

			// Disable "Open in Xcode" option
			[[postBuildRadioGroup cellAtRow:1 column:0] setEnabled:NO];

			Rtt_Log("WARNING: no Xcode tvOS Simulators found.  You should be able to rectify this using Xcode");
		}
	}

	[self populateTargetSDK:[self window] showBeta:true];
}

- (void) didSelectSimulator:(NSMenuItem *)menuItem
{
	TVOSSimulatorMenuItem *item = (TVOSSimulatorMenuItem *)menuItem;

	if (! [menuItem isKindOfClass:[TVOSSimulatorMenuItem class]])
	{
		// We get occasional [NSObject doesNotRecognizeSelector:] crashes
		// around this point in the field and this cast seems like the only
		// likely reason for them so bail if it isn't right
		return;
	}

	// Show the current item in the popup button
	NSMenuItem *customMenuItem = [[[NSMenuItem alloc] init] autorelease];
	[customMenuItem setTitle:item.fullTitle];
	[customMenuItem setRepresentedObject:[item representedObject]];
	[[availableSimulatorsPopup cell] setMenuItem:customMenuItem];

	currentTVOSSimulatorItem = menuItem;

	// Select the appropriate post-build radio button if they select an Xcode Simulator
	// (but only if we're past initialization of the dialog)
	if ([[fTargetSDK menu] numberOfItems] > 0)
	{
		[postBuildRadioGroup selectCellAtRow:1 column:0];
	}
}

- (IBAction)build:(id)sender
{
    Rtt_ASSERT(appDelegate);

	BOOL isLiveBuild = (fEnableLiveBuild.state == NSOnState);
    BOOL shouldSendToDevice = ([postBuildRadioGroup selectedRow] == 0); // first item in radio group
    BOOL shouldOpenInXcodeSimulator = ([postBuildRadioGroup selectedRow] == 1);
    BOOL shouldSendToAppStore = ([postBuildRadioGroup selectedRow] == 2);
    BOOL shouldShowApplication = ([postBuildRadioGroup selectedRow] == 3);

    useItunesConnect = shouldSendToAppStore;

	if ( ! [self verifyBuildTools:sender] )
	{
		return;
	}

    if ( ! [self validateProject] )
    {
        [self logEvent:@"build-bungled" key:@"reason" value:@"validate-project"];
		return;
    }

    if ( ! [self shouldInitiateBuild:sender] )
    {
        return;
    }

	if ([[currentProvisioningProfileItem title] isEqualToString:kValueNone] && ! shouldOpenInXcodeSimulator)
	{
		[self logEvent:@"build-bungled" key:@"reason" value:@"no-provisioning-profile"];

		[self showError:@"Provisioning Profile Needed" message:@"A provisioning profile is required to build for anything other than the Xcode AppleTV Simulator.\n\nPlease select a valid provisioning profile." helpURL:nil parentWindow:[self window]];

		return;
	}

    NSWindow *buildWindow = [self window];

    const char* dstDir = [self.dstPath UTF8String];
    const char* provisionFile = NULL;
    const char* identity = NULL;
    const char* identityName = NULL;

    SDKItem *currentSDK = [[fTargetSDK selectedItem] representedObject];
    Rtt::TargetDevice::Version targetVersion;
    bool isDistributionBuild = false;
    bool isStripDebug = true; // default to stripping

    Rtt_ASSERT( fTargetSDK != nil );

    targetVersion = (Rtt::TargetDevice::Version) currentSDK.coronaVersion;

	if ([[currentProvisioningProfileItem title] isEqualToString:kValueNone])
	{
		// We're building for the Xcode AppleTV Simulator without a provisioning profile
		isStripDebug = false;
		identityName = "none";
		identity = "none";
		provisionFile = NULL;
	}
	else
	{
		identityName = [currentProvisioningProfileItem.identity UTF8String];
		identity = [currentProvisioningProfileItem.fingerprint UTF8String];
		provisionFile = [currentProvisioningProfileItem.provisionPath UTF8String];

		if ( [[currentProvisioningProfileItem title] contains:kiPhoneDistributionIdentityTag] || [[currentProvisioningProfileItem title] contains:kAppleDistributionIdentityTag] )
		{
			// Here, we mean distribution build, as in store build,
			// so a store build is anything that does NOT have provisioned devices
			isDistributionBuild = ! [AppleSigningIdentityController hasProvisionedDevices:currentProvisioningProfileItem.provisionPath];
		}
		else
		{
			// Only preserve debug info in Lua for pure dev builds
			isStripDebug = false;
		}
	}

    // Remember the chosen cert for next time we need a default
    [[NSUserDefaults standardUserDefaults] setObject:[currentProvisioningProfileItem title] forKey:kUserPreferenceLastTVOSCertificate];

    MacConsolePlatform platform;
    MacPlatformServices *services = new MacPlatformServices( platform );

    const char* name = [self.appName UTF8String];
    const char* versionname = NULL;
    versionname = [self.appVersion UTF8String];

    S32 targetDevice = TargetDevice::kAppleTV;

    TVOSAppPackagerParams * params = NULL;
    const char * srcDir = [self.projectPath UTF8String];

    const char* apppackage = "NULL";

    TVOSAppPackager *packager = new TVOSAppPackager( *services, simulatorServices );

    bool isvalidsettings = packager->ReadBuildSettings( srcDir );

    // Abort build if the build.settings is invalid
    if ( ! isvalidsettings )
    {
		[self logEvent:@"build-bungled" key:@"reason" value:@"bad-build-settings"];

        NSString *buildSettingsError = [NSString stringWithExternalString:packager->GetErrorMesg()];

        Rtt_DELETE( packager );
        packager = NULL;

        // fAnalytics->Log( "build-bad-build-settings", NULL );

        [self showModalSheet:@"Error in build.settings" message:[NSString stringWithFormat:@"There is an error in `build.settings`:\n\n`%@`\n\nPlease correct and retry the build.", buildSettingsError] buttonLabels:@[@"Dismiss", @"Edit build.settings" ] alertStyle:NSCriticalAlertStyle helpURL:nil parentWindow:buildWindow completionHandler: ^(NSModalResponse returnCode)
             {
                 if (returnCode == NSAlertSecondButtonReturn)
                 {
                     TextEditorSupport_LaunchTextEditorWithFile([self.projectPath stringByAppendingPathComponent:@"build.settings"], 0);
                 }

                 [NSApp stopModalWithCode:returnCode];
             }];

        return;
    }
	
	// Enforce that apps must support landscape. If it does not, throw an error informing the user.
	ProjectSettings projectSettings; projectSettings.LoadFromDirectory( [self.projectPath UTF8String] );
	if ( ! projectSettings.IsLandscapeSupported() )
	{
		[self logEvent:@"build-bungled" key:@"reason" value:@"not-landscape"];

		[self showError:@"Unsupported Orientation" message:@"tvOS apps run exclusively in landscape orientation.\n\nEnsure that both your app and build.settings supports landscape." helpURL:nil parentWindow:buildWindow];
		
		return;
	}

    const char * customBuildId = packager->GetCustomBuildId();

    if (shouldOpenInXcodeSimulator)
    {
        targetDevice = (targetDevice + TargetDevice::kXCodeSimulator);
        isDistributionBuild = false;

		if (currentProvisioningProfileItem == nil || [[currentProvisioningProfileItem title] isEqualToString:kValueNone])
        {
            // We need a placeholder name for the server
            apppackage = "com.coronalabs.tvos.xcodesimulator";
        }
        else
        {
            apppackage = packager->GetBundleId( provisionFile, name );
        }

        // Don't sign Xcode AppleTV Simulator builds as some versions of macOS wont run them
		// (as a side effect we don't make an IPA even if a store identity was specified)
        identity = NULL;
    }
    else
    {
        apppackage = packager->GetBundleId( provisionFile, name );
    }

    if (apppackage == NULL || strlen(apppackage) == 0)
    {
        Rtt_DELETE( packager );
        packager = NULL;

		[self logEvent:@"build-bungled" key:@"reason" value:@"no-application-id"];

        [self showError:@"Cannot determine a valid application id for this application" message:@"Please check you are using the correct Provisioning Profile and try again." helpURL:nil parentWindow:buildWindow];

        return;
    }

    params = new TVOSAppPackagerParams(
                                      name,
                                      versionname,
                                      identity,
                                      provisionFile,
                                      srcDir,
                                      dstDir,
                                      [xCodeSdkRoot UTF8String],
                                      TargetDevice::kTVOSPlatform,
                                      targetVersion,
                                      targetDevice,
                                      customBuildId,
                                      NULL,
                                      apppackage,
                                      isDistributionBuild );

    params->SetStripDebug( isStripDebug );
	params->SetLiveBuild(isLiveBuild);
	if(currentSDK.customTemplate)
	{
		params->SetCustomTemplate([currentSDK.customTemplate UTF8String]);
	}

#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
/*
    // If "debugMonetizationPlugin" is set, honor the setting of the "Enable Monetization" checkbox
    NSString *debugMonetizationPluginStr = [[NSUserDefaults standardUserDefaults] stringForKey:@"debugMonetizationPlugin"];
    if ([debugMonetizationPluginStr boolValue])
    {
        BOOL includeFusePlugins  = ([fEnableMonetization state] == NSOnState);

        params->SetIncludeFusePlugins( includeFusePlugins );
        params->SetUsesMonetization( includeFusePlugins );
    }
*/
#endif // AUTO_INCLUDE_MONETIZATION_PLUGIN

	NSString *kBuildSettings = @"build.settings";
	params->SetBuildSettingsPath( [[self.projectPath stringByAppendingPathComponent:kBuildSettings] UTF8String]);

    [self setProgressBarLabel:@"Building for tvOS…"];

    // Do the actual build
    __block size_t code = PlatformAppPackager::kNoError;

	[self logEvent:@"build"];

    void (^performBuild)() = ^()
    {
        // Some IDEs will terminate us quite abruptly so make sure we're on disk before starting a long operation
        [[NSUserDefaults standardUserDefaults] synchronize];
        
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
                         description:[NSString stringWithFormat:@"tvOS build of \"%@\" complete", self.appName]
                            iconData:nil];
        
		if(isLiveBuild)
		{
			[self logEvent:@"build-is-live-build"];
			NSString *liveServerPath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"Corona Live Server.app"];
			[[NSWorkspace sharedWorkspace] openFile:self.projectPath withApplication:liveServerPath andDeactivate:NO];
		}

        if (shouldSendToDevice)
        {
            // Send to device
			[self logEvent:@"build-post-action" key:@"post-action" value:@"send-to-device"];

            [self sendAppToDevice];
        }
        else if (shouldOpenInXcodeSimulator)
        {
			[self logEvent:@"build-post-action" key:@"post-action" value:@"open-sim"];

			[self runAppInXcodeSimulator];
        }
        else if (shouldSendToAppStore)
        {
			[self logEvent:@"build-post-action" key:@"post-action" value:@"send-to-app-store"];

            [self sendToAppStore:buildWindow packager:packager params:params];
		}
        else if (shouldShowApplication)
        {
			[self logEvent:@"build-post-action" key:@"post-action" value:@"show-app"];

            // Reveal built app or ipa in Finder
			NSString *extension = ([self isStoreBuild] ? @"ipa" : @"app");
			NSString *bundleFile = [[self appBundleFile] stringByReplacingOccurrencesOfString:@"app" withString:extension options:0 range:NSMakeRange([[self appBundleFile] length] - 3, 3)];

            NSString *message = [NSString stringWithFormat:@"Showing built tvOS %@ *%@* in Finder", extension, self.appName];

            [[NSWorkspace sharedWorkspace] selectFile:bundleFile inFileViewerRootedAtPath:@""];

            [self showMessage:@"Build Complete" message:message helpURL:nil parentWindow:[self window]];

            [self closeBuild:self];
        }
        else
        {
            // Do nothing
			[self logEvent:@"build-post-action" key:@"post-action" value:@"do-nothing"];

            [self closeBuild:self];
        }
    }
    else
    {
		[self logEvent:@"build-failed" key:@"reason" value:[NSString stringWithFormat:@"[%ld] %s", code, params->GetBuildMessage()]];

        NSString *msg = nil;

        [appDelegate notifyWithTitle:@"Corona Simulator"
                         description:[NSString stringWithFormat:@"Error building \"%@\" for tvOS", self.appName]
                            iconData:nil];

        // Display the error
        if (params->GetBuildMessage() != NULL)
        {
            msg = [NSString stringWithExternalString:params->GetBuildMessage()];
        }
        else
        {
            msg = [NSString stringWithFormat:@"Error code: %ld\n\n", code];
        }
        
        [self showError:@"Build Failed" message:msg helpURL:nil parentWindow:buildWindow];
    }

    [self saveBuildPreferences];
}

/*
- (void) saveBuildPreferences
{
    [appDelegate saveAppSpecificPreference:@"enableMonetization" value:(([fEnableMonetization state] == NSOnState) ? kValueYes : kValueNo)];

    [super saveBuildPreferences];
}
*/

- (void) runAppInXcodeSimulator
{
	NSString *tvosSimulatorLauncher = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"xcodesim_sendapp.sh"];
	NSString *tvosSimulatorUDID = [currentTVOSSimulatorItem representedObject];
	NSString *appBundlePath = [self appBundleFile];
	TVOSSimulatorMenuItem *menuItem = (TVOSSimulatorMenuItem *) currentTVOSSimulatorItem;
	__block NSString *tvosSimulatorOutput = nil;

	Rtt_Log("Running '%s' on Xcode tvOS Simulator - %s (%s)",
			[appBundlePath UTF8String], [menuItem.fullTitle UTF8String], [tvosSimulatorUDID UTF8String]);

	[self setProgressBarLabel:@"Copying app to Xcode tvOS Simulator…"];

	void (^sendToTVOSSimBlock)() = ^()
	{
		// The copy is needed because the autoreleased return value gets released in the dancing of the threads
		tvosSimulatorOutput = [[self launchTaskAndReturnOutput:tvosSimulatorLauncher arguments:@[appBundlePath, tvosSimulatorUDID]] copy];
	};

	[self runLengthyOperationForWindow:[self window] delayProgressWindowBy:0 allowStop:YES withBlock:sendToTVOSSimBlock];

	if (appDelegate.stopRequested)
	{
		[self stopLaunchedTask];
		tvosSimulatorOutput = [@"WARNING: installation stopped by request" copy];
	}

	if ([tvosSimulatorOutput isEqualToString:@"ERROR:"])
	{
		// We didn't get anything on stderr so substitute something generic
		tvosSimulatorOutput = [@"ERROR: unexpected error - check the console for more information" copy];
	}

	Rtt_Log("Xcode tvOS Simulator installation:\n%s", [tvosSimulatorOutput UTF8String]);

	if ([tvosSimulatorOutput hasPrefix:@"ERROR:"])
	{
		NSString *errorMsg = @"*Corona Simulator* encountered an error installing the app:\n\n";

		[self logEvent:@"build-bungled" key:@"reason" value:@"sim-install-error"];

		[self showError:@"Xcode tvOS Simulator Problem" message:[errorMsg stringByAppendingString:tvosSimulatorOutput] helpURL:nil parentWindow:[self window]];
	}
	else
	{
		[self startTailDeviceSyslog:[XcodeToolHelper pathForXcodeSimulatorDeviceSyslogUtility] appBundlePath:appBundlePath deviceID:tvosSimulatorUDID];

		NSString *message = [NSString stringWithFormat:@"*%@* has been launched in the Xcode tvOS Simulator\n\nThe Xcode tvOS Simulator's syslog will appear in the Console until this message is closed", self.appName];

		[self showMessage:@"Xcode tvOS Simulator Running" message:message helpURL:nil parentWindow:[self window]];

		[self stopTailDeviceSyslog];
	}

	[tvosSimulatorOutput release];

	[self closeBuild:nil];
}

- (NSError *)willPresentError:(NSError *)error
{
    return [super willPresentError:error];
}

- (void)didPresentErrorWithRecovery:(BOOL)didRecover contextInfo:(void  *)contextInfo
{

}

- (BOOL)validateProject
{
	BOOL result = [super validateProject];

	if ( result )
	{
        // Check that we haven't already used this app name for an macOS build
        NSString *appBundle = [self appBundleFile];

        if([[NSFileManager defaultManager] fileExistsAtPath:appBundle] &&
           [[NSFileManager defaultManager] fileExistsAtPath:[appBundle stringByAppendingPathComponent:@"Contents/Info.plist"]])
        {
            [self showError:@"Build Problem" message:@"A macOS application with the same name exists in the output folder and will not be overwritten." helpURL:@"https://docs.coronalabs.com/guide/distribution/iOSBuild/index.html#build-process" parentWindow:[self window]];

            result = NO;
        }
	}

    if (result)
    {
        // Have they selected "Send to App Store"?
        if ([postBuildRadioGroup selectedRow] == 2)
        {
            if (! [self isStoreBuild])
            {
				[self logEvent:@"build-bungled" key:@"reason" value:@"not-distribution-provisioning-profile"];

                [self showError:@"Cannot Send To App Store" message:@"Only apps built with distribution profiles can be sent to the App Store.\n\nChoose a provisioning profile signed with an \"iPhone Distribution\" or \"Apple Distribution\" certificate and note that the provisioning profile used should not specify any devices (i.e. is not \"ad hoc\")." helpURL:@"https://docs.coronalabs.com/guide/distribution/iOSBuild/index.html#build-process" parentWindow:[self window]];

                result = NO;
            }
            else
            {
#ifdef USE_APPLICATION_LOADER
                // They can't "Send to App Store" if we don't have iTunes Connect credentials available or the standalone version
                // of Application Loader installed (we actually use the copy of Application Loader embedded in Xcode but the "altool"
                // CLI doesn't work properly unless the standalone version is *also* installed)
                NSString *applicationLoader = @"/Applications/Application Loader.app";
                BOOL isApplicationLoaderInstalled = NO;

                if ([[NSWorkspace sharedWorkspace] isFilePackageAtPath:applicationLoader])
                {
                    isApplicationLoaderInstalled = YES;
                }

                if ([itunesConnectPassword isEqualToString:@""]) || ! isApplicationLoaderInstalled)
                {
                    NSString *message = @"In order to send to the App Store the following tasks must be performed:\n\n";

                    if (! isApplicationLoaderInstalled)
                    {
                        message = [message stringByAppendingString:@" - install the standalone version of Application Loader from [iTunes Connect](https://itunespartner.apple.com/en/apps/overview) (see \"Download\" link at the bottom of the page)\n"];
                    }

                    if ([itunesConnectPassword isEqualToString:@""])
                    {
                        message = [message stringByAppendingString:@" - log into [Application Loader](launch-bundle:com.apple.itunes.connect.ApplicationLoader) and save your credentials\n"];
                    }

                    message = [message stringByAppendingString:@"\nOr you can open [Application Loader](launch-bundle:com.apple.itunes.connect.ApplicationLoader) to manage your app manually."];

                    // FIXME: current connection to the internet is also a requirement

                    [self showMessage:@"Cannot Send To App Store" message:message helpURL:nil parentWindow:[self window]];
                    
                    result = NO;
                }
#endif // USE_APPLICATION_LOADER
            }
        }
    }
    
    if (result)
    {
        // Have they selected "Copy to device"?
        if ([postBuildRadioGroup selectedRow] == 0 && [self isStoreBuild])
        {
            [self showError:@"Cannot Copy To Device" message:@"Only apps built with developer profiles can be copied to local devices.\n\nChoose a provisioning profile signed with an \"iPhone Developer\" certificate to copy to device." helpURL:@"https://docs.coronalabs.com/guide/distribution/iOSBuild/index.html#build-process" parentWindow:[self window]];

            result = NO;
        }
    }
    
	if (result)
	{
		if (fEnableLiveBuild.state == NSOnState && [self isStoreBuild])
		{
			[self showError:@"Cannot create Live Build" message:@"Live Build cannot be created with distribution profile selected."
			 "\n\nChoose a provisioning profile signed with an \"iPhone Developer\" certificate." helpURL:@"https://docs.coronalabs.com/guide/distribution/iOSBuild/index.html#build-process" parentWindow:[self window]];
			result = NO;
		}
	}

	return result;
}

- (BOOL)isStoreBuild
{
	using namespace Rtt;

	IdentityMenuItem* item = currentProvisioningProfileItem; Rtt_ASSERT( item != [fSigningIdentities itemAtIndex:0] );
	BOOL result = ( [[item title] contains:kiPhoneDistributionIdentityTag]  || [[item title] contains:kAppleDistributionIdentityTag] );

	if ( result )
	{
		// A store build is anything that does NOT have provisioned devices (e.g. Ad Hoc is not a store build)
		result = ! [AppleSigningIdentityController hasProvisionedDevices:item.provisionPath];

		if (! result)
		{
			Rtt_Log("Provisioning profile '%s' has provisioned devices (i.e. it is adhoc)", [[item title] UTF8String]);
		}
	}

	return result;
}

- (BOOL)isDeveloperBuild
{
	IdentityMenuItem* item = currentProvisioningProfileItem; Rtt_ASSERT( item != [fSigningIdentities itemAtIndex:0] );

	// A developer build does NOT have iPhone Distribution in the prefix
	BOOL result = ! ( [[item title] contains:kiPhoneDistributionIdentityTag] || [[item title] contains:kAppleDistributionIdentityTag]);
	return result;
}

// Called by the AppDelegate to determine if we're setup to build for this platform
- (BOOL)verifyBuildTools:(id)sender
{
	BOOL isOkayToBuild = NO;
	NSString* error_string = nil;
	NSString *sdkRoot = [self developerRoot:&isOkayToBuild errorString:&error_string ];
	Rtt_UNUSED( sdkRoot );

	if ( ! isOkayToBuild )
	{
		NSString *installXcodeBtn = @"Install Xcode";
		NSString *dailyBuildBtn = @"Daily Builds";
		NSString *title = nil;
		NSString *msg = nil;
		NSString *helpURL = @"https://docs.coronalabs.com/guide/start/systemReqs/index.html#macos";
		NSArray *buttons = @[ installXcodeBtn,dailyBuildBtn,  @"Cancel Build" ];

		if ( sdkRoot == nil || [sdkRoot isEqualToString:@""] )
		{
			[self logEvent:@"build-bungled" key:@"reason" value:@"no-xcode"];
			
			// No Xcode found
			title = @"Xcode Required";

			msg = [NSString stringWithFormat:@"The Xcode tvOS SDK could not be found. Please install Xcode (or use `xcode-select` to choose an existing installation).\n\nXcode is required by Solar2D and needs to be installed to build tvOS applications.\n\nPress the *%@* button to go to the App Store and get Xcode.  When it is installed, build for tvOS again.\n", installXcodeBtn];

			// fAnalytics->Log("bad-tvos-sdk", "not-found");
		}
		else
		{
			[self logEvent:@"build-bungled" key:@"reason" value:@"invalid-xcode"];

			// xcode-select gave us a path, but a component could not be found.
			title = @"Xcode Compatibility Problem";

			msg = [NSString stringWithFormat:@"Solar2D can't find the following components in the\nXcode %g tvOS SDK located at *%@*:\n\n%@\n\nPlease update Solar2D to the latest [Daily Build](%@) (it might also be necessary to re-install Xcode)",
						 [XcodeToolHelper getXcodeVersion], sdkRoot, error_string, kDailyBuildsURL];

			// fAnalytics->Log("bad-tvos-sdk", "incomplete");
		}

		NSInteger alertResult = [self showModalSheet:title message:msg buttonLabels:buttons alertStyle:NSCriticalAlertStyle helpURL:helpURL parentWindow:[self window] completionHandler:nil];

		Rtt_Log( "Corona Simulator: %s", [msg UTF8String] );

		if (alertResult == NSAlertFirstButtonReturn)
		{
			// Go to Xcode in the App Store
			[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:kMacAppStoreXcodeURL]];
		}
		else if (alertResult == NSAlertSecondButtonReturn)
		{
			// Go to the Daily Builds page
			[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:kDailyBuildsURL]];
		}

		[self closeBuild:sender];
		return NO;
    }

	return isOkayToBuild && [super verifyBuildTools:sender];
}

- (BOOL)shouldRemoveWhitespaceFromPackageName
{
	return NO;
}

- (Rtt::TargetDevice::Platform)targetPlatform
{
	return Rtt::TargetDevice::kTVOSPlatform;
}

- (void) populateTargetSDK:(NSWindow *)buildWindow showBeta:(BOOL)showBeta
{
    Rtt_ASSERT( fTargetSDK != nil );

    // parse the JSON describing the supported SDKs
    NSError *jsonError = nil;
    NSString *supportedSDKsFile = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"tvOS-SDKs.json"];
    NSData *jsonData = [NSData dataWithContentsOfFile:supportedSDKsFile];
    id jsonObject = [NSJSONSerialization JSONObjectWithData:jsonData options:kNilOptions error:&jsonError];

    [[fTargetSDK menu] removeAllItems];
    [[fTargetSDK menu] setAutoenablesItems:NO];

    // If we successfully parse the JSON data proceed to act on it
    if (jsonError == nil && [jsonObject isKindOfClass:[NSDictionary class]])
    {
        NSDictionary *supportedSDKsDict = (NSDictionary *)jsonObject;
        SDKList *supportedSDKs = [[[SDKList alloc] initWithDictionary:supportedSDKsDict platform:@"tvos"] autorelease];
        BOOL foundCompatibleSDK = NO;
        SDKItem *firstNonBetaSDK = nil;
        NSString *defaultSDK = [self defaultSDK];

        if ([defaultSDK length] == 0)
        {
            defaultSDK = @"<check console for error>";
        }

        for (SDKItem *sdk in supportedSDKs.SDKItems)
        {
            NSMenuItem *newItem = [[NSMenuItem alloc] init];

            /*
             NSLog(@"        sdk.label: %@", sdk.label);
             NSLog(@" sdk.xcodeVersion: %@", sdk.xcodeVersion);
             NSLog(@"sdk.coronaVersion: %g", sdk.coronaVersion);
             NSLog(@"         sdk.beta: %d (%@)", sdk.beta, (showBeta ? @"shown" : @"not shown"));
             NSLog(@"  sdk.failMessage: %@", sdk.failMessage);
             */

            // Remember the first non-beta SDK we find
            if (! sdk.beta && firstNonBetaSDK == nil)
            {
                firstNonBetaSDK = sdk;
            }

            if (sdk.beta && ! showBeta)
            {
                continue;
            }

            // We enable whichever SDK is the default one for the current Xcode installation (which may be
            // changed using xcode-select to choose between different SDKs)
            if ([sdk.xcodeVersion isEqualToString:defaultSDK])
            {
                [newItem setTitle:[NSString stringWithFormat:@"%@%@", sdk.label, (sdk.beta ? @" BETA" : @"")]];
                [newItem setEnabled:YES];
                // Provide access to the SDK attributes later
                [newItem setRepresentedObject:sdk];
                [[fTargetSDK menu] addItem:newItem];
                [fTargetSDK selectItem:newItem]; // always make sure a valid item is selected
                foundCompatibleSDK = YES;
            }
            else
            {
                [newItem setTitle:[NSString stringWithFormat:@"%@%@ (%@)", sdk.label, (sdk.beta ? @" BETA" : @""), sdk.failMessage]];
                [newItem setEnabled:NO];
                [[fTargetSDK menu] addItem:newItem];
            }
        }

        // We must always have a non-beta SDK listed in the JSON file
        Rtt_ASSERT(firstNonBetaSDK != nil);

        // If we didn't find a target SDK which is compatible with the one installed on the user's computer we
        // add a "get lucky" option which offers, with warnings, the chance to try linking the potentially
        // incompatible SDKs with each other (you might end up linking an iphoneos8.2 build from the server with
        // an iphoneos8.0 SDK On the local machine which may or may not work but at least you aren't dead
        // in the water)
        if (! foundCompatibleSDK && firstNonBetaSDK != nil)
        {
            NSMenuItem *newItem = [[NSMenuItem alloc] init];

            [newItem setTitle:[NSString stringWithFormat:@"%@ (UNSUPPORTED: Local tvOS SDK is %@ which does not match)", firstNonBetaSDK.label, defaultSDK]];
            [[fTargetSDK menu] addItem:newItem];
            [fTargetSDK selectItem:newItem];
            // We'll detect this case later and just use the default SDK found on the client
            firstNonBetaSDK.xcodeVersion = nil;
            [newItem setRepresentedObject:firstNonBetaSDK];

            if (! fWarnedAboutUnsupportedSDK)
            {
                // Make sure the build window is visible so that the error sheet displays correctly
                [buildWindow makeKeyAndOrderFront:nil];

                fWarnedAboutUnsupportedSDK = true;

                NSAlert* alert = [[[NSAlert alloc] init] autorelease];
                [alert addButtonWithTitle:@"Yes"];
                [alert addButtonWithTitle:@"No"];

                NSString *informativeText = [NSString stringWithFormat:@"The version of tvOS SDK (%@) that's installed on this computer does not match the target tvOS version (%@). You may encounter issues building or incompatibilities at runtime.\n\nYou should consider updating your version of Xcode or using xcode-select to enable a compatible version.\n\nDo you want to proceed with a build?", defaultSDK, firstNonBetaSDK.label];
                NSString *message = @"UNSUPPORTED tvOS SDK Mismatch";

                Rtt_PRINT(("%s\n%s", [message UTF8String], [informativeText UTF8String]));

                [alert setMessageText:message];
                [alert setInformativeText:informativeText];
                [alert setAlertStyle:NSInformationalAlertStyle];
                [alert setHelpAnchor:kMacAppStoreXcodeURL]; // show Xcode in the App Store
                [alert setShowsHelp:YES];
                // Fix up buttons: put focus ring on button #1 ("No")
                NSArray *buttons = [alert buttons];
                Rtt_ASSERT( [buttons count] == 2 );
                [[alert window] makeFirstResponder:[buttons objectAtIndex:1]]; // stop the focus ring settling on the last button defined
                [alert setDelegate:self];
                [alert beginSheetModalForWindow:buildWindow
                                  modalDelegate:self
                                 didEndSelector:@selector(populateTargetSDKAlertDidEnd:returnCode:contextInfo:)
                                    contextInfo:buildWindow];
            }
        }
    }
    else
    {
        // We have a problem with the supported SDKs JSON file
        Rtt_ERROR(("populateTargetSDK: problem parsing '%s': %s", [supportedSDKsFile UTF8String], [[jsonError localizedFailureReason] UTF8String]));
        Rtt_ERROR(("------\n%s\n------", [jsonData bytes]));

        // Make sure the build window is visible so that the error sheet displays correctly
        [buildWindow makeKeyAndOrderFront:nil];

        NSAlert* alert = [[[NSAlert alloc] init] autorelease];
        [alert addButtonWithTitle:@"OK"];

        NSString *message = [NSString stringWithFormat:@"We were unable to parse the list of supported tvOS SDKs.  Check the console for more information.\n\nYou should re-install Solar2D."];

        [alert setMessageText:@"Internal Error"];
        [alert setInformativeText:message];
        [alert setAlertStyle:NSCriticalAlertStyle];
        [alert setDelegate:self];
        [alert beginSheetModalForWindow:buildWindow
                          modalDelegate:self
                         didEndSelector:@selector(populateTargetSDKAlertDidEnd:returnCode:contextInfo:)
                            contextInfo:buildWindow];
    }
}

-(void)populateTargetSDKAlertDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void  *)contextInfo
{
    if ( NSAlertSecondButtonReturn == returnCode )
    {
        // They opted not to continue with the build, cancel the build dialog
        [self closeBuild:nil];
    }
}

- (NSString *) defaultSDK
{
    NSString *cmd = @"/usr/bin/xcrun";
    NSArray *args = @[@"--sdk", @"appletvos", @"--show-sdk-version" ];
    
    return [self launchTaskAndReturnOutput:cmd arguments:args];
}

- (void) commonPostBuildTabOKActions
{

}

- (void) sendAppToDevice
{
	NSString *appBundlePath = [self appBundleFile];
	NSString *deviceInstallationUtility = [XcodeToolHelper pathForIOSDeviceInstallationUtility];

	Rtt_ASSERT(deviceInstallationUtility != nil);

	Rtt_Log("Installing '%s' on tvOS device", [appBundlePath UTF8String]);

	__block NSString *ideviceinstallerOutput = nil;

	[self setProgressBarLabel:@"Copying app to tvOS device…"];

	void (^sendToDeviceBlock)() = ^()
	{
		// The copy is needed because the autoreleased return value gets released in the dancing of the threads
        ideviceinstallerOutput = [[self launchTaskAndReturnOutput:deviceInstallationUtility arguments:@[appBundlePath, @"Apple TVOS"]] copy];
	};

	[self runLengthyOperationForWindow:[self window] delayProgressWindowBy:0 allowStop:NO withBlock:sendToDeviceBlock];

	if ([ideviceinstallerOutput contains:@"can't find 'ideviceinstaller'"])
	{
		NSString *errorMsg = @"*ideviceinstaller* does not appear to be installed on this computer\n\n"
		"To enable device installation:\n\n"
		"1. install Homebrew: [https://brew.sh/](https://brew.sh/) (_if not already installed_)\n"
		"2. run the Terminal command:\n\n"
		"\t`brew install ideviceinstaller`\n\n"
		"[More info](https://libimobiledevice.org/)";

		[self showError:@"Device Installation Utility Not Found" message:errorMsg helpURL:nil parentWindow:[self window]];
	}
	else if ([ideviceinstallerOutput contains:@"ERROR:"] || [ideviceinstallerOutput contains:@"Error occurred:"])
	{
		[self logEvent:@"build-bungled" key:@"reason" value:@"failed-to-copy-to-device"];

		NSString *errorMsg = @"*ideviceinstaller* encountered an error installing the app:\n\n";

		[self showError:@"App Installation Problem" message:[errorMsg stringByAppendingString:ideviceinstallerOutput] helpURL:nil parentWindow:[self window]];
	}
	else
	{
		Rtt_Log("%s", [ideviceinstallerOutput UTF8String]);

		[self startTailDeviceSyslog:[XcodeToolHelper pathForIOSDeviceSyslogUtility] appBundlePath:appBundlePath deviceID:@"Apple TVOS"];

		NSString *message = [NSString stringWithFormat:@"*%@* is installed on the tvOS device and is ready to run\n\nThe device's syslog will appear in the Console until this message is closed (you'll need to *launch* the app on the device before anything appears in the syslog)", self.appName];

		[self showMessage:@"App Installation Complete" message:message helpURL:nil parentWindow:[self window]];

		[self stopTailDeviceSyslog];
	}

	[ideviceinstallerOutput release];

	[self closeBuild:nil];
}

- (void) sendToAppStore:(NSWindow*)parent packager:(TVOSAppPackager *)packager params:(TVOSAppPackagerParams *)params
{
     [self setProgressBarLabel:@"Sending app to App Store…"];

    __block size_t code = PlatformAppPackager::kNoError;

#ifdef USE_APPLICATION_LOADER
	// All this does is make calls to Application Loader so unless we're using that, it can skipped
    void (^sendToAppStoreBlock)() = ^()
    {
        code = packager->SendToAppStore( params, [itunesConnectUsername UTF8String], [itunesConnectPassword UTF8String]);
    };

    [self runLengthyOperationForWindow:[self window] delayProgressWindowBy:0 allowStop:NO withBlock:sendToAppStoreBlock];
#endif // USE_APPLICATION_LOADER

    if (code != PlatformAppPackager::kNoError)
    {
		NSString *buildMsg = nil;
        NSString *title = nil;

		if (params->GetBuildMessage() != NULL)
		{
			buildMsg = [NSString stringWithExternalString:params->GetBuildMessage()];
		}
		else
		{
			buildMsg = [NSString stringWithFormat:@"Unexpected build result %ld", code];
		}

		title = @"Build Error";

		if ([buildMsg contains:@"Transporter not found at path"])
		{
			buildMsg = [buildMsg stringByAppendingString:@"\n\nThis may be an Xcode issue which is most easily worked around by opening [Application Loader](launch-bundle:com.apple.itunes.connect.ApplicationLoader) and managing your app manually."];

			// Show the .ipa we built in the Finder to make use of Application Loader easier
			[[NSWorkspace sharedWorkspace] selectFile:[[self appBundleFile] stringByReplacingOccurrencesOfString:@".app" withString:@".ipa"] inFileViewerRootedAtPath:@""];

			// Make us foreground again
			[[NSRunningApplication currentApplication] activateWithOptions:(NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps)];
		}

        [self showError:title message:buildMsg helpURL:nil parentWindow:[self window]];
    }
    else
    {
#ifdef USE_APPLICATION_LOADER
        NSString *message = [NSString stringWithFormat:@"*%@* has been sent to the App Store", self.appName];

		[appDelegate notifyWithTitle:@"Corona Simulator"
						 description:[NSString stringWithFormat:@"Upload of \"%@\" to the App Store is complete", self.appName]
							iconData:nil];
#else
		NSString *message;
		if([[NSWorkspace sharedWorkspace] URLForApplicationToOpenURL:[NSURL URLWithString:@"transporter://"]])
		{
			message = [NSString stringWithFormat:@"*%@* is ready to be sent to the App Store using the [Transporter](launch-bundle:com.apple.TransporterApp|macappstore://itunes.apple.com/app/id1450874784) app.\n\nPress *Add App* on the *Transporter* window to load `%@` into it", self.appName, [[self appBundleFile] stringByReplacingOccurrencesOfString:@".app" withString:@".ipa"]];
		}
		else
		{
			message = [NSString stringWithFormat:@"*%@* is ready to be sent to the App Store. Install and run the [Transporter](launch-bundle:com.apple.TransporterApp|macappstore://itunes.apple.com/app/id1450874784) app.\n\nAfter signing into *Transporter* app, press *Add App* on its window to load `%@` into it", self.appName, [[self appBundleFile] stringByReplacingOccurrencesOfString:@".app" withString:@".ipa"]];
		}

		// Open Application Loader
		[[NSWorkspace sharedWorkspace] launchAppWithBundleIdentifier:@"com.apple.TransporterApp"
															 options:NSWorkspaceLaunchDefault
									  additionalEventParamDescriptor:nil
													launchIdentifier:nil];

		// Show the .ipa we built in the Finder to make use of Application Loader easier
		[[NSWorkspace sharedWorkspace] selectFile:[[self appBundleFile] stringByReplacingOccurrencesOfString:@".app" withString:@".ipa"] inFileViewerRootedAtPath:@""];

		// Make us foreground again
		[[NSRunningApplication currentApplication] activateWithOptions:(NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps)];

#endif // USE_APPLICATION_LOADER

        [self showMessage:@"Build Complete" message:message helpURL:nil parentWindow:[self window]];
    }
}

@end

// ----------------------------------------------------------------------------


