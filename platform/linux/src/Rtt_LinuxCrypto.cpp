//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_LinuxCrypto.h"
#include "crypto++/hmac.h"
#include "crypto++/md4.h"
#include "crypto++/md5.h"
#include "crypto++/sha.h"

namespace Rtt
{
	size_t LinuxCrypto::GetDigestLength(Algorithm algorithm) const
	{
		// Return the byte length of the hash for the given algorithm.
		switch (algorithm)
		{
			case kMD4Algorithm:
				return (size_t)CryptoPP::Weak1::MD4::DIGESTSIZE;
			case kMD5Algorithm:
				return (size_t)CryptoPP::Weak1::MD5::DIGESTSIZE;
			case kSHA1Algorithm:
				return (size_t)CryptoPP::SHA1::DIGESTSIZE;
			case kSHA224Algorithm:
				return (size_t)CryptoPP::SHA224::DIGESTSIZE;
			case kSHA256Algorithm:
				return (size_t)CryptoPP::SHA256::DIGESTSIZE;
			case kSHA384Algorithm:
				return (size_t)CryptoPP::SHA384::DIGESTSIZE;
			case kSHA512Algorithm:
				return (size_t)CryptoPP::SHA512::DIGESTSIZE;
		}

		// The given algorithm is not supported or unknown.
		Rtt_LogException("The given crypto algorithm is not supported on this platform.\r\n");

		return 0;
	}

	void LinuxCrypto::CalculateDigest(Algorithm algorithm, const Rtt::Data<const char> &data, U8 *digest) const
	{
		switch (algorithm)
		{
			case kMD4Algorithm:
			{
				CryptoPP::Weak1::MD4 md4;
				md4.CalculateDigest(digest, (const U8*) data.GetData(), data.GetLength());
				break;
			}
			case kMD5Algorithm:
			{
				CryptoPP::Weak1::MD5 md5;
				md5.CalculateDigest(digest, (const U8*)data.GetData(), data.GetLength());
				break;
			}
			case kSHA1Algorithm:
			{
				CryptoPP::SHA1 sha;
				sha.CalculateDigest(digest, (const U8*)data.GetData(), data.GetLength());
				break;
			}
			case kSHA224Algorithm:
			{
				CryptoPP::SHA224 sha;
				sha.CalculateDigest(digest, (const U8*)data.GetData(), data.GetLength());
				break;
			}
			case kSHA256Algorithm:
			{
				CryptoPP::SHA256 sha;
				sha.CalculateDigest(digest, (const U8*)data.GetData(), data.GetLength());
				break;
			}
			case kSHA384Algorithm:
			{
				CryptoPP::SHA384 sha;
				sha.CalculateDigest(digest, (const U8*)data.GetData(), data.GetLength());
				break;
			}
			case kSHA512Algorithm:
			{
				CryptoPP::SHA512 sha;
				sha.CalculateDigest(digest, (const U8*)data.GetData(), data.GetLength());
				break;
			}
		}
	}

	void LinuxCrypto::CalculateHMAC(Algorithm algorithm, const Rtt::Data<const char> &key, const Rtt::Data<const char> &data, U8 *digest) const
	{
		switch (algorithm)
		{
			case kMD4Algorithm:
			{
				CryptoPP::HMAC<CryptoPP::Weak1::MD4> hmac((const U8*)key.GetData(), key.GetLength());
				hmac.CalculateDigest(digest, (const U8*)data.GetData(), data.GetLength());
				break;
			}
			case kMD5Algorithm:
			{
				CryptoPP::HMAC<CryptoPP::Weak1::MD5> hmac((const U8*)key.GetData(), key.GetLength());
				hmac.CalculateDigest(digest, (const U8*)data.GetData(), data.GetLength());
				break;
			}
			case kSHA1Algorithm:
			{
				CryptoPP::HMAC<CryptoPP::SHA1> hmac((const U8*)key.GetData(), key.GetLength());
				hmac.CalculateDigest(digest, (const U8*)data.GetData(), data.GetLength());
				break;
			}
			case kSHA224Algorithm:
			{
				CryptoPP::HMAC<CryptoPP::SHA224> hmac((const U8*)key.GetData(), key.GetLength());
				hmac.CalculateDigest(digest, (const U8*)data.GetData(), data.GetLength());
				break;
			}
			case kSHA256Algorithm:
			{
				CryptoPP::HMAC<CryptoPP::SHA256> hmac((const U8*)key.GetData(), key.GetLength());
				hmac.CalculateDigest(digest, (const U8*)data.GetData(), data.GetLength());
				break;
			}
			case kSHA384Algorithm:
			{
				CryptoPP::HMAC<CryptoPP::SHA384> hmac((const U8*)key.GetData(), key.GetLength());
				hmac.CalculateDigest(digest, (const U8*)data.GetData(), data.GetLength());
				break;
			}
			case kSHA512Algorithm:
			{
				CryptoPP::HMAC<CryptoPP::SHA512> hmac((const U8*)key.GetData(), key.GetLength());
				hmac.CalculateDigest(digest, (const U8*)data.GetData(), data.GetLength());
				break;
			}
		}
	}
}; // namespace Rtt
