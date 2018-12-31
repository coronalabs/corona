// ----------------------------------------------------------------------------
// 
// Rtt_WinRTVideoProvider.cpp
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "Rtt_WinRTVideoProvider.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Rtt_Lua.h"
#	include "Rtt_LuaContext.h"
#	include "Rtt_Runtime.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

#pragma region Constructors/Destructors
/// Creates a new Video provider.
/// @param handle Reference to Lua state used to send Video selection notifications to a Lua listener function.
WinRTVideoProvider::WinRTVideoProvider(const ResourceHandle<lua_State> & handle)
:	PlatformVideoProvider(handle)
{
}

#pragma endregion


#pragma region Public Member Functions
/// Determines if the given Video source (Camera, Photo Library, etc.) is supported on this platform.
/// @param source Unique integer ID of the Video source. IDs are defined in PlatformVideoProvider class.
/// @return Returns true if given Video source is supported on this platform. Returns false if not.
bool WinRTVideoProvider::Supports(int source) const
{
	return false;
}

/// Displays a window for selecting an Video.
/// @param source Unique integer ID indicating what kind of window to display such as the Camera or Photo Library.
/// @return Returns true if the window was shown.
///         Returns false if given source is not provided or if an Video provider window is currently shown.
bool WinRTVideoProvider::Show(int source, lua_State* L, int maxTime, int quality)
{
	// Show the Video provider window.
	return false;
}

#pragma endregion

} // namespace Rtt
