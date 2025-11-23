#ifdef ALMIXER_COMPILE_WITHOUT_SDL

/*
 * SDL_sound Core Audio backend
 * Copyright (C) 2010 Eric Wing <ewing . public @ playcontrol.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef __APPLE__

#include <stddef.h> /* NULL */
#include <stdio.h> /* printf */
#include <al.h>
#include <arpa/inet.h> /* htonl */
#include <AudioToolbox/AudioToolbox.h>

#include "SoundDecoder.h"

#include "SoundDecoder_Internal.h"

typedef struct CoreAudioFileContainer
{
	AudioFileID* audioFileID;
	ExtAudioFileRef extAudioFileRef;
	AudioStreamBasicDescription* outputFormat;
} CoreAudioFileContainer;

static int CoreAudio_init(void);
static void CoreAudio_quit(void);
static int CoreAudio_open(Sound_Sample *sample, const char *ext);
static void CoreAudio_close(Sound_Sample *sample);
static size_t CoreAudio_read(Sound_Sample *sample);
static int CoreAudio_rewind(Sound_Sample *sample);
static int CoreAudio_seek(Sound_Sample *sample, size_t ms);

static const char *extensions_coreaudio[] = 
{
	"aif",
	"aiff",
	"aifc",
	"wav",
	"wave",
	"mp3",
	"mp4",
	"m4a",
	"aac",
	"adts",
	"caf",
	"Sd2f",
	"Sd2",
	"au",
	"snd",
	"next",
	"mp2",
	"mp1",
	"ac3",
	"3gpp",
	"3gp",
	"3gp2",
	"3g2",
	"amrf",
	"amr",
	"ima4",
	"ima",
	NULL 
};
const Sound_DecoderFunctions __Sound_DecoderFunctions_CoreAudio =
{
    {
        extensions_coreaudio,
        "Decode audio through Core Audio through",
        "Eric Wing <ewing . public @ playcontrol.net>",
        "http://playcontrol.net"
    },
	
    CoreAudio_init,       /*   init() method */
    CoreAudio_quit,       /*   quit() method */
    CoreAudio_open,       /*   open() method */
    CoreAudio_close,      /*  close() method */
    CoreAudio_read,       /*   read() method */
    CoreAudio_rewind,     /* rewind() method */
    CoreAudio_seek        /*   seek() method */
};


static int CoreAudio_init(void)
{
    return(1);  /* always succeeds. */
} /* CoreAudio_init */


static void CoreAudio_quit(void)
{
    /* it's a no-op. */
} /* CoreAudio_quit */

/*
   http://developer.apple.com/library/ios/#documentation/MusicAudio/Reference/AudioFileConvertRef/Reference/reference.html
   kAudioFileAIFFType = 'AIFF',
   kAudioFileAIFCType            = 'AIFC',
   kAudioFileWAVEType            = 'WAVE',
   kAudioFileSoundDesigner2Type  = 'Sd2f',
   kAudioFileNextType            = 'NeXT',
   kAudioFileMP3Type             = 'MPG3',
   kAudioFileMP2Type             = 'MPG2',
   kAudioFileMP1Type             = 'MPG1',
   kAudioFileAC3Type             = 'ac-3',
   kAudioFileAAC_ADTSType        = 'adts',
   kAudioFileMPEG4Type           = 'mp4f',
   kAudioFileM4AType             = 'm4af',
   kAudioFileCAFType             = 'caff',
   kAudioFile3GPType             = '3gpp',
   kAudioFile3GP2Type            = '3gp2',
   kAudioFileAMRType             = 'amrf'
*/
static AudioFileTypeID CoreAudio_GetAudioTypeForExtension(const char* file_extension)
{
	if( (__Sound_strcasecmp(file_extension, "aif") == 0)
		|| (__Sound_strcasecmp(file_extension, "aiff") == 0)
		|| (__Sound_strcasecmp(file_extension, "aifc") == 0)
	)
	{
		return kAudioFileAIFCType;
	}
	else if( (__Sound_strcasecmp(file_extension, "wav") == 0)
		|| (__Sound_strcasecmp(file_extension, "wave") == 0)
	)
	{
		return kAudioFileWAVEType;
	}
	else if( (__Sound_strcasecmp(file_extension, "mp3") == 0)
	)
	{
		return kAudioFileMP3Type;
	}
	else if( (__Sound_strcasecmp(file_extension, "mp4") == 0)
	)
	{
		return kAudioFileMPEG4Type;
	}
	else if( (__Sound_strcasecmp(file_extension, "m4a") == 0)
	)
	{
		return kAudioFileM4AType;
	}
	else if( (__Sound_strcasecmp(file_extension, "aac") == 0)
	)
	{
		return kAudioFileAAC_ADTSType;
	}
	else if( (__Sound_strcasecmp(file_extension, "adts") == 0)
	)
	{
		return kAudioFileAAC_ADTSType;
	}
	else if( (__Sound_strcasecmp(file_extension, "caf") == 0)
		|| (__Sound_strcasecmp(file_extension, "caff") == 0)
	)
	{
		return kAudioFileCAFType;
	}
	else if( (__Sound_strcasecmp(file_extension, "Sd2f") == 0)
		|| (__Sound_strcasecmp(file_extension, "sd2") == 0)
	)
	{
		return kAudioFileSoundDesigner2Type;
	}
	else if( (__Sound_strcasecmp(file_extension, "au") == 0)
		|| (__Sound_strcasecmp(file_extension, "snd") == 0)
		|| (__Sound_strcasecmp(file_extension, "next") == 0)
	)
	{
		return kAudioFileNextType;
	}
	else if( (__Sound_strcasecmp(file_extension, "mp2") == 0)
	)
	{
		return kAudioFileMP2Type;
	}
	else if( (__Sound_strcasecmp(file_extension, "mp1") == 0)
	)
	{
		return kAudioFileMP1Type;
	}
	else if( (__Sound_strcasecmp(file_extension, "ac3") == 0)
	)
	{
		return kAudioFileAC3Type;
	}
	else if( (__Sound_strcasecmp(file_extension, "3gpp") == 0)
			|| (__Sound_strcasecmp(file_extension, "3gp") == 0)
	)
	{
		return kAudioFile3GPType;
	}
	else if( (__Sound_strcasecmp(file_extension, "3gp2") == 0)
			|| (__Sound_strcasecmp(file_extension, "3g2") == 0)
	)
	{
		return kAudioFile3GP2Type;
	}
	else if( (__Sound_strcasecmp(file_extension, "amrf") == 0)
		|| (__Sound_strcasecmp(file_extension, "amr") == 0)
	)
	{
		return kAudioFileAMRType;
	}
	else if( (__Sound_strcasecmp(file_extension, "ima4") == 0)
		|| (__Sound_strcasecmp(file_extension, "ima") == 0)
	)
	{
		/* not sure about this one */
		return kAudioFileCAFType;
	}
	else
	{
		return 0;
	}

}

static const char* CoreAudio_FourCCToString(int32_t error_code)
{
	static char return_string[16];
	uint32_t big_endian_code = htonl(error_code);
	char* big_endian_str = (char*)&big_endian_code;
	// see if it appears to be a 4-char-code
	if(isprint(big_endian_str[0])
	   && isprint(big_endian_str[1])
	   && isprint(big_endian_str[2])
	   && isprint (big_endian_str[3]))
	{
		return_string[0] = '\'';
		return_string[1] = big_endian_str[0];
		return_string[2] = big_endian_str[1];
		return_string[3] = big_endian_str[2];
		return_string[4] = big_endian_str[3];
		return_string[5] = '\'';
		return_string[6] = '\0';
	}
	else if(error_code > -200000 && error_code < 200000)
	{
		// no, format it as an integer
		snprintf(return_string, 16, "%d", error_code);
	}
	else
	{
		// no, format it as an integer but in hex
		snprintf(return_string, 16, "0x%x", error_code);
	}
	return return_string;
}



SInt64 CoreAudio_SizeCallback(void* inClientData)
{
	ALmixer_RWops* rw_ops = (ALmixer_RWops*)inClientData;
	SInt64 current_position = ALmixer_RWtell(rw_ops);
	SInt64 end_position = ALmixer_RWseek(rw_ops, 0, SEEK_END);
	ALmixer_RWseek(rw_ops, current_position, SEEK_SET);
//	fprintf(stderr, "CoreAudio_SizeCallback:%d\n", end_position);

	return end_position;
}

OSStatus CoreAudio_ReadCallback(
	void* inClientData,
	SInt64 inPosition,
	UInt32 requestCount,
	void* data_buffer,
	UInt32* actualCount
)
{
	ALmixer_RWops* rw_ops = (ALmixer_RWops*)inClientData;
	ALmixer_RWseek(rw_ops, inPosition, SEEK_SET);
	size_t bytes_actually_read = ALmixer_RWread(rw_ops, data_buffer, 1, requestCount);
	// Not sure how to test for a read error with ALmixer_RWops
//	fprintf(stderr, "CoreAudio_ReadCallback:%d, %d\n", requestCount, bytes_actually_read);

	*actualCount = (UInt32) bytes_actually_read;
	return noErr;
}


static int CoreAudio_open(Sound_Sample *sample, const char *ext)
{
	CoreAudioFileContainer* core_audio_file_container;
	AudioFileID* audio_file_id;
	OSStatus error_result;
	Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
	AudioStreamBasicDescription	actual_format;
	AudioStreamBasicDescription	output_format;
	Float64 estimated_duration;
	UInt32 format_size;
	

	core_audio_file_container = (CoreAudioFileContainer*)malloc(sizeof(CoreAudioFileContainer));
	BAIL_IF_MACRO(core_audio_file_container == NULL, ERR_OUT_OF_MEMORY, 0);


	audio_file_id = (AudioFileID*)malloc(sizeof(AudioFileID));
	BAIL_IF_MACRO(audio_file_id == NULL, ERR_OUT_OF_MEMORY, 0);

	error_result = AudioFileOpenWithCallbacks(
		internal->rw,
		CoreAudio_ReadCallback,
		NULL,
		CoreAudio_SizeCallback,
		NULL,
		CoreAudio_GetAudioTypeForExtension(ext),
		audio_file_id
	);
	if (error_result != noErr)
	{
		AudioFileClose(*audio_file_id);
		free(audio_file_id);
		free(core_audio_file_container);
		SNDDBG(("Core Audio: can't grok data. reason: [%s].\n", CoreAudio_FourCCToString(error_result)));
		BAIL_MACRO("Core Audio: Not valid audio data.", 0);
	} /* if */
	
    format_size = sizeof(actual_format);
    error_result = AudioFileGetProperty(
		*audio_file_id,
		kAudioFilePropertyDataFormat,
		&format_size,
		&actual_format
	);
    if (error_result != noErr)
	{
		AudioFileClose(*audio_file_id);
		free(audio_file_id);
		free(core_audio_file_container);
		SNDDBG(("Core Audio: AudioFileGetProperty failed. reason: [%s]", CoreAudio_FourCCToString(error_result)));
		BAIL_MACRO("Core Audio: Not valid audio data.", 0);
	} /* if */

    format_size = sizeof(estimated_duration);
    error_result = AudioFileGetProperty(
		*audio_file_id,
		kAudioFilePropertyEstimatedDuration,
		&format_size,
		&estimated_duration
	);
    if (error_result != noErr)
	{
		AudioFileClose(*audio_file_id);
		free(audio_file_id);
		free(core_audio_file_container);
		SNDDBG(("Core Audio: AudioFileGetProperty failed. reason: [%s].\n", CoreAudio_FourCCToString(error_result)));
		BAIL_MACRO("Core Audio: Not valid audio data.", 0);
	} /* if */


	core_audio_file_container->audioFileID = audio_file_id;
	
	internal->decoder_private = core_audio_file_container;
	
	/* If the audio file contains more audio channels than OpenAL supports, then reduce the number of channels
	 * to be decoded by CoreAudio to something that is supported. CoreAudio will downmix it for us in this case.
	 * Note: OpenAL always supports mono (1 channel) and stereo (2 channel) sound.
	 *       This is only an issue for audio files encoded for surround sound, such as 5.1 audio which is 6 channels.
	 */
	for (; actual_format.mChannelsPerFrame > 2; actual_format.mChannelsPerFrame--)
	{
		const char* alFormatName;
		switch (actual_format.mChannelsPerFrame)
		{
			case 8:
				alFormatName = (actual_format.mBitsPerChannel >= 16) ? "AL_FORMAT_71CHN16" : "AL_FORMAT_71CHN8";
				break;
			case 7:
				alFormatName = (actual_format.mBitsPerChannel >= 16) ? "AL_FORMAT_61CHN16" : "AL_FORMAT_61CHN8";
				break;
			case 6:
				alFormatName = (actual_format.mBitsPerChannel >= 16) ? "AL_FORMAT_51CHN16" : "AL_FORMAT_51CHN8";
				break;
			case 4:
				alFormatName = (actual_format.mBitsPerChannel >= 16) ? "AL_FORMAT_QUAD16" : "AL_FORMAT_QUAD8";
				break;
			default:
				alFormatName = NULL;
				break;
		}
		if (alFormatName)
		{
			ALenum alFormatType = alGetEnumValue(alFormatName);
			if ((alFormatType != 0) && (alFormatType != (ALenum)-1))
			{
				break;
			}
		}
	}

    /* Fix for Apple Silicon macOS 26 (Tahoe) regression where mono audio playback fails.
     * Apple's built-in OpenAL implementation on macOS 26+ has a bug where mono audio buffers
     */
#if TARGET_OS_OSX && (defined(__arm64__) || defined(__aarch64__))
    if (actual_format.mChannelsPerFrame == 1){
        actual_format.mChannelsPerFrame = 2;
    }
#endif
    
	sample->flags = SOUND_SAMPLEFLAG_CANSEEK;
	sample->actual.rate = (UInt32) actual_format.mSampleRate;
	sample->actual.channels = (UInt8)actual_format.mChannelsPerFrame;
	internal->total_time = (SInt32)(estimated_duration * 1000.0 + 0.5);

#if 0
	/* FIXME: Both Core Audio and SDL 1.3 support float and 32-bit formats */
	if(actual_format.mFormatFlags & kAudioFormatFlagIsBigEndian)
	{
		if(16 == actual_format.mBitsPerChannel)
		{
			if(kAudioFormatFlagIsSignedInteger & actual_format.mFormatFlags)
			{
				sample->actual.format = AUDIO_S16MSB;
			}
			else
			{
				sample->actual.format = AUDIO_U16MSB;				
			}
		}
		else if(8 == actual_format.mBitsPerChannel)
		{
			if(kAudioFormatFlagIsSignedInteger & actual_format.mFormatFlags)
			{
				sample->actual.format = AUDIO_S8;
			}
			else
			{
				sample->actual.format = AUDIO_U8;				
			}
		}
		else // might be 0 for undefined? 
		{
			// This case seems to come up a lot for me. Maybe for file types like .m4a?
			sample->actual.format = AUDIO_S16SYS;
			SNDDBG(("Core Audio: Unsupported actual_format.mBitsPerChannel: [%d].\n", actual_format.mBitsPerChannel));
			
		}
	}
	else // little endian
	{
		if(16 == actual_format.mBitsPerChannel)
		{
			if(kAudioFormatFlagIsSignedInteger & actual_format.mFormatFlags)
			{
				sample->actual.format = AUDIO_S16LSB;
			}
			else
			{
				sample->actual.format = AUDIO_U16LSB;				
			}
		}
		else if(8 == actual_format.mBitsPerChannel)
		{
			if(kAudioFormatFlagIsSignedInteger & actual_format.mFormatFlags)
			{
				sample->actual.format = AUDIO_S8;
			}
			else
			{
				sample->actual.format = AUDIO_U8;				
			}
		}
		else // might be 0 for undefined? 
		{
			sample->actual.format = AUDIO_S16SYS;
			
			SNDDBG(("Core Audio: Unsupported actual_format.mBitsPerChannel: [%d].\n", actual_format.mBitsPerChannel));
		}

	}
#else
	
	
	
    /*
     * I want to use Core Audio to do conversion and decoding for performance reasons.
	 * This is particularly important on mobile devices like iOS.
	 * Taking from the Ogg Vorbis decode, I pretend the "actual" format is the same 
	 * as the desired format. 
     */
    sample->actual.format = (sample->desired.format == 0) ?
	AUDIO_S16SYS : sample->desired.format;
#endif	


	SNDDBG(("CoreAudio: channels == (%d).\n", sample->actual.channels));
	SNDDBG(("CoreAudio: sampling rate == (%d).\n",sample->actual.rate));
	SNDDBG(("CoreAudio: total seconds of sample == (%d).\n", internal->total_time));
	SNDDBG(("CoreAudio: sample->actual.format == (%d).\n", sample->actual.format));


	
	error_result = ExtAudioFileWrapAudioFileID(*audio_file_id,
		false, // set to false for read-only
		&core_audio_file_container->extAudioFileRef
	);
	if(error_result != noErr)
	{
		AudioFileClose(*audio_file_id);
		free(audio_file_id);
		free(core_audio_file_container);
		SNDDBG(("Core Audio: can't wrap data. reason: [%s].\n", CoreAudio_FourCCToString(error_result)));
		BAIL_MACRO("Core Audio: Failed to wrap data.", 0);
	} /* if */


	/* The output format must be linear PCM because that's the only type OpenAL knows how to deal with.
	 * Set the client format to 16 bit signed integer (native-endian) data because that is the most
	 * optimal format on iPhone/iPod Touch hardware.
	 * Maintain the channel count and sample rate of the original source format.
	 */
	output_format.mSampleRate = actual_format.mSampleRate; // preserve the original sample rate
	output_format.mChannelsPerFrame = actual_format.mChannelsPerFrame; // preserve the number of channels
	output_format.mFormatID = kAudioFormatLinearPCM; // We want linear PCM data
	output_format.mFramesPerPacket = 1; // We know for linear PCM, the definition is 1 frame per packet

	if(sample->desired.format == 0)
	{
		// do AUDIO_S16SYS
		output_format.mFormatFlags = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked; // I seem to read failures problems without kAudioFormatFlagIsPacked. From a mailing list post, this seems to be a Core Audio bug.
		output_format.mBitsPerChannel = 16; // We know we want 16-bit
	}
	else
	{
		output_format.mFormatFlags = 0; // clear flags
		output_format.mFormatFlags |= kAudioFormatFlagIsPacked; // I seem to read failures problems without kAudioFormatFlagIsPacked. From a mailing list post, this seems to be a Core Audio bug.
		// Mask against bitsize
		if(0xFF & sample->desired.format)
		{
			output_format.mBitsPerChannel = 16; /* 16-bit */
		}
		else
		{
			output_format.mBitsPerChannel = 8; /* 8-bit */
		}

		// Mask for signed/unsigned
		if((1<<15) & sample->desired.format)
		{
			output_format.mFormatFlags = output_format.mFormatFlags | kAudioFormatFlagIsSignedInteger;

		}
		else
		{
			// no flag set for unsigned
		}
		// Mask for big/little endian
		if((1<<12) & sample->desired.format)
		{
			output_format.mFormatFlags = output_format.mFormatFlags | kAudioFormatFlagIsBigEndian;
		}
		else
		{
			// no flag set for little endian 
		}
	}

	output_format.mBytesPerPacket = output_format.mBitsPerChannel/8 * output_format.mChannelsPerFrame; // e.g. 16-bits/8 * channels => so 2-bytes per channel per frame
	output_format.mBytesPerFrame = output_format.mBitsPerChannel/8 * output_format.mChannelsPerFrame; // For PCM, since 1 frame is 1 packet, it is the same as mBytesPerPacket

	
/*
	output_format.mSampleRate = actual_format.mSampleRate; // preserve the original sample rate
	output_format.mChannelsPerFrame = actual_format.mChannelsPerFrame; // preserve the number of channels
	output_format.mFormatID = kAudioFormatLinearPCM; // We want linear PCM data
//	output_format.mFormatFlags = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;
	output_format.mFormatFlags = kAudioFormatFlagsNativeEndian |  kAudioFormatFlagIsSignedInteger;
	output_format.mFramesPerPacket = 1; // We know for linear PCM, the definition is 1 frame per packet
	output_format.mBitsPerChannel = 16; // We know we want 16-bit
	output_format.mBytesPerPacket = 2 * output_format.mChannelsPerFrame; // We know we are using 16-bit, so 2-bytes per channel per frame
	output_format.mBytesPerFrame = 2 * output_format.mChannelsPerFrame; // For PCM, since 1 frame is 1 packet, it is the same as mBytesPerPacket
*/
	SNDDBG(("output_format: mSampleRate: %lf\n", output_format.mSampleRate)); 
	SNDDBG(("output_format: mChannelsPerFrame: %d\n", output_format.mChannelsPerFrame)); 
	SNDDBG(("output_format: mFormatID: %d\n", output_format.mFormatID)); 
	SNDDBG(("output_format: mFormatFlags: %d\n", output_format.mFormatFlags)); 
	SNDDBG(("output_format: mFramesPerPacket: %d\n", output_format.mFramesPerPacket)); 
	SNDDBG(("output_format: mBitsPerChannel: %d\n", output_format.mBitsPerChannel)); 
	SNDDBG(("output_format: mBytesPerPacket: %d\n", output_format.mBytesPerPacket)); 
	SNDDBG(("output_format: mBytesPerFrame: %d\n", output_format.mBytesPerFrame)); 
	
	
	/* Set the desired client (output) data format */
	error_result = ExtAudioFileSetProperty(core_audio_file_container->extAudioFileRef, kExtAudioFileProperty_ClientDataFormat, sizeof(output_format), &output_format);
	if(noErr != error_result)
	{
		ExtAudioFileDispose(core_audio_file_container->extAudioFileRef);
		AudioFileClose(*audio_file_id);
		free(audio_file_id);
		free(core_audio_file_container);
		SNDDBG(("Core Audio: ExtAudioFileSetProperty(kExtAudioFileProperty_ClientDataFormat) failed, reason: [%s].\n", CoreAudio_FourCCToString(error_result)));
		BAIL_MACRO("Core Audio: Not valid audio data.", 0);
	}	


	core_audio_file_container->outputFormat = (AudioStreamBasicDescription*)malloc(sizeof(AudioStreamBasicDescription));
	BAIL_IF_MACRO(core_audio_file_container->outputFormat == NULL, ERR_OUT_OF_MEMORY, 0);


	
	/* Copy the output format to the audio_description that was passed in so the 
	 * info will be returned to the user.
	 */
	memcpy(core_audio_file_container->outputFormat, &output_format, sizeof(AudioStreamBasicDescription));

	

	return(1);
} /* CoreAudio_open */


static void CoreAudio_close(Sound_Sample *sample)
{
	Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
	CoreAudioFileContainer* core_audio_file_container = (CoreAudioFileContainer *) internal->decoder_private;
	
	free(core_audio_file_container->outputFormat);
	ExtAudioFileDispose(core_audio_file_container->extAudioFileRef);
	AudioFileClose(*core_audio_file_container->audioFileID);
	free(core_audio_file_container->audioFileID);
	free(core_audio_file_container);
		
} /* CoreAudio_close */


static size_t CoreAudio_read(Sound_Sample *sample)
{
	OSStatus error_result = noErr;	
	/* Documentation/example shows SInt64, but is problematic for big endian
	 * on 32-bit cast for ExtAudioFileRead() because it takes the upper
	 * bits which turn to 0.
	 */
	UInt32 buffer_size_in_frames = 0;
	UInt32 buffer_size_in_frames_remaining = 0;
	UInt32 total_frames_read = 0;
	UInt32 data_buffer_size = 0;
	UInt32 bytes_remaining = 0;
	size_t total_bytes_read = 0;
	Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
	CoreAudioFileContainer* core_audio_file_container = (CoreAudioFileContainer *) internal->decoder_private;
	UInt32 max_buffer_size = (UInt32) internal->buffer_size;

//	printf("internal->buffer_size=%d, internal->buffer=0x%x, sample->buffer_size=%d\n", internal->buffer_size, internal->buffer, sample->buffer_size); 
//	printf("internal->max_buffer_size=%d\n", max_buffer_size); 

	/* Compute how many frames will fit into our max buffer size */
	/* Warning: If this is not evenly divisible, the buffer will not be completely filled which violates the SDL_sound assumption. */
	buffer_size_in_frames = max_buffer_size / core_audio_file_container->outputFormat->mBytesPerFrame;
//	printf("buffer_size_in_frames=%ld, internal->buffer_size=%d, internal->buffer=0x%x outputFormat->mBytesPerFrame=%d, sample->buffer_size=%d\n", buffer_size_in_frames, internal->buffer_size, internal->buffer, core_audio_file_container->outputFormat->mBytesPerFrame, sample->buffer_size); 


//	void* temp_buffer = malloc(max_buffer_size);
	
	AudioBufferList audio_buffer_list;
	audio_buffer_list.mNumberBuffers = 1;
	audio_buffer_list.mBuffers[0].mDataByteSize = max_buffer_size;
	audio_buffer_list.mBuffers[0].mNumberChannels = core_audio_file_container->outputFormat->mChannelsPerFrame;
	audio_buffer_list.mBuffers[0].mData = internal->buffer;


	bytes_remaining = max_buffer_size;
	buffer_size_in_frames_remaining = buffer_size_in_frames;
	
	// oops. Due to the kAudioFormatFlagIsPacked bug, 
	// I was misled to believe that Core Audio
	// was not always filling my entire requested buffer. 
	// So this while-loop might be unnecessary.
	// However, I have not exhaustively tested all formats, 
	// so maybe it is possible this loop is useful.
	// It might also handle the not-evenly disvisible case above.
	while(buffer_size_in_frames_remaining > 0 && !(sample->flags & SOUND_SAMPLEFLAG_EOF))
	{
		
		data_buffer_size = (UInt32)(buffer_size_in_frames * core_audio_file_container->outputFormat->mBytesPerFrame);
//		printf("data_buffer_size=%d\n", data_buffer_size); 

		buffer_size_in_frames = buffer_size_in_frames_remaining;
		
//		printf("reading buffer_size_in_frames=%"PRId64"\n", buffer_size_in_frames); 


		audio_buffer_list.mBuffers[0].mDataByteSize = bytes_remaining;
		audio_buffer_list.mBuffers[0].mData = &(((UInt8*)internal->buffer)[total_bytes_read]);

		
		/* Read the data into an AudioBufferList */
		error_result = ExtAudioFileRead(core_audio_file_container->extAudioFileRef, &buffer_size_in_frames, &audio_buffer_list);
		if(error_result == noErr)
		{
		
		
			/* Success */
			
			total_frames_read += buffer_size_in_frames;
			buffer_size_in_frames_remaining = buffer_size_in_frames_remaining - buffer_size_in_frames;
			
//			printf("read buffer_size_in_frames=%"PRId64", buffer_size_in_frames_remaining=%"PRId64"\n", buffer_size_in_frames, buffer_size_in_frames_remaining); 

			/* ExtAudioFileRead returns the number of frames actually read. Need to convert back to bytes. */
			data_buffer_size = (UInt32)(buffer_size_in_frames * core_audio_file_container->outputFormat->mBytesPerFrame);
//			printf("data_buffer_size=%d\n", data_buffer_size); 

			total_bytes_read += data_buffer_size;
			bytes_remaining = bytes_remaining - data_buffer_size;

			/* Note: 0 == buffer_size_in_frames is a legitimate value meaning we are EOF. */
			if(0 == buffer_size_in_frames)
			{
				sample->flags |= SOUND_SAMPLEFLAG_EOF;			
			}

		}
		else 
		{
			SNDDBG(("Core Audio: ExtAudioFileReadfailed, reason: [%s].\n", CoreAudio_FourCCToString(error_result)));

			sample->flags |= SOUND_SAMPLEFLAG_ERROR;
			break;
			
		}
	}
	
	if( (!(sample->flags & SOUND_SAMPLEFLAG_EOF)) && (total_bytes_read < max_buffer_size))
	{
		SNDDBG(("Core Audio: ExtAudioFileReadfailed SOUND_SAMPLEFLAG_EAGAIN, reason: [total_bytes_read < max_buffer_size], %d, %d.\n", total_bytes_read , max_buffer_size));
		
		/* Don't know what to do here. */
		sample->flags |= SOUND_SAMPLEFLAG_EAGAIN;
	}
	return total_bytes_read;
} /* CoreAudio_read */


static int CoreAudio_rewind(Sound_Sample *sample)
{
	OSStatus error_result = noErr;	
	Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
	CoreAudioFileContainer* core_audio_file_container = (CoreAudioFileContainer *) internal->decoder_private;
	
	error_result = ExtAudioFileSeek(core_audio_file_container->extAudioFileRef, 0);
	if(error_result != noErr)
	{
		sample->flags |= SOUND_SAMPLEFLAG_ERROR;
	}
	return(1);
} /* CoreAudio_rewind */

/* Note: I found this tech note:
 http://developer.apple.com/library/mac/#qa/qa2009/qa1678.html
 I don't know if this applies to us. So far, I haven't noticed the problem,
 so I haven't applied any of the techniques.
 */
static int CoreAudio_seek(Sound_Sample *sample, size_t ms)
{
	OSStatus error_result = noErr;	
	Sound_SampleInternal *internal = (Sound_SampleInternal *) sample->opaque;
	CoreAudioFileContainer* core_audio_file_container = (CoreAudioFileContainer *) internal->decoder_private;
	SInt64 frame_offset = 0;

	/* I'm confused. The Apple documentation says this:
	 "Seek position is specified in the sample rate and frame count of the file’s audio data format
	 — not your application’s audio data format."
	 My interpretation is that I want to get the "actual format of the file and compute the frame offset.
	 But when I try that, seeking goes to the wrong place.
	 When I use outputFormat, things seem to work correctly.
	 I must be misinterpreting the documentation or doing something wrong.
	 */
#if 0 /* not working */
	AudioStreamBasicDescription	actual_format;
	UInt32 format_size;
    format_size = sizeof(AudioStreamBasicDescription);
    error_result = AudioFileGetProperty(
		*core_audio_file_container->audioFileID,
		kAudioFilePropertyDataFormat,
		&format_size,
		&actual_format
	);
    if(error_result != noErr)
	{
		sample->flags |= SOUND_SAMPLEFLAG_ERROR;
		BAIL_MACRO("Core Audio: Could not GetProperty for kAudioFilePropertyDataFormat.", 0);
	} /* if */

	// packetIndex = (pos * sampleRate) / framesPerPacket
	frame_offset = (SInt64)((ms/1000.0 * actual_format.mSampleRate) / actual_format.mFramesPerPacket);
	// computed against actual format and not the client format

	// packetIndex = (pos * sampleRate) / framesPerPacket
	//	frame_offset = (SInt64)((ms/1000.0 * actual_format.mSampleRate) / actual_format.mFramesPerPacket);
#else /* seems to work, but I'm confused */
	// packetIndex = (pos * sampleRate) / framesPerPacket
	frame_offset = (SInt64)((ms/1000.0 * core_audio_file_container->outputFormat->mSampleRate) / core_audio_file_container->outputFormat->mFramesPerPacket);	
#endif
	
	// computed against actual format and not the client format
	error_result = ExtAudioFileSeek(core_audio_file_container->extAudioFileRef, frame_offset);
	if(error_result != noErr)
	{
		sample->flags |= SOUND_SAMPLEFLAG_ERROR;
	}
	return(1);
} /* CoreAudio_seek */

#endif /* __APPLE__ */


#endif /* ALMIXER_COMPILE_WITHOUT_SDL */

