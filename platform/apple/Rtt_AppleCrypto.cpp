//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_AppleCrypto.h"

#include <CommonCrypto/CommonDigest.h>
#include <CommonCrypto/CommonHMAC.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

template class Rtt::Data<const char>;

// ----------------------------------------------------------------------------

size_t
AppleCrypto::GetDigestLength( Algorithm algorithm ) const
{
	size_t result = 0;

	switch ( algorithm )
	{
		case MCrypto::kMD4Algorithm:
			result = CC_MD4_DIGEST_LENGTH;
			break;
		case MCrypto::kMD5Algorithm:
			result = CC_MD5_DIGEST_LENGTH;
			break;
		case MCrypto::kSHA1Algorithm:
			result = CC_SHA1_DIGEST_LENGTH;
			break;
		case MCrypto::kSHA256Algorithm:
			result = CC_SHA256_DIGEST_LENGTH;
			break;
		case MCrypto::kSHA384Algorithm:
			result = CC_SHA384_DIGEST_LENGTH;
			break;
		case MCrypto::kSHA512Algorithm:
			result = CC_SHA512_DIGEST_LENGTH;
			break;
		case MCrypto::kSHA224Algorithm:
			result = CC_SHA224_DIGEST_LENGTH;;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return result;
}

void
AppleCrypto::CalculateDigest( Algorithm algorithm, const Rtt::Data<const char> & data, U8 *md ) const
{
	switch ( algorithm )
	{
		case MCrypto::kMD4Algorithm:
			CC_MD4( data.GetData(), data.GetLength(), md );
			break;
		case MCrypto::kMD5Algorithm:
			CC_MD5( data.GetData(), data.GetLength(), md );
			break;
		case MCrypto::kSHA1Algorithm:
			CC_SHA1( data.GetData(), data.GetLength(), md );
			break;
		case MCrypto::kSHA256Algorithm:
			CC_SHA256( data.GetData(), data.GetLength(), md );
			break;
		case MCrypto::kSHA384Algorithm:
			CC_SHA384( data.GetData(), data.GetLength(), md );
			break;
		case MCrypto::kSHA512Algorithm:
			CC_SHA512( data.GetData(), data.GetLength(), md );
			break;
		case MCrypto::kSHA224Algorithm:
			CC_SHA224( data.GetData(), data.GetLength(), md );;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}
}

void
AppleCrypto::CalculateHMAC( Algorithm algorithm, const Rtt::Data<const char> & key, const Rtt::Data<const char> & data, U8 *outMac ) const
{
	CCHmacAlgorithm alg;

	switch ( algorithm )
	{
		case MCrypto::kMD5Algorithm:
			alg = kCCHmacAlgMD5;
			break;
		case MCrypto::kSHA1Algorithm:
			alg = kCCHmacAlgSHA1;
			break;
		case MCrypto::kSHA256Algorithm:
			alg = kCCHmacAlgSHA256;
			break;
		case MCrypto::kSHA384Algorithm:
			alg = kCCHmacAlgSHA384;
			break;
		case MCrypto::kSHA512Algorithm:
			alg = kCCHmacAlgSHA512;
			break;
		case MCrypto::kSHA224Algorithm:
			alg = kCCHmacAlgSHA224;
			break;
		case MCrypto::kMD4Algorithm:
			Rtt_TRACE_SIM( ( "WARNING: HMAC MD4 is not supported on iPhone\n" ) );
			// fall through
		default:
			Rtt_ASSERT_NOT_REACHED();
			goto exit_gracefully;
			break;
	}

	CCHmac( alg, key.GetData(), key.GetLength(), data.GetData(), data.GetLength(), outMac );

exit_gracefully:
	(void)0; // Need a no-op after the label; otherwise compiler error occurs.
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

