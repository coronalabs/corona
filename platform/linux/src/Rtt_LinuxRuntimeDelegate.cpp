//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_LinuxRuntimeDelegate.h"
//#include "Rtt_Lua.h"
#include "Rtt_Runtime.h"
#include "Rtt_LuaContext.h"

// Lua Loader for flattened directories
// This allows .so plugins to load.
extern "C" int loader_Cflat(lua_State * L);

namespace Rtt
{
	/// Creates a new delegate used to receive events from the Corona runtime.
	LinuxRuntimeDelegate::LinuxRuntimeDelegate()
		: RuntimeDelegate()
		, fContentWidth(0)
		, fContentHeight(0)
		, fFPS(30)
		, fOrientation(DeviceOrientation::kUpright)
	{
	}

	/// Destructor. Cleans up allocated resources.
	LinuxRuntimeDelegate::~LinuxRuntimeDelegate()
	{
	}

#pragma endregion

#pragma region Public Member Functions
	void LinuxRuntimeDelegate::DidInitLuaLibraries(const Runtime& sender) const
	{
		lua_State* L = sender.VMContext().L();
		Lua::InsertPackageLoader(L, &loader_Cflat, -1);
	}

	/// Called just before the "main.lua" file has been loaded.
	/// This is the application's opportunity to register custom APIs into Lua.
	void LinuxRuntimeDelegate::WillLoadMain(const Runtime& sender) const
	{
		lua_State* L = sender.VMContext().L();

		if (Rtt_VERIFY(const_cast<Runtime&>(sender).PushLaunchArgs(true) > 0))
		{
			// NativeToJavaBridge::GetInstance()->PushLaunchArgumentsToLuaTable(L);
			lua_pop(L, 1);
		}
	}

	void LinuxRuntimeDelegate::WillLoadConfig(const Runtime& sender, lua_State* L) const
	{
		// Rtt_ASSERT( ! fDisplay );
		Rtt_ASSERT(1 == lua_gettop(L));
		Rtt_ASSERT(lua_istable(L, -1));

		lua_getfield(L, -1, "width");
		if (lua_tonumber(L, -1) > 0 && fContentWidth == 0) // use width from build.settings if it exist there
		{
			const_cast<LinuxRuntimeDelegate*>(this)->SetWidth(lua_tonumber(L, -1));
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "height");
		if (lua_tonumber(L, -1) > 0 && fContentHeight == 0) // use width from build.settings if it exist there
		{
			const_cast<LinuxRuntimeDelegate*>(this)->SetHeight(lua_tonumber(L, -1));
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "fps");
		int fps = (int)lua_tointeger(L, -1);

		if (60 == fps) // Besides default (30), only 60 fps is supported
		{
			fFPS = 60;
		}
		lua_pop(L, 1);

		lua_getfield(L, -1, "scale");
		if (lua_type(L, -1) == LUA_TSTRING)
		{
			fScaleMode = lua_tostring(L, -1);
		}
		lua_pop(L, 1);

		if (fScaleMode.empty())
		{
			fScaleMode = "none";
		}

		// default
		if (fContentWidth == 0)
		{
			const_cast<LinuxRuntimeDelegate*>(this)->SetWidth(320);
		}
		if (fContentHeight == 0)
		{
			const_cast<LinuxRuntimeDelegate*>(this)->SetHeight(480);
		}

		Rtt_ASSERT(1 == lua_gettop(L));
	}

	void LinuxRuntimeDelegate::DidLoadConfig(const Runtime& sender, lua_State* L) const
	{
	}

#pragma endregion
}; // namespace Rtt
