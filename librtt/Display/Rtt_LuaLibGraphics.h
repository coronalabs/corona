//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_LuaLibGraphics__
#define __Rtt_LuaLibGraphics__

#include "Rtt_Lua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Display;

// ----------------------------------------------------------------------------

class LuaLibGraphics
{
	public:
		typedef LuaLibGraphics Self;

	public:
		static void Initialize( lua_State *L, Display& display );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_LuaLibGraphics__
