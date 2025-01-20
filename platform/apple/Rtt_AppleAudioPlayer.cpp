//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_AppleAudioPlayer.h"
#include "Rtt_LuaContext.h"

#ifdef Rtt_IPHONE_ENV
#include "Rtt_IPhoneAudioSessionManager.h"
#endif
// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------


// TODO: Should we be using AVAudioPlayer?
// http://developer.apple.com/iphone/library/documentation/AVFoundation/Reference/AVAudioPlayerClassReference/Reference/Reference.html
	
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)

// ----------------------------------------------------------------------------
/*
static void
CleanupAudio( AQPlayerState& aqData )
{
	AudioQueueDispose (                            // 1
		aqData.mQueue,                             // 2
		true                                       // 3
	);
	 
	AudioFileClose (aqData.mAudioFile);            // 4
	 
	free (aqData.mPacketDescs); 
}
*/
// -------------------------------

int
AppleAudioPlayer::ReadAndPlayPackets( AppleAudioPlayer * player, AudioQueueBufferRef bufferReference )
{
	UInt32 numBytes;
	UInt32 numPackets = player->fNumPacketsToRead;

	// This callback is called when the playback audio queue object has an audio queue buffer
	// available for filling with more data from the file being played
#if defined(Rtt_TVOS_ENV)
	numBytes = player->fBufferByteSize;
	OSStatus status = AudioFileReadPacketData(
#else
	OSStatus status = AudioFileReadPackets(
#endif
										   player->fAudioFileID,
										   false,
										   &numBytes,
										   player->fPacketDescriptions,
										   player->fCurrentPacket,
										   &numPackets,
										   bufferReference->mAudioData
										   );
	Rtt_ASSERT( noErr == status );
	
	if (numPackets > 0)
	{
		bufferReference->mAudioDataByteSize = numBytes;		
		
		status = AudioQueueEnqueueBuffer(
										 player->fQueueObject,
										 bufferReference,
										 ( player->fPacketDescriptions ? numPackets : 0 ),
										 player->fPacketDescriptions
										 );
		Rtt_ASSERT( noErr == status );
		
		player->fCurrentPacket += numPackets;
		
		
	}
	
	return numPackets;
}
	
void
AppleAudioPlayer::QueueOutputCallback(
	void* inUserData,
	AudioQueueRef inAudioQueue,
	AudioQueueBufferRef bufferReference )
{
	// This callback, being outside the implementation block, needs a reference to the AudioPlayer object
	Self* player = (Self*)inUserData;
	if ( ! player->fIsPlaying ) return;

	UInt32 numPackets = ReadAndPlayPackets( player, bufferReference );
	
	if ( numPackets == 0 )
	{
		if ( player->GetLooping() )
		{
			player->fCurrentPacket = 0;

			// Retry at the beginning
			numPackets = ReadAndPlayPackets( player, bufferReference );
		}

		if ( numPackets == 0 )
		{
			player->fIsPlaying = false;
			AudioQueueStop( inAudioQueue, false );
		}
	}
}

// property callback function, invoked when a property changes. 
void
AppleAudioPlayer::PropertyListenerCallback(
	void* inUserData,
	AudioQueueRef queueObject,
	AudioQueuePropertyID propertyID )
{
	// This callback, being outside the implementation block, needs a reference to the AudioPlayer object
	Self * player = (Self*)inUserData;

	UInt32 propertySize;

	switch ( propertyID )
	{
		case kAudioQueueProperty_IsRunning:
			{
				UInt32 isRunning = 0;
				propertySize = sizeof( isRunning );

				OSStatus status = AudioQueueGetProperty(
					queueObject,
					propertyID,
					&isRunning,
					&propertySize
					);
				Rtt_UNUSED( status );

				if ( isRunning == 0 && !player->fDidStop ) 
				{
					player->NotificationCallback();
				}
			}
			break;
	}
}


// --------------

AppleAudioPlayer::AppleAudioPlayer( const ResourceHandle<lua_State> & handle )
	: PlatformAudioPlayer( handle ),
	fAudioFileID( NULL ),
	fQueueObject( NULL ),
	fAudioFileURL( NULL ),
	fPacketDescriptions( NULL ),
	fIsPlaying( false ),
	fAudioLevels( NULL ),
	fDidStop( false )
//	fDonePlayingFile( false ),
//	fAudioPlayerShouldStopImmediately( false )
{
}

AppleAudioPlayer::~AppleAudioPlayer()
{
	Cleanup();
}

void 
AppleAudioPlayer::SetVolume( Rtt_Real volume )
{
	Rtt_ASSERT( volume >= 0.0f && volume <= 1.0f );
	
	fVolume = volume;
	
	if ( fQueueObject )
	{
		AudioQueueParameterValue value = volume;
	 
		OSStatus status = AudioQueueSetParameter( fQueueObject, kAudioQueueParam_Volume, value );
		Rtt_UNUSED( status );
		Rtt_ASSERT( noErr == status );
	}
}
	
Rtt_Real 
AppleAudioPlayer::GetVolume() const
{
	if ( fQueueObject )
	{
		AudioQueueParameterValue value;

		OSStatus status = AudioQueueGetParameter( fQueueObject, kAudioQueueParam_Volume, &value );
		Rtt_UNUSED( status );
		Rtt_ASSERT( noErr == status );
		
		return value;
	}
	
	return fVolume;
}

bool
AppleAudioPlayer::Load( const char* filePath )
{
	if ( fAudioFileURL )
	{
		Cleanup();
	}

	if ( filePath )
	{
		fAudioFileURL = CFURLCreateFromFileSystemRepresentation( NULL, (const UInt8*)filePath, strlen(filePath), false );
	}
	else
	{
		fAudioFileURL = NULL;
	}

	return fAudioFileURL;
}

void
AppleAudioPlayer::Cleanup()
{
	Stop();

	if ( fAudioFileURL )
	{
		CFRelease( fAudioFileURL );
		fAudioFileURL = NULL;
	}
	
	if ( fAudioFileID ) 
	{
		AudioQueueDispose( fQueueObject, true );
		fQueueObject = NULL;

		delete [] fPacketDescriptions;
		fPacketDescriptions = NULL;

		AudioFileClose( fAudioFileID );
		fAudioFileID = NULL;
	}
}

void
AppleAudioPlayer::Play()
{
	if ( fAudioFileURL )
	{
		if ( ! fIsPlaying )
		{
			Initialize();

			if ( fAudioFileID )
			{
				SetupAudioQueueBuffers();
			}
		}

		Resume();
	}
}

void
AppleAudioPlayer::Stop()
{
	if ( fIsPlaying )
	{
		fIsPlaying = false;

	//	AudioQueueStop( fQueueObject, fAudioPlayerShouldStopImmediately );
		fDidStop = true;
		AudioQueueStop( fQueueObject, true );
/* Removed because this is going to interfere with the new OpenAL system
	#ifdef Rtt_IPHONE_ENV
		AudioSessionSetActive( false );
	#endif
 */
	}
}

void
AppleAudioPlayer::Pause()
{
	if ( fIsPlaying )
	{
		AudioQueuePause( fQueueObject );
		fDidStop = true;
	}
}

void
AppleAudioPlayer::Resume()
{
	if ( fIsPlaying )
	{
		#ifdef Rtt_IPHONE_ENV
			IPhoneAudioSessionManager::GetInstance()->SetAudioSessionActive(true);
		#endif
		AudioQueueStart( fQueueObject, NULL ); // 2nd arg is start time; NULL means ASAP.
		fDidStop = false;
		
		SetVolume( fVolume );
	}
}

void
AppleAudioPlayer::SetupAudioQueueBuffers()
{
	const Float64 kSecondsPerBuffer = 0.5;

	// calcluate the size to use for each audio queue buffer, and calculate the
	// number of packets to read into each buffer
	CalculateSizes( kSecondsPerBuffer );

	// prime the queue with some data before starting
	// allocate and enqueue buffers				
	for ( int i = 0; i < kNumberAudioDataBuffers && fIsPlaying; ++i )
	{
		AudioQueueAllocateBuffer( fQueueObject, fBufferByteSize, &fBuffers[i] );

		QueueOutputCallback( this, fQueueObject, fBuffers[i] );

		// if ( fDonePlayingFile ) break;
	}
}

void
AppleAudioPlayer::CalculateSizes( Float64 seconds )
{
	UInt32 maxPacketSize;
	UInt32 propertySize = sizeof (maxPacketSize);

	AudioFileGetProperty (
		fAudioFileID, 
		kAudioFilePropertyPacketSizeUpperBound,
		&propertySize,
		&maxPacketSize
	);

	const UInt32 kMaxBufferSize = 0x10000;	// limit maximum size to 64K
	const UInt32 kMinBufferSize = 0x4000;	// limit minimum size to 16K

	UInt32& rBufferByteSize = fBufferByteSize;

	const AudioStreamBasicDescription& audioFormat = fAudioFormat;
	if ( audioFormat.mFramesPerPacket)
	{
		Float64 numPacketsForTime = audioFormat.mSampleRate / audioFormat.mFramesPerPacket * seconds;
		rBufferByteSize = numPacketsForTime * maxPacketSize;
	}
	else
	{
		// if frames per packet is zero, then the codec doesn't know the relationship between 
		// packets and time -- so we return a default buffer size
		rBufferByteSize = kMaxBufferSize > maxPacketSize ? kMaxBufferSize : maxPacketSize;
	}
	
	// we're going to limit our size to our default
	if ( rBufferByteSize > kMaxBufferSize && rBufferByteSize > maxPacketSize)
	{
		rBufferByteSize = kMaxBufferSize;
	}
	else
	{
		// also make sure we're not too small - we don't want to go the disk for too small chunks
		if ( rBufferByteSize < kMinBufferSize )
		{
			rBufferByteSize = kMinBufferSize;
		}
	}

	fNumPacketsToRead = rBufferByteSize / maxPacketSize;

	// We have a couple of things to take care of now
	// (1) Setting up the conditions around VBR or a CBR format - affects how we will read from the file
		// if format is VBR we need to use a packet table.
	if ( audioFormat.mBytesPerPacket == 0 || audioFormat.mFramesPerPacket == 0 )
	{
		// we also need packet descpriptions for the file reading
		fPacketDescriptions = new AudioStreamPacketDescription[fNumPacketsToRead];
		
	}
}

void
AppleAudioPlayer::Initialize()
{
	if ( ! fAudioFileID )
	{
		OpenPlaybackFile();
//		fDonePlayingFile = false;
//		fAudioPlayerShouldStopImmediately = false;
	}	

	// Only continue if fAudioFileID is not NULL
	if ( fAudioFileID )
	{
		fCurrentPacket = 0;
		SetupPlaybackAudioQueueObject();
		fIsPlaying = true;
		fDidStop = false;
	}
}

void
AppleAudioPlayer::OpenPlaybackFile()
{
	Rtt_ASSERT( NULL == fAudioFileID );

	OSStatus status = AudioFileOpenURL(
		(CFURLRef)fAudioFileURL,
		kAudioFileReadPermission, // read only (fsRdPerm)
		0,
		&fAudioFileID
	);

	if ( noErr == status )
	{
		UInt32 sizeOfPlaybackFormatASBDStruct = sizeof( fAudioFormat );
		
		// get the AudioStreamBasicDescription format for the playback file
		status = AudioFileGetProperty(
			fAudioFileID,
			kAudioFilePropertyDataFormat,
			&sizeOfPlaybackFormatASBDStruct,
			&fAudioFormat
		);

		if ( ! Rtt_VERIFY( noErr == status ) )
		{
			Cleanup();
		}
	}
}

void
AppleAudioPlayer::SetupPlaybackAudioQueueObject()
{
	if ( fQueueObject )
	{
		AudioQueueDispose( fQueueObject, true );
	}

	// create the playback audio queue object
	OSStatus status = AudioQueueNewOutput(
		&fAudioFormat,
		QueueOutputCallback,
		this, 
		CFRunLoopGetCurrent(),
		kCFRunLoopCommonModes,
		0,								// run loop flags
		&fQueueObject
	);
	Rtt_ASSERT( noErr == status ); Rtt_UNUSED(status);

	// add the property listener callback to the playback audio queue
	AudioQueueAddPropertyListener(
		fQueueObject,
		kAudioQueueProperty_IsRunning,
		PropertyListenerCallback,
		this
	);

	// copy the audio file's magic cookie to the audio queue object to give it 
	// as much info as possible about the audio data to play
	CopyMagicCookieToQueue();
}

// an audio queue object doesn't provide audio level information unless you 
// enable it to do so
void
AppleAudioPlayer::EnableLevelMetering()
{
	// allocate the memory needed to store audio level information
	fAudioLevels = (AudioQueueLevelMeterState*)calloc( sizeof(AudioQueueLevelMeterState), fAudioFormat.mChannelsPerFrame );

	UInt32 trueValue = true;

	OSStatus status = AudioQueueSetProperty(
		fQueueObject,
		kAudioQueueProperty_EnableLevelMetering,
		&trueValue,
		sizeof( UInt32 ) );
	Rtt_ASSERT( noErr == status ); Rtt_UNUSED(status);
}

// magic cookies are not used by linear PCM audio. this method is included here
// so this app still works if you change the recording format to one that uses
// magic cookies.
void
AppleAudioPlayer::CopyMagicCookieToQueue()
{
	UInt32 propertySize = sizeof( UInt32 );

	OSStatus result = AudioFileGetPropertyInfo(
							fAudioFileID,
							kAudioFilePropertyMagicCookieData,
							&propertySize,
							NULL );

	if ( !result && propertySize )
	{
		char* cookie = (char*)malloc( propertySize );

		result = AudioFileGetProperty(
			fAudioFileID,
			kAudioFilePropertyMagicCookieData,
			&propertySize,
			cookie );
		Rtt_ASSERT( noErr == result );

		result = AudioQueueSetProperty(
			fQueueObject,
			kAudioQueueProperty_MagicCookie,
			cookie,
			propertySize );
		Rtt_ASSERT( noErr == result );

		free( cookie );
	}
}

// ----------------------------------------------------------------------------

#else

bool
AppleAudioPlayer::Load( const char* filePath )
{
	return false;
}

void
AppleAudioPlayer::Play()
{
}

void
AppleAudioPlayer::Stop()
{
}

void
AppleAudioPlayer::Pause()
{
}

void
AppleAudioPlayer::Resume()
{
}

#endif // !defined( Rtt_MAC_ENV ) || (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

