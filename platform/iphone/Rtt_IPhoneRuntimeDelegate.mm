//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_IPhoneRuntimeDelegate.h"

#include "Rtt_RuntimeDelegatePlayer.h"
#import "CoronaAppDelegate.h"
#import "CoronaDelegate.h"

#if ! defined( Rtt_TVOS_ENV )
#import "AppDelegate.h"
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

IPhoneRuntimeDelegate::IPhoneRuntimeDelegate(
	CoronaView *owner,
	id<CoronaRuntime> coronaRuntime,
	id<CoronaDelegate> enterpriseDelegate )
:	Super( owner ),
	fCoronaRuntime( coronaRuntime )
{
	fCoronaAppDelegate = [[CoronaAppDelegate alloc] initWithEnterpriseDelegate:enterpriseDelegate];
}

IPhoneRuntimeDelegate::~IPhoneRuntimeDelegate()
{
	[fCoronaAppDelegate release];
}

id<CoronaDelegate>
IPhoneRuntimeDelegate::GetCoronaDelegate() const
{
	return fCoronaAppDelegate;
}
	
void
IPhoneRuntimeDelegate::DidInitLuaLibraries( const Runtime& sender ) const
{
	Super::DidInitLuaLibraries( sender );

	RuntimeDelegatePlayer::PreloadLibraries( sender );
}

void
IPhoneRuntimeDelegate::WillLoadMain( const Runtime& sender ) const
{
	Super::WillLoadMain( sender );
	[fCoronaAppDelegate willLoadMain:fCoronaRuntime];
}

void
IPhoneRuntimeDelegate::DidLoadMain( const Runtime& sender ) const
{
	Super::DidLoadMain( sender );
#if ! defined( Rtt_TVOS_ENV )
	AppDelegate *appDelegate = (AppDelegate *)fCoronaRuntime;
	if ([appDelegate respondsToSelector:@selector(didLoadMain:)])
	{
		[appDelegate didLoadMain:fCoronaRuntime];
	}
#endif
	[fCoronaAppDelegate didLoadMain:fCoronaRuntime];
}

void
IPhoneRuntimeDelegate::DidLoadConfig( const Runtime& sender, lua_State *L ) const
{
	Super::DidLoadConfig( sender, L );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

