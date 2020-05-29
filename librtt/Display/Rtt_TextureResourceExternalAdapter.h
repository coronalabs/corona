//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __ratatouille__Rtt_TextureResourceExternalAdapter__
#define __ratatouille__Rtt_TextureResourceExternalAdapter__

#include "Rtt_TextureResourceAdapter.h"

namespace Rtt
{
		
class TextureResourceExternalAdapter : public TextureResourceAdapter
{
	public:
		typedef TextureResourceExternalAdapter Self;
		typedef TextureResourceAdapter Super;
		
	public:
		static const TextureResourceExternalAdapter& Constant();
		
	protected:
		TextureResourceExternalAdapter(){};
		
	public:
		virtual int ValueForKey(
								const LuaUserdataProxy& sender,
								lua_State *L,
								const char *key ) const override;

		virtual StringHash *GetHash( lua_State *L ) const override;
	private:
		static int Invalidate( lua_State *L );

	public:
		static void* GetUserData( lua_State*L, int index );
};
	
} // namespace Rtt


#endif /* defined(__ratatouille__Rtt_TextureResourceExternalAdapter__) */
