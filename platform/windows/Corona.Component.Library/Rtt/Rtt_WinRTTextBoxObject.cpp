//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Rtt_WinRTTextBoxObject.h"
#include "Rtt_WinRTFont.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Rtt_Lua.h"
#	include "Rtt_LuaContext.h"
#	include "Rtt_LuaLibNative.h"
#	include "Rtt_LuaProxy.h"
#	include "Rtt_LuaProxyVTable.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

#pragma region Constructors/Destructors
WinRTTextBoxObject::WinRTTextBoxObject(const Rect& bounds, bool isSingleLine)
:	Super(bounds),
	fIsSingleLine(isSingleLine)
{
}

WinRTTextBoxObject::~WinRTTextBoxObject()
{
}

#pragma endregion


#pragma region Public Member Functions
bool WinRTTextBoxObject::Initialize()
{
//TODO: Figure out why GetScreenBounds() is triggering an assert.
//	Rect screenBounds;
//	GetScreenBounds(screenBounds);
	Super::InitializeView(this);
	return false;
}

const LuaProxyVTable& WinRTTextBoxObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetTextFieldObjectProxyVTable();
}

int WinRTTextBoxObject::ValueForKey(lua_State *L, const char key[]) const
{
	Rtt_ASSERT(key);

	int result = 1;

	if (strcmp("text", key) == 0)
	{
	}
	else if (strcmp("size", key) == 0)
	{
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
	}
	else if (strcmp("isSecure", key) == 0)
	{
		if (fIsSingleLine)
		{
			//TODO: Fetch this setting from the text box here.
		}
		else
		{
			result = 0;
		}
	}
	else if (strcmp("inputType", key) == 0)
	{
		if (fIsSingleLine)
		{
			//TODO: Fetch this setting from the text box here.
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
			//TODO: Fetch this setting from the text box here.
		}
	}
	else if (strcmp("margin", key) == 0)
	{
		if (fIsSingleLine)
		{
			lua_pushnumber(L, 0);
		}
	}
	else
	{
		result = Super::ValueForKey(L, key);
	}

	return result;
}

bool WinRTTextBoxObject::SetValueForKey(lua_State *L, const char key[], int valueIndex)
{
	Rtt_ASSERT(key);

	bool result = true;

	if (strcmp("text", key) == 0)
	{
		const char *s = lua_tostring(L, valueIndex);
		if (Rtt_VERIFY(s))
		{
			//TODO: Set this property here.
		}
	}
	else if (strcmp("size", key) == 0)
	{
		if (Rtt_VERIFY(lua_isnumber(L, valueIndex)))
		{
			float size = (float)lua_tonumber(L, valueIndex);
			//TODO: Set this property here.
		}
	}
	else if (strcmp("font", key) == 0)
	{
		PlatformFont *font = LuaLibNative::ToFont(L, valueIndex);
		if (font)
		{
			//TODO: Set this property here.
		}
	}
	else if (strcmp("isSecure", key) == 0)
	{
		if (fIsSingleLine)
		{
			bool secure = lua_toboolean(L, valueIndex) ? true : false;
			//TODO: Set this property here.
		}
	}
	else if (strcmp("align", key) == 0)
	{
		const char * align = lua_tostring(L, valueIndex);
		//TODO: Set this property here.
	}
	else if (strcmp("inputType", key) == 0)
	{
		if (fIsSingleLine)
		{
			const char * inputType = lua_tostring(L, valueIndex);
			if (inputType)
			{
				//TODO: Set this property here.
			}
		}
	}
	else if (strcmp("isEditable", key) == 0)
	{
		// This propery is only supported on multiline text boxes to match iOS behavior.
		if (!fIsSingleLine)
		{
			bool isEditable = lua_toboolean(L, valueIndex) ? true : false;
			//TODO: Set this property here.
		}
	}
	else
	{
		result = Super::SetValueForKey(L, key, valueIndex);
	}

	return result;
}

#pragma endregion


#pragma region Protected Static Functions
int WinRTTextBoxObject::SetTextColor(lua_State *L)
{
	PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject(L, 1);
	if (&o->ProxyVTable() == &PlatformDisplayObject::GetTextFieldObjectProxyVTable())
	{
		int index = 2;
		int r = lua_tointeger(L, index++);
		int g = lua_tointeger(L, index++);
		int b = lua_tointeger(L, index++);
		int a = (lua_isnone(L, index) ? 255 : lua_tointeger(L, index));
		//TODO: Set the text color here.
	}
	return 0;
}

int WinRTTextBoxObject::SetReturnKey(lua_State *L)
{
	PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject(L, 1);
	if (&o->ProxyVTable() == &PlatformDisplayObject::GetTextFieldObjectProxyVTable())
	{
		const char *keyType = lua_tostring(L, 2);
		//TODO: Set the return key type here.
	}
	return 0;
}

#pragma endregion

} // namespace Rtt
