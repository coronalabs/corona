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

#include "CoronaAdsLibrary.h"

#include "CoronaAssert.h"
#include "CoronaLibrary.h"

// ----------------------------------------------------------------------------

CORONA_EXPORT int CoronaPluginLuaLoad_ads( lua_State * );
CORONA_EXPORT int CoronaPluginLuaLoad_CoronaProvider_ads( lua_State * );

// ----------------------------------------------------------------------------

static const char kProviderName[] = "CoronaProvider.ads";

CORONA_EXPORT
int luaopen_ads( lua_State *L )
{
	using namespace Corona;

	Corona::Lua::RegisterModuleLoader(
		L, kProviderName, Corona::Lua::Open< CoronaPluginLuaLoad_CoronaProvider_ads > );

	lua_CFunction factory = Corona::Lua::Open< CoronaPluginLuaLoad_ads >;
	int result = CoronaLibraryNewWithFactory( L, factory, NULL, NULL );

	return result;
}

// ----------------------------------------------------------------------------
