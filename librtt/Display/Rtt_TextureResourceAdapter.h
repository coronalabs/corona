//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __libcorona__Rtt_TextureResourceAdapter__
#define __libcorona__Rtt_TextureResourceAdapter__


#include "Rtt_LuaUserdataProxy.h"

namespace Rtt
{
	class TextureResourceAdapter: public MLuaUserdataAdapter
	{
	public:
		typedef TextureResourceAdapter Self;
		static const TextureResourceAdapter& Constant();
	
	protected:
		TextureResourceAdapter();
		
	public:
		virtual int ValueForKey(const LuaUserdataProxy& sender,
								lua_State *L,
								const char *key ) const override;
		
		virtual bool SetValueForKey(LuaUserdataProxy& sender,
									lua_State *L,
									const char *key,
									int valueIndex ) const override;
		
		// This is invoked when sender's object is about to be deleted
		virtual void WillFinalize( LuaUserdataProxy& sender ) const override;
		
		// Return the hash table containing the adapter's property keys
		virtual StringHash *GetHash( lua_State *L ) const override;
		
	private:
		static int ReleaseStrongReference( lua_State *L );
		
	};
}


#endif /* defined(__libcorona__Rtt_TextureResourceAdapter__) */
