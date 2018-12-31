
/*
 * This is a wrapper interface that tries to provide a similar
 * front-end interface to SDL_sound.
 */

#ifndef SOUNDDECODER_H 
#define SOUNDDECODER_H

#ifdef __cplusplus
extern "C" {
#endif
	
	#if defined(_WIN32) || defined(WINAPI_FAMILY)
		#if defined(SOUND_DECODER_BUILD_LIBRARY)
			#define SOUND_DECODER_DECLSPEC __declspec(dllexport)
		#else
			#define SOUND_DECODER_DECLSPEC
		#endif
	#else
		#if defined(SOUND_DECODER_BUILD_LIBRARY)
			#if defined (__GNUC__) && __GNUC__ >= 4
				#define SOUND_DECODER_DECLSPEC __attribute__((visibility("default")))
			#else
				#define SOUND_DECODER_DECLSPEC
			#endif
		#else
			#define SOUND_DECODER_DECLSPEC
		#endif
	#endif

/*
 * For some reason, the __cdecl triggers a compiler error for Windows Phone with the SoundDecoder_DecoderInfo struct below.
 * I don't know why. This is to be investigated later.
 */
//	#if defined(_WIN32) || defined(WINAPI_FAMILY)
//		#define SOUND_DECODER_CALL __cdecl
//	#else
		#define SOUND_DECODER_CALL
//	#endif
	
#include <stdint.h>
#include <stddef.h>

#include "al.h" /* OpenAL */

/* Compatibility defines for SDL */
#define AUDIO_U8 0x0008
#define AUDIO_S8 0x8008
#define AUDIO_U16LSB 0x0010	
#define AUDIO_S16LSB 0x8010
#define AUDIO_U16MSB 0x1010
#define AUDIO_S16MSB 0x9010
#define AUDIO_U16 AUDIO_U16LSB
#define AUDIO_S16 AUDIO_S16LSB

/* Apple defines __BIG_ENDIAN__ and __LITTLE_ENDIAN__ appropriately.
 * Linux provides <endian.h>
 */
#if !defined(__BIG_ENDIAN__) && !defined(__LITTLE_ENDIAN__)
	#ifdef __linux__
		#include <endian.h>
		#if _BYTE_ORDER == _BIG_ENDIAN
			#define __BIG_ENDIAN__ 1
		#else
			#define __LITTLE_ENDIAN__ 1
		#endif
	#else /* __linux __ */
		#if defined(__hppa__) || \
			defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
			(defined(__MIPS__) && defined(__MISPEB__)) || \
			defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
			defined(__sparc__)
		
			#define __BIG_ENDIAN__ 1
		#else
			#define __LITTLE_ENDIAN__ 1
		#endif
	#endif /* __linux __ */
#endif /*  */


#if __BIG_ENDIAN__
	/* #warning "Using __BIG_ENDIAN__" */
	#define AUDIO_U16SYS AUDIO_U16MSB
	#define AUDIO_S16SYS AUDIO_S16MSB
#elif __LITTLE_ENDIAN__
	#define AUDIO_U16SYS	AUDIO_U16LSB
	#define AUDIO_S16SYS	AUDIO_S16LSB
#else
	#warning "Using __LITTLE_ENDIAN__ as fallback"
	#define AUDIO_U16SYS	AUDIO_U16LSB
	#define AUDIO_S16SYS	AUDIO_S16LSB
#endif

struct ALmixer_RWops;

typedef enum
{
	SOUND_SAMPLEFLAG_NONE = 0,
	SOUND_SAMPLEFLAG_CANSEEK = 1,
	SOUND_SAMPLEFLAG_EOF     = 1 << 29,
	SOUND_SAMPLEFLAG_ERROR   = 1 << 30,
	SOUND_SAMPLEFLAG_EAGAIN  = 1 << 31 
} SoundDecoder_SampleFlags;

#define Sound_SampleFlags SoundDecoder_SampleFlags;

typedef struct SoundDecoder_AudioInfo
{
    //uint16_t format;  /**< Equivalent of SDL_AudioSpec.format. */
    ALushort format;  /**< Equivalent of SDL_AudioSpec.format. */
    ALubyte channels;
    // uint8_t channels;
    //uint32_t rate; 
    ALuint rate; 
} SoundDecoder_AudioInfo;

//#define Sound_AudioInfo SoundDecoder_AudioInfo;
typedef struct SoundDecoder_AudioInfo Sound_AudioInfo;



typedef struct SoundDecoder_DecoderInfo
{
    const char** extensions;
    const char* description;
    const char* author;
    const char* url;
} SoundDecoder_DecoderInfo;

//#define Sound_DecoderInfo SoundDecoder_DecoderInfo;
typedef struct SoundDecoder_DecoderInfo Sound_DecoderInfo;



typedef struct SoundDecoder_Sample
{
    void* opaque;
    const SoundDecoder_DecoderInfo* decoder;
	SoundDecoder_AudioInfo desired;
    SoundDecoder_AudioInfo actual;
    void *buffer;
    size_t buffer_size;
    SoundDecoder_SampleFlags flags;
} SoundDecoder_Sample;

//#define Sound_Sample SoundDecoder_Sample;
typedef struct SoundDecoder_Sample Sound_Sample;


typedef struct SoundDecoder_Version
{
    int major;
    int minor;
    int patch;
} SoundDecoder_Version;

//#define Sound_Version SoundDecoder_Version;
typedef struct SoundDecoder_Version Sound_Version;


#define SOUNDDECODER_VER_MAJOR 0
#define SOUNDDECODER_VER_MINOR 0
#define SOUNDDECODER_VER_PATCH 1

#define SOUNDDECODER_VERSION(x) \
{ \
    (x)->major = SOUNDDECODER_VER_MAJOR; \
    (x)->minor = SOUNDDECODER_VER_MINOR; \
    (x)->patch = SOUNDDECODER_VER_PATCH; \
}

#define SOUND_VERSION SOUNDDECODER_VERSION

extern SOUND_DECODER_DECLSPEC void SOUND_DECODER_CALL SoundDecoder_GetLinkedVersion(SoundDecoder_Version *ver);
#define Sound_GetLinkedVersion SoundDecoder_GetLinkedVersion

extern SOUND_DECODER_DECLSPEC int SOUND_DECODER_CALL SoundDecoder_Init(void);
#define Sound_Init SoundDecoder_Init

extern SOUND_DECODER_DECLSPEC void SOUND_DECODER_CALL SoundDecoder_Quit(void);
#define Sound_Quit SoundDecoder_Quit


extern SOUND_DECODER_DECLSPEC const SOUND_DECODER_CALL SoundDecoder_DecoderInfo** SoundDecoder_AvailableDecoders(void);
#define Sound_AvailableDecoders SoundDecoder_AvailableDecoders


extern SOUND_DECODER_DECLSPEC const char* SOUND_DECODER_CALL SoundDecoder_GetError(void);
#define Sound_GetError SoundDecoder_GetError


extern SOUND_DECODER_DECLSPEC void SOUND_DECODER_CALL SoundDecoder_ClearError(void);
#define Sound_ClearError SoundDecoder_ClearError



extern SOUND_DECODER_DECLSPEC SoundDecoder_Sample* SOUND_DECODER_CALL SoundDecoder_NewSample(
	struct ALmixer_RWops* rw_ops,
	const char* ext,
	SoundDecoder_AudioInfo* desired,
	size_t buffer_size);
#define Sound_NewSample SoundDecoder_NewSample

extern SOUND_DECODER_DECLSPEC SoundDecoder_Sample* SOUND_DECODER_CALL SoundDecoder_NewSampleFromFile(const char* file_name,
	SoundDecoder_AudioInfo* desired,
	size_t bufferSize);
#define Sound_NewSampleFromFile SoundDecoder_NewSampleFromFile


extern SOUND_DECODER_DECLSPEC void SOUND_DECODER_CALL SoundDecoder_FreeSample(SoundDecoder_Sample* sound_sample);
#define Sound_FreeSample SoundDecoder_FreeSample


extern SOUND_DECODER_DECLSPEC ptrdiff_t SOUND_DECODER_CALL SoundDecoder_GetDuration(SoundDecoder_Sample* sound_sample);
#define Sound_GetDuration SoundDecoder_GetDuration

extern SOUND_DECODER_DECLSPEC int SOUND_DECODER_CALL SoundDecoder_SetBufferSize(SoundDecoder_Sample* sound_sample, size_t new_buffer_size);
#define Sound_SetBufferSize SoundDecoder_SetBufferSize

extern SOUND_DECODER_DECLSPEC size_t SOUND_DECODER_CALL SoundDecoder_Decode(SoundDecoder_Sample* sound_sample);
#define Sound_Decode SoundDecoder_Decode

extern SOUND_DECODER_DECLSPEC size_t SOUND_DECODER_CALL SoundDecoder_DecodeAll(SoundDecoder_Sample* sound_sample);
#define Sound_DecodeAll SoundDecoder_DecodeAll

extern SOUND_DECODER_DECLSPEC int SOUND_DECODER_CALL SoundDecoder_Rewind(SoundDecoder_Sample* sound_sample);
#define Sound_Rewind SoundDecoder_Rewind

extern SOUND_DECODER_DECLSPEC int SOUND_DECODER_CALL SoundDecoder_Seek(SoundDecoder_Sample* sound_sample, size_t ms);
#define Sound_Seek SoundDecoder_Seek


/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif

