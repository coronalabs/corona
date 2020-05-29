//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CoronaIOSLoader_H__
#define _CoronaIOSLoader_H__
#include "CoronaLua.h"

// ----------------------------------------------------------------------------

namespace Corona
{

// ----------------------------------------------------------------------------

class IOSLoader
{
	public:
		typedef IOSLoader Self;

	protected:
		static int SymbolLoader( lua_State *L );
		static int FrameworkSymbolLoader( lua_State *L );

	public:
		static void Register( lua_State *L, void *platformContext );
};

// ----------------------------------------------------------------------------

} // namespace Corona

// ----------------------------------------------------------------------------

#endif // _CoronaIOSLoader_H__
