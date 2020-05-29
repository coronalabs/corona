//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_RuntimeDelegatePlayer__
#define __Rtt_RuntimeDelegatePlayer__

#include "Rtt_RuntimeDelegate.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// This class is used by Corona/Native to ensure that certain optional
// modules are loaded into Lua.
class RuntimeDelegatePlayer : public RuntimeDelegate
{
	public:
		virtual ~RuntimeDelegatePlayer();

	public:
		virtual void DidInitLuaLibraries( const Runtime& sender ) const;

		// Convenience class to preload libraries without requiring a subclass.
		static void PreloadLibraries( const Runtime& sender );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_RuntimeDelegatePlayer__
