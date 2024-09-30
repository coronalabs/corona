//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_TextureResourceAdapter.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibSystem.h"

#include "Rtt_TextureResource.h"

namespace Rtt
{
	
TextureResourceAdapter::TextureResourceAdapter()
{
}

const TextureResourceAdapter&
TextureResourceAdapter::Constant()
{
	static const TextureResourceAdapter sAdapter;
	return sAdapter;
}

// Return the hash table containing the adapter's property keys
StringHash *TextureResourceAdapter::GetHash( lua_State *L ) const
{
	static const char *keys[] = {
		"filename",           //0
		"baseDir",            //1
		"type",               //2
		"releaseSelf",        //3
	};
	
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof( const char * ), 4, 1, 1, __FILE__, __LINE__ );
	return &sHash;
	
}

int TextureResourceAdapter::ValueForKey(const LuaUserdataProxy& sender,
									  lua_State *L,
									  const char *key ) const
{
	int results = 0;
	Rtt_ASSERT(key);
	
	TextureResource *entry = (TextureResource *)sender.GetUserdata();
	if (!entry) { return results; }
	
	int index = GetHash( L )->Lookup( key );
	if ( index < 0 ) { return results; }
	
	switch (index) {
		case 0: //filename
			lua_pushstring( L, entry->GetFilename() );
			results = 1;
			break;

		case 1: //baseDir
			lua_pushlightuserdata( L, UserdataForEnum( LuaLibSystem::Directories(), entry->GetDirectory() ) );
			results = 1;
			break;

		case 2: //type
			switch (entry->GetType())
			{
				case TextureResource::kTextureResourceBitmap:
					lua_pushstring( L, "image" );
					break;
				case TextureResource::kTextureResourceCanvas:
					lua_pushstring( L, "canvas" );
					break;
				case TextureResource::kTextureResourceCapture:
					lua_pushstring( L, "capture" );
					break;
				case TextureResource::kTextureResourceExternal:
					lua_pushstring( L, "external" );
					break;
				case TextureResource::kTextureResource_Any:
				default:
					lua_pushstring( L, "unknown" );
					Rtt_ASSERT_NOT_REACHED();
					break;
			}
			results = 1;
			break;

		case 3:
			Lua::PushCachedFunction( L, Self::ReleaseStrongReference );
			results = 1;
			break;
			
			
		default:
			Rtt_ASSERT_NOT_REACHED();
		break;
	}
	
	return results;
}

bool TextureResourceAdapter::SetValueForKey(LuaUserdataProxy& sender,
										  lua_State *L,
										  const char *key,
										  int valueIndex ) const
{
	//all properties are readonly
	return false;
}

// This is invoked when sender's object is about to be deleted
void TextureResourceAdapter::WillFinalize( LuaUserdataProxy& sender ) const
{
	TextureResource *entry = (TextureResource *)sender.GetUserdata();
	if ( ! entry ) { return; }
	
	entry->DetachProxy();

}

	
int TextureResourceAdapter::ReleaseStrongReference( lua_State *L )
{
	LuaUserdataProxy* sender = LuaUserdataProxy::ToProxy( L, 1 );
	if (sender)
	{
		TextureResource *entry = (TextureResource *)sender->GetUserdata();
		if (entry)
		{
			entry->ReleaseLuaReferences(L);
			entry->ReleaseSelfFromFactoryOwnership();
		}
	}
	
	return 0;
}

	
} // namespace Rtt
