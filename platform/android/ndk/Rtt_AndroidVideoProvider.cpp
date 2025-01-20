//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"

#include "Rtt_AndroidVideoProvider.h"

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

/// Creates a new Video provider for Android.
/// @param handle Reference to Lua state used to send Video selection notifications to a Lua listener function.
AndroidVideoProvider::AndroidVideoProvider( const ResourceHandle<lua_State> & handle, NativeToJavaBridge *ntjb )
:	PlatformVideoProvider( handle ),
	fIsVideoProviderShown( false ),
	fNativeToJavaBridge(ntjb)
{
}

/// Determines if the given Video source (Camera, Photo Library, etc.) is supported on this platform.
/// @param source Unique integer ID of the Video source. IDs are defined in PlatformVideoProvider class.
/// @return Returns true if given Video source is supported on this platform. Returns false if not.
bool 
AndroidVideoProvider::Supports( int source ) const
{
	return fNativeToJavaBridge->HasMediaSource(source);
}

/// Displays a window for selecting an Video.
/// @param source Unique integer ID indicating what kind of window to display such as the Camera or Photo Library.
/// @return Returns true if the window was shown.
///         Returns false if given source is not provided or if an Video provider window is currently shown.
bool
AndroidVideoProvider::Show( int source, lua_State* L, int maxTime, int quality )
{
	// Do not continue if:
	// 1) Given Video source type is not supported on this platform.
	// 2) Video providing window is already shown.
	bool isSupported = Rtt_VERIFY( Supports( source ) );
	if (!isSupported || fIsVideoProviderShown)
	{
		EndSession();
		return false;
	}
	
	// Show the Video provider window.
	fIsVideoProviderShown = true;
	fNativeToJavaBridge->ShowVideoPicker(source, maxTime, quality);
	return true;
}

/// To be called by the JNI bridge to indicate that the window has been closed.
/// Creates a display object for the selected Video (if selected) and then invokes the Lua listener.
/// @param selectedVideoFileName The path\file name of the Video that was selected.
///                              Set to NULL or empty string to indicate that the user canceled out of the window.
/// @param duration The duration of the video file in seconds.  A value of <0 will result in nil being pushed to lua
/// @param duration The size of the video in bytes.  A value of <0 will result in nil being pushed to lua
void
AndroidVideoProvider::CloseWithResult( const char *selectedVideoFileName, const int duration, const long size )
{
	// Do not continue if the Show() function was not called first.
	if (!fIsVideoProviderShown)
	{
		return;
	}
	
	// Flag that the window has been closed.
	fIsVideoProviderShown = false;
	
	// Invoke the Lua listener to indicate that the window was closed.
	VideoProviderResult result;
	result.SelectedVideoFileName = selectedVideoFileName;
	result.Duration = duration;
	result.Size = size;
	DidDismiss(AddProperties, (void*)&result);
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

