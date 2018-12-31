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
