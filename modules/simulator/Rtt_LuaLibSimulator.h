//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_LuaLibSimulator__
#define __Rtt_LuaLibSimulator__

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

class MSimulator;

// ----------------------------------------------------------------------------

class LuaLibSimulator
{
	public:
		typedef LuaLibSimulator Self;

	public:
		static const char kName[];
		static int Open( lua_State *L );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_LuaLibSimulator__
