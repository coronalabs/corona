//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include "Core/Rtt_Build.h"
#include "Rtt_LinuxVideoObject.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibMedia.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_Event.h"
#include "wx/app.h"
#include "wx/url.h"
#include "Rtt_LinuxContext.h"

#if (wxUSE_MEDIACTRL == 1)

namespace Rtt
{
	LinuxVideoObject::LinuxVideoObject(const Rect &bounds)
		: Super(bounds, "video")
		, fAutoPlay(true)
	{
	}

	LinuxVideoObject::~LinuxVideoObject()
	{
	}

	bool LinuxVideoObject::Initialize()
	{
		if (Super::Initialize())
		{
			// todo: check if video available
			fBounds = StageBounds();
			return true;
		}

		return false;
	}

	const LuaProxyVTable& LinuxVideoObject::ProxyVTable() const
	{
		return PlatformDisplayObject::GetVideoObjectProxyVTable();
	}

	int LinuxVideoObject::ValueForKey(lua_State *L, const char key[]) const
	{
		Rtt_ASSERT(key);

		const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
		LinuxVideoObject* obj = (LinuxVideoObject *)luaL_todisplayobject(L, 1, table);

		int result = 1;

		if (strcmp("currentTime", key) == 0)
		{
			myMediaCtrl *video = dynamic_cast<myMediaCtrl*>(obj->fWindow);

			if (video)
			{
				double val = video->Tell() / 1000.0f;  // in seconds
				lua_pushnumber(L, val);
			}
		}
		else if (strcmp("totalTime", key) == 0)
		{
			myMediaCtrl *video = dynamic_cast<myMediaCtrl*>(obj->fWindow);

			if (video)
			{
				double val = video->Length() / 1000.0; // in seconds
				lua_pushnumber(L, val);
			}
		}
		else if (strcmp("isMuted", key) == 0)
		{
			myMediaCtrl *video = dynamic_cast<myMediaCtrl*>(obj->fWindow);

			if (video)
			{
				double vol = video->GetVolume();
				bool val = vol <= 0;
				lua_pushboolean(L, val);
			}
		}
		else if (strcmp("fillMode", key) == 0)
		{
		}
		else if (strcmp("load", key) == 0)
		{
			lua_pushcfunction(L, LuaLoad);
		}
		else if (strcmp("play", key) == 0)
		{
			lua_pushcfunction(L, Play);
		}
		else if (strcmp("pause", key) == 0)
		{
			lua_pushcfunction(L, Pause);
		}
		else if (strcmp("seek", key) == 0)
		{
			lua_pushcfunction(L, Seek);
		}
		else if (strcmp("isToggleEnabled", key) == 0)
		{
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

	bool LinuxVideoObject::SetValueForKey(lua_State *L, const char key[], int valueIndex)
	{
		Rtt_ASSERT(key);

		const LuaProxyVTable &table = PlatformDisplayObject::GetVideoObjectProxyVTable();
		LinuxVideoObject *obj = (LinuxVideoObject*)luaL_todisplayobject(L, 1, table);

		bool result = true;

		if (strcmp("isToggleEnabled", key) == 0)
		{
		}
		else if (strcmp("isMuted", key) == 0)
		{
			bool val = lua_toboolean(L, valueIndex);
			myMediaCtrl *video = dynamic_cast<myMediaCtrl*>(obj->fWindow);

			if (video)
			{
				video->SetVolume(val ? 0 : 1);
			}
		}
		else if (strcmp("fillMode", key) == 0)
		{
		}
		else
		{
			result = Super::SetValueForKey(L, key, valueIndex);
		}

		return result;
	}

	int LinuxVideoObject::LuaLoad(lua_State *L)
	{
		const LuaProxyVTable &table = PlatformDisplayObject::GetVideoObjectProxyVTable();
		LinuxVideoObject *o = (LinuxVideoObject *)luaL_todisplayobject(L, 1, table);

		// Get the absolute path to this file and pass it to Java.
		int nextArg = 2;
		String sourceWithPath;
		bool isRemote = false;
		const char* source = LuaLibMedia::GetLocalOrRemotePath(L, nextArg, sourceWithPath, isRemote);

		o->load(source, isRemote);
		return 0;
	}

	void LinuxVideoObject::load(const char *source, bool isRemote)
	{
		if (source)
		{
			if (fWindow)
			{
				delete fWindow;
			}

			fWindow = new myMediaCtrl(this, wxGetApp().GetParent(), 0, 0, fBounds.Width(), fBounds.Height());
			myMediaCtrl *video = dynamic_cast<myMediaCtrl*>(fWindow);
			bool rc = isRemote ? video->Load(wxURI(source)) : video->Load(source);

			if (rc)
			{
				if (fAutoPlay)
				{
					video->Play();
				}
			}
			else
			{
				Rtt_LogException("Failed to load video from %s\n", source);
			}
		}
	}

	int LinuxVideoObject::Play(lua_State *L)
	{
		const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
		LinuxVideoObject *o = (LinuxVideoObject *)luaL_todisplayobject(L, 1, table);

		if (o)
		{
			wxMediaCtrl *video = dynamic_cast<wxMediaCtrl*>(o->fWindow);

			if (video)
			{
				video->Play();
			}
		}

		return 0;
	}

	int LinuxVideoObject::Pause(lua_State *L)
	{
		const LuaProxyVTable &table = PlatformDisplayObject::GetVideoObjectProxyVTable();
		LinuxVideoObject *o = (LinuxVideoObject *)luaL_todisplayobject(L, 1, table);

		if (o)
		{
			wxMediaCtrl *video = dynamic_cast<wxMediaCtrl*>(o->fWindow);

			if (video)
			{
				video->Pause();
			}
		}

		return 0;
	}

	int LinuxVideoObject::Seek(lua_State *L)
	{
		const LuaProxyVTable &table = PlatformDisplayObject::GetVideoObjectProxyVTable();
		LinuxVideoObject *o = (LinuxVideoObject *)luaL_todisplayobject(L, 1, table);

		int seekTo = -1;

		if (lua_isnumber(L, 2))
		{
			seekTo = (int)lua_tonumber(L, 2);
		}

		if (o && seekTo > -1)
		{
			wxMediaCtrl *video = dynamic_cast<wxMediaCtrl*>(o->fWindow);

			if (video)
			{
				//	video->Seek(seekTo * 1000);
			}
		}
		return 0;
	}

	int LinuxVideoObject::addEventListener(lua_State *L)
	{
		const LuaProxyVTable &table = PlatformDisplayObject::GetVideoObjectProxyVTable();
		LinuxVideoObject *o = (LinuxVideoObject *)luaL_todisplayobject(L, 1, table);

		if (o && lua_isstring(L, 2))
		{
			const char *eventName = lua_tostring(L, 2);
			myMediaCtrl *media = dynamic_cast<myMediaCtrl*>(o->fWindow);

			// Store callback
			if (media && CoronaLuaIsListener(L, 3, eventName))
			{
				// deleted old
				if (media->fLuaReference)
				{
					CoronaLuaDeleteRef(L, media->fLuaReference);
				}
				media->fLuaReference = CoronaLuaNewRef(L, 3); // listenerIndex=3
			}
		}

		return 0;
	}

	//  myMediaCtrl
	LinuxVideoObject::myMediaCtrl::myMediaCtrl(LinuxVideoObject *parent, wxWindow *panel, float x, float y, float w, float h)
		: wxMediaCtrl(panel, wxID_ANY, "", wxPoint(x, y), wxSize(w, h))
		, fParent(parent)
		, fLuaReference(NULL)
	{
		Connect(wxEVT_MEDIA_LOADED, wxCommandEventHandler(myMediaCtrl::onMediaEvent));
		Connect(wxEVT_MEDIA_STOP, wxCommandEventHandler(myMediaCtrl::onMediaEvent));
	}

	LinuxVideoObject::myMediaCtrl::~myMediaCtrl()
	{
		Disconnect(wxEVT_MEDIA_STOP);

		if (fParent->fHandle && fParent->fHandle->IsValid())
		{
			CoronaLuaDeleteRef(fParent->fHandle->Dereference(), fLuaReference);
			fLuaReference = NULL;
		}
	}

	void LinuxVideoObject::myMediaCtrl::onMediaEvent(wxCommandEvent& e)
	{
		wxEventType eType = e.GetEventType();
		const char *phase = NULL;

		if (eType == wxEVT_MEDIA_LOADED)
		{
			phase = "ready";
		}
		else if (eType == wxEVT_MEDIA_STOP)
		{
			phase = "ended";
		}
		else
		{
			return;
		}

		if (fParent->fHandle && fParent->fHandle->IsValid())
		{
			lua_State *L = fParent->fHandle->Dereference();
			CoronaLuaNewEvent(L, "video"); //fEventName.c_str());
			int luaTableStackIndex = lua_gettop(L);
			int nPushed = 0;

			lua_pushstring(L, phase);
			lua_setfield(L, luaTableStackIndex, "phase");
			nPushed++;

			// Add 'self' to the event table
			fParent->GetProxy()->PushTable(L);
			lua_setfield(L, -2, "target");
			nPushed++;

			CoronaLuaDispatchEvent(L, fLuaReference, 0);
		}
	}
}; // namespace Rtt

#endif
