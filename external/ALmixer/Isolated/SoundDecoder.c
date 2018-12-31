#ifndef ALMIXER_COMPILED_WITH_SDL

#include "SoundDecoder.h"
#include "SoundDecoder_Internal.h"
#include "tErrorLib.h"
#include "LinkedList.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef __ANDROID__
#include <android/log.h>
#endif

/* A partial shim reimplementation of SDL_sound to work around the LGPL issues.
 * This implementation is more limited than SDL_sound. 
 * For example, there is no generic software conversion routines.
 * Functions are also not necessarily thread safe.
 * This implementation relies on the back-end decoder much more heavily 
 * than SDL_sound. (For example, I bypass the internal->buffer.)
 */

static LinkedList* s_listOfLoadedSamples = NULL;

static signed char s_isInitialized = 0;
static TErrorPool* s_errorPool = NULL;

static const SoundDecoder_DecoderInfo** s_availableDecoders = NULL;

#ifdef __APPLE__ /* I'm making Apple use the Core Audio backend. */
	//extern const SoundDecoder_DecoderFunctions __SoundDecoder_DecoderFunctions_CoreAudio;
	extern const Sound_DecoderFunctions __Sound_DecoderFunctions_CoreAudio;
#else /* All other platforms. */
#	if (defined(_WIN32) && !defined(WINAPI_FAMILY)) || (defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP))
		extern const Sound_DecoderFunctions __Sound_DecoderFunctions_DirectX;
#	endif
	#ifdef SOUND_SUPPORTS_WAV
		extern const Sound_DecoderFunctions __Sound_DecoderFunctions_WAV;
	#endif
	#ifdef SOUND_SUPPORTS_MPG123
		extern const Sound_DecoderFunctions __Sound_DecoderFunctions_MPG123;
	#endif
#endif

/* Note: Make sure to compile only Vorbis xor Tremor, not both. */
#ifdef SOUND_SUPPORTS_OGG
	extern const Sound_DecoderFunctions __Sound_DecoderFunctions_OGG;
#endif

typedef struct
{
    int available;
    const SoundDecoder_DecoderFunctions* funcs;
} SoundElement;

static SoundElement s_linkedDecoders[] =
{
#if defined(__APPLE__)
		{ 0, &__Sound_DecoderFunctions_CoreAudio },
#else /* All other platforms. */
#	if ( (defined(_WIN32) && !defined(WINAPI_FAMILY)) || (defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)) && !defined(Rtt_ANDROID_ENV) )
	#if (!defined(NINTENDO_LIB))
		{ 0, &__Sound_DecoderFunctions_DirectX },
	#endif
#	endif
	#ifdef SOUND_SUPPORTS_WAV
		{ 0, &__Sound_DecoderFunctions_WAV },
	#endif
	#ifdef SOUND_SUPPORTS_MPG123
		{ 0, &__Sound_DecoderFunctions_MPG123 },
	#endif
#endif
/* Note: Make sure to link only Vorbis xor Tremor, not both. */		
#ifdef SOUND_SUPPORTS_OGG
    { 0, &__Sound_DecoderFunctions_OGG },
#endif
    { 0, NULL }
};


#include <ctype.h>
int SoundDecoder_strcasecmp(const char* str1, const char* str2)
{
	int the_char1;
	int the_char2;
	int i = 0;
	if(str1 == str2)
	{
		return 0;
	}
	if(NULL == str1)
	{
		return -1;
	}
	if(NULL == str2)
	{
		return 1;
	}

	do
	{
		the_char1 = tolower(str1[i]);
		the_char2 = tolower(str2[i]);
		if(the_char1 < the_char2)
		{
			return -1;
		}
		else if(the_char1 > the_char2)
		{
			return 1;
		}
		i++;
	} while( (0 != the_char1) && (0 != the_char2) );

	return 0;
}


#ifdef __ANDROID__
#include <stdarg.h>
#include <android/log.h>
int SoundDecoder_DebugPrint(const char* format, ...)
{
	va_list arg_list; 
	int ret_val;

	va_start(arg_list, format); 
	ret_val = __android_log_vprint(ANDROID_LOG_INFO, "SoundDecoder", format, arg_list); 
	va_end(arg_list);
	return ret_val;
}
#endif

const char* SoundDecoder_GetError()
{
	const char* error_string = NULL;
	if(NULL == s_errorPool)
	{
		return "Error: You should not call SoundDecoder_GetError while Sound is not initialized";
	}
	error_string = TError_GetLastErrorStr(s_errorPool);
	/* SDL returns empty strings instead of NULL */
	if(NULL == error_string)
	{
		return "";
	}
	else
	{
		return error_string;
	}
}

void SoundDecoder_ClearError()
{
	if(NULL == s_errorPool)
	{
		return;
	}
	TError_SetError(s_errorPool, 0, NULL);
}

void SoundDecoder_SetError(const char* err_str, ...)
{
	va_list argp;

	if (NULL == s_errorPool)
	{
		fprintf(stderr, "Error: You should not call SoundDecoder_SetError while Sound is not initialized\n");
		return;
	}
	va_start(argp, err_str);
	// SDL_SetError which I'm emulating has no number parameter.
	TError_SetErrorv(s_errorPool, 1, err_str, argp);
	va_end(argp);
#ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "SoundDecoder_SetError", "%s", TError_GetLastErrorStr(s_errorPool));
#endif
}


void SoundDecoder_GetLinkedVersion(SoundDecoder_Version* the_version)
{
	if(NULL != the_version)
	{
		the_version->major = SOUNDDECODER_VER_MAJOR;
		the_version->minor = SOUNDDECODER_VER_MINOR;
		the_version->patch = SOUNDDECODER_VER_PATCH;
	}
}


const SoundDecoder_DecoderInfo** SoundDecoder_AvailableDecoders()
{
    return(s_availableDecoders);
}

int SoundDecoder_Init()
{
	size_t total_number_of_decoders;
	size_t i;
	size_t current_pos = 0;	
	if(1 == s_isInitialized)
	{
		return 1;
	}
	if(NULL == s_errorPool)
	{
		s_errorPool = TError_CreateErrorPool();
	}
	if(NULL == s_errorPool)
	{
		return 0;
	}

	total_number_of_decoders = sizeof(s_linkedDecoders) / sizeof(s_linkedDecoders[0]);
	s_availableDecoders = (const SoundDecoder_DecoderInfo **)malloc((total_number_of_decoders) * sizeof(SoundDecoder_DecoderInfo*));
	if(NULL == s_availableDecoders)
	{
		SoundDecoder_SetError(ERR_OUT_OF_MEMORY);
		return 0;
	}

	/* Allocate memory for linked list of sound samples. */
	s_listOfLoadedSamples = LinkedList_Create();
	if(NULL == s_listOfLoadedSamples)
	{
		free((void*)s_availableDecoders);
		s_availableDecoders = NULL;
		SoundDecoder_SetError(ERR_OUT_OF_MEMORY);
		return 0;
	}

	for(i = 0; s_linkedDecoders[i].funcs != NULL; i++)
	{
		s_linkedDecoders[i].available = s_linkedDecoders[i].funcs->init();
		if(s_linkedDecoders[i].available)
		{
			s_availableDecoders[current_pos] = &(s_linkedDecoders[i].funcs->info);
			current_pos++;
		}
	}

	s_availableDecoders[current_pos] = NULL;
	s_isInitialized = 1;
	return 1;
}

void SoundDecoder_Quit()
{
    size_t i;	
	if(0 == s_isInitialized)
	{
		return;
	}

	/* 
	 * SDL_sound actually embeds the linked list in the internal data structure.
	 * So any sample can potentially reach any other sample.
	 * But I'm keeping my own separate list.
	 */
	while(LinkedList_Size(s_listOfLoadedSamples) > 0)
	{
		SoundDecoder_Sample* sound_sample = (SoundDecoder_Sample*)LinkedList_PopBack(s_listOfLoadedSamples);
		SoundDecoder_FreeSample(sound_sample);
	}
	LinkedList_Free(s_listOfLoadedSamples);
	s_listOfLoadedSamples = NULL;

	
    for(i = 0; s_linkedDecoders[i].funcs != NULL; i++)
    {
        if (s_linkedDecoders[i].available)
        {
            s_linkedDecoders[i].funcs->quit();
            s_linkedDecoders[i].available = 0;
        }
    }

    if(NULL != s_availableDecoders)
	{
        free((void*)s_availableDecoders);
	}
	s_availableDecoders = NULL;


	/* Remember: ALmixer_SetError/GetError calls will not work while this is gone. */
	TError_FreeErrorPool(s_errorPool);
	s_errorPool = NULL;

	s_isInitialized = 0;	
}


void SoundDecoder_FreeSample(SoundDecoder_Sample* sound_sample)
{
	SoundDecoder_SampleInternal* sample_internal;
	LinkedListNode* the_node;

	/* Quit unloads all samples, so it is not possible to free a sample
	 * when not initialized.
	 */
	if(0 == s_isInitialized)
	{
		return;
	}

	if(sound_sample == NULL)
	{
		return;
	}

	/* SDL_sound keeps a linked list of all the loaded samples.
	 * We want to remove the current sample from that list.
	 */
	the_node = LinkedList_Find(s_listOfLoadedSamples, sound_sample, NULL);
	if(NULL == the_node)
	{
		SoundDecoder_SetError("SoundDecoder_FreeSample: Internal Error, sample does not exist in linked list.");
		return;
	}
	LinkedList_Remove(s_listOfLoadedSamples, the_node);

	sample_internal = (SoundDecoder_SampleInternal*)sound_sample->opaque;

	/* Ugh...SDL_sound has a lot of pointers. 
	 * I hope I didn't miss any dynamic memory.
	 */

	/* Call close on the decoder */
	sample_internal->funcs->close(sound_sample);

	if(NULL != sample_internal->rw)
	{
		sample_internal->rw->close(sample_internal->rw);
	}

	/* Ooops. The public buffer might be shared with the internal buffer.
	 * Make sure to not accidentally double delete.
	 */
	if((NULL != sample_internal->buffer)
		&& (sample_internal->buffer != sound_sample->buffer)
	)
	{
		free(sample_internal->buffer);
	}
	free(sample_internal);

	if(NULL != sound_sample->buffer)
	{
		free(sound_sample->buffer);
	}
	free(sound_sample);
}



static int Internal_LoadSample(const SoundDecoder_DecoderFunctions *funcs,
	SoundDecoder_Sample* sound_sample, const char *ext
)
{
    SoundDecoder_SampleInternal* internal_sample = (SoundDecoder_SampleInternal*)sound_sample->opaque;
    long current_file_position = internal_sample->rw->seek(internal_sample->rw, 0, SEEK_CUR);

    /* fill in the funcs for this decoder... */
    sound_sample->decoder = &funcs->info;
	internal_sample->funcs = funcs;
	if (!funcs->open(sound_sample, ext))
	{
		internal_sample->rw->seek(internal_sample->rw, current_file_position, SEEK_SET);
		return 0;
	}

	/* we found a compatible decoder */

	/* SDL_sound normally goes on to setup a bunch of things to 
	 * support format conversion via SDL APIs.
	 * I am not porting any of that stuff.
	 * My goal is to simply setup the struct properties to values
	 * that will not cause any confusion with other parts of the implementation.
	 */


	if(0 == sound_sample->desired.format)
	{
		sound_sample->desired.format = sound_sample->actual.format;
	}
	if(0 == sound_sample->desired.channels)
	{
		sound_sample->desired.channels = sound_sample->actual.channels;
	}
	if(0 == sound_sample->desired.rate)
	{
		sound_sample->desired.rate = sound_sample->actual.rate;
	}

	/* I'm a little confused at the difference between the internal
	 * public buffer. I am going to make them the same.
	 * I assume I already allocated the public buffer.
	 */
	internal_sample->buffer = sound_sample->buffer;
	internal_sample->buffer_size = sound_sample->buffer_size;

	/* Insert the new sample into the linked list of samples. */
	LinkedList_PushBack(s_listOfLoadedSamples, sound_sample);
	
	return 1;
}



SoundDecoder_Sample* SoundDecoder_NewSampleFromFile(const char* file_name,
                                      SoundDecoder_AudioInfo* desired_format,
                                      size_t buffer_size)
{
	
    const char* file_extension;
    ALmixer_RWops* rw_ops;


	if(0 == s_isInitialized)
	{
		SoundDecoder_SetError(ERR_NOT_INITIALIZED);
		return NULL;
	}
	if(NULL == file_name)
	{
		SoundDecoder_SetError("No file specified");
		return NULL;
	}

    file_extension = strrchr(file_name, '.');
	if(NULL != file_extension)
	{
        file_extension++;
	}

    rw_ops = ALmixer_RWFromFile(file_name, "rb");

     return SoundDecoder_NewSample(rw_ops, file_extension, desired_format, buffer_size);
}


SoundDecoder_Sample* SoundDecoder_NewSample(ALmixer_RWops* rw_ops, const char* file_extension, SoundDecoder_AudioInfo* desired_format, size_t buffer_size)
{
    SoundDecoder_Sample* new_sample;
    SoundDecoder_SampleInternal* internal_sample;
    SoundElement* current_decoder;

	if(0 == s_isInitialized)
	{
		SoundDecoder_SetError(ERR_NOT_INITIALIZED);
		return NULL;
	}
	if(NULL == rw_ops)
	{
		SoundDecoder_SetError("No file specified");
		return NULL;
	}

	new_sample = (SoundDecoder_Sample*)calloc(1, sizeof(SoundDecoder_Sample));
	if(NULL == new_sample)
	{
		SoundDecoder_SetError(ERR_OUT_OF_MEMORY);
		return NULL;
	}
	internal_sample = (SoundDecoder_SampleInternal*)calloc(1, sizeof(SoundDecoder_SampleInternal));
	if(NULL == internal_sample)
	{
		free(new_sample);
		SoundDecoder_SetError(ERR_OUT_OF_MEMORY);
		return NULL;
	}
	new_sample->buffer = calloc(1, buffer_size);
	if(NULL == new_sample->buffer)
	{
		free(internal_sample);
		free(new_sample);
		SoundDecoder_SetError(ERR_OUT_OF_MEMORY);
		return NULL;
	}

	new_sample->buffer_size = buffer_size;

    if(NULL != desired_format)
	{
		memcpy(&new_sample->desired, desired_format, sizeof(SoundDecoder_AudioInfo));
	}

	internal_sample->rw = rw_ops;
	new_sample->opaque = internal_sample;


    if(NULL != file_extension)
    {
        for(current_decoder = &s_linkedDecoders[0]; current_decoder->funcs != NULL; current_decoder++)
        {
            if(current_decoder->available)
            {
                const char** decoder_file_extension = current_decoder->funcs->info.extensions;
                while(*decoder_file_extension)
                {
                    if(0 == (SoundDecoder_strcasecmp(*decoder_file_extension, file_extension)))
                    {
                        if(Internal_LoadSample(current_decoder->funcs, new_sample, file_extension))
						{
							return(new_sample);
						}
                        break;  /* go to the next decoder */ 
                    }
					decoder_file_extension++;
                }
            }
        }
    }

    /* no direct file_extensionension match? Try everything we've got... */
	for(current_decoder = &s_linkedDecoders[0]; current_decoder->funcs != NULL; current_decoder++)
	{
		if(current_decoder->available)
		{
			int already_tried_decoder = 0;
			const char** decoder_file_extension = current_decoder->funcs->info.extensions;

			/* skip decoders we already tried from the above loop */
			while(*decoder_file_extension)
			{
				if(SoundDecoder_strcasecmp(*decoder_file_extension, file_extension) == 0)
				{
					already_tried_decoder = 1;
					break;
				}
				decoder_file_extension++;
			}

			if(0 == already_tried_decoder)
			{
				if (Internal_LoadSample(current_decoder->funcs, new_sample, file_extension))
				{
					return new_sample;
				}
			}
		}
	}

	/* could not find a decoder */
	SoundDecoder_SetError(ERR_UNSUPPORTED_FORMAT);
	/* clean up the memory */
	free(new_sample->opaque);
	if(NULL != new_sample->buffer)
	{
		free(new_sample->buffer);
	}
	free(new_sample);
	
	rw_ops->close(rw_ops);
	return NULL;
}


int SoundDecoder_SetBufferSize(SoundDecoder_Sample* sound_sample, size_t new_buffer_size)
{
	SoundDecoder_SampleInternal* internal_sample = NULL;
	void* new_buffer_ptr = NULL;

    BAIL_IF_MACRO(!s_isInitialized, ERR_NOT_INITIALIZED, 0);
	BAIL_IF_MACRO(NULL == sound_sample, ERR_NULL_SAMPLE, 0);

    internal_sample = ((SoundDecoder_SampleInternal*)sound_sample->opaque);


    new_buffer_ptr = realloc(sound_sample->buffer, new_buffer_size);
    BAIL_IF_MACRO(NULL == new_buffer_ptr, ERR_OUT_OF_MEMORY, 0);

	sound_sample->buffer = new_buffer_ptr;
    sound_sample->buffer_size = new_buffer_size;
	internal_sample->buffer = sound_sample->buffer;
	internal_sample->buffer_size = sound_sample->buffer_size;

    return 1;
}


size_t SoundDecoder_Decode(SoundDecoder_Sample* sound_sample)
{
    SoundDecoder_SampleInternal* internal_sample = NULL;
    size_t bytes_read = 0;

    BAIL_IF_MACRO(!s_isInitialized, ERR_NOT_INITIALIZED, 0);
	BAIL_IF_MACRO(NULL == sound_sample, ERR_NULL_SAMPLE, 0);
	BAIL_IF_MACRO(sound_sample->flags & SOUND_SAMPLEFLAG_ERROR, ERR_PREVIOUS_SAMPLE_ERROR, 0);
	BAIL_IF_MACRO(sound_sample->flags & SOUND_SAMPLEFLAG_EOF, ERR_ALREADY_AT_EOF_ERROR, 0);

    internal_sample = (SoundDecoder_SampleInternal*)sound_sample->opaque;

    assert(sound_sample->buffer != NULL);
    assert(sound_sample->buffer_size > 0);
    assert(internal_sample->buffer != NULL);
    assert(internal_sample->buffer_size > 0);

 	/* reset EAGAIN. Decoder can flip it back on if it needs to. */
    sound_sample->flags &= ~SOUND_SAMPLEFLAG_EAGAIN;
    bytes_read = internal_sample->funcs->read(sound_sample);
    return bytes_read;
}


size_t SoundDecoder_DecodeAll(SoundDecoder_Sample* sound_sample)
{
    SoundDecoder_SampleInternal* internal_sample = NULL;
    void* data_buffer = NULL;
    size_t updated_buffer_size = 0;

    BAIL_IF_MACRO(!s_isInitialized, ERR_NOT_INITIALIZED, 0);
	BAIL_IF_MACRO(NULL == sound_sample, ERR_NULL_SAMPLE, 0);

	/* My original thought was to call SetBufferSize and resize to 
	 * the size needed to hold the entire decoded file utilizing total_time,
	 * but it appears SDL_sound simply loops on SoundDecoder_Decode.
	 * I suppose it is possible to partially decode or seek a file, and then
	 * call DecodeAll so you won't have the whole thing in which case
	 * my idea would waste memory.
	 */
    while( (0 == (sound_sample->flags & SOUND_SAMPLEFLAG_EOF) )
		&& (0 == (sound_sample->flags & SOUND_SAMPLEFLAG_ERROR))
	)
    {
        size_t bytes_decoded = SoundDecoder_Decode(sound_sample);
        void* realloced_ptr = realloc(data_buffer, updated_buffer_size + bytes_decoded);
        if(NULL == realloced_ptr)
        {
            sound_sample->flags |= SOUND_SAMPLEFLAG_ERROR;
            SoundDecoder_SetError(ERR_OUT_OF_MEMORY);
			if(NULL != data_buffer)
			{
				free(data_buffer);
			}
			return bytes_decoded;
        }
		data_buffer = realloced_ptr;
		/* copy the chunk of decoded PCM to the end of our new data buffer */
		memcpy( ((char*)data_buffer) + updated_buffer_size, sound_sample->buffer, bytes_decoded );
		updated_buffer_size += bytes_decoded;
    }

    internal_sample = (SoundDecoder_SampleInternal*)sound_sample->opaque;
	if(internal_sample->buffer != sound_sample->buffer)
	{
		free(internal_sample->buffer);
	}
    free(sound_sample->buffer);

	sound_sample->buffer = data_buffer;
	sound_sample->buffer_size = updated_buffer_size;
	internal_sample->buffer = sound_sample->buffer;
	internal_sample->buffer_size = sound_sample->buffer_size;

    return sound_sample->buffer_size;
}


int SoundDecoder_Rewind(SoundDecoder_Sample* sound_sample)
{
    SoundDecoder_SampleInternal* internal_sample;
	int ret_val;
	
    BAIL_IF_MACRO(!s_isInitialized, ERR_NOT_INITIALIZED, 0);
	BAIL_IF_MACRO(NULL == sound_sample, ERR_NULL_SAMPLE, 0);

    internal_sample = (SoundDecoder_SampleInternal*)sound_sample->opaque;
	ret_val = internal_sample->funcs->rewind(sound_sample);
    if(0 == ret_val)
    {
        sound_sample->flags |= SOUND_SAMPLEFLAG_ERROR;
		SoundDecoder_SetError("Rewind failed");		
        return 0;
    }
	/* reset flags */
    sound_sample->flags &= ~SOUND_SAMPLEFLAG_EAGAIN;
    sound_sample->flags &= ~SOUND_SAMPLEFLAG_ERROR;
    sound_sample->flags &= ~SOUND_SAMPLEFLAG_EOF;

    return 1;
}


int SoundDecoder_Seek(SoundDecoder_Sample* sound_sample, size_t ms)
{
    SoundDecoder_SampleInternal* internal_sample;
	int ret_val;

    BAIL_IF_MACRO(!s_isInitialized, ERR_NOT_INITIALIZED, 0);
	BAIL_IF_MACRO(NULL == sound_sample, ERR_NULL_SAMPLE, 0);
	
	BAIL_IF_MACRO(!(sound_sample->flags & SOUND_SAMPLEFLAG_CANSEEK), "Sound sample is not seekable", 0);

    internal_sample = (SoundDecoder_SampleInternal*)sound_sample->opaque;
	ret_val = internal_sample->funcs->seek(sound_sample, ms);
	if(0 == ret_val)
	{
        sound_sample->flags |= SOUND_SAMPLEFLAG_ERROR;		
		SoundDecoder_SetError("Seek failed");
		return 0;
	}
	/* reset flags */
    sound_sample->flags &= ~SOUND_SAMPLEFLAG_EAGAIN;
    sound_sample->flags &= ~SOUND_SAMPLEFLAG_ERROR;
    sound_sample->flags &= ~SOUND_SAMPLEFLAG_EOF;

    return 1;
}


ptrdiff_t SoundDecoder_GetDuration(SoundDecoder_Sample* sound_sample)
{
    SoundDecoder_SampleInternal* internal_sample;
    BAIL_IF_MACRO(!s_isInitialized, ERR_NOT_INITIALIZED, -1);
	BAIL_IF_MACRO(NULL == sound_sample, ERR_NULL_SAMPLE, 0);	
    internal_sample = (SoundDecoder_SampleInternal*)sound_sample->opaque;
    return internal_sample->total_time;
}

#endif
