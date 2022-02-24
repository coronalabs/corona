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

#if 0
namespace Rtt
{
	class LinuxTextBoxObject : public LinuxDisplayObject
	{
	public:
		typedef LinuxTextBoxObject Self;
		typedef LinuxDisplayObject Super;

		struct myTextCtrl : public wxTextCtrl
		{
			myTextCtrl(LinuxTextBoxObject *parent, bool singleLine);
			virtual ~myTextCtrl();
			void onTextEvent(wxCommandEvent &e);

		private:
			LinuxTextBoxObject* fLinuxTextBoxObject;
		};

		LinuxTextBoxObject(const Rect &bounds, bool isSingleLine);
		virtual ~LinuxTextBoxObject();

		virtual bool Initialize();
		virtual const LuaProxyVTable& ProxyVTable() const;
		virtual int ValueForKey(lua_State *L, const char key[]) const;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex);
		static int addEventListener(lua_State *L);
		void dispatch(const char* phase);

	protected:
		static int SetTextColor(lua_State *L);
		static int SetReturnKey(lua_State *L);
		static int SetSelection(lua_State *L);

	private:

		myTextCtrl* getTextCtrl() const { return dynamic_cast<myTextCtrl*>(fWindow); }

		/// Set TRUE if this is a single line text field. Set FALSE for a multiline text box.
		/// This value is not expected to change after initialization.
		bool fIsSingleLine;

		wxString fOldValue;
	};
}; // namespace Rtt
#endif