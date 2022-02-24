//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include "Core/Rtt_Build.h"
#include "Rtt_EmscriptenVideoObject.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibMedia.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_Event.h"

#if defined(EMSCRIPTEN)
#include "emscripten/emscripten.h"		// for native alert and etc
extern "C"
{
	extern bool jsVideoInit(int id, void* thiz);
	extern double jsVideoCurrentTime(int id);
	extern double jsVideoTotalTime(int id);
	extern bool jsVideoIsMuted(int id);
	extern void	jsVideoMuted(int id, bool val);
	extern void jsVideoLoad(int id, const char* source, bool autoPlay);
	extern void jsVideoPlay(int id);
	extern void jsVideoPause(int id);
	extern void jsVideoSeek(int id, double seekTo);

	// Java ==> Lua callback
	void EMSCRIPTEN_KEEPALIVE jsVideoCallback(Rtt::EmscriptenVideoObject* obj, int eventID)
	{
		Rtt_ASSERT(obj);
		obj->dispatch(eventID);
	}
}
#else
	bool jsVideoInit(int id, void* thiz) { return true; }
	double jsVideoCurrentTime(int id) { return 0; }
	double jsVideoTotalTime(int id) { return 0; }
	bool jsVideoIsMuted(int id) { return true; }
	void	jsVideoMuted(int id, bool val) {}
	void jsVideoLoad(int id, const char* source, bool autoPlay) {}
	void jsVideoPlay(int id) {}
	void jsVideoPause(int id) {}
	void jsVideoSeek(int id, double seekTo) {}
#endif

namespace Rtt
{

#pragma region Constructors/Destructors
	EmscriptenVideoObject::EmscriptenVideoObject(const Rect& bounds)
		: Super(bounds, "video")
		, fAutoPlay(true)
	{
	}

	EmscriptenVideoObject::~EmscriptenVideoObject()
	{
	}

#pragma endregion


#pragma region Public Member Functions
	bool EmscriptenVideoObject::Initialize()
	{
		if (Super::Initialize())
		{
			return jsVideoInit(fElementID, this);
		}
		return false;
	}

	const LuaProxyVTable& EmscriptenVideoObject::ProxyVTable() const
	{
		return PlatformDisplayObject::GetVideoObjectProxyVTable();
	}

	int EmscriptenVideoObject::ValueForKey(lua_State *L, const char key[]) const
	{
		Rtt_ASSERT(key);

		int result = 1;
		if (strcmp("currentTime", key) == 0)
		{
			double val = jsVideoCurrentTime(fElementID);
			lua_pushnumber(L, val);
		}
		else if (strcmp("totalTime", key) == 0)
		{
			double val = jsVideoTotalTime(fElementID);
			lua_pushnumber(L, val);
		}
		else if (strcmp("isMuted", key) == 0)
		{
			bool val = jsVideoIsMuted(fElementID);
			lua_pushboolean(L, val);
		}
		else if (strcmp("fillMode", key) == 0)
		{
		}
		else if (strcmp("load", key) == 0)
		{
			lua_pushcfunction(L, LuaLoad);
		}
		else if (strcmp("play", key) == 0)
		{
			lua_pushcfunction(L, Play);
		}
		else if (strcmp("pause", key) == 0)
		{
			lua_pushcfunction(L, Pause);
		}
		else if (strcmp("seek", key) == 0)
		{
			lua_pushcfunction(L, Seek);
		}
		else if (strcmp("isToggleEnabled", key) == 0)
		{
		}
		else if (strcmp("addEventListener", key) == 0)
		{
			lua_pushcfunction(L, addEventListener);
		}
		else
		{
			result = Super::ValueForKey(L, key);
		}
		return result;
	}

	bool EmscriptenVideoObject::SetValueForKey(lua_State *L, const char key[], int valueIndex)
	{
		Rtt_ASSERT(key);

		bool result = true;
//		printf("set member %s\n", key);

		if (strcmp("isToggleEnabled", key) == 0)
		{
		}
		else if (strcmp("isMuted", key) == 0)
		{
			bool val = lua_toboolean(L, valueIndex);
			jsVideoMuted(fElementID, val);
		}
		else if (strcmp("fillMode", key) == 0)
		{
		}
		else
		{
			result = Super::SetValueForKey(L, key, valueIndex);
		}

		return result;
	}

#pragma endregion


#pragma region Protected Static Functions
	int EmscriptenVideoObject::LuaLoad(lua_State *L)
	{
		const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
		EmscriptenVideoObject *o = (EmscriptenVideoObject *)luaL_todisplayobject(L, 1, table);

		const char * source = NULL;
		if (lua_isstring(L, 2))
		{
			source = lua_tostring(L, 2);
		}
		o->load(source);
		return 0;
	}

	void EmscriptenVideoObject::load(const char* source)
	{
		if (source)
		{
			jsVideoLoad(fElementID, source, fAutoPlay);
		}
	}

	int EmscriptenVideoObject::Play(lua_State *L)
	{
		const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
		EmscriptenVideoObject *o = (EmscriptenVideoObject *)luaL_todisplayobject(L, 1, table);
		if (o)
		{
			jsVideoPlay(o->fElementID);
		}
		return 0;
	}

	int EmscriptenVideoObject::Pause(lua_State *L)
	{
		const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
		EmscriptenVideoObject *o = (EmscriptenVideoObject *)luaL_todisplayobject(L, 1, table);
		if (o)
		{
			jsVideoPause(o->fElementID);
		}
		return 0;
	}

	int EmscriptenVideoObject::Seek(lua_State *L)
	{
		const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
		EmscriptenVideoObject *o = (EmscriptenVideoObject *)luaL_todisplayobject(L, 1, table);

		int seekTo = -1;
		if (lua_isnumber(L, 2))
		{
			seekTo = (int)lua_tonumber(L, 2);
		}

		if (o && seekTo > -1)
		{
			jsVideoSeek(o->fElementID, seekTo);
		}
		return 0;
	}

	int EmscriptenVideoObject::addEventListener(lua_State *L)
	{
		const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
		EmscriptenVideoObject *o = (EmscriptenVideoObject *)luaL_todisplayobject(L, 1, table);
		if (o && lua_isstring(L, 2))
		{
			const char* eventName = lua_tostring(L, 2);

			// Store callback
			if (CoronaLuaIsListener(L, 3, eventName))
			{
				o->fListener = new listener(LuaContext::GetContext(L)->LuaState(), eventName, 3);
			}
		}
		return 0;
	}

	void EmscriptenVideoObject::dispatch(int eventID)
	{
		if (fListener)
		{
			fListener->dispatch(fElementID, eventID);
		}
	}


	void EmscriptenVideoObject::listener::dispatch(int fElementID, int eventID)
	{
		static const char* phase[] = {"", "ready", "ended"};
		Rtt_ASSERT(eventID > 0 && eventID < (sizeof(phase) / sizeof(phase[0])));

		lua_State *L = fLuaState.Dereference();

		CoronaLuaNewEvent(L, fEventName.c_str());

		int luaTableStackIndex = lua_gettop(L);
		int nPushed = 0;

		lua_pushstring(L, phase[eventID]);
		lua_setfield(L, luaTableStackIndex, "phase");
		nPushed++;

		lua_pushboolean(L, false);
		lua_setfield(L, luaTableStackIndex, "isError");
		nPushed++;

		double val = jsVideoCurrentTime(fElementID);
		lua_pushnumber(L, val);
		lua_setfield(L, luaTableStackIndex, "currentTime");
		nPushed++;

		val = jsVideoTotalTime(fElementID);
		lua_pushnumber(L, val);
		lua_setfield(L, luaTableStackIndex, "totalTime");
		nPushed++;

		CoronaLuaDispatchEvent(L, fLuaReference, 0);
	}

	EmscriptenVideoObject::listener::listener(const ResourceHandle<lua_State> & handle, const char* eventName, int listenerIndex)
		: fLuaState(handle)
		, fLuaReference(CoronaLuaNewRef(handle.Dereference(), listenerIndex))		// 3
		, fEventName(eventName)
	{
	}

	EmscriptenVideoObject::listener::~listener()
	{
		if (fLuaState.IsValid())
		{
			CoronaLuaDeleteRef(fLuaState.Dereference(), fLuaReference);
			fLuaReference = NULL;
		}
	}


#pragma endregion

} // namespace Rtt

