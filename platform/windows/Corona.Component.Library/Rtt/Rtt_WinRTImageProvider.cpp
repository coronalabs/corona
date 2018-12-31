// ----------------------------------------------------------------------------
// 
// Rtt_WinRTImageProvider.cpp
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "Rtt_WinRTImageProvider.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Display/Rtt_BitmapPaint.h"
#	include "Display/Rtt_LuaLibDisplay.h"
#	include "Rtt_Lua.h"
#	include "Rtt_LuaContext.h"
#	include "Rtt_Runtime.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

#pragma region Constructors/Destructors
/// Creates a new image provider.
/// @param handle Reference to Lua state used to send image selection notifications to a Lua listener function.
WinRTImageProvider::WinRTImageProvider(const ResourceHandle<lua_State> & handle)
:	PlatformImageProvider(handle)
{
}

#pragma endregion


#pragma region Public Member Functions
/// Determines if the given image source (Camera, Photo Library, etc.) is supported on this platform.
/// @param source Unique integer ID of the image source. IDs are defined in PlatformImageProvider class.
/// @return Returns true if given image source is supported on this platform. Returns false if not.
bool WinRTImageProvider::Supports(int source) const
{
	return false;
}

/// Displays a window for selecting an image.
/// @param source Unique integer ID indicating what kind of window to display such as the Camera or Photo Library.
/// @param filePath The path\file name to save the selected image file to.
///                 Set to NULL to not save to file and display the selected photo as a display object instead.
/// @return Returns true if the window was shown.
///         Returns false if given source is not provided or if an image provider window is currently shown.
bool WinRTImageProvider::Show(int source, const char* filePath, lua_State* L)
{
	// Do not continue if:
	// 1) Given image source type is not supported on this platform.
	// 2) Image providing window is already shown.

	// If given file path is an empty string, then change it to NULL.
	if (filePath && (strlen(filePath) <= 0))
	{
		filePath = NULL;
	}

	// Show the image provider window.
	return false;
}

#pragma endregion

} // namespace Rtt
