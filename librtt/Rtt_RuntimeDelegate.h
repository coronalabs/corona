//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_RuntimeDelegate__
#define __Rtt_RuntimeDelegate__

#include "Rtt_MRuntimeDelegate.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class RuntimeDelegate : public MRuntimeDelegate
{
	public:
		virtual ~RuntimeDelegate();

	public:
		// TODO: These stubs should be REMOVED. Instead, add to the concrete
		// derived class.
		// On the Mac, these are: HomeScreenRuntimeDelegate and RuntimeDelegateWrapper
		virtual void DidInitLuaLibraries( const Runtime& sender ) const;
		virtual bool HasDependencies( const Runtime& sender ) const;
        virtual void WillLoadMain( const Runtime& sender ) const;
		virtual void DidLoadMain( const Runtime& sender ) const;
		virtual void WillLoadConfig( const Runtime& sender, lua_State *L ) const;
		virtual void InitializeConfig( const Runtime& sender, lua_State *L ) const;
		virtual void DidLoadConfig( const Runtime& sender, lua_State *L ) const;
		virtual void WillSuspend( const Runtime& sender ) const;
		virtual void DidSuspend( const Runtime& sender ) const;
		virtual void WillResume( const Runtime& sender ) const;
		virtual void DidResume( const Runtime& sender ) const;
		virtual void WillDestroy( const Runtime& sender ) const;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_RuntimeDelegate__
