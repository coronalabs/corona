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

#if 0
namespace Rtt
{
	LinuxTextBoxObject::LinuxTextBoxObject(const Rect& bounds, bool isSingleLine)
		: Super(bounds, isSingleLine ? "input" : "textarea")
		, fIsSingleLine(isSingleLine)
	{
	}

	LinuxTextBoxObject::~LinuxTextBoxObject()
	{
	}

	bool LinuxTextBoxObject::Initialize()
	{
		if (Super::Initialize())
		{
			fWindow = new myTextCtrl(this, fIsSingleLine);
			fWindow->Hide();
			return true;
		}
		return false;
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

		wxTextCtrl* fControl = getTextCtrl();
		if (fControl == NULL)
		{
			return 0;
		}

		int result = 1;
		if (strcmp("text", key) == 0)
		{
			wxString val = fControl->GetValue();
			lua_pushstring(L, val.c_str());
		}
		else if (strcmp("size", key) == 0)
		{
			// font size
			wxFont font = fControl->GetFont();
			int val = font.GetPointSize();
			lua_pushinteger(L, val);
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
		else if (strcmp("align", key) == 0)
		{
			char buf[16] = { 0 };
			Rtt_LogException("LinuxTextBoxObject: ValueForKey '%s' is not implemented\n", key);
			lua_pushstring(L, buf);
		}
		else if (strcmp("isSecure", key) == 0)
		{
			char buf[64] = { 0 };
			Rtt_LogException("LinuxTextBoxObject: ValueForKey '%s' is not implemented\n", key);
			lua_pushboolean(L, strcmp(buf, "password") == 0);
		}
		else if (strcmp("inputType", key) == 0)
		{
			if (fIsSingleLine)
			{
				char buf[64] = { 0 };
				Rtt_LogException("LinuxTextBoxObject: ValueForKey '%s' is not implemented\n", key);
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
				bool val = 0; //jsTextFieldGetEditable(fElementID);
				Rtt_LogException("LinuxTextBoxObject: ValueForKey '%s' is not implemented\n", key);
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

	bool LinuxTextBoxObject::SetValueForKey(lua_State* L, const char key[], int valueIndex)
	{
		Rtt_ASSERT(key);

		wxTextCtrl* fControl = getTextCtrl();
		if (fControl == NULL)
		{
			return false;
		}

		bool result = true;
		if (strcmp("text", key) == 0)
		{
			const char* s = lua_tostring(L, valueIndex);

			if (Rtt_VERIFY(s))
			{
				fControl->ChangeValue(s);
			}
		}
		else if (strcmp("size", key) == 0)
		{
			if (Rtt_VERIFY(lua_isnumber(L, valueIndex)))
			{
				float size = (float)lua_tonumber(L, valueIndex);

				// sanity check
				if (size > 0)
				{
					wxFont font = fControl->GetFont();
					font.SetPixelSize(wxSize(0, size)); // hack 0.8
					fControl->SetFont(font);
				}
				else
				{
					// Rtt_LogException("LinuxTextBoxObject: set size=0\n", key);
				}
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
					bool rc = wxFont::AddPrivateFont(name);
					wxFont wxf = fControl->GetFont();
					wxf.SetPixelSize(wxSize(0, size)); // hack 0.8
					wxf.SetFaceName(face);
					fControl->SetFont(wxf);
				}
			}
		}
		else if (strcmp("placeholder", key) == 0)
		{
			const char* s = lua_tostring(L, valueIndex);
			//	jsTextFieldSetPlaceholder(fElementID, s ? s : "");
			Rtt_LogException("LinuxTextBoxObject: SetValueForKey '%s' is not implemented\n", key);
		}
		else if (strcmp("isSecure", key) == 0)
		{
			bool secure = lua_toboolean(L, valueIndex) ? true : false;
			//	jsTextFieldSetSecure(fElementID, secure);
			Rtt_LogException("LinuxTextBoxObject: SetValueForKey '%s' is not implemented\n", key);
		}
		else if (strcmp("align", key) == 0)
		{
			const char* align = lua_tostring(L, valueIndex);
			//	jsTextFieldSetAlign(fElementID, align);
			Rtt_LogException("LinuxTextBoxObject: SetValueForKey '%s' is not implemented\n", key);
		}
		else if (strcmp("inputType", key) == 0)
		{
			const char* inputType = lua_tostring(L, valueIndex);
			if (inputType)
			{
				//	jsTextFieldSetInputType(fElementID, inputType);
				Rtt_LogException("LinuxTextBoxObject: SetValueForKey '%s' is not implemented\n", key);
			}
		}
		else if (strcmp("isEditable", key) == 0)
		{
			bool isEditable = lua_toboolean(L, valueIndex) ? true : false;
			//	jsTextFieldSetEditable(fElementID, isEditable);
			Rtt_LogException("LinuxTextBoxObject: SetValueForKey '%s' is not implemented\n", key);
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

	void LinuxTextBoxObject::dispatch(const char* phase)
	{
		if (fHandle && fHandle->IsValid())
		{
			lua_State* L = fHandle->Dereference();
			CoronaLuaNewEvent(L, "userInput"); //fEventName.c_str());
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
				wxString strval = getTextCtrl()->GetValue();

				lua_pushstring(L, strval.c_str());
				lua_setfield(L, luaTableStackIndex, "newCharacters");
				nPushed++;

				int val = 0; //jsVideoTotalTime(fElementID);
				lua_pushnumber(L, val);
				lua_setfield(L, luaTableStackIndex, "numDeleted");
				nPushed++;

				lua_pushstring(L, fOldValue.c_str());
				lua_setfield(L, luaTableStackIndex, "oldText");
				nPushed++;

				int pos = getTextCtrl()->GetInsertionPoint();
				lua_pushnumber(L, pos);
				lua_setfield(L, luaTableStackIndex, "startPosition");
				nPushed++;

				lua_pushstring(L, strval.c_str());
				lua_setfield(L, luaTableStackIndex, "text");
				nPushed++;

				fOldValue = strval;
			}
			CoronaLuaDispatchEvent(L, fLuaReference, 0);
		}
	}

	//
	// myTextCtrl
	//

	LinuxTextBoxObject::myTextCtrl::myTextCtrl(LinuxTextBoxObject* parent, bool singleLine)
		: wxTextCtrl(solarApp, -1, "", wxDefaultPosition, wxDefaultSize, singleLine ? wxTE_MULTILINE : wxTE_MULTILINE)
		, fLinuxTextBoxObject(parent)
	{
		Connect(wxEVT_TEXT, wxCommandEventHandler(myTextCtrl::onTextEvent));
		Connect(wxEVT_SET_FOCUS, wxCommandEventHandler(myTextCtrl::onTextEvent));
		Connect(wxEVT_KILL_FOCUS, wxCommandEventHandler(myTextCtrl::onTextEvent));
	}

	LinuxTextBoxObject::myTextCtrl::~myTextCtrl()
	{
		Disconnect(wxEVT_TEXT);
		Disconnect(wxEVT_SET_FOCUS);
		Disconnect(wxEVT_KILL_FOCUS);
	}

	void LinuxTextBoxObject::myTextCtrl::onTextEvent(wxCommandEvent& e)
	{
		e.Skip();
		wxEventType eType = e.GetEventType();
		if (eType == wxEVT_SET_FOCUS)
		{
			fLinuxTextBoxObject->dispatch("began");
		}
		else if (eType == wxEVT_TEXT)
		{
			fLinuxTextBoxObject->dispatch("editing");
		}
		else if (eType == wxEVT_KILL_FOCUS)
		{
			fLinuxTextBoxObject->dispatch("ended");
		}
	}

}; // namespace Rtt
#endif