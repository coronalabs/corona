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

#ifndef _Rtt_LuaAux_H__
#define _Rtt_LuaAux_H__

// ----------------------------------------------------------------------------

#include "Rtt_Lua.h"

#include "Core/Rtt_Geometry.h"
#include "Core/Rtt_ResourceHandle.h"

namespace Rtt
{

class DisplayObject;
class LuaProxyVTable;
class MPlatform;
class String;

// ----------------------------------------------------------------------------

Rtt_INLINE static Rtt_Real
luaL_checkreal( lua_State *L, int narg )
{
	return Rtt_FloatToReal( (float) luaL_checknumber( L, narg ) );
}

Rtt_INLINE static Rtt_Real
luaL_toreal( lua_State *L, int narg )
{
	return Rtt_FloatToReal( (float) lua_tonumber( L, narg ) );
}

DisplayObject* luaL_todisplayobject( lua_State *L, int index, const LuaProxyVTable& table );

void setProperty( lua_State *L, const char key[], size_t keyLen, Coordinate value );


#ifdef Rtt_DEBUG

	bool luaL_verify( lua_State *L, bool cond, const char *reason );
	#define Rtt_LUA_ASSERT( L, cond, msg )	(void)luaL_verify( (L), (cond), (msg) )

#else

	#define Rtt_LUA_ASSERT( L, cond, msg )

#endif

// These functions map an enum constant to an address. They assume the enum
// starts at 0 and consecutively increase.  Further, the number of char's in 
// the buffer must be the number of constants in the enum.
void* UserdataForEnum( const char buffer[], U32 index );
bool EnumExistsForUserdata( const char buffer[], void *p, S32 numIndices );
S32 EnumForUserdata( const char buffer[], void* p, S32 numIndices, S32 defaultIndex );

// Constructs a path grabbing the filename from the stack at "index" and using a 
// base directory at "index + 1". If no base directory is specified, then MPlatform::kResourceDir
// is used by default. It returns the index on the stack of the next argument
// after all path related arguments. Also, the results of the stack are placed in outPath.
int luaL_initpath( lua_State *L, const MPlatform& platform, int index, String& outPath  );

// ----------------------------------------------------------------------------

// UserdataWrapper is a convenience class for managing situations where you need Lua to own
// the wrapper but not own the C pointer stored inside the wrapper b/c the pointer has a 
// distinct lifetime outside the Lua GC system.  
// 
// It also ensure that for a given C pointer, the *same* Lua userdata instance is 
// pushed to Lua.
// 
// Ownership assumptions and usage consequences:
// * Some other entity will own the actual userdata pointer (e.g. b2World owns b2Joint's).
// * UserdataWrapper will have a *weak* pointer to the C pointer (userdata). Therefore:
//		You should mark the pointer (the one you are wrapping) as invalid when
//		the pointer is destroyed by calling Invalidate() on the wrapper, since
//		the UserdataWrapper's lifetime is different from the pointer's.
// * The userdata pointer can have a *weak* reference back to UserdataWrapper. Therefore:
// 		When the UserdataWrapper instance is GC'd by Lua, you should NULL-ify any
//		reference to the UserdataWrapper instance held by the userdata pointer.
class UserdataWrapper
{
	public:
		typedef UserdataWrapper Self;

	public:
		static void* GetFinalizedValue();

	public:
		UserdataWrapper( const ResourceHandle< lua_State >& handle, void *ud, const char *mtName );
		~UserdataWrapper();

	public:
		int Push() const;
		void Invalidate() { fData = NULL; }
		void* Dereference() const { return fData; }

	private:
		ResourceHandle< lua_State > fHandle;
		void *fData;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_LuaAux_H__
