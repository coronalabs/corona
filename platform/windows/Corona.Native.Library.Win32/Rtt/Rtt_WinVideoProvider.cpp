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

#include "Rtt_WinVideoProvider.h"

#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_WinBitmap.h"
#include "Rtt_WinData.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

bool 
WinVideoProvider::Supports( int source ) const
{
	Rtt_LogException("Video capture is not supported on this platform.\n");
	Rtt_ASSERT_NOT_IMPLEMENTED();
	return false;
}

bool 
WinVideoProvider::Show( int source, lua_State* L, int maxTime, int quality )
{
	Rtt_LogException("Video capture is not supported on this platform.\n");
	Rtt_ASSERT_NOT_IMPLEMENTED();
	return false;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

