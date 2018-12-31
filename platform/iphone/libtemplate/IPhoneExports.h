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
