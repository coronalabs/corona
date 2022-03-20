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

#import "IOSAppBuildController.h"

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

@interface IOSSimulatorMenuItem : NSMenuItem

@property (nonatomic, readwrite, copy) NSString *fullTitle;

@end

@implementation IOSSimulatorMenuItem

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

@implementation IOSAppBuildController

- (id)initWithWindowNibName:(NSString*)nibFile projectPath:(NSString *)projPath;
{
	self = [super initWithWindowNibName:nibFile projectPath:projPath];

	if ( self )
	{
        platformName = @"ios";
		platformTitle = @"iOS";
		tailSyslogTask = nil;
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

#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
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
#endif // AUTO_INCLUDE_MONETIZATION_PLUGIN

	if ([availableSimulatorsPopup numberOfItems] == 0)
	{
		NSDictionary *availableSimulators = [XcodeToolHelper loadSupportedIOSSimulatorDevices:@"iOS"];

		if ([availableSimulators count] > 0)
		{
			[[availableSimulatorsPopup menu] removeAllItems];
			[[availableSimulatorsPopup menu] setAutoenablesItems:NO];

			// NSLog(@"availableSimulators: %@", availableSimulators);

			NSString *lastIOSSimulator = [appDelegate restoreAppSpecificPreference:@"lastIOSSimulator" defaultValue:kValueNotSet];
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
					IOSSimulatorMenuItem *newItem = [[IOSSimulatorMenuItem alloc] initWithFullTitle:[NSString stringWithFormat:@"%@ / %@", device, prettyOsName]
																							  title:[NSString stringWithFormat:@"      %@", device]];

					[newItem setEnabled:YES];
					[newItem setRepresentedObject:[devices objectForKey:device]];
					[newItem setAction:@selector(didSelectSimulator:)];
					[newItem setTarget:self];

					[[availableSimulatorsPopup menu] addItem:newItem];

					// This will pick the last "iPhone" device on the latest iOS as the default (presumably the newest device)
					// (this will generally be overridden by a remembered setting)
					if ([device hasPrefix:@"iPhone "])
					{
						[availableSimulatorsPopup selectItem:newItem];
					}

					if ([lastIOSSimulator isEqualToString:[devices objectForKey:device]])
					{
						currentIOSSimulatorItem = newItem;
					}
				}
			}

			if (currentIOSSimulatorItem != nil)
			{
				[availableSimulatorsPopup selectItem:currentIOSSimulatorItem];
			}
			else if (! [lastIOSSimulator isEqualToString:kValueNotSet] && [lastIOSSimulator length] > 0 && currentIOSSimulatorItem == nil)
			{
				// We have a remembered iOS Simulator but it's not in the menu (happens frequently with Xcode updates)
				// (we'll fall through and use the new default)
				Rtt_Log("WARNING: previously used iOS Simulator (%s) not available", [lastIOSSimulator UTF8String]);
			}

			// Make the iOS Simulator popup display the correct string
			[self didSelectSimulator:[availableSimulatorsPopup selectedItem]];
		}
		else
		{
			// No iOS Simulators configured in Xcode
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

			Rtt_Log("WARNING: no Xcode iOS Simulators found.  You should be able to rectify this using Xcode");
		}
	}

	[self willChangeValueForKey:@"buildFormComplete"];

	[self populateTargetIOSSDK:[self window] showBeta:true];

	[self didChangeValueForKey:@"buildFormComplete"];
}

- (void) didSelectSimulator:(NSMenuItem *)menuItem
{
	IOSSimulatorMenuItem *item = (IOSSimulatorMenuItem *)menuItem;

	if (! [menuItem isKindOfClass:[IOSSimulatorMenuItem class]])
	{
		// We get occasional [NSObject doesNotRecognizeSelector:] crashes
		// around this point in the field and this cast seems like the only
		// likely reason for them so bail if it isn't right
		return;
	}

	[self willChangeValueForKey:@"buildFormComplete"];

	// Show the current item in the popup button
	NSMenuItem *customMenuItem = [[[NSMenuItem alloc] init] autorelease];
	[customMenuItem setTitle:item.fullTitle];
	[customMenuItem setRepresentedObject:[item representedObject]];
	[[availableSimulatorsPopup cell] setMenuItem:customMenuItem];

	currentIOSSimulatorItem = menuItem;

	// Select the appropriate post-build radio button if they select an Xcode Simulator
	// (but only if we're past initialization of the dialog)
	if ([[fTargetIOSSDK menu] numberOfItems] > 0)
	{
		[postBuildRadioGroup selectCellAtRow:1 column:0];
	}

	[self didChangeValueForKey:@"buildFormComplete"];
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
		[self showError:@"Provisioning Profile Needed" message:@"A provisioning profile is required to build for anything other than the Xcode iOS Simulator.\n\nPlease select a valid provisioning profile." helpURL:nil parentWindow:[self window]];

		return;
	}

    NSWindow *buildWindow = [self window];

    const char* dstDir = [self.dstPath UTF8String];
    const char* provisionFile = NULL;
    const char* identity = NULL;
    const char* identityName = NULL;

    SDKItem *currentSDK = [[fTargetIOSSDK selectedItem] representedObject];
    Rtt::TargetDevice::Version targetVersion;
    bool isDistributionBuild = false;
    bool isStripDebug = true; // default to stripping

    Rtt_ASSERT( fTargetIOSSDK != nil );

    targetVersion = (Rtt::TargetDevice::Version) currentSDK.coronaVersion;

	if ([[currentProvisioningProfileItem title] isEqualToString:kValueNone])
	{
		// We're building for the Xcode iOS Simulator without a provisioning profile
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

		if ( [[currentProvisioningProfileItem title] contains:kiPhoneDistributionIdentityTag] || [[currentProvisioningProfileItem title] contains:kAppleDistributionIdentityTag])
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
    [[NSUserDefaults standardUserDefaults] setObject:[currentProvisioningProfileItem title] forKey:kUserPreferenceLastIOSCertificate];

    MacConsolePlatform platform;
    MacPlatformServices *services = new MacPlatformServices( platform );

    const char* name = [self.appName UTF8String];
    const char* versionname = NULL;
    versionname = [self.appVersion UTF8String];

    TargetDevice::Platform targetDevice = TargetDevice::kUnknownPlatform;

    // the tags of the NSPopupButton are set to match our TargetDevice constants
    targetDevice = (TargetDevice::Platform ) [[fSupportedDevices selectedItem] tag];

    IOSAppPackagerParams * params = NULL;
    const char * srcDir = [self.projectPath UTF8String];

    const char* apppackage = "NULL";

    IOSAppPackager *iosPackager = new IOSAppPackager( *services, simulatorServices );

    bool isvalidsettings = iosPackager->ReadBuildSettings( srcDir );

    // Abort build if the build.settings is invalid
    if ( ! isvalidsettings )
    {
        NSString *buildSettingsError = [NSString stringWithExternalString:iosPackager->GetErrorMesg()];

        Rtt_DELETE( iosPackager );
        iosPackager = NULL;

        [self logEvent:@"build-bungled" key:@"reason" value:@"bad-build-settings"];

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

    const char * customBuildId = iosPackager->GetCustomBuildId();

    if (shouldOpenInXcodeSimulator)
    {
        targetDevice = (TargetDevice::Platform ) (targetDevice + TargetDevice::kXCodeSimulator);
        isDistributionBuild = false;

		if (currentProvisioningProfileItem == nil || [[currentProvisioningProfileItem title] isEqualToString:kValueNone])
        {
            // We need a placeholder name for the server
            apppackage = "com.coronalabs.ios.xcodesimulator";
        }
        else
        {
            apppackage = iosPackager->GetBundleId( provisionFile, name );
        }

        // Don't sign Xcode iOS Simulator builds as some versions of macOS wont run them
		// (as a side effect we don't make an IPA even if a store identity was specified)
        identity = NULL;
    }
    else
    {
        apppackage = iosPackager->GetBundleId( provisionFile, name );
    }

    if (apppackage == NULL || strlen(apppackage) == 0)
    {
        Rtt_DELETE( iosPackager );
        iosPackager = NULL;

        [self showError:@"Cannot determine a valid application id for this application" message:@"Please check you are using the correct Provisioning Profile and try again." helpURL:nil parentWindow:buildWindow];

        return;
    }

    params = new IOSAppPackagerParams(
                                      name,
                                      versionname,
                                      identity,
                                      provisionFile,
                                      srcDir,
                                      dstDir,
                                      [xCodeSdkRoot UTF8String],
                                      TargetDevice::kIPhonePlatform,
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
    // If "debugMonetizationPlugin" is set, honor the setting of the "Enable Monetization" checkbox
    NSString *debugMonetizationPluginStr = [[NSUserDefaults standardUserDefaults] stringForKey:@"debugMonetizationPlugin"];
    if ([debugMonetizationPluginStr boolValue])
    {
        BOOL includeFusePlugins  = ([fEnableMonetization state] == NSOnState);

        params->SetIncludeFusePlugins( includeFusePlugins );
        params->SetUsesMonetization( includeFusePlugins );
    }
#endif // AUTO_INCLUDE_MONETIZATION_PLUGIN

	NSString *kBuildSettings = @"build.settings";
	params->SetBuildSettingsPath( [[self.projectPath stringByAppendingPathComponent:kBuildSettings] UTF8String]);

    [self setProgressBarLabel:@"Building for iOS…"];

    // Do the actual build
    __block size_t code = PlatformAppPackager::kNoError;

	[self logEvent:@"build"];

    void (^performBuild)() = ^()
    {
        // Some IDEs will terminate us quite abruptly so make sure we're on disk before starting a long operation
        [[NSUserDefaults standardUserDefaults] synchronize];
        
        NSString* tmpDirBase = NSTemporaryDirectory();
        code = iosPackager->Build( params, [tmpDirBase UTF8String] );
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
                         description:[NSString stringWithFormat:@"iOS build of \"%@\" complete", self.appName]
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

            [self runAppInXcodeSimulator:targetDevice];
        }
        else if (shouldSendToAppStore)
        {
			[self logEvent:@"build-post-action" key:@"post-action" value:@"send-to-app-store"];

			[self sendToAppStore:buildWindow packager:iosPackager params:params];
        }
        else if (shouldShowApplication)
        {
			[self logEvent:@"build-post-action" key:@"post-action" value:@"show-app"];

            // Reveal built app or ipa in Finder
			NSString *extension = ([self isStoreBuild] ? @"ipa" : @"app");
			NSString *bundleFile = [[self appBundleFile] stringByReplacingOccurrencesOfString:@"app" withString:extension options:0 range:NSMakeRange([[self appBundleFile] length] - 3, 3)];

            NSString *message = [NSString stringWithFormat:@"Showing built iOS %@ *%@* in Finder", extension, self.appName];

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
                         description:[NSString stringWithFormat:@"Error building \"%@\" for iOS", self.appName]
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

- (void) saveBuildPreferences
{
#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
	[appDelegate saveAppSpecificPreference:@"enableMonetization" value:(([fEnableMonetization state] == NSOnState) ? kValueYes : kValueNo)];
#endif
	[appDelegate saveAppSpecificPreference:@"lastIOSSimulator" value:[currentIOSSimulatorItem representedObject]];

    [super saveBuildPreferences];
}

- (void) runAppInXcodeSimulator:(TargetDevice::Platform) targetDevice
{
	NSString *iosSimulatorLauncher = [XcodeToolHelper pathForXcodeSimulatorDeviceInstallationUtility];
	NSString *iosSimulatorUDID = [currentIOSSimulatorItem representedObject];
	NSString *appBundlePath = [self appBundleFile];
	IOSSimulatorMenuItem *menuItem = (IOSSimulatorMenuItem *) currentIOSSimulatorItem;
	__block NSString *iosSimulatorOutput = nil;

	if (iosSimulatorUDID == nil)
	{
		Rtt_Log("ERROR: No Xcode iOS Simulator found");
		return;
	}

	Rtt_Log("Running '%s' on Xcode iOS Simulator - %s (%s)",
			[appBundlePath UTF8String], [menuItem.fullTitle UTF8String], [iosSimulatorUDID UTF8String]);

	[self setProgressBarLabel:@"Copying app to Xcode iOS Simulator…"];

	void (^sendToIOSSimBlock)() = ^()
	{
		// The copy is needed because the autoreleased return value gets released in the dancing of the threads
		iosSimulatorOutput = [[self launchTaskAndReturnOutput:iosSimulatorLauncher arguments:@[appBundlePath, iosSimulatorUDID]] copy];
	};

	[self runLengthyOperationForWindow:[self window] delayProgressWindowBy:0 allowStop:YES withBlock:sendToIOSSimBlock];

	if (appDelegate.stopRequested)
	{
		[self stopLaunchedTask];

		iosSimulatorOutput = [@"WARNING: installation stopped by request" copy];
	}

	Rtt_Log("Xcode iOS Simulator installation:\n%s", [iosSimulatorOutput UTF8String]);

	if ([iosSimulatorOutput isEqualToString:@"ERROR:"])
	{
		// We didn't get anything on stderr so substitute something generic
		iosSimulatorOutput = [@"ERROR: unexpected error - check the console for more information" copy];
	}

	if ([iosSimulatorOutput hasPrefix:@"ERROR:"])
	{
		NSString *errorMsg = @"*Corona Simulator* encountered an error installing the app:\n\n";

		[self showError:@"Xcode iOS Simulator Problem" message:[errorMsg stringByAppendingString:iosSimulatorOutput] helpURL:nil parentWindow:[self window]];
	}
	else
	{
		[self startTailDeviceSyslog:[XcodeToolHelper pathForXcodeSimulatorDeviceSyslogUtility] appBundlePath:appBundlePath deviceID:iosSimulatorUDID];

		NSString *message = [NSString stringWithFormat:@"*%@* has been launched in the Xcode iOS Simulator\n\nThe Xcode iOS Simulator's syslog will appear in the Console until this message is closed", self.appName];

		[self showMessage:@"Xcode iOS Simulator Running" message:message helpURL:nil parentWindow:[self window]];

		[self stopTailDeviceSyslog];
	}

	[iosSimulatorOutput release];

	[self closeBuild:nil];
}

- (NSError *)willPresentError:(NSError *)error
{
    return [super willPresentError:error];
}

- (void)didPresentErrorWithRecovery:(BOOL)didRecover contextInfo:(void  *)contextInfo
{

}

- (BOOL) buildFormComplete
{
	return [super buildFormComplete];
}

- (BOOL)validateProject
{
	BOOL result = [super validateProject];

	if ( result )
	{
        // Check that we haven't already used this app name for a macOS build
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

                if ([itunesConnectPassword isEqualToString:@""] || ! isApplicationLoaderInstalled)
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
	BOOL result = ( [[item title] contains:kiPhoneDistributionIdentityTag] || [[item title] contains:kAppleDistributionIdentityTag] );

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
		NSArray *buttons = @[ installXcodeBtn, dailyBuildBtn, @"Cancel Build" ];

		if ( sdkRoot == nil || [sdkRoot isEqualToString:@""] )
		{
			// No Xcode found
			title = @"Xcode Required";

			msg = [NSString stringWithFormat:@"The Xcode iOS SDK could not be found. Please install Xcode (or use `xcode-select` to choose an existing installation).\n\nXcode is required by Solar2D and needs to be installed to build iOS applications.\n\nPress the *%@* button to go to the App Store and get Xcode.  When it is installed, build for iOS again.\n", installXcodeBtn];

			[self logEvent:@"build-bungled" key:@"reason" value:@"ios-sdk-not-found"];
		}
		else
		{
			// xcode-select gave us a path, but a component could not be found.
			title = @"Xcode Compatibility Problem";

			msg = [NSString stringWithFormat:@"Solar2D can't find the following components in the\nXcode %g iOS SDK located at *%@*:\n\n%@\n\nPlease update Solar2D to the latest [Daily Build](%@) (it might also be necessary to re-install Xcode)",
						 [XcodeToolHelper getXcodeVersion], sdkRoot, error_string, kDailyBuildsURL];

			[self logEvent:@"build-bungled" key:@"reason" value:@"ios-sdk-incomplete"];
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
	return Rtt::TargetDevice::kIPhonePlatform;
}

- (void) populateTargetIOSSDK:(NSWindow *)buildWindow showBeta:(BOOL)showBeta
{
    Rtt_ASSERT( fTargetIOSSDK != nil );

    // parse the JSON describing the supported SDKs
    NSError *jsonError = nil;
    NSString *supportedSDKsFile = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"iOS-SDKs.json"];
    NSData *jsonData = [NSData dataWithContentsOfFile:supportedSDKsFile];
    id jsonObject = [NSJSONSerialization JSONObjectWithData:jsonData options:kNilOptions error:&jsonError];

    [[fTargetIOSSDK menu] removeAllItems];
    [[fTargetIOSSDK menu] setAutoenablesItems:NO];

    // If we successfully parse the JSON data proceed to act on it
    if (jsonError == nil && [jsonObject isKindOfClass:[NSDictionary class]])
    {
        NSDictionary *supportedSDKsDict = (NSDictionary *)jsonObject;
        SDKList *supportedSDKs = [[[SDKList alloc] initWithDictionary:supportedSDKsDict platform:@"ios"] autorelease];
        BOOL foundCompatibleSDK = NO;
        SDKItem *firstNonBetaSDK = nil;
        NSString *defaultIOSSDK = [self defaultIOSSDK];

        if ([defaultIOSSDK length] == 0)
        {
            defaultIOSSDK = @"<check console for error>";
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
            if ([sdk.xcodeVersion isEqualToString:defaultIOSSDK])
            {
                [newItem setTitle:[NSString stringWithFormat:@"%@%@", sdk.label, (sdk.beta ? @" BETA" : @"")]];
                [newItem setEnabled:YES];
                // Provide access to the SDK attributes later
                [newItem setRepresentedObject:sdk];
                [[fTargetIOSSDK menu] addItem:newItem];
                [fTargetIOSSDK selectItem:newItem]; // always make sure a valid item is selected
                foundCompatibleSDK = YES;
            }
            else
            {
                [newItem setTitle:[NSString stringWithFormat:@"%@%@ (%@)", sdk.label, (sdk.beta ? @" BETA" : @""), sdk.failMessage]];
                [newItem setEnabled:NO];
                [[fTargetIOSSDK menu] addItem:newItem];
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

            [newItem setTitle:[NSString stringWithFormat:@"%@ (UNSUPPORTED: Local iOS SDK is %@ which does not match)", firstNonBetaSDK.label, defaultIOSSDK]];
            [[fTargetIOSSDK menu] addItem:newItem];
            [fTargetIOSSDK selectItem:newItem];
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

                NSString *informativeText = [NSString stringWithFormat:@"The version of iOS SDK (%@) that's installed on this computer does not match the target iOS version (%@). You may encounter issues building or incompatibilities at runtime.\n\nYou should consider updating your version of Xcode or using xcode-select to enable a compatible version.\n\nDo you want to proceed with a build?", defaultIOSSDK, firstNonBetaSDK.label];
                NSString *message = @"UNSUPPORTED iOS SDK Mismatch";

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
        Rtt_ERROR(("populateTargetIOSSDK: problem parsing '%s': %s", [supportedSDKsFile UTF8String], [[jsonError localizedFailureReason] UTF8String]));
        Rtt_ERROR(("------\n%s\n------", [jsonData bytes]));

        // Make sure the build window is visible so that the error sheet displays correctly
        [buildWindow makeKeyAndOrderFront:nil];

        NSAlert* alert = [[[NSAlert alloc] init] autorelease];
        [alert addButtonWithTitle:@"OK"];

        NSString *message = [NSString stringWithFormat:@"We were unable to parse the list of supported iOS SDKs.  Check the console for more information.\n\nYou should re-install Solar2D."];

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

/*
 - (BOOL) isIOSSDKAvailable:(NSString *)sdkXcodeVersion
 {
	NSString *cmd = @"/usr/bin/xcrun";
	NSArray *args = @[@"--sdk", sdkXcodeVersion, @"--show-sdk-version" ];
	NSTask *task = [[[NSTask alloc] init] autorelease];
 
	[task setLaunchPath:cmd];
	[task setArguments:args];
	[task setStandardError:nil];
	[task launch];
	[task waitUntilExit];
 
	return ([task terminationStatus] == 0);
 }
 */

- (NSString *) defaultIOSSDK
{
    NSString *cmd = @"/usr/bin/xcrun";
    NSArray *args = @[@"--sdk", @"iphoneos", @"--show-sdk-version" ];
    
    return [self launchTaskAndReturnOutput:cmd arguments:args];
}

- (void) commonPostBuildTabOKActions
{

}

- (void) sendAppToDevice
{
	NSString *appBundlePath = [self appPackagePath];
	NSString *deviceInstallationUtility = [XcodeToolHelper pathForIOSDeviceInstallationUtility];

	Rtt_ASSERT(deviceInstallationUtility != nil);

	__block NSString *ideviceinstallerOutput = nil;

	[self setProgressBarLabel:@"Copying app to iOS device…"];

	void (^sendToDeviceBlock)() = ^()
	{
		// The copy is needed because the autoreleased return value gets released in the dancing of the threads
        ideviceinstallerOutput = [[self launchTaskAndReturnOutput:deviceInstallationUtility arguments:@[appBundlePath, @"iPhone OS"]] copy];
	};

	[self runLengthyOperationForWindow:[self window] delayProgressWindowBy:0 allowStop:YES withBlock:sendToDeviceBlock];

	if (appDelegate.stopRequested)
	{
		[self stopLaunchedTask];

		ideviceinstallerOutput = [@"WARNING: installation stopped by request" copy];
	}

	if ([ideviceinstallerOutput isEqualToString:@"ERROR:"])
	{
		// We didn't get anything on stderr so substitute something generic
		ideviceinstallerOutput = [@"ERROR: unexpected error - check the console for more information" copy];
	}

	Rtt_Log("%s", [ideviceinstallerOutput UTF8String]);

	if ([ideviceinstallerOutput contains:@"ERROR:"] || [ideviceinstallerOutput contains:@"Error occurred:"])
	{
		NSString *errorMsg = @"*ideviceinstaller* encountered an error installing the app:\n\n";

		// [self showError:@"App Installation Problem" message:[errorMsg stringByAppendingString:ideviceinstallerOutput] helpURL:nil parentWindow:[self window]];
		[self showModalSheet:@"Device Installation Problem" message:[errorMsg stringByAppendingString:ideviceinstallerOutput] buttonLabels:@[@"OK", @"Retry Installation" ] alertStyle:NSCriticalAlertStyle helpURL:nil parentWindow:[self window] completionHandler: ^(NSModalResponse returnCode)
		 {
			 if (returnCode == NSAlertSecondButtonReturn)
			 {
				 [self performSelector:@selector(sendAppToDevice) withObject:nil afterDelay:0.01];
			 }

			 [NSApp stopModalWithCode:returnCode];
		 }];

		[ideviceinstallerOutput release];

		return;
	}
	else
	{
		[self startTailDeviceSyslog:[XcodeToolHelper pathForIOSDeviceSyslogUtility] appBundlePath:appBundlePath deviceID:@"iPhone OS"];

		NSString *message = [NSString stringWithFormat:@"*%@* is installed on the iOS device and is ready to run\n\nThe device's syslog will appear in the Console until this message is closed (you'll need to *launch* the app on the device before anything appears in the syslog)", self.appName];

		[self showMessage:@"App Installation Complete" message:message helpURL:nil parentWindow:[self window]];

		[self stopTailDeviceSyslog];
	}

	[ideviceinstallerOutput release];

	[self closeBuild:nil];
}

- (void) sendToAppStore:(NSWindow*)parent packager:(IOSAppPackager *)packager params:(IOSAppPackagerParams *)params
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

	[self closeBuild:nil];
}

@end

// ----------------------------------------------------------------------------


