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

#ifndef _Rtt_LuaLibNative_H__
#define _Rtt_LuaLibNative_H__

// ----------------------------------------------------------------------------

#include "Rtt_Lua.h"

namespace Rtt
{

class LuaResource;
class MPlatform;
class PlatformFont;

// ----------------------------------------------------------------------------

class LuaLibNative
{
	public:
		typedef LuaLibNative Self;

	public:
		static void Initialize( lua_State *L );

	public:
		static PlatformFont* CreateFont( lua_State *L, const MPlatform& platform, int index, Real fontSize );
		static int PushFont( lua_State *L, PlatformFont *font );
		static PlatformFont* ToFont( lua_State *L, int index );
		static bool IsFont( lua_State *L, int index );

	public:
		// buttonIndex is 0-based
		static void AlertComplete( LuaResource& resource, S32 buttonIndex, bool cancelled );
		static void PopupClosed( LuaResource& resource, const char *popupName, bool wasCanceled );

		enum
		{
			RecordStatusNone = 0,
			RecordStatusError = 1
		};
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_LuaLibNative_H__
