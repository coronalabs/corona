//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformAudioRecorder_H__
#define _Rtt_PlatformAudioRecorder_H__

#include "Core/Rtt_ResourceHandle.h"
#include "Core/Rtt_String.h"
#include "Rtt_Scheduler.h"
#include "Rtt_PlatformNotifier.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

// ----------------------------------------------------------------------------

class PlatformAudioRecorder : public PlatformNotifier
{
	public:
		PlatformAudioRecorder( const ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator, const char * file );
		virtual ~PlatformAudioRecorder();

	public:
		virtual void Start() = 0;
		virtual void Stop() = 0;

		virtual void NotificationCallback( int status );
		
		bool IsRecording() const
		{
			return fIsRunning;
		}
		
		void SetSampleRate( U32 sampleRate )
		{
				fSampleRate = sampleRate;
		}

		U32 GetSampleRate() const
		{
			return fSampleRate;
		}
	
	protected:
		friend class PlatformAudioRecorderCallListenerTask;

		String						fFile;
		Rtt_Allocator &				fAllocator;
		bool                        fIsRunning;
		U32                         fSampleRate; // Hz

};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformAudioRecorder_H__
