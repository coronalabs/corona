//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

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
