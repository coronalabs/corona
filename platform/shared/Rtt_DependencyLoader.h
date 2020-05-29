//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _DependencyLoader_H__
#define _DependencyLoader_H__

#include "CoronaLua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{
	class Runtime;
}

namespace Corona
{

// ----------------------------------------------------------------------------

class DependencyLoader
{
	public:
		static int CCDataLoader( lua_State *L );
		static bool CCDependencyCheck( const Rtt::Runtime& sender );
		
	private:
		static const char *GetK();
};

// ----------------------------------------------------------------------------

} // namespace Corona

// ----------------------------------------------------------------------------

#endif // _DependencyLoader_H__
