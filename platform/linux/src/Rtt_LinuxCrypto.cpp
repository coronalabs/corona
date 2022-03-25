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
#include "openssl/hmac.h"
#include "openssl/md4.h"
#include "openssl/md5.h"
#include "openssl/sha.h"

namespace Rtt
{
	size_t LinuxCrypto::GetDigestLength(Algorithm algorithm) const
	{
		// Return the byte length of the hash for the given algorithm.
		switch (algorithm)
		{
			case kMD4Algorithm:
				return (size_t)MD4_DIGEST_LENGTH;
			case kMD5Algorithm:
				return (size_t)MD5_DIGEST_LENGTH;
			case kSHA1Algorithm:
				return (size_t)SHA_DIGEST_LENGTH;
			case kSHA224Algorithm:
				return (size_t)SHA224_DIGEST_LENGTH;
			case kSHA256Algorithm:
				return (size_t)SHA256_DIGEST_LENGTH;
			case kSHA384Algorithm:
				return (size_t)SHA384_DIGEST_LENGTH;
			case kSHA512Algorithm:
				return (size_t)SHA512_DIGEST_LENGTH;
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
				MD4((unsigned char*)(const U8*)data.GetData(), data.GetLength(), digest);
				break;
			}
			case kMD5Algorithm:
			{
				MD5((unsigned char*)(const U8*)data.GetData(), data.GetLength(), digest);
				break;
			}
			case kSHA1Algorithm:
			{
				SHA1((unsigned char*)(const U8*)data.GetData(), data.GetLength(), digest);
				break;
			}
			case kSHA224Algorithm:
			{
				SHA224((unsigned char*)(const U8*)data.GetData(), data.GetLength(), digest);
				break;
			}
			case kSHA256Algorithm:
			{
				SHA256((unsigned char*)(const U8*)data.GetData(), data.GetLength(), digest);
				break;
			}
			case kSHA384Algorithm:
			{
				SHA384((unsigned char*)(const U8*)data.GetData(), data.GetLength(), digest);
				break;
			}
			case kSHA512Algorithm:
			{
				SHA512((unsigned char*)(const U8*)data.GetData(), data.GetLength(), digest);
				break;
			}
		}
	}

	void LinuxCrypto::CalculateHMAC(Algorithm algorithm, const Rtt::Data<const char> &key, const Rtt::Data<const char> &data, U8 *digest) const
	{
		Rtt_LogException("todo: CalculateHMAC\n");
		// todo using openssl
		/*switch (algorithm)
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
		}*/
	}
}; // namespace Rtt
