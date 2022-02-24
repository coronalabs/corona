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

struct lua_State;

namespace Rtt
{
	class EmscriptenCPluginLoader
	{
		public:
			static int Loader(lua_State *L);
		private:
			static int LualoadCInvoker(lua_State *L);
	};
}
