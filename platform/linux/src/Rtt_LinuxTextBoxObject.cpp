//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_LinuxTextBoxObject.h"
#include "Rtt_LinuxFont.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_LinuxApp.h"
#include "Display/Rtt_Display.h"
#include "Rtt_Freetype.h"
#include <string.h>

using namespace std;

namespace Rtt
{
	LinuxTextBoxObject::LinuxTextBoxObject(const Rect& bounds, bool isSingleLine)
		: Super(bounds)
		, fIsSingleLine(isSingleLine)
		, fIsEditable(true)
		, fIsSecure(false)
		, fHasFocus(false)
		, fInputType(InputType::undefined)
		, fFontSize(0)
	{
		*fValue = 0;
		*fOldValue = 0;
	}

	LinuxTextBoxObject::~LinuxTextBoxObject()
	{
	}

	const LuaProxyVTable& LinuxTextBoxObject::ProxyVTable() const
	{
		return PlatformDisplayObject::GetTextFieldObjectProxyVTable();
	}

	int LinuxTextBoxObject::addEventListener(lua_State* L)
	{
		const LuaProxyVTable& table = PlatformDisplayObject::GetTextFieldObjectProxyVTable();
		LinuxTextBoxObject* obj = (LinuxTextBoxObject*)luaL_todisplayobject(L, 1, table);

		if (obj && lua_isstring(L, 2))
		{
			const char* eventName = lua_tostring(L, 2);

			// Store callback
			if (CoronaLuaIsListener(L, 3, eventName))
			{
				// deleted old
				if (obj->fLuaReference)
				{
					CoronaLuaDeleteRef(L, obj->fLuaReference);
				}
				obj->fLuaReference = CoronaLuaNewRef(L, 3); // listenerIndex=3
			}
		}
		return 0;
	}

	int LinuxTextBoxObject::ValueForKey(lua_State* L, const char key[]) const
	{
		Rtt_ASSERT(key);

		int result = 1;
		if (strcmp("text", key) == 0)
		{
			string val; // = fControl->GetValue();
			lua_pushstring(L, val.c_str());
		}
		else if (strcmp("size", key) == 0)
		{
			// font size
			lua_pushinteger(L, (int)fFontSize);
		}
		else if (strcmp("font", key) == 0)
		{
			Runtime* runtime = LuaContext::GetRuntime(L);
			auto fontPointer = Rtt_NEW(runtime->GetAllocator(), LinuxFont(*runtime->GetAllocator(), "", 16, false));
			LuaLibNative::PushFont(L, fontPointer);
		}
		else if (strcmp("setTextColor", key) == 0)
		{
			lua_pushcfunction(L, SetTextColor);
		}
		else if (strcmp("setReturnKey", key) == 0)
		{
			lua_pushcfunction(L, SetReturnKey);
		}
		else if (strcmp("setSelection", key) == 0)
		{
			lua_pushcfunction(L, SetSelection);
		}
		else if (strcmp("getSelection", key) == 0)
		{
			lua_pushcfunction(L, GetSelection);
		}
		else if (strcmp("align", key) == 0)
		{
			char buf[16] = { 0 };
			Rtt_LogException("LinuxTextBoxObject: ValueForKey '%s' is not implemented\n", key);
			lua_pushstring(L, buf);
		}
		else if (strcmp("isSecure", key) == 0)
		{
			lua_pushboolean(L, fIsSecure);
		}
		else if (strcmp("inputType", key) == 0)
		{
			switch (fInputType)
			{
			case InputType::number:
				lua_pushstring(L, "number");
				break;
			case InputType::decimal:
				lua_pushstring(L, "decimal");
				break;
			case InputType::phone:
				lua_pushstring(L, "phone");
				break;
			case InputType::url:
				lua_pushstring(L, "url");
				break;
			case InputType::noemoji:
				lua_pushstring(L, "no-emoji");
				break;
			default:
				lua_pushstring(L, "default");
				break;
			}
		}
		else if (strcmp("isEditable", key) == 0)
		{
			lua_pushboolean(L, fIsEditable);
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

	bool LinuxTextBoxObject::SetValueForKey(lua_State* L, const char key[], int valueIndex)
	{
		Rtt_ASSERT(key);

		bool result = true;
		if (strcmp("text", key) == 0)
		{
			strncpy(fValue, lua_tostring(L, valueIndex), sizeof(fValue));
		}
		else if (strcmp("size", key) == 0)
		{
			if (Rtt_VERIFY(lua_isnumber(L, valueIndex)))
			{
				fFontSize = (float)lua_tonumber(L, valueIndex);
			}
		}
		else if (strcmp("font", key) == 0)
		{
			PlatformFont* font = LuaLibNative::ToFont(L, valueIndex);

			if (font)
			{
				int size = font->Size();
				const char* name = font->Name();

				glyph_freetype_provider* gp = getGlyphProvider();
				const char* face = gp ? gp->getFace(name) : NULL;

				if (size > 0 && face)
				{
					// todo
				}
			}
		}
		else if (strcmp("placeholder", key) == 0)
		{
			const char* s = lua_tostring(L, valueIndex);
			Rtt_LogException("LinuxTextBoxObject: SetValueForKey '%s' is not implemented\n", key);
		}
		else if (strcmp("isSecure", key) == 0)
		{
			fIsSecure = lua_toboolean(L, valueIndex) ? true : false;
		}
		else if (strcmp("align", key) == 0)
		{
			const char* align = lua_tostring(L, valueIndex);
			Rtt_LogException("LinuxTextBoxObject: SetValueForKey '%s' is not implemented\n", key);
		}
		else if (strcmp("inputType", key) == 0)
		{
			const char* inputType = lua_tostring(L, valueIndex);
			if (strcasecmp(inputType, "number") == 0)
				fInputType = InputType::number;
			else if (strcasecmp(inputType, "decimal") == 0)
				fInputType = InputType::decimal;
			else if (strcasecmp(inputType, "phone") == 0)
				fInputType = InputType::phone;
			else if (strcasecmp(inputType, "url") == 0)
				fInputType = InputType::url;
			else if (strcasecmp(inputType, "no-emoji") == 0)
				fInputType = InputType::noemoji;
			else
				fInputType = InputType::undefined;
		}
		else if (strcmp("isEditable", key) == 0)
		{
			bool isEditable = lua_toboolean(L, valueIndex) ? true : false;
		}
		else
		{
			result = Super::SetValueForKey(L, key, valueIndex);
		}
		return result;
	}

	int LinuxTextBoxObject::SetTextColor(lua_State* L)
	{
		const LuaProxyVTable& table = PlatformDisplayObject::GetTextFieldObjectProxyVTable();
		LinuxTextBoxObject* obj = (LinuxTextBoxObject*)luaL_todisplayobject(L, 1, table);

		if (obj == NULL)
		{
			return 0;
		}

		int index = 2;
		int r = lua_tointeger(L, index++);
		int g = lua_tointeger(L, index++);
		int b = lua_tointeger(L, index++);
		int a = (lua_isnone(L, index) ? 255 : lua_tointeger(L, index));

		char rgb[8];
		snprintf(rgb, sizeof(rgb), "#%02X%02X%02X", r, g, b);
		//jsTextFieldSetColor(obj->fElementID, rgb);
		Rtt_LogException("LinuxTextBoxObject:SetTextColor() is not implemented\n");

		return 0;
	}

	int LinuxTextBoxObject::SetReturnKey(lua_State* L)
	{
		PlatformDisplayObject* o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject(L, 1);

		if (&o->ProxyVTable() == &PlatformDisplayObject::GetTextFieldObjectProxyVTable())
		{
			const char* keyType = lua_tostring(L, 2);
			//TODO: Set the return key type here.
		}
		return 0;
	}

	int LinuxTextBoxObject::SetSelection(lua_State* L)
	{
		Rtt_LogException("LinuxTextBoxObject:SetSelection() is not implemented\n");
		return 0;
	}

	int LinuxTextBoxObject::GetSelection(lua_State* L)
	{
	    Rtt_LogException("LinuxTextBoxObject:GetSelection() is not implemented\n");
	    return 0;
	}

	void LinuxTextBoxObject::dispatch(const char* phase, int pos, ImWchar ch)
	{
		if (fHandle && fHandle->IsValid())
		{
			lua_State* L = fHandle->Dereference();
			CoronaLuaNewEvent(L, "userInput");
			int luaTableStackIndex = lua_gettop(L);
			int nPushed = 0;

			lua_pushstring(L, phase);
			lua_setfield(L, luaTableStackIndex, "phase");
			nPushed++;

			// Add 'self' to the event table
			GetProxy()->PushTable(L);
			lua_setfield(L, -2, "target");
			nPushed++;

			if (strcmp(phase, "editing") == 0)
			{
				char s[3];
				memcpy(s, &ch, 2);
				s[2] = 0;
				lua_pushstring(L, s);
				lua_setfield(L, luaTableStackIndex, "newCharacters");
				nPushed++;

				int numDeleted = 0;
				lua_pushnumber(L, numDeleted);
				lua_setfield(L, luaTableStackIndex, "numDeleted");
				nPushed++;

				lua_pushstring(L, fOldValue);
				lua_setfield(L, luaTableStackIndex, "oldText");
				nPushed++;

				lua_pushnumber(L, pos);
				lua_setfield(L, luaTableStackIndex, "startPosition");
				nPushed++;

				lua_pushstring(L, fValue);
				lua_setfield(L, luaTableStackIndex, "text");
				nPushed++;

				strcpy(fOldValue, fValue);
			}
			CoronaLuaDispatchEvent(L, fLuaReference, 0);
		}
	}

	static int ImGuiInputTextCallback(ImGuiInputTextCallbackData* data)
	{
		LinuxTextBoxObject* thiz = (LinuxTextBoxObject*)data->UserData;
		thiz->dispatch("editing", data->CursorPos, data->EventChar);

		// accept
		return 0;
	}

	void LinuxTextBoxObject::Draw()
	{
		// center this window when appearing
		float w = fBounds.Width();
		float h = fBounds.Height() + 2;
		ImVec2 center(fBounds.xMin + w / 2, fBounds.yMin + h + app->GetMenuHeight() / 2);
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(w, h));

		char windowLabel[32];
		snprintf(windowLabel, sizeof(windowLabel), "##Text%p", this);
		char fldLabel[32];
		snprintf(fldLabel, sizeof(fldLabel), "##TextFld%p", this);

		if (ImGui::Begin(windowLabel, NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar))
		{
			// set fontsize
			float fontSize = ImGui::GetTextLineHeight();
			if (fFontSize != fontSize)
			{
				if (fFontSize > 0)
					ImGui::SetWindowFontScale(fFontSize / fontSize);
				fFontSize = ImGui::GetTextLineHeight();		// reset
			}

			const ImVec2& window_size = ImGui::GetWindowSize();

			if (fHasFocus && !ImGui::IsWindowFocused())
			{
				dispatch("ended", 0, 0);
			}
			if (!fHasFocus && ImGui::IsWindowFocused())
			{
				dispatch("began", 0, 0);
			}
			fHasFocus = ImGui::IsWindowFocused();

			ImGui::SetCursorPosX(0);
			ImGui::SetCursorPosY(0);
			ImGui::PushItemWidth(fBounds.Width());		// input field width
			{
				ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackCharFilter;
				if (!fIsEditable)
					flags |= ImGuiInputTextFlags_ReadOnly;

				// input type
				if (fInputType == InputType::number || fInputType == InputType::decimal)
					flags |= ImGuiInputTextFlags_CharsDecimal;

				if (fIsSecure)
					flags |= ImGuiInputTextFlags_Password;

				if (fIsSingleLine)
					ImGui::InputText(fldLabel, fValue, sizeof(fValue), flags, ImGuiInputTextCallback, this);
				else
					ImGui::InputTextMultiline(fldLabel, fValue, sizeof(fValue), ImVec2(w, h), flags, ImGuiInputTextCallback, this);

			}
			ImGui::PopItemWidth();
			ImGui::End();
		}
	}

}; // namespace Rtt
