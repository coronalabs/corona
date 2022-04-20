//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_LinuxCEF.h"
#include "Rtt_LinuxDisplayObject.h"
#include "Rtt_MPlatform.h"

namespace Rtt
{
	class LinuxWebView : public LinuxDisplayObject
	{
	public:
		typedef LinuxWebView Self;
		typedef LinuxDisplayObject Super;

		LinuxWebView(const Rect &bounds);
		virtual ~LinuxWebView();
		virtual bool Initialize();
		virtual const LuaProxyVTable &ProxyVTable() const;
		virtual int ValueForKey(lua_State *L, const char key[]) const;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex);
		virtual void Prepare(const Display &display) override;
		void openURL(const char *url);
		bool Close();
		void Draw(Renderer& renderer) const override;
		bool ProcessEvent(const SDL_Event& evt) override;

	protected:

		static int Load(lua_State *L);
		static int Request(lua_State *L);
		static int Stop(lua_State *L);
		static int Back(lua_State *L);
		static int Forward(lua_State *L);
		static int Reload(lua_State *L);
		static int Resize(lua_State *L);
		static void onWebPopupNavigatingEvent();
		static void onWebPopupNavigatedEvent();
		static void onWebPopupLoadedEvent();
		static void onWebPopupErrorEvent();
		static int OnDeleteCookies(lua_State *L);

		smart_ptr<WebView> fWebView;

	};
}; // namespace Rtt
