//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidAudioPlayer_H__
#define _Rtt_AndroidAudioPlayer_H__

#include "librtt/Rtt_PlatformAudioPlayer.h"
#include "Core/Rtt_String.h"

class NativeToJavaBridge;

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// TODO: Rename AndroidAudioPlayer

class AndroidAudioPlayer : public PlatformAudioPlayer
{
	String			mySoundName;

	public:
		AndroidAudioPlayer( const ResourceHandle<lua_State> & handle, 
			Rtt_Allocator & allocator, NativeToJavaBridge *ntjb ) : 
			PlatformAudioPlayer( handle ),
			mySoundName( & allocator ),
			fNativeToJavaBridge(ntjb)
		{
		}
		virtual ~AndroidAudioPlayer();

	public:
		virtual bool Load( const char * filePath );

	public:
		virtual void Play();
		virtual void Stop();
		virtual void Pause();
		virtual void Resume();
		virtual void SetVolume( Rtt_Real volume );
		virtual Rtt_Real GetVolume() const;

	private:
		NativeToJavaBridge *fNativeToJavaBridge;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidAudioPlayer_H__
