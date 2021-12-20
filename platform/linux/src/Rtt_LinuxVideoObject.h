//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_LinuxDisplayObject.h"
#include "Corona/CoronaLua.h"
#include <string.h>
#include "wx/wx.h"
#include "wx/mediactrl.h"

#if (wxUSE_MEDIACTRL == 1)

namespace Rtt
{
	class LinuxVideoObject : public LinuxDisplayObject
	{
	public:
		struct myMediaCtrl : public wxMediaCtrl
		{
			myMediaCtrl(LinuxVideoObject *parent, wxWindow *panel, float x, float y, float w, float h);
			virtual ~myMediaCtrl();

			void dispatch(wxCommandEvent &e);
			void onMediaEvent(wxCommandEvent &e);

			LinuxVideoObject *fParent;
			CoronaLuaRef fLuaReference;
			wxString fURL;
		};

		typedef LinuxVideoObject Self;
		typedef LinuxDisplayObject Super;

		LinuxVideoObject(const Rect &bounds);
		virtual ~LinuxVideoObject();
		virtual bool Initialize();
		virtual const LuaProxyVTable &ProxyVTable() const;
		virtual int ValueForKey(lua_State *L, const char key[]) const;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex);
		void dispatch(int eventID);
		void load(const char *source, bool isRemote);

	public:
		bool fAutoPlay;
		Rect fBounds;

	protected:
		static int LuaLoad(lua_State *L);
		static int Play(lua_State *L);
		static int Pause(lua_State *L);
		static int Seek(lua_State *L);
		static int addEventListener(lua_State *L);
	};
}; // namespace Rtt

#endif
