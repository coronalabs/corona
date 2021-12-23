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
#include <wx/textctrl.h>

namespace Rtt
{
	class LinuxTextBoxObject : public LinuxDisplayObject
	{
	public:
		typedef LinuxTextBoxObject Self;
		typedef LinuxDisplayObject Super;

		struct myTextCtrl : public wxTextCtrl
		{
			myTextCtrl(LinuxTextBoxObject *parent, int x, int y, int w, int h, bool singleLine);
			virtual ~myTextCtrl();
			void dispatch(wxCommandEvent &e);
			void onTextEvent(wxCommandEvent &e);

			LinuxTextBoxObject *fParent;
			CoronaLuaRef fLuaReference;
			wxString fOldValue;
		};

		LinuxTextBoxObject(const Rect &bounds, bool isSingleLine);
		virtual ~LinuxTextBoxObject();
		virtual bool Initialize();
		virtual const LuaProxyVTable &ProxyVTable() const;
		virtual int ValueForKey(lua_State *L, const char key[]) const;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex);
		virtual void Prepare(const Display &display) override;
		static int addEventListener(lua_State *L);

	protected:
		static int SetTextColor(lua_State *L);
		static int SetReturnKey(lua_State *L);
		static int SetSelection(lua_State *L);

	private:
		/// Set TRUE if this is a single line text field. Set FALSE for a multiline text box.
		/// This value is not expected to change after initialization.
		bool fIsSingleLine;
	};
}; // namespace Rtt
