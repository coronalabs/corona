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

#ifndef _Rtt_AuthorizationTicket_H__
#define _Rtt_AuthorizationTicket_H__

#include "Rtt_TargetDevice.h"

// ----------------------------------------------------------------------------

class private_key;
class public_key;

namespace Rtt
{

class MPlatformDevice;
class MPlatformServices;

// ----------------------------------------------------------------------------

//bool operator==( const AuthorizationTicketData& lhs, const AuthorizationTicketData& rhs );

class AuthorizationTicket
{
	public:
		enum Subscription
		{
			kUnknownSubscription = 0,
			kTrialSubscription = 1, // SKU: Starter
			kProSubscription = 2, // SKU: Pro (formerly: kGameEditionSubscription)
			kBasicSubscriptionREMOVED = 3, // Old SKU to delineate from Game Edition.
			kBasicSubscription = 4, // SKU: Basic, a tier between Starter and Pro. Formerly called Indie-iOS
			kIndieAndroidSubscriptionREMOVED = 5, // Old SKU for Indie-Android
			kProPlusSubscription = 6,
			kEnterpriseAutomatedBuildsSubscription = 7, // SKU: Enterprise for command-line driven device builds via build server
			kEnterpriseHostedTemplatesSubscription = 8,
			kEnterpriseNativeExtensionsSubscription = 9 // SKU: Enterprise
		};

		static const char* StringForSubscription( Subscription sub );
		static const char* DisplayStringForSubscription( Subscription sub );

		enum License
		{
			kUnknownLicense = 0,
			kIndividual,
			kStudent,
			kEnterprise
		};

		enum Product
		{
			kUnknownProduct = 0,
			kCoronaSDK,
			kCoronaGameEdition
		};

		enum PlatformAllowed
		{
			kUnknownPlatform = 0,
			kMacPlatform,
			kWinPlatform
		};

		enum Property 
		{
			kUnknownProperty = 0,
			kTicketActive,
			kSubscriptionPaid,
			kDeviceAuthorized
		};

	public:
		// Following is used as a form of digital signature.
		// E.g. sender encrypts using their private key; recipient decrypts using sender's public key
		// TODO: Very fragile implementation.  Assumes null-termination...
		// Use free() to free results. NULL returned if error.
		static U8* CipherToPlain( const public_key& k, const char *ciphertext, size_t& outNumBytes );
		static U8* PlainToCipher( const private_key& k, const char *plaintext, size_t& outNumBytes );

		// Same as above but this is traditional public crypto.
		// E.g. sender encrypts using recipient's public key; recipient decrypts using their private key
		static U8* CipherToPlainPrivate( const private_key& k, const char *ciphertext, size_t& outNumBytes );
		static U8* PlainToCipherPublic( const public_key& k, const char *plaintext, size_t& outNumBytes );

		static int StringToPlatform( const char *str );

	protected:
		Subscription VerifySubscriptionValue( unsigned int value );
		License VerifyLicenseValue( unsigned int value );
		PlatformAllowed VerifyPlatformAllowedValue( unsigned int value );

 	public:
		AuthorizationTicket( const MPlatformDevice& device, const char *data );
		AuthorizationTicket( const AuthorizationTicket& rhs );
		AuthorizationTicket( const MPlatformDevice& device );
		~AuthorizationTicket();

	public:
		AuthorizationTicket& operator=( const AuthorizationTicket& rhs );

	private:
		void CopyUsername( const char *username );

	public:
		// NOTE: If this returns false, the user can still do trial builds for that platform.
		bool IsAllowedToBuild( TargetDevice::Platform platform ) const;
		int GetBuildWaitTime( TargetDevice::Platform platform ) const;
		U32 CalculateBuildDelayTimeInSeconds( TargetDevice::Platform platform ) const;
    //		bool IsSkinAllowed( TargetDevice::Skin skin ) const;
		bool IsDailyBuildAllowed() const;

	public:
		S32 GetExpiration() const { return fExpiration; }
		float GetNumDaysLeft() const;
		bool IsPaidSubscription() const;
		U8 GetAllowedPlatform() const { return fPlatformAllowed; }		
		const char* GetUsername() const { return fUsername; }
		const char* GetDeviceId() const { return fDeviceId; }

	public:
		Subscription GetSubscription() const { return (Subscription)fSubscription; }
		U32 GetUid() const { return fUid; }

	public:
		bool IsUpdateAllowed() const { return MatchesCommon() && IsExpired(); }
		bool DoesSubscriptionIncludeProduct() const;
		bool IsAppAllowedToRun() const;
		bool IsSubscriptionCurrent() const { return IsPaidSubscription() && ! IsExpired(); }

	public:
		bool IsTrial() const;
		bool IsPlaceholder() const;

	public:
		bool Verify() const { return MatchesCommon(); }

		// Returns whether corresponding subscription is expired.
		bool IsExpired() const;

	protected:
		bool MatchesCommon() const { return MatchesDeviceIdentifier() && MatchesPlatform(); }
		bool MatchesDeviceIdentifier() const;
		bool MatchesPlatform() const;

	private:
		const MPlatformDevice& fDevice;
		char fDeviceId[32 + 1 + 3]; // 1 byte for '\0' termination and 3 bytes to pad to 4-byte alignment
		S32 fExpiration;
		U8 fSubscription;
		U8 fLicense;
		U8 fPlatformAllowed;
		U8 fIsActive;
		U32 fUid;
		char *fUsername;
		U8 fProduct;	// Assigned at compile-time
		U8 fUnused0;	// 4 byte alignment
		U16 fUnused1;	// 4 byte alignment
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AuthorizationTicket_H__
