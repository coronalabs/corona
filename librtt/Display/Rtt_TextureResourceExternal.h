//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_TextureResourceExternal__
#define __Rtt_TextureResourceExternal__

#include "Renderer/Rtt_Texture.h"
#include "Display/Rtt_TextureResource.h"

extern "C"
{
	struct CoronaExternalTextureCallbacks;
}

// ----------------------------------------------------------------------------

namespace Rtt
{
	
	class PlatformBitmap;
	class TextureFactory;
	
	// ----------------------------------------------------------------------------
	
	class TextureResourceExternal : public TextureResource
	{
	public:
		typedef TextureResourceExternal Self;
		
	public:
		
		static Self *Create(
							TextureFactory& factory,
							const CoronaExternalTextureCallbacks *callbacks,
							void *callbacksContext,
							bool isRetina );
		
	protected:
		TextureResourceExternal(
							  TextureFactory &factory,
							  Texture *texture,
							  PlatformBitmap *bitmap );
		
	public:
		virtual ~TextureResourceExternal();
		
		void Teardown() override;
		
		//Lua interface
		virtual const MLuaUserdataAdapter& GetAdapter() const override;
		
		int GetField(lua_State *L, const char *field) const;
		void* GetUserData() const;
	};
	
	// ----------------------------------------------------------------------------
	
} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_TextureResourceExternal__
