//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_DependencyUtils.h"
#include "NativeToJavaBridge.h"
#include <android/log.h>
// ----------------------------------------------------------------------------

namespace Corona
{

// ----------------------------------------------------------------------------
		
int
DependencyUtils::DecodeBase64( lua_State *L )
{
	Rtt::Data<const char> payloadData( lua_tostring( L, 1 ), lua_objlen( L, 1 ) );
	Rtt::Data<char> decodedPayload(payloadData.Allocator());

	if (NativeToJavaBridge::DecodeBase64( payloadData, decodedPayload ))
	{
		lua_pushlstring(L, decodedPayload.Get(), decodedPayload.GetLength());
	}
	else
	{
		lua_pushnil(L);
	}
	return 1;
}

int
DependencyUtils::Check( lua_State *L )
{
	Rtt::Data<const char> publicKey( lua_tostring( L, lua_upvalueindex( 1 ) ), lua_objlen( L, lua_upvalueindex( 1 ) ) );
	Rtt::Data<const char> signature( lua_tostring( L, 1 ), lua_objlen( L, 1 ) );
	Rtt::Data<const char> payloadData( lua_tostring( L, 2 ), lua_objlen( L, 2 ) );
	
	// publicKey and payloadData are decoded while signature is encoded
	bool verified = NativeToJavaBridge::Check(publicKey, signature, payloadData);

	lua_pushboolean(L, verified);

	return 1;
}

// ----------------------------------------------------------------------------

} // namespace Corona

// ----------------------------------------------------------------------------

