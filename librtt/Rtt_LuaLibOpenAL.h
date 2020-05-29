//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_LuaLibOpenAL_H__
#define _Rtt_LuaLibOpenAL_H__

// ----------------------------------------------------------------------------

#include "Rtt_Lua.h"
#include "Core/Rtt_ResourceHandle.h"

namespace Rtt
{
	// ----------------------------------------------------------------------------

	class LuaLibOpenAL
	{
	public:
		typedef LuaLibOpenAL Self;
		
	public:
		static void Initialize( lua_State *L );	
	};
	
	// ----------------------------------------------------------------------------
	
} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_LuaLibOpenAL_H__
