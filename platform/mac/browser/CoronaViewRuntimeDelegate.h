//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_CoronaViewRuntimeDelegate_H__
#define _Rtt_CoronaViewRuntimeDelegate_H__

#include "Rtt_RuntimeDelegatePlayer.h"

// ----------------------------------------------------------------------------

@class CoronaView;
@protocol CoronaRuntime;

namespace Rtt
{

// ----------------------------------------------------------------------------

class CoronaViewRuntimeDelegate : public RuntimeDelegatePlayer
{
	public:
		CoronaViewRuntimeDelegate( CoronaView *owner );
		virtual ~CoronaViewRuntimeDelegate();

    public:
        virtual void DidInitLuaLibraries( const Runtime& sender ) const;
		virtual bool HasDependencies( const Runtime& sender ) const;
		virtual void InitializeConfig( const Runtime& sender, lua_State *L ) const;
		virtual void DidLoadConfig( const Runtime& sender, lua_State *L ) const;

	protected:
		virtual id< CoronaRuntime > GetPluginContext() const;

	private:
		CoronaView *fOwner; // Weak ref
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_CoronaViewRuntimeDelegate_H__
