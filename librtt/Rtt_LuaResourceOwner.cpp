//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_LuaResourceOwner.h"

#include "Rtt_LuaResource.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

LuaResourceOwner::LuaResourceOwner()
:	fResource( NULL )
{
}

LuaResourceOwner::~LuaResourceOwner()
{
	Rtt_DELETE( fResource );
}

void
LuaResourceOwner::SetResource( LuaResource *resource )
{
	if ( resource != fResource )
	{
		Rtt_DELETE( fResource );
		fResource = resource;
	}
}

void
LuaResourceOwner::DispatchEvent( const MEvent& e )
{
	LuaResource *resource = fResource;
	
	if ( resource )
	{
		resource->DispatchEvent( e );
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

