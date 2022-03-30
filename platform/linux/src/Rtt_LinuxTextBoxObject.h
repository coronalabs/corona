//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Corona/CoronaLua.h"
#include "Rtt_LinuxDisplayObject.h"
#include "Display/Rtt_TextObject.h"
#include "imgui/imgui.h"

namespace Rtt
{
	class LinuxTextBoxObject : public LinuxDisplayObject
	{
	public:
		typedef LinuxTextBoxObject Self;
		typedef LinuxDisplayObject Super;

		LinuxTextBoxObject(const Rect &bounds, bool isSingleLine);
		virtual ~LinuxTextBoxObject();

		virtual const LuaProxyVTable& ProxyVTable() const;
		virtual int ValueForKey(lua_State *L, const char key[]) const;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex);
		static int addEventListener(lua_State *L);
		void dispatch(const char* phase, int pos, ImWchar ch);
		void Draw() override;		// for ImGui renderer

	protected:

		static int SetTextColor(lua_State *L);
		static int SetReturnKey(lua_State *L);
		static int SetSelection(lua_State *L);

	private:

		enum class InputType
		{
			undefined,
			number,
			decimal,
			phone,
			url,
			email,
			noemoji
		};

		bool fIsSingleLine;
		bool fIsEditable;
		bool fIsSecure;
		bool fHasFocus;
		InputType fInputType;
		char fValue[1024];
		char fOldValue[1024];
		float fFontSize;
	};
}; // namespace Rtt
