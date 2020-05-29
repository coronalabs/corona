//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidEventSound_H__
#define _Rtt_AndroidEventSound_H__

#include "Rtt_PlatformEventSound.h"
#include "Core/Rtt_String.h"

class NativeToJavaBridge;

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AndroidEventSound : public PlatformEventSound
{
	public:
		AndroidEventSound( const ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator, NativeToJavaBridge *ntjb );
		virtual ~AndroidEventSound();
		
	public:
		
	public:
		virtual bool Load( const char* filePath );
		
	public:
		virtual void Play();
		virtual void Stop();
		virtual void Pause();
		virtual void Resume();
		virtual void SetVolume( Rtt_Real volume );
		virtual Rtt_Real GetVolume() const;
		
		virtual void ReleaseOnComplete();
		
	private:
		String			mySoundName;
		NativeToJavaBridge *fNativeToJavaBridge;
};

// ----------------------------------------------------------------------------

}  // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidEventSound_H__
