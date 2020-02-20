//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __ratatouille__Rtt_TextureResourceBitmapAdapter__
#define __ratatouille__Rtt_TextureResourceBitmapAdapter__

#include "Rtt_TextureResourceAdapter.h"

namespace Rtt
{
		
class TextureResourceBitmapAdapter : public TextureResourceAdapter
{
	public:
		typedef TextureResourceBitmapAdapter Self;
		typedef TextureResourceAdapter Super;
		
	public:
		static const TextureResourceBitmapAdapter& Constant();
		
	protected:
		TextureResourceBitmapAdapter(){};
		
	public:
		virtual int ValueForKey(
								const LuaUserdataProxy& sender,
								lua_State *L,
								const char *key ) const override;

		virtual StringHash *GetHash( lua_State *L ) const override;
		
	private:
		static int Preload( lua_State *L );

};
	
} // namespace Rtt


#endif /* defined(__ratatouille__Rtt_TextureResourceBitmapAdapter__) */
