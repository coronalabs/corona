//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_TextureResourceBitmapAdapter.h"
#include "Rtt_TextureResourceBitmap.h"
#include "Rtt_LuaContext.h"


namespace Rtt {
	
const TextureResourceBitmapAdapter&
TextureResourceBitmapAdapter::Constant()
{
	static const TextureResourceBitmapAdapter sAdapter;
	return sAdapter;
}

StringHash*
TextureResourceBitmapAdapter::GetHash( lua_State *L ) const
{
	static const char *keys[] =
	{
		"preload",            //0
		"wrapX",              //1
		"wrapY",              //2
	};
	
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 3, 1, 1, __FILE__, __LINE__ );
	return &sHash;
}

int
TextureResourceBitmapAdapter::ValueForKey(
								const LuaUserdataProxy& sender,
								lua_State *L,
								const char *key ) const
{
	int results = 0;
	
	Rtt_ASSERT( key ); // Caller should check at the top-most level
	
	const TextureResourceBitmap *entry = (const TextureResourceBitmap *)sender.GetUserdata();
	if ( ! entry ) { return results; }
	
	int index = GetHash( L )->Lookup( key );
	
	if ( index >= 0 )
	{
		switch ( index )
		{
			case 0:
				Lua::PushCachedFunction( L, Self::Preload );
				results = 1;
				break;

			case 1:
				if( entry->GetBitmap() )
				{
					RenderTypes::TextureWrap wrap = entry->GetBitmap()->GetWrapX();
					lua_pushstring( L, RenderTypes::StringForTextureWrap( wrap ) );
					results = 1;
				} // else return nothing - no texture was loaded yet.
				break;

			case 2:
				if( entry->GetBitmap() )
				{
					RenderTypes::TextureWrap wrap = entry->GetBitmap()->GetWrapY();
					lua_pushstring( L, RenderTypes::StringForTextureWrap( wrap ) );
					results = 1;
				} // else return nothing - no texture was loaded yet.
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
	
	return results;
}


int TextureResourceBitmapAdapter::Preload( lua_State *L )
{
	LuaUserdataProxy* sender = LuaUserdataProxy::ToProxy( L, 1 );
	if (sender)
	{
		TextureResourceBitmap *entry = (TextureResourceBitmap *)sender->GetUserdata();
		if (entry)
		{
			entry->Preload();
		}
	}
	return 0;

}

	
} // namespace Rtt
