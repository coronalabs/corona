//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_AppleAudioPlayer_H__
#define _Rtt_AppleAudioPlayer_H__

#include "Rtt_PlatformAudioPlayer.h"

#include <AvailabilityMacros.h>

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)

#include <AudioToolbox/AudioToolbox.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#define kNumberAudioDataBuffers 2

class AppleAudioPlayer : public PlatformAudioPlayer
{
	public:
		typedef AppleAudioPlayer Self;

	protected:
		static void QueueOutputCallback(
									void* inUserData,
									AudioQueueRef inAudioQueue,
									AudioQueueBufferRef bufferReference );
	
	static int ReadAndPlayPackets( AppleAudioPlayer * player, AudioQueueBufferRef bufferReference );
	
	static void PropertyListenerCallback(
			void* inUserData,
			AudioQueueRef queueObject,
			AudioQueuePropertyID propertyID );

	public:
		AppleAudioPlayer( const ResourceHandle<lua_State> & handle );
		virtual ~AppleAudioPlayer();

	protected:
		void Cleanup();

	public:
		virtual bool Load( const char* filePath );

	public:
		virtual void Play();
		virtual void Stop();
		virtual void Pause();
		virtual void Resume();
		virtual void SetVolume( Rtt_Real volume );
		virtual Rtt_Real GetVolume() const;

	protected:
		void SetupAudioQueueBuffers();
		void CalculateSizes( Float64 seconds );

	protected:
		void Initialize();
		void OpenPlaybackFile();
		void SetupPlaybackAudioQueueObject();
		void EnableLevelMetering();
		void CopyMagicCookieToQueue();

	private:
		AudioFileID fAudioFileID;					// the identifier for the audio file to play
		AudioQueueRef fQueueObject;					// the audio queue object being used for playback
		CFURLRef fAudioFileURL;

		AudioStreamBasicDescription fAudioFormat;
		AudioQueueLevelMeterState* fAudioLevels;
		SInt64 fCurrentPacket;			// the current packet number in the playback file

		AudioQueueBufferRef fBuffers[kNumberAudioDataBuffers];	// the audio queue buffers for the audio queue

		UInt32 fBufferByteSize;						// the number of bytes to use in each audio queue buffer
		UInt32 fNumPacketsToRead;					// the number of audio data packets to read into each audio queue buffer

		AudioStreamPacketDescription* fPacketDescriptions;

		bool fIsPlaying;
		bool fDidStop;
	
//		bool fDonePlayingFile;
//		bool fAudioPlayerShouldStopImmediately;
};
	
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#else

namespace Rtt
{

class AppleAudioPlayer : public PlatformAudioPlayer
{
	public:
		virtual bool Load( const char* filePath );

	public:
		virtual void Play();
		virtual void Stop();
		virtual void Pause();
		virtual void Resume();
};

} // namespace Rtt

#endif // !defined( Rtt_MAC_ENV ) || (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)

#endif // _Rtt_AppleAudioPlayer_H__
