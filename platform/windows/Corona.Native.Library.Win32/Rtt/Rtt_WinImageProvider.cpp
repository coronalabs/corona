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

#include "Rtt_WinImageProvider.h"

#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"

#include "Display/Rtt_LuaLibDisplay.h"
#include "Rtt_WinBitmap.h"
#include "Rtt_WinData.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

bool 
WinImageProvider::Supports( int source ) const
{
	Rtt_LogException("Camera not supported.\n");
	Rtt_ASSERT_NOT_IMPLEMENTED();
	return false;
}

bool 
WinImageProvider::Show( int source, const char* filePath, lua_State* L )
{
	Rtt_LogException("Camera not supported.\n");
	Rtt_ASSERT_NOT_IMPLEMENTED();
	return false;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

