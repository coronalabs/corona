//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _IPhoneExports_H__
#define _IPhoneExports_H__

#include "Rtt_Lua.h"
#include "CoronaMacros.h"

CORONA_EXPORT int luaopen_network( lua_State *L );

static luaL_Reg *IPhoneExports()
{
	static luaL_Reg sExports[] =
	{
		{ "", luaopen_network },

		{ NULL, NULL }
	};

	return sExports;
}

static bool
IPhoneVerifyExports()
{
	luaL_Reg *exports = IPhoneExports();

	bool result = true;

	for ( const luaL_Reg *lib = exports; lib->func; lib++ )
	{
		if ( reinterpret_cast< const void * >( lib->name ) == reinterpret_cast< void * >( lib->func ) )
		{
			// The above condition is only true for the last entry (NULL == NULL ) in exports
			// However, the lib->func break condition in the for-loop ensures we never hit this.
			Rtt_ASSERT_NOT_REACHED();
			result = false;
			break;
		}
	}

	return result;
}

// ----------------------------------------------------------------------------

#endif // _IPhoneExports_H__
