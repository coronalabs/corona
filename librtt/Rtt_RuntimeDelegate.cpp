//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_RuntimeDelegate.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

RuntimeDelegate::~RuntimeDelegate()
{
}

void
RuntimeDelegate::DidInitLuaLibraries( const Runtime& sender ) const
{
}

bool
RuntimeDelegate::HasDependencies( const Runtime& sender ) const
{
	return true;
}

void
RuntimeDelegate::WillLoadMain( const Runtime& sender ) const
{
}

void
RuntimeDelegate::DidLoadMain( const Runtime& sender ) const
{
}

void
RuntimeDelegate::WillLoadConfig( const Runtime& sender, lua_State *L ) const
{
}

void
RuntimeDelegate::InitializeConfig( const Runtime& sender, lua_State *L ) const
{
}

void
RuntimeDelegate::DidLoadConfig( const Runtime& sender, lua_State *L ) const
{
}

void
RuntimeDelegate::WillSuspend( const Runtime& sender ) const
{
}

void
RuntimeDelegate::DidSuspend( const Runtime& sender ) const
{
}

void
RuntimeDelegate::WillResume( const Runtime& sender ) const
{
}

void
RuntimeDelegate::DidResume( const Runtime& sender ) const
{
}

void
RuntimeDelegate::WillDestroy( const Runtime& sender ) const
{
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
