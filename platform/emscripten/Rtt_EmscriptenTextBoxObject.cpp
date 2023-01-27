//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_EmscriptenTextBoxObject.h"
#include "Rtt_EmscriptenFont.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"

#if defined(EMSCRIPTEN)
#include "emscripten/emscripten.h"		// for native alert and etc
#include <SDL2/SDL.h>

extern "C"
{

	// Java ==> Lua callback
	void EMSCRIPTEN_KEEPALIVE jsTextBoxCallback(Rtt::EmscriptenTextBoxObject* obj, int eventID)
	{
		Rtt_ASSERT(obj);
		obj->dispatch(eventID);
	}

	extern bool jsTextFieldInitialize(int id);
	extern void jsTextFieldGetValue(int id, void* buf, int bufsize);
	extern void jsTextFieldGetStringProperty(int id, void* buf, int bufsize, const char* name);
	extern int jsTextFieldGetIntProperty(int id, const char* name);
	extern void jsTextFieldSetValue(int id, const char* val);
	extern int jsTextFieldGetFontSize(int id);
	extern void	jsTextFieldSetFontSize(int id, int size); 
	extern void	jsTextFieldGetAlign(int id, void* buf, int size);
	extern void jsTextFieldSetAlign(int id, const char* align);
	extern void	jsTextFieldGetSecure(int id, void* buf, int size);
	extern void jsTextFieldSetSecure(int id, bool password);
	extern void	jsTextFieldGetInputType(int id, void* buf, int size);
	extern void jsTextFieldSetInputType(int id, const char* inputType);
	extern bool jsTextFieldGetEditable(int id);
	extern void	jsTextFieldSetEditable(int id, bool isEditable);
	extern void jsTextFieldSetFont(int id, const char* name, int size);
	extern void jsTextFieldSetPlaceholder(int id, const char* text);
	extern void jsTextFieldSetColor(int id, const char* rgb);

	void EMSCRIPTEN_KEEPALIVE jsEnableKeyboard(int val)
	{
		SDL_EventState(SDL_TEXTINPUT, val == 0 ? SDL_DISABLE : SDL_ENABLE);
		SDL_EventState(SDL_KEYDOWN, val == 0 ? SDL_DISABLE : SDL_ENABLE);
		SDL_EventState(SDL_KEYUP, val == 0 ? SDL_DISABLE : SDL_ENABLE);
	}


}
#else
	bool jsTextFieldInitialize(int id) { return true; }
	void jsTextFieldGetValue(int id, void* buf, int bufsize) {}
	void jsTextFieldGetStringProperty(int id, void* buf, int bufsize, const char* name) {};
	int jsTextFieldGetIntProperty(int id, const char* name) { return 0; };
	void jsTextFieldSetValue(int id, const char* val) {}
	int jsTextFieldGetFontSize(int id) { return 0; }
	void	jsTextFieldSetFontSize(int id, int size) {}
	void	jsTextFieldGetAlign(int id, void* buf, int size) {}
	void jsTextFieldSetAlign(int id, const char* align) {}
	void	jsTextFieldGetSecure(int id, void* buf, int size) {}
	void jsTextFieldSetSecure(int id, bool password) {}
	void	jsTextFieldGetInputType(int id, void* buf, int size) {}
	void jsTextFieldSetInputType(int id, const char* inputType) {}
	bool jsTextFieldGetEditable(int id) { return true; }
	void	jsTextFieldSetEditable(int id, bool isEditable) {}
	void jsTextFieldSetFont(int id, const char* name, int size) {}
	void jsTextFieldSetPlaceholder(int id, const char* text) {}
	void jsTextFieldSetColor(int id, const char* rgb) {}

#endif

#include <string.h>

namespace Rtt
{

#pragma region Constructors/Destructors
	EmscriptenTextBoxObject::EmscriptenTextBoxObject(const Rect& bounds, bool isSingleLine)
		: Super(bounds, isSingleLine ? "input" : "textarea")
		, fIsSingleLine(isSingleLine)
	{
	}

	EmscriptenTextBoxObject::~EmscriptenTextBoxObject()
	{
	}

#pragma endregion


#pragma region Public Member Functions
	bool EmscriptenTextBoxObject::Initialize()
	{
		if (Super::Initialize())
		{
			return jsTextFieldInitialize(fElementID);
		}
		return false;
	}

	const LuaProxyVTable& EmscriptenTextBoxObject::ProxyVTable() const
	{
		return PlatformDisplayObject::GetTextFieldObjectProxyVTable();
	}

	int EmscriptenTextBoxObject::ValueForKey(lua_State *L, const char key[]) const
	{
		Rtt_ASSERT(key);

		int result = 1;
		if (strcmp("text", key) == 0)
		{
			char buf[4096] = {0};
			jsTextFieldGetValue(fElementID, buf, sizeof(buf));
			lua_pushstring(L, buf);
		}
		else if (strcmp("size", key) == 0)
		{
			// font size
			int val = jsTextFieldGetFontSize(fElementID);
			lua_pushinteger(L, val);
		}
		else if (strcmp("font", key) == 0)
		{
		}
		else if (strcmp("setTextColor", key) == 0)
		{
			lua_pushcfunction(L, SetTextColor);
		}
		else if (strcmp("setReturnKey", key) == 0)
		{
			lua_pushcfunction(L, SetReturnKey);
		}
		else if (strcmp("align", key) == 0)
		{
			char buf[16] = {0};
			jsTextFieldGetAlign(fElementID, buf, sizeof(buf));
			lua_pushstring(L, buf);
		}
		else if (strcmp("isSecure", key) == 0)
		{
			char buf[64] = {0};
			jsTextFieldGetSecure(fElementID, buf, sizeof(buf));
			lua_pushboolean(L, strcmp(buf, "password") == 0);
		}
		else if (strcmp("inputType", key) == 0)
		{
			if (fIsSingleLine)
			{
				char buf[64] = {0};
				jsTextFieldGetInputType(fElementID, buf, sizeof(buf));
				lua_pushstring(L, buf);
			}
			else
			{
				result = 0;
			}
		}
		else if (strcmp("isEditable", key) == 0)
		{
			if (!fIsSingleLine)
			{
				bool val = jsTextFieldGetEditable(fElementID);
				lua_pushboolean(L, val);
			}
		}
		else if (strcmp("margin", key) == 0)
		{
			if (fIsSingleLine)
			{
				lua_pushnumber(L, 0);
			}
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

	bool EmscriptenTextBoxObject::SetValueForKey(lua_State *L, const char key[], int valueIndex)
	{
		Rtt_ASSERT(key);

		bool result = true;

		if (strcmp("text", key) == 0)
		{
			const char *s = lua_tostring(L, valueIndex);
			if (Rtt_VERIFY(s))
			{
				jsTextFieldSetValue(fElementID, s);
			}
		}
		else if (strcmp("size", key) == 0)
		{
			if (Rtt_VERIFY(lua_isnumber(L, valueIndex)))
			{
				float size = (float)lua_tonumber(L, valueIndex);
				jsTextFieldSetFontSize(fElementID, size);
			}
		}
		else if (strcmp("font", key) == 0)
		{
			PlatformFont *font = LuaLibNative::ToFont(L, valueIndex);
			if (font)
			{
				jsTextFieldSetFont(fElementID, font->Name(), font->Size());
			}
		}
		else if (strcmp("placeholder", key) == 0)
		{
			const char *s = lua_tostring( L, valueIndex );
			jsTextFieldSetPlaceholder(fElementID, s ? s : "");
		}
		else if (strcmp("isSecure", key) == 0)
		{
			bool secure = lua_toboolean(L, valueIndex) ? true : false;
			jsTextFieldSetSecure(fElementID, secure);
		}
		else if (strcmp("align", key) == 0)
		{
			const char * align = lua_tostring(L, valueIndex);
			jsTextFieldSetAlign(fElementID, align);
		}
		else if (strcmp("inputType", key) == 0)
		{
			const char * inputType = lua_tostring(L, valueIndex);
			if (inputType)
			{
				jsTextFieldSetInputType(fElementID, inputType);
			}
		}
		else if (strcmp("isEditable", key) == 0)
		{
			bool isEditable = lua_toboolean(L, valueIndex) ? true : false;
			jsTextFieldSetEditable(fElementID, isEditable);
		}
		else
		{
			result = Super::SetValueForKey(L, key, valueIndex);
		}

		return result;
	}

#pragma endregion


#pragma region Protected Static Functions
	int EmscriptenTextBoxObject::SetTextColor(lua_State *L)
	{
		const LuaProxyVTable& table = PlatformDisplayObject::GetTextFieldObjectProxyVTable();
		EmscriptenTextBoxObject *obj = (EmscriptenTextBoxObject*)luaL_todisplayobject(L, 1, table);
		if (obj == NULL)
		{
			return 0;
		}

		int index = 2;
		int r = lua_tonumber(L, index++) * 255;
		int g = lua_tonumber(L, index++) * 255;
		int b = lua_tonumber(L, index++) * 255;
		int a = (lua_isnone(L, index) ? 255 : (lua_tonumber(L, index)) * 255);

		char rgb[8];
		snprintf(rgb, sizeof(rgb), "#%02X%02X%02X", r, g, b);
		jsTextFieldSetColor(obj->fElementID, rgb);

		return 0;
	}

	int EmscriptenTextBoxObject::SetReturnKey(lua_State *L)
	{
		PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject(L, 1);
		if (&o->ProxyVTable() == &PlatformDisplayObject::GetTextFieldObjectProxyVTable())
		{
			const char *keyType = lua_tostring(L, 2);
			//TODO: Set the return key type here.
		}
		return 0;
	}

	int EmscriptenTextBoxObject::addEventListener(lua_State *L)
	{
		const LuaProxyVTable& table = PlatformDisplayObject::GetTextFieldObjectProxyVTable();
		EmscriptenTextBoxObject *o = (EmscriptenTextBoxObject *) luaL_todisplayobject(L, 1, table);
		if (o && lua_isstring(L, 2))
		{
			const char* eventName = "userInput"; //lua_tostring(L, 2);

			// Store callback
			if (CoronaLuaIsListener(L, 3, eventName))
			{
				o->fListener = new listener(LuaContext::GetContext(L)->LuaState(), eventName, 3);
			}
		}
		return 0;
	}

	void EmscriptenTextBoxObject::dispatch(int eventID)
	{
		if (fListener && IsReachable())
		{
			fListener->dispatch(fElementID, eventID, GetProxy());
		}
	}

	void EmscriptenTextBoxObject::listener::dispatch(int fElementID, int eventID, LuaProxy* proxy)
	{
		static const char* phase[] = {"", "began", "editing", "ended", "submitted"};
		Rtt_ASSERT(eventID > 0 && eventID < (sizeof(phase) / sizeof(phase[0])));

		lua_State *L = fLuaState.Dereference();
		CoronaLuaNewEvent(L, fEventName.c_str());
		int luaTableStackIndex = lua_gettop(L);
		int nPushed = 0;

		lua_pushstring(L, phase[eventID]);
		lua_setfield(L, luaTableStackIndex, "phase");
		nPushed++;

		// Add 'self' to the event table
		proxy->PushTable( L );
		lua_setfield( L, -2, "target" );
		nPushed++;

		char buf[4096];
		switch(eventID)
		{
		case 1:		// began
			// User begins editing "defaultField"
			break;

		case 2:		// editing
		{
			double val = 0; //jsVideoCurrentTime(fElementID);
			lua_pushnumber(L, val);
			lua_setfield(L, luaTableStackIndex, "newCharacters");
			nPushed++;

			val = 0; //jsVideoTotalTime(fElementID);
			lua_pushnumber(L, val);
			lua_setfield(L, luaTableStackIndex, "numDeleted");
			nPushed++;

			*buf = 0;
			jsTextFieldGetStringProperty(fElementID, buf, sizeof(buf), "oldText");
			lua_pushstring(L, buf);
			lua_setfield(L, luaTableStackIndex, "oldText");
			nPushed++;

			int pos = jsTextFieldGetIntProperty(fElementID, "selectionStart");
			lua_pushnumber(L, pos);
			lua_setfield(L, luaTableStackIndex, "startPosition");
			nPushed++;

			*buf = 0;
			jsTextFieldGetValue(fElementID, buf, sizeof(buf));
			lua_pushstring(L, buf);
			lua_setfield(L, luaTableStackIndex, "text");
			nPushed++;

			break;
		}
		case 3:		// ended
			// Output resulting text from "defaultField"
			break;

		default:
			break;
		}

		CoronaLuaDispatchEvent(L, fLuaReference, 0);
	}

	EmscriptenTextBoxObject::listener::listener(const ResourceHandle<lua_State> & handle, const char* eventName, int listenerIndex)
		: fLuaState(handle)
		, fLuaReference(CoronaLuaNewRef(handle.Dereference(), listenerIndex))		// 3
		, fEventName(eventName)
	{
	}

	EmscriptenTextBoxObject::listener::~listener()
	{
		if (fLuaState.IsValid())
		{
			CoronaLuaDeleteRef(fLuaState.Dereference(), fLuaReference);
			fLuaReference = NULL;
		}
	}

#pragma endregion

} // namespace Rtt
