//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_EmscriptenDisplayObject.h"
#include "Rtt_MPlatform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class EmscriptenWebViewObject : public EmscriptenDisplayObject
{
	public:
		typedef EmscriptenWebViewObject Self;
		typedef EmscriptenDisplayObject Super;

		EmscriptenWebViewObject(const Rect& bounds);
		virtual ~EmscriptenWebViewObject();

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
		static int InjectJS(lua_State *L);
		static int RegisterCallback(lua_State *L);
		static int On(lua_State *L);
		static int Send(lua_State *L);
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
