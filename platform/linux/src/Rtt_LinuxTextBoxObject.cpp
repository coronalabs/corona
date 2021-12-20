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
#include "Rtt_LinuxContext.h"
#include "Display/Rtt_Display.h"
#include "Rtt_Freetype.h"
#include <string.h>

namespace Rtt
{
	LinuxTextBoxObject::LinuxTextBoxObject(const Rect &bounds, bool isSingleLine)
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
			Rect outBounds;
			GetScreenBounds(outBounds);

			Rtt_ASSERT(fWindow == NULL);
			fWindow = new myTextCtrl(this, outBounds.xMin, outBounds.yMin, outBounds.Width(), outBounds.Height(), fIsSingleLine);
			return true;
		}

		return false;
	}

	void LinuxTextBoxObject::Prepare(const Display &display)
	{
		if (fWindow)
		{
			Rect outBounds{};
			GetScreenBounds(outBounds);

			fWindow->SetPosition((wxPoint(outBounds.xMin, outBounds.yMin)));
			fWindow->SetSize(outBounds.Width(), outBounds.Height());
		}
	}

	const LuaProxyVTable &LinuxTextBoxObject::ProxyVTable() const
	{
		return PlatformDisplayObject::GetTextFieldObjectProxyVTable();
	}

	int LinuxTextBoxObject::addEventListener(lua_State *L)
	{
		const LuaProxyVTable &table = PlatformDisplayObject::GetTextFieldObjectProxyVTable();
		LinuxTextBoxObject *o = (LinuxTextBoxObject *)luaL_todisplayobject(L, 1, table);

		if (o && lua_isstring(L, 2))
		{
			const char *eventName = lua_tostring(L, 2);
			myTextCtrl *t = (myTextCtrl*)o->fWindow;

			// Store callback
			if (CoronaLuaIsListener(L, 3, eventName) && strcmp(eventName, "userInput") == 0)	// hack
			{
				// deleted old
				if (t->fLuaReference)
				{
					CoronaLuaDeleteRef(L, t->fLuaReference);
				}

				t->fLuaReference = CoronaLuaNewRef(L, 3);		// listenerIndex=3
			}
		}

		return 0;
	}

	int LinuxTextBoxObject::ValueForKey(lua_State *L, const char key[]) const
	{
		Rtt_ASSERT(key);

		wxTextCtrl *fControl = dynamic_cast<wxTextCtrl*>(fWindow);

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
			Runtime *runtime = LuaContext::GetRuntime(L);
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
			char buf[16] = {0};
			Rtt_LogException("LinuxTextBoxObject: ValueForKey '%s' is not implemented\n", key);
			lua_pushstring(L, buf);
		}
		else if (strcmp("isSecure", key) == 0)
		{
			char buf[64] = {0};
			Rtt_LogException("LinuxTextBoxObject: ValueForKey '%s' is not implemented\n", key);
			lua_pushboolean(L, strcmp(buf, "password") == 0);
		}
		else if (strcmp("inputType", key) == 0)
		{
			if (fIsSingleLine)
			{
				char buf[64] = {0};
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

	bool LinuxTextBoxObject::SetValueForKey(lua_State *L, const char key[], int valueIndex)
	{
		Rtt_ASSERT(key);

		wxTextCtrl *fControl = dynamic_cast<wxTextCtrl*>(fWindow);

		if (fControl == NULL)
		{
			return false;
		}

		bool result = true;

		if (strcmp("text", key) == 0)
		{
			const char *s = lua_tostring(L, valueIndex);

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
			PlatformFont *font = LuaLibNative::ToFont(L, valueIndex);

			if (font)
			{
				int size = font->Size();
				const char *name = font->Name();
				const char *face = glyph_freetype_provider::getFace(name);

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
			const char *s = lua_tostring(L, valueIndex);
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
			const char *align = lua_tostring(L, valueIndex);
			//	jsTextFieldSetAlign(fElementID, align);
			Rtt_LogException("LinuxTextBoxObject: SetValueForKey '%s' is not implemented\n", key);
		}
		else if (strcmp("inputType", key) == 0)
		{
			const char *inputType = lua_tostring(L, valueIndex);
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

	int LinuxTextBoxObject::SetTextColor(lua_State *L)
	{
		const LuaProxyVTable &table = PlatformDisplayObject::GetTextFieldObjectProxyVTable();
		LinuxTextBoxObject *obj = (LinuxTextBoxObject*)luaL_todisplayobject(L, 1, table);

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

	int LinuxTextBoxObject::SetReturnKey(lua_State *L)
	{
		PlatformDisplayObject *o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject(L, 1);

		if (&o->ProxyVTable() == &PlatformDisplayObject::GetTextFieldObjectProxyVTable())
		{
			const char *keyType = lua_tostring(L, 2);
			//TODO: Set the return key type here.
		}
		return 0;
	}

	int LinuxTextBoxObject::SetSelection(lua_State *L)
	{
		Rtt_LogException("LinuxTextBoxObject:SetSelection() is not implemented\n");
		return 0;
	}

	LinuxTextBoxObject::myTextCtrl::myTextCtrl(LinuxTextBoxObject *parent, int x, int y, int w, int h, bool singleLine)
		: wxTextCtrl(wxGetApp().GetParent(), -1, "", wxPoint(x, y), wxSize(w, h), singleLine ? wxTE_MULTILINE : wxTE_MULTILINE)
		, fParent(parent)
		, fLuaReference(NULL)
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

		if (fParent->fHandle->IsValid())
		{
			CoronaLuaDeleteRef(fParent->fHandle->Dereference(), fLuaReference);
			fLuaReference = NULL;
		}
	}

	void LinuxTextBoxObject::myTextCtrl::onTextEvent(wxCommandEvent &e)
	{
		e.Skip();

		int eventID = 0;
		wxEventType eType = e.GetEventType();

		if (eType == wxEVT_SET_FOCUS)
		{
			eventID = 1;
		}
		else if (eType == wxEVT_TEXT)
		{
			eventID = 2;
		}
		else if (eType == wxEVT_KILL_FOCUS)
		{
			eventID = 3;
		}

		static const char* phase[] = {"", "began", "editing", "ended", "submitted"};
		Rtt_ASSERT(eventID > 0 && eventID < (sizeof(phase) / sizeof(phase[0])));

		lua_State *L = fParent->fHandle->Dereference();
		CoronaLuaNewEvent(L, "userInput"); //fEventName.c_str());
		int luaTableStackIndex = lua_gettop(L);
		int nPushed = 0;

		lua_pushstring(L, phase[eventID]);
		lua_setfield(L, luaTableStackIndex, "phase");
		nPushed++;

		// Add 'self' to the event table
		fParent->GetProxy()->PushTable(L);
		lua_setfield(L, -2, "target");
		nPushed++;

		switch (eventID)
		{
			case 1: // began
				// User begins editing "defaultField"
				break;

			case 2: // editing
			{
				wxString strval = GetValue();

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

				int pos = GetInsertionPoint();
				lua_pushnumber(L, pos);
				lua_setfield(L, luaTableStackIndex, "startPosition");
				nPushed++;

				lua_pushstring(L, strval.c_str());
				lua_setfield(L, luaTableStackIndex, "text");
				nPushed++;

				fOldValue = strval;

				break;
			}
			case 3: // ended
				// Output resulting text from "defaultField"
				break;

			default:
				break;
		}

		CoronaLuaDispatchEvent(L, fLuaReference, 0);
	}
}; // namespace Rtt
