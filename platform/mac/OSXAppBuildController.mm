//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include <sys/sysctl.h>

#import "OSXAppBuildController.h"

#import "AppleSigningIdentityController.h"
#import "ValidationSupportMacUI.h"
#import "ValidationToolOutputViewController.h"

#include "Rtt_MacConsolePlatform.h"
#include "Rtt_Assert.h"
#include "Rtt_MacPlatform.h"
#include "Rtt_MacSimulatorServices.h"

#import "XcodeToolHelper.h"
#import "NSAlert-OAExtensions.h"
#import "TextEditorSupport.h"

using namespace Rtt;

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

static NSString *kValueNotSet = @"not set";
static NSString *kValueYes = @"YES";
static NSString *kValueNo = @"NO";
static NSString *k3rdPartyMacDeveloperIdentityTag = @"3rd Party Mac Developer ";
static NSString *kDeveloperIDIdentityTag = @"Developer ID ";

@implementation OSXAppBuildController

- (id)initWithWindowNibName:(NSString*)nibFile
                projectPath:(NSString *)projPath;
{
	self = [super initWithWindowNibName:nibFile projectPath:projPath];

	if ( self )
	{
		appDelegate = (AppDelegate*)[NSApp delegate];
		signingIdentitiesAdded = NO;
		simulatorServices = new MacSimulatorServices(appDelegate, (CoronaWindowController *)self, nil);
		platformName = @"osx";
		platformTitle = @"macOS";
	}

	return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
}

- (void) showWindow:(id)sender
{
    [super showWindow:sender];
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
		ValidationSupportMacUI *validator = [[[ValidationSupportMacUI alloc] initWithParentWindow:[self window]] autorelease];
		{
			result = [validator runOSXAppNameValidation:[self appName]];
		}
	}

    if (result)
    {
        // Have they selected "Send to App Store"?
        if ([postBuildRadioGroup selectedRow] == 1)
        {
            if (! [self isStoreBuild])
            {
				[self logEvent:@"build-bungled" key:@"reason" value:@"not-distribution-provisioning-profile"];
				
                [self showError:@"Cannot Send To App Store" message:@"Only apps built with distribution profiles can be sent to the App Store.\n\nChoose a provisioning profile signed with an \"3rd Party Mac Developer\" certificate to build for the App Store." helpURL:@"https://docs.coronalabs.com/daily/guide/distribution/osxBuild/index.html#app-signing" parentWindow:[self window]];

                result = NO;
            }
            else
            {
#ifdef USE_APPLICATION_LOADER
                // They can't "Send to App Store" if we don't have iTunes Connect credentials available or the standalone version
                // of Application Loader installed (we actually use the copy of Application Loader embedded in Xcode but the "altool"
                // CLI doesn't work properly unless the standalone version is also installed)
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

                    // Show the .pkg we built earlier in the Finder to make use of Application Loader easier
                    [[NSWorkspace sharedWorkspace] selectFile:[[self appBundleFile] stringByReplacingOccurrencesOfString:@".app" withString:@".pkg"] inFileViewerRootedAtPath:@""];

                    // Make us foreground again
                    [[NSRunningApplication currentApplication] activateWithOptions:(NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps)];

                    [self showMessage:@"Cannot Send To App Store" message:message helpURL:nil parentWindow:[self window]];
                    
                    result = NO;
                }
#endif // USE_APPLICATION_LOADER
            }
        }
    }
    
	return result;
}

- (BOOL)isStoreBuild
{
    IdentityMenuItem* item = currentProvisioningProfileItem; Rtt_ASSERT( item != [fSigningIdentities itemAtIndex:0] );

    return ( ([[item title] contains:k3rdPartyMacDeveloperIdentityTag] || [[item title] contains:kAppleDistributionIdentityTag] )
			|| [[NSUserDefaults standardUserDefaults] boolForKey:@"macOSIgnoreCertType"]);
}

- (BOOL)isSelfDistributionBuild
{
    IdentityMenuItem* item = currentProvisioningProfileItem; Rtt_ASSERT( item != [fSigningIdentities itemAtIndex:0] );

    return ( [[item title] contains:kDeveloperIDIdentityTag]
			|| [[NSUserDefaults standardUserDefaults] boolForKey:@"macOSIgnoreCertType"]);
}

- (BOOL)isDeveloperBuild
{
	return ((! [self isStoreBuild] && ! [self isSelfDistributionBuild])
			|| [[NSUserDefaults standardUserDefaults] boolForKey:@"macOSIgnoreCertType"]);
}

- (void)checkAndWarnARM //Apple Silicon(Arm) computers require signing to test locally
{
    //Check for ARM
    cpu_type_t type;
    size_t size = sizeof(type);
    sysctlbyname("hw.cputype", &type, &size, NULL, 0);
    cpu_type_t typeWithABIInfoRemoved = type & ~CPU_ARCH_MASK;
    IdentityMenuItem* item = currentProvisioningProfileItem;
    
    if (typeWithABIInfoRemoved == CPU_TYPE_ARM  && ![[NSUserDefaults standardUserDefaults] boolForKey:@"macOSARMSignWarn"] && item == [fSigningIdentities itemAtIndex:1])
    {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:@"WARNING: Signing Required to run App"];
        [alert setInformativeText:@"Apple requires Apple Silicon(ARM) based Apps to be signed in order run on Apple Silicon(M1) based Macs"];
        
        [alert addButtonWithTitle:@"Ok"];
        [alert setShowsSuppressionButton:YES];
        [alert runModal];
        
        if(alert.suppressionButton.state == 1){
            [[NSUserDefaults standardUserDefaults] setObject:@(YES) forKey:@"macOSARMSignWarn"];
        }
        
        
    }
    
}

- (void)willShowAlert:(NSAlert*)alert
{
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
	BOOL isOkayToBuild = YES;
#ifdef XCODE_REQUIRED_TO_BUILD_OSX_APPS
	NSString* error_string = nil;
	NSString *sdkRoot = [self developerRoot:&isOkayToBuild errorString:&error_string ];
	Rtt_UNUSED( sdkRoot );

	if ( ! isOkayToBuild )
	{
		NSAlert* alert = [[[NSAlert alloc] init] autorelease];
		NSString *installBtn = @"Install Xcode…";

		NSString *detailMsg  = nil;
		if ( sdkRoot == nil || [sdkRoot isEqualToString:@""] )
		{
			// No Xcode found
			detailMsg = [NSString stringWithFormat:@"The Xcode macOS SDK could not be found. Please install Xcode."];

			// fAnalytics->Log("bad-osx-sdk", "not-found");
		}
		else
		{
			// xcode-select gave us a path, but a component could not be found.
			detailMsg = [NSString stringWithFormat:@"The Xcode macOS SDK located at \"%@\" is missing the following components:\n\n%@\nPlease re-install Xcode.", sdkRoot, error_string];

			// fAnalytics->Log("bad-osx-sdk", "incomplete");
		}

		NSString *msg = [NSString stringWithFormat:@"%@\n\nXcode is required by Corona and needs to be installed to build macOS applications.\n\nPress the \"%@\" button to go to the App Store and get Xcode.  When it is installed, build for macOS again.\n", detailMsg, installBtn];

		[alert setAlertStyle:NSWarningAlertStyle];
		[alert setMessageText:@"Xcode Required"];
		[alert setHelpAnchor:@"https://coronalabs.com/links/simulator/xcode-required"];
		[alert setShowsHelp:YES];
		[alert setDelegate:self];
		[alert setInformativeText:msg];
		[alert addButtonWithTitle:installBtn];
		[alert addButtonWithTitle:@"Cancel Build"];

		NSLog( @"Corona Simulator: %@", msg );

		NSInteger alertResult = [alert runModal];

		if (alertResult == NSAlertFirstButtonReturn)
		{
			// Go to Xcode in the App Store
			[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:kMacAppStoreXcodeURL]];
		}
	}
#endif // XCODE_REQUIRED_TO_BUILD_OSX_APPS

	return isOkayToBuild && [super verifyBuildTools:sender];
}

- (BOOL)shouldRemoveWhitespaceFromPackageName
{
	return NO;
}

- (BOOL)buildFormComplete
{
    BOOL result = ([fSigningIdentities indexOfSelectedItem] > 0);

	if ( result )
	{
        result = [super buildFormComplete];
	}

	return result;
}

- (IBAction)build:(id)sender
{
    BOOL shouldOpenApplication = ([postBuildRadioGroup selectedRow] == 0); // first item in radio group
    BOOL shouldSendToAppStore = ([postBuildRadioGroup selectedRow] == 1);
    BOOL shouldCreateDMG = ([postBuildRadioGroup selectedRow] == 2);
    BOOL shouldShowApplication = ([postBuildRadioGroup selectedRow] == 3);

    [self setProgressBarLabel:@"Building for macOS…"];

    useItunesConnect = shouldSendToAppStore;

	if ( ! [self verifyBuildTools:sender] )
	{
		return;
	}
	
	if ( ! [self shouldInitiateBuild:sender] )
	{
		return;
	}
    
    if( [self isDeveloperBuild] ){
        [self checkAndWarnARM];
    }

	NSWindow *buildWindow = [self window];

	if ( ! [self buildFormComplete] )
    {
        return; // this shouldn't ever happen
    }
	
	if ( ! [self validateProject] )
	{
		[self logEvent:@"build-bungled" key:@"reason" value:@"validate-project"];
		return;
	}

	// We don't login to build macOS apps so we output this here
	Rtt_Log("Building %s app with %s", [self.platformTitle UTF8String], Rtt_STRING_BUILD);
	
	MacConsolePlatform platform; // TODO: Use fConsolePlatform
	MacPlatformServices services( platform );

	const char* name = [self.appName UTF8String];
	const char* versionname = NULL;
	versionname = [self.appVersion UTF8String];

	const char* dstDir = [self.dstPath UTF8String];
	const char* provisionFile = NULL;
	const char* identity = NULL;
	const char* identityName = NULL;

	TargetDevice::Version targetVersion = TargetDevice::kUnknownVersion;
	bool isDistributionBuild = false;
	bool isStripDebug = true; // default to stripping

	IdentityMenuItem* item = (IdentityMenuItem*)[fSigningIdentities selectedItem];

	if ( item != [fSigningIdentities itemAtIndex:0] && item != [fSigningIdentities itemAtIndex:1] )
	{
		identityName = [item.identity UTF8String];
		identity = [item.fingerprint UTF8String];
		provisionFile = [item.provisionPath UTF8String];

		if ( [[item title] hasPrefix:@"MacApp Distribution"] )
		{
			// Here, we mean distribution build, as in store build,
			// so a store build is anything that does NOT have provisioned devices
			isDistributionBuild = ! [AppleSigningIdentityController hasProvisionedDevices:item.provisionPath];
		}
		else
		{
			// Only preserve debug info in Lua for pure dev builds
			isStripDebug = false;
		}
	}

    if (shouldOpenApplication)
    {
        // Stop any currently running copy of the application we are about to build (sometimes there
        // are Lua thrown as the code is overwritten)
        
        [self stopBuiltApplication];
    }

    // Remember the chosen cert for next time we need a default
    [[NSUserDefaults standardUserDefaults] setObject:[currentProvisioningProfileItem title] forKey:kUserPreferenceLastOSXCertificate];

	// TODO: Bogus magic number to mean undefined
	S32 targetDevice = -1;

	OSXAppPackagerParams * params = NULL;
	const char * srcDir = [self.projectPath UTF8String];

	const char* apppackage = "NULL";

	OSXAppPackager *osxPackager = new OSXAppPackager( services, simulatorServices );

	bool isvalidsettings = osxPackager->ReadBuildSettings( srcDir );

	// Abort build if the build.settings is corrupt
	if ( ! isvalidsettings )
	{
		[self logEvent:@"build-bungled" key:@"reason" value:@"bad-build-settings"];

        NSString *buildSettingsError = [NSString stringWithExternalString:osxPackager->GetErrorMesg()];

        Rtt_DELETE( osxPackager );
        osxPackager = NULL;

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

	const char * customBuildId = osxPackager->GetCustomBuildId();

	// targetDevice = (S32) [[fSupportedDevices selectedItem] tag]; // the tags of the NSPopupButton are set to match our TargetDevice constants

	apppackage = osxPackager->GetBundleId( provisionFile, name );

	if (apppackage == NULL || strlen(apppackage) == 0)
	{
		Rtt_DELETE( osxPackager );
		osxPackager = NULL;

        [self showError:@"Build Problem" message:@"Cannot determine a valid application id for this application.\n\nPlease check you are using the correct Provisioning Profile and try again." helpURL:nil parentWindow:buildWindow];

        [self closeBuild:self];
	}

	params = new OSXAppPackagerParams(
									  name,
									  versionname,
									  identity,
									  provisionFile,
									  srcDir,
									  dstDir,
									  [xCodeSdkRoot UTF8String],
									  TargetDevice::kOSXPlatform,
									  targetVersion,
									  targetDevice,
									  customBuildId,
									  NULL,
									  apppackage,
									  isDistributionBuild );

	params->SetStripDebug( isStripDebug );
	params->SetRuntime([appDelegate runtime]);

	// Some IDEs will terminate us quite abruptly so make sure we're on disk before starting a long operation
	[[NSUserDefaults standardUserDefaults] synchronize];

	// Do the actual build
	[self logEvent:@"build"];

	NSString* tmpDirBase = NSTemporaryDirectory();
	size_t code = osxPackager->Build( params, [tmpDirBase UTF8String] );

	[self endProgressSheetBanner:[self window]];

	if (code == PlatformAppPackager::kNoError)
	{
		[self logEvent:@"build-succeeded"];

		[appDelegate notifyWithTitle:@"Corona Simulator"
						 description:[NSString stringWithFormat:@"macOS build of \"%@\" complete", self.appName]
							iconData:nil];

		if (shouldOpenApplication)
		{
			[self logEvent:@"build-post-action" key:@"post-action" value:@"open-application"];

			[self openBuiltApplication];

			[self closeBuild:self];
		}
		else if (shouldSendToAppStore)
		{
			[self logEvent:@"build-post-action" key:@"post-action" value:@"send-to-app-store"];

			[self beginAppStorePackageSheet:[self window] packager:osxPackager params:params];
		}
        else if (shouldCreateDMG)
        {
			[self logEvent:@"build-post-action" key:@"post-action" value:@"create-dmg"];

            [self beginSelfDistPackageSheet:[self window] packager:osxPackager params:params];
        }
        else if (shouldShowApplication)
        {
			[self logEvent:@"build-post-action" key:@"post-action" value:@"show-app"];

            // Reveal built app in Finder

            NSString *message = [NSString stringWithFormat:@"Showing built macOS app *%@* in Finder", self.appName];

            [[NSWorkspace sharedWorkspace] selectFile:[self appBundleFile] inFileViewerRootedAtPath:@""];

            [self showMessage:@"Build Complete" message:message helpURL:nil parentWindow:[self window]];

            [self closeBuild:self];
		}
		else
		{
			[self logEvent:@"build-post-action" key:@"post-action" value:@"do-nothing"];

			[self closeBuild:self];
		}
	}
    else
    {
		[self logEvent:@"build-failed" key:@"reason" value:[NSString stringWithFormat:@"[%ld] %s", code, params->GetBuildMessage()]];

        NSDictionary* details = nil;
        NSError* error = nil;
        NSString *msg = nil;

		[appDelegate notifyWithTitle:@"Corona Simulator"
						 description:[NSString stringWithFormat:@"Error building \"%@\" for macOS", self.appName]
							iconData:nil];

        // Display the error
        if (params->GetBuildMessage() != NULL)
        {
            msg = [NSString stringWithExternalString:params->GetBuildMessage()];
        }
        else
        {
            msg = [NSString stringWithFormat:@"Unexpected build error.\n\nError code: %ld", code];
        }
        [self showError:@"Build Failed" message:msg helpURL:nil parentWindow:[self window]];
    }

    [self saveBuildPreferences];
}

- (NSError *)willPresentError:(NSError *)error
{
    return [super willPresentError:error];
}

- (void)didPresentErrorWithRecovery:(BOOL)didRecover contextInfo:(void  *)contextInfo
{
    
}

- (void) stopBuiltApplication
{
    @try
    {
        if ([fOpenApplicationTask isRunning])
        {
            [fOpenApplicationTask terminate];
            [fOpenApplicationTask release];
        }
    }
    @catch( NSException* exception )
    {
        NSLog( @"stopBuiltApplication: exception %@", exception );
    }
}

- (void) openBuiltApplication
{
    NSArray *args = [NSArray arrayWithObjects:@"-makeForeground", kValueYes, nil];
    NSString *launchApp = [self appPackagePath];
    //NSPipe *stdoutPipe = [NSPipe pipe];
    //NSPipe *stderrPipe = [NSPipe pipe];
    NSFileHandle *stdoutHandle = [[[NSFileHandle alloc] initWithFileDescriptor:fileno(stdout)] autorelease];
    
    // Stop any previously started task
    [self stopBuiltApplication];
    
    // NSTask wants the executable in the bundle
    NSString *launchPath = [[launchApp stringByAppendingPathComponent:@"Contents/MacOS"] stringByAppendingPathComponent:[[launchApp lastPathComponent] stringByDeletingPathExtension]];
    
    fOpenApplicationTask = [[[NSTask alloc] init] retain];
    
    [fOpenApplicationTask setLaunchPath:launchPath];
    [fOpenApplicationTask setArguments:args];

    // Allow the Simulator to capture the app's output
    [fOpenApplicationTask setStandardOutput:stdoutHandle];
    [fOpenApplicationTask setStandardError:stdoutHandle];
    [fOpenApplicationTask setTerminationHandler:^(NSTask *) {
        Rtt_Log("---------- Finished: %s (exit code %d) ----------", [[launchApp stringByAbbreviatingWithTildeInPath] UTF8String], [fOpenApplicationTask terminationStatus]);
    }];
    
    @try
    {
        Rtt_Log("---------- Launching: %s ----------", [[launchApp stringByAbbreviatingWithTildeInPath] UTF8String]);
        
        [fOpenApplicationTask launch];
    }
    @catch( NSException* exception )
    {
        NSLog( @"openBuiltApplication: exception %@ (%@)", exception, launchPath );
        
        [fOpenApplicationTask release];
    }
}

- (TargetDevice::Platform)targetPlatform
{
	return TargetDevice::kOSXPlatform;
}

- (void) beginAppStorePackageSheet:(NSWindow*)parent packager:(OSXAppPackager *) packager params:(OSXAppPackagerParams *) params
{
    // Save for later
    savedPackager = packager;
    savedParams = params;

    if ( appStorePackageSheet == nil )
    {
        [NSBundle loadNibNamed:@"OSXAppPackageSheet" owner:self]; Rtt_ASSERT( appStorePackageSheet );
        [appStorePackageSheet setReleasedWhenClosed:NO];
    }

    // We have 2 popup menus of signing identities to populate
    [appStoreAppSigningIdentityPopup setAutoenablesItems:NO];
    [appStoreInstallerSigningIdentityPopup setAutoenablesItems:NO];

    if (! signingIdentitiesAdded)
    {
        NSDictionary *signingIdentities = [AppleSigningIdentityController getSigningIdentities];

        NSArray *keys = [signingIdentities allKeys];
        NSArray *sortedKeys = [keys sortedArrayUsingComparator:^NSComparisonResult(id a, id b)
                               {
                                   //NSString *first = [signingIdentities objectForKey:a];
                                   //NSString *second = [signingIdentities objectForKey:b];
                                   return [a caseInsensitiveCompare:b];
                               }];

        for (NSString *certName in sortedKeys)
        {
            if ([certName hasPrefix:k3rdPartyMacDeveloperIdentityTag]
				|| [[NSUserDefaults standardUserDefaults] boolForKey:@"macOSIgnoreCertType"])
            {
                NSMenuItem *newItem = [[NSMenuItem alloc] init];
                [newItem setTitle:certName];
                [newItem setEnabled:YES];
                [newItem setRepresentedObject:[signingIdentities objectForKey:certName]];
                [[appStoreAppSigningIdentityPopup menu] addItem:newItem];
                if ([certName hasPrefix:@"3rd Party Mac Developer Application: "])
                {
                    [appStoreAppSigningIdentityPopup selectItem:newItem];
                }
            }

            if ([certName hasPrefix:k3rdPartyMacDeveloperIdentityTag]
				|| [[NSUserDefaults standardUserDefaults] boolForKey:@"macOSIgnoreCertType"])
            {
                NSMenuItem *newItem = [[NSMenuItem alloc] init];
                [newItem setTitle:certName];
                [newItem setEnabled:YES];
                [newItem setRepresentedObject:[signingIdentities objectForKey:certName]];
                [[appStoreInstallerSigningIdentityPopup menu] addItem:newItem];
                if ([certName hasPrefix:@"3rd Party Mac Developer Installer: "])
                {
                    [appStoreInstallerSigningIdentityPopup selectItem:newItem];
                }
            }
        }

        // Retrieve any previous settings for the code signing identities from the user's preferences
        NSString *savedAppStoreAppChoice = [appDelegate restoreAppSpecificPreference:@"osxAppStoreAppSigningID" defaultValue:kValueNotSet];
        if (! [savedAppStoreAppChoice isEqualToString:kValueNotSet])
        {
            [appStoreAppSigningIdentityPopup selectItemWithTitle:savedAppStoreAppChoice];
        }
        NSString *savedAppStoreInstallerChoice = [appDelegate restoreAppSpecificPreference:@"osxAppStoreInstallerSigningID" defaultValue:kValueNotSet];
        if (! [savedAppStoreInstallerChoice isEqualToString:kValueNotSet])
        {
            [appStoreInstallerSigningIdentityPopup selectItemWithTitle:savedAppStoreInstallerChoice];
        }

        signingIdentitiesAdded = YES;
    }
    
    if ([appStoreSendButton isEnabled])
    {
        [appStoreSendButton setState:([[appDelegate restoreAppSpecificPreference:@"osxAppStoreSend" defaultValue:kValueNotSet] isEqualToString:kValueYes] ? NSOnState : NSOffState)];
    }
    
    [[self window] beginSheet:appStorePackageSheet completionHandler:nil];
}

- (void) beginSelfDistPackageSheet:(NSWindow*)parent packager:(OSXAppPackager *) packager params:(OSXAppPackagerParams *) params
{
    // Save for later
    savedPackager = packager;
    savedParams = params;

    if ( selfDistPackageSheet == nil )
    {
        [NSBundle loadNibNamed:@"OSXCreateDMGSheet" owner:self]; Rtt_ASSERT( selfDistPackageSheet );
        [selfDistPackageSheet setReleasedWhenClosed:NO];
    }

    // We have 1 popup menu of signing identities to populate
    [selfDistAppSigningIdentityPopup setAutoenablesItems:NO];

    if (! signingIdentitiesAdded)
    {
        NSDictionary *signingIdentities = [AppleSigningIdentityController getSigningIdentities];

        NSArray *keys = [signingIdentities allKeys];
        NSArray *sortedKeys = [keys sortedArrayUsingComparator:^NSComparisonResult(id a, id b)
                               {
                                   //NSString *first = [signingIdentities objectForKey:a];
                                   //NSString *second = [signingIdentities objectForKey:b];
                                   return [a caseInsensitiveCompare:b];
                               }];

        for (NSString *certName in sortedKeys)
        {
            if ([certName hasPrefix:kDeveloperIDIdentityTag])
            {
                NSMenuItem *newItem = [[NSMenuItem alloc] init];
                [newItem setTitle:certName];
                [newItem setEnabled:YES];
                [newItem setRepresentedObject:[signingIdentities objectForKey:certName]];
                [[selfDistAppSigningIdentityPopup menu] addItem:newItem];
            }
        }

        // Retrieve any previous settings for the code signing identities from the user's preferences
        NSString *savedSelfDistAppChoice = [appDelegate restoreAppSpecificPreference:@"osxSelfDistAppSigningID" defaultValue:kValueNotSet];
        if (! [savedSelfDistAppChoice isEqualToString:kValueNotSet])
        {
            [selfDistAppSigningIdentityPopup selectItemWithTitle:savedSelfDistAppChoice];
        }
        
        signingIdentitiesAdded = YES;
    }
    
    if ([appStoreSendButton isEnabled])
    {
        [appStoreSendButton setState:([[appDelegate restoreAppSpecificPreference:@"osxAppStoreSend" defaultValue:kValueNotSet] isEqualToString:kValueYes] ? NSOnState : NSOffState)];
    }
    
    [[self window] beginSheet:selfDistPackageSheet completionHandler:nil];
}


- (void) endAppStorePackageSheet
{
    [[self window] endSheet:appStorePackageSheet];
}

- (void) endSelfDistPackageSheet
{
    [[self window] endSheet:selfDistPackageSheet];
}

- (void) commonPostBuildOKActions
{
    [self closeBuild:self];
}

- (IBAction)appStoreOKClicked:(id)sender
{
    [self endAppStorePackageSheet];

	if ([appStoreSendButton state] == NSOnState)
	{
		[self setProgressBarLabel:@"Creating and sending Mac App Store package…"];
	}
	else
	{
		[self setProgressBarLabel:@"Creating Mac App Store package…"];
	}

    [self beginProgressSheetBanner:[self window] withDelay:0];

	savedParams->SetAppSigningIdentity( [[[appStoreAppSigningIdentityPopup selectedItem] representedObject] UTF8String] );
	savedParams->SetInstallerSigningIdentity( [[[appStoreInstallerSigningIdentityPopup selectedItem] representedObject] UTF8String] );

#ifdef USE_APPLICATION_LOADER
	size_t code = savedPackager->PackageForAppStore( savedParams, ([appStoreSendButton state] == NSOnState),
							[itunesConnectUsername UTF8String], [itunesConnectPassword UTF8String]);
#else
	// These params stop the Lua from from trying to use Application Loader
	size_t code = savedPackager->PackageForAppStore( savedParams, false, "", "");
#endif // USE_APPLICATION_LOADER

	[self endProgressSheetBanner:[self window]];

	if (code != PlatformAppPackager::kNoError)
	{
		NSString *buildMsg = nil;
		NSString *title = nil;

		if (savedParams->GetBuildMessage() != NULL)
		{
			buildMsg = [NSString stringWithExternalString:savedParams->GetBuildMessage()];
		}
		else
		{
			buildMsg = [NSString stringWithFormat:@"Unexpected build result %ld", code];
		}

		title = @"Build Error";

		if ([buildMsg contains:@"Transporter not found at path"])
		{
			buildMsg = [buildMsg stringByAppendingString:@"\n\nThis may be an Xcode issue which is most easily worked around by opening [Application Loader](launch-bundle:com.apple.itunes.connect.ApplicationLoader) and managing your app manually."];

			// Show the .pkg we built earlier in the Finder to make use of Application Loader easier
			[[NSWorkspace sharedWorkspace] selectFile:[[self appBundleFile] stringByReplacingOccurrencesOfString:@".app" withString:@".pkg"] inFileViewerRootedAtPath:@""];

			// Make us foreground again
			[[NSRunningApplication currentApplication] activateWithOptions:(NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps)];
		}

		[self showError:title message:buildMsg helpURL:nil parentWindow:[self window]];
	}
	else
	{
#ifdef USE_APPLICATION_LOADER
		NSString *message = [NSString stringWithFormat:@"*%@* has been sent to the Mac App Store", self.appName];
#else
		NSString *message;
		if([[NSWorkspace sharedWorkspace] URLForApplicationToOpenURL:[NSURL URLWithString:@"transporter://"]])
		{
			message = [NSString stringWithFormat:@"*%@* is ready to be sent to the App Store using the [Transporter](launch-bundle:com.apple.TransporterApp|macappstore://itunes.apple.com/app/id1450874784) app.\n\nPress *Add App* on the *Transporter* window to load `%@` into it", self.appName, [[self appBundleFile] stringByReplacingOccurrencesOfString:@".app" withString:@".pkg"]];
		}
		else
		{
			message = [NSString stringWithFormat:@"*%@* is ready to be sent to the App Store. Install and run the [Transporter](launch-bundle:com.apple.TransporterApp|macappstore://itunes.apple.com/app/id1450874784) app.\n\nAfter signing into *Transporter* app, press *Add App* on its window to load `%@` into it", self.appName, [[self appBundleFile] stringByReplacingOccurrencesOfString:@".app" withString:@".pkg"]];
		}

		// Open Application Loader
		[[NSWorkspace sharedWorkspace] launchAppWithBundleIdentifier:@"com.apple.TransporterApp"
															 options:NSWorkspaceLaunchDefault
									  additionalEventParamDescriptor:nil
													launchIdentifier:nil];

		// Show the .ipa we built in the Finder to make use of Application Loader easier
		[[NSWorkspace sharedWorkspace] selectFile:[[self appBundleFile] stringByReplacingOccurrencesOfString:@".app" withString:@".pkg"] inFileViewerRootedAtPath:@""];

		// Make us foreground again
		[[NSRunningApplication currentApplication] activateWithOptions:(NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps)];

#endif // USE_APPLICATION_LOADER

		[self showMessage:@"Build Complete" message:message helpURL:nil parentWindow:[self window]];

		[self commonPostBuildOKActions];
	}

    // Save the preferences even if there's an error as submitting to the App Store is often a multi-shot process
    [appDelegate saveAppSpecificPreference:@"osxAppStoreAppSigningID" value:[[appStoreAppSigningIdentityPopup selectedItem] title]];
    [appDelegate saveAppSpecificPreference:@"osxAppStoreInstallerSigningID" value:[[appStoreInstallerSigningIdentityPopup selectedItem] title]];
    [appDelegate saveAppSpecificPreference:@"osxAppStoreSend" value:([appStoreSendButton state] == NSOnState ? kValueYes : kValueNo)];
}

- (IBAction)selfDistOKClicked:(id)sender
{
    [self endSelfDistPackageSheet];

    [self setProgressBarLabel:@"Creating distributable DMG…"];

    [self beginProgressSheetBanner:[self window] withDelay:0];

	savedParams->SetAppSigningIdentity( [[selfDistAppSigningIdentityPopup titleOfSelectedItem] UTF8String] );

	size_t code = savedPackager->PackageForSelfDistribution( savedParams, true );

	[self endProgressSheetBanner:[self window]];

	if (code != PlatformAppPackager::kNoError)
	{
        NSString *buildMsg = nil;

        if (savedParams->GetBuildMessage() != NULL)
        {
            buildMsg = [NSString stringWithExternalString:savedParams->GetBuildMessage()];
        }
        else
        {
            buildMsg = [NSString stringWithFormat:@"Unexpected error code %ld", code];
        }

        [self showError:@"Build Error" message:[self limitString:buildMsg toLength:1500] helpURL:nil parentWindow:[self window]];
	}
	else
	{
        NSString *message = [NSString stringWithFormat:@"*%@* has been packaged in a DMG", self.appName];

        [self showMessage:@"Build Complete" message:message helpURL:nil parentWindow:[self window]];

        [self commonPostBuildOKActions];
	}

    [appDelegate saveAppSpecificPreference:@"osxSelfDistAppSigningID" value:[[appStoreAppSigningIdentityPopup selectedItem] title]];
}

- (IBAction)appStoreCancelClicked:(id)sender
{
    if (appStorePackageSheet != nil)
    {
        [self endAppStorePackageSheet];
    }
}

- (IBAction)selfDistCancelClicked:(id)sender
{
    if (selfDistPackageSheet != nil)
    {
        [self endSelfDistPackageSheet];
    }
}

- (void) saveBuildPreferences
{
    [super saveBuildPreferences];

    // Add any platform specific preferences here
}

@end

// ----------------------------------------------------------------------------
