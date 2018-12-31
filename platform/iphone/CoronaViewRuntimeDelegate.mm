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

#include "Core/Rtt_Build.h"

#import "CoronaViewRuntimeDelegate.h"

#import "CoronaViewPrivate.h"
#import <UIKit/UIApplication.h>

#include "CoronaIOSLoader.h"
#include "Display/Rtt_Display.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"

#ifdef Rtt_IPHONE_ENV
	#include "Rtt_IPhoneTemplate.h"
	#include "Rtt_DependencyLoader.h"
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

CoronaViewRuntimeDelegate::CoronaViewRuntimeDelegate( CoronaView *owner )
:	fOwner( owner )
{
}

CoronaViewRuntimeDelegate::~CoronaViewRuntimeDelegate()
{
}

void
CoronaViewRuntimeDelegate::DidInitLuaLibraries( const Runtime& sender ) const
{
	lua_State *L = sender.VMContext().L();
	Corona::IOSLoader::Register( L, GetPluginContext() );
	
	//Add verification library
	//Lua::InsertPackageLoader( L, & (Corona::DependencyLoader::CCDataLoader), -1 );
}

bool
CoronaViewRuntimeDelegate::HasDependencies( const Runtime& sender ) const
{
#ifdef Rtt_IPHONE_ENV
	bool isCoronaCards = IPhoneTemplate::IsProperty( IPhoneTemplate::kIsCoronaKit );
	bool result = ( ! isCoronaCards ) || ( Corona::DependencyLoader::CCDependencyCheck( sender ) );
	return result;
#elif Rtt_TVOS_ENV
	return true;
#else
	Rtt_ASSERT_NOT_IMPLEMENTED();
	return false;
#endif
}

void
CoronaViewRuntimeDelegate::InitializeConfig( const Runtime& sender, lua_State *L ) const
{
	static const char kApplication[] = "application";
	static const char kMetadata[] = "metadata";
	static const char kAppId[] = "appId";

	Rtt_LUA_STACK_GUARD( L );

	lua_getglobal( L, kApplication ); // application
	{
		// [Lua] if not application then
		// [Lua]     application = {}
		// [Lua] end
		if ( ! lua_istable( L, -1 ) )
		{
			lua_pop( L, 1 ); // pop non-nil value
			lua_createtable( L, 0, 1 );

			lua_setglobal( L, kApplication ); // application
		}
		
		int index = lua_gettop( L );

		lua_getfield( L, index, kMetadata );
		{
			// [Lua] if not application.metadata then
			// [Lua]     application.metadata = {}
			// [Lua] end
			if ( ! lua_istable( L, -1 ) )
			{
				lua_pop( L, 1 ); // pop non-nil value
				lua_createtable( L, 0, 1 );

				lua_pushvalue( L, -1 ); // leave metadata on stack, so push extra
				lua_setfield( L, index, kMetadata );
			}

			// [Lua] if type(application.metadata ~= "string" then
			// [Lua]     application.metadata = bundleId
			// [Lua] end
			lua_getfield( L, -1, kAppId );
			bool isValidAppId = ( lua_type( L, -1 ) == LUA_TSTRING );
			lua_pop( L, 1 );
			if ( ! isValidAppId )
			{
				NSString *bundleId = [[NSBundle mainBundle] bundleIdentifier];
				lua_pushstring( L, [bundleId UTF8String] );
				lua_setfield( L, -2, kAppId );
			}
		}
		lua_pop( L, 1 );
	}
	lua_pop( L, 1 );
}

void
CoronaViewRuntimeDelegate::DidLoadConfig( const Runtime& sender, lua_State *L ) const
{
	bool antialias = sender.GetDisplay().IsAntialiased();
	if (antialias)
	{
		// TODO: Re-enable once we get this working properly
		// GLKView *view = fOwner;
		// view.drawableMultisample = GLKViewDrawableMultisample4X;
	}
}

id< CoronaRuntime >
CoronaViewRuntimeDelegate::GetPluginContext() const
{
	return fOwner.pluginContext;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
