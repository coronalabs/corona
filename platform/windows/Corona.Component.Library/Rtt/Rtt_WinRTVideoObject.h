// ----------------------------------------------------------------------------
// 
// Rtt_WinRTVideoObject.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

#include "Rtt_WinRTDisplayObject.h"


namespace Rtt
{

class WinRTVideoObject : public WinRTDisplayObject
{
	public:
		typedef WinRTVideoObject Self;
		typedef WinRTDisplayObject Super;

		WinRTVideoObject(const Rect& bounds);
		virtual ~WinRTVideoObject();

		virtual bool Initialize();
		virtual const LuaProxyVTable& ProxyVTable() const;
		virtual int ValueForKey(lua_State *L, const char key[]) const;
		virtual bool SetValueForKey(lua_State *L, const char key[], int valueIndex);

	protected:
		static int Load(lua_State *L);
		static int Play(lua_State *L);
		static int Pause(lua_State *L);
		static int Seek(lua_State *L);
};

} // namespace Rtt
