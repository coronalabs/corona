//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "AppleAppBuildController.h"
#include "Rtt_TVOSAppPackager.h"

// ----------------------------------------------------------------------------

@interface TVOSAppBuildController : AppleAppBuildController
{
	@private

        BOOL signingIdentitiesAdded;
		NSMenuItem *currentTVOSSimulatorItem;

//        IBOutlet NSButton *fEnableMonetization;
        BOOL fWarnedAboutUnsupportedSDK;

        IBOutlet NSWindow *packageSheet;

        IBOutlet NSPopUpButton* fTargetSDK;

        IBOutlet NSPopUpButton *appStoreAppSigningIdentityPopup;
        IBOutlet NSButton *appStoreSendButton;
        IBOutlet NSScrollView *appStoreSendButtonMesg;
		IBOutlet NSPopUpButton *availableSimulatorsPopup;
}

@end

// ----------------------------------------------------------------------------
