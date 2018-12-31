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

#include "Rtt_AndroidImageProvider.h"

#include "Display/Rtt_BitmapPaint.h"
#include "Display/Rtt_LuaLibDisplay.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_AndroidBitmap.h"
#include "Rtt_AndroidData.h"
#include "Rtt_Runtime.h"
#include "NativeToJavaBridge.h"


// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

/// Creates a new image provider for Android.
/// @param handle Reference to Lua state used to send image selection notifications to a Lua listener function.
AndroidImageProvider::AndroidImageProvider( const ResourceHandle<lua_State> & handle, NativeToJavaBridge *ntjb )
:	PlatformImageProvider( handle ),
	fIsImageProviderShown( false ),
	fShouldDisplaySelectedImage( true ),
	fNativeToJavaBridge(ntjb)
{
}

/// Determines if the given image source (Camera, Photo Library, etc.) is supported on this platform.
/// @param source Unique integer ID of the image source. IDs are defined in PlatformImageProvider class.
/// @return Returns true if given image source is supported on this platform. Returns false if not.
bool 
AndroidImageProvider::Supports( int source ) const
{
	return fNativeToJavaBridge->HasMediaSource(source);
}

/// Determines if we have access to the given image source (Camera, Photo Library, etc.).
/// @param source Unique integer ID of the image source. IDs are defined in PlatformImageProvider class.
/// @return Returns true if we have access to the given image source. Returns false if not.
bool 
AndroidImageProvider::HasAccessTo( int source ) const
{
	return fNativeToJavaBridge->HasAccessToMediaSource(source);
}

/// Displays a window for selecting an image.
/// @param source Unique integer ID indicating what kind of window to display such as the Camera or Photo Library.
/// @param filePath The path\file name to save the selected image file to.
///                 Set to NULL to not save to file and display the selected photo as a display object instead.
/// @return Returns true if the window was shown.
///         Returns false if given source is not provided or if an image provider window is currently shown.
bool
AndroidImageProvider::Show( int source, const char* filePath, lua_State* L )
{
	// Do not continue if:
	// 1) Given image source type is not supported on this platform.
	// 2) Image providing window is already shown.
	bool isSupported = Rtt_VERIFY( Supports( source ) );
	if (!isSupported || fIsImageProviderShown)
	{
		EndSession();
		return false;
	}
	
	// If given file path is an empty string, then change it to NULL.
	if (filePath && (strlen(filePath) <= 0))
	{
		filePath = NULL;
	}
	
	// Show the image provider window.
	fIsImageProviderShown = true;
	fShouldDisplaySelectedImage = (NULL == filePath);
	fNativeToJavaBridge->ShowImagePicker(source, filePath);
	return true;
}

/// To be called by the JNI bridge to indicate that the window has been closed.
/// Creates a display object for the selected image (if selected) and then invokes the Lua listener.
/// @param selectedImageFileName The path\file name of the image that was selected.
///                              Set to NULL or empty string to indicate that the user canceled out of the window.
void
AndroidImageProvider::CloseWithResult( const char *selectedImageFileName )
{
	// Do not continue if the Show() function was not called first.
	if (!fIsImageProviderShown)
	{
		return;
	}
	
	// Flag that the window has been closed.
	fIsImageProviderShown = false;
	
	// Invoke the Lua listener to indicate that the window was closed.
	AndroidImageProviderResult result;
	result.SelectedImageFileName = selectedImageFileName;
	result.ShouldDisplaySelectedImage = fShouldDisplaySelectedImage;
	DidDismiss(AddProperties, (void*)&result);
}

/// Static callback function used by the inherited DidDismiss() function to add properties to the a Lua listener's event table.
/// This function overrides the default AddProperties() behavior because we need to store the image file path for all display objects.
/// This is because Android needs to dump all textures when exiting the activity and reload them when returning to the activity
/// and the only way to restore the textures is to persist them in image files to be read on app resume.
void
AndroidImageProvider::AddProperties( lua_State *L, void* userdata )
{
	// Validate.
	if (!L || !userdata)
	{
		return;
	}
	
	// Add additional properties to be passed into the Lua listener.
	AndroidImageProviderResult *resultPointer = (AndroidImageProviderResult*)userdata;
	bool wasCompleted = (resultPointer->SelectedImageFileName && (strlen(resultPointer->SelectedImageFileName) > 0));
	if (resultPointer->ShouldDisplaySelectedImage)
	{
		// Create a display object for the selected image and pass it via "target" property.
		// A NULL or empty string means a selection was not made and the task was not completed.
		if (wasCompleted)
		{
			Runtime& runtime = *(LuaContext::GetRuntime( L ));
			BitmapPaint *paint = BitmapPaint::NewBitmap( runtime, resultPointer->SelectedImageFileName, MPlatform::kUnknownDir, 0 );
			LuaLibDisplay::PushImage( L, NULL, paint, runtime.GetDisplay(), NULL );
			lua_setfield( L, -2, "target" );
		}
	}
	else
	{
		// We are saving the image to file instead. The "event.target" property should be nil in this case.
	}
	lua_pushboolean( L, wasCompleted ? 1 : 0 );
	lua_setfield( L, -2, "completed" );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

