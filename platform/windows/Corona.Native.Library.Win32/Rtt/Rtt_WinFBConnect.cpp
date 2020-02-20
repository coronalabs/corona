//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Core/Rtt_Build.h"

#include "Rtt_WinFBConnect.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

WinFBConnect::WinFBConnect()
:	Super()
{
}

void
WinFBConnect::Login( const char *appId, const char *permissions[], int numPermissions ) const
{
	Rtt_TRACE_SIM( ( "WARNING: facebook.login() not supported on the simulator.\n" ) );
}

void
WinFBConnect::Logout() const
{
	Rtt_TRACE_SIM( ( "WARNING: facebook.logout() not supported on the simulator.\n" ) );
}

void
WinFBConnect::Request( lua_State *L, const char *path, const char *httpMethod, int index ) const
{
	Rtt_TRACE_SIM( ( "WARNING: facebook.request() not supported on the simulator.\n" ) );
}

void
WinFBConnect::RequestOld( lua_State *L, const char *method, const char *httpMethod, int index ) const
{
	Rtt_TRACE_SIM( ( "WARNING: facebook.requestOld() not supported on the simulator.\n" ) );
}

void
WinFBConnect::ShowDialog( lua_State *L, int index ) const
{
	Rtt_TRACE_SIM( ( "WARNING: facebook.showDialog() not supported on the simulator.\n" ) );
}

void
WinFBConnect::PublishInstall( const char *appId ) const
{
	Rtt_TRACE_SIM( ( "WARNING: facebook.publishInstall() not supported on the simulator.\n" ) );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

