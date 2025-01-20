//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"

#include "Rtt_AndroidCrypto.h"
#include "NativeToJavaBridge.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static const char kMD4AlgorithmParam[] = "MD4";
static const char kMD5AlgorithmParam[] = "MD5";
static const char kSHA1AlgorithmParam[] = "SHA1";
static const char kSHA256lgorithmParam[] = "SHA256";
static const char kSHA384AlgorithmParam[] = "SHA384";
static const char kSHA512AlgorithmParam[] = "SHA512";
static const char kSHA224AlgorithmParam[] = "SHA224";

static const char kHmacMD4AlgorithmParam[] = "HmacMD4";
static const char kHmacMD5AlgorithmParam[] = "HmacMD5";
static const char kHmacSHA1AlgorithmParam[] = "HmacSHA1";
static const char kHmacSHA256lgorithmParam[] = "HmacSHA256";
static const char kHmacSHA384AlgorithmParam[] = "HmacSHA384";
static const char kHmacSHA512AlgorithmParam[] = "HmacSHA512";
static const char kHmacSHA224AlgorithmParam[] = "HmacSHA224";

AndroidCrypto::AndroidCrypto( NativeToJavaBridge *ntjb)
: fNativeToJavaBridge(ntjb)
{
}

size_t
AndroidCrypto::GetDigestLength( Algorithm algorithm ) const
{
	size_t result = 0;
	const char * algorithmParam = NULL;

	switch ( algorithm )
	{
	case MCrypto::kMD4Algorithm:
		algorithmParam = kMD4AlgorithmParam;
		break;
	case MCrypto::kMD5Algorithm:
		algorithmParam = kMD5AlgorithmParam;
		break;
	case MCrypto::kSHA1Algorithm:
		algorithmParam = kSHA1AlgorithmParam;
		break;
	case MCrypto::kSHA256Algorithm:
		algorithmParam = kSHA256lgorithmParam;
		break;
	case MCrypto::kSHA384Algorithm:
		algorithmParam = kSHA384AlgorithmParam;
		break;
	case MCrypto::kSHA512Algorithm:
		algorithmParam = kSHA512AlgorithmParam;
		break;
	case MCrypto::kSHA224Algorithm:
		algorithmParam = kSHA224AlgorithmParam;;
		break;
	default:
		Rtt_ASSERT_NOT_REACHED();
		break;
	}

	if ( algorithmParam != NULL )
	{
		result = fNativeToJavaBridge->CryptoGetDigestLength( algorithmParam );
	}
		

	return result;
}

void
AndroidCrypto::CalculateDigest( Algorithm algorithm, const Rtt::Data<const char> & data, U8 *digest ) const
{
	const char * algorithmParam = NULL;

	switch ( algorithm )
	{
	case MCrypto::kMD4Algorithm:
		algorithmParam = kMD4AlgorithmParam;
		break;
	case MCrypto::kMD5Algorithm:
		algorithmParam = kMD5AlgorithmParam;
		break;
	case MCrypto::kSHA1Algorithm:
		algorithmParam = kSHA1AlgorithmParam;
		break;
	case MCrypto::kSHA256Algorithm:
		algorithmParam = kSHA256lgorithmParam;
		break;
	case MCrypto::kSHA384Algorithm:
		algorithmParam = kSHA384AlgorithmParam;
		break;
	case MCrypto::kSHA512Algorithm:
		algorithmParam = kSHA512AlgorithmParam;
		break;
	case MCrypto::kSHA224Algorithm:
		algorithmParam = kSHA224AlgorithmParam;;
		break;
	default:
		Rtt_ASSERT_NOT_REACHED();
		break;
	}
	fNativeToJavaBridge->CryptoCalculateDigest( algorithmParam, data, digest );
}

void
AndroidCrypto::CalculateHMAC( Algorithm algorithm, const Rtt::Data<const char> & key, const Rtt::Data<const char> & data, U8 *digest ) const
{
	const char * algorithmParam = NULL;

	switch ( algorithm )
	{
	case MCrypto::kMD4Algorithm:
		algorithmParam = kHmacMD4AlgorithmParam;
		break;
	case MCrypto::kMD5Algorithm:
		algorithmParam = kHmacMD5AlgorithmParam;
		break;
	case MCrypto::kSHA1Algorithm:
		algorithmParam = kHmacSHA1AlgorithmParam;
		break;
	case MCrypto::kSHA256Algorithm:
		algorithmParam = kHmacSHA256lgorithmParam;
		break;
	case MCrypto::kSHA384Algorithm:
		algorithmParam = kHmacSHA384AlgorithmParam;
		break;
	case MCrypto::kSHA512Algorithm:
		algorithmParam = kHmacSHA512AlgorithmParam;
		break;
	case MCrypto::kSHA224Algorithm:
		algorithmParam = kHmacSHA224AlgorithmParam;;
		break;
	default:
		Rtt_ASSERT_NOT_REACHED();
		break;
	}
	fNativeToJavaBridge->CryptoCalculateHMAC( algorithmParam, key, data, digest );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

