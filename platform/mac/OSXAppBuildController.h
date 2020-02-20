//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "AppleAppBuildController.h"
#import "AppDelegate.h"

#include "Rtt_OSXAppPackager.h"
#include "Rtt_PlatformAppPackager.h"

// ----------------------------------------------------------------------------
using namespace Rtt;

@interface OSXAppBuildController : AppleAppBuildController
{
	@private
		BOOL signingIdentitiesAdded;

        NSTask *fOpenApplicationTask;

        IBOutlet NSWindow *appStorePackageSheet;
        IBOutlet NSWindow *selfDistPackageSheet;
		OSXAppPackager *savedPackager;
		OSXAppPackagerParams *savedParams;

		IBOutlet NSPopUpButton *appStoreAppSigningIdentityPopup;
		IBOutlet NSPopUpButton *appStoreInstallerSigningIdentityPopup;
		IBOutlet NSButton *appStoreSendButton;

		IBOutlet NSPopUpButton *selfDistAppSigningIdentityPopup;
}

- (IBAction)selfDistOKClicked:(id)sender;
- (IBAction)appStoreOKClicked:(id)sender;
- (IBAction)appStoreCancelClicked:(id)sender;
- (IBAction)selfDistCancelClicked:(id)sender;
- (void) beginAppStorePackageSheet:(NSWindow*)parent packager:(OSXAppPackager *) inPackager params:(OSXAppPackagerParams *) inParams;
- (void) beginSelfDistPackageSheet:(NSWindow*)parent packager:(OSXAppPackager *) inPackager params:(OSXAppPackagerParams *) inParams;

@end

// ----------------------------------------------------------------------------
