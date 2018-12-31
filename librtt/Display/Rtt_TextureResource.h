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

#ifndef __Rtt_TextureResource__
#define __Rtt_TextureResource__

#include "Renderer/Rtt_Texture.h"
#include "Rtt_MPlatform.h"
#include <string>

// ----------------------------------------------------------------------------

extern "C"
{
	struct lua_State;
}

namespace Rtt
{
class MLuaUserdataAdapter;
class LuaUserdataProxy;
class TextureFactory;
class PlatformBitmap;
// ----------------------------------------------------------------------------

class TextureResource
{
	public:
		typedef TextureResource Self;

	enum TextureResourceType{
		kTextureResourceBitmap,
		kTextureResourceCanvas,
		kTextureResourceExternal,
		
		kTextureResource_Any
	};
	
	protected:
		TextureResource(TextureFactory &factory, Texture *texture,  PlatformBitmap *bitmap, TextureResourceType type);
	public:

		static PlatformBitmap::Format ConvertFormat( Texture::Format );
	
	public:
	virtual ~TextureResource();

	public:
		PlatformBitmap *GetBitmap()  { return fBitmap; }
		const PlatformBitmap *GetBitmap() const  { return fBitmap; }

		Texture& GetTexture() { return *fTexture; }
		const Texture& GetTexture() const { return *fTexture; }

		virtual void Render(Renderer &renderer){};
		virtual void Teardown(){};
		virtual void ReleaseLuaReferences( lua_State *L ) {};
	public:
		// Dimensions of underlying bit data
		U32 GetWidth() const;
		U32 GetHeight() const;
	
		TextureFactory &GetTextureFactory() const {return fFactory;}
	
	//Lua exposure
		void PushProxy( lua_State *L );
		void DetachProxy( );
		
		TextureResourceType GetType() const {return fType;};
	
		const char* GetFilename() const { return fCacheKey.c_str(); }
		std::string GetCacheKey() const { return fCacheKey; }

		MPlatform::Directory GetDirectory() { return MPlatform::kVirtualTexturesDir; }
	
		void SetCacheKey( const std::string & cacheKey ) {fCacheKey = cacheKey;}
		void ReleaseSelfFromFactoryOwnership();
	
	private:
		virtual const MLuaUserdataAdapter& GetAdapter() const = 0;
	
	private:
		std::string fCacheKey;
		TextureFactory &fFactory;
		Texture *fTexture;
		PlatformBitmap *fBitmap;
		TextureResourceType fType;
		mutable LuaUserdataProxy *fProxy;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_TextureResource__
