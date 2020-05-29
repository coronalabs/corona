//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_LuaLibFacebook__
#define __Rtt_LuaLibFacebook__

#include "Rtt_Lua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class LuaResource;
class PlatformFBConnect;

// ----------------------------------------------------------------------------

class LuaLibFacebook
{
	public:
		typedef LuaLibFacebook Self;

	public:
		static int newSession( lua_State *L );

	public:
		static int Open( lua_State *L );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_LuaLibFacebook__
