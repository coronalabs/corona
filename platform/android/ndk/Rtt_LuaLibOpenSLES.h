//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_LuaLibOpenSLES_H__
#define _Rtt_LuaLibOpenSLES_H__

#include "Rtt_Lua.h"
#include "Core/Rtt_ResourceHandle.h"

#if defined(Rtt_USE_OPENSLES)

namespace Rtt
{
	class LuaLibOpenSLES
	{
	public:
		typedef LuaLibOpenSLES Self;
		
	public:
		static void Initialize( lua_State *L );	
	};


} // namespace Rtt

#endif

#endif // _Rtt_LuaLibOpenSLES_H__
