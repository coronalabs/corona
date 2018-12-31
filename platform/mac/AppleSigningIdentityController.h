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
