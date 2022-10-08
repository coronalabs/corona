//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "AndroidAppBuildController.h"

#import "ValidationSupportMacUI.h"
#import "ValidationToolOutputViewController.h"

#import "Rtt_AndroidAppPackager.h"
#import "Rtt_MacPlatform.h"
#import "Rtt_MacConsolePlatform.h"
#import "TextEditorSupport.h"
#import "XcodeToolHelper.h"

#include "Rtt_TargetAndroidAppStore.h"
#include "Rtt_JavaHost.h"
#include "AntHost.h"

#include "ListKeyStore.h"
#include "Rtt_MacDialogController.h"

using namespace Rtt;

// ----------------------------------------------------------------------------

static NSString* kAndroidDebugKeystoreTitle = @"Debug";
static NSString* kAndroidDebugKeystorePassword = @"android";
static NSString *kValueNotSet = @"not set";
//static NSString *kValueYes = @"YES";
//static NSString *kValueNo = @"NO";
static NSString *kChooseFromFollowing = @"Choose from the following…";

// ----------------------------------------------------------------------------

@implementation AndroidAppBuildController {
	MacConsolePlatform platform;
	MacPlatformServices *platformServices;
}

@synthesize androidAppPackage;
@synthesize appVersionCode;
@synthesize androidKeyAlias;
@synthesize androidKeyAliasPassword;
@synthesize targetStoreId;

- (id)initWithWindowNibName:(NSString*)nibFile
                projectPath:(NSString *)projPath
{
	self = [super initWithWindowNibName:nibFile projectPath:projPath];

	if ( self )
	{
		platformServices = new MacPlatformServices( platform );
		platformName = @"android";
		platformTitle = @"Android";

		androidAppPackage = nil;
        appVersionCode = 0;
		fAndroidPasswordController = nil;
		fAndroidAliasPasswordController = nil;
	}

	return self;
}

- (void)dealloc
{
	delete platformServices;
	[super dealloc];
}

- (void)windowDidLoad
{
    [super windowDidLoad];

    // Make sure the build window is visible early so that any error sheets display correctly
    [[self window] makeKeyAndOrderFront:nil];

    [fSigningIdentitiesAndroid setAutoenablesItems:NO];

    NSMenu* menu = [fSigningIdentitiesAndroid menu];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(didSelectAlias:)
                                                 name:NSMenuDidSendActionNotification
                                               object:menu];

    [self initKeystore];

    [self setKeystore:nil];
}

- (void) showWindow:(id)sender;
{
    [super showWindow:sender];

	[self initTargetStore];

	TargetDevice::Platform targetPlatform = [self getPlatformFromMenuSelection];

	[self window].title = @"Build for Android";
	if ( TargetDevice::kKindlePlatform == targetPlatform )
	{
		[self window].title = @"Build for Amazon/Kindle";
	}
	else if ( TargetDevice::kNookPlatform == targetPlatform )
	{
		[self window].title = @"Build for Nook";
	}

	if ( [self.androidAppPackage length] == 0 )
	{
		// Generate a default package id based on the user's email address + the app name
		NSString *tmpPackageName = [@"com.coronalabs." stringByAppendingString:[[self.appName componentsSeparatedByCharactersInSet:[[NSCharacterSet alphanumericCharacterSet] invertedSet]] componentsJoinedByString:@"_"]];

		self.androidAppPackage = tmpPackageName;
	}

    self.appVersionCode = [NSNumber numberWithInt:[[appDelegate restoreAppSpecificPreference:@"androidAppVersionCode" defaultValue:@"1"] intValue]];

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
}

- (BOOL) buildFormComplete
{
	if (self.appVersionCode == nil)
	{
		[self showError:@"Missing Version Code" message:@"Enter a numeric version code for the application (aka internal build id [android:versionCode])" helpURL:nil parentWindow:[self window]];

		return NO;
	}

	if (self.appVersion == nil || [self.appVersion isEqualToString:@""])
	{
		[self showError:@"Missing Version Name" message:@"Enter a version name for the application (aka user visible version id [android:versionName])" helpURL:nil parentWindow:[self window]];

		return NO;
	}

	if (self.androidAppPackage == nil || [self.androidAppPackage isEqualToString:@""])
	{
		[self showError:@"Missing Package Name" message:@"Enter a package name for the application\n\nA full Java-language-style package name for the application. The name should be unique. The name may contain uppercase or lowercase letters ('A' through 'Z'), numbers, and underscores ('_'). However, individual package name parts may only start with letters [package]" helpURL:nil parentWindow:[self window]];

		return NO;
	}

	if ([fSigningIdentitiesAndroid indexOfSelectedItem] == 0)
	{
		[self showError:@"Missing Key Alias" message:@"Choose a key alias in the keystore to identify the desired certificate pair" helpURL:nil parentWindow:[self window]];

		return NO;
	}

	return [super buildFormComplete];
}

- (IBAction)build:(id)sender
{
	BOOL androidSDKAccepted = [[NSUserDefaults standardUserDefaults] boolForKey:@"AndroidSDKAccepted"];
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
	NSString *androidSDKLicensePath = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"Corona/Android Build/sdk/licenses/android-sdk-license"];
	BOOL androidSDKLicenseExists = [[NSFileManager defaultManager] fileExistsAtPath:androidSDKLicensePath];

	if(!androidSDKLicenseExists || !androidSDKAccepted) {
		NSModalResponse agreedmentRet = [self showModalSheet:@"Building Android App" message:@"You are about to build an Android app for the first time with this build system.\nIt requires that the Android SDK is installed locally and you must read and accept its [license agreement](https://developer.android.com/studio/terms) in order to proceed.\nNote, the first time it will download about 250Mb which can take several minutes." buttonLabels:@[@"I Accept", @"Cancel"] alertStyle:NSWarningAlertStyle helpURL:@"https://developer.android.com/studio/terms" parentWindow:[self window] completionHandler:nil];
		if(agreedmentRet == NSAlertSecondButtonReturn) return;
		[[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"AndroidSDKAccepted"];
	}
	
    MacConsolePlatform platform;
    MacPlatformServices *services = new MacPlatformServices( platform );
    BOOL shouldSendToDevice = ([postBuildRadioGroup selectedRow] == 0); // first item in radio group
    BOOL shouldShowApplication = ([postBuildRadioGroup selectedRow] == 1);
	BOOL isLiveBuild = (fEnableLiveBuild.state == NSOnState);

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

	const char *package = [self.androidAppPackage UTF8String];
	NSString* script = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"AndroidValidation.lu"];
	if ( ! ValidationSupportMacUI_ValidateAndroidPackageName(package, [script fileSystemRepresentation]))
	{
		[self showError:@"Invalid Android Package Identifier" message:[NSString stringWithFormat:@"\"*%@*\" is not a valid Android package identifier.\n\nThe *Package* identifier must be a full Java-language-style package name for the application. The name should be unique. The name may contain uppercase or lowercase letters ('A' through 'Z'), numbers, and underscores ('_'). However, individual package name parts may only start with letters.", self.androidAppPackage] helpURL:nil parentWindow:[self window]];

		[self logEvent:@"build-bungled" key:@"reason" value:@"bad-package-identifier"];

		return;
	}

    [self setProgressBarLabel:@"Authorizing Android build…"];

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
    const char* provisionFile = NULL;
    const char* identity = NULL;
    Rtt::TargetDevice::Version targetVersion;
    bool isDistributionBuild = false;
    bool isStripDebug = true; // default to stripping

    NSMenuItem * menuItem = [fSigningIdentitiesAndroid selectedItem];
    identity = [[menuItem title] UTF8String];

    targetVersion = Rtt::TargetDevice::VersionForPlatform(Rtt::TargetDevice::kAndroidPlatform);


    NSMenuItem *keystoreItem = [fAndroidKeystore selectedItem];
    if ( ! [[keystoreItem title] isEqualToString:kAndroidDebugKeystoreTitle] )
    {
        isDistributionBuild = true;
    }
    else
    {
        // Only preserve debug info in Lua for pure dev builds
        isStripDebug = false;
    }

    NSString *keystorePath = [self keystorePath];
    NSString *keystorePassword = [self keystorePassword];
    NSMenuItem *targetStartMenuItem = [fTargetStore selectedItem];
    self.targetStoreId = [targetStartMenuItem representedObject];
    Rtt::TargetAndroidAppStore *pLastSelectedStore = Rtt::TargetAndroidAppStore::GetByStringId([self.targetStoreId UTF8String]);
    TargetDevice::Platform targetPlatform = pLastSelectedStore->GetPlatform();

    [appDelegate saveAppSpecificPreference:kUserPreferenceLastAndroidTargetStore value:self.targetStoreId];

    NSMenuItem *keyAliasItem = [fSigningIdentitiesAndroid selectedItem];
    self.androidKeyAlias = [keyAliasItem title];

    // Remember the chosen keystore and cert for next time
    [[NSUserDefaults standardUserDefaults] setObject:[[fAndroidKeystore selectedItem] title] forKey:kUserPreferenceLastAndroidKeystore];
    [[NSUserDefaults standardUserDefaults] setObject:self.androidKeyAlias forKey:kUserPreferenceLastAndroidKeyAlias];

    U32 versioncode = 1;
    // The number formatter for Android seems to be causing the value to be returned to be a NSNumber instead of NSString.
    // We must react accordingly.
    if( [self.appVersionCode isKindOfClass:[NSNumber class]] )
    {
        versioncode = [(NSNumber*)self.appVersionCode intValue];
    }
    else if( [self.appVersionCode isKindOfClass:[NSString class]] )
    {
        // Assume NSString?
        versioncode = [self.appVersionCode intValue];
    }

    Rtt::String resourcesDir;
    PlatformAppPackager *androidPackager = NULL;
    AppPackagerParams * params = NULL;
    const char * srcDir = [self.projectPath UTF8String];

    platform.PathForFile( NULL, Rtt::MPlatform::kResourceDir, Rtt::MPlatform::kDefaultPathFlags, resourcesDir );

    androidPackager = new AndroidAppPackager( *services, resourcesDir.GetString() );

    bool isvalidsettings = androidPackager->ReadBuildSettings( srcDir );

    // Abort build if the build.settings is corrupt
    if ( ! isvalidsettings )
    {
		[self logEvent:@"build-bungled" key:@"reason" value:@"bad-build-settings"];

        NSString *buildSettingsError = [NSString stringWithExternalString:androidPackager->GetErrorMesg()];

        Rtt_DELETE( androidPackager );
        androidPackager = NULL;

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

    const char * customBuildId = androidPackager->GetCustomBuildId();

    Rtt_ASSERT(
               TargetDevice::kAndroidPlatform == targetPlatform
               || TargetDevice::kKindlePlatform == targetPlatform
               || TargetDevice::kNookPlatform == targetPlatform );

    params = new AndroidAppPackagerParams(
                                          name,
                                          versionname,
                                          identity,
                                          provisionFile,					// TODO: remove me
                                          srcDir,
                                          dstDir,
                                          "",  // TODO: sdkRoot needed?
                                          targetPlatform,
                                          [self.targetStoreId UTF8String],
                                          targetVersion,
                                          customBuildId,
                                          NULL,
                                          [self.androidAppPackage UTF8String],
                                          isDistributionBuild,
                                          [keystorePath UTF8String],
                                          [keystorePassword UTF8String],
                                          [self.androidKeyAlias UTF8String],
                                          [androidKeyAliasPassword UTF8String],
                                          versioncode
                                          );

    params->SetStripDebug( isStripDebug );
	params->SetLiveBuild(isLiveBuild);

#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
    // If "debugMonetizationPlugin" is set, honor the setting of the "Enable Monetization" checkbox
    NSString *debugMonetizationPluginStr = [[NSUserDefaults standardUserDefaults] stringForKey:@"debugMonetizationPlugin"];
    if ([debugMonetizationPluginStr boolValue])
    {
        BOOL includeFusePlugins = ([fEnableMonetization state] == NSOnState);

        params->SetIncludeFusePlugins( includeFusePlugins );
        params->SetUsesMonetization( includeFusePlugins );
    }
#endif // AUTO_INCLUDE_MONETIZATION_PLUGIN

	NSString *kBuildSettings = @"build.settings";
	params->SetBuildSettingsPath( [[self.projectPath stringByAppendingPathComponent:kBuildSettings] UTF8String]);

    [self setProgressBarLabel:@"Building for Android…"];

    // Some IDEs will terminate us quite abruptly so make sure we're on disk before starting a long operation
    [[NSUserDefaults standardUserDefaults] synchronize];

    // Do the actual build
    __block size_t code = PlatformAppPackager::kNoError;

	[self logEvent:@"build" key:@"store" value:self.targetStoreId];

    void (^performBuild)() = ^()
    {
        NSString* tmpDirBase = NSTemporaryDirectory();
        code = androidPackager->Build( params, [tmpDirBase UTF8String] );
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
                         description:[NSString stringWithFormat:@"Android build of \"%@\" complete", self.appName]
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
        else if (shouldShowApplication)
        {
            // Reveal built app in Finder
			[self logEvent:@"build-post-action" key:@"post-action" value:@"show-app"];

            NSString *message = [NSString stringWithFormat:@"Showing built Android app *%@* in Finder", self.appName];

            [[NSWorkspace sharedWorkspace] selectFile:[self appBundleFile] inFileViewerRootedAtPath:@""];

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

        NSString *msg = @"";

        [appDelegate notifyWithTitle:@"Corona Simulator"
                         description:[NSString stringWithFormat:@"Error building \"%@\" for Android", self.appName]
                            iconData:nil];

        // Display the error
        if (params->GetBuildMessage() != NULL)
        {
            msg = [NSString stringWithFormat:@"%s\n\n", params->GetBuildMessage()];
        }

		msg = [msg stringByAppendingFormat:@"Error code: %ld", code];

        [self showError:@"Build Failed" message:msg helpURL:nil parentWindow:[self window]];
    }

    [self saveBuildPreferences];
}

- (void) sendAppToDevice
{
    NSString *appBundlePath = [self appPackagePath];
    NSString *deviceInstallationUtility = [XcodeToolHelper pathForAndroidDeviceInstallationUtility];

	__block NSString *adbOutput = nil;

	[self setProgressBarLabel:@"Copying app to Android device…"];
	[self beginProgressSheetBanner:[self window] withDelay:0.5];

	void (^sendToDeviceBlock)() = ^()
	{
		// The copy is needed because the autoreleased return value gets released in the dancing of the threads
		adbOutput = [[self launchTaskAndReturnOutput:deviceInstallationUtility arguments:@[@"-r", appBundlePath]] copy];
	};

	[self runLengthyOperationForWindow:[self window] delayProgressWindowBy:0 allowStop:YES withBlock:sendToDeviceBlock];

	if (appDelegate.stopRequested)
	{
		[self stopLaunchedTask];

		adbOutput = [@"WARNING: installation stopped by request" copy];
	}

	if ([adbOutput containsString:@"ERROR:"])
	{
		NSString *errorMsg = @"*adb* encountered an error installing the app:\n\n";

		// [self showError:@"Device Installation Problem" message:[errorMsg stringByAppendingString:adbOutput] helpURL:nil parentWindow:[self window]];
		[self showModalSheet:@"Device Installation Problem" message:[errorMsg stringByAppendingString:adbOutput] buttonLabels:@[@"OK", @"Retry Installation" ] alertStyle:NSCriticalAlertStyle helpURL:nil parentWindow:[self window] completionHandler: ^(NSModalResponse returnCode)
		 {
			 if (returnCode == NSAlertSecondButtonReturn)
			 {
				 [self performSelector:@selector(sendAppToDevice) withObject:nil afterDelay:0.01];
			 }

			 [NSApp stopModalWithCode:returnCode];
		 }];

		[adbOutput release];

		return;
	}
	else
	{
		Rtt_Log("%s", [adbOutput UTF8String]);

		[self startTailDeviceSyslog:[XcodeToolHelper pathForAndroidDeviceSyslogUtility] appBundlePath:appBundlePath deviceID:@""];

		NSString *message = [NSString stringWithFormat:@"*%@* is installed on the Android device and is ready to run\n\nThe Android device's syslog will appear in the Corona Console until this message is closed", self.appName];

		[self showMessage:@"Device Installation Complete" message:message helpURL:nil parentWindow:[self window]];

		[self stopTailDeviceSyslog];
	}

	[adbOutput release];

	[self closeBuild:nil];
}

-(void)beginPasswordSheetWithKeyStore:(NSString*)path modalForWindow:(NSWindow*)parent modalDelegate:(id)delegate message:(NSString*)msg contextInfo:(void*)contextInfo
{
	if ( ! fAndroidPasswordController )
	{
		fAndroidPasswordController = [[DialogController alloc] initWithNibNamed:@"AndroidKeyStorePassword"];
	}
	else
	{
		Rtt_ASSERT( nil == [fAndroidPasswordController.inputs valueForKey:@"password"] );
		Rtt_ASSERT( nil == [fAndroidPasswordController.inputs valueForKey:@"keyStorePath"] );
	}

	NSMutableDictionary *inputs = fAndroidPasswordController.inputs;

	[inputs setValue:path forKey:@"keyStorePath"];
	[inputs setValue:[path lastPathComponent] forKey:@"keyStore"];
	if ( msg )
	{
		[inputs setValue:msg forKey:@"message"];
	}
    
	[fAndroidPasswordController beginSheet:parent modalDelegate:delegate contextInfo:contextInfo];
}

-(void)beginPasswordSheetWithKeyAlias:(NSString*)keyAlias modalForWindow:(NSWindow*)parent modalDelegate:(id)delegate message:(NSString*)msg contextInfo:(void*)contextInfo
{
	if ( ! fAndroidAliasPasswordController )
	{
		fAndroidAliasPasswordController = [[DialogController alloc] initWithNibNamed:@"AndroidKeyAliasPassword"];
	}
	else
	{
//		Rtt_ASSERT( nil == [fAndroidAliasPasswordController.inputs valueForKey:@"password"] );

		// Clear pwd just in case, but it should be nil
		[fAndroidAliasPasswordController.inputs removeObjectForKey:@"password"];
	}

	[fAndroidAliasPasswordController.inputs setValue:keyAlias forKey:@"keyAlias"];
	if ( msg )
	{
		[fAndroidAliasPasswordController.inputs setValue:msg forKey:@"message"];
	}
	[fAndroidAliasPasswordController beginSheet:parent modalDelegate:delegate contextInfo:contextInfo];
}

// Returns TRUE if alias list was updated, FALSE otherwise
// (callers are expected to call showKeystoreError: if necessary)
-(BOOL)updateAliasListWithKeystorePath:(NSString*)path password:(NSString *)password
{
	BOOL result = NO;

	if ( Rtt_VERIFY( path && password ) )
	{
		ListKeyStore listKeyStore;
		const char *keyStore = [path UTF8String];
		const char *pw = [password UTF8String];

		if ( listKeyStore.GetAliasList( keyStore, pw ) )
		{
            NSMutableArray *itemArray = [[[NSMutableArray alloc] initWithCapacity:listKeyStore.GetSize()] autorelease];

			for ( int i = 0; i < listKeyStore.GetSize(); i++ )
			{
                const char *title = listKeyStore.GetAlias( i );

                if (title != NULL)
                {
                    [itemArray addObject:[NSString stringWithExternalString:title]];
                }
            }

            NSArray *sortedArray = [itemArray sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];

            NSMenu* identitiesMenu = [fSigningIdentitiesAndroid menu];

            [identitiesMenu removeAllItems];

            [[identitiesMenu addItemWithTitle:kChooseFromFollowing action:nil keyEquivalent:@""] setEnabled:NO];

            for (NSString *titleStr in sortedArray)
            {
                NSMenuItem *menuItem = [[[NSMenuItem alloc] initWithTitle:titleStr action:nil keyEquivalent:@""] autorelease];
                [identitiesMenu addItem:menuItem];
			}

            // If the last key alias selected is in the list, reselect it
            NSString* androidLastKeyAlias = [appDelegate restoreAppSpecificPreference:@"androidKeystoreAlias" defaultValue:kValueNotSet];
            if (! [androidLastKeyAlias isEqualToString:kValueNotSet])
            {
                [fSigningIdentitiesAndroid selectItemWithTitle:androidLastKeyAlias];
                [self setKeystoreAlias:[fSigningIdentitiesAndroid selectedItem]];
            }

            if (! [[[fSigningIdentitiesAndroid selectedItem] title] isEqualToString:androidLastKeyAlias] && listKeyStore.GetSize() == 1)
            {
                // If we didn't find a remembered item and there's only one item in the list, select it
                // (it's menu item #1, #0 is "Choose from the following"; the item is typically "androiddebugkey")
                [fSigningIdentitiesAndroid selectItem:[identitiesMenu itemAtIndex:1]];
                [self setKeystoreAlias:[fSigningIdentitiesAndroid selectedItem]];
            }
            else if (! [[[fSigningIdentitiesAndroid selectedItem] title] isEqualToString:androidLastKeyAlias])
            {
                // Nothing selected and there's more than one alias in the list, select the zeroth item ("Choose from the following")
                [fSigningIdentitiesAndroid selectItem:[identitiesMenu itemAtIndex:0]];
            }

            result = YES;
		}
	}

	return result;
}

- (void) showKeystoreError:(NSString*)path window:(NSWindow*)window
{
    NSString *msg = [NSString stringWithFormat:@"The password for the Keystore located at `%@` is not correct or the file is not a valid Keystore", path];

    [self showError:@"Cannot Unlock Keystore" message:msg helpURL:nil parentWindow:window];
}

- (void) showKeystoreAliasError:(NSString*)path aliasName:(NSString *)aliasName window:(NSWindow*)window
{
    NSString *msg = [NSString stringWithFormat:@"The password for the alias `%@` in Keystore located at `%@` is not correct or the file is not a valid Keystore", aliasName, path];

    [self showError:@"Cannot Unlock Keystore" message:msg helpURL:nil parentWindow:window];
}

-(void)setKeystorePassword:(NSString *)password forPath:(NSString*)path
{
	NSString *key = [NSString stringWithFormat:@"androidKeyStore/%@", path];
	platformServices->SetSecurePreference( [key UTF8String], [password UTF8String] );
}

-(BOOL)isDebugKeystore
{
	NSMenuItem *item = [fAndroidKeystore selectedItem];
	return [[item title] isEqualToString:kAndroidDebugKeystoreTitle];
}

-(NSString*)keystorePath
{
	NSString *result = nil;

	NSMenuItem *item = [fAndroidKeystore selectedItem];
	if ( [[item title] isEqualToString:kAndroidDebugKeystoreTitle] )
	{
		result = [[NSBundle mainBundle] pathForResource:@"debug" ofType:@"keystore"];
		Rtt_ASSERT( [[NSFileManager defaultManager] fileExistsAtPath:result] );
	}
	else
	{
		result = (NSString*)[item representedObject];
	}

	return result;
}

-(NSString*)keystorePasswordForPath:(NSString*)path
{
	// You should only call this if you know if it's a custom keystore file
	Rtt_ASSERT( ! [[[fAndroidKeystore selectedItem] title] isEqualToString:kAndroidDebugKeystoreTitle] );

	NSString *result = nil;

	NSString *key = [NSString stringWithFormat:@"androidKeyStore/%@", path];
	Rtt::String value;
	platformServices->GetSecurePreference( [key UTF8String], &value );
	const char *p = value.GetString();
	if ( p )
	{
		result = [NSString stringWithExternalString:p];
	}

	return result;
}

-(NSString*)keystorePassword
{
	NSString *result = nil;

	NSMenuItem *item = [fAndroidKeystore selectedItem];
	if ( [[item title] isEqualToString:kAndroidDebugKeystoreTitle] )
	{
		result = kAndroidDebugKeystorePassword;
	}
	else
	{
		NSString *path = (NSString*)[item representedObject]; Rtt_ASSERT( path );
		result = [self keystorePasswordForPath:path];
	}

	return result;
}

-(NSMenuItem*)addKeystoreWithPath:(NSString*)path
{
	NSString *title = [[[NSString alloc] initWithFormat:@"%@ (%@)", [path lastPathComponent], [path stringByAbbreviatingWithTildeInPath]] autorelease];

    [fAndroidKeystore addItemWithTitle:title];

    NSMenuItem *item = [fAndroidKeystore itemWithTitle:title];
	[item setRepresentedObject:path];

	return item;
}

-(void)initKeystore
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString *pathsKey = @"androidKeystorePaths";
    NSArray *paths = [defaults arrayForKey:pathsKey];

    for ( size_t i = 0, iMax = [paths count]; i < iMax; i++ )
    {
        NSString *p = [paths objectAtIndex:i];
        [self addKeystoreWithPath:p];
    }

    // If the last key alias selected is in the list, reselect it (otherwise the first item "Debug" remains selected)
    NSString* androidLastKeystore = [appDelegate restoreAppSpecificPreference:@"androidKeystore" defaultValue:kValueNotSet];
    if (! [androidLastKeystore isEqualToString:kValueNotSet])
    {
        [fAndroidKeystore selectItemWithTitle:androidLastKeystore];
    }
}

-(IBAction)setKeystore:(id)sender
{
	NSMenuItem *item = [fAndroidKeystore selectedItem];
	NSString *path = [self keystorePath];
	NSString *pwd = nil;

	bool inhibitRemoval = false;
	if ( [[item title] isEqualToString:kAndroidDebugKeystoreTitle] )
	{
		inhibitRemoval = true;
		pwd = kAndroidDebugKeystorePassword;
	}
	else
	{
		Rtt_ASSERT( [item representedObject] );
		Rtt_ASSERT( [[item representedObject] isKindOfClass:[NSString class]] );

		path = (NSString*)[item representedObject];
		pwd = [self keystorePasswordForPath:path];

		if ( ! pwd )
		{
			NSWindow *buildWindow = [self window];
			[self beginPasswordSheetWithKeyStore:path
								  modalForWindow:buildWindow
								   modalDelegate:self
										 message:@"Enter the password for this keystore"
									 contextInfo:nil];
			return;
		}
	}

	if ( ! [self updateAliasListWithKeystorePath:path password:pwd]
		 && ! inhibitRemoval )
	{
		[self showKeystoreError:path window:[self window]];

		// Set default selection
		[fAndroidKeystore selectItemWithTitle:kAndroidDebugKeystoreTitle];
	}
}

- (IBAction)setKeystoreAlias:(id)sender
{
    NSString *aliasName = [(NSMenuItem *)sender title];

    if ( ! [self isDebugKeystore] && aliasName != nil )
    {
        Rtt::String value;
        NSString *key = [NSString stringWithFormat:@"androidKeyAlias/%@", aliasName];

        platformServices->GetSecurePreference( [key UTF8String], &value );

        // verify pw so we can retry if we need to
        if ( value.GetString() != NULL && aliasName != nil )
        {
            // passwords
            const char * aks = [[self keystorePath] UTF8String];
            const char * aka = [aliasName UTF8String];

            if ( ! [self areKeystoreAndAliasPasswordsValid:aks
                                                     keyPW:[[self keystorePassword] UTF8String]
                                                     alias:aka
                                                   aliasPW:value.GetString()] )
            {
                value.Set( NULL );
            }
        }

        if ( value.GetString() == NULL )
        {
            [self beginPasswordSheetWithKeyAlias:aliasName
                                  modalForWindow:[self window]
                                   modalDelegate:self
                                         message:@"Enter the key alias password to continue"
                                     contextInfo:nil];

            return;
        }

        self.androidKeyAliasPassword = [NSString stringWithExternalString:value.GetString()];
    }
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
			result = [validator runAndroidFileValidationTestsInProjectPath:[self projectPath]];
		}
		[validator release];

		if ( ! result )
		{
			[self logEvent:@"build-invalid-project"];
		}
	}

	if (result)
	{
		if (fEnableLiveBuild.state == NSOnState && ![self isDebugKeystore])
		{
			[self showError:@"Cannot create Live Build" message:@"Live Build can be created only with the Debug keystore selected." helpURL:nil parentWindow:[self window]];
			result = NO;
		}
	}

	return result;
}

- (BOOL)isStoreBuild
{
	NSMenuItem *item = [fAndroidKeystore selectedItem];
	BOOL result = ( ! [[item title] isEqualToString:kAndroidDebugKeystoreTitle] );
	return result;
}

-(BOOL)areKeystoreAndAliasPasswordsValid:(const char *)keyStore keyPW:(const char *)keyPW alias:(const char *)alias aliasPW:(const char *)aliasPW 
{
	ListKeyStore listKeyStore;
	Rtt::String resourcesDir;

	platformServices->Platform().PathForFile( NULL, Rtt::MPlatform::kResourceDir, Rtt::MPlatform::kDefaultPathFlags, resourcesDir );

	return listKeyStore.AreKeyStoreAndAliasPasswordsValid(keyStore, keyPW, alias, aliasPW, resourcesDir.GetString());

#if 0
	Rtt::String resourcesDir;

	self.authorizer->GetServices().Platform().PathForFile( NULL, Rtt::MPlatform::kResourceDir, Rtt::MPlatform::kDefaultPathFlags, resourcesDir );

	NSString *resourceDir = [NSString stringWithExternalString:resourcesDir.GetString()];
//	NSString *buildFilePath = [resourceDir stringByAppendingPathComponent:@"build.xml"];
	NSString *tmpDir = NSTemporaryDirectory();
	NSString *srcTestJar = [resourceDir stringByAppendingPathComponent:@"_coronatest.jar"];
	NSString *dstTestJar = [tmpDir stringByAppendingPathComponent:@"_coronatest.jar"];

// Escape quotes in the strings by replacing single quotes with "'\''" and then enclosing in single quotes when we build the command
    NSString *keyStoreStr = [[NSString stringWithExternalString:keyStore] stringByReplacingOccurrencesOfString:@"'" withString:@"'\''"];
    NSString *keyPWStr = [[NSString stringWithExternalString:keyPW] stringByReplacingOccurrencesOfString:@"'" withString:@"'\''"];
    NSString *aliasStr = [[NSString stringWithExternalString:alias] stringByReplacingOccurrencesOfString:@"'" withString:@"'\''"];
    NSString *aliasPWStr = [[NSString stringWithExternalString:aliasPW] stringByReplacingOccurrencesOfString:@"'" withString:@"'\''"];
    
	NSString *script = [[NSString alloc] initWithFormat:
		@"cp -f '%@' '%@'; JAVA_TOOL_OPTIONS='-Duser.language=en' jarsigner -tsa http://timestamp.digicert.com -keystore '%@' -storepass '%@' -keypass '%@' '%@' '%@'; exit $?",
		srcTestJar, tmpDir, keyStoreStr, keyPWStr, aliasPWStr, dstTestJar, aliasStr];

//	NSLog( @"tmp dir(%@)", tmpDir );
//	NSLog( @"<script>\n%@\n</script>", script ); 

    // The command we run to test the password emits the string "jar signed", this makes that look sensible
    printf("Testing credentials for '%s': ", keyStore);

	NSArray *arguments = [[NSArray alloc] initWithObjects:@"-c", script, nil];
	[script release];

	NSTask* task = [[NSTask alloc] init];
	[task setLaunchPath:@"/bin/sh"];
	[task setArguments:arguments];
	[arguments release];
//	NSFileHandle* devNull = [NSFileHandle fileHandleWithNullDevice];
//	[task setStandardOutput:devNull];
//	[task setStandardError:devNull];
	[task launch];
	[task waitUntilExit];
	int exitCode = [task terminationStatus];
//	[task setStandardOutput:[NSFileHandle fileHandleWithStandardOutput]];
//	[task setStandardError:[NSFileHandle fileHandleWithStandardOutput]];
	[task release];
//	Rtt_TRACE_SIM( ( "ERROR: Failed to verify alias password.  Error code(%d)\n", exitCode ) );
//	NSLog( @"alias password.  exit code(%d)\n", exitCode );
	return ( 0 == exitCode );
#endif
}

- (NSString*)appExtension
{
    BOOL shouldSendToDevice = ([postBuildRadioGroup selectedRow] == 0); // first item in radio group
    
    //Use apk extension for sending app to device 
    if(shouldSendToDevice){
        return @"apk";
    }else{
        return @"aab";
    }
}

-(void)sheetDidEnd:(NSWindow*)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
	if ( fAndroidPasswordController.fWindow == sheet )
	{
		NSMutableDictionary *inputs = fAndroidPasswordController.inputs;

		if ( returnCode == kActionDefault )
		{
			[sheet close];

			NSString *path = [inputs valueForKey:@"keyStorePath"]; Rtt_ASSERT( path );
			NSString *password = [inputs valueForKey:@"password"];

			if ( [self updateAliasListWithKeystorePath:path password:password] )
			{
				// Success!

				// Add item to list of available keystores
				NSMenuItem *item = [self addKeystoreWithPath:path];
				[fAndroidKeystore selectItem:item];

				// Save key
				NSString *pathsKey = @"androidKeystorePaths";
				NSMutableArray *newValue = [NSMutableArray arrayWithArray:[[NSUserDefaults standardUserDefaults] arrayForKey:pathsKey]];
				[newValue addObject:path];
				[[NSUserDefaults standardUserDefaults] setObject:newValue forKey:pathsKey];

				// Save pwd
				[self setKeystorePassword:password forPath:path];
				// NSString *key = [NSString stringWithFormat:@"androidKeyStore/%@", path];
				// platformServices->SetSecurePreference( [key UTF8String], [password UTF8String] );
			}
			else
			{
				// Bad pwd or bad keystore.
				[self showKeystoreError:path window:[self window]];
			}
		}

		[inputs removeObjectForKey:@"password"];
		[inputs removeObjectForKey:@"keyStorePath"];
	}
	else if ( fAndroidAliasPasswordController.fWindow == sheet )
	{
        // We come here after beginPasswordSheetWithKeyAlias:

		NSMutableDictionary *inputs = fAndroidAliasPasswordController.inputs;

		if ( returnCode == kActionDefault )
		{
			[sheet close];

			NSString *keystorePath = [self keystorePath];
			NSString *keystorePwd = [self keystorePasswordForPath:keystorePath];

			NSMenuItem *menuItem = [fSigningIdentitiesAndroid selectedItem];
			NSString *keyAlias = [menuItem title];
			NSString *androidPwd = [inputs valueForKey:@"password"];

			// Verify key alias pwd
			if ( [self areKeystoreAndAliasPasswordsValid:[keystorePath UTF8String]
						keyPW:[keystorePwd UTF8String]
						alias:[keyAlias UTF8String]
						aliasPW:[androidPwd UTF8String]] )
			{
				// Save the key alias password
				NSString *key = [NSString stringWithFormat:@"androidKeyAlias/%@", keyAlias];
				platformServices->SetSecurePreference( [key UTF8String], [androidPwd UTF8String] );

                self.androidKeyAliasPassword = androidPwd;
			}
			else
			{
                // Bad pwd or bad keystore.
                [self showKeystoreAliasError:keystorePath aliasName:keyAlias window:[self window]];

                // Can't use this alias so set default selection
                if (! [[[fAndroidKeystore selectedItem] title] isEqualToString:kAndroidDebugKeystoreTitle])
                {
                    [self setKeystore:[fAndroidKeystore itemWithTitle:kAndroidDebugKeystoreTitle]];
                }
			}
		}

		[inputs removeObjectForKey:@"password"];
	}
}

- (Rtt::TargetDevice::Platform)targetPlatform
{
	return Rtt::TargetDevice::kAndroidPlatform;
}

- (IBAction)browseKeystorePath:(id)sender
{
    NSOpenPanel* panel = panel = [NSOpenPanel openPanel];

    [panel setAllowsMultipleSelection:NO];
    [panel setCanChooseDirectories:NO];
    [panel setCanChooseFiles:YES];

    // Pull the user's last folder from user defaults for their convenience
    NSString* start_directory = [[NSUserDefaults standardUserDefaults] stringForKey:kKeyStoreFolderPath];

    if(nil != start_directory)
    {
        Rtt_ASSERT( [start_directory isAbsolutePath] );
        [panel setDirectoryURL:[NSURL fileURLWithPath:start_directory]];
    }

    NSInteger result = [panel runModal];

    if ( NSCancelButton != result )
    {
        NSArray* filenames = [panel URLs]; Rtt_ASSERT( [filenames count] <= 1 );
        NSString* path = [[filenames lastObject] path];
        Rtt_ASSERT( [[NSFileManager defaultManager] fileExistsAtPath:path] );
        // Save the chosen path into user defaults so we can use it as the starting path next time
        [[NSUserDefaults standardUserDefaults] setObject:[[panel directoryURL] path] forKey:kKeyStoreFolderPath];
        [[NSUserDefaults standardUserDefaults] setObject:path forKey:kKeyStoreFolderPathAndFile];

        [self beginPasswordSheetWithKeyStore:path
                              modalForWindow:[self window]
                               modalDelegate:self
                                     message:@"Enter the password for this keystore"
                                 contextInfo:nil];
    }
}

- (Rtt::TargetDevice::Platform) getPlatformFromMenuSelection
{
    Rtt::TargetAndroidAppStore *pLastSelectedStore = Rtt::TargetAndroidAppStore::GetByStringId([self.targetStoreId UTF8String]);

    Rtt::TargetDevice::Platform result = Rtt::TargetDevice::kAndroidPlatform;
    if (NULL != pLastSelectedStore)
    {
        result = pLastSelectedStore->GetPlatform();
    }
    
    return result;
}

-(void)initTargetStore
{
    [fTargetStore removeAllItems];

    NSString* lastAndroidTargetStoreId = [appDelegate restoreAppSpecificPreference:@"androidTargetStore" defaultValue:kValueNotSet];

    if ([lastAndroidTargetStoreId isEqualToString:kValueNotSet])
    {
        lastAndroidTargetStoreId = [[NSUserDefaults standardUserDefaults] stringForKey:kUserPreferenceLastAndroidTargetStore];
    }

    Rtt::TargetAndroidAppStore *pLastSelectedStore = Rtt::TargetAndroidAppStore::GetByStringId([lastAndroidTargetStoreId UTF8String]);

    int iStoreCount = Rtt::TargetAndroidAppStore::GetCount();
    for (int iStoreIndex = 0; iStoreIndex < iStoreCount; iStoreIndex++)
    {
        Rtt::TargetAndroidAppStore *pNextStore = Rtt::TargetAndroidAppStore::GetByIndex(iStoreIndex);
        if (pNextStore != NULL)
        {

            NSString *storeName = [[NSString alloc] initWithUTF8String:pNextStore->GetName()];
            NSMenuItem *item = [self addTargetStoreWithTitle:storeName];
            if (nil != item)
            {
                NSString *storeId = [[NSString alloc] initWithUTF8String:pNextStore->GetStringId()];

                [item setRepresentedObject:storeId];

                if (pNextStore == pLastSelectedStore)
                {
                    [fTargetStore selectItem:item];
                }
                
                if (pLastSelectedStore == NULL && pNextStore == &Rtt::TargetAndroidAppStore::kGoogle)
                {
                    [fTargetStore selectItem:item];
                }
            }
            
        }
    }
}

-(NSMenuItem*)addTargetStoreWithTitle:(NSString*)store
{
    NSString *title = store;
    [fTargetStore addItemWithTitle:title];
    NSMenuItem *item = [fTargetStore itemWithTitle:title];
    [title release];

    return item;
}

-(void)didSelectAlias:(NSNotification*)notification
{
    [fSigningIdentities setTitle:@"Selection made"];
    [self willChangeValueForKey:@"buildAndroidFormComplete"];
    [self didChangeValueForKey:@"buildAndroidFormComplete"];
}

- (void) restoreBuildPreferences
{
	[super restoreBuildPreferences];

	self.appVersionCode = [NSNumber numberWithInt:[[appDelegate restoreAppSpecificPreference:@"androidAppVersionCode" defaultValue:@"1"] intValue]];
	self.androidAppPackage = [appDelegate restoreAppSpecificPreference:@"androidAppPackage" defaultValue:@""];
}

- (void) saveBuildPreferences
{
    [super saveBuildPreferences];

	[appDelegate saveAppSpecificPreference:@"androidAppPackage" value:self.androidAppPackage];
	[appDelegate saveAppSpecificPreference:@"androidAppVersionCode" value:[self.appVersionCode stringValue]];
    [appDelegate saveAppSpecificPreference:@"androidKeystore" value:[[fAndroidKeystore selectedItem] title]];
    [appDelegate saveAppSpecificPreference:@"androidKeystoreAlias" value:self.androidKeyAlias];
    [appDelegate saveAppSpecificPreference:@"androidTargetStore" value:self.targetStoreId];
#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
    [appDelegate saveAppSpecificPreference:@"enableMonetization" value:(([fEnableMonetization state] == NSOnState) ? kValueYes : kValueNo)];
#endif
}

- (BOOL)verifyBuildTools:(id)sender
{
	// We can only build for Android if we have Java installed

	NSString *buttonLabel = @"More Info...";
	static BOOL installStarted = NO;

	const char *failureText = "No Java runtime present";
	Rtt::String cmdResult;
	FILE *pipeFP = popen("2>&1 /usr/libexec/java_home --task CommandLine", "r");

	if (pipeFP != NULL)
	{
		char buf[BUFSIZ];

		while (fgets(buf, BUFSIZ, pipeFP) != NULL)
		{
			cmdResult.Append(buf);
		}

		pclose(pipeFP);
	}

	if (strstr(cmdResult.GetString(), failureText) != NULL)
	{
		NSAlert* alert = [[[NSAlert alloc] init] autorelease];
		NSString *msg = nil;
		NSString *jreLocation = @"/Library/Internet Plug-Ins/JavaAppletPlugin.plugin";
		NSString *jreMessage = @"";

		if ([[NSFileManager defaultManager] fileExistsAtPath:jreLocation])
		{
			jreMessage = @"You have the Java Runtime Environment (JRE) installed but Android builds require the Java Development Kit (JDK).  Be sure to install the right package.\n\n";
		}

		if (installStarted)
		{
			msg = [NSString stringWithFormat:@"Java is required by the Android SDK and needs to be installed to build Android applications.\n\n%@An install of Java has already been requested and should be allowed to complete.  If you'd like to try again, press the \"%@\" button on the dialog that follows.\n", jreMessage, buttonLabel];

			[alert addButtonWithTitle:@"Retry Installation"];
			[alert addButtonWithTitle:@"Cancel"];
		}
		else
		{
			msg = [NSString stringWithFormat:@"Java is required by the Android SDK and needs to be installed to build Android applications.  Note that the Java Development Kit (JDK) is required (the JRE is not sufficient).\n\n%@To install Java, press the \"%@\" button on the dialog that follows.  When it is installed, build for Android again.\n", jreMessage, buttonLabel];

			[alert addButtonWithTitle:@"Install Java…"];
			[alert addButtonWithTitle:@"Cancel Build"];
		}

		[alert setAlertStyle:NSWarningAlertStyle];
		[alert setMessageText:@"Java Required"];
		[alert setInformativeText:msg];
		[alert setHelpAnchor:@"https://coronalabs.com/links/simulator/java-required"];
		[alert setShowsHelp:YES];
		[alert setDelegate:self];

		Rtt_Log("Corona Simulator: %s", [msg UTF8String]);

		NSInteger alertResult = [alert runModal];

		if (alertResult == NSAlertFirstButtonReturn)
		{
			installStarted = YES;

			Rtt_Log("Running: /usr/libexec/java_home --request");

			// This command returns immediately and then shows the Java install dialog
			// (the use of 'script' forces 'java_home' to believe it's running interactively
			// otherwise it insists on installing the JNI version of Java)
			system("/usr/bin/script -q /dev/null /usr/libexec/java_home --request >/dev/stdout");
		}

		// No Java so we can't continue to build
		return NO;
	}
	else
	{
		if (![[NSFileManager defaultManager] fileExistsAtPath:@"/usr/bin/java"])
		{
			[self showError:@"Cannot Find java Command Line Utility" message:@"Cannot find `/usr/bin/java`.  You should try reinstalling Java." helpURL:@"https://coronalabs.com/links/simulator/java-required" parentWindow:[self window]];

			return NO;
		}

		if (![[NSFileManager defaultManager] fileExistsAtPath:@"/usr/bin/keytool"])
		{
			[self showError:@"Cannot Find keytool Utility" message:@"Cannot find `/usr/bin/keytool`.  You should try reinstalling Java." helpURL:@"https://coronalabs.com/links/simulator/java-required" parentWindow:[self window]];

			return NO;
		}

		// Java is installed, emit the version to the console log

		Rtt::String cmdResult;
		FILE *pipeFP = popen("2>&1 /usr/bin/java -version", "r");

		if (pipeFP != NULL)
		{
			char buf[BUFSIZ];

			while (fgets(buf, BUFSIZ, pipeFP) != NULL)
			{
				cmdResult.Append(buf);
			}

			pclose(pipeFP);
		}

		Rtt_Log("Using %s", cmdResult.GetString());
	}

	return YES;
}

@end

// ----------------------------------------------------------------------------
