//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_AppleAudioRecorder.h"
#include "Rtt_LuaContext.h"
#include "Rtt_PlatformAudioSessionManager.h"
#include "Rtt_Runtime.h"
#import <AVFoundation/AVFoundation.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

void
AppleAudioRecorder::HandleInputBufferCallback(
	void *                              aqData,
	AudioQueueRef                       inAQ,
	AudioQueueBufferRef                 inBuffer,
	const AudioTimeStamp *              inStartTime,
	UInt32                              inNumPackets,
	const AudioStreamPacketDescription *inPacketDesc
)
{
	AppleAudioRecorder * recorder = (AppleAudioRecorder *) aqData;

    if (inNumPackets == 0 && recorder->fDataFormat.mBytesPerPacket != 0)
	{
		inNumPackets = inBuffer->mAudioDataByteSize / recorder->fDataFormat.mBytesPerPacket;
	}

    // This corrects the getTunerVolume problem with iPhone and iPad (limited volume range)
    bool swapBytes;
#if( ( TARGET_OS_IPHONE == 1 ) || ( TARGET_IPHONE_SIMULATOR == 1 ) )
	// true to swap PCM bytes for RMS calculations
	swapBytes = true;
#else
	swapBytes = false;
#endif

	// Process buffer
	if( recorder->fAudioFile &&
		AudioFileWritePackets( recorder->fAudioFile,
								false,
								inBuffer->mAudioDataByteSize,
								inPacketDesc,
								recorder->fCurrentPacket,
								&inNumPackets,
								inBuffer->mAudioData ) == noErr )
	{
		recorder->fCurrentPacket += inNumPackets;
	}

	if (recorder->fIsRunning == 0) 
		return;

	AudioQueueEnqueueBuffer (
        recorder->fQueue,
        inBuffer,
        0,
        NULL
    );
}

static void 
DeriveBufferSize (
    AudioQueueRef                audioQueue,
    AudioStreamBasicDescription & ASBDescription,
    Float64                      seconds,
    UInt32 *                     outBufferSize
) {
    static const int maxBufferSize = 0x10000;
 
    int maxPacketSize = ASBDescription.mBytesPerPacket;
    if (maxPacketSize == 0) {
        UInt32 maxVBRPacketSize = sizeof(maxPacketSize);
        AudioQueueGetProperty (
			audioQueue,
			kAudioConverterPropertyMaximumOutputPacketSize,
			&maxPacketSize,
			&maxVBRPacketSize
        );
    }
 
    Float64 numBytesForTime =
        ASBDescription.mSampleRate * maxPacketSize * seconds;
    *outBufferSize =
		UInt32 (numBytesForTime < maxBufferSize ?
			numBytesForTime : maxBufferSize);
}

#if 0
static OSStatus 
SetMagicCookieForFile (
    AudioQueueRef inQueue,
    AudioFileID   inFile
) {
    OSStatus result = noErr;
    UInt32 cookieSize;
 
    if ( AudioQueueGetPropertySize (
		inQueue,
		kAudioQueueProperty_MagicCookie,
		&cookieSize ) == noErr ) 
	{
        char * magicCookie = (char *) malloc (cookieSize);

        if ( AudioQueueGetProperty (
			inQueue,
			kAudioQueueProperty_MagicCookie,
			magicCookie,
			&cookieSize ) == noErr )
		{
			result = AudioFileSetProperty (
				inFile,
				kAudioFilePropertyMagicCookieData,
				cookieSize,
				magicCookie );
		}
		
        free (magicCookie);
    }
    return result;
}
#endif // 0

AppleAudioRecorder::AppleAudioRecorder( const ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator, const char * file )
	: PlatformAudioRecorder( handle, allocator, file ),
	fQueue( NULL ), fAudioFile( 0 ), fBufferByteSize( 0 ), fCurrentPacket( 0 ), fCancelPendingRecording(false)
{
}

AppleAudioRecorder::~AppleAudioRecorder()
{
	Cleanup();
}

void
AppleAudioRecorder::Cleanup()
{
	AudioQueueDispose (
		fQueue,
		true );
	 
	if ( fAudioFile )
		AudioFileClose (fAudioFile);
}

void
AppleAudioRecorder::Start()
{
	if ( fIsRunning )
		return;
	fCancelPendingRecording = false;
#ifdef Rtt_IPHONE_ENV
	[[AVAudioSession sharedInstance] requestRecordPermission:^(BOOL granted) {
#else
	bool granted = true;
#endif

		if ( fIsRunning || !granted || fCancelPendingRecording )
			return;

		int numChannels = 1;

		if ( fFile.GetString() != NULL )
		{
			const char * path = fFile.GetString();
			size_t pathLen = strlen( path );
			AudioFileTypeID fileType = kAudioFileAIFFType;

			CFURLRef fileURL = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*)path, pathLen, false);


			//Aif is the only supported working type, any extension not listed below would create confusion as to what
			//actually worked, this is updated to support the documentation
			//The other types can only be supported with the proper format, bytes, frame setup, and encoder requirements

			/*
			if ( pathLen > 4 )
			{
				const char * ext = strrchr( path, '.' );

				if ( ext != NULL )
				{
					ext++;

					if ( Rtt_StringCompareNoCase( ext, "aif" ) == 0 || Rtt_StringCompareNoCase( ext, "aiff" ) == 0 )
						fileType = kAudioFileAIFFType;
					else if ( Rtt_StringCompareNoCase( ext, "mp3" ) == 0 || Rtt_StringCompareNoCase( ext, "mpg3" ) == 0 )
						fileType = kAudioFileMP3Type;
					else if ( Rtt_StringCompareNoCase( ext, "aac" ) == 0 )
						fileType = kAudioFileAAC_ADTSType;
				}
			}
			*/

			fDataFormat.mFormatID         = kAudioFormatLinearPCM;
			fDataFormat.mFramesPerPacket  = 1;
			fDataFormat.mSampleRate       = GetSampleRate();
			fDataFormat.mChannelsPerFrame = numChannels;
			fDataFormat.mBitsPerChannel   = 16;
			fDataFormat.mBytesPerFrame =
			  fDataFormat.mChannelsPerFrame * sizeof (SInt16);
			fDataFormat.mBytesPerPacket   = fDataFormat.mBytesPerFrame;
			fDataFormat.mReserved = 0;
			fDataFormat.mFormatFlags =
				kLinearPCMFormatFlagIsBigEndian
				| kLinearPCMFormatFlagIsSignedInteger
				| kLinearPCMFormatFlagIsPacked;

	/*
			// http://developer.apple.com/mac/library/documentation/MusicAudio/Reference/CoreAudioDataTypesRef/Reference/reference.html#//apple_ref/doc/c_ref/AudioStreamBasicDescription
			switch ( fileType )
			{
			case kAudioFileAIFFType:
				fDataFormat.mFormatID         = kAudioFormatLinearPCM;
				fDataFormat.mBytesPerPacket   = 1;
				fDataFormat.mFramesPerPacket  = 1;

				fDataFormat.mFormatFlags =
					kLinearPCMFormatFlagIsBigEndian
					| kLinearPCMFormatFlagIsSignedInteger
					| kLinearPCMFormatFlagIsPacked;
				break;
			case kAudioFileMP3Type:
				fDataFormat.mFormatID         = kAudioFormatMPEG4AAC;
				fDataFormat.mBytesPerPacket   = 256; // ???
				fDataFormat.mFramesPerPacket  = 1;

				fDataFormat.mFormatFlags = 0;
				break;
			case kAudioFileAAC_ADTSType:
				fDataFormat.mFormatID         = kAudioFormatMPEGLayer3;
				fDataFormat.mBytesPerPacket   = 256; // ???
				fDataFormat.mFramesPerPacket  = 1;

				fDataFormat.mFormatFlags = 0;
				break;
			}
	*/

			if ( fileURL )
			{
				OSStatus status = AudioFileCreateWithURL(fileURL,
					fileType,
					&fDataFormat,
					kAudioFileFlags_EraseFile,
					&fAudioFile);

				CFRelease( fileURL );
				if ( status != noErr )
					return;
			}
			else
			{
				return;
			}

		}
		else
		{
			// Used for Tuner API
			fDataFormat.mFormatID         = kAudioFormatLinearPCM;
			fDataFormat.mSampleRate       = GetSampleRate();
			fDataFormat.mChannelsPerFrame = numChannels;
			fDataFormat.mBitsPerChannel   = 16;
			fDataFormat.mBytesPerPacket   =
			   fDataFormat.mBytesPerFrame =
				  fDataFormat.mChannelsPerFrame * sizeof (SInt16);
			fDataFormat.mFramesPerPacket  = 1;

			fDataFormat.mFormatFlags =
				kLinearPCMFormatFlagIsBigEndian
				| kLinearPCMFormatFlagIsSignedInteger
				| kLinearPCMFormatFlagIsPacked;

		}

		/* This is iOS-only for AudioSessions.
		   We are trying to preserve basic automatic behavior for people who don't want to bother with audio sessions.
		   But for those that do want finer control, we want to try to stay out of their way.
		 */
	//	PlatformAudioSessionManager::Get()->RestoreAudioSessionCategoryIfNecessary();
		PlatformAudioSessionManager::SharedInstance()->SetImplicitRecordingModeIfNecessary();

		AudioQueueNewInput (
			&fDataFormat,
			HandleInputBufferCallback,
			this,
			NULL,
			kCFRunLoopCommonModes,
			0,
			&fQueue
		);

		UInt32 dataFormatSize = sizeof (fDataFormat);

		AudioQueueGetProperty(
			fQueue,
	#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1060
			kAudioQueueProperty_StreamDescription,
	#else
			kAudioConverterCurrentInputStreamDescription,
	#endif
			&fDataFormat,
			&dataFormatSize
		);

		DeriveBufferSize(
			fQueue,
			fDataFormat,
			0.5,
			&fBufferByteSize );

		for (int i = 0; i < kNumberAudioRecordDataBuffers; ++i) {
			AudioQueueAllocateBuffer(
				fQueue,
				fBufferByteSize,
				&fBuffers[i]
			);

			AudioQueueEnqueueBuffer (
				fQueue,
				fBuffers[i],
				0,
				NULL
			);
		}

		fCurrentPacket = 0;
		fIsRunning = true;

		AudioQueueStart(
			fQueue,
			NULL
		);
#ifdef Rtt_IPHONE_ENV
	}];
#endif
}

void
AppleAudioRecorder::Stop()
{
	if ( !fIsRunning ) {
		fCancelPendingRecording = true;
		return;
	}

	AudioQueueStop(	fQueue, true );
	
	if ( fAudioFile )
	{
		AudioFileClose( fAudioFile );
		fAudioFile = NULL;
	}
	
	/* This is iOS-only for AudioSessions. 
	   We are trying to preserve basic automatic behavior for people who don't want to bother with audio sessions.
	   But for those that do want finer control, we want to try to stay out of their way.
	 */
	PlatformAudioSessionManager::SharedInstance()->RestoreAudioSessionCategoryIfNecessary();
		
	fIsRunning = false;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

