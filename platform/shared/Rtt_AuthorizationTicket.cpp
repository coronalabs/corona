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

#include "Rtt_AuthorizationTicket.h"

#include "Core/Rtt_VersionTimestamp.h"

#include "Rtt_MPlatformDevice.h"
#include "Rtt_MPlatformServices.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "rsa.hpp"
#include "vlong_io.h"

//#include "openssl/md5.h"
//#include <CommonCrypto/CommonDigest.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const char*
AuthorizationTicket::StringForSubscription( Subscription sub )
{
	// *** IMPORTANT NOTE ***
	// The following strings have to match the strings on the build server.
	// 
	// Two cases:
	//    1. The server uses this string for permissioning device builds
	//       (e.g. restricted library access).
	//       See 'StringForSubscriptionType' in corona_service.module on Drupal.
	//
	//    2. The Simulator gets the below strings based on the
	//       authorization ticket's subscription value.
	//
	// Since the business logic is shared in both cases (via prepending the
	// Lua code in config_require.lua to config.lua), we need to ensure that
	// the subscription strings match.
	//
	static const char kUnknown[] = "";
	static const char kTrial[] = "trial";
	static const char kPro[] = "pro";
	static const char kProPlus[] = "pro-plus";
	static const char kBasic[] = "basic";
	static const char kIndieAndroid[] = "indie-android";
	static const char kEnterpriseAutomatedBuilds[] = "enterprise-automated-builds";
	static const char kEnterpriseHostedTemplates[] = "enterprise-hosted-templates";
	static const char kEnterpriseNativeExtensions[] = "enterprise-native-extensions";
	
	const char *result = kUnknown;

	switch ( sub )
	{
		case kTrialSubscription:
			result = kTrial;
			break;
		case kProSubscription:
			result = kPro;
			break;
		case kProPlusSubscription:
			result = kProPlus;
			break;
		case kBasicSubscription:
			result = kBasic;
			break;
		case kIndieAndroidSubscriptionREMOVED:
			result = kIndieAndroid;
			break;
		case kEnterpriseAutomatedBuildsSubscription:
			result = kEnterpriseAutomatedBuilds;
			break;
		case kEnterpriseHostedTemplatesSubscription:
			result = kEnterpriseHostedTemplates;
			break;
		case kEnterpriseNativeExtensionsSubscription:
			result = kEnterpriseNativeExtensions;
			break;
		default:
			break;
	}

	return result;
}

const char*
AuthorizationTicket::DisplayStringForSubscription( Subscription sub )
{
	static const char kUnknown[] = "";
	static const char kTrial[] = "Starter";
	static const char kPro[] = "Corona"; // Changed for "Corona Free"; was "Pro"; was "Corona SDK"
	static const char kProPlus[] = "Pro+";
	static const char kBasic[] = "Basic";
	static const char kIndieAndroid[] = "Indie (Android)";
	static const char kEnterpriseAutomatedBuilds[] = "Enterprise (Automated builds)";
	static const char kEnterpriseHostedTemplates[] = "Enterprise (Hosted templates)";
	static const char kEnterpriseNativeExtensions[] = "Enterprise (Native extensions)";

	const char *result = kUnknown;

	switch ( sub )
	{
		case kTrialSubscription:
			result = kTrial;
			break;
		case kProSubscription:
			result = kPro;
			break;
		case kProPlusSubscription:
			result = kProPlus;
			break;
		case kBasicSubscription:
			result = kBasic;
			break;
		case kIndieAndroidSubscriptionREMOVED:
			result = kIndieAndroid;
			break;
		case kEnterpriseAutomatedBuildsSubscription:
			result = kEnterpriseAutomatedBuilds;
			break;
		case kEnterpriseHostedTemplatesSubscription:
			result = kEnterpriseHostedTemplates;
			break;
		case kEnterpriseNativeExtensionsSubscription:
			result = kEnterpriseNativeExtensions;
			break;
		default:
			break;
	}

	return result;
}

U8*
AuthorizationTicket::CipherToPlain( const public_key& keyPublic, const char *ciphertext, size_t& outNumBytes )
{
	U8* result = NULL;

	size_t numBytes = strlen( ciphertext );
	// len is a multiple of 8 because we emit little-endian hex during encryption
	if ( Rtt_VERIFY( (numBytes & 0x7) == 0 ) )
	{
		// during serialization, each byte was emitted as a 2-character hex string.
		unsigned int numWords = (unsigned int) numBytes >> 3;
		U32 *buf32 = (U32*)malloc( numWords*sizeof(U32) );
		for ( size_t i = 0, j = 0; i < numBytes; i += 8, j++ )
		{
			const char *bytes = ciphertext + i;
			// ciphertext was little-endian
			buf32[j] =
				vlong_io::HexToByte( bytes )
				| ( vlong_io::HexToByte( bytes+2 ) << 8 )
				| ( vlong_io::HexToByte( bytes+4 ) << 16 )
				| ( vlong_io::HexToByte( bytes+6 ) << 24 );
		}

		vlong c;
		c.load( (unsigned*)buf32, numWords ); Rtt_STATIC_ASSERT( sizeof( U32 ) == sizeof( unsigned ) );

		// public_key keyPublic( key.m, key.e );
		vlong cPlain = keyPublic.encrypt( c );

		numWords = cPlain.numWords();
		cPlain.store( (unsigned*)buf32, numWords );

		#if 0 //def Rtt_DEBUG
			// Print as hex (little-endian)
			for ( size_t i = 0; i < numWords; i++ )
			{
				U32 value = buf32[i];
				printf( "%02x", value & 0xFF );
				printf( "%02x", (value>>8) & 0xFF );
				printf( "%02x", (value>>16) & 0xFF );
				printf( "%02x", (value>>24) & 0xFF );
			}
	 
			vlong_io::PrintVlong( "\n[cPlain]",  cPlain );
		#endif

//printf( "numWords %d\n", numWords );
		numBytes = numWords*sizeof(*buf32);
		result = (U8*)malloc( numBytes + 1 );
		vlong_io::ToBytes( result, buf32, numBytes, true );

		outNumBytes = numBytes;

		#if 0
		for ( size_t i = 0; i < numBytes; i++ )
		{
			printf( "%c", result[i] );
		}
		printf( "\n" );
		#endif

		free( buf32 );
	}

	return result;
}

U8*
AuthorizationTicket::PlainToCipher( const private_key& keyPrivate, const char *plaintext, size_t& outNumBytes )
{
	U8 *result = NULL;
	const size_t numBytes = strlen( plaintext ) + 1; // Include '\0' termination character
	const unsigned int numWordsInKey = keyPrivate.get_m().numWords();
	if ( Rtt_VERIFY( numBytes < (numWordsInKey<< 2) ) )
	{
		unsigned int numBytes4 = ((unsigned int) numBytes + 3) & (~3);
		unsigned int numWords4 = numBytes4 >> 2;

		U32 *buf32 = (U32*)malloc( sizeof(U32)*numWordsInKey );
		vlong_io::ToWords( buf32, (const U8*)plaintext, numBytes );

		vlong cPlain;
		cPlain.load( (unsigned*)buf32, numWords4 ); Rtt_STATIC_ASSERT( sizeof( U32 ) == sizeof( unsigned ) );

		// Pad remaining plain msg with random bits.  Subtract 1 word since msg len < key len
		cPlain.pad( (numWordsInKey - 1) - numWords4 );

		#ifdef Rtt_DEBUG
			printf( "[plaintext] %s\n", plaintext );
			vlong_io::PrintVlong( "[cPlain]", cPlain );
		#endif

//		private_key keyPrivate( key.p, key.q );
		vlong c = keyPrivate.decrypt( cPlain );

		#ifdef Rtt_DEBUG
			vlong_io::PrintVlong( "[c     ]", c );
		#endif

		/*
		for ( size_t i = 0; i < numWordsInKey; i++ )
		{
			U32 value = buf32[i];
			printf( "%02x", value & 0xFF );
			printf( "%02x", (value>>8) & 0xFF );
			printf( "%02x", (value>>16) & 0xFF );
			printf( "%02x", (value>>24) & 0xFF );
		}
		 */	

		Rtt_ASSERT( numWordsInKey == c.numWords() );
		c.store( (unsigned*)buf32, numWordsInKey );

		size_t resultSize = numWordsInKey*sizeof(*buf32);
		result = (U8*)malloc( resultSize );
		vlong_io::ToBytes( result, buf32, resultSize, true );
		outNumBytes = resultSize;

		#ifdef Rtt_DEBUG
			// Print as hex (little-endian)
			for ( size_t i = 0; i < numWordsInKey; i++ )
			{
				// To get rid of annoying warnings, use unsigned int instead of U32
				unsigned int value = (unsigned int)buf32[i];
				Rtt_STATIC_ASSERT( sizeof(U32) == sizeof(unsigned int) );

				printf( "%02x", value & 0xFF );
				printf( "%02x", (value>>8) & 0xFF );
				printf( "%02x", (value>>16) & 0xFF );
				printf( "%02x", (value>>24) & 0xFF );
			}

			printf( "\n\n" );
		#endif

		#if 0
			public_key keyPublic( key.m, key.e );
			vlong cDecrypted = keyPublic.encrypt( c );

			vlong_io::PrintVlong( "[cPlain]", cPlain );
			vlong_io::PrintVlong( "[c     ]", c );
			PrintVlong( "[cDecrypted]", cDecrypted );

			cDecrypted.store( (unsigned*)buf32, cDecrypted.numWords() );
			// Print as hex (little-endian)
			for ( size_t i = 0; i < cDecrypted.numWords(); i++ )
			{
				U32 value = buf32[i];
				printf( "%02x", value & 0xFF );
				printf( "%02x", (value>>8) & 0xFF );
				printf( "%02x", (value>>16) & 0xFF );
				printf( "%02x", (value>>24) & 0xFF );
			}
		#endif

		free( buf32 );
	}

	return result;
}

U8*
AuthorizationTicket::CipherToPlainPrivate( const private_key& k, const char *ciphertext, size_t& outNumBytes )
{
	U8* result = NULL;

	size_t numBytes = strlen( ciphertext );
	// len is a multiple of 8 because we emit little-endian hex during encryption
	if ( Rtt_VERIFY( (numBytes & 0x7) == 0 ) )
	{
		// during serialization, each byte was emitted as a 2-character hex string.
		unsigned int numWords = (unsigned int) numBytes >> 3;
		U32 *buf32 = (U32*)malloc( numWords*sizeof(U32) );
		for ( size_t i = 0, j = 0; i < numBytes; i += 8, j++ )
		{
			Rtt_ASSERT( j < numWords );
			const char *bytes = ciphertext + i;
			// ciphertext was little-endian
			buf32[j] =
				vlong_io::HexToByte( bytes )
				| ( vlong_io::HexToByte( bytes+2 ) << 8 )
				| ( vlong_io::HexToByte( bytes+4 ) << 16 )
				| ( vlong_io::HexToByte( bytes+6 ) << 24 );
		}

		vlong c;
		c.load( (unsigned*)buf32, numWords ); Rtt_STATIC_ASSERT( sizeof( U32 ) == sizeof( unsigned ) );

		vlong cPlain = k.decrypt( c );

		//numWords = cPlain.numWords();
		cPlain.store( (unsigned*)buf32, numWords );

		numBytes = numWords*sizeof(*buf32);
		result = (U8*)malloc( numBytes + 1 );
		vlong_io::ToBytes( result, buf32, numBytes, true );

		outNumBytes = numBytes;

		free( buf32 );
	}

	return result;
}

U8*
AuthorizationTicket::PlainToCipherPublic( const public_key& k, const char *plaintext, size_t& outNumBytes )
{
	U8 *result = NULL;
	const size_t numBytes = strlen( plaintext ) + 1; // Include '\0' termination character
	const unsigned int numWordsInKey = k.get_m().numWords();
	if ( Rtt_VERIFY( numBytes < (numWordsInKey<< 2) ) )
	{
		const unsigned int numBytes4 = ((unsigned int) numBytes + 3) & (~3);
		const unsigned int numWords4 = numBytes4 >> 2;

		U32 *buf32 = (U32*)malloc( sizeof(U32)*numWordsInKey );
		vlong_io::ToWords( buf32, (const U8*)plaintext, numBytes );

		vlong cPlain;
		cPlain.load( (unsigned*)buf32, numWords4 ); Rtt_STATIC_ASSERT( sizeof( U32 ) == sizeof( unsigned ) );

		// Pad remaining plain msg with random bits.  Subtract 1 word since msg len < key len
		cPlain.pad( numWordsInKey - numWords4 - 1 );

		vlong c = k.encrypt( cPlain );

		Rtt_ASSERT( numWordsInKey == c.numWords() );
		c.store( (unsigned*)buf32, numWordsInKey );

		size_t resultSize = numWordsInKey*sizeof(*buf32);
		result = (U8*)malloc( resultSize );
		vlong_io::ToBytes( result, buf32, resultSize, true );
		outNumBytes = resultSize;

		#ifdef xRtt_DEBUG
			// Print as hex (little-endian)
			for ( size_t i = 0; i < numWordsInKey; i++ )
			{
				// To get rid of annoying warnings, use unsigned int instead of U32
				unsigned int value = (unsigned int)buf32[i];
				Rtt_STATIC_ASSERT( sizeof(U32) == sizeof(unsigned int) );

				printf( "%02x%02x%02x%02x", 
					(unsigned int)(value & 0xFF), 
					(unsigned int)((value>>8) & 0xFF), 
					(unsigned int)((value>>16) & 0xFF), 
					(unsigned int)((value>>24) & 0xFF) );
			}
			printf( "\n\n" );
		#endif

		free( buf32 );
	}

	return result;
}

int
AuthorizationTicket::StringToPlatform( const char *str )
{
	int result = kUnknownPlatform;

#if defined(Rtt_WIN_ENV )
	if ( 0 == _stricmp( str, "Mac OS X" ) )
	{
		result = kMacPlatform;
	}
	else if ( 0 == _stricmp( str, "Win" ) )
	{
		result = kWinPlatform;
	}
#else
	if ( 0 == strcasecmp( str, "Mac OS X" ) )
	{
		result = kMacPlatform;
	}
	else if ( 0 == strcasecmp( str, "Win" ) )
	{
		result = kWinPlatform;
	}
#endif

	return result;
}

/*
static void
ConvertHexStringToBytes( U8 *dst, const char *src, size_t numBytes )
{
	for ( size_t i = 0; i < numBytes; i++ )
	{
		char str[3];
		str[0] = *src++;
		str[1] = *src++;
		str[2] = '\0';
		long value = strtol( str, NULL, 16 ); Rtt_ASSERT( value <= 0xFF );
		*dst++ = (U8)value;
	}
}
*/

AuthorizationTicket::Subscription
AuthorizationTicket::VerifySubscriptionValue( unsigned int value )
{
	Subscription result = kTrialSubscription; // Default to trial mode

	switch ( value )
	{
		case kEnterpriseAutomatedBuildsSubscription:
		case kEnterpriseHostedTemplatesSubscription:
		case kEnterpriseNativeExtensionsSubscription:
		case kBasicSubscription:
		case kProSubscription:
		case kProPlusSubscription:
			result = (Subscription)value;
			break;
		case kIndieAndroidSubscriptionREMOVED:
		default:
			result = kTrialSubscription;
			break;
	}

	return result;
}

AuthorizationTicket::License
AuthorizationTicket::VerifyLicenseValue( unsigned int value )
{
	License result = kUnknownLicense;

	switch ( value )
	{
		case kIndividual:
		case kStudent:
		case kEnterprise:
			result = (License)value;
			break;
		default:
			result = kUnknownLicense;
			break;
	}

	return result;
}

AuthorizationTicket::PlatformAllowed
AuthorizationTicket::VerifyPlatformAllowedValue( unsigned int value )
{
	PlatformAllowed result = kUnknownPlatform;

	switch ( value )
	{
		case kMacPlatform:
		case kWinPlatform:
			result = (PlatformAllowed)value;
			break;
		default:
			result = kUnknownPlatform;
			break;
	}

	return result;
}

AuthorizationTicket::AuthorizationTicket( const MPlatformDevice& device, const char *data )
:	fDevice( device ),
	fExpiration( 0 ),
	fSubscription( 0 ),
	fLicense( kUnknownLicense ),
	fPlatformAllowed( kUnknownPlatform ),
	fIsActive( 0 ),
	fUid( 0 ),
	fUsername( NULL ),
#ifdef Rtt_GAMEEDITION
	fProduct( kCoronaGameEdition )
#else
	fProduct( kCoronaSDK )
#endif
{
	char md5buffer[sizeof(fDeviceId)];
	char username[128];
	unsigned int exp, sub, lic, pla, sta, uid;

	if ( Rtt_VERIFY( sscanf( data, "%s %x %x %x %x %x %x %s",
			md5buffer,
			& exp,
            & sub, & lic, & pla, & sta,
			& uid,
			username ) == 8 ) )
	{
		fExpiration = (S32)exp; Rtt_ASSERT( fExpiration > 0 );
		fSubscription = VerifySubscriptionValue( sub );
		fLicense = VerifyLicenseValue( lic );
		fPlatformAllowed = VerifyPlatformAllowedValue( pla );
		fIsActive = sta;
		fUid = uid;

		memcpy( fDeviceId, md5buffer, sizeof( fDeviceId ) );

		// lowercase the username so that we aren't sensitive to case differences
		// when they log into the Simulator
		for (size_t i = 0; i < strlen(username); i++)
		{
			username[i] = tolower(username[i]);
		}

		CopyUsername( username );
	}
	else
	{
#if defined( Rtt_WIN_ENV ) || defined( Rtt_ANDROID_ENV ) 
		memset( fDeviceId, 0, sizeof( fDeviceId ) );
#else
		bzero( fDeviceId, sizeof( fDeviceId ) );
#endif
	}
}

AuthorizationTicket::AuthorizationTicket( const AuthorizationTicket& rhs )
:	fDevice( rhs.fDevice ),
	fExpiration( rhs.fExpiration ),
	fSubscription( rhs.fSubscription ),
	fLicense( rhs.fLicense ),
	fPlatformAllowed( rhs.fPlatformAllowed ),
	fIsActive( rhs.fIsActive ),
	fUid( rhs.fUid ),
	fUsername( NULL ),
	fProduct( rhs.fProduct )
{
	memcpy( fDeviceId, rhs.fDeviceId, sizeof( fDeviceId ) );
	CopyUsername( rhs.fUsername );
}

AuthorizationTicket::~AuthorizationTicket()
{
	if ( fUsername )
	{
		free( fUsername );
	}
}

AuthorizationTicket&
AuthorizationTicket::operator=( const AuthorizationTicket& rhs )
{
	if ( this != & rhs )
	{
		memcpy( fDeviceId, rhs.fDeviceId, sizeof( fDeviceId ) );
		fExpiration = rhs.fExpiration;
		fSubscription = rhs.fSubscription;
		fLicense = rhs.fLicense;
		fPlatformAllowed = rhs.fPlatformAllowed;
		fIsActive = rhs.fIsActive;
		fUid = rhs.fUid;
		fProduct = rhs.fProduct;
		CopyUsername( rhs.fUsername );
	}

	return *this;
}

void
AuthorizationTicket::CopyUsername( const char *username )
{
	if ( username )
	{
		if ( fUsername ) { free( fUsername ); }
		const size_t kUsernameSize = strlen( username ) + 1;
		fUsername = (char*)malloc( kUsernameSize );
		memcpy( fUsername, username, kUsernameSize );
	}
}

/*
bool
AuthorizationTicket::operator==( const AuthorizationTicket& rhs ) const
{
	return Rtt::operator==( fData, rhs.fData );
}

bool
AuthorizationTicket::operator==( const AuthorizationTicketData& rhs ) const
{
	return Rtt::operator==( fData, rhs );
}
*/

AuthorizationTicket::AuthorizationTicket( const MPlatformDevice& device )
	:	fDevice( device )
{
#if defined( Rtt_WIN_ENV ) || defined( Rtt_ANDROID_ENV )
	memset( fDeviceId, 0, sizeof( fDeviceId ) );
#else
	bzero( fDeviceId, sizeof( fDeviceId ) );
#endif
	fExpiration = (S32) time(NULL) + 10000000;  // far in the future
	fSubscription = kTrialSubscription;
	fLicense = kUnknownLicense;
	fPlatformAllowed = kUnknownPlatform;
	fIsActive = true;
	fUid = 0;
	fProduct = kCoronaSDK;
	fUsername = strdup( "<none>" );
}

	
// If returns false, then run in trial mode
// TODO: Rename this to something more descriptive/suggestive
bool
AuthorizationTicket::IsAllowedToBuild( TargetDevice::Platform platform ) const
{
#if OLD_SUBSCRIPTION_CODE
	bool result = false;
	
	switch ( fSubscription )
	{
		case kIndieAndroidSubscriptionREMOVED:
			result = ( TargetDevice::kAndroidPlatform == platform );
			break;
		case kTrialSubscription: // Now Starter subscription
		case kBasicSubscription:
		case kProSubscription:
			result = ( TargetDevice::kIPhonePlatform == platform
					  || TargetDevice::kAndroidPlatform == platform
					  || TargetDevice::kKindlePlatform == platform
					  || TargetDevice::kNookPlatform == platform );
			break;
		case kEnterpriseAutomatedBuildsSubscription:
		case kEnterpriseHostedTemplatesSubscription:
		case kEnterpriseNativeExtensionsSubscription:
		case kProPlusSubscription:
			result = true;
			break;
		default:
			break;
	}
	
	return result;
#endif

	// Concept doesn't exist anymore
	return true;
}

// Calculates a delay to be introduced before the build operation begins.
// Subscribers get priority with no delay versus trial users have random delays.
int
AuthorizationTicket::GetBuildWaitTime( TargetDevice::Platform platform ) const
{
	int result = 0;
	bool isPriorityBuild = (fSubscription != kTrialSubscription);

	// If not subscribed for the platform, then treat as trial build
	if ( ! isPriorityBuild )
	{
		result = CalculateBuildDelayTimeInSeconds( platform );
	}

	return result;
}


// @return Returns the delay time in seconds.
U32
AuthorizationTicket::CalculateBuildDelayTimeInSeconds( TargetDevice::Platform platform ) const
{
	U32 delayInSeconds;

	// Calculate a random delay for Starter subscribers (no delay ~10% of the time, 5-15 seconds otherwise)
	const U32 kMinDelay = 0;
	const U32 kMaxDelay = 10;
	Rtt_STATIC_ASSERT( kMinDelay < kMaxDelay );
	delayInSeconds = (float)( kMaxDelay ) * (float)rand() / RAND_MAX;
	
	if (delayInSeconds != 0)
	{
		delayInSeconds += 15;
	}
	
	return delayInSeconds;
}

#if OLD_SUBSCRIPTION_CODE
bool
AuthorizationTicket::IsSkinAllowed( TargetDevice::Skin skin ) const
{
	bool result = false;
	
	// Special short circuit so we can run the demos
	if ( TargetDevice::kDemoSkin == skin )
	{
		return true;
	}
	
#if !defined( Rtt_SHELL_TOOLS )
	switch ( fSubscription )
	{
		case kEnterpriseAutomatedBuildsSubscription:
		case kEnterpriseHostedTemplatesSubscription:
		case kEnterpriseNativeExtensionsSubscription:
		case kProPlusSubscription:
			result = TargetDevice::IsSkinForPlatform( skin, TargetDevice::kIPhonePlatform )
						|| TargetDevice::IsSkinForPlatform( skin, TargetDevice::kAndroidPlatform )
						|| TargetDevice::IsSkinForPlatform( skin, TargetDevice::kKindlePlatform )
						|| TargetDevice::IsSkinForPlatform( skin, TargetDevice::kNookPlatform )
						|| TargetDevice::IsSkinForPlatform( skin, TargetDevice::kMacPlatform );
			break;
		case kProSubscription:
		case kBasicSubscription:
		case kIndieAndroidSubscriptionREMOVED:
		case kTrialSubscription:
			result = TargetDevice::IsSkinForPlatform( skin, TargetDevice::kIPhonePlatform )
						|| TargetDevice::IsSkinForPlatform( skin, TargetDevice::kAndroidPlatform )
						|| TargetDevice::IsSkinForPlatform( skin, TargetDevice::kKindlePlatform )
						|| TargetDevice::IsSkinForPlatform( skin, TargetDevice::kNookPlatform );
			break;
		default:
			result = false;
			break;
	}
#endif

	return result;
}
#endif
    
bool
AuthorizationTicket::IsDailyBuildAllowed() const
{
#if OLD_SUBSCRIPTION_CODE
	// Check for an active subscription
	bool result = ! IsExpired();
	
	// Then restrict to certain subscription tiers
	if ( result )
	{
		switch ( fSubscription )
		{
			case kEnterpriseAutomatedBuildsSubscription:
			case kEnterpriseHostedTemplatesSubscription:
			case kEnterpriseNativeExtensionsSubscription:
			case kProPlusSubscription:
			case kProSubscription:
				result = true;
				break;
			default:
				result = false;
				break;
		}
	}

	return result;
#endif

	// Everyone can use Daily Builds now
	return true;
}

float
AuthorizationTicket::GetNumDaysLeft() const
{
#if OLD_SUBSCRIPTION_CODE
	time_t current = time( NULL );
	time_t expiration = GetExpiration();
	const time_t kNumSecondsPerDay = 24*60*60;
	float numDaysLeft = ( expiration > current ? ((float)expiration - current)/kNumSecondsPerDay : 0 );
	return numDaysLeft;
#endif

	// Builds last forever now
	return 1000;
}

bool
AuthorizationTicket::IsPaidSubscription() const
{
#if OLD_SUBSCRIPTION_CODE
	return DoesSubscriptionIncludeProduct();
#endif

	// All subscriptions are now "paid" for the purposes of this call (the above code didn't answer the question anyway)
	return true;
}

bool
AuthorizationTicket::DoesSubscriptionIncludeProduct() const
{
	bool result = false;

	switch ( fProduct )
	{
		case kCoronaGameEdition:
			result = ( kEnterpriseAutomatedBuildsSubscription == fSubscription
						|| kEnterpriseHostedTemplatesSubscription == fSubscription
						|| kEnterpriseNativeExtensionsSubscription == fSubscription
						|| kProPlusSubscription == fSubscription
						|| kProSubscription == fSubscription
						|| kBasicSubscription == fSubscription
						|| kIndieAndroidSubscriptionREMOVED == fSubscription );
			break;
		case kCoronaSDK:
			// GameEdition subscription is a superset of CoronaSDK subscription
			result = ( kEnterpriseAutomatedBuildsSubscription == fSubscription
						|| kEnterpriseHostedTemplatesSubscription == fSubscription
						|| kEnterpriseNativeExtensionsSubscription == fSubscription
						|| kBasicSubscription == fSubscription
						|| kProPlusSubscription == fSubscription
						|| kProSubscription == fSubscription
						|| kBasicSubscription == fSubscription
						|| kIndieAndroidSubscriptionREMOVED == fSubscription );
			break;
		default:
			break;
	}

	return result;
}

bool
AuthorizationTicket::IsAppAllowedToRun() const
{
#if OLD_SUBSCRIPTION_CODE
	// Old expiration logic
	bool result = false;
	if ( MatchesCommon() )
	{
		if ( IsTrial() || IsUpsell() )
		{
			// For trial users, can only use Corona until ticket expires.
			result = ! IsExpired();
		}
		else if ( DoesSubscriptionIncludeProduct() )
		{
			time_t buildTimestamp = Rtt_VersionTimestamp();

			// 1 day grace period, so subtract 1 day's worth of seconds
			time_t buildTimestampWithGracePeriod = buildTimestamp - 60*60*24;

			// If the user had paid previously, the ticket is only valid
			// for updated versions of Corona available *before* the expiration
			// of the ticket.
			result = Rtt_VERIFY( buildTimestamp >= 0 )
						&& ( buildTimestampWithGracePeriod <= fExpiration );
		}
	}
#else
	bool result = true; // Simplified: DPC
#endif

	return result;
}

bool
AuthorizationTicket::IsTrial() const
{
#if OLD_SUBSCRIPTION_CODE
	return kTrialSubscription == fSubscription || kUnknownSubscription == fSubscription;
#endif

	// There are no trials anymore
	return false;
}

bool
AuthorizationTicket::IsPlaceholder() const
{
#if OLD_SUBSCRIPTION_CODE
	return (fUsername == NULL || (fUsername != NULL && strcmp(fUsername, "<none>") == 0));
#endif

	// There are no placeholders anymore
	return false;
}

bool
AuthorizationTicket::IsExpired() const
{
#if OLD_SUBSCRIPTION_CODE
	const time_t kCurrentTime = time( NULL );
	bool result = Rtt_VERIFY( kCurrentTime > 0 ) && fExpiration <= kCurrentTime;

	// For trial users, there is an additional requirement that the
	// expiration date must be within a 31 day window of the current system time
	// (otherwise, we consider the ticket as expired, since the current time
	// is suspicious.)
	if ( ! result && IsTrial() )
	{
		const time_t kDaysInTrial = 31;
		const time_t kSecondsInTrial = kDaysInTrial*24*60*60;
		result = kCurrentTime < (fExpiration - kSecondsInTrial);
	}

	return result;
#endif

	// Subscriptions don't expire anymore
	return false;
}

bool
AuthorizationTicket::MatchesDeviceIdentifier() const
{
	return (strcmp(fDevice.GetUniqueIdentifier( MPlatformDevice::kDeviceIdentifier ), fDeviceId) == 0);
}

bool
AuthorizationTicket::MatchesPlatform() const
{
	return ( kUnknownPlatform != fPlatformAllowed )
				&& StringToPlatform( fDevice.GetPlatformName() ) == fPlatformAllowed;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

