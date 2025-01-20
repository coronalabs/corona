//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_EmscriptenCrypto.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#pragma region Public Member Functions
size_t EmscriptenCrypto::GetDigestLength(Algorithm algorithm) const
{
	size_t result = 0;
	return result;
}

void EmscriptenCrypto::CalculateDigest(Algorithm algorithm, const Rtt::Data<const char> & data, U8 *digest) const
{
}

void EmscriptenCrypto::CalculateHMAC(Algorithm algorithm, const Rtt::Data<const char> & key, const Rtt::Data<const char> & data, U8 *digest) const
{
}

#pragma endregion

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
