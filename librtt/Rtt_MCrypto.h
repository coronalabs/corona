//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MCrypto_H__
#define _Rtt_MCrypto_H__

#include "Core/Rtt_Data.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class MCrypto
{
	public:
		typedef enum _Algorithm
		{
			kMD4Algorithm = 0, // HMAC not available for MD4
			kMD5Algorithm,
			kSHA1Algorithm,
			kSHA224Algorithm,
			kSHA256Algorithm,
			kSHA384Algorithm,
			kSHA512Algorithm,

			kNumAlgorithms
		}
		Algorithm;

		enum
		{
			kMaxDigestSize = 64  // longest known SHA512
		};

	public:
		virtual size_t GetDigestLength( Algorithm algorithm ) const = 0;
		virtual void CalculateDigest( Algorithm algorithm, const Rtt::Data<const char> & data, U8 *md ) const = 0;
		virtual void CalculateHMAC( Algorithm algorithm, const Rtt::Data<const char> & key, const Rtt::Data<const char> & data, U8 *outMac ) const = 0;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MCrypto_H__
