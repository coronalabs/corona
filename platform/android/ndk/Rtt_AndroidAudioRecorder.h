//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidAudioRecorder_H__
#define _Rtt_AndroidAudioRecorder_H__

#include "Rtt_PlatformAudioRecorder.h"

class NativeToJavaBridge;

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AndroidAudioRecorder : public PlatformAudioRecorder
{
	public:
		typedef AndroidAudioRecorder Self;

	protected:
		NativeToJavaBridge * fNativeToJavaBridge;

	public:
		AndroidAudioRecorder( const ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator, const char * file, NativeToJavaBridge *ntjb );
		virtual ~AndroidAudioRecorder();

	protected:
		void						Cleanup();

	public:
		virtual void				Start();
		virtual void				Stop();
		virtual void				SetRunningState( bool isRecording );
		virtual void				NotificationCallback( int status );
};
	
// ----------------------------------------------------------------------------

} // namespace Rtt

#endif // _Rtt_AndroidAudioRecorder_H__
