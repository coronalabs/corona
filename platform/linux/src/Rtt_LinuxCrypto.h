//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_MCrypto.h"

namespace Rtt
{
	class LinuxCrypto : public MCrypto
	{
	public:
		virtual size_t GetDigestLength(Algorithm algorithm) const;
		virtual void CalculateDigest(Algorithm algorithm, const Rtt::Data<const char> &data, U8 *md) const;
		virtual void CalculateHMAC(Algorithm algorithm, const Rtt::Data<const char> &key, const Rtt::Data<const char> & data, U8 *outMac) const;
	};
}; // namespace Rtt
