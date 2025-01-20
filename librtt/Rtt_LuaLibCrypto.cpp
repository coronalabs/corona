//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_LuaLibCrypto.h"

#include "Rtt_LuaAux.h"
#include "Rtt_LuaContext.h"

#include <string.h>

#include "Rtt_MCrypto.h"
#include "Rtt_MPlatform.h"

#include "CoronaLua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static void
PushDigest( lua_State *L, unsigned char *digest, unsigned int written, bool raw )
{
	if ( raw )
	{
		lua_pushlstring( L, (char *)digest, written );
	}
	else
	{
		char *hex = (char*)calloc( sizeof( char ), written*2 + 1 );
		for ( unsigned int i = 0; i < written; i++ )
		{
			char *p = hex;
			p += sprintf( hex + 2*i, "%02x", digest[i] );
		}
		lua_pushlstring( L, hex, written*2 );
		free( hex );
	}
}

static const char kAlgorithms[MCrypto::kNumAlgorithms + 1] = "0123456";

static int
digest( lua_State *L )
{
	S32 algorithm = EnumForUserdata( kAlgorithms, lua_touserdata( L, 1 ), MCrypto::kNumAlgorithms, -1 );

	int result = ( algorithm >= 0 );

	if ( result )
	{
		const MPlatform& platform = LuaContext::GetPlatform( L );
		const MCrypto& crypto = platform.GetCrypto();

		U8 digest[MCrypto::kMaxDigestSize];
		size_t digestLen = crypto.GetDigestLength( (MCrypto::Algorithm)algorithm );

		Rtt::Data<const char> data( luaL_checkstring( L, 2 ), (int) lua_strlen( L, 2 ) );
		
		crypto.CalculateDigest( (MCrypto::Algorithm)algorithm, data, digest );

		PushDigest( L, digest, (int) digestLen, lua_toboolean( L, 3 ) );
	}
	else
	{
		CoronaLuaError(L, "crypto.digest() unknown message digest algorithm. No bytes returned" );
	}

	return result;
}

static int
hmac( lua_State *L )
{
	S32 algorithm = EnumForUserdata( kAlgorithms, lua_touserdata( L, 1 ), MCrypto::kNumAlgorithms, -1 );

	int result = ( algorithm >= 0 );

	if ( result )
	{
		const MPlatform& platform = LuaContext::GetPlatform( L );
		const MCrypto& crypto = platform.GetCrypto();

		U8 digest[MCrypto::kMaxDigestSize];
		memset( digest, 0, sizeof( U8 ) * sizeof( MCrypto::kMaxDigestSize ) );
		size_t digestLen = crypto.GetDigestLength( (MCrypto::Algorithm)algorithm );

		Rtt::Data<const char> data( luaL_checkstring( L, 2 ), (int) lua_strlen( L, 2 ) );
		Rtt::Data<const char> key( luaL_checkstring( L, 3 ), (int) lua_strlen( L, 3 ) );
		crypto.CalculateHMAC( (MCrypto::Algorithm)algorithm, key, data, digest );

		PushDigest( L, digest, (int) digestLen, lua_toboolean( L, 4 ) );
	}
	else
	{
		CoronaLuaError( L, "crypto.hmac() unknown HMAC digest algorithm. No bytes returned" );
	}

	return result;
}

int
LuaLibCrypto::Open( lua_State *L )
{
	const luaL_Reg kVTable[] =
	{
		{ "digest", digest },
		{ "hmac", hmac },

		{ NULL, NULL }
	};

	luaL_register( L, "crypto", kVTable );
	{
		lua_pushlightuserdata( L, UserdataForEnum( kAlgorithms, MCrypto::kMD4Algorithm ) );
		lua_setfield( L, -2, "md4" );
		lua_pushlightuserdata( L, UserdataForEnum( kAlgorithms, MCrypto::kMD5Algorithm ) );
		lua_setfield( L, -2, "md5" );
		lua_pushlightuserdata( L, UserdataForEnum( kAlgorithms, MCrypto::kSHA1Algorithm ) );
		lua_setfield( L, -2, "sha1" );
		lua_pushlightuserdata( L, UserdataForEnum( kAlgorithms, MCrypto::kSHA224Algorithm ) );
		lua_setfield( L, -2, "sha224" );
		lua_pushlightuserdata( L, UserdataForEnum( kAlgorithms, MCrypto::kSHA256Algorithm ) );
		lua_setfield( L, -2, "sha256" );
		lua_pushlightuserdata( L, UserdataForEnum( kAlgorithms, MCrypto::kSHA384Algorithm ) );
		lua_setfield( L, -2, "sha384" );
		lua_pushlightuserdata( L, UserdataForEnum( kAlgorithms, MCrypto::kSHA512Algorithm ) );
		lua_setfield( L, -2, "sha512" );
	}

	return 1;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

