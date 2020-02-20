//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_LuaLibStore_H__
#define _Rtt_LuaLibStore_H__

// ----------------------------------------------------------------------------

#include "Rtt_Lua.h"

// ----------------------------------------------------------------------------
	
namespace Rtt
{
	
class LuaResource;

// ----------------------------------------------------------------------------

class LuaLibStore
{
	public:
		typedef LuaLibStore Self;

	public:
		static const char kTransactionMetaTableName[];

	public:
		static int Open( lua_State *L );
};

// ----------------------------------------------------------------------------
	
} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_LuaLibStore_H__
