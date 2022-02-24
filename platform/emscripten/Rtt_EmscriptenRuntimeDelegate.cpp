//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_EmscriptenRuntimeDelegate.h"
#include "Rtt_EmscriptenCPluginLoader.h"
#include "Rtt_EmscriptenJSPluginLoader.h"
#include "Rtt_Lua.h"
#include "Rtt_Runtime.h"
#include "Rtt_LuaContext.h"

namespace Rtt
{

	/// Creates a new delegate used to receive events from the Corona runtime.
	EmscriptenRuntimeDelegate::EmscriptenRuntimeDelegate()
		: RuntimeDelegate()
		, fContentWidth(0)
		, fContentHeight(0)
	{
	}

	/// Destructor. Cleans up allocated resources.
	EmscriptenRuntimeDelegate::~EmscriptenRuntimeDelegate()
	{
	}

#pragma endregion


#pragma region Public Member Functions

	/// Called just before the "main.lua" file has been loaded.
	/// This is the application's opportunity to register custom APIs into Lua.
	void EmscriptenRuntimeDelegate::WillLoadMain(const Runtime& sender) const
	{
		lua_State *L = sender.VMContext().L();

		if (Rtt_VERIFY(const_cast<Runtime&>(sender).PushLaunchArgs(true) > 0))
		{
			//		NativeToJavaBridge::GetInstance()->PushLaunchArgumentsToLuaTable(L);
			lua_pop(L, 1);
		}

		// Add a custom package/library loader to the runtime's Lua state.
		// This allows us to load plugins that are compiled into this Corona library, such as the "network" plugin.
		Rtt::Lua::InsertPackageLoader(L, &EmscriptenJSPluginLoader::Loader, -1);
		Rtt::Lua::InsertPackageLoader(L, &EmscriptenCPluginLoader::Loader, -1);
	}

	void EmscriptenRuntimeDelegate::DidLoadConfig( const Runtime& sender, lua_State *L ) const
	{
		lua_getglobal(L, "application"); // application
		if (lua_istable(L, -1))
		{
			lua_getfield(L, -1, "content"); // application.content
			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "width");
				if ((!lua_isnil(L, -1)) && (lua_isnumber(L, -1)))
				{
					fContentWidth = lua_tointeger(L, -1);
				}
				lua_pop(L, 1);

				lua_getfield(L, -1, "height");
				if ((!lua_isnil(L, -1)) && (lua_isnumber(L, -1)))
				{
					fContentHeight = lua_tointeger(L, -1);
				}
				lua_pop(L, 1);

				lua_getfield(L, -1, "scale");
				if ((!lua_isnil(L, -1)) && (lua_isstring(L, -1)))
				{
					fScaleMode = lua_tostring(L, -1);
				}
				lua_pop(L, 1);
			}
			lua_pop(L, 1);		// remove content
		}
		lua_pop(L, 1);		// remove application
	}


#pragma endregion


} // namespace Rtt
