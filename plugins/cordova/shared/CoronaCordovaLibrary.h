//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CoronaCordovaLibrary_H__
#define _CoronaCordovaLibrary_H__

#include "CoronaLuaLibrary.h"

// ----------------------------------------------------------------------------

CORONA_EXPORT int luaopen_cordova( lua_State *L );

// ----------------------------------------------------------------------------

namespace Corona
{

// ----------------------------------------------------------------------------

class CordovaLibrary : public LuaLibrary
{
	public:
		typedef CordovaLibrary Self;
		typedef LuaLibrary Super;

	public:
		static const char kName[];

	protected:
		virtual lua_CFunction GetFactory() const;
};

// ----------------------------------------------------------------------------

} // namespace Corona

// ----------------------------------------------------------------------------

#endif // _CoronaCordovaLibrary_H__
