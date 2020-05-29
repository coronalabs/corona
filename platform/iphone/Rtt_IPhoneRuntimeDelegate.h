//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneRuntimeDelegate_H__
#define _Rtt_IPhoneRuntimeDelegate_H__

#include "CoronaViewRuntimeDelegate.h"

// ----------------------------------------------------------------------------

@class CoronaAppDelegate;
@class CoronaView;
@protocol CoronaDelegate;
@protocol CoronaRuntime;

namespace Rtt
{

// ----------------------------------------------------------------------------

class IPhoneRuntimeDelegate : public CoronaViewRuntimeDelegate
{
	public:
		typedef CoronaViewRuntimeDelegate Super;

	public:
		IPhoneRuntimeDelegate(
			CoronaView *owner,
			id<CoronaRuntime> coronaRuntime,
			id<CoronaDelegate> enterpriseDelegate );
		~IPhoneRuntimeDelegate();

    public:
		virtual void DidInitLuaLibraries( const Runtime& sender ) const;
		//virtual bool HasDependencies( const Runtime& sender ) const;
		virtual void WillLoadMain( const Runtime& sender ) const;
		virtual void DidLoadMain( const Runtime& sender ) const;
		virtual void DidLoadConfig( const Runtime& sender, lua_State *L ) const;

	public:
		virtual id<CoronaDelegate> GetCoronaDelegate() const;
	
	private:
		id<CoronaRuntime> fCoronaRuntime; // Weak ref
		CoronaAppDelegate *fCoronaAppDelegate;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneRuntimeDelegate_H__
