//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_AppleAudioRecorder_H__
#define _Rtt_AppleAudioRecorder_H__

#include "Rtt_PlatformAudioRecorder.h"

#include <AudioToolbox/AudioToolbox.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#define kNumberAudioRecordDataBuffers 3

class AppleAudioRecorder : public PlatformAudioRecorder
{
	public:
		typedef AppleAudioRecorder Self;

	protected:
		static void HandleInputBufferCallback(
			void *                              aqData,
			AudioQueueRef                       inAQ,
			AudioQueueBufferRef                 inBuffer,
			const AudioTimeStamp *              inStartTime,
			UInt32                              inNumPackets,
			const AudioStreamPacketDescription *inPacketDesc
		);

	public:
		AppleAudioRecorder( const ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator, const char * file );
		virtual ~AppleAudioRecorder();

	protected:
		void						Cleanup();

	public:
		virtual void				Start();
		virtual void				Stop();

	private:
		AudioStreamBasicDescription fDataFormat;
		AudioQueueRef               fQueue;
		AudioQueueBufferRef         fBuffers[kNumberAudioRecordDataBuffers];
		AudioFileID                 fAudioFile;
		UInt32                      fBufferByteSize;
		SInt64                      fCurrentPacket;
		UInt32						fSavedSessionCategory; /* only used on iOS */
		bool                        fCancelPendingRecording;
};
	
// ----------------------------------------------------------------------------

} // namespace Rtt

#endif // _Rtt_AppleAudioRecorder_H__
