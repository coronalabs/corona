//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidVideoPlayer_H__
#define _Rtt_AndroidVideoPlayer_H__

#include "librtt/Rtt_PlatformVideoPlayer.h"
#include "Core/Rtt_String.h"

class NativeToJavaBridge;

// ----------------------------------------------------------------------------

namespace Rtt
{

class Runtime;

// ----------------------------------------------------------------------------

class AndroidVideoPlayer : public PlatformVideoPlayer
{
	private:
		String fPath;
		bool fMediaControlsEnabled;
		NativeToJavaBridge *fNativeToJavaBridge;

	public:
		AndroidVideoPlayer( const Rtt::ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator, NativeToJavaBridge *ntjb ) 
			: PlatformVideoPlayer( handle ), fPath( & allocator ), fMediaControlsEnabled( true ), fNativeToJavaBridge(ntjb)
		{
		}
		
		virtual bool Load( const char* path, bool remote );

		virtual void NotificationCallback();

	public:
		virtual void Play();
		virtual void SetProperty( U32 mask, bool newValue );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidVideoPlayer_H__
