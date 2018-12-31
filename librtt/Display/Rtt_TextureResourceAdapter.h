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
