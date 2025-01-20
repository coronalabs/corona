//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Cocoa/Cocoa.h>

#include "Rtt_TargetDevice.h"

// ----------------------------------------------------------------------------

namespace Rtt
{
	class AuthorizationTicket;
}

// ----------------------------------------------------------------------------

@interface IdentityMenuItem : NSMenuItem
{
	NSString* identity;
    NSString* provisionPath;
    NSString* fingerprint;
}

@property (nonatomic, readonly) NSString *identity;
@property (nonatomic, readonly) NSString *provisionPath;
@property (nonatomic, readonly) NSString *fingerprint;

-(id)initWithIdentity:(NSString*)inIdentity provisionPath:(NSString*)inPath provisionName:(NSString *)provisionName expirationDate:(NSString *)expirationDate fingerprint:(NSString *)certFingerprint;

@end

// ----------------------------------------------------------------------------

// TODO: Controller is the wrong word
@interface AppleSigningIdentityController : NSObject
{
	@private
		NSString *fPath;
        NSString *fNewPath;
}

+ (NSString*)defaultProvisionPath;
+ (BOOL)hasProvisionedDevices:(NSString*)provisionFile;
+ (NSString*)signingIdentity:(NSString*)provisionFile commonName:(NSString **)commonName;
+ (NSDictionary *) getSigningIdentities;

- (id)init;
- (id)initWithProvisionPath:(NSString*)path;

- (void)populateMenu:(NSMenu*)menu platform:(Rtt::TargetDevice::Platform)platform delegate:(id)delegate;

@end

// ----------------------------------------------------------------------------
