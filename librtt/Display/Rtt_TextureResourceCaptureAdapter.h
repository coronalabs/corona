#pragma once
//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __ratatouille__Rtt_TextureResourceCaptureAdapter__
#define __ratatouille__Rtt_TextureResourceCaptureAdapter__

#include "Rtt_TextureResourceAdapter.h"

namespace Rtt
{
		
class TextureResourceCaptureAdapter : public TextureResourceAdapter
{
	public:
		typedef TextureResourceCaptureAdapter Self;
		typedef TextureResourceAdapter Super;
		
	public:
		static const TextureResourceCaptureAdapter& Constant();
		
	protected:
		TextureResourceCaptureAdapter(){};
		
	public:
		virtual int ValueForKey(
								const LuaUserdataProxy& sender,
								lua_State *L,
								const char *key ) const override;

		virtual bool SetValueForKey(LuaUserdataProxy& sender,
									lua_State *L,
									const char *key,
									int valueIndex ) const override;

		virtual StringHash *GetHash( lua_State *L ) const override;
		
	private:
		static int newCaptureEvent( lua_State *L );

};

} // namespace Rtt


#endif /* defined(__ratatouille__Rtt_TextureResourceCaptureAdapter__) */
