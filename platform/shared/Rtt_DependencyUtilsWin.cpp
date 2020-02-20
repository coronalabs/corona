//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core\Rtt_Build.h"
#include "Rtt_DependencyUtils.h"
#include "Core\Rtt_Data.h"
#include "CryptoPP\base64.h"
#include "CryptoPP\filters.h"
#include "CryptoPP\rsa.h"
#include <iostream>


namespace Corona
{

int DependencyUtils::DecodeBase64(lua_State *L)
{
	if (lua_type(L, 1) == LUA_TSTRING)
	{
		std::string decodedString;
		CryptoPP::StringSource base64Decoder(
				(const byte*)lua_tostring(L, 1), lua_objlen(L, 1),
				true, new CryptoPP::Base64Decoder(new CryptoPP::StringSink(decodedString)));
		lua_pushlstring(L, decodedString.data(), decodedString.length());
	}
	else
	{
		lua_pushnil(L);
	}
	return 1;
}

int DependencyUtils::Check(lua_State *L)
{
	// Fetch the public RSA key in binary form.
	CryptoPP::StringSource publicKeyDecoder(
			(const byte*)lua_tostring(L, lua_upvalueindex(1)), lua_objlen(L, lua_upvalueindex(1)),
			true, new CryptoPP::Base64Decoder());
	CryptoPP::RSA::PublicKey publicKey;
	publicKey.Load(publicKeyDecoder);

	// Fetch the license file's signature in binary form.
	std::string signatureString;
	CryptoPP::StringSource signatureDecoder(
			(const byte*)lua_tostring(L, 1), lua_objlen(L, 1),
			true, new CryptoPP::Base64Decoder(new CryptoPP::StringSink(signatureString)));
	
	// Verify that the signature for the license data is valid.
	CryptoPP::RSASSA_PKCS1v15_SHA_Verifier verifier(publicKey);
	bool isValid = verifier.VerifyMessage(
						(const byte*)lua_tostring(L, 2), lua_objlen(L, 2),
						(const byte*)signatureString.data(), signatureString.length());
	
	// Return the result to Lua.
	lua_pushboolean(L, isValid ? 1 : 0);
	return 1;
}

} // namespace Corona
