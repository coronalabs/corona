//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "AppBuildController.h"
#import "AppleSigningIdentityController.h"

// ----------------------------------------------------------------------------
extern NSString *kAppleDistributionIdentityTag;

@interface AppleAppBuildController : AppBuildController
{
    @protected
        IdentityMenuItem *currentProvisioningProfileItem;
        NSString *itunesConnectUsername;
        NSString *itunesConnectPassword;
        NSString *xCodeSdkRoot;
        BOOL useItunesConnect;

}

- (NSString *) developerRoot:(BOOL *)outOkayToBuild errorString:(NSString **)errorString;

- (Rtt::TargetDevice::Platform)targetPlatform;
- (void) getItunesConnectAccountInfo;
- (void) restorePreviousProvisioningProfileSelection;
@end

// ----------------------------------------------------------------------------
