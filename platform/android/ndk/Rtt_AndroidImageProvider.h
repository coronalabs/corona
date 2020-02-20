//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidImageProvider_H__
#define _Rtt_AndroidImageProvider_H__

#include "librtt/Rtt_PlatformImageProvider.h"

class NativeToJavaBridge;

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

/// Provides an image picker window. Selected image can be sent to a Lua listener function.
class AndroidImageProvider : public PlatformImageProvider
{
	public:
		AndroidImageProvider( const ResourceHandle<lua_State> & handle, NativeToJavaBridge *ntjb );
		virtual bool Supports( int source ) const;
		virtual bool HasAccessTo( int source ) const;
		virtual bool Show( int source, const char* filePath, lua_State* L );
		virtual bool IsShown() { return fIsImageProviderShown; }
		virtual void CloseWithResult( const char *selectedImageFileName );
		void Abort() {
			fIsImageProviderShown = false; 
		}
	
	private:
		/// Stores information returned by the image picker window to be used by the static AddProperties function.
		struct AndroidImageProviderResult
		{
			AndroidImageProviderResult() : SelectedImageFileName(NULL), ShouldDisplaySelectedImage(true)
			{ }
			const char* SelectedImageFileName;
			bool ShouldDisplaySelectedImage;
		};
	
		static void AddProperties( lua_State *L, void* userdata );
	
		/// Set TRUE if image picker window is currently shown. Set FALSE if not.
		bool fIsImageProviderShown;
	
		/// Set TRUE to display the selected image in Corona.
		/// Set FALSE to not create a display object and save to file instead.
		bool fShouldDisplaySelectedImage;

		NativeToJavaBridge *fNativeToJavaBridge;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidImageProvider_H__
