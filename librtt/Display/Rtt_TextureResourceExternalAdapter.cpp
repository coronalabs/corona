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

#include "Rtt_TextureResourceExternalAdapter.h"
#include "Rtt_TextureResourceExternal.h"
#include "Rtt_LuaContext.h"

#include "Rtt_TextureFactory.h"
#include "Rtt_Display.h"


namespace Rtt {
	
const TextureResourceExternalAdapter&
TextureResourceExternalAdapter::Constant()
{
	static const TextureResourceExternalAdapter sAdapter;
	return sAdapter;
}

StringHash*
TextureResourceExternalAdapter::GetHash( lua_State *L ) const
{
	static const char *keys[] =
	{
		"width",            //0
		"height",           //1
		"invalidate"		//2
	};
	
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 3, 1, 1, __FILE__, __LINE__ );
	return &sHash;
}

int
TextureResourceExternalAdapter::ValueForKey(
								const LuaUserdataProxy& sender,
								lua_State *L,
								const char *key ) const
{
	int results = 0;
	
	Rtt_ASSERT( key ); // Caller should check at the top-most level
	
	const TextureResourceExternal *entry = (const TextureResourceExternal *)sender.GetUserdata();
	if ( ! entry ) { return results; }
	
	int index = GetHash( L )->Lookup( key );
	
	if ( index >= 0 )
	{
		switch ( index )
		{
			case 0:
				if( entry->GetBitmap() )
				{
					lua_pushnumber( L, entry->GetBitmap()->Width());
					results = 1;
				}
				break;
			case 1:
				if( entry->GetBitmap() )
				{
					lua_pushnumber( L, entry->GetBitmap()->Height());
					results = 1;
				}
				break;
			case 2:
				Lua::PushCachedFunction( L, Self::Invalidate );
				results = 1;
				break;

			default:
				Rtt_ASSERT_NOT_REACHED();
				break;
		}
	}
	else
	{
		results = Super::Constant().ValueForKey( sender, L, key );
	}
	
	if (results == 0)
	{
		results = entry->GetField(L, key);
	}
	
	return results;
}


int TextureResourceExternalAdapter::Invalidate( lua_State *L )
{
	LuaUserdataProxy* sender = LuaUserdataProxy::ToProxy( L, 1 );
	if (sender)
	{
		TextureResourceExternal *entry = (TextureResourceExternal *)sender->GetUserdata();
		if (entry)
		{
			entry->GetTextureFactory().GetDisplay().Invalidate();
			entry->GetTexture().Invalidate();
		}
	}
	return 0;
}
	

void* TextureResourceExternalAdapter::GetUserData( lua_State*L, int index )
{
	void *ret = NULL;
	LuaUserdataProxy* sender = LuaUserdataProxy::ToProxy( L, 1 );
	if (sender && sender->GetAdapter() == &Constant())
	{
		TextureResourceExternal *entry = (TextureResourceExternal *)sender->GetUserdata();
		ret = entry->GetUserData();
	}
	return ret;
}
	
} // namespace Rtt
