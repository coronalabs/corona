//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_MRuntimeDelegate__
#define __Rtt_MRuntimeDelegate__

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

class Runtime;

// ----------------------------------------------------------------------------

class MRuntimeDelegate
{
	public:
		virtual void DidInitLuaLibraries( const Runtime& sender ) const = 0;
		virtual bool HasDependencies( const Runtime& sender ) const = 0;

    public:
        virtual void WillLoadMain( const Runtime& sender ) const = 0;
		virtual void DidLoadMain( const Runtime& sender ) const = 0;

	public:
		virtual void WillLoadConfig( const Runtime& sender, lua_State *L ) const = 0;
		virtual void InitializeConfig( const Runtime& sender, lua_State *L ) const = 0;
		virtual void DidLoadConfig( const Runtime& sender, lua_State *L ) const = 0;

	public:
		virtual void WillSuspend( const Runtime& sender ) const = 0;
		virtual void DidSuspend( const Runtime& sender ) const = 0;
		virtual void WillResume( const Runtime& sender ) const = 0;
		virtual void DidResume( const Runtime& sender ) const = 0;
		virtual void WillDestroy( const Runtime& sender ) const = 0;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_MRuntimeDelegate__
