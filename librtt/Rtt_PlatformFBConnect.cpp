//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_PlatformFBConnect.h"

#include "Rtt_LuaResource.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

PlatformFBConnect::PlatformFBConnect()
:	fListener( NULL )
{
}

PlatformFBConnect::~PlatformFBConnect()
{
	Rtt_DELETE( fListener );
}

void
PlatformFBConnect::SetListener( LuaResource *listener )
{
	if ( fListener != listener )
	{
		Rtt_DELETE( fListener );
		fListener = listener;
	}
}

void
PlatformFBConnect::DispatchEvent( const MEvent& e )
{
	LuaResource *resource = fListener;

	if ( resource )
	{
		resource->DispatchEvent( e );
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

