//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_AppleCrypto_H__
#define _Rtt_AppleCrypto_H__

#include "Rtt_MCrypto.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AppleCrypto : public MCrypto
{
	public:
		virtual size_t GetDigestLength( Algorithm algorithm ) const;
		virtual void CalculateDigest( Algorithm algorithm, const Rtt::Data<const char> & data, U8 *md ) const;
		virtual void CalculateHMAC( Algorithm algorithm, const Rtt::Data<const char> & key, const Rtt::Data<const char> & data, U8 *outMac ) const;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AppleCrypto_H__
