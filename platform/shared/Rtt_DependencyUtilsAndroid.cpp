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

