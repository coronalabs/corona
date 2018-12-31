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

#include "Rtt_Authorization.h"
#include "Rtt_AuthorizationTicket.h"
#include "Rtt_WebServicesSession.h"
#include "Rtt_MacConsolePlatform.h"
#include "Rtt_Assert.h"
#include "Rtt_MacPlatform.h"
#include "Rtt_MacSimulatorServices.h"
#include "Rtt_MacSimulator.h"

#import "AppleAppBuildController.h"

#import "AppleSigningIdentityController.h"
#import "ValidationSupportMacUI.h"
#import "ValidationToolOutputViewController.h"

#import "Rtt_IOSAppPackager.h"
#import "Rtt_MacPlatform.h"
#import "XcodeToolHelper.h"

using namespace Rtt;

// ----------------------------------------------------------------------------

static NSString *kValueNotSet = @"not set";
static NSString *kValueNone = @"None";
static double kMinXcodeVersion = 9.0;

// ----------------------------------------------------------------------------
@implementation AppleAppBuildController

- (id)initWithWindowNibName:(NSString*)nibFile projectPath:(NSString *)projPath authorizer:(const Rtt::Authorization *)authorizer;
{
	self = [super initWithWindowNibName:nibFile projectPath:projPath authorizer:authorizer];

	if ( self )
	{
        simulatorServices = new MacSimulatorServices(appDelegate, (CoronaWindowController *)self, nil);
        useItunesConnect = NO;
	}

	return self;
}

- (void)dealloc
{
    [xCodeSdkRoot release];

	[super dealloc];
}

- (void) windowDidLoad
{
    [super windowDidLoad];

    NSMenu *signingIdentitiesMenu = [fSigningIdentities menu];

	NSMenuItem *noneItem = [[fSigningIdentities menu] itemWithTitle:kValueNone];
	[noneItem setTarget:self];
	[noneItem setAction:@selector(didSelectIdentity:)];

    /*
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(didSelectIdentity:)
                                                 name:NSMenuDidSendActionNotification
                                               object:signingIdentitiesMenu];
     */
    [self setProgressBarLabel:@"Scanning provisioning profiles…"];

    AppleSigningIdentityController *signingIdentityController = [[[AppleSigningIdentityController alloc] init] autorelease];
    __block BOOL okayToBuild = YES;
    __block NSString *missingComponentNames = nil;

    void (^processProvisioningProfiles)() = ^()
    {
        [signingIdentityController populateMenu:signingIdentitiesMenu platform:[self targetPlatform] delegate:self];

        xCodeSdkRoot = [[self developerRoot:&okayToBuild errorString:&missingComponentNames] copy];
    };

    [self runLengthyOperationForWindow:[self window] delayProgressWindowBy:0.75 allowStop:NO withBlock:processProvisioningProfiles];

    if (! okayToBuild)
    {
        // TODO: Show error message detailing missing build components
    }
}

- (IBAction)showWindow:(id)sender
{
    [super showWindow:sender];

    // Load the last provisioning profile used to build this app and select it in the menu (needs to be done every time
    // the build window is shown because the project may have changed)
    [self restorePreviousProvisioningProfileSelection];
}

- (void) didSelectIdentity:(NSMenuItem *)menuItem
{
    //NSDictionary *notificationInfo = [notification userInfo];
    IdentityMenuItem *item = (IdentityMenuItem *) menuItem; // [notificationInfo objectForKey:@"MenuItem"];

    Rtt_ASSERT([item isKindOfClass:[IdentityMenuItem class]] || [item isKindOfClass:[NSMenuItem class]]);

    // NSLog(@"iOSAppBuildController: didSelectIdentity: %@", item);

    [self willChangeValueForKey:@"buildFormComplete"];

    // Deselect the previously selected item (i.e. remove the checkmark)
    [currentProvisioningProfileItem setState:NSOffState];

    currentProvisioningProfileItem = item;

    // Mark the currently selected item
    [currentProvisioningProfileItem setState:NSOnState];

    // Show the current item in the popup button (even it's on a submenu)
    [[fSigningIdentities cell] setMenuItem:currentProvisioningProfileItem];

    [self didChangeValueForKey:@"buildFormComplete"];
}

- (void) visitProvisioningProfileURL:(id)sender
{
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://developer.apple.com/account/ios/profile/profileList.action"]];
}

- (void) restorePreviousProvisioningProfileSelection
{
    [self willChangeValueForKey:@"buildFormComplete"];

    NSString *provProfilePreferenceName = [[self platformName] stringByAppendingString:@"ProvisioningProfile"];
    NSString *prevProvProfile = [appDelegate restoreAppSpecificPreference:provProfilePreferenceName defaultValue:kValueNotSet];

    // Disable "Choose from the following" item
    [[[fSigningIdentities menu] itemAtIndex:0] setEnabled:NO];

    if ([prevProvProfile isEqualToString:kValueNotSet])
    {
        // No project specific provisioning profile has been saved, look for a default for the platform
        if ([self targetPlatform] == TargetDevice::kOSXPlatform)
        {
            provProfilePreferenceName = kUserPreferenceLastOSXCertificate;
        }
		else if ([self targetPlatform] == TargetDevice::kTVOSPlatform)
		{
			provProfilePreferenceName = kUserPreferenceLastTVOSCertificate;
		}
        else
        {
            provProfilePreferenceName = kUserPreferenceLastIOSCertificate;
        }

        prevProvProfile = [[NSUserDefaults standardUserDefaults] stringForKey:provProfilePreferenceName];

        if ([prevProvProfile length] == 0)
        {
            // Select "None" value as a default
            [fSigningIdentities selectItemAtIndex:1];

            [self didChangeValueForKey:@"buildFormComplete"];

            return;
        }
    }

    // Find the item with the given title in the menu and submenus and check it while unchecking everything else
    for (IdentityMenuItem *anItem in [[fSigningIdentities menu] itemArray])
    {
        // Luckily we only have one level of submenus
        if ([anItem hasSubmenu])
        {
            for (IdentityMenuItem *anItem2 in [[anItem submenu] itemArray])
            {
                if ([[anItem2 title] isEqualToString:prevProvProfile])
                {
                    [anItem2 setState:NSOnState];
                    currentProvisioningProfileItem = anItem2;
                    [fSigningIdentities selectItem:currentProvisioningProfileItem];
                    // We need to set the cell manually because we have submenus
                    [[fSigningIdentities cell] setMenuItem:currentProvisioningProfileItem];
                }
                else
                {
                    [anItem2 setState:NSOffState];
                }
            }
        }
        else if ([[anItem title] isEqualToString:prevProvProfile])
        {
            [anItem setState:NSOnState];
            currentProvisioningProfileItem = anItem;
            [fSigningIdentities selectItem:currentProvisioningProfileItem];
            // We need to set the cell manually because we have submenus
            [[fSigningIdentities cell] setMenuItem:currentProvisioningProfileItem];
        }
        else
        {
            [anItem setState:NSOffState];
        }
    }

    if (currentProvisioningProfileItem == nil)
    {
        // We didn't find the provisioning profile in the menu so we didn't load it (but we found its name in the
        // user defaults so we found it last time)
        Rtt_Log("WARNING: the provisioning profile last used to build this project was not found (%s)",
                [prevProvProfile UTF8String]);

        [fSigningIdentities selectItemAtIndex:0];
    }
    
    [self didChangeValueForKey:@"buildFormComplete"];
}

- (void) getItunesConnectAccountInfo
{
    OSStatus keychainStatus = errSecItemNotFound;
    NSString *serverName = @"idmsa.apple.com"; // this is more reliable than "itunesconnect.apple.com"
    UInt32 passwordLength = 0;
    void *passwordData = NULL;
    SecKeychainItemRef itemRef;
    NSString *warningMesg = @"No itunesConnect account info found: make sure you are signed into Application Loader (and have checked \"Keep me signed in.\")";

    // Load the user's iTunes Connect username from their Application Loader preferences
    NSUserDefaults *applicationLoaderDefaults = [[[NSUserDefaults alloc] init] autorelease];
    [applicationLoaderDefaults addSuiteNamed:@"com.apple.itunes.connect.ApplicationLoader"];
    NSString *applLoaderITunesUsername = [applicationLoaderDefaults stringForKey:@"ITunesUsername"];

    if ([itunesConnectUsername isEqualToString:applLoaderITunesUsername] && [itunesConnectPassword length] > 0)
    {
        // We already have the correct credentials
        return;
    }

    itunesConnectUsername = @"";
    itunesConnectPassword = @"";

    if ([applLoaderITunesUsername length] == 0)
    {
        Rtt_LogException("%s", [warningMesg UTF8String]);

        return;
    }

    // Look for the user's Application Loader password
    serverName = [NSString stringWithFormat:@"Application Loader: %@", applLoaderITunesUsername];

    keychainStatus = SecKeychainFindGenericPassword(NULL,  (UInt32) [serverName length], [serverName UTF8String],
                                                    0, NULL, // (UInt32) [accountName length], [accountName UTF8String],
                                                    &passwordLength, &passwordData,
                                                    &itemRef);

    if (keychainStatus != errSecItemNotFound && passwordData != NULL)
    {
        // If we find the password, we can find the account name (email) using the SecKeychainItemRef that corresponds to the password

        SecKeychainAttributeList *attributeList = nil;
        SecKeychainAttributeInfo info;
        UInt32 attributeConstants[1] = { kSecAccountItemAttr };				// what we're looking for
        UInt32 formatConstants[1] = { CSSM_DB_ATTRIBUTE_FORMAT_STRING };	// what its format is

        info.count = 1;
        info.tag = attributeConstants;
        info.format = formatConstants;

        keychainStatus = SecKeychainItemCopyAttributesAndData(itemRef, &info, NULL, &attributeList, 0, NULL);

        if (keychainStatus != errSecItemNotFound && attributeList != NULL)
        {
            SecKeychainAttribute accountNameAttribute = attributeList->attr[0];

            NSString* accountName = [[[NSString alloc] initWithData:[NSData dataWithBytes:accountNameAttribute.data length:accountNameAttribute.length] encoding:NSUTF8StringEncoding] autorelease];

            NSString *passwordString = [[[NSString alloc] initWithData:[NSData dataWithBytes:passwordData length:passwordLength] encoding:NSUTF8StringEncoding] autorelease];
            
            Rtt_Log("Using iTunes Connect account: %s", [accountName UTF8String]);
            
            itunesConnectUsername = [accountName retain];
            itunesConnectPassword = [passwordString retain];
            
            SecKeychainItemFreeAttributesAndData( attributeList, NULL );
        }
        
        SecKeychainItemFreeContent(NULL, passwordData);
        CFRelease(itemRef);
    }
    else
    {
        Rtt_LogException("%s", [warningMesg UTF8String]);
    }
}

- (NSString*)appExtension
{
	return @"app";
}

- (BOOL)buildFormComplete
{
	if (currentProvisioningProfileItem == nil)
	{
		[self showError:@"Missing Provisioning Profile" message:@"Choose a provisioning profile from the list" helpURL:nil parentWindow:[self window]];

		return NO;
	}

	// The minimum is checked in the super class so all we need to worry about is the max
	if (appName != nil && ! [appName isEqualToString:@""] && [appName length] > 50)
	{
		[self showError:@"Invalid Application Name" message:[NSString stringWithFormat:@"%@ application names must be 50 characters or less (and much less than that will actually be displayed in most circumstances)\n\nSee Apple's [App Store Product Page](https://developer.apple.com/app-store/product-page/)", self.platformTitle] helpURL:nil parentWindow:[self window]];

		return NO;
	}

    return [super buildFormComplete];
}

- (NSString *) developerRoot:(BOOL *)outOkayToBuild errorString:(NSString **)errorString
{
    BOOL debugBuildProcess = ([[NSUserDefaults standardUserDefaults] integerForKey:@"debugBuildProcess"] > 1);
    NSString *sdkRoot = nil;
    NSString* temporaryErrorString = @"";

    *outOkayToBuild = NO;

    sdkRoot = [XcodeToolHelper getXcodePath];

    if ( sdkRoot != nil )
    {
        NSString* developerBase = sdkRoot;

        // Assume okay to build, and will set NO when we discover a problem
        *outOkayToBuild = YES;

        if ([self targetPlatform] == TargetDevice::kIPhonePlatform)
        {
            NSString* copypng = [XcodeToolHelper pathForCopyPngUsingDeveloperBase:developerBase printWarning:debugBuildProcess];
            if (copypng == nil)
            {
                *outOkayToBuild = NO;
                temporaryErrorString = [temporaryErrorString stringByAppendingString:@"copypng\n"];
            }

            NSString* codesign_allocate = [XcodeToolHelper pathForCodesignAllocateUsingDeveloperBase:developerBase printWarning:debugBuildProcess];
            if (codesign_allocate == nil)
            {
                temporaryErrorString = [temporaryErrorString stringByAppendingString:@"codesign_allocate\n"];
                *outOkayToBuild = NO;
            }
        }

        if ([self targetPlatform] == TargetDevice::kOSXPlatform)
        {
        }

        NSString* codesign = [XcodeToolHelper pathForCodesignUsingDeveloperBase:developerBase printWarning:debugBuildProcess];
        if (codesign == nil)
        {
            temporaryErrorString = [temporaryErrorString stringByAppendingString:@"codesign\n"];
            *outOkayToBuild = NO;
        }

		double xcodeVersion = [XcodeToolHelper getXcodeVersion];

		if (xcodeVersion < kMinXcodeVersion)
		{
            temporaryErrorString = [temporaryErrorString stringByAppendingFormat:@"Xcode needs to be at least version %g (currently %g)\n\nIf you already have Xcode installed, run the following command:\n`sudo xcode-select -s /Applications/Xcode.app`", kMinXcodeVersion, xcodeVersion];
			*outOkayToBuild = NO;
		}
    }

    *errorString = temporaryErrorString;

    return sdkRoot;
}

// Subclasses should override this
- (Rtt::TargetDevice::Platform)targetPlatform
{
    return Rtt::TargetDevice::kUnknownPlatform;
}

- (BOOL)verifyBuildTools:(id)sender
{
    if (useItunesConnect)
    {
        // Do this here so it happens every time they press the build button which gives them the chance to fix things without
        // quitting the Simulator and restarting
        [self getItunesConnectAccountInfo];
    }

    return YES;
}

- (void) saveBuildPreferences
{
    [super saveBuildPreferences];

    [appDelegate saveAppSpecificPreference:[self.platformName stringByAppendingString:@"ProvisioningProfile"] value:[currentProvisioningProfileItem title]];
    
}

@end

// ----------------------------------------------------------------------------
