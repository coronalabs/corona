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

#include "Rtt_Verifier.h"

#include "Core/Rtt_MD4.h"
#include "Core/Rtt_MessageDigest.h"

#include "Rtt_Message.h"
#include "Rtt_VerifierData.h"
#include "rsa.hpp"

#include <string.h>

//#include <sys/stat.h>

//#include "Rtt_MessageDigest.h"

//#ifdef Rtt_ANDROID_ENV
//#include <android/log.h>
//#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

bool
Verifier::IsValidApplication( const char* filename )
{
	// The signature read from the given file will be compared with the signature in this array.
	// Note: Corona's "app_sign" tool will overwrite the binary in this array after the Corona app/library is compiled.
	static volatile const U32 kData[] =
	{
		// Modulus: n = p*q
		0xBAADF00D, 0xBAADF01D, 0xBAADF02D, 0xBAADF03D, 0xBAADF04D, 0xBAADF05D, 0xBAADF06D, 0xBAADF07D, 
		0xBAADF08D, 0xBAADF09D, 0xBAADF10D, 0xBAADF11D, 0xBAADF12D, 0xBAADF13D, 0xBAADF14D, 0xBAADF15D,

		// Signature (expected)
		0xBAADF16D, 0xBAADF17D, 0xBAADF18D, 0xBAADF19D, 0xBAADF20D, 0xBAADF21D, 0xBAADF22D, 0xBAADF23D, 
		0xBAADF24D, 0xBAADF25D, 0xBAADF26D, 0xBAADF27D,	0xBAADF28D, 0xBAADF29D, 0xBAADF30D, 0xBAADF31D, 

		// Exponent: coprime/relative prime to (p-1)*(q-1)
		0xBAADF32D, 0xBAADF33D,

		// Random padding for JS signing.
		// ------------------------------
		// In JS, this block gets converted to a string of comma delimited 
		// numbers (decimal, base 10). For example, 0xBAADF00D becomes 
		// the string "13,240,173,186," in little endian order.
		//
		// In this case, app_sign searches for the equivalent JS string
		// and in the worst case, must replace this by numbers that are 
		// 3 digits, e.g. "200,255,100,400,".
		//
		// Thus if the actual number of 32-bit words in kData is 34,
		// then we need a buffer that's 544 chars in size:
		//    (34*4 bytes) * (4 chars per byte) = 544 chars
		// The 34 refers to the 32-bit words: modulus, sig, exp.
		//
		// Currently, kData converts to a string that's 522 chars in length,
		// so we need additional (dummy) padding to guarantee that there are 22
		// more characters in JS.
		// 
		// To do this, we add padding. Note that each byte is chosen so that 
		// it is >= 100, ensuring that in the JS string rep, it maximizes the
		// JS char padding.
		//
		// We add 4 words (64 chars of padding).
		// To guarantee consistency, the padded region must produce 16 (4 per word) unique elements
		// even after modification. Hence, we need not just 22 add'l characters of padding,
		// but at least 16*2 (2 is for a "0" followed by a comma ",") chars of space
		// to reproduce the elements in the padding. This works out b/c (22+16*2) < 64.
		0xBAADF3FD, 0xBAADF3FD, 0xBAADF3FD, 0xBAADF3FD
	};

	const volatile U32* kN = VerifierData::ModulusBase( kData ); // kData + kModulusIndex;
	const volatile U32* kC = VerifierData::SignatureBase( kData ); // kData + kSignatureIndex;
	const volatile U32* kE = VerifierData::ExponentBase( kData ); // kData + kExponentIndex;

#if defined ( Rtt_TVOS_ENV )
	// TODO: figure out how to fix validation with bitcode.
	return true;
#endif

#if defined(Rtt_EMSCRIPTEN_ENV) || defined(Rtt_LINUX_ENV)
	// TODO: figure out how to fix validation with emscripten & release.
	return true;
#endif
	
#if defined(Rtt_WIN_PHONE_ENV)
	//TODO: Remove this when we're able to support signing a "resource.car" on Windows for local builds.
	//      This is needed for CoronaLabs internal development (local builds) and Corona Enterprise builds.
	return true;
#endif

#if defined( Rtt_DEBUG ) && defined( Rtt_ANDROID_ENV )
	return true;
#endif

/*
	// Exponent: coprime/relative prime to (p-1)*(q-1)
	static const U32 kE[] = { 0x0000c353, 0x0000000 };

	// Modulus: n = p*q
	static const U32 kN[] =
	{
		0x6ff35c7f, 0xe63c292c, 0x8700e61c, 0x1bf2a049,
		0xc769ec26, 0x2c107b26, 0x06c0c539, 0xb8da6b9f,
		0x7b269cee, 0x80209082, 0xd96eaf32, 0x0a155759,
		0xb66eacf5, 0xa456507c, 0x42bd13df, 0x65fb3484
	};

	// Signature (expected)
	static const U32 kC[] =
	{
		0x50670ab5, 0xeeb8d16d, 0x85caab29, 0xb4cf122b,
		0x12f214d5, 0xfdd54a5b, 0xc7bff174, 0xec5e74ec,
		0x9964e41a, 0xb756360a, 0x20c6b09f, 0x3644f874,
		0x6c6e0267, 0xbf56bcc1, 0x2ea83eb1, 0x3436d6da
	};

	Rtt_STATIC_ASSERT( sizeof( kN ) == 16*sizeof(kN[0]) ); // must be 16 words
	Rtt_STATIC_ASSERT( sizeof( kE ) == 2*sizeof(kE[0]) ); // must be 2 words
	Rtt_STATIC_ASSERT( sizeof( kC ) == 16*sizeof(kC[0]) ); // must be 16 words

	vlong m; m.load( (unsigned int*)kN, sizeof(kN)/sizeof(kN[0]) );
	vlong e; e.load( (unsigned int*)kE, sizeof(kE)/sizeof(kE[0]) );
	vlong c; c.load( (unsigned int*)kC, sizeof(kC)/sizeof(kC[0]) );
*/
	vlong m; m.load( (unsigned int*)kN, VerifierData::kModulusLen );
	vlong e; e.load( (unsigned int*)kE, VerifierData::kExponentLen );
	vlong c; c.load( (unsigned int*)kC, VerifierData::kSignatureLen );

	public_key keyPublic( m, e );
	vlong cDecrypted = keyPublic.encrypt( c );

	const size_t kDigestLen = (MD4::kNumBytesDigest >> 2);
	const size_t kMsgBufferLen = kDigestLen + 1;

	U32 cDecryptedBuffer[kMsgBufferLen];
	cDecrypted.store( (unsigned*)cDecryptedBuffer, kMsgBufferLen );

	Message actual( filename );
	U32 cActualBuffer[kMsgBufferLen];
	MD4Decode( cActualBuffer, actual.Data().digest, MD4::kNumBytesDigest );

	// Make error for timestamp equivalent to one day
	const U32 kTimestampError = 24 * 60 * 60;
	U32 decryptedTimestamp = cDecryptedBuffer[kDigestLen];
	decryptedTimestamp = ( decryptedTimestamp > kTimestampError ? decryptedTimestamp - kTimestampError : 0 );

	// The actual timestamp must be later than the server-generated timestamp in the decrypted data
#if defined(Rtt_ANDROID_ENV)
	// TODO: Android timestamp test is broken in Message
	return Rtt_VERIFY( 0 == memcmp( cDecryptedBuffer, cActualBuffer, kDigestLen << 2 ) );
#else
	return Rtt_VERIFY( 0 == memcmp( cDecryptedBuffer, cActualBuffer, kDigestLen << 2 ) )
			&& Rtt_VERIFY( decryptedTimestamp <= actual.Data().timestamp );
#endif

#if 0
	bool result = Rtt_VERIFY( 0 == memcmp( cActualBuffer, cDecryptedBuffer, kMsgBufferLen << 2 ) );

#ifdef Rtt_DEBUG
	result |= true;
#endif

	return true;
#endif
}

/*
static bool
IsInternetConnectionAvailable()
{
	const char kAddress[] = "www.coronalabs.com";
	SCNetworkReachabilityRef target = SCNetworkReachabilityCreateWithName( NULL, kAddress );

	SCNetworkConnectionFlags flags;
	Boolean success = SCNetworkReachabilityGetFlags( target, & flags );
	if ( success )
	{
		success = (flags & kSCNetworkFlagsReachable) && !(flags & kSCNetworkFlagsConnectionRequired);
	}

	CFRelease( target );
	return success;
}

static bool
VerifyAccount()
{
	Rtt_ASSERT( IsInternetConnectionAvailable() );

	bool result = false;

	NSString* user = @"walter";
	NSString* password = @"smtmhmhy115";

	return result;
}
*/

bool
Verifier::IsValidSubscription( const char* filename )
{
	return true;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

