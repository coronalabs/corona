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

#include "stdafx.h"

// Enables the Crypto++ library's MD4 and MD5 support without compiler warnings.
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include "Rtt_WinCrypto.h"
#include "Core\Rtt_Build.h"
#include "CryptoPP\hmac.h"
#include "CryptoPP\md4.h"
#include "CryptoPP\md5.h"
#include "CryptoPP\sha.h"


namespace Rtt
{

#pragma region Public Methods
size_t WinCrypto::GetDigestLength(Algorithm algorithm) const
{
	// Return the byte length of the hash for the given algorithm.
	switch (algorithm)
	{
		case kMD4Algorithm:			return (size_t)CryptoPP::Weak1::MD4::DIGESTSIZE;
		case kMD5Algorithm:			return (size_t)CryptoPP::Weak1::MD5::DIGESTSIZE;
		case kSHA1Algorithm:		return (size_t)CryptoPP::SHA1::DIGESTSIZE;
		case kSHA224Algorithm:		return (size_t)CryptoPP::SHA224::DIGESTSIZE;
		case kSHA256Algorithm:		return (size_t)CryptoPP::SHA256::DIGESTSIZE;
		case kSHA384Algorithm:		return (size_t)CryptoPP::SHA384::DIGESTSIZE;
		case kSHA512Algorithm:		return (size_t)CryptoPP::SHA512::DIGESTSIZE;
	}

	// The given algorithm is not supported or unknown.
	Rtt_LogException("The given crypto algorithm is not supported on this platform.\r\n");
	return 0;
}

void WinCrypto::CalculateDigest(Algorithm algorithm, const Rtt::Data<const char> &data, U8 *digest) const
{
	switch (algorithm)
	{
		case kMD4Algorithm:
		{
			CryptoPP::Weak1::MD4 md4;
			md4.CalculateDigest(digest, (const byte*)data.GetData(), data.GetLength());
			break;
		}
		case kMD5Algorithm:
		{
			CryptoPP::Weak1::MD5 md5;
			md5.CalculateDigest(digest, (const byte*)data.GetData(), data.GetLength());
			break;
		}
		case kSHA1Algorithm:
		{
			CryptoPP::SHA1 sha;
			sha.CalculateDigest(digest, (const byte*)data.GetData(), data.GetLength());
			break;
		}
		case kSHA224Algorithm:
		{
			CryptoPP::SHA224 sha;
			sha.CalculateDigest(digest, (const byte*)data.GetData(), data.GetLength());
			break;
		}
		case kSHA256Algorithm:
		{
			CryptoPP::SHA256 sha;
			sha.CalculateDigest(digest, (const byte*)data.GetData(), data.GetLength());
			break;
		}
		case kSHA384Algorithm:
		{
			CryptoPP::SHA384 sha;
			sha.CalculateDigest(digest, (const byte*)data.GetData(), data.GetLength());
			break;
		}
		case kSHA512Algorithm:
		{
			CryptoPP::SHA512 sha;
			sha.CalculateDigest(digest, (const byte*)data.GetData(), data.GetLength());
			break;
		}
	}
}

void WinCrypto::CalculateHMAC(
	Algorithm algorithm, const Rtt::Data<const char> &key, const Rtt::Data<const char> &data, U8 *digest) const
{
	switch (algorithm)
	{
		case kMD4Algorithm:
		{
			CryptoPP::HMAC<CryptoPP::Weak1::MD4> hmac((const byte*)key.GetData(), key.GetLength());
			hmac.CalculateDigest(digest, (const byte*)data.GetData(), data.GetLength());
			break;
		}
		case kMD5Algorithm:
		{
			CryptoPP::HMAC<CryptoPP::Weak1::MD5> hmac((const byte*)key.GetData(), key.GetLength());
			hmac.CalculateDigest(digest, (const byte*)data.GetData(), data.GetLength());
			break;
		}
		case kSHA1Algorithm:
		{
			CryptoPP::HMAC<CryptoPP::SHA1> hmac((const byte*)key.GetData(), key.GetLength());
			hmac.CalculateDigest(digest, (const byte*)data.GetData(), data.GetLength());
			break;
		}
		case kSHA224Algorithm:
		{
			CryptoPP::HMAC<CryptoPP::SHA224> hmac((const byte*)key.GetData(), key.GetLength());
			hmac.CalculateDigest(digest, (const byte*)data.GetData(), data.GetLength());
			break;
		}
		case kSHA256Algorithm:
		{
			CryptoPP::HMAC<CryptoPP::SHA256> hmac((const byte*)key.GetData(), key.GetLength());
			hmac.CalculateDigest(digest, (const byte*)data.GetData(), data.GetLength());
			break;
		}
		case kSHA384Algorithm:
		{
			CryptoPP::HMAC<CryptoPP::SHA384> hmac((const byte*)key.GetData(), key.GetLength());
			hmac.CalculateDigest(digest, (const byte*)data.GetData(), data.GetLength());
			break;
		}
		case kSHA512Algorithm:
		{
			CryptoPP::HMAC<CryptoPP::SHA512> hmac((const byte*)key.GetData(), key.GetLength());
			hmac.CalculateDigest(digest, (const byte*)data.GetData(), data.GetLength());
			break;
		}
	}
}

#pragma endregion

} // namespace Rtt
