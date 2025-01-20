//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_LuaGCInhibitor_H__
#define _Rtt_LuaGCInhibitor_H__

#include "Core/Rtt_Macros.h"

Rtt_EXPORT_BEGIN

#include "lua.h"

Rtt_EXPORT_END

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

// ----------------------------------------------------------------------------

class LuaGCInhibitor
{
	Rtt_CLASS_NO_DYNAMIC_ALLOCATION
	
	public:
		Rtt_INLINE LuaGCInhibitor( lua_State *L )
		:	fL( L )
		{
			Rtt_ASSERT( L );
			lua_gc( L, LUA_GCSTOP, 0 );
		}
		
		Rtt_INLINE ~LuaGCInhibitor()
		{
			lua_gc( fL, LUA_GCRESTART, 0 );
		}
		
	private:
		lua_State* fL;
};

	
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Matrix_H__
