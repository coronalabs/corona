// ----------------------------------------------------------------------------
// 
// Rtt_WinRTWebViewObject.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

#include "Rtt_WinRTDisplayObject.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Rtt_MPlatform.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

class WinRTWebViewObject : public WinRTDisplayObject
{
	public:
		typedef WinRTWebViewObject Self;
		typedef WinRTDisplayObject Super;

		WinRTWebViewObject(const Rect& bounds);
		virtual ~WinRTWebViewObject();

		virtual bool Initialize();
		virtual const LuaProxyVTable& ProxyVTable() const;
		virtual int ValueForKey(lua_State *L, const char key[]) const;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex);

	protected:
		static int Load(lua_State *L);
		static int Request(lua_State *L);
		static int Stop(lua_State *L);
		static int Back(lua_State *L);
		static int Forward(lua_State *L);
		static int Reload(lua_State *L);
		static int Resize(lua_State *L);
};

} // namespace Rtt
