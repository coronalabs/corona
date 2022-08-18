//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core/Rtt_Build.h"
#include <string>

struct lua_State;

namespace Rtt
{
	class EmscriptenJSPluginLoader
	{
		public:
			static int Loader(lua_State *L);
		private:
			static int Open(lua_State *L);
			static int Index(lua_State *L);
			static int NewIndex(lua_State *L);
			static int JSFunctionWrapper(lua_State *L);
			static int EncodeException(lua_State*L);
		public:
			static int json_decode(lua_State* L, const char* json);
			static const char* json_encode(lua_State* L, int valueIndex, bool storeFunction);
	};
}
