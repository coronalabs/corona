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

#if (wxUSE_MEDIACTRL == 1)

namespace Rtt
{
	struct LinuxVideoObject;

	struct LinuxVideoObject : public LinuxDisplayObject
	{
		typedef LinuxVideoObject Self;
		typedef LinuxDisplayObject Super;

		struct myMediaCtrl : public wxMediaCtrl
		{
			myMediaCtrl(LinuxVideoObject* fLinuxVideoObject);
			virtual ~myMediaCtrl();
			void onMediaEvent(wxMediaEvent& e);

		private:
			LinuxVideoObject* fLinuxVideoObject;
		};

		LinuxVideoObject(const Rect& bounds);
		virtual ~LinuxVideoObject();
		virtual bool Initialize();
		virtual const LuaProxyVTable& ProxyVTable() const;
		virtual int ValueForKey(lua_State* L, const char key[]) const;
		virtual bool SetValueForKey(lua_State* L, const char key[], int valueIndex);
		void load(const char* source, bool isRemote);
		void dispatch(const char* ev);

	protected:
		static int LuaLoad(lua_State* L);
		static int Play(lua_State* L);
		static int Pause(lua_State* L);
		static int Seek(lua_State* L);
		static int addEventListener(lua_State* L);

	private:
		myMediaCtrl* getMediaCtrl() const { return dynamic_cast<myMediaCtrl*>(fWindow); }

	};

}; // namespace Rtt

#endif
