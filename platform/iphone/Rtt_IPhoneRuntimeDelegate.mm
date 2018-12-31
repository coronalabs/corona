//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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

