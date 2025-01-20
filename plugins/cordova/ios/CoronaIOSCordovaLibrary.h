//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CoronaIOSCordovaLibrary_H__
#define _CoronaIOSCordovaLibrary_H__

#include "CoronaCordovaLibrary.h"

// ----------------------------------------------------------------------------

@protocol CoronaRuntime;

namespace Corona
{

// ----------------------------------------------------------------------------

class IOSCordovaLibrary : public CordovaLibrary
{
	public:
		typedef CordovaLibrary Super;
		typedef IOSCordovaLibrary Self;

	public:
		IOSCordovaLibrary();

	public:
		virtual bool Initialize( lua_State *L, void *platformContext );

	public:
		// virtual int ValueForKey( lua_State *L );

	protected:
		virtual const luaL_Reg *GetFunctions() const;

	public:
		id<CoronaRuntime> GetRuntime() const { return fRuntime; }

	public:
		static int newCleaver( lua_State *L );

	private:
		id<CoronaRuntime> fRuntime;
};

// ----------------------------------------------------------------------------

} // namespace Corona

// ----------------------------------------------------------------------------

#endif // _CoronaIOSCordovaLibrary_H__
