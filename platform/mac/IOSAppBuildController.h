//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "AppleAppBuildController.h"
#include "Rtt_IOSAppPackager.h"

// ----------------------------------------------------------------------------

@interface IOSAppBuildController : AppleAppBuildController
{
	@private

        BOOL signingIdentitiesAdded;
        NSMenuItem *currentIOSSimulatorItem;

        IBOutlet NSButton *fEnableMonetization;
        BOOL fWarnedAboutUnsupportedSDK;

        IBOutlet NSWindow *packageSheet;

        IBOutlet NSPopUpButton* fSupportedDevices;
        IBOutlet NSPopUpButton* fTargetIOSSDK;

        IBOutlet NSPopUpButton *appStoreAppSigningIdentityPopup;
        IBOutlet NSButton *appStoreSendButton;
        IBOutlet NSScrollView *appStoreSendButtonMesg;
        IBOutlet NSPopUpButton *availableSimulatorsPopup;
        
        //ToDo: Move to App Build Controller (when more added to more platfroms)
        
        IBOutlet NSButton *fIncludeStandardResources;
}

@end

// ----------------------------------------------------------------------------
