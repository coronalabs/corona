
/* Here's an OpenAL implementation modeled after 
 * the SDL_SoundMixer which was built ontop of SDL_Mixer
 * and SDL_Sound. 
 * Eric Wing
 */

#include "ALmixer.h"
#include "CORONA_LABS_ALmixerUtils.c"

// Note that ALMIXER_COMPILE_WITHOUT_SDL is defined for device builds but not for desktop ones thus
// the behavior of the code may be quite different on different platforms.

#ifdef ALMIXER_COMPILE_WITHOUT_SDL
	#include "ALmixer_RWops.h"
	#include "SoundDecoder.h"
#else
	#include "SDL_sound.h"
#endif

#include "al.h" /* OpenAL */
#include "alc.h" /* For creating OpenAL contexts */

#ifdef __APPLE__
	/* For performance things like ALC_CONVERT_DATA_UPON_LOADING */
	/* Note: ALC_CONVERT_DATA_UPON_LOADING used to be in the alc.h header.
	 * But in the Tiger OpenAL 1.1 release (10.4.7 and Xcode 2.4), the 
	 * define was moved to a new header file and renamed to
	 * ALC_MAC_OSX_CONVERT_DATA_UPON_LOADING.
	 */
	#include <TargetConditionals.h>
/*
	#if (TARGET_OS_IPHONE == 1) || (TARGET_IPHONE_SIMULATOR == 1)
		#include <AudioToolbox/AudioToolbox.h>
	#else
		#include <OpenAL/MacOSX_OALExtensions.h>
	#endif
*/
#endif

/* For malloc, bsearch, qsort */
#include <stdlib.h>

/* For va_list, va_start(), va_end(). */
#include <stdarg.h>

/* For memcpy */
#include <string.h>

#if 0
/* for toupper */
#include <ctype.h>
/* for strrchr */
#include <string.h>
#endif

/* Currently used in the output debug functions */
#include <stdio.h>

/* My own CircularQueue implementation needed
 * to work around the Nvidia problem of the
 * lack of a buffer query.
 */
#include "CircularQueue.h"

/* SDL_sound keeps a private linked list of sounds which get auto-deleted
 * on Sound_Quit. This might actually create some leaks for me in certain
 * usage patterns. To be safe, I should do the same.
 */
#include "LinkedList.h"

#ifdef __ANDROID__
	#undef fprintf
	#include <android/log.h>
	#define fprintf(stderr, ...) __android_log_print(ANDROID_LOG_INFO, "ALmixer", __VA_ARGS__)
#endif

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
	#include "WinRTServices.h"
#endif

#ifdef ENABLE_ALMIXER_THREADS
/* Needed for the Mutex locks (and threads if enabled) */
	#ifdef ALMIXER_COMPILE_WITHOUT_SDL
		#include "SimpleMutex.h"
		#include "SimpleThread.h"
		typedef struct SimpleMutex SDL_mutex;
		typedef struct SimpleThread SDL_Thread;
		#define SDL_CreateMutex SimpleMutex_CreateMutex
		#define SDL_DestroyMutex SimpleMutex_DestroyMutex
		#define SDL_LockMutex SimpleMutex_LockMutex
		#define SDL_UnlockMutex SimpleMutex_UnlockMutex
		#define SDL_CreateThread SimpleThread_CreateThread
		#define SDL_WaitThread SimpleThread_WaitThread
	
	#else
		#include "SDL_thread.h"
	#endif
#endif

/* Because of the API differences between the Loki
 * and Creative distributions, we need to know which
 * version to use. The LOKI distribution currently
 * has AL_BYTE_LOKI defined in altypes.h which
 * I will use as a flag to identify the distributions.
 * If this is ever removed, I might revert back to the
 * if defined(_WIN32) or defined(__APPLE__) test to
 * identify the Creative dist. 
 * I'm not sure if or how the Nvidia distribution differs
 * from the Creative distribution. So for
 * now, the Nvidia distribution gets lumped with the 
 * Creative dist and I hope nothing will break.
 * My alGetString may be the most vulnerable.
 */
#ifdef AL_BYTE_LOKI
	#define USING_LOKI_AL_DIST
	/* This is a short term fix to get around the 
	 * queuing problem with non-power of two buffer sizes.
	 * Hopefully the maintainers will fix this before 
	 * we're ready to ship.
	 */
	#define ENABLE_LOKI_QUEUE_FIX_HACK

	/* The AL_GAIN in the Loki dist doesn't seem to do
	 * what I want/expect it to do. I want to use it for 
	 * Fading, but it seems to work like an off/on switch.
	 * 0 = off, >0 = on. 
	 * The AL_GAIN_LINEAR_LOKI switch seems to do what 
	 * I want, so I'll redefine it here so the code is consistent
	 */
	/* Update: I've changed the source volume implementations 
	 * to use AL_MAX_GAIN, so I don't think I need this block 
	 * of code anymore. The listener uses AL_GAIN, but I 
	 * hope they got this one right since there isn't a AL_MAX_GAIN
	 * for the listener.
	 */
/*
	#undef AL_GAIN
	#include "alexttypes.h"
	#define AL_GAIN AL_GAIN_LINEAR_LOKI
*/
#else
	/* Might need to run other tests to figure out the DIST */
	/* I've been told that Nvidia doesn't define constants
	 * in the headers like Creative. Instead of
	 * #define AL_REFERENCE_DISTANCE 0x1020, 
	 * Nvidia prefers you query OpenAL for a value.
	 * int AL_REFERENCE_DISTANCE = alGetEnumValue(ALubyte*)"AL_REFERNECE_DISTANCE");
	 * So I'm assuming this means the Nvidia lacks this value.
	 * If this is the case,
	 * I guess we can use it to identify the Nvidia dist
	 */
	#ifdef AL_REFERENCE_DISTANCE
		#define USING_CREATIVE_AL_DIST
	#else
		#define USING_NVIDIA_AL_DIST
	#endif
#endif

#ifdef ENABLE_LOKI_QUEUE_FIX_HACK
/* Need memset to zero out data */
#include <string.h>
#endif


/* Seek issues for predecoded samples:
 * The problem is that OpenAL makes us copy an
 * entire buffer if we want to use it. This 
 * means we potentially have two copies of the 
 * same data. For predecoded data, this can be a 
 * large amount of memory. However, for seek 
 * support, I need to be able to get access to 
 * the original data so I can set byte positions.
 * The following flags let you disable seek support
 * if you don't want the memory hit, keep everything,
 * or let you try to minimize the memory wasted by
 * fetching it from the OpenAL buffer if needed
 * and making a copy of it.
 * Update: I don't think I need this flag anymore. I've made the
 * effects of this user customizable by the access_data flag on load.
 * If set to true, then seek and data callbacks work, with the 
 * cost of more memory and possibly CPU for copying the data through
 * the callbacks. If false, then the extra memory is freed, but 
 * you don't get the features.
 */
/*
#define DISABLE_PREDECODED_SEEK
*/
/* Problem: Even though alGetBufferi(., AL_DATA, .)
 * is in the Creative Programmer's reference,
 * it actually isn't in the dist. (Invalid enum
 * in Creative, can't compile in Loki.)
 * So we have to keep it disabled
 */
#define DISABLE_SEEK_MEMORY_OPTIMIZATION

#ifndef DISABLE_SEEK_MEMORY_OPTIMIZATION
/* Needed for memcpy */
#include <string.h>
#endif

/* Old way of doing things: 
#if defined(_WIN32) || defined(__APPLE__)			
#define USING_CREATIVE_AL_DIST
#else
#define USING_LOKI_AL_DIST
#endif
*/

/************ REMOVE  ME (Don't need anymore) ********/
#if 0 
/* Let's get fancy and see if triple buffering
 * does anything good for us 
 * Must be 2 or more or things will probably break
 */
#define NUMBER_OF_QUEUE_BUFFERS 	5
/* This is the number of buffers that are queued up
 * when play first starts up. This should be at least 1
 * and no more than NUMBER_OF_QUEUE_BUFFERS
 */
#define NUMBER_OF_START_UP_BUFFERS 	2
#endif
/************ END REMOVE  ME (Don't need anymore) ********/

#ifdef ALMIXER_COMPILE_WITHOUT_SDL
	#include "tErrorLib.h"
	static TErrorPool* s_ALmixerErrorPool = NULL;
#endif

static ALboolean ALmixer_Initialized = AL_FALSE;
/* This should be set correctly by Init */
static ALuint ALmixer_Frequency_global = ALMIXER_DEFAULT_FREQUENCY;

/* Will be initialized in Init */
static ALint Number_of_Channels_global = 0;
static ALint Number_of_Reserve_Channels_global = 0;
static ALuint Is_Playing_global = 0;

#ifdef ENABLE_ALMIXER_THREADS
/* This is for a simple lock system. It is not meant to be good,
 * but just sufficient to minimize/avoid threading issues
 */
static ALboolean g_StreamThreadEnabled = AL_FALSE;
static SDL_mutex* s_simpleLock;
static SDL_Thread* Stream_Thread_global = NULL;
#endif /* ENABLE_ALMIXER_THREADS */

static LinkedList* s_listOfALmixerData = NULL;

/* Special stuff for iOS interruption handling */
ALboolean g_inInterruption = AL_FALSE;
static ALCcontext* s_interruptionContext = NULL;

#ifndef ALMIXER_COMPILE_WITHOUT_SDL

static void ALmixer_PrintErrors(const char *fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	fputs("Warning: audio error: ", stderr);
	vfprintf(stderr, fmt, argp);
    fputc('\n', stderr);

	va_end(argp);
}

// This is defined elsewhere but via a convoluted route just blackholes all errors so we
// define something useful here statically
#undef ALmixer_SetError
#define ALmixer_SetError ALmixer_PrintErrors

#endif // ! ALMIXER_COMPILE_WITHOUT_SDL

#ifdef __APPLE__
static ALvoid Internal_alcMacOSXMixerOutputRate(const ALdouble sample_rate)
{
    static void (*alcMacOSXMixerOutputRateProcPtr)(const ALdouble) = NULL;
    
    if(NULL == alcMacOSXMixerOutputRateProcPtr)
	{
		alcMacOSXMixerOutputRateProcPtr = alGetProcAddress((const ALCchar*) "alcMacOSXMixerOutputRate");
    }
	
    if(NULL != alcMacOSXMixerOutputRateProcPtr)
	{
        alcMacOSXMixerOutputRateProcPtr(sample_rate);		
	}
	
    return;
}

ALdouble Internal_alcMacOSXGetMixerOutputRate()
{
    static ALdouble (*alcMacOSXGetMixerOutputRateProcPtr)(void) = NULL;
    
    if(NULL == alcMacOSXGetMixerOutputRateProcPtr)
	{
		alcMacOSXGetMixerOutputRateProcPtr = alGetProcAddress((const ALCchar*) "alcMacOSXGetMixerOutputRate");
    }
	
    if(NULL != alcMacOSXGetMixerOutputRateProcPtr)
	{
        return alcMacOSXGetMixerOutputRateProcPtr();		
	}
	
    return 0.0;
}
#endif

#ifdef ALMIXER_COMPILE_WITHOUT_SDL

	#if defined(__APPLE__)
		#include <QuartzCore/QuartzCore.h>
   	  	#include <unistd.h>
		static CFTimeInterval s_ticksBaseTime = 0.0;
		
	#elif defined(_WIN32) || defined(WINAPI_FAMILY)
		#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
		#include <winbase.h>
			LARGE_INTEGER s_hiResTicksPerSecond;
			double s_hiResSecondsPerTick;
			LARGE_INTEGER s_ticksBaseTime;
	#else
   	  	#include <unistd.h>
		#include <time.h>
		static struct timespec s_ticksBaseTime;
	#endif
	static void ALmixer_InitTime()
	{
		#if defined(__APPLE__)
			s_ticksBaseTime = CACurrentMediaTime();
		
		#elif defined(_WIN32) || defined(WINAPI_FAMILY)
			LARGE_INTEGER hi_res_ticks_per_second;
			if(TRUE == QueryPerformanceFrequency(&hi_res_ticks_per_second))
			{
				QueryPerformanceCounter(&s_ticksBaseTime);
				s_hiResSecondsPerTick = 1.0 / hi_res_ticks_per_second.QuadPart;
			}
			else
			{
				ALmixer_SetError("Windows error: High resolution clock failed.");
				fprintf(stderr, "Windows error: High resolution clock failed. Audio will not work correctly.\n");
			}
		#else
			/* clock_gettime is POSIX.1-2001 */
			clock_gettime(CLOCK_MONOTONIC, &s_ticksBaseTime);
		#endif

	}
	static ALuint ALmixer_GetTicks()
	{
		#if defined(__APPLE__)
			return (ALuint)((CACurrentMediaTime()-s_ticksBaseTime)*1000.0);
		#elif defined(_WIN32) || defined(WINAPI_FAMILY)
			LARGE_INTEGER current_time;
			QueryPerformanceCounter(&current_time);
			return (ALuint)((current_time.QuadPart - s_ticksBaseTime.QuadPart) * 1000 * s_hiResSecondsPerTick);
		#else /* assuming POSIX */
			/* clock_gettime is POSIX.1-2001 */
			struct timespec current_time;
			clock_gettime(CLOCK_MONOTONIC, &current_time);
			return (ALuint)((current_time.tv_sec - s_ticksBaseTime.tv_sec)*1000.0 + (current_time.tv_nsec - s_ticksBaseTime.tv_nsec) / 1000000);
		#endif
	}
	static void ALmixer_Delay(ALuint milliseconds_delay)
	{
		#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
			WinRTServices_Sleep(milliseconds_delay);
		#elif defined(_WIN32)
			Sleep(milliseconds_delay);
		#else
			usleep(milliseconds_delay*1000); // Microseconds = ( Milliseconds * 1000 ).
		#endif
	}
#else
	#include "SDL.h" /* For SDL_GetTicks(), SDL_Delay */
	#define ALmixer_GetTicks SDL_GetTicks
	#define ALmixer_Delay SDL_Delay
#endif

/* On iOS, usleep() of small numbers (say less than 100, very pronounced from 0-50)
 * seems to be sucking up quite a bit of CPU time and causing performance problems.
 * Instead of increasing the sleep time, I started changing the thread priority.
 * This seemed to help the problem.
 * Experimentally, the default priority seems to be 31. According to the docs,
 * valid ranges are from 0 to 31. 6 was still giving me some hiccups so setting to
 * 0 (PTHREAD_MIN_PRIORITY) seems to be the best value so far.
 * Mac also reports 31 as the default. However, I have not noticed the same
 * performance problems and cannot get audio to show up as a significant percentage
 * of the CPU time in Shark/Instruments.
 */
#ifdef ENABLE_ALMIXER_THREADS
#if defined(__APPLE__) && !defined(ALMIXER_COMPILE_WITHOUT_SDL) && ( (TARGET_OS_IPHONE == 1) || (TARGET_IPHONE_SIMULATOR == 1) )
#include <pthread.h>
#endif
static void Internal_LowerThreadPriority(SDL_Thread* simple_thread)
{
	/* Might open to other platforms as needed */
#if defined(__APPLE__) && ( (TARGET_OS_IPHONE == 1) || (TARGET_IPHONE_SIMULATOR == 1) )
	#ifdef ALMIXER_COMPILE_WITHOUT_SDL
		SimpleThread_SetThreadPriority(Stream_Thread_global, 0);
	#else
		struct sched_param schedule_param;
		int sched_policy;
		int ret_val;
		schedule_param.sched_priority = 0; /* PTHREAD_MIN_PRIORITY, max=31 */
		/* EVIL! This will break if the SDL_Thread structure layout changes. */
		pthread_t* native_thread_ptr_hack = (pthread_t*)(((char*)(Stream_Thread_global))+sizeof(unsigned long));
		ret_val = pthread_setschedparam(*native_thread_ptr_hack, SCHED_OTHER, &schedule_param);
	#endif
#else
	/* No-Op */
#endif
}
#endif /* ENABLE_ALMIXER_THREADS */

/* If ENABLE_PARANOID_SIGNEDNESS_CHECK is used,
 * these values will be reset on Init()
 * Consider these values Read-Only.
 */

#define ALMIXER_SIGNED_VALUE 127
#define ALMIXER_UNSIGNED_VALUE 255

#ifdef ENABLE_PARANOID_SIGNEDNESS_CHECK
static ALushort SIGN_TYPE_16_BIT_FORMAT = AUDIO_S16SYS;
static ALushort SIGN_TYPE_8_BIT_FORMAT = AUDIO_S8;
#else
static const ALushort SIGN_TYPE_16_BIT_FORMAT = AUDIO_S16SYS;
static const ALushort SIGN_TYPE_8_BIT_FORMAT = AUDIO_S8;
#endif


/* This can be private instead of being in the header now that I moved
 * ALmixer_Data inside here.
 */
typedef struct ALmixer_Buffer_Map ALmixer_Buffer_Map;


struct ALmixer_Data
{
	ALboolean decoded_all; /* dictates different behaviors */
	ALint total_time; /* total playing time of sample (msec), obsolete now that we pushed our changes to SDL_sound */
	
	ALuint in_use; /* needed to prevent sharing for streams */
	ALboolean eof; /* flag for eof, only used for streams  */
	
	ALuint total_bytes; /* For predecoded */
	ALuint loaded_bytes; /* For predecoded (for seek) */

	Sound_Sample* sample; /* SDL_Sound provides the data */
	ALuint* buffer; /* array of OpenAL buffers (at least 1 for predecoded) */

	/* Needed for streamed buffers */
	ALuint max_queue_buffers; /* Max number of queue buffers */
	ALuint num_startup_buffers; /* Number of ramp-up buffers */
	ALuint num_buffers_in_use; /* number of buffers in use */
	ALuint num_target_buffers_per_pass; /* number of buffers to try to queue in an update pass */
	
	/* This stuff is for streamed buffers that require data access */
	ALmixer_Buffer_Map* buffer_map_list; /* translate ALbuffer to index 
									and holds pointer to copy of data for
									data access */
	ALuint current_buffer; /* The current playing buffer */

	/* Nvidia distribution refuses to recognize a simple buffer query command
	 * unlike all other distributions. It's forcing me to redo the code 
	 * to accomodate this Nvidia flaw by making me maintain a "best guess"
	 * copy of what I think the buffer queue state looks like.
	 * A circular queue would be a helpful data structure for this task,
	 * but I wanted to avoid making an additional header requirement,
	 * so I'm making it a void* 
	 */
	void* circular_buffer_queue; 
		
	
};

static struct ALmixer_Channel
{
	ALboolean channel_in_use;
	ALboolean callback_update; /* For streaming determination */
	ALboolean needs_stream; /* For streaming determination */
	ALboolean halted;
	ALboolean paused;
	ALuint alsource;
	ALmixer_Data* almixer_data;
	ALint loops;
	ALint expire_ticks;
	ALuint start_time;

	ALboolean fade_enabled;
	ALuint fade_expire_ticks;
	ALuint fade_start_time;
	ALfloat fade_inv_time;
	ALfloat fade_start_volume;
	ALfloat fade_end_volume;
	ALfloat max_volume;
	ALfloat min_volume;

	/* Do we need other flags?
	ALbyte *samples;
	int volume;
	int looping;
	int tag;
	ALuint expire;
	ALuint start_time;
	Mix_Fading fading;
	int fade_volume;
	ALuint fade_length;
	ALuint ticks_fade;
	effect_info *effects;
	*/
} *ALmixer_Channel_List = NULL;

struct ALmixer_Buffer_Map
{
	ALuint albuffer;
	ALint index; /* might not need */
	ALbyte* data;
	ALuint num_bytes;
};

/* This will be used to find a channel if the user supplies a source */
typedef struct Source_Map
{
    ALuint source;
    ALint channel;
} Source_Map;
/* Keep an array of all sources with their associated channel */
static Source_Map* Source_Map_List;

static int Compare_Source_Map(const void* a, const void* b)
{
    return ( ((Source_Map*)a)->source - ((Source_Map*)b)->source );
}

/* Sort by channel instead of source */
static int Compare_Source_Map_by_channel(const void* a, const void* b)
{
    return ( ((Source_Map*)a)->channel - ((Source_Map*)b)->channel );
}

/* Compare by albuffer */
static int Compare_Buffer_Map(const void* a, const void* b)
{
    return ( ((ALmixer_Buffer_Map*)a)->albuffer - ((ALmixer_Buffer_Map*)b)->albuffer );
}

/* This is for the user defined callback via 
 * ALmixer_ChannelFinished()
 */
static void (*Channel_Done_Callback)(ALint which_channel, ALuint al_source, ALmixer_Data* almixer_data, ALboolean finished_naturally, void* user_data) = NULL;
static void* Channel_Done_Callback_Userdata = NULL;
static void (*Channel_Data_Callback)(ALint which_channel, ALuint al_source, ALbyte* data, ALuint num_bytes, ALuint frequency, ALubyte channels, ALubyte bit_depth, ALboolean is_unsigned, ALboolean decode_mode_is_predecoded, ALuint length_in_msec, void* user_data) = NULL;
static void* Channel_Data_Callback_Userdata = NULL;

#if 0
static void PrintQueueStatus(ALuint source)
{
	ALint buffers_queued = 0;
	ALint buffers_processed = 0;
	ALenum error;
	
	/* Get the number of buffers still queued */
	alGetSourcei(
		source,
		AL_BUFFERS_QUEUED, 
		&buffers_queued
	);
	
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "Error in PrintQueueStatus, Can't get buffers_queued: %s\n",
			alGetString(error));				
	}
	/* Get the number of buffers processed
	 * so we know if we need to refill 
	 */
	alGetSourcei(
		source,
		AL_BUFFERS_PROCESSED, 
		&buffers_processed
	);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "Error in PrintQueueStatus, Can't get buffers_processed: %s\n",
			alGetString(error));				
	}
	
	/*
	fprintf(stderr, "For source: %d, buffers_queued=%d, buffers_processed=%d\n",
			source,
			buffers_queued,
			buffers_processed);
*/
}
#endif // 0


static void Init_Channel(ALint channel)
{
	ALmixer_Channel_List[channel].channel_in_use = 0;
	ALmixer_Channel_List[channel].callback_update = 0;
	ALmixer_Channel_List[channel].needs_stream = 0;
	ALmixer_Channel_List[channel].paused = 0;
	ALmixer_Channel_List[channel].halted = 0;
	ALmixer_Channel_List[channel].loops = 0;
	
	ALmixer_Channel_List[channel].expire_ticks = 0;
	ALmixer_Channel_List[channel].start_time = 0;

	ALmixer_Channel_List[channel].fade_enabled = 0;
	ALmixer_Channel_List[channel].fade_expire_ticks = 0;
	ALmixer_Channel_List[channel].fade_start_time = 0;
	ALmixer_Channel_List[channel].fade_inv_time = 0.0f;
	ALmixer_Channel_List[channel].fade_start_volume = 0.0f;
	ALmixer_Channel_List[channel].fade_end_volume = 0.0f;
	ALmixer_Channel_List[channel].max_volume = 1.0f;
	ALmixer_Channel_List[channel].min_volume = 0.0f;
	
	ALmixer_Channel_List[channel].almixer_data = NULL;
}
/* Quick helper function to clean up a channel 
 * after it's done playing */
static void Clean_Channel(ALint channel)
{
	ALenum error;
	ALmixer_Channel_List[channel].channel_in_use = 0;
	ALmixer_Channel_List[channel].callback_update = 0;
	ALmixer_Channel_List[channel].needs_stream = 0;
	ALmixer_Channel_List[channel].paused = 0;
	ALmixer_Channel_List[channel].halted = 0;
	ALmixer_Channel_List[channel].loops = 0;
	
		
	ALmixer_Channel_List[channel].expire_ticks = 0;
	ALmixer_Channel_List[channel].start_time = 0;

	ALmixer_Channel_List[channel].fade_enabled = 0;
	ALmixer_Channel_List[channel].fade_expire_ticks = 0;
	ALmixer_Channel_List[channel].fade_start_time = 0;
	ALmixer_Channel_List[channel].fade_inv_time = 0.0f;
	ALmixer_Channel_List[channel].fade_start_volume = 0.0f;
	ALmixer_Channel_List[channel].fade_end_volume = 0.0f;

	alSourcef(ALmixer_Channel_List[channel].alsource, AL_MAX_GAIN, 
		ALmixer_Channel_List[channel].max_volume);

	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "10Testing errpr before unqueue because getting stuff, for OS X this is expected: %s\n",
			alGetString(error));				
	}
	
	alSourcef(ALmixer_Channel_List[channel].alsource, AL_MIN_GAIN, 
		ALmixer_Channel_List[channel].min_volume);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "11Testing errpr before unqueue because getting stuff, for OS X this is expected: %s\n",
			alGetString(error));				
	}
	
	if(ALmixer_Channel_List[channel].almixer_data != NULL)
	{
		if(ALmixer_Channel_List[channel].almixer_data->in_use > 0)
		{
			ALmixer_Channel_List[channel].almixer_data->in_use--;
		}
	}
	/* Needed to determine if rewind is needed, can't reset */
	/*
	ALmixer_Channel_List[channel].almixer_data->eof = 0;
	*/

	ALmixer_Channel_List[channel].almixer_data = NULL;
}

/* What shoud this return?
 * 127 for signed, 255 for unsigned
 */
static ALubyte GetSignednessValue(ALushort format)
{
	switch(format)
	{
		case AUDIO_U8:
		case AUDIO_U16LSB:
		case AUDIO_U16MSB:
			return ALMIXER_UNSIGNED_VALUE;
			break;
		case AUDIO_S8:
		case AUDIO_S16LSB:
		case AUDIO_S16MSB:
			return ALMIXER_SIGNED_VALUE;
			break;
		default:
			return 0;
	}
	return 0;
}


static ALubyte GetBitDepth(ALushort format)
{
	ALubyte bit_depth = 16;
	
	switch(format)
	{
		case AUDIO_U8:
		case AUDIO_S8:
			bit_depth = 8;
			break;
				
		case AUDIO_U16LSB:
		/*
		case AUDIO_U16:
		*/
		case AUDIO_S16LSB:
		/*
		case AUDIO_S16:
		*/
		case AUDIO_U16MSB:
		case AUDIO_S16MSB:
		/*
		case AUDIO_U16SYS:
		case AUDIO_S16SYS:
		*/
			bit_depth = 16;
			break;
		default:
			bit_depth = 0;
	}
	return bit_depth;
}
	
/* Need to translate between SDL/SDL_Sound audiospec
 * and OpenAL conventions */
static ALenum TranslateFormat(Sound_AudioInfo* info)
{
	ALenum format_type;
	ALubyte bit_depth;
	
	bit_depth = GetBitDepth(info->format);
	if(0 == bit_depth)
	{
		fprintf(stderr, "Warning: Unknown bit depth. Setting to 16\n");
		bit_depth = 16;
	}
	
	switch (info->channels)
	{
		/* These formats are supported by all OpenAL library implementations. */
		case 1:
			format_type = (16 == bit_depth) ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
			break;
		case 2:
			format_type = (16 == bit_depth) ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8;
			break;

		/* These are non-standard OpenAL audio format constants (aka: OpenAL extensions).
		 * Querying for them via alGetEnumValue() is the safest approach. Will return 0 if not supported. */
		case 4:
			if (16 == bit_depth)
			{
				format_type = alGetEnumValue("AL_FORMAT_QUAD16");
			}
			else
			{
				format_type = alGetEnumValue("AL_FORMAT_QUAD8");
			}
			break;
		case 6:
			if (16 == bit_depth)
			{
				format_type = alGetEnumValue("AL_FORMAT_51CHN16");
			}
			else
			{
				format_type = alGetEnumValue("AL_FORMAT_51CHN8");
			}
			break;
		case 7:
			if (16 == bit_depth)
			{
				format_type = alGetEnumValue("AL_FORMAT_61CHN16");
			}
			else
			{
				format_type = alGetEnumValue("AL_FORMAT_61CHN8");
			}
			break;
		case 8:
			if (16 == bit_depth)
			{
				format_type = alGetEnumValue("AL_FORMAT_71CHN16");
			}
			else
			{
				format_type = alGetEnumValue("AL_FORMAT_71CHN8");
			}
			break;
		default:
			format_type = 0;
			break;
	}
	return format_type;
}


/* This will compute the total playing time
* based upon the number of bytes and audio info.
* (In prinicple, it should compute the time for any given length) 
*/
static ALuint Compute_Total_Time_Decomposed(ALuint bytes_per_sample, ALuint frequency, ALubyte channels, size_t total_bytes)
{
	double total_sec;
	ALuint total_msec;
	ALuint bytes_per_sec;
	
	if(0 == total_bytes)
	{
		return 0;
	}
	/* To compute Bytes per second, do
		* samples_per_sec * bytes_per_sample * number_of_channels
		*/
	bytes_per_sec = frequency * bytes_per_sample * channels;
	
	/* Now to get total time (sec), do
		* total_bytes / bytes_per_sec
		*/
	total_sec = total_bytes / (double)bytes_per_sec;
	
	/* Now convert seconds to milliseconds
		* Add .5 to the float to do rounding before the final cast
		*/
	total_msec = (ALuint) ( (total_sec * 1000) + 0.5 );
	/*
	 fprintf(stderr, "freq=%d, bytes_per_sample=%d, channels=%d, total_msec=%d\n", frequency, bytes_per_sample, channels, total_msec);
	*/
	return total_msec;
}

static ALuint Compute_Total_Time(Sound_AudioInfo *info, size_t total_bytes)
{
	ALuint bytes_per_sample;
	
	if(0 == total_bytes)
	{
		return 0;
	}
	/* SDL has a mask trick I was not aware of. Mask the upper bits
	 * of the format, and you get 8 or 16 which is the bits per sample.
 	 * Divide by 8bits_per_bytes and you get bytes_per_sample
	 * I tested this under 32-bit and 64-bit and big and little endian
	 * to make sure this still works since I have since moved from 
	 * Uint32 to unspecified size types like ALuint.
	 */
	bytes_per_sample = (ALuint) ((info->format & 0xFF) / 8);
	
	return Compute_Total_Time_Decomposed(bytes_per_sample, info->rate, info->channels, total_bytes);
} /* End Compute_Total_Time */
	

#ifndef ALMIXER_DISABLE_PREDECODED_PRECOMPUTE_BUFFER_SIZE_OPTIMIZATION
static size_t Compute_Total_Bytes_Decomposed(ALuint bytes_per_sample, ALuint frequency, ALubyte channels, ALuint total_msec)
{
	double total_sec;
	ALuint bytes_per_sec;
	size_t total_bytes;

	if(0 >= total_msec)
	{
		return 0;
	}
	/* To compute Bytes per second, do
		* samples_per_sec * bytes_per_sample * number_of_channels
		*/
	bytes_per_sec = frequency * bytes_per_sample * channels;
	
	/* convert milliseconds to seconds */
	total_sec = total_msec / 1000.0;

	/* Now to get total bytes */
	total_bytes = (size_t)(((double)bytes_per_sec * total_sec) + 0.5);
	
/*	 fprintf(stderr, "freq=%d, bytes_per_sample=%d, channels=%d, total_msec=%d, total_bytes=%d\n", frequency, bytes_per_sample, channels, total_msec, total_bytes);
*/

	return total_bytes;
}

static size_t Compute_Total_Bytes(Sound_AudioInfo *info, ALuint total_msec)
{
	ALuint bytes_per_sample;
	
	if(0 >= total_msec)
	{
		return 0;
	}
	/* SDL has a mask trick I was not aware of. Mask the upper bits
	 * of the format, and you get 8 or 16 which is the bits per sample.
 	 * Divide by 8bits_per_bytes and you get bytes_per_sample
	 * I tested this under 32-bit and 64-bit and big and little endian
	 * to make sure this still works since I have since moved from 
	 * Uint32 to unspecified size types like ALuint.
	 */
	bytes_per_sample = (ALuint) ((info->format & 0xFF) / 8);
	
	return Compute_Total_Bytes_Decomposed(bytes_per_sample, info->rate, info->channels, total_msec);
}


/* The back-end decoders seem to need to decode in quantized frame sizes.
 * So if I can pad the bytes to the next quanta, things might go more smoothly.
 */
static size_t Compute_Total_Bytes_With_Frame_Padding(Sound_AudioInfo *info, ALuint total_msec)
{
	ALuint bytes_per_sample;
	ALuint bytes_per_frame;
	size_t evenly_divisible_frames;
	size_t remainder_frames;
	size_t return_bytes;

	size_t total_bytes = Compute_Total_Bytes(info, total_msec);

	bytes_per_sample = (ALuint) ((info->format & 0xFF) / 8);
	
	bytes_per_frame = bytes_per_sample * info->channels;

	evenly_divisible_frames = total_bytes / bytes_per_frame;
	remainder_frames = total_bytes % bytes_per_frame;

	return_bytes = (evenly_divisible_frames * bytes_per_frame)  + (remainder_frames * bytes_per_frame);

	/* Experimentally, some times I see to come up short in 
	 * actual bytes decoded and I see a second pass is needed.
	 * I'm worried this may have additional performance implications.
	 * Sometimes in the second pass (depending on file), 
	 * I have seen between 0 and 18 bytes.
	 * I'm tempted to pad the bytes by some arbitrary amount.
	 * However, I think currently the way SDL_sound is implemented,
	 * there is a big waste of memory up front instead of per-pass,
	 * so maybe I shouldn't worry about this.
	 */
	/*
	return_bytes += 64;
	*/
	/*
	 fprintf(stderr, "remainder_frames=%d, padded_total_bytes=%d\n", remainder_frames, return_bytes);
	 */
	 return return_bytes;
}
#endif /* ALMIXER_DISABLE_PREDECODED_PRECOMPUTE_BUFFER_SIZE_OPTIMIZATION */



/**************** REMOVED ****************************/
/* This was removed because I originally thought
 * OpenAL could return a pointer to the buffer data,
 * but I was wrong. If something like that is ever
 * implemented, then this might become useful.
 */
#if 0
/* Reconstruct_Sound_Sample and Set_AudioInfo only
 * are needed if the Seek memory optimization is 
 * used. Also, the Loki dist doesn't seem to support
 * AL_DATA which I need for it.
 */
#ifndef DISABLE_SEEK_MEMORY_OPTIMIZATION

static void Set_AudioInfo(Sound_AudioInfo* info, ALint frequency, ALint bits, ALint channels)
{
	info->rate = (ALuint)frequency;
	info->channels = (ALubyte)channels;
	
	/* Not sure if it should be signed or unsigned. Hopefully
	 * that detail won't be needed.
	 */
	if(8 == bits)
	{
		info->format = AUDIO_U8;
	}
	else
	{
		info->format = AUDIO_U16SYS;
	}
	fprintf(stderr, "Audio info: freq=%d, chan=%d, format=%d\n", 
		info->rate, info->channels, info->format);
	
}


static ALint Reconstruct_Sound_Sample(ALmixer_Data* data)
{
	ALenum error;	
	ALint* data_from_albuffer;
	ALint freq;
	ALint bits;
	ALint channels;
	ALint size;
	
	/* Create memory all initiallized to 0. */
	data->sample = (Sound_Sample*)calloc(1, sizeof(Sound_Sample));
	if(NULL == data->sample)
	{
		ALmixer_SetError("Out of memory for Sound_Sample");
		return -1;
	}

	/* Clear errors */
	alGetError();

	alGetBufferi(data->buffer[0], AL_FREQUENCY, &freq);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		ALmixer_SetError("alGetBufferi(AL_FREQUENCY): %s", alGetString(error) );
		free(data->sample);
		data->sample = NULL;
		return -1;
	}
	
	alGetBufferi(data->buffer[0], AL_BITS, &bits);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		ALmixer_SetError("alGetBufferi(AL_BITS): %s", alGetString(error) );
		free(data->sample);
		data->sample = NULL;
		return -1;
	}

	alGetBufferi(data->buffer[0], AL_CHANNELS, &channels);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		ALmixer_SetError("alGetBufferi(AL_CHANNELS): %s", alGetString(error) );
		free(data->sample);
		data->sample = NULL;
		return -1;
	}

	alGetBufferi(data->buffer[0], AL_SIZE, &size);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		ALmixer_SetError("alGetBufferi(AL_SIZE): %s", alGetString(error) );
		free(data->sample);
		data->sample = NULL;
		return -1;
	}
	
	alGetBufferi(data->buffer[0], AL_DATA, data_from_albuffer);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		ALmixer_SetError("alGetBufferi(AL_DATA): %s", alGetString(error) );
		free(data->sample);
		data->sample = NULL;
		return -1;
	}
	
	if(size <= 0)
	{
		ALmixer_SetError("No data in al buffer");
		free(data->sample);
		data->sample = NULL;
		return -1;
	}
	
	/* Now that we have all the attributes, we need to 
	 * allocate memory for the buffer and reconstruct
	 * the AudioInfo attributes.
	 */
	data->sample->buffer = malloc(size*sizeof(ALbyte));
	if(NULL == data->sample->buffer)
	{
		ALmixer_SetError("Out of memory for sample->buffer (allocating %d bytes)", (size*sizeof(ALbyte)));
		free(data->sample);
		data->sample = NULL;
		return -1;
	}
	
	memcpy(data->sample->buffer, data_from_albuffer, size);
	data->sample->buffer_size = size;

	/* Fill up the Sound_AudioInfo structures */
	Set_AudioInfo(&data->sample->desired, freq, bits, channels);
	Set_AudioInfo(&data->sample->actual, freq, bits, channels);

	return 0;
}
	
#endif /* End DISABLE_SEEK_MEMORY_OPTIMIZATION */
#endif
/*************** END REMOVED *************************/	

static void Invoke_Channel_Done_Callback(ALint which_channel, ALboolean did_finish_naturally)
{
	if(NULL == Channel_Done_Callback)
	{
		return;
	}
	Channel_Done_Callback(which_channel, ALmixer_Channel_List[which_channel].alsource, ALmixer_Channel_List[which_channel].almixer_data, did_finish_naturally, Channel_Done_Callback_Userdata);
}

static ALint LookUpBuffer(ALuint buffer, ALmixer_Buffer_Map* buffer_map_list, ALuint num_items_in_list)
{
	/* Only the first value is used for the key */
	ALmixer_Buffer_Map key = { 0, 0, NULL, 0 };
	ALmixer_Buffer_Map* found_item = NULL;
	key.albuffer = buffer;

	/* Use the ANSI C binary search feature (yea!) */
	found_item = (ALmixer_Buffer_Map*)bsearch(&key, buffer_map_list, num_items_in_list, sizeof(ALmixer_Buffer_Map), Compare_Buffer_Map);
	if(NULL == found_item)
	{
		ALmixer_SetError("Can't find buffer");
		return -1;
	}
	return found_item->index;
}


/* FIXME: Need to pass back additional info to be useful.
 * Bit rate, stereo/mono (num chans), time in msec?
 * Precoded/streamed flag so user can plan for future data?
 */
/*
 * channels: 1 for mono, 2 for stereo
 *
 */
static void Invoke_Channel_Data_Callback(ALint which_channel, ALbyte* data, ALuint num_bytes, ALuint frequency, ALubyte channels, ALushort format, ALboolean decode_mode_is_predecoded)
{
	ALboolean is_unsigned;
	ALubyte bits_per_sample = GetBitDepth(format);
	ALuint bytes_per_sample;
	ALuint length_in_msec;

	if(GetSignednessValue(format) == ALMIXER_UNSIGNED_VALUE)
	{
		is_unsigned = 1;
	}
	else
	{
		is_unsigned = 0;
	}

	bytes_per_sample = (ALuint) (bits_per_sample / 8);

	length_in_msec = Compute_Total_Time_Decomposed(bytes_per_sample, frequency, channels, num_bytes);

/*
	fprintf(stderr, "%x %x %x %x, bytes=%d, whichchan=%d, freq=%d, channels=%d\n", data[0], data[1], data[2], data[3], num_bytes, channels, frequency, channels);
*/
	if(NULL == Channel_Data_Callback)
	{
		return;
	}
	/*
	 * Channel_Data_Callback(which_channel, data, num_bytes, frequency, channels, GetBitDepth(format), format, decode_mode_is_predecoded);
	*/
	Channel_Data_Callback(which_channel, ALmixer_Channel_List[which_channel].alsource, data, num_bytes, frequency, channels, bits_per_sample, is_unsigned, decode_mode_is_predecoded, length_in_msec, Channel_Data_Callback_Userdata);
}

static void Invoke_Predecoded_Channel_Data_Callback(ALint channel, ALmixer_Data* data)
{
	if(NULL == data->sample)
	{
		return;
	}
	/* The buffer position is complicated because if the current data was seeked,
	 * we must adjust the buffer to the seek position
	 */
	Invoke_Channel_Data_Callback(channel, 
		(((ALbyte*) data->sample->buffer) + (data->total_bytes - data->loaded_bytes) ),
		data->loaded_bytes,
		data->sample->desired.rate,
		data->sample->desired.channels,
		data->sample->desired.format,
		AL_TRUE
	);
}

static void Invoke_Streamed_Channel_Data_Callback(ALint channel, ALmixer_Data* data, ALuint buffer)
{
	ALint index;
	if(NULL == data->buffer_map_list)
	{
		return;
	}
	index = LookUpBuffer(buffer, data->buffer_map_list, data->max_queue_buffers);
	/* This should catch the case where all buffers are unqueued
	 * and the "current" buffer is id: 0
	 */
	if(-1 == index)
	{
		return;
	}
	Invoke_Channel_Data_Callback(channel, 
		data->buffer_map_list[index].data,
		data->buffer_map_list[index].num_bytes,
		data->sample->desired.rate,
		data->sample->desired.channels,
		data->sample->desired.format,
		AL_FALSE
	);
}

/* Converts milliseconds to byte positions.
 * This is needed for seeking on predecoded samples 
 */
static ALuint Convert_Msec_To_Byte_Pos(Sound_AudioInfo* audio_info, ALuint number_of_milliseconds)
{
	ALuint bytes_per_sample;
	ALuint bytes_per_frame;
	float bytes_per_millisecond;
	float byte_position;

	if(audio_info == NULL)
	{
		fprintf(stderr, "Error, info is NULL\n");
	}

	/* SDL has a mask trick I was not aware of. Mask the upper bits
	 * of the format, and you get 8 or 16 which is the bits per sample.
 	 * Divide by 8bits_per_bytes and you get bytes_per_sample
	 * I tested this under 32-bit and 64-bit and big and little endian
	 * to make sure this still works since I have since moved from 
	 * Uint32 to unspecified size types like ALuint.
	 */
	bytes_per_sample = (ALuint) ((audio_info->format & 0xFF) / 8);
	bytes_per_frame = bytes_per_sample * audio_info->channels;
	bytes_per_millisecond = (float)bytes_per_frame * (audio_info->rate / 1000.0f);
	byte_position = ((float)(number_of_milliseconds)) * bytes_per_millisecond;  
	return (ALuint)(byte_position + 0.5);
}

static ALint Set_Predecoded_Seek_Position(ALmixer_Data* data, ALuint byte_position)
{
	ALenum error;
	/* clear error */
	alGetError();
	
	/* Is it greater than,  or greater-than or equal to ?? */
	if(byte_position > data->total_bytes)
	{
		/* We can't go past the end, so set to end? */
		/*
	fprintf(stderr, "Error, can't seek past end\n");
	*/

	/* In case the below thing doesn't work, 
	 * just rewind the whole thing.
	 *
		alBufferData(data->buffer[0],
			TranslateFormat(&data->sample->desired), 
			 (ALbyte*) data->sample->buffer,
			 data->total_bytes,
			data->sample->desired.rate
		);
	*/
	
		/* I was trying to set to the end, (1 byte remaining),
		 * but I was getting freezes. I'm thinking it might be
		 * another Power of 2 bug in the Loki dist. I tried 2,
		 * and it still hung. 4 didn't hang, but I got a clip
		 * artifact. 8 seemed to work okay.
		 */
		alBufferData(data->buffer[0],
			TranslateFormat(&data->sample->desired), 
			 (((ALbyte*) data->sample->buffer) + (data->total_bytes - 8) ),
			 8,
			data->sample->desired.rate
		);
		if( (error = alGetError()) != AL_NO_ERROR)
		{
			ALmixer_SetError("Can't seek past end and alBufferData failed: %s\n", alGetString(error));
			return -1;
		}
		/* Need to set the loaded_bytes field because I don't trust the OpenAL
		 * query command to work because I don't know if it will mutilate the
		 * size for its own purposes or return the original size
		 */
		 data->loaded_bytes = 8;

		/* Not sure if this should be an error or not */
/*
		ALmixer_SetError("Can't Seek past end");	
		return -1;
*/
		return 0;
	}
	
	alBufferData(data->buffer[0],
		TranslateFormat(&data->sample->desired), 
		&(((ALbyte*)data->sample->buffer)[byte_position]),
		data->total_bytes - byte_position,
		data->sample->desired.rate
	);
	if( (error = alGetError()) != AL_NO_ERROR)
	{
		ALmixer_SetError("alBufferData failed: %s", alGetString(error));
		return -1;
	}
	/* Need to set the loaded_bytes field because I don't trust the OpenAL
	 * query command to work because I don't know if it will mutilate the
	 * size for its own purposes or return the original size
	 */
	 data->loaded_bytes = data->total_bytes - byte_position;

	return 0;
}

/* Because we have multiple queue buffers and OpenAL won't let
 * us access them, we need to keep copies of each buffer around
 */
static ALint CopyDataToAccessBuffer(ALmixer_Data* data, ALuint num_bytes, ALuint buffer)
{
	ALint index;
	/* We only want to copy if access_data is true.
	 * This is determined by whether memory has been
	 * allocated in the buffer_map_list or not
	 */
	if(NULL == data->buffer_map_list)
	{
		return -1;
	}
	index = LookUpBuffer(buffer, data->buffer_map_list, data->max_queue_buffers);
	if(-1 == index)
	{
		/*
fprintf(stderr, ">>>>>>>CopyData catch, albuffer=%d\n",buffer);
*/
		return -1;
	}
	/* Copy the data to the access buffer */
	memcpy(data->buffer_map_list[index].data, data->sample->buffer, num_bytes);
	data->buffer_map_list[index].num_bytes = data->sample->buffer_size;
	
	return 0;
}


/* For streamed data, gets more data
 * and prepares it in the active Mix_chunk
 */
static ALuint GetMoreData(ALmixer_Data* data, ALuint buffer)
{
	ALuint bytes_decoded;
	ALenum error;
	if( ! ( data && data->sample ) )
	{
		ALmixer_SetError("Cannot GetMoreData() because ALmixer_Data or Sound_Sample are NULL");
		return 0;
	}

	bytes_decoded = Sound_Decode(data->sample);
	if(data->sample->flags & SOUND_SAMPLEFLAG_ERROR)
	{
		ALmixer_SetError("GetMoreData: Sound_Decode: %s", Sound_GetError());
		/* Force cleanup through FreeData
		Sound_FreeSample(data->sample);
		*/
		return 0;
	}
	
/*	fprintf(stderr, "GetMoreData bytes_decoded=%d\n", bytes_decoded); */

	/* Don't forget to add check for EOF */
	/* Will return 0 bytes and pass the buck to check sample->flags */
	if(0 == bytes_decoded)
	{
		data->eof = 1;

#if 0 
fprintf(stderr, "Hit eof while trying to buffer\n");
		if(data->sample->flags & SOUND_SAMPLEFLAG_EOF)
		{
			fprintf(stderr, "\tEOF flag\n");
		}
		if(data->sample->flags & SOUND_SAMPLEFLAG_CANSEEK)
		{
			fprintf(stderr, "\tCanSeek flag\n");
		}
		if(data->sample->flags & SOUND_SAMPLEFLAG_EAGAIN)
		{
			fprintf(stderr, "\tEAGAIN flag\n");
		}
		if(data->sample->flags & SOUND_SAMPLEFLAG_NONE)
		{
			fprintf(stderr, "\tNONE flag\n");
		}
#endif
		return 0;
	}

#ifdef ENABLE_LOKI_QUEUE_FIX_HACK
/******* REMOVE ME ********************************/
/***************** ANOTHER EXPERIEMENT *******************/
	/* The PROBLEM: It seems that the Loki distribution has problems
	 * with Queuing when the buffer size is not a power of two
	 * and additional buffers must come after it.
	 * The behavior is inconsistent, but one of several things
	 * usually happens:
	 *   Playback is normal
	 *   Playback immediately stops after the non-pow2 buffer
	 *   Playback gets distorted on the non-pow2 buffer
	 *   The entire program segfaults.
	 * The workaround is to always specify a power of two buffer size
	 * and hope that SDL_sound always fill it. (By lucky coincidence,
	 * I already submitted the Ogg fix.) However, this won't catch
	 * cases where a loop happens because the read at the end of the
	 * file is typically less than the buffer size.
	 *
	 * This fix addresses this issue, however it may break in
	 * other conditions. Always decode in buffer sizes of powers of 2.
	 * 
	 * The HACK:
	 * If the buffer is short, try filling it up with 0's
 	 * to meet the user requested buffer_size which 
	 * is probably a nice number OpenAL likes, in 
	 * hopes to avoid a possible Loki bug with
	 * short buffers. If looping (which is the main
	 * reason for this), the negative side effect is
	 * that it may take longer for the loop to start
	 * because it must play dead silence. Or if the decoder
	 * doesn't guarantee to return the requested bytes
	 * (like Ogg), then you will get breakup in between
	 * packets.
	 */
	if( (bytes_decoded) < data->sample->buffer_size)
	{
		ALubyte bit_depth;
		ALubyte signedness_value;
		int silence_value;
		/* Crap, memset value needs to be the "silent" value, 
		 * but it will differ for signed/unsigned and bit depth
		 */
		bit_depth = GetBitDepth(data->sample->desired.format);
		signedness_value = GetSignednessValue(data->sample->desired.format);
		if(ALMIXER_SIGNED_VALUE == signedness_value)
		{
			/* I'm guessing that if it's signed, then 0 is the
			 * "silent" value */
			silence_value = 0;
		}
		else
		{
			if(8 == bit_depth)
			{
				/* If 8 bit, I'm guessing it's (2^7)-1 = 127 */ 
				silence_value = 127;
			}
			else
			{
				/* For 16 bit, I'm guessing it's (2^15)-1 = 32767 */
				silence_value = 32767;
			}
		}
		/* Now fill up the rest of the data buffer with the 
		 * silence_value.
		 * I don't think I have to worry about endian issues for
		 * this part since the data is for internal use only
		 * at this point.
		 */
		memset( &( ((ALbyte*)(data->sample->buffer))[bytes_decoded] ), silence_value, data->sample->buffer_size - bytes_decoded);
		/* Now reset the bytes_decoded to reflect the entire 
		 * buffer to tell alBufferData what our full size is.
		 */
		/*
	fprintf(stderr, "ALTERED bytes decoded for silence: Original end was %d\n", bytes_decoded);
	*/
		bytes_decoded = data->sample->buffer_size;
	}
/*********** END EXPERIMENT ******************************/
/******* END REMOVE ME ********************************/
#endif

	/* Now copy the data to the OpenAL buffer */
	/* We can't just set a pointer because the API needs
	 * its own copy to assist hardware acceleration */
	alBufferData(buffer,
		TranslateFormat(&data->sample->desired), 
		data->sample->buffer,
		bytes_decoded,
		data->sample->desired.rate
	);
	if( (error = alGetError()) != AL_NO_ERROR)
	{
		ALmixer_SetError("GetMoreData: alBufferData failed: %s", alGetString(error));
		return 0;
	}

	/* If we need to, copy the data also to the access area 
	 * (the function will do the check for us)
	 */
	CopyDataToAccessBuffer(data, bytes_decoded, buffer);
	return bytes_decoded;
}




/********************  EXPERIEMENT **************************** 
 * Test function to force maximum buffer filling during loops
 * REMOVE LATER
 *********************************************/
#if 0
static ALint GetMoreData2(ALmixer_Data* data, ALuint buffer)
{
	ALint bytes_decoded;
	ALenum error;
	if(NULL == data)
	{
		ALmixer_SetError("Cannot GetMoreData() because ALmixer_Data* is NULL");
		return -1;
	}

if(AL_FALSE == alIsBuffer(buffer))
{
	fprintf(stderr, "NOT A BUFFER>>>>>>>>>>>>>>>\n");
	return -1;
}
fprintf(stderr, "Entered GetMoreData222222: buffer id is %d\n", buffer);
	
/*
fprintf(stderr, "Decode in GetMoreData\n");
*/

#if 0
if(buffer%2 == 1)
{
	fprintf(stderr, "Setting buffer size to 16384\n");
	Sound_SetBufferSize(data->sample, 16384);
}
else
{
	fprintf(stderr, "Setting buffer size to 8192\n");
	Sound_SetBufferSize(data->sample, 8192);
}
#endif

	bytes_decoded = Sound_Decode(data->sample);
	if(data->sample->flags & SOUND_SAMPLEFLAG_ERROR)
	{
		ALmixer_SetError("GetMoreData2: Sound_Decode: %s", Sound_GetError());
		/*
		Sound_FreeSample(data->sample);
		*/
		return -1;
	}
	/* Don't forget to add check for EOF */
	/* Will return 0 bytes and pass the buck to check sample->flags */
	if(0 == bytes_decoded)
	{
#if 1 
fprintf(stderr, "Hit eof while trying to buffer\n");
		data->eof = 1;
		if(data->sample->flags & SOUND_SAMPLEFLAG_EOF)
		{
			fprintf(stderr, "\tEOF flag\n");
		}
		if(data->sample->flags & SOUND_SAMPLEFLAG_CANSEEK)
		{
			fprintf(stderr, "\tCanSeek flag\n");
		}
		if(data->sample->flags & SOUND_SAMPLEFLAG_EAGAIN)
		{
			fprintf(stderr, "\tEAGAIN flag\n");
		}
		if(data->sample->flags & SOUND_SAMPLEFLAG_NONE)
		{
			fprintf(stderr, "\tNONE flag\n");
		}
#endif
		return 0;
	}

	if(bytes_decoded < 16384)
	{
		char* tempbuffer1 = (char*)malloc(16384);
		char* tempbuffer2 = (char*)malloc(16384);
		int retval;
		memcpy(tempbuffer1, data->sample->buffer, bytes_decoded);
		retval = Sound_SetBufferSize(data->sample, 16384-bytes_decoded);
		if(retval == 1)
		{
			ALuint new_bytes;
			Sound_Rewind(data->sample);
			new_bytes = Sound_Decode(data->sample);
			fprintf(stderr, "Orig bytes: %d, Make up bytes_decoded=%d, total=%d\n", bytes_decoded, new_bytes, new_bytes+bytes_decoded);

			memcpy(tempbuffer2, data->sample->buffer, new_bytes);
			
		retval = Sound_SetBufferSize(data->sample, 16384);
		fprintf(stderr, "Finished reset...now danger copy\n");
			memcpy(data->sample->buffer, tempbuffer1,bytes_decoded);

		fprintf(stderr, "Finished reset...now danger copy2\n");
			memcpy( &( ((char*)(data->sample->buffer))[bytes_decoded] ), tempbuffer2, new_bytes);
			
		fprintf(stderr, "Finished \n");
			
			free(tempbuffer1);
			free(tempbuffer2);
			bytes_decoded += new_bytes;
			fprintf(stderr, "ASSERT bytes should equal 16384: %d\n", bytes_decoded);
		}
		else
		{
			fprintf(stderr, "Experiment failed: %s\n", Sound_GetError());
		}
	}
		
	/* Now copy the data to the OpenAL buffer */
	/* We can't just set a pointer because the API needs
	 * its own copy to assist hardware acceleration */
	alBufferData(buffer,
		TranslateFormat(&data->sample->desired), 
		data->sample->buffer,
		bytes_decoded,
		data->sample->desired.rate
	);
	if( (error = alGetError()) != AL_NO_ERROR)
	{
		ALmixer_SetError("GetMoreData2: alBufferData failed: %s\n", alGetString(error));
		return -1;
	}
	
    // fprintf(stderr, "GetMoreData2222 returning %d bytes decoded\n", bytes_decoded);
	return bytes_decoded;
}
#endif

/************ END EXPERIEMENT - REMOVE ME *************************/









/* This function will look up the source for the corresponding channel */
/* Must return 0 on error instead of -1 because of unsigned int */
static ALuint Internal_GetSource(ALint channel)
{
	ALint i;
	/* Make sure channel is in bounds */
	if(channel >= Number_of_Channels_global)
	{
		ALmixer_SetError("Requested channel (%d) exceeds maximum channel (%d) because only %d channels are allocated", channel, Number_of_Channels_global-1, Number_of_Channels_global);
		return 0;	
	}
	/* If the user specified -1, then return the an available source */
	if(channel < 0)
	{
		for(i=Number_of_Reserve_Channels_global; i<Number_of_Channels_global; i++)
		{
			if( ! ALmixer_Channel_List[i].channel_in_use )
			{
				return ALmixer_Channel_List[i].alsource;
			}
		}
		/* If we get here, all sources are in use */			
		/* Error message seems too harsh
		ALmixer_SetError("All sources are in use");
		*/
		return 0;
	}
	/* Last case: Return the source for the channel */	
	return ALmixer_Channel_List[channel].alsource; 
}

/* This function will look up the channel for the corresponding source */
static ALint Internal_GetChannel(ALuint source)
{
	ALint i;
	/* Only the first value is used for the key */
	Source_Map key = { 0, 0 };
	Source_Map* found_item = NULL;
	key.source = source;

	/* If the source is 0, look up the first available channel */
	if(0 == source)
	{
		for(i=Number_of_Reserve_Channels_global; i<Number_of_Channels_global; i++)
		{
			if( ! ALmixer_Channel_List[i].channel_in_use )
			{
				return i;
			}
		}
		/* If we get here, all sources are in use */			
		/* Error message seems too harsh
		ALmixer_SetError("All channels are in use");
		*/
		return -1;
	}
	
	
	/* Else, look up the source and return the channel */
	if(AL_FALSE == alIsSource(source))
	{
		ALmixer_SetError("Internal_GetChannel: Is not a source");
		return -1;
	}
	
	/* Use the ANSI C binary search feature (yea!) */
	found_item = (Source_Map*)bsearch(&key, Source_Map_List, Number_of_Channels_global, sizeof(Source_Map), Compare_Source_Map);
	if(NULL == found_item)
	{
		ALmixer_SetError("Source is valid but not registered with ALmixer (to a channel)");
		return -1;
	}
	return found_item->channel;
}



/* This function will find the first available channel (not in use)
 * from the specified start channel. Reserved channels to not qualify
 * as available.
 */
static ALint Internal_FindFreeChannel(ALint start_channel)
{
	/* Start at the number of reserved so we skip over
	 * all the reserved channels.
	 */
	ALint i = Number_of_Reserve_Channels_global;
	/* Quick check to see if we're out of bounds */
	if(start_channel >= Number_of_Channels_global)
	{
		return -1;
	}
	
	/* If the start channel is even higher than the reserved,
	 * then start at the higher value.
	 */
	if(start_channel > Number_of_Reserve_Channels_global)
	{
		i = start_channel;
	}
	
	/* i has already been set */
	for( ; i<Number_of_Channels_global; i++)
	{
		if( ! ALmixer_Channel_List[i].channel_in_use )
		{
			return i;
		}
	}
	/* If we get here, all sources are in use */			
	return -1;
}


static ALboolean Internal_DetachBuffersFromSource(ALuint source_id, ALboolean is_predecoded)
{
	ALboolean retval = AL_TRUE;
	ALenum error;
	/* Here's the situation. My old method of using
	 * alSourceUnqueueBuffers() seemed to be invalid in light
	 * of all the problems I suffered through with getting 
	 * the CoreData backend to work with this code.
	 * As such, I'm changing all the code to set the buffer to
	 * AL_NONE. Furthermore, the queued vs. non-queued issue
	 * doesn't need to apply here. For non-queued, Loki,
	 * Creative Windows, and CoreAudio seem to leave the 
	 * buffer queued (Old Mac didn't.) For queued, we need to 
	 * remove the processed buffers and force remove the
	 * still-queued buffers.
	 * Update: This code is was changed agian due to a serious regression bug in iOS 5.0
	 * Since the 1.1 spec, I think I can make some simplifying assumptions sans the iOS 5.0 bug.
	 * Before I looked at buffers_still_queued and buffers_processed.
	 * According to the spec, all buffers get marked processed when alSourceStop is called.
	 * Also, in addition, alSourcei(source, AL_BUFFER, AL_NONE) is supposed
	 * to detach buffers for both streamed and non-streamed so all the code
	 * should just go through that.
	 * Unfortunately, the iOS 5.0 bug doesn't detach/clear buffers with AL_NONE.
	 * So I need a special handler for iOS 5.0 which manually unqueues the buffers.
	 * Ironically, I think the original Mac version did not work reliably
	 * with unqueuing buffers which is why I moved the code to the AL_NONE
	 * solution in the first place. This means for safety, I need to
	 * conditionalize the workaround as not to risk breaking Mac or other * platforms.
	 */
#ifdef __APPLE__  /* iOS 5.0 workaround */
	#if (TARGET_OS_IPHONE == 1) || (TARGET_IPHONE_SIMULATOR == 1)
	/*
		fprintf(stderr, "kCFCoreFoundationVersionNumber: %lf\n", kCFCoreFoundationVersionNumber);
	 */
		/* I needed a C way to get the iOS version at runtime. This is returning 674.0 iOS 5.0 Beta 7. Apple hasn't updated the headers
		 * for these constants since iOS 4.2, so I don't know if the value is also catching 4.3.
		 * iOS 5.0.1 final is returning 675.0.
		 * TODO: Once I learn which version Apple fixes the bug in, I need to update the range so this check is not run on fixed versions.
		 * iOS 5.1 Beta 1 is returning 690.0.
		 */
		if(kCFCoreFoundationVersionNumber >= 674.0 &&  kCFCoreFoundationVersionNumber < 690.0)
		{
			/* For OpenAL experts, this is contrary to what you know, but must be done because the OpenAL implementation is broken.
			   Instead of unqueuing buffers on only streaming sources, it appears that alSourcei(source, AL_BUFFER, AL_NONE) is not reliable at all.
			   In cases where I switch between stream and non-stream on the same source and then stream again, the bug breaks playback on the third playback
			   and only one buffer plays.
			   The workaround seems to be to always unqueue buffers regardless of whether the source is streamed or not. 
			   And then avoid calling (source, AL_BUFFER, AL_NONE)
			   From past experience, I know it is a bad idea to try to unqueue buffers from a non-streamed source (which is the contrary to OpenAL part),
			   but this seems to work for this bug.
			 */
			ALint buffers_processed;
			/* Crap. Another iOS 5.0 problem. It seems our loop workaround below can get in cases where the buffer never clears.
			 * It appears that in some cases (probably predecoded, but not always which makes it hard), that the buffer never can be unqueued.
			 * I think it may possibly happen if you never use a source for streaming, and it is also possible it happens after loading or using a certain number of sources.
			 * So to workaround, we need to abort after a certain amount of time to prevent an infinite loop check.
			 * Some testing on iPad 2, 122ms is the highest number I've seen so far. So maybe 200ms is the cap?
			 */
			ALuint timeout_counter = ALmixer_GetTicks();
			const ALuint MAX_NUMBER_OF_TICKS_TO_WAIT_IN_WORKAROUND = 200;

			/* Wow this is the bug that just keeps on sucking. There is even a race condition bug where the unqueue may not actually work. 
			 * So I have to keep doing it until it does.
			 */
			do
			{
				ALint temp_count;
				
				/* Wow this is the bug that just keeps on sucking. There is even a race condition bug where the unqueue may not actually work. 
				 * So I have to keep doing it until it does.
				 * Sleeping for 20ms seems to help. 10ms was not long enough. (iPad 2).
				 * Update: A user is hitting this in a tight loop and calling dozens/hundreds of times. The sleep some times seems to result in a stutter.
				 * My theory is that there is thread contention and sleeps may be queuing up. The workaround seems to be reduce the sleep or eliminate it.
				 * I prefer to reduce it because fundamentally I am waiting for the Core Audio/OpenAL thread to finish processing so yielding seems like a good idea.
				 * The flipside is that I hit a lot of OpenAL errors because the commands keep failing and I'm burning CPU unnecessarily.
				 * So far, the OpenAL errors seem harmless and there are no serious cascading failures I've encountered.
				 */
				ALmixer_Delay(0);

				alGetSourcei(
					source_id,
					AL_BUFFERS_PROCESSED, &buffers_processed
				);
				if((error = alGetError()) != AL_NO_ERROR)
				{
					fprintf(stderr, "17aTesting Error with buffers_processed on Halt. (You may be seeing this because of a bad Apple OpenAL iOS 5.0 regression bug): %s",
						alGetString(error));
					ALmixer_SetError("Failed detecting still processed buffers: %s",
						alGetString(error) );
					/* This whole iOS 5.0 bug is so messed up that returning an error value probably isn't helpful.
					retval = AL_FALSE;
					*/
				}
				/*
				fprintf(stderr, "Going to unqueue %d buffers\n", buffers_processed);
				 */
				for(temp_count=0; temp_count<buffers_processed; temp_count++)
				{
					ALuint unqueued_buffer_id;

					alSourceUnqueueBuffers(
						source_id,
						1, &unqueued_buffer_id
					);
					if((error = alGetError()) != AL_NO_ERROR)
					{
						/* Disabling this print because we hit it way too much with the usleep(0) */
/*						fprintf(stderr, "17bTesting error with unqueuing buffers on Halt (You may be seeing this because of a bad Apple OpenAL iOS 5.0 regression bug): %s\n",	alGetString(error));
 */
						/* This whole iOS 5.0 bug is so messed up that returning an error value probably isn't helpful.
						 retval = AL_FALSE;
						 */
					}
				}
				
				alGetSourcei(
							 source_id,
							 AL_BUFFERS_PROCESSED, &buffers_processed
							 );
				if((error = alGetError()) != AL_NO_ERROR)
				{
					fprintf(stderr, "17cTesting Error with buffers_processed on Halt. (You may be seeing this because of a bad Apple OpenAL iOS 5.0 regression bug): %s", alGetString(error));
					ALmixer_SetError("Failed detecting still processed buffers: %s", alGetString(error) );
					/* This whole iOS 5.0 bug is so messed up that returning an error value probably isn't helpful.
					 retval = AL_FALSE;
					 */
				}
				/*
				fprintf(stderr, "unqueued buffers should be 0. Actual value is %d\n", buffers_processed);
				*/
				/* Wow this is the bug that just keeps on sucking. There is an additional race condition bug where the unqueue may not actually work. 
				 * So I have to keep doing it until it does.
				 * I hope this doesn't infinite loop.
				 */
				/* Disabling this print because we hit it way too much with the usleep(0) */
				/*
				if(0 != buffers_processed)
				{
					fprintf(stderr, "Evil Apple OpenAL iOS 5.0 race condition. Buffers didn't actually unqueue. Repeating unqueue loop. %d, %d %d\n", ALmixer_GetTicks(), timeout_counter, ALmixer_GetTicks()-timeout_counter);
				}
				*/
			} while(0 != buffers_processed && ( (ALmixer_GetTicks()-timeout_counter) < MAX_NUMBER_OF_TICKS_TO_WAIT_IN_WORKAROUND) );

			/* Avoid calling the normal cleanup because part of this bug seems to be triggered by alSourcei(source_id, AL_BUFFER, AL_NONE); */
			return retval;
		}
	
	#endif
#endif /* iOS 5.0 workaround */

		/* According to the spec, this is the best way to clear a source. 
		 * This is supposed to work for both streamed and non-streamed sources. 
		 */
		alSourcei(source_id, AL_BUFFER, AL_NONE);
		if((error = alGetError()) != AL_NO_ERROR)
		{
			fprintf(stderr, "17dTesting Error with clearing buffer from source: %s",
				alGetString(error));
			ALmixer_SetError("Failed to clear buffer from source: %s",
				alGetString(error) );
			retval = AL_FALSE;
		}

		return retval;
}

/* Will return the number of channels halted
 * or 0 for error
 */
static ALint Internal_HaltChannel(ALint channel, ALboolean did_finish_naturally)
{
	ALint retval = 0;
	ALint counter = 0;
	ALenum error;
	ALboolean clear_succeeded;

	if(channel >= Number_of_Channels_global)
	{
		ALmixer_SetError("Cannot halt channel %d because it exceeds maximum number of channels (%d)\n", channel, Number_of_Channels_global);
		return -1;
	}
	/* If the user specified a specific channel */
	if(channel >= 0)
	{
		/* only need to process channel if in use */
		if(ALmixer_Channel_List[channel].channel_in_use)
		{
			alSourceStop(ALmixer_Channel_List[channel].alsource);
			if((error = alGetError()) != AL_NO_ERROR)
			{
				fprintf(stderr, "14Testing error: %s\n",
					alGetString(error));				
			}

			clear_succeeded = Internal_DetachBuffersFromSource(ALmixer_Channel_List[channel].alsource, ALmixer_Channel_List[channel].almixer_data->decoded_all);
			if(AL_FALSE == clear_succeeded)
			{
				retval = -1;
			}
			

			ALmixer_Channel_List[channel].almixer_data->num_buffers_in_use  = 0;

			/* Launch callback for consistency? */
			Invoke_Channel_Done_Callback(channel, did_finish_naturally);

			Clean_Channel(channel);
			Is_Playing_global--;
			counter++;
		}
	}
	/* The user wants to halt all channels */
	else
	{
		ALint i;
		for(i=0; i<Number_of_Channels_global; i++)
		{
			/* only need to process channel if in use */
			if(ALmixer_Channel_List[i].channel_in_use)
			{
				alSourceStop(ALmixer_Channel_List[i].alsource);
				if((error = alGetError()) != AL_NO_ERROR)
				{
					fprintf(stderr, "19Testing error: %s\n",
						alGetString(error));				
				}

				clear_succeeded = Internal_DetachBuffersFromSource(ALmixer_Channel_List[i].alsource, ALmixer_Channel_List[i].almixer_data->decoded_all);
				if(AL_FALSE == clear_succeeded)
				{
					retval = -1;
				}
			
				ALmixer_Channel_List[i].almixer_data->num_buffers_in_use  = 0;

				/* Launch callback for consistency? */
				Invoke_Channel_Done_Callback(i, did_finish_naturally);

				Clean_Channel(i);
				Is_Playing_global--;

				/* Increment the counter */
				counter++;
			}
			/* Let's halt everything just in case there 
			 * are bugs.
			 */
			/*
			else
			{
				alSourceStop(ALmixer_Channel_List[channel].alsource);
				/ * Can't clean because the in_use counter for
				 * data will get messed up * /
				Clean_Channel(channel);
			}
			*/
			/* Just in case */
			Is_Playing_global = 0;
		}
	}
	if(-1 == retval)
	{		
		return -1;	
	}
	return counter;
}


/* Will return the source halted or the total number of channels
 * if all were halted or 0 for error
 */
static ALint Internal_HaltSource(ALuint source, ALboolean did_finish_naturally)
{
	ALint channel;
	if(0 == source)
	{
		/* Will return the number of sources halted */
		return Internal_HaltChannel(-1, did_finish_naturally);
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot halt source: %s", ALmixer_GetError());
		return -1;
	}
	return Internal_HaltChannel(channel, did_finish_naturally);
}



/* Note: Behaves, almost like SDL_mixer, but keep in mind
 * that there is no "music" channel anymore, so 0
 * will remove everything. (Note, I no longer allow 0
 * so it gets set to the default number.)
 * Also, callbacks for deleted channels will not be called.
 * I really need to do error checking, for realloc and 
 * GenSources, but reversing the damage is too painful
 * for me to think about at the moment, so it's not in here.
 */
static ALint Internal_AllocateChannels(ALint numchans)
{
	ALenum error;
	int i;
	/* Return info */
	if(numchans < 0)
	{
		return Number_of_Channels_global;
	}
	if(0 == numchans)
	{
		numchans = ALMIXER_DEFAULT_NUM_CHANNELS;
	}
	/* No change */
	if(numchans == Number_of_Channels_global)
	{
		return Number_of_Channels_global;
	}
	/* We need to increase the number of channels */
	if(numchans > Number_of_Channels_global)
	{
		/* Not sure how safe this is, but SDL_mixer does it
		 * the same way */
		ALmixer_Channel_List = (struct ALmixer_Channel*) realloc( ALmixer_Channel_List, numchans * sizeof(struct ALmixer_Channel));

		/* Allocate memory for the list of sources that map to the channels */
		Source_Map_List = (Source_Map*) realloc(Source_Map_List, numchans * sizeof(Source_Map));

		for(i=Number_of_Channels_global; i<numchans; i++)
		{
			Init_Channel(i);
			/* Generate a new source and associate it with the channel */
			_alGenSources(1, &ALmixer_Channel_List[i].alsource);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "12Testing errpr before unqueue because getting stuff, for OS X this is expected: %s\n",
			alGetString(error));				
	}
			/* Copy the source so the SourceMap has it too */
			Source_Map_List[i].source = ALmixer_Channel_List[i].alsource;
			Source_Map_List[i].channel = i;
			/* Clean the channel because there are some things that need to 
			 * be done that can't happen until the source is set
			 */
			Clean_Channel(i);
		}

		/* The Source_Map_List must be sorted by source for binary searches
		 */
		qsort(Source_Map_List, numchans, sizeof(Source_Map), Compare_Source_Map);
	
		Number_of_Channels_global = numchans;
		return numchans;
	}
	/* Need to remove channels. This might be dangerous */
	if(numchans < Number_of_Channels_global)
	{
		for(i=numchans; i<Number_of_Channels_global; i++)
		{
			/* Halt the channel */
			Internal_HaltChannel(i, AL_FALSE);

			/* Delete source associated with the channel */
			_alDeleteSources(1, &ALmixer_Channel_List[i].alsource);
		}


		/* Not sure how safe this is, but SDL_mixer does it
		 * the same way */
		ALmixer_Channel_List = (struct ALmixer_Channel*) realloc( ALmixer_Channel_List, numchans * sizeof(struct ALmixer_Channel));

		/* The tricky part is that we must remove the entries
		 * in the source map that correspond to the deleted channels.
		 * We'll resort the map by channels so we can pick them off
		 * in order.
		 */
		qsort(Source_Map_List, Number_of_Channels_global, sizeof(Source_Map), Compare_Source_Map_by_channel);

		/* Deallocate memory for the list of sources that map to the channels */
		Source_Map_List = (Source_Map*) realloc(Source_Map_List, numchans * sizeof(Source_Map));

		/* Now resort the map by source and the correct num of chans */
		qsort(Source_Map_List, numchans, sizeof(Source_Map), Compare_Source_Map);

		/* Reset the number of channels */
		Number_of_Channels_global = numchans;
		return numchans;
	}
	/* Shouldn't ever reach here */
	return -1;
	
}
	
static ALint Internal_ReserveChannels(ALint num)
{
	/* Can't reserve more than the max num of channels */
	/* Actually, I'll allow it for people who just want to
	 * set the value really high to effectively disable
	 * auto-assignment
	 */

	/* Return the current number of reserved channels */
	if(num < 0)
	{
		return Number_of_Reserve_Channels_global;
	}
	Number_of_Reserve_Channels_global = num;
	return Number_of_Reserve_Channels_global;
}
	

/* This will rewind the SDL_Sound sample for streamed
 * samples and start buffering up the data for the next
 * playback. This may require samples to be halted
 */
static ALboolean Internal_RewindData(ALmixer_Data* data)
{
	ALint retval = 0;
	/*
	ALint bytes_returned;
	ALint i;
	*/
	if(NULL == data)
	{
		ALmixer_SetError("Cannot rewind because data is NULL\n");
		return AL_FALSE;
	}


	/* Might have to require Halt */
	/* Okay, we assume Halt or natural stop has already
	 * cleared the data buffers
	 */
	if(data->in_use)
	{
		/*
		fprintf(stderr, "Warning sample is in use. May not be able to rewind\n");
		*/
		/*
		ALmixer_SetError("Data is in use. Cannot rewind unless all sources using the data are halted\n");
		return -1;
		*/
	}
		
	
	/* Because Seek can alter things even in predecoded data,
	 * decoded data must also be rewound 
	 */
	if(data->decoded_all)
	{
		data->eof = 0;

#if 0
#if defined(DISABLE_PREDECODED_SEEK)
		/* Since we can't seek predecoded stuff, it should be rewound */
		return AL_TRUE;
#elif !defined(DISABLE_SEEK_MEMORY_OPTIMIZATION)
		/* This case is if the Sound_Sample has been deleted.
		 * It assumes the data is already at the beginning.
		 */
		if(NULL == data->sample)
		{
			return AL_TRUE;
		}
		/* Else, the sample has already been reallocated,
		 * and we can fall to normal behavior
		 */
#endif
#endif
		/* If access_data, was enabled, the sound sample 
		 * still exists and we can do stuff. 
		 * If it's NULL, we can't do anything, but
		 * it should already be "rewound".
		 */
		if(NULL == data->sample)
		{
			return AL_TRUE;
		}
		/* Else, the sample has already been reallocated,
		 * and we can fall to normal behavior
		 */

		Set_Predecoded_Seek_Position(data, 0);
	/*
		return data->total_bytes;
	*/
		return AL_TRUE;
	}
	
	/* Remaining stuff for streamed data */
	
	data->eof = 0;
	retval = Sound_Rewind(data->sample);
	if(0 == retval)
	{
		ALmixer_SetError( Sound_GetError() );
		return AL_FALSE;
	}
#if 0
	/* Clear error */
	alGetError();
	for(i=0; i<data->num_buffers; i++)
	{
		bytes_returned = GetMoreData(data, data->buffer[i]);
		if(-1 == bytes_returned)
		{
			return AL_FALSE;
		}
		else if(0 == bytes_returned)
		{
			return AL_FALSE;
		}
		retval += bytes_returned;
		
	}
#endif

	
	
	return AL_TRUE;
}




static ALint Internal_RewindChannel(ALint channel)
{
	ALint retval = 0;
	ALenum error;
	ALint state;
	ALint running_count = 0;

	if(channel >= Number_of_Channels_global)
	{
		ALmixer_SetError("Cannot rewind channel %d because it exceeds maximum number of channels (%d)\n", channel, Number_of_Channels_global);
		return -1;
	}

	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "24Testing error: %s\n",
			alGetString(error));				
	}
	/* Clear error */
	alGetError();
	
	/* If the user specified a specific channel */
	if(channel >= 0)
	{
		/* only need to process channel if in use */
		if(ALmixer_Channel_List[channel].channel_in_use)
		{
			running_count = 1;
			/* What should I do? Do I just rewind the channel
			 * or also rewind the data? Since the data is
			 * shared, let's make it the user's responsibility
			 * to rewind the data.
			 */
			if(ALmixer_Channel_List[channel].almixer_data->decoded_all)
			{
				alGetSourcei(
					ALmixer_Channel_List[channel].alsource,
					AL_SOURCE_STATE, &state
				);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "25Testing error: %s\n",
			alGetString(error));				
	}
				alSourceRewind(ALmixer_Channel_List[channel].alsource);
				if((error = alGetError()) != AL_NO_ERROR)
				{
					ALmixer_SetError("Internal_RewindChannel: alSourceRewind(%d): %s", channel, alGetString(error) );
					retval = -1;
				}
				/* Need to resume playback if it was originally playing */
				if(AL_PLAYING == state)
				{
					alSourcePlay(ALmixer_Channel_List[channel].alsource);
					if((error = alGetError()) != AL_NO_ERROR)
					{
						ALmixer_SetError("Internal_RewindChannel: alSourcePlay(%d): %s", channel, alGetString(error) );
						retval = -1;
					}
				}
				else if(AL_PAUSED == state)
				{
					/* HACK: The problem is that when paused, after
					 * the Rewind, I can't get it off the INITIAL
					 * state without restarting
					 */
					alSourcePlay(ALmixer_Channel_List[channel].alsource);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "25Testing error: %s\n",
			alGetString(error));				
	}
					alSourcePause(ALmixer_Channel_List[channel].alsource);
					if((error = alGetError()) != AL_NO_ERROR)
					{
						ALmixer_SetError("Internal_RewindChannel: alSourcePause(%d): %s", channel, alGetString(error) );
						retval = -1;
					}
				}
			}
			else
			{
				/* Streamed data is different. Rewinding the channel
				 * does no good. Rewinding the data will have an
				 * effect, but it will be lagged based on how
				 * much data is queued. Recommend users call Halt
				 * before rewind if they want immediate results.
				 */
				if(AL_FALSE == Internal_RewindData(ALmixer_Channel_List[channel].almixer_data))
				{
					retval = -1;
				}
			}
		}
	}
	/* The user wants to rewind all channels */
	else
	{
		ALint i;
		for(i=0; i<Number_of_Channels_global; i++)
		{
			/* only need to process channel if in use */
			if(ALmixer_Channel_List[i].channel_in_use)
			{
				running_count++;
				/* What should I do? Do I just rewind the channel
				 * or also rewind the data? Since the data is
				 * shared, let's make it the user's responsibility
				 * to rewind the data.
				 */
				if(ALmixer_Channel_List[i].almixer_data->decoded_all)
				{
					alGetSourcei(
						ALmixer_Channel_List[i].alsource,
						AL_SOURCE_STATE, &state
					);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "26Testing error: %s\n",
			alGetString(error));				
	}
					alSourceRewind(ALmixer_Channel_List[i].alsource);
					if((error = alGetError()) != AL_NO_ERROR)
					{
						ALmixer_SetError("Internal_RewindChannel: alSourceRewind(all): %s", alGetString(error) );
						retval = -1;
					}
					/* Need to resume playback if it was originally playing */
					if(AL_PLAYING == state)
					{
						alSourcePlay(ALmixer_Channel_List[i].alsource);
						if((error = alGetError()) != AL_NO_ERROR)
						{
							ALmixer_SetError("Internal_RewindChannel: alSourceRewind(all): %s", alGetString(error) );
							retval = -1;
						}
					}
					else if(AL_PAUSED == state)
					{
						/* HACK: The problem is that when paused, after
						 * the Rewind, I can't get it off the INITIAL
						 * state without restarting
						 */
						alSourcePlay(ALmixer_Channel_List[i].alsource);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "27Testing error: %s\n",
			alGetString(error));				
	}
						alSourcePause(ALmixer_Channel_List[i].alsource);
						if((error = alGetError()) != AL_NO_ERROR)
						{
							ALmixer_SetError("Internal_RewindChannel: alSourceRewind(all): %s", alGetString(error) );
							retval = -1;
						}
					}
				}
				else
				{
					/* Streamed data is different. Rewinding the channel
					 * does no good. Rewinding the data will have an
					 * effect, but it will be lagged based on how
					 * much data is queued. Recommend users call Halt
					 * before rewind if they want immediate results.
					 */
					if(AL_FALSE == Internal_RewindData(ALmixer_Channel_List[i].almixer_data))
					{
						retval = -1;
					}
				}
			}
		}
	}
	if(-1 == retval)
	{
		return -1;
	}
	else
	{
		return running_count;
	}

}


static ALint Internal_RewindSource(ALuint source)
{
	ALint channel;
	if(0 == source)
	{
		return Internal_RewindChannel(-1);
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot rewind source: %s", ALmixer_GetError());
		return 0;
	}
	return Internal_RewindChannel(channel);
}





static ALint Internal_PlayChannelTimed(ALint channel, ALmixer_Data* data, ALint loops, ALint ticks)
{
	ALenum error;
	int ret_flag = 0;
	if(NULL == data)
	{
		ALmixer_SetError("Can't play because data is NULL\n");
		return -1;
	}

	/* There isn't a good way to share streamed files because
	 * the decoded data doesn't stick around. 
	 * You must "Load" a brand new instance of
	 * the data. If you try using the same data,
	 * bad things may happen. This check will attempt
	 * to prevent sharing
	 */
	if(0 == data->decoded_all)
	{
		if(data->in_use)
		{
			ALmixer_SetError("Can't play shared streamed sample because it is already in use");
			return -1;
		}

		/* Make sure SDL_sound sample is not at EOF.
		 * This mainly affects streamed files,
	 	 * so the check is placed here
		 */
		if(data->eof)
		{
			if( -1 == Internal_RewindData(data) )
			{
				ALmixer_SetError("Can't play sample because it is at EOF and cannot rewind");
				return -1;
			}
		}
	}
	/* We need to provide the user with the first available channel */
	if(-1 == channel)
	{
		ALint i;
		for(i=Number_of_Reserve_Channels_global; i<Number_of_Channels_global; i++)
		{
			if(0 == ALmixer_Channel_List[i].channel_in_use)
			{
				channel = i;
				break;
			}
		}
		/* if we couldn't find a channel, return an error */
		if(i == Number_of_Channels_global)
		{
			ALmixer_SetError("No channels available for playing");
			return -1;
		}
	}
	/* If we didn't assign the channel number, make sure it's not
	 * out of bounds or in use */
	else
	{
		if(channel >= Number_of_Channels_global)
		{
			ALmixer_SetError("Requested channel (%d) exceeds maximum channel (%d) because only %d channels are allocated", channel, Number_of_Channels_global-1, Number_of_Channels_global);
			return -1;
		}
		else if(ALmixer_Channel_List[channel].channel_in_use)
		{
			ALmixer_SetError("Requested channel (%d) is in use", channel, Number_of_Channels_global-1, Number_of_Channels_global);
			return -1;
		}	
	}
	/* Make sure the user doesn't enter some meaningless value */
	if(loops < -1)
	{
		loops = -1;
	}
							
	/* loops will probably have to change to be controlled by SDL_Sound */

	/* Set up the initial values for playing */
	ALmixer_Channel_List[channel].channel_in_use = 1;
	data->in_use++;
		
	/* Shouldn't need updating until a callback is fired
	 * (assuming that we call Play in this function 
	 */
	ALmixer_Channel_List[channel].needs_stream = 0;
	ALmixer_Channel_List[channel].almixer_data = data;
	ALmixer_Channel_List[channel].start_time = ALmixer_GetTicks();

	/* If user entered -1 (or less), set to -1 */
	if(ticks < 0)
	{
		ALmixer_Channel_List[channel].expire_ticks = -1;
	}
	else
	{
		ALmixer_Channel_List[channel].expire_ticks = ticks;
	}

	
	ALmixer_Channel_List[channel].halted = 0;
	ALmixer_Channel_List[channel].paused = 0;

	/* Ran just use OpenAL to control loops if predecoded and infinite */
	ALmixer_Channel_List[channel].loops = loops; 
	if( (-1 == loops) && (data->decoded_all) )
	{
		alSourcei(ALmixer_Channel_List[channel].alsource, AL_LOOPING, AL_TRUE);
	}
	else
	{
		alSourcei(ALmixer_Channel_List[channel].alsource, AL_LOOPING, AL_FALSE);
	}
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "13Testing error: %s\n",
			alGetString(error));				
	}

#if 0
	/* Because of the corner case, predecoded
	 * files must add +1 to the loops.
	 * Streams do not have this problem
	 * because they can use the eof flag to 
	 * avoid the conflict.
	 * Sharing data chunks prevents the use of the eof flag.
	 * Since streams, cannot share, only predecoded
	 * files are affected 
	 */
	if(data->decoded_all)
	{
		/* Corner Case: Now that play calls are pushed
		 * off to update(), the start call must
		 * also come through here. So, start loops
		 * must be +1 
		 */
		if(-1 == loops)
		{
			/* -1 is a special case, and you don't want
			 * to add +1 to it */
			ALmixer_Channel_List[channel].loops = -1;
			alSourcei(ALmixer_Channel_List[channel].alsource, AL_LOOPING, AL_TRUE);
		}
		else
		{
			ALmixer_Channel_List[channel].loops = loops+1;
			alSourcei(ALmixer_Channel_List[channel].alsource, AL_LOOPING, AL_FALSE);
		}
	}
	else
	{
		ALmixer_Channel_List[channel].loops = loops;
		/* Can we really loop on streamed data? */
		alSourcei(ALmixer_Channel_List[channel].alsource, AL_LOOPING, AL_TRUE);
	}
#endif

	/* Should I start playing here or pass the buck to update? */
	/* Unlike SDL_SoundMixer, I think I'll do it here because
	 * this library isn't a *total* hack and OpenAL has more 
	 * built in functionality I need, so less needs to be 
	 * controlled and directed through the update function.
	 * The downside is less functionality is centralized.
	 * The upside is that the update function should be
	 * easier to maintain.
	 */

	/* Clear the error flag */
	alGetError();
	if(data->decoded_all)
	{
		/* Bind the data to the source */
		alSourcei(
			ALmixer_Channel_List[channel].alsource, 
			AL_BUFFER, 
			data->buffer[0]);
		if((error = alGetError()) != AL_NO_ERROR)
		{
			ALmixer_SetError("Could not bind data to source: %s",
				alGetString(error) );
			Clean_Channel(channel);
			return -1;
		}

		/* Make data available if access_data is enabled */
		Invoke_Predecoded_Channel_Data_Callback(channel, data);
	}
	else
	{
		/* Need to use the streaming buffer for binding */

		ALuint bytes_returned;
		ALuint j;
		data->num_buffers_in_use=0;
/****** MODIFICATION must go here *********/
		/* Since buffer queuing is pushed off until here to 
		 * avoid buffer conflicts, we must start reading 
		 * data here. First we make sure we have at least one
		 * packet. Then we queue up until we hit our limit.
		 */
		bytes_returned = GetMoreData(
			data,
			data->buffer[0]);
		if(0 == bytes_returned)
		{
			/* No data or error */
			ALmixer_SetError("Could not get data for streamed PlayChannel: %s", ALmixer_GetError());
			Clean_Channel(channel);
			return -1;
		}
		/* Increment the number of buffers in use */
		data->num_buffers_in_use++;
		

		/* Now we need to fill up the rest of the buffers.
		 * There is a corner case where we run out of data
		 * before the last buffer is filled.
		 * Stop conditions are we run out of 
		 * data or we max out our preload buffers.
		 */
			
		/*
	fprintf(stderr, "Filling buffer #%d (AL id is %d)\n", 0, data->buffer[0]);
	*/
		for(j=1; j<data->num_startup_buffers; j++)
		{
	/*
	fprintf(stderr, "Filling buffer #%d (AL id is %d)\n", j, data->buffer[j]);
	fprintf(stderr, ">>>>>>>>>>>>>>>>>>HACK for GetMoreData2\n");
	*/
		bytes_returned = GetMoreData(
				data,
				data->buffer[j]);
		/* 
		 * This might be a problem. I made a mistake with the types. I accidentally
		 * made the bytes returned an ALint and returned -1 on error.
		 * Bytes returned should be a ALuint, so now I no longer have a -1 case
		 * to check. I hope I didn't break anything here
		 */
		#if 0
			if(bytes_returned < 0)
			{
				/* Error found */
				ALmixer_SetError("Could not get data for additional startup buffers for PlayChannel: %s", ALmixer_GetError());
				/* We'll continue on because we do have some valid data */
				ret_flag = -1;
				break;
			}
			else if(0 == bytes_returned)
		#endif
			if(0 == bytes_returned)
			{
				/* No more data to buffer */
				/* Check for loops */
				if( ALmixer_Channel_List[channel].loops != 0 )
				{
					/*
					fprintf(stderr, "Need to rewind. In RAMPUP, handling loop\n");
					*/
					if(0 == Sound_Rewind(data->sample))
					{
fprintf(stderr, "error in rewind\n"); 
						ALmixer_SetError( Sound_GetError() );
						ALmixer_Channel_List[channel].loops = 0;
						ret_flag = -1;
						/* We'll continue on because we do have some valid data */
						break;
					}
					/* Remember to reset the data->eof flag */
					data->eof = 0;
					if(ALmixer_Channel_List[channel].loops > 0)
					{
						ALmixer_Channel_List[channel].loops--;
						/*
						fprintf(stderr, "Inside 000 >>>>>>>>>>Loops=%d\n", ALmixer_Channel_List[channel].loops); 
						*/
					}
					/* Would like to redo the loop, but due to 
					 * Sound_Rewind() bugs, we would risk falling 
					 * into an infinite loop
					 */
					bytes_returned = GetMoreData(
						data,
						data->buffer[j]);
					if(bytes_returned <= 0)
					{
						ALmixer_SetError("Could not get data: %s", ALmixer_GetError());
						/* We'll continue on because we do have some valid data */
						ret_flag = -1;
						break;
					}
				}
				else
				{
					/* No loops to do so quit here */
					break;
				}
			}
			/* Increment the number of buffers in use */
			data->num_buffers_in_use++;
		}
		/*
		fprintf(stderr, "In PlayChannel, about to queue: source=%d, num_buffers_in_use=%d\n",
				ALmixer_Channel_List[channel].alsource, 
				data->num_buffers_in_use);
		*/
		
		alSourceQueueBuffers(
			ALmixer_Channel_List[channel].alsource, 
			data->num_buffers_in_use, 
			data->buffer);
		if((error = alGetError()) != AL_NO_ERROR)
		{
			ALmixer_SetError("Could not bind data to source: %s",
				alGetString(error) );
			Clean_Channel(channel);
			return -1;
		}
		/* This is part of the hideous Nvidia workaround. In order to figure out
		 * which buffer to show during callbacks (for things like
		 * o-scopes), I must keep a copy of the buffers that are queued in my own
		 * data structure. This code will be called only if
		 * "access_data" was set, indicated by whether the queue is NULL.
		 */
		if(data->circular_buffer_queue != NULL)
		{
			ALuint k;
			ALuint queue_ret_flag;
			for(k=0; k<data->num_buffers_in_use; k++)
			{
/*				fprintf(stderr, "56c: CircularQueue_PushBack.\n"); */
				queue_ret_flag = CircularQueueUnsignedInt_PushBack(data->circular_buffer_queue, data->buffer[k]);
				if(0 == queue_ret_flag)
				{
					fprintf(stderr, "Serious internal error: CircularQueue could not push into queue.\n");
					ALmixer_SetError("Serious internal error: CircularQueue failed to push into queue");
				}
				/*
				else
				{
					fprintf(stderr, "Queue in PlayTimed\n");
					CircularQueueUnsignedInt_Print(data->circular_buffer_queue);
				}
				 */
			}
		}
				
		
/****** END **********/
	}
	/* We have finished loading the data (predecoded or queued)
	 * so now we can play 
	 */
	alSourcePlay(ALmixer_Channel_List[channel].alsource);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		ALmixer_SetError("Play failed: %s",
			alGetString(error) );
		Clean_Channel(channel);
		return -1;
	}

	/* Add to the counter that something is playing */
	Is_Playing_global++;
	if(-1 == ret_flag)
	{
			fprintf(stderr, "BACKDOOR ERROR >>>>>>>>>>>>>>>>>>\n");
		return -1;
	}
	return channel;
}


/* In case the user wants to specify a source instead of a channel,
 * they may use this function. This function will look up the 
 * source-to-channel map, and convert the call into a
 * PlayChannelTimed() function call.
 * Returns the channel it's being played on.
 * Note: If you are prefer this method, then you need to be careful
 * about using PlayChannel, particularly if you request the
 * first available channels because source and channels have 
 * a one-to-one mapping in this API. It is quite easy for 
 * a channel/source to already be in use because of this.
 * In this event, an error message will be returned to you.
 */
static ALuint Internal_PlaySourceTimed(ALuint source, ALmixer_Data* data, ALint loops, ALint ticks)
{
	ALint channel;
	ALint retval;
	if(0 == source)
	{
		retval = Internal_PlayChannelTimed(-1, data, loops, ticks);
		if(-1 == retval)
		{
			return 0;
		}
		else
		{
			return Internal_GetSource(retval);
		}
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot Play source: %s", ALmixer_GetError());
		return 0;
	}
	retval = Internal_PlayChannelTimed(channel, data, loops, ticks);
	if(-1 == retval)
	{
		return 0;
	}
	else
	{
		return source;
	}
	/* make compiler happy */
	return 0;
}




/* Returns the channel or number of channels actually paused */

static ALint Internal_PauseChannel(ALint channel)
{
	ALenum error;
	ALint state;
	ALint retval = 0;
	ALint counter = 0;
	
	if(channel >= Number_of_Channels_global)
	{
		ALmixer_SetError("Cannot pause channel %d because it exceeds maximum number of channels (%d)\n", channel, Number_of_Channels_global);
		return -1;
	}

	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "28Testing error: %s\n",
			alGetString(error));				
	}
	/* Clear error */
	alGetError();
	
	/* If the user specified a specific channel */
	if(channel >= 0)
	{
		/* only need to process channel if in use */
		if(ALmixer_Channel_List[channel].channel_in_use)
		{
			/* We don't want to repause if already
			 * paused because the fadeout/expire
			 * timing will get messed up
			 */
			alGetSourcei(
				ALmixer_Channel_List[channel].alsource,
				AL_SOURCE_STATE, &state
			);
			if((error = alGetError()) != AL_NO_ERROR)
			{
				fprintf(stderr, "Internal_PauseChannel specific channel error: %s\n",
				alGetString(error));				
			}
			if(AL_PLAYING == state)
			{
				/* Count the actual number of channels being paused */
				counter++;
				
				alSourcePause(ALmixer_Channel_List[channel].alsource);
				if((error = alGetError()) != AL_NO_ERROR)
				{
					ALmixer_SetError("Internal_PauseChannel: alSourcePause(%d): %s", channel, alGetString(error) );
					retval = -1;
				}
				/* We need to pause the expire time count down */
				if(ALmixer_Channel_List[channel].expire_ticks != -1)
				{
					ALuint current_time = ALmixer_GetTicks();
					ALuint diff_time;
					diff_time = current_time - 
						ALmixer_Channel_List[channel].start_time;
					/* When we unpause, we will want to reset
					 * the start time so we can continue
					 * to base calculations off GetTicks().
					 * This means we need to subtract the amount
					 * of time already used up from expire_ticks.
					 */
					ALmixer_Channel_List[channel].expire_ticks =
						ALmixer_Channel_List[channel].expire_ticks -
						diff_time;
					/* Because -1 is a special value, we can't 
					 * allow the time to go negative
					 */
					if(ALmixer_Channel_List[channel].expire_ticks < 0)
					{
						ALmixer_Channel_List[channel].expire_ticks = 0;
					}
				}
				/* Do the same as expire time for fading */
				if(ALmixer_Channel_List[channel].fade_enabled)
				{
					ALuint current_time = ALmixer_GetTicks();
					ALuint used_up_time;
					used_up_time = current_time - 
						ALmixer_Channel_List[channel].fade_start_time;
					/* When we unpause, we will want to reset
					 * the start time so we can continue
					 * to base calculations off GetTicks().
					 * I originally did this wrong. I computed the amount of time remaining
					 * and when I resumed, I grabbed the current time as the start and used the time remaining as the end.
					 * I think this is wrong because the volume curve may not be linear.
					 * Instead, I will try saving the time remaining, but preserve the original fade duration.
					 * Then I will adjust the start time for the amount already played.
					 * So I don't need to create another variable, I will overwrite the start time with the remaining time.
					 * (Resume needs to know this implementation detail.)
					 */
					ALmixer_Channel_List[channel].fade_start_time = used_up_time;
				} /* End fade check */
			} /* End if PLAYING */
		} /* End If in use */
	} /* End specific channel */
	/* The user wants to halt all channels */
	else
	{
		ALint i;
		for(i=0; i<Number_of_Channels_global; i++)
		{
			/* only need to process channel if in use */
			if(ALmixer_Channel_List[i].channel_in_use)
			{
				/* We don't want to repause if already
				 * paused because the fadeout/expire
				 * timing will get messed up
				 */
				alGetSourcei(
					ALmixer_Channel_List[i].alsource,
					AL_SOURCE_STATE, &state
				);
				if((error = alGetError()) != AL_NO_ERROR)
				{
					fprintf(stderr, "Internal_PauseChannel all channels error: %s\n",
					alGetString(error));				
				}
				if(AL_PLAYING == state)
				{
					/* Count the actual number of channels being paused */
					counter++;
						
					alSourcePause(ALmixer_Channel_List[i].alsource);
					if((error = alGetError()) != AL_NO_ERROR)
					{
						ALmixer_SetError("Internal_PauseChannel: alSourcePause(all): %s", alGetString(error) );
						retval = -1;
					}
					/* We need to pause the expire time count down */
					if(ALmixer_Channel_List[i].expire_ticks != -1)
					{
						ALuint current_time = ALmixer_GetTicks();
						ALuint diff_time;
						diff_time = current_time - 
							ALmixer_Channel_List[i].start_time;
						/* When we unpause, we will want to reset
						 * the start time so we can continue
						 * to base calculations off GetTicks().
						 * This means we need to subtract the amount
						 * of time already used up from expire_ticks.
						 */
						ALmixer_Channel_List[i].expire_ticks =
							ALmixer_Channel_List[i].expire_ticks -
							diff_time;
						/* Because -1 is a special value, we can't 
						 * allow the time to go negative
						 */
						if(ALmixer_Channel_List[i].expire_ticks < 0)
						{
							ALmixer_Channel_List[i].expire_ticks = 0;
						}
					}
					/* Do the same as expire time for fading */
					if(ALmixer_Channel_List[i].fade_enabled)
					{
						ALuint current_time = ALmixer_GetTicks();
						ALuint used_up_time;
						used_up_time = current_time - 
								ALmixer_Channel_List[i].fade_start_time;
						/* When we unpause, we will want to reset
						 * the start time so we can continue
						 * to base calculations off GetTicks().
						 * I originally did this wrong. I computed the amount of time remaining
						 * and when I resumed, I grabbed the current time as the start and used the time remaining as the end.
						 * I think this is wrong because the volume curve may not be linear.
						 * Instead, I will try saving the time remaining, but preserve the original fade duration.
						 * Then I will adjust the start time for the amount already played.
						 * So I don't need to create another variable, I will overwrite the start time with the remaining time.
						 * (Resume needs to know this implementation detail.)
						 */
						ALmixer_Channel_List[i].fade_start_time = used_up_time;
					} /* End fade check */	
				} /* End if PLAYING */
			} /* End channel in use */
		} /* End for-loop */
	}
	if(-1 == retval)
	{
		return -1;
	}
	return counter;	
}

/* Returns the channel or number of channels actually paused */
static ALint Internal_PauseSource(ALuint source)
{
	ALint channel;
	if(0 == source)
	{
		return Internal_PauseChannel(-1);
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot pause source: %s", ALmixer_GetError());
		return -1;
	}
	return Internal_PauseChannel(channel);
}



static ALint Internal_ResumeChannel(ALint channel)
{
	ALint state;
	ALenum error;
	ALint retval = 0;
	ALint counter = 0;
	
	if(channel >= Number_of_Channels_global)
	{
		ALmixer_SetError("Cannot pause channel %d because it exceeds maximum number of channels (%d)\n", channel, Number_of_Channels_global);
		return -1;
	}

	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "31Testing error: %s\n",
			alGetString(error));				
	}
	/* Clear error */
	alGetError();

	/* If the user specified a specific channel */
	if(channel >= 0)
	{
		/* only need to process channel if in use */
		if(ALmixer_Channel_List[channel].channel_in_use)
		{
			alGetSourcei(
				ALmixer_Channel_List[channel].alsource,
				AL_SOURCE_STATE, &state
			);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "32Testing error: %s\n",
			alGetString(error));				
	}
			if(AL_PAUSED == state)
			{
				/* Count the actual number of channels resumed */
				counter++;

				/* We need to resume the expire time count down */
				if(ALmixer_Channel_List[channel].expire_ticks != -1)
				{
					ALmixer_Channel_List[channel].start_time = ALmixer_GetTicks();
				}
				/* Do the same as expire time for fading */
				if(ALmixer_Channel_List[channel].fade_enabled)
				{
					/* I originally did this wrong. On pause, I computed the amount of time remaining
					 * and when I resumed, I grabbed the current time as the start and used the time remaining as the end.
					 * I think this is wrong because the volume curve may not be linear.
					 * Instead, I will try saving the time remaining, but preserve the original fade duration.
					 * Then I will adjust the start time for the amount already played.
					 * So I don't need to create another variable, I will overwrite the start time with the remaining time
					 * (Resume needs to know this implementation detail.)
					 */
					ALuint current_time = ALmixer_GetTicks();
					/* Recover the used_up_time from the overloaded fade_start_time variable */
					ALuint used_up_time = ALmixer_Channel_List[channel].fade_start_time;		
					/* So the adjusted start time is: */
					ALmixer_Channel_List[channel].fade_start_time = current_time - used_up_time;
				}

				alSourcePlay(ALmixer_Channel_List[channel].alsource);
				if((error = alGetError()) != AL_NO_ERROR)
				{
					ALmixer_SetError("Internal_ResumeChannel: alSourcePlay(%d): %s", channel, alGetString(error) );
					retval = -1;
				}
			}
		}
	}
	/* The user wants to halt all channels */
	else
	{
		ALint i;
		for(i=0; i<Number_of_Channels_global; i++)
		{
			/* only need to process channel if in use */
			if(ALmixer_Channel_List[i].channel_in_use)
			{
				alGetSourcei(
					ALmixer_Channel_List[i].alsource,
					AL_SOURCE_STATE, &state
				);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "33Testing error: %s\n",
			alGetString(error));				
	}
				if(AL_PAUSED == state)
				{
					/* Count the actual number of channels resumed */
					counter++;

					/* We need to resume the expire time count down */
					if(ALmixer_Channel_List[i].expire_ticks != -1)
					{
						ALmixer_Channel_List[i].start_time = ALmixer_GetTicks();
					}
					/* Do the same as expire time for fading */
					if(ALmixer_Channel_List[i].fade_enabled)
					{
						/* I originally did this wrong. On pause, I computed the amount of time remaining
						 * and when I resumed, I grabbed the current time as the start and used the time remaining as the end.
						 * I think this is wrong because the volume curve may not be linear.
						 * Instead, I will try saving the time remaining, but preserve the original fade duration.
						 * Then I will adjust the start time for the amount already played.
						 * So I don't need to create another variable, I will overwrite the start time with the remaining time
						 * (Resume needs to know this implementation detail.)
						 */
						ALuint current_time = ALmixer_GetTicks();
						/* Recover the used_up_time from the overloaded fade_start_time variable */
						ALuint used_up_time = ALmixer_Channel_List[i].fade_start_time;		
						/* So the adjusted start time is: */
						ALmixer_Channel_List[i].fade_start_time = current_time - used_up_time;
					}	
						
					alSourcePlay(ALmixer_Channel_List[i].alsource);
					if((error = alGetError()) != AL_NO_ERROR)
					{
						ALmixer_SetError("Internal_ResumeChannel: alSourcePlay(all): %s", alGetString(error) );
						retval = -1;
					}
				}
			}
		}
	}
	if(-1 == retval)
	{
		return -1;
	}
	return counter;	
}


static ALint Internal_ResumeSource(ALuint source)
{
	ALint channel;
	if(0 == source)
	{
		return Internal_ResumeChannel(-1);
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot resume source: %s", ALmixer_GetError());
		return -1;
	}
	return Internal_ResumeChannel(channel);
}


/* Might consider setting eof to 0 as a "feature"
 * This will allow seek to end to stay there because
 * Play automatically rewinds if at the end */
static ALboolean Internal_SeekData(ALmixer_Data* data, ALuint msec)
{
	ALint retval;
	
	if(NULL == data)
	{
		ALmixer_SetError("Cannot Seek because data is NULL");
		return AL_FALSE;
	}
	
	/* Seek for predecoded files involves moving the chunk pointer around */
	if(data->decoded_all)
	{
		ALuint byte_position;

		/* OpenAL doesn't seem to like it if I change the buffer
		 * while playing (crashes), so I must require that Seek only
		 * be done when the data is not in use. 
		 * Since data may be shared among multiple sources,
		 * I can't shut them down myself, so I have to return an error.
		 */
		if(data->in_use)
		{
			ALmixer_SetError("Cannot seek on predecoded data while instances are playing");
			return AL_FALSE;
		}
#if 0
#if defined(DISABLE_PREDECODED_SEEK)
		ALmixer_SetError("Seek support for predecoded samples was not compiled in");
		return AL_FALSE;

#elif !defined(DISABLE_SEEK_MEMORY_OPTIMIZATION)
		/* By default, ALmixer frees the Sound_Sample for predecoded
		 * samples because of the potential memory waste.
		 * However, to seek a sample, we need to have a full
		 * copy of the data around. So the strategy is to
		 * recreate a hackish Sound_Sample to be used for seeking
		 * purposes. If Sound_Sample is NULL, we will reallocate
		 * memory for it and then procede as if everything 
		 * was normal.
		 */
		if(NULL == data->sample)
		{
			if( -1 == Reconstruct_Sound_Sample(data) )
			{
				return AL_FALSE;
			}
		}
#endif
#endif
		/* If access_data was set, then we still have the 
		 * Sound_Sample and we can move around in the data.
		 * If it was not set, the data has been freed and we 
		 * cannot do anything because there is no way to 
		 * recover the data because OpenAL won't let us
		 * get access to the buffers
		 */
		if(NULL == data->sample)
		{
			ALmixer_SetError("Cannot seek because access_data flag was set false when data was initialized");
			return AL_FALSE;
		}
		
		byte_position = Convert_Msec_To_Byte_Pos(&data->sample->desired, msec);
		retval = Set_Predecoded_Seek_Position(data, byte_position);
		if(-1 == retval)
		{
			return AL_FALSE;
		}
		else
		{
			return AL_TRUE;
		}

	}
	else
	{
		/* Reset eof flag?? */
		data->eof = 0;
		retval = Sound_Seek(data->sample, msec);
		if(0 == retval)
		{
			ALmixer_SetError(Sound_GetError());
			
		fprintf(stderr, "Sound seek error: %s\n", ALmixer_GetError());
			/* Try rewinding to clean up? */
/*
			Internal_RewindData(data);
*/
			return AL_FALSE;
		}
		return AL_TRUE;
	}

	return AL_TRUE;
}			
		

static ALint Internal_SeekChannel(ALint channel, ALuint msec)
{
	ALint retval = 0;
	ALenum error;
	ALint state;
	ALint running_count = 0;
	
	if(0 == msec)
	{
		return Internal_RewindChannel(channel);
	}
	
	if(channel >= Number_of_Channels_global)
	{
		ALmixer_SetError("Cannot seek channel %d because it exceeds maximum number of channels (%d)\n", channel, Number_of_Channels_global);
		return -1;
	}
	
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "24Testing error: %s\n",
				alGetString(error));				
	}
	/* Clear error */
	alGetError();
	
	/* If the user specified a specific channel */
	if(channel >= 0)
	{
		/* only need to process channel if in use */
		if(ALmixer_Channel_List[channel].channel_in_use)
		{
			
			/* What should I do? Do I just rewind the channel
			 * or also rewind the data? Since the data is
			 * shared, let's make it the user's responsibility
			 * to rewind the data.
			 */
			if(ALmixer_Channel_List[channel].almixer_data->decoded_all)
			{
				/* convert milliseconds to seconds */
				ALfloat sec_offset = msec / 1000.0f;

				alGetSourcei(
							 ALmixer_Channel_List[channel].alsource,
							 AL_SOURCE_STATE, &state
							 );
				if((error = alGetError()) != AL_NO_ERROR)
				{
					fprintf(stderr, "25Testing error: %s\n",
							alGetString(error));				
				}
				/* OpenAL seek */
				alSourcef(ALmixer_Channel_List[channel].alsource, AL_SEC_OFFSET, sec_offset);
				if((error = alGetError()) != AL_NO_ERROR)
				{
					ALmixer_SetError("Internal_SeekChannel: alSourcef(%d): %s", channel, alGetString(error) );
					retval = -1;
				}
				/* OpenAL 1.1 spec says if this succeeds on a playing source, it will automatically jump */
				if(AL_PAUSED == state)
				{
					/* HACK: The problem is that when paused, after
					 * the Rewind, I can't get it off the INITIAL
					 * state without restarting
					 */
					alSourcePlay(ALmixer_Channel_List[channel].alsource);
					if((error = alGetError()) != AL_NO_ERROR)
					{
						fprintf(stderr, "25Testing error: %s\n",
								alGetString(error));				
					}
					alSourcePause(ALmixer_Channel_List[channel].alsource);
					if((error = alGetError()) != AL_NO_ERROR)
					{
						ALmixer_SetError("Internal_SeekChannel: alSourcePause(%d): %s", channel, alGetString(error) );
						retval = -1;
					}
				}
			}
			else
			{
				/* Streamed data is different. Rewinding the channel
				 * does no good. Rewinding the data will have an
				 * effect, but it will be lagged based on how
				 * much data is queued. Recommend users call Halt
				 * before rewind if they want immediate results.
				 */
				if(AL_FALSE == Internal_SeekData(ALmixer_Channel_List[channel].almixer_data, msec))
				{
					retval = -1;
				}
			}
			running_count = 1;
		}
	}
	/* The user wants to rewind all channels */
	else
	{
		ALint i;
		ALfloat sec_offset = msec / 1000.0f;

		for(i=0; i<Number_of_Channels_global; i++)
		{
			/* only need to process channel if in use */
			if(ALmixer_Channel_List[i].channel_in_use)
			{
				/* What should I do? Do I just rewind the channel
				 * or also rewind the data? Since the data is
				 * shared, let's make it the user's responsibility
				 * to rewind the data.
				 */
				if(ALmixer_Channel_List[i].almixer_data->decoded_all)
				{
					alGetSourcei(
								 ALmixer_Channel_List[i].alsource,
								 AL_SOURCE_STATE, &state
								 );
					if((error = alGetError()) != AL_NO_ERROR)
					{
						fprintf(stderr, "26Testing error: %s\n",
								alGetString(error));				
					}

					/* OpenAL 1.1 spec says if this succeeds on a playing source, it will automatically jump */
					alSourcef(ALmixer_Channel_List[channel].alsource, AL_SEC_OFFSET, sec_offset);
					if((error = alGetError()) != AL_NO_ERROR)
					{
						ALmixer_SetError("Internal_SeekChannel: alSourcef(all): %s", alGetString(error) );
						retval = -1;
					}
					if(AL_PAUSED == state)
					{
						/* HACK: The problem is that when paused, after
						 * the Rewind, I can't get it off the INITIAL
						 * state without restarting
						 */
						alSourcePlay(ALmixer_Channel_List[i].alsource);
						if((error = alGetError()) != AL_NO_ERROR)
						{
							fprintf(stderr, "27Testing error: %s\n",
									alGetString(error));				
						}
						alSourcePause(ALmixer_Channel_List[i].alsource);
						if((error = alGetError()) != AL_NO_ERROR)
						{
							ALmixer_SetError("Internal_SeekChannel: alSourcePause(all): %s", alGetString(error) );
							retval = -1;
						}
					}
				}
				else
				{
					/* Streamed data is different. Rewinding the channel
					 * does no good. Rewinding the data will have an
					 * effect, but it will be lagged based on how
					 * much data is queued. Recommend users call Halt
					 * before rewind if they want immediate results.
					 */
					if(AL_FALSE == Internal_SeekData(ALmixer_Channel_List[i].almixer_data, msec))
					{
						retval = -1;
					}
				}
				running_count++;
			}
		}
	}
	if(-1 == retval)
	{
		return -1;
	}
	else
	{
		return running_count;
	}
	
}

static ALint Internal_SeekSource(ALuint source, ALuint msec)
{
	ALint channel;
	if(0 == source)
	{
		return Internal_SeekChannel(-1, msec);
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot seek source: %s", ALmixer_GetError());
		return 0;
	}
	return Internal_SeekChannel(channel, msec);
}



static ALint Internal_FadeInChannelTimed(ALint channel, ALmixer_Data* data, ALint loops, ALuint fade_ticks, ALint expire_ticks)
{
	ALfloat value;
	ALenum error;
	ALfloat original_value;
/*	ALuint current_time = ALmixer_GetTicks(); */
	ALint retval;

	
	
	if(channel >= Number_of_Channels_global)
	{
			ALmixer_SetError("Requested channel (%d) exceeds maximum channel (%d) because only %d channels are allocated", channel, Number_of_Channels_global-1, Number_of_Channels_global);
		return -1;
	}
	/* Let's call PlayChannelTimed to do the job. 
	 * There are two catches:
	 * First is that we must set the volumes before the play call(s).
	 * Second is that we must initialize the channel values
	 */

	if(channel < 0)
	{
		/* This might cause a problem for threads/race conditions.
		 * We need to set the volume on an unknown channel,
		 * so we need to request a channel first. Remember
		 * that requesting a channel doesn't lock and it 
		 * could be surrendered to somebody else before we claim it.
		 */
		channel = Internal_GetChannel(0);
		if(-1 == channel)
		{
			return -1;
		}
	}	
	else if(ALmixer_Channel_List[channel].channel_in_use)
	{
		ALmixer_SetError("Channel %d is already in use", channel);
		return -1;
	}

	
	/* Get the original volume in case of a problem */
	alGetSourcef(ALmixer_Channel_List[channel].alsource,
		AL_GAIN, &original_value);
	
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "35Testing error: %s\n",
			alGetString(error));				
	}
	ALmixer_Channel_List[channel].fade_end_volume = original_value;

	/* Get the Min volume */
	alGetSourcef(ALmixer_Channel_List[channel].alsource,
		AL_MIN_GAIN, &value);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "36Testing error: %s\n",
			alGetString(error));				
	}
	ALmixer_Channel_List[channel].fade_start_volume = value;
	
	/* Set the actual volume */
	alSourcef(ALmixer_Channel_List[channel].alsource,
		AL_GAIN, value);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "37Testing error: %s\n",
			alGetString(error));				
	}
	
	
	/* Now call PlayChannelTimed */
	retval = Internal_PlayChannelTimed(channel, data, loops, expire_ticks);
	if(-1 == retval)
	{
		/* Chance of failure is actually pretty high since 
		 * a channel might already be in use or streamed
		 * data can be shared
		 */
		/* Restore the original value to avoid accidental 
		 * distruption of playback
		 */
		alSourcef(ALmixer_Channel_List[channel].alsource,
			AL_GAIN, original_value);
		if((error = alGetError()) != AL_NO_ERROR)
		{
			fprintf(stderr, "38Testing error: %s\n",
				alGetString(error));				
		}
		return retval;
	}

	/* We can't accept 0 as a value because of div-by-zero.
	 * If zero, just call PlayChannelTimed at normal
	 * volume
	 */
	if(0 == fade_ticks)
	{
		alSourcef(ALmixer_Channel_List[channel].alsource,
			AL_GAIN, 
			ALmixer_Channel_List[channel].fade_end_volume
		);
		if((error = alGetError()) != AL_NO_ERROR)
		{
			fprintf(stderr, "39Testing error: %s\n",
				alGetString(error));				
		}

		return retval;
	}
	
	/* Enable fading effects via the flag */
	ALmixer_Channel_List[channel].fade_enabled = 1;
	/* Set fade start time */
	ALmixer_Channel_List[channel].fade_start_time
		= ALmixer_Channel_List[channel].start_time;
	/* Set the fade expire ticks */
	ALmixer_Channel_List[channel].fade_expire_ticks = fade_ticks;

	/* Set 1/(endtime-starttime) or 1/deltaT */
	ALmixer_Channel_List[channel].fade_inv_time = 1.0f / fade_ticks;

	return retval;
	
}

		
static ALuint Internal_FadeInSourceTimed(ALuint source, ALmixer_Data* data, ALint loops, ALuint fade_ticks, ALint expire_ticks)
{
	ALint channel;
	ALint retval;
	if(0 == source)
	{
		retval = Internal_FadeInChannelTimed(-1, data, loops, fade_ticks, expire_ticks);
		if(-1 == retval)
		{
			return 0;
		}
		else
		{
			return Internal_GetSource(retval);
		}
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot FadeIn source: %s", ALmixer_GetError());
		return 0;
	}
	retval = Internal_FadeInChannelTimed(channel, data, loops, fade_ticks, expire_ticks);
	if(-1 == retval)
	{
		return 0;
	}
	else
	{
		return source;
	}
	/* make compiler happy */
	return 0;
}




/* Will fade out currently playing channels.
 * It starts at the current volume level and goes down */
static ALint Internal_FadeOutChannel(ALint channel, ALuint ticks)
{
	ALfloat value;
	ALenum error;
	ALuint current_time = ALmixer_GetTicks();
	ALuint counter = 0;
	
	/* We can't accept 0 as a value because of div-by-zero.
	 * If zero, just call Halt at normal
	 * volume
	 */
	if(0 == ticks)
	{
		return Internal_HaltChannel(channel, AL_FALSE);
	}
	
	
	if(channel >= Number_of_Channels_global)
	{
			ALmixer_SetError("Requested channel (%d) exceeds maximum channel (%d) because only %d channels are allocated", channel, Number_of_Channels_global-1, Number_of_Channels_global);
		return -1;
	}

	if(channel >= 0)
	{
		if(ALmixer_Channel_List[channel].channel_in_use)
		{
			/* Get the current volume */
			alGetSourcef(ALmixer_Channel_List[channel].alsource,
				AL_GAIN, &value);
			ALmixer_Channel_List[channel].fade_start_volume = value;
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "40Testing error: %s\n",
			alGetString(error));				
	}
		
			/* Get the Min volume */
			alGetSourcef(ALmixer_Channel_List[channel].alsource,
				AL_MIN_GAIN, &value);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "41Testing error: %s\n",
			alGetString(error));				
	}
			ALmixer_Channel_List[channel].fade_end_volume = value;
	
			/* Set expire start time */
			ALmixer_Channel_List[channel].start_time = current_time;
			/* Set the expire ticks */
			ALmixer_Channel_List[channel].expire_ticks = ticks;
			/* Set fade start time */
			ALmixer_Channel_List[channel].fade_start_time = current_time;
			/* Set the fade expire ticks */
			ALmixer_Channel_List[channel].fade_expire_ticks = ticks;
			/* Enable fading effects via the flag */
			ALmixer_Channel_List[channel].fade_enabled = 1;

			/* Set 1/(endtime-starttime) or 1/deltaT */
			ALmixer_Channel_List[channel].fade_inv_time = 1.0f / ticks;

			counter++;
		}
	}
	/* Else need to fade out all channels */
	else
	{
		ALint i;
		for(i=0; i<Number_of_Channels_global; i++)
		{
			if(ALmixer_Channel_List[i].channel_in_use)
			{
				/* Get the current volume */
				alGetSourcef(ALmixer_Channel_List[i].alsource,
					AL_GAIN, &value);
				ALmixer_Channel_List[i].fade_start_volume = value;
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "42Testing error: %s\n",
			alGetString(error));				
	}
			
				/* Get the Min volume */
				alGetSourcef(ALmixer_Channel_List[i].alsource,
					AL_MIN_GAIN, &value);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "43Testing error: %s\n",
			alGetString(error));				
	}
				ALmixer_Channel_List[i].fade_end_volume = value;
	
				/* Set expire start time */
				ALmixer_Channel_List[i].start_time = current_time;
				/* Set the expire ticks */
				ALmixer_Channel_List[i].expire_ticks = ticks;
				/* Set fade start time */
				ALmixer_Channel_List[i].fade_start_time = current_time;
				/* Set the fade expire ticks */
				ALmixer_Channel_List[i].fade_expire_ticks = ticks;
				/* Enable fading effects via the flag */
				ALmixer_Channel_List[i].fade_enabled = 1;

				/* Set 1/(endtime-starttime) or 1/deltaT */
				ALmixer_Channel_List[i].fade_inv_time = 1.0f / ticks;

				counter++;
			}
		} /* End for loop */
	} 
	return counter;
}


static ALint Internal_FadeOutSource(ALuint source, ALuint ticks)
{
	ALint channel;
	if(0 == source)
	{
		return Internal_FadeOutChannel(-1, ticks);
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot FadeOut source: %s", ALmixer_GetError());
		return -1;
	}
	return Internal_FadeOutChannel(channel, ticks);
}


/* Will fade currently playing channels.
 * It starts at the current volume level and go to target 
 * Only affects channels that are playing
 */
static ALint Internal_FadeChannel(ALint channel, ALuint ticks, ALfloat volume)
{
	ALfloat value;
	ALenum error;
	ALuint current_time = ALmixer_GetTicks();
	ALuint counter = 0;
	
	if(channel >= Number_of_Channels_global)
	{
			ALmixer_SetError("Requested channel (%d) exceeds maximum channel (%d) because only %d channels are allocated", channel, Number_of_Channels_global-1, Number_of_Channels_global);
		return -1;
	}

	if(channel >= 0)
	{
		if(volume < ALmixer_Channel_List[channel].min_volume)
		{
			volume = ALmixer_Channel_List[channel].min_volume;
		}
		else if(volume > ALmixer_Channel_List[channel].max_volume)
		{
			volume = ALmixer_Channel_List[channel].max_volume;
		}
		
		if(ALmixer_Channel_List[channel].channel_in_use)
		{
			if(ticks > 0)
			{
				/* Get the current volume */
				alGetSourcef(ALmixer_Channel_List[channel].alsource,
					AL_GAIN, &value);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "44Testing error: %s\n",
			alGetString(error));				
	}
				ALmixer_Channel_List[channel].fade_start_volume = value;
			
				/* Set the target volume */
				ALmixer_Channel_List[channel].fade_end_volume = volume;
		
				/* Set fade start time */
				ALmixer_Channel_List[channel].fade_start_time = current_time;
				/* Set the fade expire ticks */
				ALmixer_Channel_List[channel].fade_expire_ticks = ticks;
				/* Enable fading effects via the flag */
				ALmixer_Channel_List[channel].fade_enabled = 1;

				/* Set 1/(endtime-starttime) or 1/deltaT */
				ALmixer_Channel_List[channel].fade_inv_time = 1.0f / ticks;
			}
			else
			{
				alSourcef(ALmixer_Channel_List[channel].alsource,
					AL_GAIN, volume);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "45Testing error: %s\n",
			alGetString(error));				
	}
			}
			counter++;
		}
	}
	/* Else need to fade out all channels */
	else
	{
		ALint i;
		for(i=0; i<Number_of_Channels_global; i++)
		{
			if(volume < ALmixer_Channel_List[i].min_volume)
			{
				volume = ALmixer_Channel_List[i].min_volume;
			}
			else if(volume > ALmixer_Channel_List[i].max_volume)
			{
				volume = ALmixer_Channel_List[i].max_volume;
			}
		
			if(ALmixer_Channel_List[i].channel_in_use)
			{
				if(ticks > 0)
				{
					/* Get the current volume */
					alGetSourcef(ALmixer_Channel_List[i].alsource,
						AL_GAIN, &value);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "46Testing error: %s\n",
			alGetString(error));				
	}
					ALmixer_Channel_List[i].fade_start_volume = value;
				
					/* Set target volume */
					ALmixer_Channel_List[i].fade_end_volume = volume;
		
					/* Set fade start time */
					ALmixer_Channel_List[i].fade_start_time = current_time;
					/* Set the fade expire ticks */
					ALmixer_Channel_List[i].fade_expire_ticks = ticks;
					/* Enable fading effects via the flag */
					ALmixer_Channel_List[i].fade_enabled = 1;

					/* Set 1/(endtime-starttime) or 1/deltaT */
					ALmixer_Channel_List[i].fade_inv_time = 1.0f / ticks;
				}
				else
				{
					alSourcef(ALmixer_Channel_List[i].alsource,
						AL_GAIN, volume);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "47Testing error: %s\n",
			alGetString(error));				
	}
				}
				counter++;
			}
		} /* End for loop */
	} 
	return counter;
}

static ALint Internal_FadeSource(ALuint source, ALuint ticks, ALfloat volume)
{
	ALint channel;
	if(0 == source)
	{
		return Internal_FadeChannel(-1, ticks, volume);
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot Fade source: %s", ALmixer_GetError());
		return -1;
	}
	return Internal_FadeChannel(channel, ticks, volume);
}




/* Set a volume regardless if it's in use or not.
 */
static ALboolean Internal_SetVolumeChannel(ALint channel, ALfloat volume)
{
	ALenum error;
	ALboolean retval = AL_TRUE;
	
	if(channel >= Number_of_Channels_global)
	{
		ALmixer_SetError("Requested channel (%d) exceeds maximum channel (%d) because only %d channels are allocated", channel, Number_of_Channels_global-1, Number_of_Channels_global);
		return AL_FALSE;
	}
	
	if(channel >= 0)
	{
		if(volume < 0.0f)
		{
			volume = 0.0f;
		}
		else if(volume > 1.0f)
		{
			volume = 1.0f;
		}
		alSourcef(ALmixer_Channel_List[channel].alsource,
				  AL_GAIN, volume);
		if((error = alGetError()) != AL_NO_ERROR)
		{
			ALmixer_SetError("Internal_SetVolumeChannel: alSourcef(%d): %s", channel, alGetString(error) );
			retval = AL_FALSE;
		}
	}
	else
	{
		ALint i;
		for(i=0; i<Number_of_Channels_global; i++)
		{
			if(volume < 0.0f)
			{
				volume = 0.0f;
			}
			else if(volume > 1.0f)
			{
				volume = 1.0f;
			}
			alSourcef(ALmixer_Channel_List[i].alsource,
					  AL_GAIN, volume);
			if((error = alGetError()) != AL_NO_ERROR)
			{
				ALmixer_SetError("Internal_SetVolumeChannel: alSourcef(all): %s", alGetString(error) );
				retval = AL_FALSE;
			}
		}
	}
	return retval;
}

static ALboolean Internal_SetVolumeSource(ALuint source, ALfloat volume)
{
	ALint channel;
	if(0 == source)
	{
		return Internal_SetVolumeChannel(-1, volume);
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot SetMaxVolume: %s", ALmixer_GetError());
		return AL_FALSE;
	}
	return Internal_SetVolumeChannel(channel, volume);
}


static ALfloat Internal_GetVolumeChannel(ALint channel)
{
	 ALfloat value;
	 ALenum error;
	ALfloat running_total = 0.0f;
	ALfloat retval = 0.0f;
	
	if(channel >= Number_of_Channels_global)
	{
		ALmixer_SetError("Requested channel (%d) exceeds maximum channel (%d) because only %d channels are allocated", channel, Number_of_Channels_global-1, Number_of_Channels_global);
		return -1.0f;
	}
	
	if(channel >= 0)
	{
		 alGetSourcef(ALmixer_Channel_List[channel].alsource,
		 AL_GAIN, &value);
		 if((error = alGetError()) != AL_NO_ERROR)
		 {
			 ALmixer_SetError("Internal_GetVolumeChannel: alGetSourcef(%d): %s", channel, alGetString(error) );
			 retval = -1.0f;
		 }
		 else
		 {
			 retval = value;
		 }
	}
	else
	{
		ALint i;
		for(i=0; i<Number_of_Channels_global; i++)
		{
			 alGetSourcef(ALmixer_Channel_List[i].alsource,
			 AL_GAIN, &value);
			 if((error = alGetError()) != AL_NO_ERROR)
			 {
				 ALmixer_SetError("Internal_GetVolumeChannel: alGetSourcef(all): %s", alGetString(error) );
				 retval = -1;
			 }
			 else
			 {
				 running_total += value;
			 }
		}
		if(0 == Number_of_Channels_global)
		{
			ALmixer_SetError("Internal_GetVolumeChannel: No channels are allocated");
			retval = -1.0f;
		}
		else
		{
			retval = running_total / Number_of_Channels_global;
		}
	}
	return retval;
}

static ALfloat Internal_GetVolumeSource(ALuint source)
{
	ALint channel;
	if(0 == source)
	{
		return Internal_GetVolumeChannel(-1);
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot GetVolume: %s", ALmixer_GetError());
		return -1.0f;
	}
	
	return Internal_GetVolumeChannel(channel);
}



/* Set a volume regardless if it's in use or not.
 */
static ALboolean Internal_SetMaxVolumeChannel(ALint channel, ALfloat volume)
{
	ALenum error;
	ALboolean retval = AL_TRUE;
	
	if(channel >= Number_of_Channels_global)
	{
			ALmixer_SetError("Requested channel (%d) exceeds maximum channel (%d) because only %d channels are allocated", channel, Number_of_Channels_global-1, Number_of_Channels_global);
		return AL_FALSE;
	}

	if(channel >= 0)
	{
		if(volume < 0.0f)
		{
			volume = 0.0f;
		}
		else if(volume > 1.0f)
		{
			volume = 1.0f;
		}
		ALmixer_Channel_List[channel].max_volume = volume;
		alSourcef(ALmixer_Channel_List[channel].alsource,
			AL_MAX_GAIN, volume);
		if((error = alGetError()) != AL_NO_ERROR)
		{
			ALmixer_SetError("Internal_SetMaxVolumeChannel: alSourcef(%d, max): %s", channel, alGetString(error) );
			retval = AL_FALSE;
		}
		if(ALmixer_Channel_List[channel].max_volume < ALmixer_Channel_List[channel].min_volume)
		{
			ALmixer_Channel_List[channel].min_volume = volume;
			alSourcef(ALmixer_Channel_List[channel].alsource,
				AL_MIN_GAIN, volume);
			if((error = alGetError()) != AL_NO_ERROR)
			{
				ALmixer_SetError("Internal_SetMaxVolumeChannel: alSourcef(%d, min): %s", channel, alGetString(error) );
				retval = AL_FALSE;
			}
		}
	}
	else
	{
		ALint i;
		for(i=0; i<Number_of_Channels_global; i++)
		{
			if(volume < 0.0f)
			{
				volume = 0.0f;
			}
			else if(volume > 1.0f)
			{
				volume = 1.0f;
			}
			ALmixer_Channel_List[i].max_volume = volume;
			alSourcef(ALmixer_Channel_List[i].alsource,
				AL_MAX_GAIN, volume);
			if((error = alGetError()) != AL_NO_ERROR)
			{
				ALmixer_SetError("Internal_SetMaxVolumeChannel: alSourcef(all, max): %s", alGetString(error) );
				retval = AL_FALSE;
			}
			if(ALmixer_Channel_List[i].max_volume < ALmixer_Channel_List[i].min_volume)
			{
				ALmixer_Channel_List[i].min_volume = volume;
				alSourcef(ALmixer_Channel_List[i].alsource,
					AL_MIN_GAIN, volume);
				if((error = alGetError()) != AL_NO_ERROR)
				{
					ALmixer_SetError("Internal_SetMaxVolumeChannel: alSourcef(all, min): %s", alGetString(error) );
					retval = AL_FALSE;
				}
			}
		}
	}
	return retval;
}

static ALint Internal_SetMaxVolumeSource(ALuint source, ALfloat volume)
{
	ALint channel;
	if(0 == source)
	{
		return Internal_SetMaxVolumeChannel(-1, volume);
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot SetMaxVolume: %s", ALmixer_GetError());
		return AL_FALSE;
	}
	return Internal_SetMaxVolumeChannel(channel, volume);
}

static ALfloat Internal_GetMaxVolumeChannel(ALint channel)
{
	/*
	ALfloat value;
	ALenum error;
	*/
	ALfloat running_total = 0.0f;
	ALfloat retval = 0.0f;
	
	if(channel >= Number_of_Channels_global)
	{
			ALmixer_SetError("Requested channel (%d) exceeds maximum channel (%d) because only %d channels are allocated", channel, Number_of_Channels_global-1, Number_of_Channels_global);
		return -1.0f;
	}

	if(channel >= 0)
	{
		/*
		alGetSourcef(ALmixer_Channel_List[channel].alsource,
			AL_GAIN, &value);
		if((error = alGetError()) != AL_NO_ERROR)
		{
			ALmixer_SetError("%s",
				alGetString(error) );
			retval = -1.0f;
		}
		else
		{
			retval = value;
		}
		*/
		retval = ALmixer_Channel_List[channel].max_volume;
	
	}
	else
	{
		ALint i;
		for(i=0; i<Number_of_Channels_global; i++)
		{
			/*
			alGetSourcef(ALmixer_Channel_List[i].alsource,
				AL_GAIN, &value);
			if((error = alGetError()) != AL_NO_ERROR)
			{
				ALmixer_SetError("%s",
					alGetString(error) );
				retval = -1;
			}
			else
			{
				running_total += value;
			}
			*/
			running_total += ALmixer_Channel_List[i].max_volume;
		}
		if(0 == Number_of_Channels_global)
		{
			ALmixer_SetError("No channels are allocated");
			retval = -1.0f;
		}
		else
		{
			retval = running_total / Number_of_Channels_global;
		}
	}
	return retval;
}

static ALfloat Internal_GetMaxVolumeSource(ALuint source)
{
	ALint channel;
	if(0 == source)
	{
		return Internal_GetMaxVolumeChannel(-1);
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot GetVolume: %s", ALmixer_GetError());
		return -1.0f;
	}

	return Internal_GetMaxVolumeChannel(channel);
}


/* Set a volume regardless if it's in use or not.
 */
static ALboolean Internal_SetMinVolumeChannel(ALint channel, ALfloat volume)
{
	ALenum error;
	ALboolean retval = AL_TRUE;
	
	if(channel >= Number_of_Channels_global)
	{
			ALmixer_SetError("Requested channel (%d) exceeds maximum channel (%d) because only %d channels are allocated", channel, Number_of_Channels_global-1, Number_of_Channels_global);
		return AL_FALSE;
	}

	if(channel >= 0)
	{
		if(volume < 0.0f)
		{
			volume = 0.0f;
		}
		else if(volume > 1.0f)
		{
			volume = 1.0f;
		}
		ALmixer_Channel_List[channel].min_volume = volume;
		alSourcef(ALmixer_Channel_List[channel].alsource,
			AL_MIN_GAIN, volume);
		if((error = alGetError()) != AL_NO_ERROR)
		{
			ALmixer_SetError("Internal_SetMinVolumeChannel: alSourcef(%d, max): %s", channel, alGetString(error) );
			retval = AL_FALSE;
		}
		if(ALmixer_Channel_List[channel].max_volume < ALmixer_Channel_List[channel].min_volume)
		{
			ALmixer_Channel_List[channel].max_volume = volume;
			alSourcef(ALmixer_Channel_List[channel].alsource,
				AL_MAX_GAIN, volume);
			if((error = alGetError()) != AL_NO_ERROR)
			{
                ALmixer_SetError("Internal_SetMinVolumeChannel: alSourcef(%d, min): %s", channel, alGetString(error) );
				retval = AL_FALSE;
			}
		}
	}
	else
	{
		ALint i;
		for(i=0; i<Number_of_Channels_global; i++)
		{
			if(volume < 0.0f)
			{
				volume = 0.0f;
			}
			else if(volume > 1.0f)
			{
				volume = 1.0f;
			}
			ALmixer_Channel_List[i].min_volume = volume;
			alSourcef(ALmixer_Channel_List[i].alsource,
				AL_MIN_GAIN, volume);
			if((error = alGetError()) != AL_NO_ERROR)
			{
                ALmixer_SetError("Internal_SetMinVolumeChannel: alSourcef(all, max): %s", alGetString(error) );
				retval = AL_FALSE;
			}
			if(ALmixer_Channel_List[i].max_volume < ALmixer_Channel_List[i].min_volume)
			{
				ALmixer_Channel_List[i].max_volume = volume;
				alSourcef(ALmixer_Channel_List[i].alsource,
					AL_MAX_GAIN, volume);
				if((error = alGetError()) != AL_NO_ERROR)
				{
                    ALmixer_SetError("Internal_SetMinVolumeChannel: alSourcef(all, min): %s", alGetString(error) );
					retval = AL_FALSE;
				}
			}
		}
	}
	return retval;
}

static ALboolean Internal_SetMinVolumeSource(ALuint source, ALfloat volume)
{
	ALint channel;
	if(0 == source)
	{
		return Internal_SetMinVolumeChannel(-1, volume);
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot SetMaxVolume: %s", ALmixer_GetError());
		return AL_FALSE;
	}
	return Internal_SetMinVolumeChannel(channel, volume);
}

static ALfloat Internal_GetMinVolumeChannel(ALint channel)
{
	/*
	ALfloat value;
	ALenum error;
	*/
	ALfloat running_total = 0.0f;
	ALfloat retval = 0.0f;
	
	if(channel >= Number_of_Channels_global)
	{
			ALmixer_SetError("Requested channel (%d) exceeds maximum channel (%d) because only %d channels are allocated", channel, Number_of_Channels_global-1, Number_of_Channels_global);
		return -1.0f;
	}

	if(channel >= 0)
	{
		/*
		alGetSourcef(ALmixer_Channel_List[channel].alsource,
			AL_GAIN, &value);
		if((error = alGetError()) != AL_NO_ERROR)
		{
			ALmixer_SetError("%s",
				alGetString(error) );
			retval = -1.0f;
		}
		else
		{
			retval = value;
		}
		*/
		retval = ALmixer_Channel_List[channel].min_volume;
	
	}
	else
	{
		ALint i;
		for(i=0; i<Number_of_Channels_global; i++)
		{
			/*
			alGetSourcef(ALmixer_Channel_List[i].alsource,
				AL_GAIN, &value);
			if((error = alGetError()) != AL_NO_ERROR)
			{
				ALmixer_SetError("%s",
					alGetString(error) );
				retval = -1;
			}
			else
			{
				running_total += value;
			}
			*/
			running_total += ALmixer_Channel_List[i].min_volume;
		}
		if(0 == Number_of_Channels_global)
		{
			ALmixer_SetError("No channels are allocated");
			retval = -1.0f;
		}
		else
		{
			retval = running_total / Number_of_Channels_global;
		}
	}
	return retval;
}

static ALfloat Internal_GetMinVolumeSource(ALuint source)
{
	ALint channel;
	if(0 == source)
	{
		return Internal_GetMinVolumeChannel(-1);
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot GetVolume: %s", ALmixer_GetError());
		return -1.0f;
	}

	return Internal_GetMinVolumeChannel(channel);
}


/* Changes the listener volume */
static ALboolean Internal_SetMasterVolume(ALfloat volume)
{
	ALenum error;
	alListenerf(AL_GAIN, volume);
	if((error = alGetError()) != AL_NO_ERROR)
	{
        ALmixer_SetError("Internal_SetMasterVolume: alListenerf: %s", alGetString(error) );
		return AL_FALSE;
	}
	return AL_TRUE;
}

static ALfloat Internal_GetMasterVolume()
{
	ALenum error;
	ALfloat volume;
	alGetListenerf(AL_GAIN, &volume);
	if((error = alGetError()) != AL_NO_ERROR)
	{
        ALmixer_SetError("Internal_GetMasterVolume: alGetListenerf: %s", alGetString(error) );
		return -1.0f;
	}
	return volume;	
}




/* Will fade out currently playing channels.
 * It starts at the current volume level and goes down */
static ALint Internal_ExpireChannel(ALint channel, ALint ticks)
{
	ALuint current_time = ALmixer_GetTicks();
	ALuint counter = 0;
	
	/* We can't accept 0 as a value because of div-by-zero.
	 * If zero, just call Halt at normal
	 * volume
	 */
	if(0 == ticks)
	{
		return Internal_HaltChannel(channel, AL_FALSE);
	}
	if(ticks < -1)
	{
		ticks = -1;
	}
	
	
	if(channel >= Number_of_Channels_global)
	{
			ALmixer_SetError("Requested channel (%d) exceeds maximum channel (%d) because only %d channels are allocated", channel, Number_of_Channels_global-1, Number_of_Channels_global);
		return -1;
	}

	if(channel >= 0)
	{
		if(ALmixer_Channel_List[channel].channel_in_use)
		{
			/* Set expire start time */
			ALmixer_Channel_List[channel].start_time = current_time;
			/* Set the expire ticks */
			ALmixer_Channel_List[channel].expire_ticks = ticks;

			counter++;
		}
	}
	/* Else need to fade out all channels */
	else
	{
		ALint i;
		for(i=0; i<Number_of_Channels_global; i++)
		{
			if(ALmixer_Channel_List[i].channel_in_use)
			{
				/* Set expire start time */
				ALmixer_Channel_List[i].start_time = current_time;
				/* Set the expire ticks */
				ALmixer_Channel_List[i].expire_ticks = ticks;

				counter++;
			}
		} /* End for loop */
	} 
	return counter;
}


static ALint Internal_ExpireSource(ALuint source, ALint ticks)
{
	ALint channel;
	if(0 == source)
	{
		return Internal_ExpireChannel(-1, ticks);
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot Expire source: %s", ALmixer_GetError());
		return -1;
	}
	return Internal_ExpireChannel(channel, ticks);
}


static ALint Internal_QueryChannel(ALint channel)
{
	ALint i;
	ALint counter = 0;
	if(channel >= Number_of_Channels_global)
	{
		ALmixer_SetError("Invalid channel: %d", channel);
		return -1;
	}

	if(channel >= 0)
	{
		return ALmixer_Channel_List[channel].channel_in_use;	
	}

	/* Else, return the number of channels in use */
	for(i=0; i<Number_of_Channels_global; i++)
	{
		if(ALmixer_Channel_List[i].channel_in_use)
		{
			counter++;
		}
	}
	return counter;
}


static ALint Internal_QuerySource(ALuint source)
{
	ALint channel;
	if(0 == source)
	{
		return Internal_QueryChannel(-1);
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot query source: %s", ALmixer_GetError());
		return -1;
	}
	
	return Internal_QueryChannel(channel);
}


static ALuint Internal_CountUnreservedUsedChannels()
{
	ALint i;
	ALuint counter = 0;


	/* Else, return the number of channels in use */
	for(i=Number_of_Reserve_Channels_global; i<Number_of_Channels_global; i++)
	{
		if(ALmixer_Channel_List[i].channel_in_use)
		{
			counter++;
		}
	}
	return counter;
}

static ALuint Internal_CountUnreservedFreeChannels()
{
	ALint i;
	ALuint counter = 0;


	/* Else, return the number of channels in use */
	for(i=Number_of_Reserve_Channels_global; i<Number_of_Channels_global; i++)
	{
		if( ! ALmixer_Channel_List[i].channel_in_use)
		{
			counter++;
		}
	}
	return counter;
}

static ALuint Internal_CountAllUsedChannels()
{
	ALint i;
	ALuint counter = 0;


	/* Else, return the number of channels in use */
	for(i=0; i<Number_of_Channels_global; i++)
	{
		if(ALmixer_Channel_List[i].channel_in_use)
		{
			counter++;
		}
	}
	return counter;
}

static ALuint Internal_CountAllFreeChannels()
{
	ALint i;
	ALuint counter = 0;


	/* Else, return the number of channels in use */
	for(i=0; i<Number_of_Channels_global; i++)
	{
		if( ! ALmixer_Channel_List[i].channel_in_use)
		{
			counter++;
		}
	}
	return counter;
}


static ALint Internal_PlayingChannel(ALint channel)
{
	ALint i;
	ALint counter = 0;
	ALint state;
	
	if(channel >= Number_of_Channels_global)
	{
		ALmixer_SetError("Invalid channel: %d", channel);
		return -1;
	}

	if(channel >= 0)
	{
		if(ALmixer_Channel_List[channel].channel_in_use)	
		{
			alGetSourcei(
				ALmixer_Channel_List[channel].alsource,
				AL_SOURCE_STATE, &state
			);
			if(AL_PLAYING == state)
			{
				return 1;
			}
		}
		return 0;
	}

	/* Else, return the number of channels in use */
	for(i=0; i<Number_of_Channels_global; i++)
	{
		if(ALmixer_Channel_List[i].channel_in_use)	
		{
			alGetSourcei(
				ALmixer_Channel_List[i].alsource,
				AL_SOURCE_STATE, &state
			);
			if(AL_PLAYING == state)
			{
				counter++;
			}
		}
	}
	return counter;
}


static ALint Internal_PlayingSource(ALuint source)
{
	ALint channel;
	if(0 == source)
	{
		return Internal_PlayingChannel(-1);
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot query source: %s", ALmixer_GetError());
		return -1;
	}
	
	return Internal_PlayingChannel(channel);
}


static ALint Internal_PausedChannel(ALint channel)
{
	ALint i;
	ALint counter = 0;
	ALint state;
	
	if(channel >= Number_of_Channels_global)
	{
		ALmixer_SetError("Invalid channel: %d", channel);
		return -1;
	}

	if(channel >= 0)
	{
		if(ALmixer_Channel_List[channel].channel_in_use)	
		{
			alGetSourcei(
				ALmixer_Channel_List[channel].alsource,
				AL_SOURCE_STATE, &state
			);
			if(AL_PAUSED == state)
			{
				return 1;
			}
		}
		return 0;
	}

	/* Else, return the number of channels in use */
	for(i=0; i<Number_of_Channels_global; i++)
	{
		if(ALmixer_Channel_List[i].channel_in_use)	
		{
			alGetSourcei(
				ALmixer_Channel_List[i].alsource,
				AL_SOURCE_STATE, &state
			);
			if(AL_PAUSED == state)
			{
				counter++;
			}
		}
	}
	return counter;
}


static ALint Internal_PausedSource(ALuint source)
{
	ALint channel;
	if(0 == source)
	{
		return Internal_PausedChannel(-1);
	}
	
	channel = Internal_GetChannel(source);
	if(-1 == channel)
	{
		ALmixer_SetError("Cannot query source: %s", ALmixer_GetError());
		return -1;
	}
	
	return Internal_PausedChannel(channel);
}



static void Internal_FreeData(ALmixer_Data* data)
{
	if(NULL == data)
	{
		return;
	}
	
	if(data->in_use)
	{
		fprintf(stderr, "audio.dispose: Can't dispose of audio data because it is in use\n");
		return;
	}
	
	if(data->decoded_all)
	{
		/* If access_data was enabled, then the Sound_Sample*
		 * still exists. We need to free it
		 */
		if(data->sample != NULL)
		{
			Sound_FreeSample(data->sample);
			data->sample = NULL;
		}
		
		if(data->buffer != NULL)
		{
			_alDeleteBuffers(1, data->buffer);
		}
	}
	else
	{
		ALuint i;
		
		/* Delete buffer copies if access_data was enabled */
		if(data->buffer_map_list != NULL)
		{
			for(i=0; i<data->max_queue_buffers; i++)
			{
				_free(data->buffer_map_list[i].data);
			}
			_free(data->buffer_map_list);
			data->buffer_map_list = NULL;
		}
		if(data->circular_buffer_queue != NULL)
		{
			CircularQueueUnsignedInt_FreeQueue(data->circular_buffer_queue);
			data->circular_buffer_queue = NULL;
		}
			
		if(data->sample != NULL)
		{
			Sound_FreeSample(data->sample);
			data->sample = NULL;
		}
		
		if(data->buffer != NULL)
		{
			_alDeleteBuffers(data->max_queue_buffers, data->buffer);
		}
	}
	
	if(data->buffer != NULL)
	{
		_free(data->buffer);
		data->buffer = NULL;
	}

	LinkedList_Remove(s_listOfALmixerData,
		LinkedList_Find(s_listOfALmixerData, data, NULL)
	);

	_free(data);
}





				
/* Private function for Updating ALmixer.
 * This is a very big and ugly function.
 * It should return the number of buffers that were 
 * queued during the call. The value might be
 * used to guage how long you might wait to
 * call the next update loop in case you are worried
 * about preserving CPU cycles. The idea is that
 * when a buffer is queued, there was probably some
 * CPU intensive looping which took awhile.
 * It's mainly provided as a convenience.
 * Timing the call with ALmixer_GetTicks() would produce
 * more accurate information.
 * Returns a negative value if there was an error,
 * the value being the number of errors.
 */
static ALint Update_ALmixer(void* data)
{
	ALint retval = 0;
	ALint error_flag = 0;
	ALenum error;
	ALint state;
	ALint i=0;

#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
#ifdef ENABLE_ALMIXER_THREADS
		SDL_UnlockMutex(s_simpleLock);
#endif
		return 0;
	}

	/* Bypass if in interruption event */
	if(NULL == alcGetCurrentContext())
	{
#ifdef ENABLE_ALMIXER_THREADS
		SDL_UnlockMutex(s_simpleLock);
#endif
		return 0;
	}
	
	/* Check the quick flag to see if anything needs updating */
	/* If anything is playing, then we have to do work */
	if( 0 == Is_Playing_global)
	{
#ifdef ENABLE_ALMIXER_THREADS
		SDL_UnlockMutex(s_simpleLock);
#endif
		return 0;
	}
	/* Clear error */
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "08Testing errpr before unqueue because getting stuff, for OS X this is expected: %s\n",
				alGetString(error));				
	}
	alGetError();

	for(i=0; i<Number_of_Channels_global; i++)
	{
		if( ALmixer_Channel_List[i].channel_in_use )
		{

			/* For simplicity, before we do anything else,
			 * we can check the timeout and fading values
			 * and do the appropriate things
			 */
			ALuint current_time = ALmixer_GetTicks();

			/* Check to see if we need to halt due to Timed play */
			if(ALmixer_Channel_List[i].expire_ticks != -1) 
			{
				ALuint target_time = (ALuint)ALmixer_Channel_List[i].expire_ticks 
					 + ALmixer_Channel_List[i].start_time;
				alGetSourcei(ALmixer_Channel_List[i].alsource,
					AL_SOURCE_STATE, &state);
				if((error = alGetError()) != AL_NO_ERROR)
				{
					fprintf(stderr, "06Testing errpr before unqueue because getting stuff, for OS X this is expected: %s\n",
							alGetString(error));				
				}
				
				/* Check the time, and also make sure that it is not
				 * paused (if paused, we don't want to make the
				 * evaluation because when resumed, we will adjust
				 * the times to compensate for the pause).
				 */
				if( (current_time >= target_time) 
					&& (state != AL_PAUSED) )
				{
					/* Stop the playback */
					Internal_HaltChannel(i, AL_FALSE);
					if((error = alGetError()) != AL_NO_ERROR)
					{
						fprintf(stderr, "07Testing errpr before unqueue because getting stuff, for OS X this is expected: %s\n",
								alGetString(error));				
					}
					
					/* Everything should be done so go on to the next loop */
					continue;
				}
			} /* End if time expired check */

			/* Check to see if we need to adjust the volume for fading */
			if( ALmixer_Channel_List[i].fade_enabled )
			{
				ALuint target_time = ALmixer_Channel_List[i].fade_expire_ticks 
					 + ALmixer_Channel_List[i].fade_start_time;
				alGetSourcei(ALmixer_Channel_List[i].alsource,
					AL_SOURCE_STATE, &state);
				if((error = alGetError()) != AL_NO_ERROR)
				{
					fprintf(stderr, "05Testing errpr before unqueue because getting stuff, for OS X this is expected: %s\n",
							alGetString(error));				
				}
				
				/* Check the time, and also make sure that it is not
				 * paused (if paused, we don't want to make the
				 * evaluation because when resumed, we will adjust
				 * the times to compensate for the pause).
				 */
				if(state != AL_PAUSED)
				{
					ALfloat t;
					ALuint delta_time;
					ALfloat current_volume;
					if(current_time >= target_time)
					{
						/* Need to constrain value to the end time
						 * (can't go pass the value for calculations)
						 */
						current_time = target_time;
						/* We can disable the fade flag now */
						ALmixer_Channel_List[i].fade_enabled = 0;
					}
					/* Use the linear interpolation formula:
					 * X = (1-t)x0 + tx1
					 * where x0 would be the start value
					 * and x1 is the final value
					 * and t is delta_time*inv_time (adjusts 0 <= time <= 1)
					 * delta_time = current_time-start_time
					 * inv_time = 1/ (end_time-start_time)
					 * so t = current_time-start_time / (end_time-start_time)
					 * 
					 */
					delta_time = current_time - ALmixer_Channel_List[i].fade_start_time;
					t = (ALfloat) delta_time * ALmixer_Channel_List[i].fade_inv_time;
					current_volume = (1.0f-t) * ALmixer_Channel_List[i].fade_start_volume 
						+ t * ALmixer_Channel_List[i].fade_end_volume;
					/*
					fprintf(stderr, "start_vol=%f, end_vol:%f, current_volume: %f\n", ALmixer_Channel_List[i].fade_start_volume, ALmixer_Channel_List[i].fade_end_volume, current_volume);
					*/
					/* Set the volume */
					alSourcef(ALmixer_Channel_List[i].alsource,
						AL_GAIN, current_volume);
					if((error = alGetError()) != AL_NO_ERROR)
					{
						fprintf(stderr, "04Testing errpr before unqueue because getting stuff, for OS X this is expected: %s\n",
								alGetString(error));				
					}
					
	/*
	fprintf(stderr, "Current time =%d\n", current_time);
						fprintf(stderr, "Current vol=%f on channel %d\n", current_volume, i);
	*/
				} /* End if not PAUSED */
			} /* End if fade_enabled */
					 

			/* Okay, now that the time expired and fading stuff
			 * is done, do the rest of the hard stuff
			 */
			

			/* For predecoded, check to see if done */
			if (ALmixer_Channel_List[i].almixer_data->decoded_all)
			{

#if 0
		/********* Remove this **********/
				ALint buffers_processed;
				ALint buffers_still_queued;
		fprintf(stderr, "For Predecoded\n");
				
	alGetSourcei(
		ALmixer_Channel_List[i].alsource,
		AL_SOURCE_STATE, &state
				);
	switch(state) {
                case AL_PLAYING:
				fprintf(stderr, "Channel '%d' is PLAYING\n", i);
				break;
                case AL_PAUSED:
				fprintf(stderr, "Channel '%d' is PAUSED\n",i);
				break;
                case AL_STOPPED:
				fprintf(stderr, "Channel '%d' is STOPPED\n",i);
				break;
				case AL_INITIAL:
				fprintf(stderr, "Channel '%d' is INITIAL\n",i);
				break;
                default:
				fprintf(stderr, "Channel '%d' is UNKNOWN\n",i);
                  break;
       		}
					
				alGetSourcei(
					ALmixer_Channel_List[i].alsource,
					AL_BUFFERS_PROCESSED, &buffers_processed
				);
				fprintf(stderr, "Buffers processed = %d\n", buffers_processed);

				alGetSourcei(
					ALmixer_Channel_List[i].alsource,
					AL_BUFFERS_QUEUED, &buffers_still_queued
				);

				/******** END REMOVE *******/
#endif	
				/* FIXME: Ugh! Somewhere an alError is being thrown ("Invalid Enum Value"), but I can't
				 * find it. It only seems to be thrown for OS X. I placed error messages after every al*
				 * command I could find in the above loops, but the error doesn't seem to show 
				 * up until around here. I mistook it for a get queued buffers
				 * error in OS X. I don't think there's an error down there. 
				 * For now, I'm clearing the error here.
				 */
					
				if((error = alGetError()) != AL_NO_ERROR)
				{
					fprintf(stderr, "03Testing errpr before unqueue because getting stuff, for OS X this is expected: %s\n",
							alGetString(error));				
				}
				
					
				alGetSourcei(
					ALmixer_Channel_List[i].alsource,
					AL_SOURCE_STATE, &state
				);
				if((error = alGetError()) != AL_NO_ERROR)
				{
					fprintf(stderr, "02Testing errpr before unqueue because getting stuff, for OS X this is expected: %s\n",
							alGetString(error));				
				}
				
				
				if(AL_STOPPED == state)
				{
					/* Playback has ended. 
					 * Loop if necessary, or launch callback
					 * and clear channel (or clear channel and
					 * then launch callback?)
					 */


					/* Need to check for loops */
					if(ALmixer_Channel_List[i].loops != 0)
					{
						/* Corner Case: If the buffer has
						 * been modified using Seek,
						 * the loop will start at the seek
						 * position.
						 */
						if(ALmixer_Channel_List[i].loops != -1)
						{
							ALmixer_Channel_List[i].loops--;
						}
						alSourcePlay(ALmixer_Channel_List[i].alsource);
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "50Testing error: %s\n",
			alGetString(error));				
	}
						continue;
					}
					/* No loops. End play. */
					else
					{
						/* Problem: It seems that when mixing
						 * streamed and predecoded sources,
						 * the previous instance lingers, 
						 * so we need to force remove
						 * the data from the source.
			 			 * The sharing problem
			 			 * occurs when a previous predecoded buffer is played on
			 			 * a source, and then a streamed source is played later
			 			 * on that same source. OpenAL isn't consistently
			 			 * removing the previous buffer so both get played.
			 			 * (Different dists seem to have different quirks.
			 			 * The problem might lead to crashes in the worst case.)
						 */
						/* Additional problem: There is another 
						 * inconsistency among OpenAL distributions.
						 * Both Loki and Creative Windows seem to keep
						 * the buffer queued which requires removing.
						 * But the Creative Macintosh version does
						 * not have any buffer queued after play
						 * and it returns the error: Invalid Enum Value
						 * if I try to unqueue it.
						 * So I'm going to put in a check to see if I
						 * can detect any buffers queued first
				 		 * and then unqueue them if I can see them.
						 * Additional note: The new CoreAudio based 
						 * implementation leaves it's buffer queued
						 * like Loki and Creative Windows. But 
						 * considering all the problems I'm having
						 * with the different distributions, this
						 * check seems reasonable.
						 */
						ALint buffers_still_queued;
						if((error = alGetError()) != AL_NO_ERROR)
						{
							fprintf(stderr, "01Testing errpr before unqueue because getting stuff, for OS X this is expected: %s\n",
									alGetString(error));				
						}
						
						alGetSourcei(
							ALmixer_Channel_List[i].alsource,
							AL_BUFFERS_QUEUED, &buffers_still_queued
						);
						if((error = alGetError()) != AL_NO_ERROR)
						{
		fprintf(stderr, "Error with unqueue, for OS X this is expected: %s\n",
							alGetString(error));
							ALmixer_SetError("Failed detecting unqueued predecoded buffer (expected with OS X): %s",
								alGetString(error) );
							error_flag--;
						}
						if(buffers_still_queued > 0)
						{
							ALboolean clear_succeeded = Internal_DetachBuffersFromSource(ALmixer_Channel_List[i].alsource, ALmixer_Channel_List[i].almixer_data->decoded_all);
							if(AL_FALSE == clear_succeeded)
							{
								error_flag--;
							}
						}
						
						/* Launch callback */
						Invoke_Channel_Done_Callback(i, AL_TRUE);

						Clean_Channel(i);
						/* Subtract counter */
						Is_Playing_global--;


						/* We're done for this loop.
						 * Go to next channel 
						 */
						continue;
					}
					continue;
				}
			} /* End if decoded_all */
			/* For streamed */
			else
			{
				ALint buffers_processed;
				ALint buffers_still_queued;
				ALint current_buffer_id;

				ALuint unqueued_buffer_id;
				ALuint number_of_buffers_to_queue_this_pass = ALmixer_Channel_List[i].almixer_data->num_target_buffers_per_pass;
				ALuint current_count_of_buffer_queue_passes = 0;
				
/*		fprintf(stderr, "For Streamed\n"); */
				
	alGetSourcei(
		ALmixer_Channel_List[i].alsource,
		AL_SOURCE_STATE, &state
				);
#if 0
		/********* Remove this **********/
	switch(state)
	{
                case AL_PLAYING:
				fprintf(stderr, "Channel '%d' is PLAYING\n", i);
				break;
                case AL_PAUSED:
				fprintf(stderr, "Channel '%d' is PAUSED\n",i);
				break;
                case AL_STOPPED:
				fprintf(stderr, "Channel '%d' is STOPPED\n",i);
				break;
				case AL_INITIAL:
				fprintf(stderr, "Channel '%d' is INITIAL\n",i);
				break;
                default:
				fprintf(stderr, "Channel '%d' is UNKNOWN\n",i);
                  break;
       		}
				/******** END REMOVE *******/
#endif	
				/* Get the number of buffers still queued */
				alGetSourcei(
					ALmixer_Channel_List[i].alsource,
					AL_BUFFERS_QUEUED, &buffers_still_queued
				);
				if((error = alGetError()) != AL_NO_ERROR)
				{
					fprintf(stderr, "51Testing error: %s\n",
						alGetString(error));				
				}
				/* Get the number of buffers processed
				 * so we know if we need to refill 
				 */
				 /* WARNING: It looks like Snow Leopard some times crashes on this call under x86_64
				  * typically when I suffer a lot of buffer underruns.
				  */
/*				 fprintf(stderr, "calling AL_BUFFERS_PROCESSED on source:%d", ALmixer_Channel_List[i].alsource); */
				alGetSourcei(
					ALmixer_Channel_List[i].alsource,
					AL_BUFFERS_PROCESSED, &buffers_processed
				);
				if((error = alGetError()) != AL_NO_ERROR)
				{
					fprintf(stderr, "52Testing error: %s\n",
						alGetString(error));				
				}
/*				fprintf(stderr, "finished AL_BUFFERS_PROCESSED, buffers_processed=%d", buffers_processed); */

 /* WTF!!! The Nvidia distribution is failing on the alGetSourcei(source, AL_BUFFER, buf_id) call.
 * I need this call to figure out which buffer OpenAL is currently playing. 
 * It keeps returning an "Invalid Enum" error.
 * This is totally inane! It's a basic query.
 * By the spec, this functionality is not explicitly defined so Nvidia refuses to
 * fix this behavior, even though all other distributions work fine with this.
 * The only workaround for this is for
 * a significant rewrite of my code which requires me to
 * duplicate the OpenAL queued buffers state with my own
 * code and try to derive what the current playing buffer is by indirect observation of 
 * looking at buffers_processed. But of course this has a ton of downsides since my 
 * queries do not give me perfect timing of what OpenAL is actually doing and
 * the fact that some of the distributions seem to have buffer queuing problems
 * with their query results (CoreAudio).  This also means a ton of extra code
 * on my side. The lack of support of a 1 line call has required me to
 * implement yet another entire state machine. <sigh>
 */
#if 0 /* This code will not work until possibly OpenAL 1.1 because of Nvidia */
				/* Get the id to the current buffer playing */
				alGetSourcei(
					ALmixer_Channel_List[i].alsource,
					AL_BUFFER, &current_buffer_id
				);
				if((error = alGetError()) != AL_NO_ERROR)
				{
					fprintf(stderr, "53Testing error: %s\n",
						alGetString(error));				
				}

				/* Before the hard stuff, check to see if the 
				 * current queued AL buffer has changed.
				 * If it has, we should launch a data callback if
				 * necessary
				 */
				if( ((ALuint)current_buffer_id) != 
					ALmixer_Channel_List[i].almixer_data->current_buffer)
				{
					ALmixer_Channel_List[i].almixer_data->current_buffer 
						= (ALuint)current_buffer_id;

					Invoke_Streamed_Channel_Data_Callback(i, ALmixer_Channel_List[i].almixer_data, current_buffer_id);
				}
#else
				/* Only do this if "access_data" was requested (i.e. the circular_buffer!=NULL) 
				 * And if one of the two are true:
				 * Either buffers_processed > 0 (because the current_buffer might have changed)
				 * or if the current_buffer==0 (because we are in an initial state or recovering from 
				 * a buffer underrun)
				 */
				if((ALmixer_Channel_List[i].almixer_data->circular_buffer_queue != NULL) 
				   && (
							(buffers_processed > 0) || (0 == ALmixer_Channel_List[i].almixer_data->current_buffer)
					)
				   )
				{
					ALint k;
					ALuint queue_ret_flag;
					ALubyte is_out_of_sync = 0;
					ALuint my_queue_size = CircularQueueUnsignedInt_Size(ALmixer_Channel_List[i].almixer_data->circular_buffer_queue);
					/* Ugh, I have to deal with signed/unsigned mismatch here. */
					ALint buffers_unplayed_int = buffers_still_queued - buffers_processed;
					ALuint unplayed_buffers;
					if(buffers_unplayed_int < 0)
					{
						unplayed_buffers = 0;
					}
					else
					{
						unplayed_buffers = (ALuint)buffers_unplayed_int;
					}
/*
					fprintf(stderr, "Queue in processed check, before pop, buffers_processed=%d\n", buffers_processed);
					CircularQueueUnsignedInt_Print(ALmixer_Channel_List[i].almixer_data->circular_buffer_queue);
*/
					/* We can't make any determinations solely based on the number of buffers_processed
					 * because currently, we only unqueue 1 buffer per loop. That means if 2 or more
					 * buffers became processed in one loop, the following loop, we would have 
					 * at least that_many-1 buffers_processed (plus possible new processed).
					 * If we tried to just remove 1 buffer from our queue, we would be incorrect
					 * because we would not actually reflect the current playing buffer.
					 * So the solution seems to be to make sure our queue is the same size
					 * as the number of buffers_queued-buffers_processed, and return the head of our queue
					 * as the current playing buffer.
					 */
					/* Also, we have a corner case. When we first start playing or if we have 
					 * a buffer underrun, we have not done a data callback.
					 * In this case, we need to see if there is any new data in our queue
					 * and if so, launch that data callback.
					 */
					/* Warning, this code risks the possibility of no data callback being fired if 
					 * the system is really late (or skipped buffers).
					 */
					
					/* First, let's syncronize our queue with the OpenAL queue */
					#if 0
					fprintf(stderr, "inside, Buffers processed=%d, Buffers queued=%d, my queue=%d\n",
							buffers_processed, buffers_still_queued, my_queue_size);
					#endif	
					is_out_of_sync = 1;
					for(k=0; k<buffers_processed; k++)
					{
						queue_ret_flag = CircularQueueUnsignedInt_PopFront(
							ALmixer_Channel_List[i].almixer_data->circular_buffer_queue);
						if(0 == queue_ret_flag)
						{
							fprintf(stderr, "53 Error popping queue\n");
						}
					}		
					my_queue_size = CircularQueueUnsignedInt_Size(ALmixer_Channel_List[i].almixer_data->circular_buffer_queue);
					/* We have several possibilities we need to handle:
					 * 1) We are in an initial state or underrun and need to do a data callback on the head.
					 * 2) We were out of sync and need to do a new data callback on the new head.
					 * 3) We were not out of sync but just had left over processed buffers which caused us to
					 * fall in this block of code. (Don't do anything.)
					 */
					if( (0 == ALmixer_Channel_List[i].almixer_data->current_buffer) || (1 == is_out_of_sync) )
					{
						if(my_queue_size > 0)
						{
							current_buffer_id = CircularQueueUnsignedInt_Front(
								ALmixer_Channel_List[i].almixer_data->circular_buffer_queue);
							if(0 == current_buffer_id)
							{
								fprintf(stderr, "53a Internal Error, current_buffer_id=0 when it shouldn't be 0\n");
							}
							/*
							else
							{
								fprintf(stderr, "Queue in processed check, after pop\n");
								CircularQueueUnsignedInt_Print(ALmixer_Channel_List[i].almixer_data->circular_buffer_queue);
							}
							 */
							ALmixer_Channel_List[i].almixer_data->current_buffer 
								= (ALuint)current_buffer_id;
							
							#if 0
							/* Remove me...only for checking...doesn't work on Nvidia */
							{
								ALuint real_id;
								alGetSourcei(
										 ALmixer_Channel_List[i].alsource,
										 AL_BUFFER, &real_id
										 );
								alGetError();
								fprintf(stderr, "Callback fired on data buffer=%d, real_id shoud be=%d\n", current_buffer_id, real_id);
							}
							#endif
							Invoke_Streamed_Channel_Data_Callback(i, ALmixer_Channel_List[i].almixer_data, current_buffer_id);
						}
						else
						{
/*
							fprintf(stderr, "53b, Notice/Warning:, OpenAL queue has been depleted.\n");
							PrintQueueStatus(ALmixer_Channel_List[i].alsource);
*/
							/* In this case, we might either be in an underrun or finished with playback */
							ALmixer_Channel_List[i].almixer_data->current_buffer = 0;
						}
					}
				}
#endif
					
				
				
		/* Just a test - remove 
				if( ALmixer_Channel_List[i].loops > 0)
				{
					fprintf(stderr, ">>>>>>>>>>>>>>>Loops = %d\n", 
						ALmixer_Channel_List[i].loops);
				}
		*/
#if 0
				fprintf(stderr, "Buffers processed = %d\n", buffers_processed);
				fprintf(stderr, "Buffers queued= %d\n", buffers_still_queued);
#endif
				/* We've used up a buffer so we need to unqueue and replace */
				/* Okay, it gets more complicated here:
				 * We need to Queue more data 
				 * if buffers_processed > 0  or 
				 * if num_of_buffers_in_use < NUMBER_OF_QUEUE_BUFFERS
				 * but we don't do this if at EOF,
				 * except when there is looping
				 */

				/* NEW FEATURE: Try to queue up more buffers per pass, allowing the size of the buffer to be decoupled. */
				/* TODO: Optimization: If number of available buffers (max_buffers-buffers_in_use), adjust the number of buffers to queue for this pass. */
				/* I would benefit from a clearer flag that tells me whether we're in an underrun. 
				 * I think current_buffer only works if data callbacks are enabled.
				 * So I'll look at for AL_STOPPED which might give me false positives, depending on the code above.
				 * But I think/hope the code below will deal with it correctly.
				 * If in an underrun, queue up at least startup_buffers.
				 */
				if(AL_STOPPED == state)
				{
					number_of_buffers_to_queue_this_pass = ALmixer_Channel_List[i].almixer_data->num_startup_buffers;
/*					fprintf(stderr, "assuming underrun condition, using num_startup_buffers=%d\n", number_of_buffers_to_queue_this_pass); */
				}	

				/* Don't bother to check to make sure the number_of_buffers_to_queue_this_pass does not exceed the maximum number of buffers because of the logic hack bug. */
				/* Logic Hack/Bug: In adding the number_of_buffers_to_queue_this_pass, I discovered the for-loop needs to be more decoupled.
				 * The loop still needs to be entered because unqueuing and completion callbacks and possibly other state processing are also done. 
				 * So we always need to claim to queue one buffer. (The code already checks for max_queue_buffers.)
				 */
				if(0 == number_of_buffers_to_queue_this_pass)
				{
					number_of_buffers_to_queue_this_pass = 1;
				}
				for(current_count_of_buffer_queue_passes=0; current_count_of_buffer_queue_passes<number_of_buffers_to_queue_this_pass; current_count_of_buffer_queue_passes++)
				{
/*					fprintf(stderr, "current_count_of_buffer_queue_passes:%d\n",  current_count_of_buffer_queue_passes); */

					/* Because I introduced this for-loop, I think I need to regrab the number of processed buffers because
					 * the number may now be stale from previous iterations. I suppose I could do it at the end of the loop,
					 * but the logic flow is already too complicated to ensure that the block is being hit.
					 */
					/* Get the number of buffers processed
					*/
					alGetSourcei(
						ALmixer_Channel_List[i].alsource,
						AL_BUFFERS_PROCESSED, 
						&buffers_processed
					);
					if((error = alGetError()) != AL_NO_ERROR)
					{
						fprintf(stderr, "59aTestingError, Can't get buffers_processed: %s\n",
							alGetString(error));				
					}
					

					/* For this to work, we must rely on EVERYTHING
					 * else to unset the EOF if there is looping.
					 * Remember, even Play() must do this
					 */
					
					/* If not EOF, then we are still playing.
					 * Inside, we might find num_of_buffers < NUM...QUEUE_BUF..
					 * or buffers_process > 0 
					 * in which case we queue up.
					 * We also might find no buffers we need to fill,
					 * in which case we just keep going
					 */
					if (!ALmixer_Channel_List[i].almixer_data->eof)
					{
						ALuint bytes_returned;
						/* We have a priority. We first must assign
						 * unused buffers in reserve. If there is nothing
						 * left, then we may unqueue buffers. We can't
						 * do it the other way around because we will
						 * lose the pointer to the unqueued buffer 
						 */
						if(ALmixer_Channel_List[i].almixer_data->num_buffers_in_use 
							< ALmixer_Channel_List[i].almixer_data->max_queue_buffers) 
						{		
	#if 0
							fprintf(stderr, "Getting more data in NOT_EOF and num_buffers_in_use (%d) < max_queue (%d)\n", 
									ALmixer_Channel_List[i].almixer_data->num_buffers_in_use,
									ALmixer_Channel_List[i].almixer_data->max_queue_buffers);
	#endif
							/* Going to add an unused packet.
							 * Grab next packet */
							bytes_returned = GetMoreData(
								ALmixer_Channel_List[i].almixer_data,
								ALmixer_Channel_List[i].almixer_data->buffer[
									ALmixer_Channel_List[i].almixer_data->num_buffers_in_use]
								);
						}
						/* For processed > 0 */
						else if(buffers_processed > 0)
						{
							/* Unqueue only 1 buffer for now.
							 * If there are more than one,
							 * let the next Update pass deal with it
							 * so we don't stall the program for too long.
							 */
	#if 0
					fprintf(stderr, "About to Unqueue, Buffers processed = %d\n", buffers_processed);
					fprintf(stderr, "Buffers queued= %d\n", buffers_still_queued);
					fprintf(stderr, "Unqueuing a buffer\n");
	#endif
							alSourceUnqueueBuffers(
								ALmixer_Channel_List[i].alsource,
								1, &unqueued_buffer_id
							);
							if((error = alGetError()) != AL_NO_ERROR)
							{
					fprintf(stderr, "Error with unqueue: %s, buffer id is %d",
								alGetString(error), unqueued_buffer_id);
								ALmixer_SetError("Unqueue buffer failed: %s",
									alGetString(error) );
								error_flag--;
							}
	/*
							fprintf(stderr, "Right after unqueue...");
							PrintQueueStatus(ALmixer_Channel_List[i].alsource);
							fprintf(stderr, "Getting more data for NOT_EOF, max_buffers filled\n");
	*/
							/* Grab unqueued packet */
							bytes_returned = GetMoreData(
								ALmixer_Channel_List[i].almixer_data,
								unqueued_buffer_id);
						}
						/* We are still streaming, but currently
						 * don't need to fill any buffers */
						else
						{
							/* Might want to check state */
							/* In case the playback stopped,
							 * we need to resume 
							 * a.k.a. buffer underrun
							 */
							#if 1 
							/* Try not refetching the state here because I'm getting a duplicate
							 buffer playback (hiccup) */
							alGetSourcei(
								ALmixer_Channel_List[i].alsource,
								AL_SOURCE_STATE, &state
							);
							if((error = alGetError()) != AL_NO_ERROR)
							{
								fprintf(stderr, "54bTesting error: %s\n",
									alGetString(error));				
							}
							/* Get the number of buffers processed
							 */
							alGetSourcei(
								ALmixer_Channel_List[i].alsource,
								AL_BUFFERS_PROCESSED, 
								&buffers_processed
							);
							if((error = alGetError()) != AL_NO_ERROR)
							{
								fprintf(stderr, "54cError, Can't get buffers_processed: %s\n",
									alGetString(error));				
							}
	#endif
							if(AL_STOPPED == state)
							{
								/* Resuming in not eof, but nothing to buffer */

								/* Okay, here's another lately discovered problem:
								 * I can't find it in the spec, but for at least some of the 
								 * implementations, if I call play on a stopped source that 
								 * has processed buffers, all those buffers get marked as unprocessed
								 * on alSourcePlay. So if I had a queue of 25 with 24 of the buffers
								 * processed, on resume, the earlier 24 buffers will get replayed,
								 * causing a "hiccup" like sound in the playback.
								 * To avoid this, I must unqueue all processed buffers before
								 * calling play. But to complicate things, I need to worry about resyncing
								 * the circular queue with this since I designed this thing
								 * with some correlation between the two. However, I might
								 * have already handled this, so I will try writing this code without
								 * syncing for now.
								 * There is currently an assumption that a buffer 
								 * was queued above so I actually have something
								 * to play.
								 */
								ALint temp_count;
	#if 0
								fprintf(stderr, "STOPPED1, need to clear processed=%d, status is:\n", buffers_processed);
								PrintQueueStatus(ALmixer_Channel_List[i].alsource);
	#endif
								for(temp_count=0; temp_count<buffers_processed; temp_count++)
								{
									alSourceUnqueueBuffers(
										ALmixer_Channel_List[i].alsource,
										1, &unqueued_buffer_id
									);
									if((error = alGetError()) != AL_NO_ERROR)
									{
										fprintf(stderr, "55aTesting error: %s\n",
											alGetString(error));				
										error_flag--;
									}
								}
	#if 0
								fprintf(stderr, "After unqueue clear...:\n");
								PrintQueueStatus(ALmixer_Channel_List[i].alsource);
	#endif						
								/* My assertion: We are STOPPED but not EOF.
								 * This means we have a buffer underrun.
								 * We just cleared out the unqueued buffers.
								 * So we need to reset the mixer_data to reflect we have
								 * no buffers in queue.
								 * We need to GetMoreData and then queue up the data.
								 * Then we need to resume playing.
								 */
	#if 0
								int buffers_queued;
								alGetSourcei(
											 ALmixer_Channel_List[i].alsource,
											 AL_BUFFERS_QUEUED, 
											 &buffers_queued
											 );
								
								if((error = alGetError()) != AL_NO_ERROR)
								{
									fprintf(stderr, "Error in PrintQueueStatus, Can't get buffers_queued: %s\n",
											alGetString(error));				
								} 
								assert(buffers_queued == 0);
								fprintf(stderr, "buffer underrun: buffers_queued:%d\n", buffers_queued);
	#endif

								/* Reset the number of buffers in use to 0 */
								ALmixer_Channel_List[i].almixer_data->num_buffers_in_use = 0;

								/* Get more data and put it in the first buffer */
								if( ALmixer_Channel_List[i].almixer_data->buffer )
								{
									// Verifying "buffer" is a quick-fix to avoid dereferencing a NULL pointer.
									// A better solution is to find WHY this is happening in the first place.
									bytes_returned = GetMoreData(
										ALmixer_Channel_List[i].almixer_data,
										ALmixer_Channel_List[i].almixer_data->buffer[0]
									);
								}
								else
								{
									bytes_returned = 0;
								}

								/* NOTE: We might want to look for EOF and handle it here.
								 * Currently, I just let the next loop handle it which seems to be working.
								 */
								if(bytes_returned > 0)
								{
									/* Queue up the new data */
									alSourceQueueBuffers(
														 ALmixer_Channel_List[i].alsource,
														 1, 
														 &ALmixer_Channel_List[i].almixer_data->buffer[0]
									);
									if((error = alGetError()) != AL_NO_ERROR)
									{
										fprintf(stderr, "56e alSourceQueueBuffers error: %s\n",
												alGetString(error));				
									}
									/* Increment the number of buffers in use */
									ALmixer_Channel_List[i].almixer_data->num_buffers_in_use++;
									

									/* We need to empty and update the circular buffer queue if it is in use */
									if(ALmixer_Channel_List[i].almixer_data->circular_buffer_queue != NULL)
									{
										ALuint queue_ret_flag;
										CircularQueueUnsignedInt_Clear(ALmixer_Channel_List[i].almixer_data->circular_buffer_queue);
										queue_ret_flag = CircularQueueUnsignedInt_PushBack(
																						   ALmixer_Channel_List[i].almixer_data->circular_buffer_queue, 
																						   ALmixer_Channel_List[i].almixer_data->buffer[0]
																						   );	
										if(0 == queue_ret_flag)
										{
											fprintf(stderr, "56fSerious internal error: CircularQueue could not push into queue.\n");
											ALmixer_SetError("Serious internal error: CircularQueue failed to push into queue");
										}
									}
									
									


									/* Resume playback from underrun */
									alSourcePlay(ALmixer_Channel_List[i].alsource);
									if((error = alGetError()) != AL_NO_ERROR)
									{
										fprintf(stderr, "55Tbesting error: %s\n",
											alGetString(error));				
									}
								}

							}
							/* Let's escape to the next loop.
							 * All code below this point is for queuing up 
							 */
							/*
							   fprintf(stderr, "Entry: Nothing to do...continue\n\n");
							 */				
							continue;
						}
						/* We now know we have to fill an available
						 * buffer.
						 */
						
						/* In the previous branch, we just grabbed more data.
						 * Let's check it to make sure it's okay,
						 * and then queue it up
						 */
						/* This check doesn't work anymore because it is now ALuint */
					#if 0
						if(-1 == bytes_returned)
						{
							/* Problem occurred...not sure what to do */
							/* Go to next loop? */
							error_flag--;
							/* Set the eof flag to force a quit so 
							 * we don't get stuck in an infinite loop
							 */
							ALmixer_Channel_List[i].almixer_data->eof = 1;
							continue;
						}
					#endif
						/* This is a special case where we've run
						 * out of data. We should check for loops
						 * and get more data. If there is no loop,
						 * then do nothing and wait for future
						 * update passes to handle the EOF.
						 * The advantage of handling the loop here 
						 * instead of waiting for play to stop is
						 * that we should be able to keep the buffer
						 * filled.
						 */
					#if 0
						else if(0 == bytes_returned)
					#endif
						if(0 == bytes_returned)
						{
							/*
					fprintf(stderr, "We got 0 bytes from reading. Checking for loops\n");
					*/
							/* Check for loops */
							if( ALmixer_Channel_List[i].loops != 0 )
							{
								/* We have to loop, so rewind
								 * and fetch more data 
								 */
								/*
					fprintf(stderr, "Rewinding data\n");
					*/
								if(0 == Sound_Rewind(
									ALmixer_Channel_List[i].almixer_data->sample))
								{
					fprintf(stderr, "Rewinding failed\n");
									ALmixer_SetError( Sound_GetError() );
									ALmixer_Channel_List[i].loops = 0;
									error_flag--;
									/* We'll continue on because we do have some valid data */
									continue;
								}
								/* Remember to reset the data->eof flag */
								ALmixer_Channel_List[i].almixer_data->eof = 0;
								if(ALmixer_Channel_List[i].loops > 0)
								{
									ALmixer_Channel_List[i].loops--;
								}
								/* Try grabbing another packet now.
								 * Since we may have already unqueued a
								 * buffer, we don't want to lose it.
								 */
								if(ALmixer_Channel_List[i].almixer_data->num_buffers_in_use 
									< ALmixer_Channel_List[i].almixer_data->max_queue_buffers) 
								{
									/*
					fprintf(stderr, "We got %d bytes from reading loop. Filling unused packet\n", bytes_returned);
					*/
									/* Grab next packet */
									bytes_returned = GetMoreData(
										ALmixer_Channel_List[i].almixer_data,
											ALmixer_Channel_List[i].almixer_data->buffer[
											ALmixer_Channel_List[i].almixer_data->num_buffers_in_use]
									);
									/*
					fprintf(stderr, "We reread %d bytes into unused packet\n", bytes_returned);
					*/
								}
								/* Refilling unqueued packet */
								else
								{
									/*
					fprintf(stderr, "We got %d bytes from reading loop. Filling unqueued packet\n", bytes_returned);
					*/
									/* Grab next packet */
									bytes_returned = GetMoreData(
										ALmixer_Channel_List[i].almixer_data,
										unqueued_buffer_id);
									/*
					fprintf(stderr, "We reread %d bytes into unqueued packet\n", bytes_returned);
					*/
								}	
								/* Another error check */
								/*
								if(bytes_returned <= 0)
								*/
								if(0 == bytes_returned)
								{
			fprintf(stderr, "??????????ERROR\n");
									ALmixer_SetError("Could not loop because after rewind, no data could be retrieved");
									/* Problem occurred...not sure what to do */
									/* Go to next loop? */
									error_flag--;
									/* Set the eof flag to force a quit so 
									 * we don't get stuck in an infinite loop
									 */
									ALmixer_Channel_List[i].almixer_data->eof = 1;
									continue;
								}	
								/* We made it to the end. We still need 
								 * to BufferData, so let this branch
								 * fall into the next piece of 
								 * code below which will handle that 
								 */

								
							} /* END loop check */
							else
							{
								/* No more loops to do. 
								 * EOF flag should be set.
								 * Just go to next loop and
								 * let things be handled correctly
								 * in future update calls
								 */
	/*
							fprintf(stderr, "SHOULD BE EOF\n");
								
								PrintQueueStatus(ALmixer_Channel_List[i].alsource);
	*/							
								continue;
							}
						} /* END if bytes_returned == 0 */
	/********* Possible trouble point. I might be queueing empty buffers on the mac. 
	 * This check doesn't say if the buffer is valid. Only the EOF assumption is a clue at this point 
	 */
						/* Fall here */
						/* Everything is normal. We aren't
						 * at an EOF, but need to simply
						 * queue more data. The data is already checked for good, 
						 * so queue it up */
						if(ALmixer_Channel_List[i].almixer_data->num_buffers_in_use 
							< ALmixer_Channel_List[i].almixer_data->max_queue_buffers) 
						{
							/* Keep count of how many buffers we have 
							 * to queue so we can return the value 
							 */
							retval++;
							/*
							fprintf(stderr, "NOT_EOF???, about to Queue more data for num_buffers (%d) < max_queue (%d)\n",
									ALmixer_Channel_List[i].almixer_data->num_buffers_in_use,
									ALmixer_Channel_List[i].almixer_data->max_queue_buffers);
							*/		
							alSourceQueueBuffers(
								ALmixer_Channel_List[i].alsource,
								1, 
								&ALmixer_Channel_List[i].almixer_data->buffer[ 
									ALmixer_Channel_List[i].almixer_data->num_buffers_in_use]
							);
							if((error = alGetError()) != AL_NO_ERROR)
							{
								fprintf(stderr, "56Testing error: %s\n",
									alGetString(error));				
							}
							/* This is part of the hideous Nvidia workaround. In order to figure out
							 * which buffer to show during callbacks (for things like
							 * o-scopes), I must keep a copy of the buffers that are queued in my own
							 * data structure. This code will be called only if
							 * "access_data" was set, indicated by whether the queue is NULL.
							 */
							if(ALmixer_Channel_List[i].almixer_data->circular_buffer_queue != NULL)
							{
								ALuint queue_ret_flag;
	/* 				fprintf(stderr, "56d: CircularQueue_PushBack.\n"); */
								queue_ret_flag = CircularQueueUnsignedInt_PushBack(
									ALmixer_Channel_List[i].almixer_data->circular_buffer_queue, 
									ALmixer_Channel_List[i].almixer_data->buffer[ALmixer_Channel_List[i].almixer_data->num_buffers_in_use]
								);	
								if(0 == queue_ret_flag)
								{
									fprintf(stderr, "56aSerious internal error: CircularQueue could not push into queue.\n");
									ALmixer_SetError("Serious internal error: CircularQueue failed to push into queue");
								}
								/*
								else
								{
									CircularQueueUnsignedInt_Print(ALmixer_Channel_List[i].almixer_data->circular_buffer_queue);
								}
								 */
							}
						}
						/* for processed > 0 */
						else
						{
							/* Keep count of how many buffers we have 
							 * to queue so we can return the value 
							 */
							retval++;
	/*
							fprintf(stderr, "NOT_EOF, about to Queue more data for filled max_queue (%d)\n",
									ALmixer_Channel_List[i].almixer_data->max_queue_buffers);
	*/
							alSourceQueueBuffers(
								ALmixer_Channel_List[i].alsource,
								1, &unqueued_buffer_id);
							if((error = alGetError()) != AL_NO_ERROR)
							{
								ALmixer_SetError("Could not QueueBuffer: %s",
									alGetString(error) );
								error_flag--;
								continue;
							}
							/* This is part of the hideous Nvidia workaround. In order to figure out
							 * which buffer to show during callbacks (for things like
							 * o-scopes), I must keep a copy of the buffers that are queued in my own
							 * data structure. This code will be called only if
							 * "access_data" was set, indicated by whether the queue is NULL.
							 */
							if(ALmixer_Channel_List[i].almixer_data->circular_buffer_queue != NULL)
							{
								ALuint queue_ret_flag;
	/* 				fprintf(stderr, "56e: CircularQueue_PushBack.\n"); */
								queue_ret_flag = CircularQueueUnsignedInt_PushBack(
									ALmixer_Channel_List[i].almixer_data->circular_buffer_queue, 
									unqueued_buffer_id
								);
								if(0 == queue_ret_flag)
								{
									fprintf(stderr, "56bSerious internal error: CircularQueue could not push into queue.\n");
									ALmixer_SetError("Serious internal error: CircularQueue failed to push into queue");
								}
	#if 0
								else
								{
									CircularQueueUnsignedInt_Print(ALmixer_Channel_List[i].almixer_data->circular_buffer_queue);
								}
	#endif
							}
						}
						/* If we used an available buffer queue,
						 * then we need to update the number of them in use
						 */
						if(ALmixer_Channel_List[i].almixer_data->num_buffers_in_use 
							< ALmixer_Channel_List[i].almixer_data->max_queue_buffers) 
						{
							/* Increment the number of buffers in use */
							ALmixer_Channel_List[i].almixer_data->num_buffers_in_use++;
						}
						/* Might want to check state */
						/* In case the playback stopped,
						 * we need to resume */
						#if 1 
						/* Try not refetching the state here because I'm getting a duplicate
							 buffer playback (hiccup) */
						alGetSourcei(
							ALmixer_Channel_List[i].alsource,
							AL_SOURCE_STATE, &state
						);
						if((error = alGetError()) != AL_NO_ERROR)
						{
							fprintf(stderr, "57bTesting error: %s\n",
								alGetString(error));				
						}
						/* Get the number of buffers processed
						 */
						alGetSourcei(
							ALmixer_Channel_List[i].alsource,
							AL_BUFFERS_PROCESSED, 
							&buffers_processed
						);
						if((error = alGetError()) != AL_NO_ERROR)
						{
							fprintf(stderr, "57cError, Can't get buffers_processed: %s\n",
								alGetString(error));				
						}
						#endif
						if(AL_STOPPED == state)
						{
						/*
							fprintf(stderr, "Resuming in not eof\n");
						*/
								/* Okay, here's another lately discovered problem:
								 * I can't find it in the spec, but for at least some of the 
								 * implementations, if I call play on a stopped source that 
								 * has processed buffers, all those buffers get marked as unprocessed
								 * on alSourcePlay. So if I had a queue of 25 with 24 of the buffers
								 * processed, on resume, the earlier 24 buffers will get replayed,
								 * causing a "hiccup" like sound in the playback.
								 * To avoid this, I must unqueue all processed buffers before
								 * calling play. But to complicate things, I need to worry about resyncing
								 * the circular queue with this since I designed this thing
								 * with some correlation between the two. However, I might
								 * have already handled this, so I will try writing this code without
								 * syncing for now.
								 * There is currently an assumption that a buffer 
								 * was queued above so I actually have something
								 * to play.
								 */
								ALint temp_count;
	/*
								fprintf(stderr, "STOPPED2, need to clear processed, status is:\n");
								PrintQueueStatus(ALmixer_Channel_List[i].alsource);
	*/

								for(temp_count=0; temp_count<buffers_processed; temp_count++)
								{
									alSourceUnqueueBuffers(
										ALmixer_Channel_List[i].alsource,
										1, &unqueued_buffer_id
									);
									if((error = alGetError()) != AL_NO_ERROR)
									{
										fprintf(stderr, "58aTesting error: %s\n",
											alGetString(error));				
										error_flag--;
									}
								}
	/*
								fprintf(stderr, "After unqueue clear...:\n");
								PrintQueueStatus(ALmixer_Channel_List[i].alsource);
	*/

								alSourcePlay(ALmixer_Channel_List[i].alsource);
								if((error = alGetError()) != AL_NO_ERROR)
								{
									fprintf(stderr, "55Tbesting 8rror: %s\n",
										alGetString(error));				
								}
						}
						continue;
					} /* END if( ! eof) */
					/* We have hit EOF in the SDL_Sound sample and there
					 * are no more loops. However, there may still be
					 * buffers in the OpenAL queue which still need to
					 * be played out. The following body of code will
					 * determine if play is still happening or 
					 * initiate the stop/cleanup sequenece.
					 */
					else
					{
						/* Let's continue to remove the used up
						 * buffers as they come in. */
						if(buffers_processed > 0)
						{
							ALint temp_count;
							/* Do as a for-loop because I don't want
							 * to have to create an array for the 
							 * unqueued_buffer_id's
							 */
							for(temp_count=0; temp_count<buffers_processed; temp_count++)
							{
								/*
								fprintf(stderr, "unqueuing remainder, %d\n", temp_count);
								*/
								alSourceUnqueueBuffers(
									ALmixer_Channel_List[i].alsource,
									1, &unqueued_buffer_id
								);
								if((error = alGetError()) != AL_NO_ERROR)
								{
									fprintf(stderr, "59Testing error: %s\n",
											alGetString(error));				
								}
							}
							/*
							fprintf(stderr, "done unqueuing remainder for this loop, %d\n", temp_count);
							*/

							/* Need to update counts since we removed everything. 
							 * If we don't update the counts here, we end up in the
							 *	"Shouldn't be here section, but maybe it's okay due to race conditions"
							 */
							alGetSourcei(
										 ALmixer_Channel_List[i].alsource,
										 AL_BUFFERS_QUEUED, &buffers_still_queued
										 );
							if((error = alGetError()) != AL_NO_ERROR)
							{
								fprintf(stderr, "5100Testing error: %s\n",
										alGetString(error));				
							}
							/* Get the number of buffers processed
								* so we know if we need to refill 
								*/
							alGetSourcei(
										 ALmixer_Channel_List[i].alsource,
										 AL_BUFFERS_PROCESSED, &buffers_processed
										 );
							if((error = alGetError()) != AL_NO_ERROR)
							{
								fprintf(stderr, "5200Testing error: %s\n",
										alGetString(error));				
							}
						}


						/* Else if buffers_processed == 0
						 * and buffers_still_queued == 0.
						 * then we check to see if the source
						 * is still playing. Quit if stopped
						 * We shouldn't need to worry about
						 * looping because that should have
						 * been handled above.
						 */
						if(0 == buffers_still_queued)
						{
							/* Make sure playback has stopped before
							 * we shutdown.
							 */
							alGetSourcei(
								ALmixer_Channel_List[i].alsource,
								AL_SOURCE_STATE, &state
							);
		if((error = alGetError()) != AL_NO_ERROR)
		{
			fprintf(stderr, "60Testing error: %s\n",
				alGetString(error));				
		}
							if(AL_STOPPED == state)
							{
								ALmixer_Channel_List[i].almixer_data->num_buffers_in_use  = 0;
								/* Playback has ended. 
								 * Loop if necessary, or launch callback
								 * and clear channel (or clear channel and
								 * then launch callback?)
								 * Update: Need to do callback first because I reference the mixer_data and source
								 */

								/* Launch callback */
								Invoke_Channel_Done_Callback(i, AL_TRUE);

								Clean_Channel(i);
								/* Subtract counter */
								Is_Playing_global--;
		
		
								/* We're done for this loop.
								 * Go to next channel 
								 */
								
								/* I used to call continue here, but continue isn't safe anymore because we are in the number_of_queue_buffers_per_pass loop now.
								 * We should break, but we need to be careful that there is no code that is run once we break that wasn't run before.
								 * We want to make sure we go immediately to the next iteration in the channel loop.
								 */
								 break;
							}
						} /* End end-playback */
						else
						{
							/* Need to run out buffer */
				#if 1
							/* Might want to check state */
							/* In case the playback stopped,
							 * we need to resume */
							alGetSourcei(
								ALmixer_Channel_List[i].alsource,
								AL_SOURCE_STATE, &state
							);
		if((error = alGetError()) != AL_NO_ERROR)
		{
			fprintf(stderr, "61Testing error: %s\n",
				alGetString(error));				
		}
							if(AL_STOPPED == state)
							{
								/*
			fprintf(stderr, "Shouldn't be here. %d Buffers still in queue, but play stopped. This might be correct though because race conditions could have caused the STOP to happen right after our other tests...Checking queue status...\n", buffers_still_queued);
			*/
	/*
								PrintQueueStatus(ALmixer_Channel_List[i].alsource);
	*/							
								/* Rather than force unqueuing the buffer, let's see if
								* setting the buffer to none works (the OpenAL 1.0 
								* Reference Annotation suggests this should work).								 
								*/
								ALboolean clear_succeeded = Internal_DetachBuffersFromSource(ALmixer_Channel_List[i].alsource, ALmixer_Channel_List[i].almixer_data->decoded_all);
								if(AL_FALSE == clear_succeeded)
								{
									error_flag--;
								}
	/*							
								PrintQueueStatus(ALmixer_Channel_List[i].alsource);
	*/
								/* This doesn't work because in some cases, I think
								 * it causes the sound to be replayed
								 */
								/*
								fprintf(stderr, "Resuming in eof (trying to run out buffers\n");
								alSourcePlay(ALmixer_Channel_List[i].alsource);
			*/
							}
				#endif
						} /* End trap section */
					} /* End POST-EOF use-up buffer section */

				} /* END NEW number of queue buffers per pass */

			} /* END Streamed section */
		} /* END channel in use */
	} /* END for-loop for each channel */

#ifdef ENABLE_ALMIXER_ALC_SYNC	
	alcProcessContext(alcGetCurrentContext());
	if((error = alGetError()) != AL_NO_ERROR)
	{
		fprintf(stderr, "62Testing error: %s\n",
			alGetString(error));				
	}
#endif

#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	/* Return the number of errors */
	if(error_flag < 0) 
	{
		return error_flag;
	}
	/* Return the number of buffers that were queued */
	return retval;
}

#ifdef ENABLE_PARANOID_SIGNEDNESS_CHECK
/* This is only here so we can call SDL_OpenAudio() */
static void my_dummy_audio_callback(void* userdata, ALbyte* stream, int len)
{
}
#endif




#ifdef ENABLE_ALMIXER_THREADS
/* We might need threads. We
 * must constantly poll OpenAL to find out
 * if sound is being streamed, if play has 
 * ended, etc. Without threads, this must
 * be explicitly done by the user.
 * We could try to do it for them if we 
 * finish the threads.
 */

static int Stream_Data_Thread_Callback(void* data)
{
	ALint retval;
	
	while(AL_TRUE == g_StreamThreadEnabled)
	{
		retval = Update_ALmixer(data);
		/* 0 means that nothing needed updating and 
		 * the function returned quickly
		 */
		if(0 == retval)
		{
			/* Let's be nice and make the thread sleep since
			 * little work was done in update
			 */
			/* Make sure times are multiples of 10
			 * for optimal performance and accuracy in Linux
			 */
			ALmixer_Delay(10);
		}
		else
		{
			/* should I also be sleeping/yielding here? */
			ALmixer_Delay(0);
		}
	}
	/*
fprintf(stderr, "Thread is closing\n");
*/
	return 0;
}
#endif /* End of ENABLE_ALMIXER_THREADS */


/* SDL/SDL_mixer returns -1 on error and 0 on success.
 * I actually prefer false/true conventions (SDL_Sound/OpenAL/GL)
 * so SDL_mixer porting people beware.
 * Warning: SDL_QuitSubSystem(SDL_INIT_AUDIO) is called which
 * means the SDL audio system will be disabled. It will not
 * be restored (in case SDL is not actually being used) so
 * the user will need to restart it if they need it after
 * OpenAL shuts down.
 */
ALboolean ALmixer_Init(ALuint frequency, ALuint num_sources, ALuint refresh)
{
	ALCdevice* dev;
	ALCcontext* context;
	ALint i;
	ALenum error;
	ALuint* source;
    static int initErrorDisplayed = 0;

#ifdef USING_LOKI_AL_DIST
	/* The Loki dist requires that I set both the 
	 * device and context frequency values separately
	 */
	/* Hope this won't overflow */
	char device_string[256];
#endif
	
	/* (Venting frustration) Damn it! Nobody bothered
	 * documenting how you're supposed to use an attribute
	 * list. In fact, the not even the Loki test program
	 * writers seem to know because they use it inconsistently.
	 * For example, how do you terminate that attribute list?
	 * The Loki test code does it 3 different ways. They 
	 * set the last value to 0, or they set it to ALC_INVALID, 
	 * or they set two final values: ALC_INVALID, 0
	 * In Loki, 0 and ALC_INVALID happen to be the same,
	 * but with Creative Labs ALC_INVALID is -1. 
	 * So something's going to break. Loki's source
	 * code says to terminate with ALC_INVALID. But I 
	 * don't know if that's really true, or it happens 
	 * to be a coinicidence because it's defined to 0.
	 * Creative provides no source code, so I can't look at how 
	 * they terminate it. 
	 * So this is really, really ticking me off...
	 * For now, I'm going to use ALC_INVALID.
	 * (Update...after further review of the API spec,
	 * it seems that a NULL terminated string is the correct
	 * termination value to use, so 0 it is.)
	 */
#if 0
	ALint attrlist[] = { 
		ALC_FREQUENCY, ALMIXER_DEFAULT_FREQUENCY,
		/* Don't know anything about these values.
		 * Trust defaults? */
		/* Supposed to be the refresh rate in Hz.
		 * I think 15-120 are supposed to be good 
		 * values. Though I haven't gotten any effect except
		 * for one strange instance on a Mac. But it was 
		 * unrepeatable.
		 */
	#if 0
		ALC_REFRESH, 15,
	#endif	
		/* Sync requires a alcProcessContext() call
		 * for every cycle. By default, this is
		 * not used and the value is AL_FALSE
		 * because it will probably perform
		 * pretty badly for me.
		 */
#ifdef ENABLE_ALMIXER_ALC_SYNC	
		ALC_SYNC, AL_TRUE,
#else
		ALC_SYNC, AL_FALSE,
#endif
		/* Looking at the API spec, it implies
		 * that the list be a NULL terminated string
		 * so it's probably not safe to use ALC_INVALID
		*/
		/*
		ALC_INVALID };
		*/
		'\0'};
#endif
	/* Redo: I'm going to allow ALC_REFRESH to be set.
	 * However, if no value is specified, I don't
	 * want it in the list so I can get the OpenAL defaults
	 */
	ALint attrlist[7];
	ALsizei current_attrlist_index = 0;

#ifdef ENABLE_PARANOID_SIGNEDNESS_CHECK
	/* More problems: I'm getting bit by endian/signedness issues on
	 * different platforms. I can find the endianess easily enough,
	 * but I don't know how to determine what the correct signedness
	 * is (if such a thing exists). I do know that if I try using
	 * unsigned on OSX with an originally signed sample, I get 
	 * distortion. However, I don't have any native unsigned samples
	 * to test. But I'm assuming that the platform must be in the 
	 * correct signedness no matter what.
	 * I can either assume everybody is signed, or I can try to 
	 * determine the value. If I try to determine the values,
	 * I think my only ability to figure it out will be to open
	 * SDL_Audio, and read what the obtained settings were.
	 * Then shutdown everything. However, I don't even know how 
	 * reliable this is.
	 * Update: I think I resolved the issues...forgot to update 
	 * these comments when it happened. I should check the revision control
	 * log... Anyway, I think the issue was partly related to me not 
	 * doing something correctly with the AudioInfo or some kind 
	 * of stupid endian bug in my code, and weirdness ensued. Looking at the
	 * revision control, I think I might have assumed that SDL_Sound would 
	 * do the right thing with a NULL AudioInfo, but I was incorrect,
	 * and had to fill one out myself.
	 */
	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;
#endif


	/* Make sure ALmixer isn't already initialized */
	if(ALmixer_Initialized)
	{
		return AL_FALSE;
	}

#ifdef ALMIXER_COMPILE_WITHOUT_SDL
	ALmixer_InitTime();

	/* Note: The pool may have been created on previous Init's */
	/* I leave the pool allocated allocated in case the user wants
	 * to read the pool in case of a failure (such as in this function).
	 * This is not actually a leak.
	 */
	if(NULL == s_ALmixerErrorPool)
	{
		s_ALmixerErrorPool = TError_CreateErrorPool();
	}
	if(NULL == s_ALmixerErrorPool)
	{
		return AL_FALSE;
	}
/*
		fprintf(stderr, "tError Test0\n");
		ALmixer_SetError("Initing (and testing SetError)");
		fprintf(stderr, "tError Test1: %s\n", ALmixer_GetError());
		fprintf(stderr, "tError Test2: %s\n", ALmixer_GetError());
*/
#endif


	/* Set the defaults */
/*
	attrlist[0] = ALC_FREQUENCY;
	attrlist[1] = ALMIXER_DEFAULT_FREQUENCY;
	attrlist[2] = ALC_SYNC;
#ifdef ENABLE_ALMIXER_ALC_SYNC	
	attrlist[3] = ALC_TRUE;
#else
	attrlist[3] = ALC_FALSE;
#endif
*/	
	/* Set frequency value if it is not 0 */
	if(0 != frequency)
	{
		attrlist[current_attrlist_index] = ALC_FREQUENCY;
		current_attrlist_index++;
		attrlist[current_attrlist_index] = (ALint)frequency;
		current_attrlist_index++;
	}

#ifdef ENABLE_ALMIXER_ALC_SYNC	
		attrlist[current_attrlist_index] = ALC_SYNC;
		current_attrlist_index++;
		attrlist[current_attrlist_index] = ALC_TRUE;
		current_attrlist_index++;
#endif

	/* If the user specifies a refresh value,
	 * make room for it 
	 */
	if(0 != refresh)
	{
		attrlist[current_attrlist_index] = (ALint)ALC_REFRESH;
		current_attrlist_index++;
		attrlist[current_attrlist_index] = refresh;
		current_attrlist_index++;		
	}
			
	/* End attribute list */
	attrlist[current_attrlist_index] = '\0';


	/* Initialize SDL_Sound */
	if(! Sound_Init() )
	{
        // Users complain of multiple instances of this error so only show it once
        // (it seems that it's typically caused by bad sound drivers)
        if (! initErrorDisplayed)
        {
            Sound_Version compiled;
            Sound_Version linked;

			initErrorDisplayed = 1;

            ALmixer_SetError("ALmixer_Init: %s", Sound_GetError());
            
            // Emit DLL version info in case that's why we can't init
            SOUND_VERSION(&compiled);
            Sound_GetLinkedVersion(&linked);

            fprintf(stderr, "ALmixer_Init: built with SDL_sound: %d.%d.%d\n",
                    compiled.major, compiled.minor, compiled.patch);
            fprintf(stderr, "ALmixer_Init:   linked DLL version: %d.%d.%d\n",
                   linked.major, linked.minor, linked.patch);
        }

		return AL_FALSE;
	}
#ifdef ENABLE_PARANOID_SIGNEDNESS_CHECK
	/* Here is the paranoid check that opens
	 * SDL audio in an attempt to find the correct 
	 * system values.
	 */
	/* Doesn't have to be the actual value I think
	 * (as long as it doesn't influence format, in 
	 * which case I'm probably screwed anyway because OpenAL
	 * may easily choose to do something else).
	 */
	desired.freq = 44100;
	desired.channels = 2;
	desired.format = AUDIO_S16SYS;
	desired.callback = my_dummy_audio_callback;
	if(SDL_OpenAudio(&desired, &obtained) >= 0)
	{
		SIGN_TYPE_16BIT_FORMAT = obtained.format;
		/* Now to get really paranoid, we should probably
		 * also assume that the 8bit format is also the
		 * same sign type and set that value
		 */
		if(AUDIO_S16SYS == obtained.format)
		{
			SIGN_TYPE_8BIT_FORMAT = AUDIO_S8;
		}
		/* Should be AUDIO_U16SYS */
		else
		{
			SIGN_TYPE_8BIT_FORMAT = AUDIO_U8;
		}
		SDL_CloseAudio();
	}
	else
	{
		/* Well, I guess I'm in trouble. I guess it's my best guess
		 */
		SIGN_TYPE_16_BIT_FORMAT = AUDIO_S16SYS;
		SIGN_TYPE_8_BIT_FORMAT = AUDIO_S8;
	}
#endif

#ifndef ALMIXER_COMPILE_WITHOUT_SDL
	/* Weirdness: It seems that SDL_Init(SDL_INIT_AUDIO)
	 * causes OpenAL and SMPEG to conflict. For some reason
	 * if SDL_Init on audio is active, then all the SMPEG
	 * decoded sound comes out silent. Unfortunately,
	 * Sound_Init() invokes SDL_Init on audio. I'm
	 * not sure why it actually needs it...
	 * But we'll attempt to disable it here after the
	 * SDL_Sound::Init call and hope it doesn't break SDL_Sound.
	 */
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
#endif

	/* I'm told NULL will call the default string
	 * and hopefully do the right thing for each platform 
	 */
	/*
	dev = alcOpenDevice( NULL );
	*/
	/* Now I'm told I need to set both the device and context
	 * to have the same sampling rate, so I must pass a string
	 * to OpenDevice(). I don't know how portable these strings are.
	 * I don't even know if the format for strings is 
	 * compatible
	 * From the testattrib.c in the Loki test section
	 * dev = alcOpenDevice(  (const ALubyte *) "'((sampling-rate 22050))" );
	 */	

#ifdef USING_LOKI_AL_DIST
	sprintf(device_string, "'((sampling-rate %d))", attrlist[1]);
	dev = alcOpenDevice(  (const ALubyte *) device_string );
#else
	dev = alcOpenDevice( NULL );
#endif
	if(NULL == dev)
	{
		ALmixer_SetError("Cannot open sound device for OpenAL");
		return AL_FALSE;
	}

#ifdef __APPLE__
	/* The ALC_FREQUENCY attribute is ignored with Apple's implementation. */
	/* This extension must be called before the context is created. */
	if(0 != frequency)
	{
		Internal_alcMacOSXMixerOutputRate((ALdouble)frequency);
	}
	ALmixer_Frequency_global = (ALuint)Internal_alcMacOSXGetMixerOutputRate();
	/*
		fprintf(stderr, "Internal_alcMacOSXMixerOutputRate is: %lf", Internal_alcMacOSXGetMixerOutputRate());
		*/
#endif
	
	context = alcCreateContext(dev, attrlist);
	if(NULL == context)
	{
		ALmixer_SetError("Cannot create a context OpenAL");
		alcCloseDevice(dev);
		return AL_FALSE;
	}


	/* Hmmm, OSX is returning 1 on alcMakeContextCurrent,
	 * but ALC_NO_ERROR is defined to ALC_FALSE.
	 * According to Garin Hiebert, this is actually an inconsistency
	 * in the Loki version. The function should return a boolean.
	 * instead of ALC_NO_ERROR. Garin suggested I check via
	 * alcGetError().
	 */
	/* clear the error */
	alcGetError(dev);
	alcMakeContextCurrent(context);
	
	error = alcGetError(dev);
	if( (ALC_NO_ERROR != error) )
	{
		ALmixer_SetError("Could not MakeContextCurrent");
		alcDestroyContext(context);
		alcCloseDevice(dev);
		return AL_FALSE;
	}
	
	/* It looks like OpenAL won't let us ask it what 
	 * the set frequency is, so we need to save our
	 * own copy. Yuck.
	 * Update: J. Valenzuela just updated the Loki 
	 * dist (2003/01/02) to handle this.
	 * The demo is in testattrib.c. 
	 */
/*
	ALmixer_Frequency_global = frequency;
*/
#ifndef __APPLE__
	alcGetIntegerv(dev, ALC_FREQUENCY, 1, &ALmixer_Frequency_global);
	/*
	fprintf(stderr, "alcGetIntegerv ALC_FREQUENCY is: %d", ALmixer_Frequency_global);
	*/
#endif
	

#if 0
	/* OSX is failing on alcMakeContextCurrent(). Try checking it first? */
	if(alcGetCurrentContext() != context)
	{
		/* Hmmm, OSX is returning 1 on alcMakeContextCurrent,
		 * but ALC_NO_ERROR is defined to ALC_FALSE.
		 * I think this is a bug in the OpenAL implementation.
		 */
		fprintf(stderr,"alcMakeContextCurrent returns %d\n", alcMakeContextCurrent(context));
				
		fprintf(stderr, "Making context current\n");
#ifndef __APPLE__
		if(alcMakeContextCurrent(context) != ALC_NO_ERROR)
#else
		if(!alcMakeContextCurrent(context))
#endif			
		{
			ALmixer_SetError("Could not MakeContextCurrent");
			alcDestroyContext(context);
			alcCloseDevice(dev);
			return AL_FALSE;
		}
	}
#endif


/* #endif */
	/* Saw this in the README with the OS X OpenAL distribution.
	 * It looked interesting and simple, so I thought I might
	 * try it out.
	 * ***** ALC_CONVERT_DATA_UPON_LOADING
	 * This extension allows the caller to tell OpenAL to preconvert to the native Core
	 * Audio format, the audio data passed to the 
	 * library with the alBufferData() call. Preconverting the audio data, reduces CPU 
	 * usage by removing an audio data conversion 
	 * (per source) at render timem at the expense of a larger memory footprint.
	 *
	 *	This feature is toggled on/off by using the alDisable() & alEnable() APIs. This 
	 *	setting will be applied to all subsequent 
	 *	calls to alBufferData().
	 * 
	 * Update: Some people keep reporting that they see the enable fail on Mac, but I can't reproduce it myself.
	 * Rather than deal with it right now, I think I am going to make it an opt-in thing.
	 */	
#if defined(__APPLE__) && defined(ALMIXER_USE_OSX_CONVERT_DATA_UPON_LOADING)
/*
 iPhone is getting this enum, but is failing on the enable, so I guess I'll define it out.
 */
	#if (TARGET_OS_IPHONE == 1) || (TARGET_IPHONE_SIMULATOR == 1)
	 
	#else
		/* iOS reports this enum exists, but loading it always fails, so make it Mac only. */
		ALenum convert_data_enum = alcGetEnumValue(dev, "ALC_MAC_OSX_CONVERT_DATA_UPON_LOADING");
		 /*
		 fprintf(stderr, "ALC_MAC_OSX_CONVERT_DATA_UPON_LOADING=0x%x", convert_data_enum);
		 */
		if(0 != convert_data_enum)
		{
			alEnable(convert_data_enum);		
		}
		if( (AL_NO_ERROR != alGetError()) )
		{
			fprintf(stderr, "ALC_MAC_OSX_CONVERT_DATA_UPON_LOADING attempted but failed");
			ALmixer_SetError("ALC_MAC_OSX_CONVERT_DATA_UPON_LOADING attempted but failed");
		}
	#endif
	
#endif /* __APPLE__ */
	

	
	
	ALmixer_Initialized = AL_TRUE;

	if(num_sources == 0)
	{
		Number_of_Channels_global = ALMIXER_DEFAULT_NUM_CHANNELS;
	}
	else
	{
		/* probably should make Number_of_Channels_global an ALuint, but need to cast which_channel all the time */
		Number_of_Channels_global = (ALint)num_sources;
	}
	Number_of_Reserve_Channels_global = 0;
	Is_Playing_global = 0;
	/* Set to Null in case system quit and was reinitialized */
	Channel_Done_Callback = NULL;
	Channel_Done_Callback_Userdata = NULL;
	Channel_Data_Callback = NULL;
	Channel_Data_Callback_Userdata = NULL;

	/* Allocate memory for linked list of ALmixerData. */
	s_listOfALmixerData = LinkedList_Create();
	if(NULL == s_listOfALmixerData)
	{
		ALmixer_SetError("Couldn't create linked list");
		alcDestroyContext(context);
		alcCloseDevice(dev);
		ALmixer_Initialized = AL_FALSE;
		Number_of_Channels_global = 0;
		return AL_FALSE;
	}

	/* Allocate memory for the list of channels */
	ALmixer_Channel_List = (struct ALmixer_Channel*) malloc(Number_of_Channels_global * sizeof(struct ALmixer_Channel));
	if(NULL == ALmixer_Channel_List)
	{
		ALmixer_SetError("Out of Memory for Channel List");
		LinkedList_Free(s_listOfALmixerData);
		alcDestroyContext(context);
		alcCloseDevice(dev);
		ALmixer_Initialized = AL_FALSE;
		Number_of_Channels_global = 0;
		return AL_FALSE;
	}

	/* Allocate memory for the list of sources that map to the channels */
	Source_Map_List = (Source_Map*) malloc(Number_of_Channels_global * sizeof(Source_Map));
	if(NULL == Source_Map_List)
	{
		ALmixer_SetError("Out of Memory for Source Map List");
		_free(ALmixer_Channel_List);
		LinkedList_Free(s_listOfALmixerData);		
		alcDestroyContext(context);
		alcCloseDevice(dev);
		ALmixer_Initialized = AL_FALSE;
		Number_of_Channels_global = 0;
		return AL_FALSE;
	}

	/* Create array that will hold the sources */
	source = (ALuint*)malloc(Number_of_Channels_global * sizeof(ALuint));
	if(NULL == source)
	{
		ALmixer_SetError("Out of Memory for sources");
		_free(Source_Map_List);
		_free(ALmixer_Channel_List);
		LinkedList_Free(s_listOfALmixerData);		
		alcDestroyContext(context);
		alcCloseDevice(dev);
		ALmixer_Initialized = AL_FALSE;
		Number_of_Channels_global = 0;
		return AL_FALSE;
	}	

	/* Clear the error state */
	alGetError();
	/* Generate the OpenAL sources */
	_alGenSources(Number_of_Channels_global, source);
	if( (error=alGetError()) != AL_NO_ERROR)
	{
		ALmixer_SetError("Couldn't generate sources: %s\n", alGetString(error));
		_free(ALmixer_Channel_List);
		_free(Source_Map_List);
		LinkedList_Free(s_listOfALmixerData);		
		alcDestroyContext(context);
		alcCloseDevice(dev);
		ALmixer_Initialized = AL_FALSE;
		Number_of_Channels_global = 0;
		return AL_FALSE;
	}
	
	/* Initialize each channel and associate one source to one channel */
	for(i=0; i<Number_of_Channels_global; i++)
	{
		if(0 == source[i])
		{
			fprintf(stderr, "SDL_ALmixer serious problem. This OpenAL implementation allowed 0 to be a valid source id which is in conflict with assumptions made in this library.\n");
		}
		
		Init_Channel(i);
		/* Keeping the source allocation out of the Init function
		 * in case I want to reuse the Init
		 * function for resetting data 
		 */
		ALmixer_Channel_List[i].alsource = source[i];
		/* Now also keep a copy of the source to channel mapping
		 * in case we need to look up a channel from the source
		 * instead of a source from a channel 
		 */
		Source_Map_List[i].source = source[i];
		Source_Map_List[i].channel = i;
		/* Clean the channel because there are some things that need to 
		 * be done that can't happen until the source is set
		 */
		Clean_Channel(i);
	}

	/* The Source_Map_List must be sorted by source for binary searches
	 */
	qsort(Source_Map_List, Number_of_Channels_global, sizeof(Source_Map), Compare_Source_Map);
	
	/*
	ALmixer_OutputDecoders();
*/



#ifdef ENABLE_ALMIXER_THREADS
	s_simpleLock = SDL_CreateMutex();
	if(NULL == s_simpleLock)
	{
		/* SDL sets the error message already? */
		_free(source);
		_free(ALmixer_Channel_List);
		_free(Source_Map_List);
		LinkedList_Free(s_listOfALmixerData);		
		alcDestroyContext(context);
		alcCloseDevice(dev);
		ALmixer_Initialized = AL_FALSE;
		Number_of_Channels_global = 0;
		return AL_FALSE;
	}
		
	g_StreamThreadEnabled = AL_TRUE;
	Stream_Thread_global = SDL_CreateThread(Stream_Data_Thread_Callback, NULL);
	if(NULL == Stream_Thread_global)
	{
		/* SDL sets the error message already? */
		SDL_DestroyMutex(s_simpleLock);
		_free(source);
		_free(ALmixer_Channel_List);
		_free(Source_Map_List);
		LinkedList_Free(s_listOfALmixerData);		
		alcDestroyContext(context);
		alcCloseDevice(dev);
		ALmixer_Initialized = AL_FALSE;
		Number_of_Channels_global = 0;
		g_StreamThreadEnabled = AL_FALSE;		
		return AL_FALSE;
	}
	
	/* Note: Only a few platforms change the priority. See implementation for notes. */
	Internal_LowerThreadPriority(Stream_Thread_global);
		
/*
	fprintf(stderr, "Using threads\n");
*/
#endif /* End of ENABLE_ALMIXER_THREADS */

	/* We don't need this array any more because all the sources 
	 * are connected to channels
	 */
	_free(source);
	return AL_TRUE;
}

	
ALboolean ALmixer_InitContext(ALuint frequency, ALuint refresh)
{
	ALCdevice* dev;
	ALCcontext* context;
	ALCenum error;

#ifdef USING_LOKI_AL_DIST
	/* The Loki dist requires that I set both the 
	 * device and context frequency values separately
	 */
	/* Hope this won't overflow */
	char device_string[256];
#endif
	
	/* (Venting frustration) Damn it! Nobody bothered
	 * documenting how you're supposed to use an attribute
	 * list. In fact, the not even the Loki test program
	 * writers seem to know because they use it inconsistently.
	 * For example, how do you terminate that attribute list?
	 * The Loki test code does it 3 different ways. They 
	 * set the last value to 0, or they set it to ALC_INVALID, 
	 * or they set two final values: ALC_INVALID, 0
	 * In Loki, 0 and ALC_INVALID happen to be the same,
	 * but with Creative Labs ALC_INVALID is -1. 
	 * So something's going to break. Loki's source
	 * code says to terminate with ALC_INVALID. But I 
	 * don't know if that's really true, or it happens 
	 * to be a coinicidence because it's defined to 0.
	 * Creative provides no source code, so I can't look at how 
	 * they terminate it. 
	 * So this is really, really ticking me off...
	 * For now, I'm going to use ALC_INVALID.
	 * (Update...after further review of the API spec,
	 * it seems that a NULL terminated string is the correct
	 * termination value to use, so 0 it is.)
	 */
#if 0
	ALint attrlist[] = { 
		ALC_FREQUENCY, ALMIXER_DEFAULT_FREQUENCY,
		/* Don't know anything about these values.
		 * Trust defaults? */
		/* Supposed to be the refresh rate in Hz.
		 * I think 15-120 are supposed to be good 
		 * values. Though I haven't gotten any effect except
		 * for one strange instance on a Mac. But it was 
		 * unrepeatable.
		 */
	#if 0
		ALC_REFRESH, 15,
	#endif	
		/* Sync requires a alcProcessContext() call
		 * for every cycle. By default, this is
		 * not used and the value is AL_FALSE
		 * because it will probably perform
		 * pretty badly for me.
		 */
#ifdef ENABLE_ALMIXER_ALC_SYNC	
		ALC_SYNC, AL_TRUE,
#else
		ALC_SYNC, AL_FALSE,
#endif
		/* Looking at the API spec, it implies
		 * that the list be a NULL terminated string
		 * so it's probably not safe to use ALC_INVALID
		*/
		/*
		ALC_INVALID };
		*/
		'\0'};
#endif
	/* Redo: I'm going to allow ALC_REFRESH to be set.
	 * However, if no value is specified, I don't
	 * want it in the list so I can get the OpenAL defaults
	 */
	ALint attrlist[7];
	ALsizei current_attrlist_index = 0;

#ifdef ENABLE_PARANOID_SIGNEDNESS_CHECK
	/* More problems: I'm getting bit by endian/signedness issues on
	 * different platforms. I can find the endianess easily enough,
	 * but I don't know how to determine what the correct signedness
	 * is (if such a thing exists). I do know that if I try using
	 * unsigned on OSX with an originally signed sample, I get 
	 * distortion. However, I don't have any native unsigned samples
	 * to test. But I'm assuming that the platform must be in the 
	 * correct signedness no matter what.
	 * I can either assume everybody is signed, or I can try to 
	 * determine the value. If I try to determine the values,
	 * I think my only ability to figure it out will be to open
	 * SDL_Audio, and read what the obtained settings were.
	 * Then shutdown everything. However, I don't even know how 
	 * reliable this is.
	 * Update: I think I resolved the issues...forgot to update 
	 * these comments when it happened. I should check the revision control
	 * log... Anyway, I think the issue was partly related to me not 
	 * doing something correctly with the AudioInfo or some kind 
	 * of stupid endian bug in my code, and weirdness ensued. Looking at the
	 * revision control, I think I might have assumed that SDL_Sound would 
	 * do the right thing with a NULL AudioInfo, but I was incorrect,
	 * and had to fill one out myself.
	 */
	SDL_AudioSpec desired;
	SDL_AudioSpec obtained;
#endif




	/* Make sure ALmixer isn't already initialized */
	if(ALmixer_Initialized)
	{
		return AL_FALSE;
	}

	/* Set the defaults */
	attrlist[0] = ALC_FREQUENCY;
	attrlist[1] = ALMIXER_DEFAULT_FREQUENCY;
	attrlist[2] = ALC_SYNC;
#ifdef ENABLE_ALMIXER_ALC_SYNC	
	attrlist[3] = ALC_TRUE;
#else
	attrlist[3] = ALC_FALSE;
#endif
	/* Set frequency value if it is not 0 */
	if(0 != frequency)
	{
		attrlist[current_attrlist_index] = ALC_FREQUENCY;
		current_attrlist_index++;
		attrlist[current_attrlist_index] = (ALint)frequency;
		current_attrlist_index++;
	}

#ifdef ENABLE_ALMIXER_ALC_SYNC	
		attrlist[current_attrlist_index] = ALC_SYNC;
		current_attrlist_index++;
		attrlist[current_attrlist_index] = ALC_TRUE;
		current_attrlist_index++;
#endif

	/* If the user specifies a refresh value,
	 * make room for it 
	 */
	if(0 != refresh)
	{
		attrlist[current_attrlist_index] = (ALint)ALC_REFRESH;
		current_attrlist_index++;
		attrlist[current_attrlist_index] = refresh;
		current_attrlist_index++;		
	}
			
	/* End attribute list */
	attrlist[current_attrlist_index] = '\0';



	/* Initialize SDL_Sound */
	if(! Sound_Init() )
	{
		ALmixer_SetError("ALmixer_InitContext: %s", Sound_GetError());
		return AL_FALSE;
	}
#ifdef ENABLE_PARANOID_SIGNEDNESS_CHECK
	/* Here is the paranoid check that opens
	 * SDL audio in an attempt to find the correct 
	 * system values.
	 */
	/* Doesn't have to be the actual value I think
	 * (as long as it doesn't influence format, in 
	 * which case I'm probably screwed anyway because OpenAL
	 * may easily choose to do something else).
	 */
	desired.freq = 44100;
	desired.channels = 2;
	desired.format = AUDIO_S16SYS;
	desired.callback = my_dummy_audio_callback;
	if(SDL_OpenAudio(&desired, &obtained) >= 0)
	{
		SIGN_TYPE_16BIT_FORMAT = obtained.format;
		/* Now to get really paranoid, we should probably
		 * also assume that the 8bit format is also the
		 * same sign type and set that value
		 */
		if(AUDIO_S16SYS == obtained.format)
		{
			SIGN_TYPE_8BIT_FORMAT = AUDIO_S8;
		}
		/* Should be AUDIO_U16SYS */
		else
		{
			SIGN_TYPE_8BIT_FORMAT = AUDIO_U8;
		}
		SDL_CloseAudio();
	}
	else
	{
		/* Well, I guess I'm in trouble. I guess it's my best guess
		 */
		SIGN_TYPE_16_BIT_FORMAT = AUDIO_S16SYS;
		SIGN_TYPE_8_BIT_FORMAT = AUDIO_S8;
	}
#endif

#ifndef ALMIXER_COMPILE_WITHOUT_SDL
	/* Weirdness: It seems that SDL_Init(SDL_INIT_AUDIO)
	 * causes OpenAL and SMPEG to conflict. For some reason
	 * if SDL_Init on audio is active, then all the SMPEG
	 * decoded sound comes out silent. Unfortunately,
	 * Sound_Init() invokes SDL_Init on audio. I'm
	 * not sure why it actually needs it...
	 * But we'll attempt to disable it here after the
	 * SDL_Sound::Init call and hope it doesn't break SDL_Sound.
	 */
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
#endif

	/* I'm told NULL will call the default string
	 * and hopefully do the right thing for each platform 
	 */
	/*
	dev = alcOpenDevice( NULL );
	*/
	/* Now I'm told I need to set both the device and context
	 * to have the same sampling rate, so I must pass a string
	 * to OpenDevice(). I don't know how portable these strings are.
	 * I don't even know if the format for strings is 
	 * compatible
	 * From the testattrib.c in the Loki test section
	 * dev = alcOpenDevice(  (const ALubyte *) "'((sampling-rate 22050))" );
	 */	

#ifdef USING_LOKI_AL_DIST
	sprintf(device_string, "'((sampling-rate %d))", attrlist[1]);
	dev = alcOpenDevice(  (const ALubyte *) device_string );
#else
	dev = alcOpenDevice( NULL );
#endif
	if(NULL == dev)
	{
		ALmixer_SetError("Cannot open sound device for OpenAL");
		return AL_FALSE;
	}

#ifdef __APPLE__
	/* The ALC_FREQUENCY attribute is ignored with Apple's implementation. */
	/* This extension must be called before the context is created. */
	if(0 != frequency)
	{
		Internal_alcMacOSXMixerOutputRate((ALdouble)frequency);
	}
	ALmixer_Frequency_global = (ALuint)Internal_alcMacOSXGetMixerOutputRate();
	/*
		fprintf(stderr, "Internal_alcMacOSXMixerOutputRate is: %lf", Internal_alcMacOSXGetMixerOutputRate());
		*/
#endif
	
	
	context = alcCreateContext(dev, attrlist);
	if(NULL == context)
	{
		ALmixer_SetError("Cannot create a context OpenAL");
		alcCloseDevice(dev);
		return AL_FALSE;
	}


	/* Hmmm, OSX is returning 1 on alcMakeContextCurrent,
	 * but ALC_NO_ERROR is defined to ALC_FALSE.
	 * According to Garin Hiebert, this is actually an inconsistency
	 * in the Loki version. The function should return a boolean.
	 * instead of ALC_NO_ERROR. Garin suggested I check via
	 * alcGetError().
	 */
	/* clear the error */
	alcGetError(dev);
	alcMakeContextCurrent(context);
	
	error = alcGetError(dev);
	if( (ALC_NO_ERROR != error) )
	{
		ALmixer_SetError("Could not MakeContextCurrent");
		alcDestroyContext(context);
		alcCloseDevice(dev);
		return AL_FALSE;
	}


#if 0
	/* OSX is failing on alcMakeContextCurrent(). Try checking it first? */
	if(alcGetCurrentContext() != context)
	{
		/* Hmmm, OSX is returning 1 on alcMakeContextCurrent,
		 * but ALC_NO_ERROR is defined to ALC_FALSE.
		 * I think this is a bug in the OpenAL implementation.
		 */
		fprintf(stderr,"alcMakeContextCurrent returns %d\n", alcMakeContextCurrent(context));
				
		fprintf(stderr, "Making context current\n");
#ifndef __APPLE__
		if(alcMakeContextCurrent(context) != ALC_NO_ERROR)
#else
		if(!alcMakeContextCurrent(context))
#endif			
		{
			ALmixer_SetError("Could not MakeContextCurrent");
			alcDestroyContext(context);
			alcCloseDevice(dev);
			return AL_FALSE;
		}

	}
#endif
		
	/* It looks like OpenAL won't let us ask it what 
	 * the set frequency is, so we need to save our
	 * own copy. Yuck.
	 * Update: J. Valenzuela just updated the Loki 
	 * dist (2003/01/02) to handle this.
	 * The demo is in testattrib.c. 
	 */
#ifndef __APPLE__
	alcGetIntegerv(dev, ALC_FREQUENCY, 1, &ALmixer_Frequency_global);
	/*
	fprintf(stderr, "alcGetIntegerv ALC_FREQUENCY is: %d", ALmixer_Frequency_global);
	*/
#endif
	


	/* Saw this in the README with the OS X OpenAL distribution.
	 * It looked interesting and simple, so I thought I might
	 * try it out.
	 * ***** ALC_CONVERT_DATA_UPON_LOADING
	 * This extension allows the caller to tell OpenAL to preconvert to the native Core
	 * Audio format, the audio data passed to the 
	 * library with the alBufferData() call. Preconverting the audio data, reduces CPU 
	 * usage by removing an audio data conversion 
	 * (per source) at render timem at the expense of a larger memory footprint.
	 *
	 *	This feature is toggled on/off by using the alDisable() & alEnable() APIs. This 
	 *	setting will be applied to all subsequent 
	 *	calls to alBufferData().
	 * Update: Some people keep reporting that they see the enable fail on Mac, but I can't reproduce it myself.
	 * Rather than deal with it right now, I think I am going to make it an opt-in thing.
	 */	
#if defined(__APPLE__) && defined(ALMIXER_USE_OSX_CONVERT_DATA_UPON_LOADING)
	/*
	 iPhone is getting this enum, but is failing on the enable, so I guess I'll define it out.
	 */
	#if (TARGET_OS_IPHONE == 1) || (TARGET_IPHONE_SIMULATOR == 1)
	 
	#else
		/* iOS reports this enum exists, but loading it always fails, so make it Mac only. */
		ALenum convert_data_enum = alcGetEnumValue(dev, "ALC_MAC_OSX_CONVERT_DATA_UPON_LOADING");
		 /*
		 fprintf(stderr, "ALC_MAC_OSX_CONVERT_DATA_UPON_LOADING=0x%x", convert_data_enum);
		 */
		if(0 != convert_data_enum)
		{
			alEnable(convert_data_enum);		
		}
		if( (AL_NO_ERROR != alGetError()) )
		{
			fprintf(stderr, "ALC_MAC_OSX_CONVERT_DATA_UPON_LOADING attempted but failed");
			ALmixer_SetError("ALC_MAC_OSX_CONVERT_DATA_UPON_LOADING attempted but failed");
		}
	#endif
#endif
	
	return AL_TRUE;
}

	
ALboolean ALmixer_InitMixer(ALuint num_sources)
{	
	ALint i;
	ALenum error;
	ALuint* source;


	ALmixer_Initialized = AL_TRUE;


#ifdef ALMIXER_COMPILE_WITHOUT_SDL
	ALmixer_InitTime();

	/* Note: The pool may have been created on previous Init's */
	/* I leave the pool allocated allocated in case the user wants
	 * to read the pool in case of a failure (such as in this function).
	 * This is not actually a leak.
	 */
	if(NULL == s_ALmixerErrorPool)
	{
		s_ALmixerErrorPool = TError_CreateErrorPool();
	}
	if(NULL == s_ALmixerErrorPool)
	{
		return AL_FALSE;
	}
	/*
		fprintf(stderr, "tError Test0\n");
		ALmixer_SetError("Initing (and testing SetError)");
		fprintf(stderr, "tError Test1: %s\n", ALmixer_GetError());
		fprintf(stderr, "tError Test2: %s\n", ALmixer_GetError());
	 */
#endif

	if(num_sources == 0)
	{
		Number_of_Channels_global = ALMIXER_DEFAULT_NUM_CHANNELS;
	}
	else
	{
		Number_of_Channels_global = (ALint)num_sources;
	}
	Number_of_Reserve_Channels_global = 0;
	Is_Playing_global = 0;
	/* Set to Null in case system quit and was reinitialized */
	Channel_Done_Callback = NULL;
	Channel_Done_Callback_Userdata = NULL;
	Channel_Data_Callback = NULL;
	Channel_Data_Callback_Userdata = NULL;

	/* Allocate memory for linked list of ALmixerData. */
	s_listOfALmixerData = LinkedList_Create();
	if(NULL == s_listOfALmixerData)
	{
		ALmixer_SetError("Couldn't create linked list");
		ALmixer_Initialized = AL_FALSE;
		Number_of_Channels_global = 0;
		return AL_FALSE;
	}


	/* Allocate memory for the list of channels */
	ALmixer_Channel_List = (struct ALmixer_Channel*) malloc(Number_of_Channels_global * sizeof(struct ALmixer_Channel));
	if(NULL == ALmixer_Channel_List)
	{
		ALmixer_SetError("Out of Memory for Channel List");
		LinkedList_Free(s_listOfALmixerData);		
		ALmixer_Initialized = AL_FALSE;
		Number_of_Channels_global = 0;
		return AL_FALSE;
	}

	/* Allocate memory for the list of sources that map to the channels */
	Source_Map_List = (Source_Map*) malloc(Number_of_Channels_global * sizeof(Source_Map));
	if(NULL == Source_Map_List)
	{
		ALmixer_SetError("Out of Memory for Source Map List");
		_free(ALmixer_Channel_List);
		LinkedList_Free(s_listOfALmixerData);		
		ALmixer_Initialized = AL_FALSE;
		Number_of_Channels_global = 0;
		return AL_FALSE;
	}

	/* Create array that will hold the sources */
	source = (ALuint*)malloc(Number_of_Channels_global * sizeof(ALuint));
	if(NULL == source)
	{
		ALmixer_SetError("Out of Memory for sources");
		_free(Source_Map_List);
		_free(ALmixer_Channel_List);
		LinkedList_Free(s_listOfALmixerData);		
		ALmixer_Initialized = AL_FALSE;
		Number_of_Channels_global = 0;
		return AL_FALSE;
	}	

	/* Clear the error state */
	alGetError();
	/* Generate the OpenAL sources */
	_alGenSources(Number_of_Channels_global, source);
	if( (error=alGetError()) != AL_NO_ERROR)
	{
		ALmixer_SetError("Couldn't generate sources: %s\n", alGetString(error));
		_free(ALmixer_Channel_List);
		_free(Source_Map_List);
		LinkedList_Free(s_listOfALmixerData);
		ALmixer_Initialized = AL_FALSE;
		Number_of_Channels_global = 0;
		return AL_FALSE;
	}
	
	/* Initialize each channel and associate one source to one channel */
	for(i=0; i<Number_of_Channels_global; i++)
	{
		Init_Channel(i);
		/* Keeping the source allocation out of the Init function
		 * in case I want to reuse the Init
		 * function for resetting data 
		 */
		ALmixer_Channel_List[i].alsource = source[i];
		/* Now also keep a copy of the source to channel mapping
		 * in case we need to look up a channel from the source
		 * instead of a source from a channel 
		 */
		Source_Map_List[i].source = source[i];
		Source_Map_List[i].channel = i;
		/* Clean the channel because there are some things that need to 
		 * be done that can't happen until the source is set
		 */
		Clean_Channel(i);
	}

	/* The Source_Map_List must be sorted by source for binary searches
	 */
	qsort(Source_Map_List, Number_of_Channels_global, sizeof(Source_Map), Compare_Source_Map);
	
	
#ifdef ENABLE_ALMIXER_THREADS
	s_simpleLock = SDL_CreateMutex();
	if(NULL == s_simpleLock)
	{
		/* SDL sets the error message already? */
		_free(source);
		_free(ALmixer_Channel_List);
		_free(Source_Map_List);
		ALmixer_Initialized = AL_FALSE;
		Number_of_Channels_global = 0;
		return AL_FALSE;
	}
		

	g_StreamThreadEnabled = AL_TRUE;
	Stream_Thread_global = SDL_CreateThread(Stream_Data_Thread_Callback, NULL);
	if(NULL == Stream_Thread_global)
	{
		/* SDL sets the error message already? */
		SDL_DestroyMutex(s_simpleLock);
		_free(source);
		_free(ALmixer_Channel_List);
		_free(Source_Map_List);
		ALmixer_Initialized = AL_FALSE;
		Number_of_Channels_global = 0;
		g_StreamThreadEnabled = AL_FALSE;
		return AL_FALSE;
	}

	/* Note: Only a few platforms change the priority. See implementation for notes. */
	Internal_LowerThreadPriority(Stream_Thread_global);

	/*
	fprintf(stderr, "Using threads\n");
	*/
#endif /* End of ENABLE_ALMIXER_THREADS */

	/* We don't need this array any more because all the sources 
	 * are connected to channels
	 */
	_free(source);
	return AL_TRUE;
}

void ALmixer_BeginInterruption()
{
	if((AL_TRUE == g_inInterruption) || (AL_FALSE == ALmixer_Initialized))
	{
		return;
	}

	ALmixer_SuspendUpdates();

	/* App Portable introduced alcSuspend() and alcResume() for Android in their OpenAL Soft fork. */
#if defined(__ANDROID__) && defined(ALMIXER_USE_APPORTABLE_OPENAL_EXTENSIONS)
	alcSuspend();
#endif


	s_interruptionContext = alcGetCurrentContext();
	if(NULL != s_interruptionContext)
	{
        #if defined(__ANDROID__) && !defined(ALMIXER_USE_APPORTABLE_OPENAL_EXTENSIONS)
            ALCdevice * dev = alcGetContextsDevice(s_interruptionContext);
            if(NULL != dev)
            {
                alcDevicePauseSOFT(dev);
            }
        #endif
		/* iOS alcSuspendContext is a no-op */
		alcSuspendContext(s_interruptionContext);
		alcMakeContextCurrent(NULL);
	}

	g_inInterruption = AL_TRUE;
}

void ALmixer_EndInterruption()
{
	if((AL_FALSE == g_inInterruption) || (AL_FALSE == ALmixer_Initialized))
	{
		return;
	}

	/* Note: iOS, you need to set the AudioSession active.
	 * But if the AudioSession is not initialized, this SetActive 
	 * call fails.
	 * So this is probably better if calling app sets this.
	 */
/*
#if (TARGET_OS_IPHONE == 1) || (TARGET_IPHONE_SIMULATOR == 1)
	OSStatus the_error = AudioSessionSetActive(true);
	if(noErr != the_error)
	{
		fprintf(stderr, "Error setting audio session active! %d\n", the_error);
	}
#endif
 */
	
	if(NULL != s_interruptionContext)
	{
		alcMakeContextCurrent(s_interruptionContext);
		alcProcessContext(s_interruptionContext);
        #if defined(__ANDROID__) && !defined(ALMIXER_USE_APPORTABLE_OPENAL_EXTENSIONS)
            ALCdevice * dev = alcGetContextsDevice(s_interruptionContext);
            if(NULL != dev)
            {
                alcDeviceResumeSOFT(dev);
            }
        #endif
		s_interruptionContext = NULL;
	}


	/* App Portable introduced alcSuspend() and alcResume() for Android in their OpenAL Soft fork. */
#if defined(__ANDROID__) && defined(ALMIXER_USE_APPORTABLE_OPENAL_EXTENSIONS)
	alcResume();
#endif


	ALmixer_ResumeUpdates();
	g_inInterruption = AL_FALSE;
}


ALboolean ALmixer_IsInInterruption()
{
	if(AL_FALSE == ALmixer_Initialized)
	{
		return AL_FALSE;
	}
	return g_inInterruption;
}

void ALmixer_SuspendUpdates()
{
	if(AL_TRUE == ALmixer_AreUpdatesSuspended())
	{
		return;
	}
#ifdef ENABLE_ALMIXER_THREADS
	/* Kill bookkeeping thread to help minimize wasted CPU resources */
	
	/* Is locking really necessary here? */
	/*	SDL_LockMutex(s_simpleLock); */
	g_StreamThreadEnabled = AL_FALSE;
	/*	SDL_UnlockMutex(s_simpleLock); */
	
	SDL_WaitThread(Stream_Thread_global, NULL);
	Stream_Thread_global = NULL;
#endif
}

void ALmixer_ResumeUpdates()
{
	if(AL_FALSE == ALmixer_AreUpdatesSuspended())
	{
		return;
	}

#ifdef ENABLE_ALMIXER_THREADS
	/* This must be set before the thread is created to prevent the thread from exiting. */
	g_StreamThreadEnabled = AL_TRUE;
	
	Stream_Thread_global = SDL_CreateThread(Stream_Data_Thread_Callback, NULL);
	if(NULL == Stream_Thread_global)
	{
		fprintf(stderr, "Critical Error: Could not create bookkeeping thread in EndInterruption\n");
	}
	/* Note: Only a few platforms change the priority. See implementation for notes. */
	Internal_LowerThreadPriority(Stream_Thread_global);	
#endif
}

ALboolean ALmixer_AreUpdatesSuspended()
{
#ifdef ENABLE_ALMIXER_THREADS
	if(AL_FALSE == g_StreamThreadEnabled)
	{
		return AL_TRUE;
	}
	else
	{
		return AL_FALSE;
	}
#else
	return AL_FALSE;
#endif
}

/* Keep the return value void to allow easy use with
 * atexit()
 */
void ALmixer_Quit()
{
	ALCcontext* context;
	ALCdevice* dev;
	ALint i;
	
	if( ! ALmixer_Initialized)
	{
		return;
	}

#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	
	/* Several things we need to do:
	 First, we need to check if we are in an interruption.
	 If so, we need to reactivate the alcContext so we can call OpenAL functions.
	 Next, we should delete the OpenAL sources.
	 Next, we need to free all the sound data via ALmixer_FreeData().
	 Finally, we can delete the OpenAL context.
	 */
	
	context = alcGetCurrentContext();
	if(NULL == context)
	{
		/* We might have an interruption event where the current context is NULL */
		if(NULL == s_interruptionContext)
		{
			/* Nothing left to try. I think we're done. */
			fprintf(stderr, "ALmixer_Quit: Assertion Error. Expecting to find an OpenAL context, but could not find one.\n");
			return;
		}
		else 
		{
			context = s_interruptionContext;
			/* reactivate the context so we can call OpenAL functions */
			alcMakeContextCurrent(context);
			s_interruptionContext = NULL;
		}
	}	
	
	/* Shutdown everything before closing context */
	Internal_HaltChannel(-1, AL_FALSE);
	
	/* This flag will cause the thread to terminate */
	ALmixer_Initialized = AL_FALSE;
#ifdef ENABLE_ALMIXER_THREADS
	g_StreamThreadEnabled = AL_FALSE;
	SDL_UnlockMutex(s_simpleLock);
	/* This is safe to call with NULL thread, so we don't need to do anything special for interruptions. */
	SDL_WaitThread(Stream_Thread_global, NULL);
	Stream_Thread_global = NULL;

	SDL_DestroyMutex(s_simpleLock);
#endif
	g_inInterruption = AL_FALSE;
	
	/* Delete all the OpenAL sources */
	for(i=0; i<Number_of_Channels_global; i++)
	{
		_alDeleteSources(1, &ALmixer_Channel_List[i].alsource);
	}
	/* Delete all the channels */
	_free(ALmixer_Channel_List);
	_free(Source_Map_List);

	/* Reset the Number_of_Channels just in case somebody
	 * tries using a ALmixer function.
	 * I probably should put "Initialized" checks everywhere,
	 * but I'm too lazy at the moment.
	 */
	Number_of_Channels_global = 0;
	

	/* Delete the list of ALmixerData's before Sound_Quit deletes
	 * its own underlying information and I potentially have dangling pointers.
	 */
	while(LinkedList_Size(s_listOfALmixerData) > 0)
	{
		/* Note that ALmixer_FreeData will remove the data from the linked list for us so don't pop the list here. */
		ALmixer_Data* almixer_data = LinkedList_PopBack(s_listOfALmixerData);
		/* Watch out: ALmixer_FreeData used to escape because ALmixer_Initialized is now false. */
		Internal_FreeData(almixer_data);
	}
	LinkedList_Free(s_listOfALmixerData);
	s_listOfALmixerData = NULL;
	
	
	/* Need to get the device before I close the context */
	dev = alcGetContextsDevice(context);
	
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	
	if(NULL == dev)
	{
		return;	
	}
	alcCloseDevice(dev);
	
	Sound_Quit();

#ifdef ALMIXER_COMPILE_WITHOUT_SDL
	/* Remember: ALmixer_SetError/GetError calls will not work while this is gone. */
	TError_FreeErrorPool(s_ALmixerErrorPool);
	s_ALmixerErrorPool = NULL;
#endif
	return;
}

ALboolean ALmixer_IsInitialized()
{
	return ALmixer_Initialized;
}

ALuint ALmixer_GetFrequency()
{
	return ALmixer_Frequency_global;
}

const ALmixer_version* ALmixer_GetLinkedVersion()
{
	static ALmixer_version linked_mixver;
	ALMIXER_GET_COMPILED_VERSION(&linked_mixver);
	return(&linked_mixver);
}

#ifdef ALMIXER_COMPILE_WITHOUT_SDL

const char* ALmixer_GetError()
{
	const char* error_string = NULL;
	if(NULL == s_ALmixerErrorPool)
	{
		return "Error: You should not call ALmixer_GetError while ALmixer is not initialized";
	}
	error_string = TError_GetLastErrorStr(s_ALmixerErrorPool);
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

void ALmixer_SetError(const char* err_str, ...)
{
	va_list argp;

	if (NULL == s_ALmixerErrorPool)
	{
		fprintf(stderr, "Error: You should not call ALmixer_SetError while ALmixer is not initialized\n");
		return;
	}
	va_start(argp, err_str);
	/* SDL_SetError which I'm emulating has no number parameter. */
	TError_SetErrorv(s_ALmixerErrorPool, 1, err_str, argp);
	va_end(argp);
}

#endif




#if 0
void ALmixer_OutputAttributes()
{
	ALint num_flags = 0;
	ALint* flags = 0;
	int i;
	ALCdevice* dev = alcGetContextsDevice( alcGetCurrentContext() );
	

	printf("custom context\n");

	alcGetIntegerv(dev, ALC_ATTRIBUTES_SIZE,
				   sizeof num_flags, &num_flags );

	printf("Number of Flags: %d\n", num_flags);

	if(num_flags)
	{
		flags = malloc(sizeof(num_flags) * sizeof(ALint));

		alcGetIntegerv(dev, ALC_ALL_ATTRIBUTES,
		   sizeof num_flags * sizeof(ALint),
		   flags );
	}
	for(i = 0; i < num_flags-1; i += 2)
	{
		printf("key 0x%x : value %d\n",
		   flags[i], flags[i+1]);
	}
	free(flags);
}
#endif


void ALmixer_OutputDecoders()
{
	Sound_Version sound_compile_version;
	Sound_Version sound_link_version;
	
	const Sound_DecoderInfo **rc = Sound_AvailableDecoders();
	const Sound_DecoderInfo **i;
	const char **ext;
	FILE* stream = stdout;

	
	fprintf(stream, "SDL_sound Information:\n");

	SOUND_VERSION(&sound_compile_version);
	fprintf(stream, "\tCompiled with SDL_sound version: %d.%d.%d\n",
		sound_compile_version.major,
		sound_compile_version.minor,
		sound_compile_version.patch);

	Sound_GetLinkedVersion(&sound_link_version);
	fprintf(stream, "\tRunning (linked) with SDL_sound version: %d.%d.%d\n",
		sound_link_version.major,
		sound_link_version.minor,
		sound_link_version.patch);

	fprintf(stream, "Supported sound formats:\n");
	if (rc == NULL)
		fprintf(stream, " * Apparently, NONE!\n");
	else
	{
		for (i = rc; *i != NULL; i++)
		{
			fprintf(stream, " * %s\n", (*i)->description);

			for (ext = (*i)->extensions; *ext != NULL; ext++)
				fprintf(stream, "   File extension \"%s\"\n", *ext);

			fprintf(stream, "   Written by %s.\n   %s\n\n",
				(*i)->author, (*i)->url);
		} /* for */
	} /* else */

	fprintf(stream, "\n");
} 

void ALmixer_OutputOpenALInfo()
{
	ALmixer_version mixer_compile_version;
	const ALmixer_version * mixer_link_version=ALmixer_GetLinkedVersion();
	FILE* stream = stdout;

	fprintf(stream, "OpenAL Information:\n");
	fprintf(stream, "\tAL_VENDOR: %s\n", alGetString( AL_VENDOR ) );
	fprintf(stream, "\tAL_VERSION: %s\n", alGetString( AL_VERSION ) );
	fprintf(stream, "\tAL_RENDERER: %s\n", alGetString( AL_RENDERER ) );
	fprintf(stream, "\tAL_EXTENSIONS: %s\n", alGetString( AL_EXTENSIONS ) );

	ALMIXER_GET_COMPILED_VERSION(&mixer_compile_version);
	fprintf(stream, "\nSDL_ALmixer Information:\n");
	fprintf(stream, "\tCompiled with SDL_ALmixer version: %d.%d.%d\n",
		mixer_compile_version.major,
		mixer_compile_version.minor,
		mixer_compile_version.patch);

	fprintf(stream, "\tRunning (linked) with SDL_ALmixer version: %d.%d.%d\n",
		mixer_link_version->major,
		mixer_link_version->minor,
		mixer_link_version->patch);
	
	fprintf(stream, "\tCompile flags: ");
	#ifdef ENABLE_LOKI_QUEUE_FIX_HACK
		fprintf(stream, "ENABLE_LOKI_QUEUE_FIX_HACK ");
	#endif
	#ifdef ENABLE_ALMIXER_THREADS
		fprintf(stream, "ENABLE_ALMIXER_THREADS ");
	#endif
	#ifdef ENABLE_ALC_SYNC
		fprintf(stream, "ENABLE_ALC_SYNC ");
	#endif
	fprintf(stream, "\n");
}


ALint ALmixer_AllocateChannels(ALint numchans)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}

#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_AllocateChannels(numchans);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

	
ALint ALmixer_ReserveChannels(ALint num)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
	
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_ReserveChannels(num);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

	


static ALmixer_Data* DoLoad(Sound_Sample* sample, ALuint buffersize, ALboolean decode_mode_is_predecoded, ALuint max_queue_buffers, ALuint num_startup_buffers, ALuint suggested_number_of_buffers_to_queue_per_update_pass, ALuint access_data)
{
	ALuint bytes_decoded;
	ALmixer_Data* ret_data;
	ALenum error;

	/* Validate arguments. */
	if (!sample)
	{
		return NULL;
	}

	/* Do not continue if the format of the decoded audio cannot be played by OpenAL.
	 * Example: This can happen with 5.1 surround sound encoded audio if the OpenAL library
	 *          does not support the "AL_FORMAT_51CHN*" extension formats.
	 */
	{
		ALenum alFormatType = TranslateFormat(&sample->desired);
		if ((0 == alFormatType) || ((ALenum)-1 == alFormatType))
		{
			ALmixer_SetError(
					"DoLoad: Cannot play given audio format (channels=%d, bitrate=%d) on this platform",
					sample->desired.channels, sample->desired.rate);
			Sound_FreeSample(sample);
			return NULL;
		}
	}

	/* Allocate memory */
	ret_data = (ALmixer_Data *)malloc(sizeof(ALmixer_Data));
	if (NULL == ret_data) 
	{
		Sound_FreeSample(sample);
		ALmixer_SetError("DoLoad: Out of memory");
		return(NULL);
	}

	/* Initialize the data fields */
	
	/* Set the Sound_Sample pointer */
	ret_data->sample = sample;

	/* Flag the data to note that it is not in use */
	ret_data->in_use = 0;

	/* Initialize remaining flags */
	ret_data->total_time = -1;
	ret_data->eof = 0;

	/* Just initialize */
	ret_data->num_buffers_in_use = 0;

	/* Just initialize */
	ret_data->total_bytes = 0;

	/* Just initialize */
	ret_data->loaded_bytes = 0;
	
	/* Set the max queue buffers (minimum must be 2) */
	if(max_queue_buffers < 2)
	{
		max_queue_buffers = ALMIXER_DEFAULT_QUEUE_BUFFERS;
	}
	ret_data->max_queue_buffers = max_queue_buffers;
	/* Set up the start up buffers */
	if(0 == num_startup_buffers)
	{
		num_startup_buffers = ALMIXER_DEFAULT_STARTUP_BUFFERS;
	}
	/* Make sure start up buffers is less or equal to max_queue_buffers */
	if(num_startup_buffers > max_queue_buffers)
	{
		num_startup_buffers = max_queue_buffers;
	}
	ret_data->num_startup_buffers = num_startup_buffers;

	/* Set up the update pass buffers */
	if(0 == suggested_number_of_buffers_to_queue_per_update_pass)
	{
		suggested_number_of_buffers_to_queue_per_update_pass = ALMIXER_DEFAULT_BUFFERS_TO_QUEUE_PER_UPDATE_PASS;
	}
	/* Make sure update pass up buffers is less or equal to max_queue_buffers */
	if(suggested_number_of_buffers_to_queue_per_update_pass > max_queue_buffers)
	{
		suggested_number_of_buffers_to_queue_per_update_pass = max_queue_buffers;
	}
	ret_data->num_target_buffers_per_pass = suggested_number_of_buffers_to_queue_per_update_pass;

	ret_data->buffer_map_list = NULL;
	ret_data->current_buffer = 0;
	
	ret_data->circular_buffer_queue = NULL;
	
	/* Now decode and load the data into a data chunk */
	/* Different cases for Streamed and Predecoded 
	 * Streamed might turn into a predecoded if buffersize
	 * is large enough */
	if(AL_FALSE == decode_mode_is_predecoded)
	{
		bytes_decoded = Sound_Decode(sample);
		if(sample->flags & SOUND_SAMPLEFLAG_ERROR)
		{
			ALmixer_SetError("DoLoad: Sound_Decode: %s", Sound_GetError());
			Sound_FreeSample(sample);
			_free(ret_data);
			return NULL;
		}

		/* If no data, return an error */
		if(0 == bytes_decoded)
		{
			ALmixer_SetError("File has no data");
			Sound_FreeSample(sample);
			_free(ret_data);
			return NULL;
		}
		
		/* Note, currently, my Ogg conservative modifications
		 * prevent EOF from being detected in the first read
		 * because of the weird packet behavior of ov_read().
		 * The EAGAIN will get set, but not the EOF.
		 * I don't know the best way to handle this,
		 * so for now, Ogg's can only be explicitly
		 * predecoded.
		 */

		/* Correction: Since we no longer actually keep the 
		 * streamed data we read here (we rewind and throw
		 * it away, and start over on Play), it is
		 * safe to read another chunk to see if we've hit EOF
		 */
		if(sample->flags & SOUND_SAMPLEFLAG_EAGAIN)
		{
			bytes_decoded = Sound_Decode(sample);
			if(sample->flags & SOUND_SAMPLEFLAG_ERROR)
			{
				ALmixer_SetError("DoLoad: Sound_Decode: %s", Sound_GetError());
				Sound_FreeSample(sample);
				_free(ret_data);
				return NULL;
			}
		}


		/* If we found an EOF, the entire file was 
		 * decoded, so we can treat it like one.
		 */
		
		if(sample->flags & SOUND_SAMPLEFLAG_EOF)
		{
			/*
	fprintf(stderr, "We got LUCKY! File is predecoded even though STREAM was requested\n");
*/
			ret_data->decoded_all = 1;
			/* Need to keep this information around for
			 * seek and rewind abilities.
			 */
			ret_data->total_bytes = bytes_decoded;
			/* For now, the loaded bytes is the same as total bytes, but
			 * this could change during a seek operation
			 */
			ret_data->loaded_bytes = bytes_decoded;

			/* Let's compute the total playing time 
			 * SDL_sound does not yet provide this (we're working on
			 * that at the moment...)
			 */
			ret_data->total_time = Compute_Total_Time(&sample->desired, bytes_decoded);

			/* Create one element in the buffer array for data for OpanAL */
			ret_data->buffer = (ALuint*)malloc( sizeof(ALuint) );
			if(NULL == ret_data->buffer)
			{
				ALmixer_SetError("DoLoad: Out of Memory");
				Sound_FreeSample(sample);
				_free(ret_data);
				return NULL;
			}
			/* Clear the error code */
			alGetError();
			/* Now generate an OpenAL buffer using that first element */
			_alGenBuffers(1, ret_data->buffer);
			if( (error = alGetError()) != AL_NO_ERROR)
			{
				ALmixer_SetError("_alGenBuffers failed: %s", alGetString(error));
				Sound_FreeSample(sample);
				_free(ret_data->buffer);
				_free(ret_data);
				return NULL;
			}
			
			
			/* Now copy the data to the OpenAL buffer */
			/* We can't just set a pointer because the API needs
			 * its own copy to assist hardware acceleration */
			alBufferData(ret_data->buffer[0], 
				TranslateFormat(&sample->desired), 
				sample->buffer,
				bytes_decoded,
				sample->desired.rate
			);
			if( (error = alGetError()) != AL_NO_ERROR)
			{
				ALmixer_SetError("alBufferData failed: %s", alGetString(error));
				Sound_FreeSample(sample);
				_alDeleteBuffers(1, ret_data->buffer);
				_free(ret_data->buffer);
				_free(ret_data);
				return NULL;
			}

			/* We should be done with the sample since it's all 
			 * predecoded. So we can free the memory */
			
			/* Additional notes:
			 * We need to keep data around in case Seek() is needed
			 * or other Sound_AudioInfo is needed.
			 * This can either be done by not deleting the sample,
			 * or it can be done by dynamically recreating it
			 * when we need it. 
			 */
			/* Since OpenAL won't let us retrieve it
			 * (aka dynamically), we have to keep the Sample
			 * around because since the user requested
			 * streamed and we offered predecoded,
			 * we don't want to mess up the user who
			 * was expecting seek support
			 * So Don't Do anything
			 */
			/*
			if(0 == access_data)
			{
				Sound_FreeSample(sample);
				ret_data->sample = NULL;
			}
			*/	
			/* Else, We keep a copy of the sample around.
			 * so don't do anything.
			 */
					
#if 0
#if defined(DISABLE_PREDECODED_SEEK)
			Sound_FreeSample(sample);
			ret_data->sample = NULL;
#elif !defined(DISABLE_SEEK_MEMORY_OPTIMIZATION)
			Sound_FreeSample(sample);
			ret_data->sample = NULL;
#else
			/* We keep a copy of the sample around.
			 * so don't do anything.
			 */
#endif
#endif
			/* okay we're done here */
			
		}
		/* Else, we need to stream the data, so we'll
		 * create multple buffers for queuing */
		else
		{
			/*
	fprintf(stderr, "Loading streamed data (not lucky)\n");
	*/
			ret_data->decoded_all = 0;

			/* This information is for predecoded.
			 * Set to 0, since we don't know.
			 */
			ret_data->total_bytes = 0;

			ret_data->total_time = Sound_GetDuration(sample);

			/* Create buffers for data
			 */
			ret_data->buffer = (ALuint*)malloc( sizeof(ALuint) * max_queue_buffers);
			if(NULL == ret_data->buffer)
			{
				ALmixer_SetError("Out of Memory");
				Sound_FreeSample(sample);
				_free(ret_data);
				return NULL;
			}

			/* Clear the error code */
			alGetError();
			/* Now generate an OpenAL buffer using that first element */
			_alGenBuffers(max_queue_buffers, ret_data->buffer);
			if( (error = alGetError()) != AL_NO_ERROR)
			{
				ALmixer_SetError("alGenBuffers failed: %s\n", alGetString(error));
				Sound_FreeSample(sample);
				_free(ret_data->buffer);
				_free(ret_data);
				return NULL;
			}
			
			/* Redesign: Okay, because of the unqueuing problems and such,
			 * I've decided to redesign where and how queuing is handled.
			 * Before, everything was queued up here. However, this
			 * placed a penalty on load and made performance inconsistent
			 * when samples had to be rewound. It did make things easier
			 * to queue because I could let OpenAL decide which buffer
			 * needed to be queued next.
			 * Now, I'm going to push off the queuing to the actual
			 * Play() command. I'm going to add some book keeping,
			 * and allow for additional buffers to be filled at later
			 * times. 
			 */


			/* So first of all, because of I already decoded the sample
			 * for testing, I need to decide what to do with it.
			 * The best thing would be be to alBufferData() it.
			 * The problem is it may conflict with the rest of 
			 * the system because everything now assumes buffers
			 * are entirely stripped (because of the unqueing
			 * problem).
			 * So it looks like I have to do the crappy thing 
			 * and throw away the data, and rewind.
			 */
			
			if(0 == Sound_Rewind(ret_data->sample))
			{
				ALmixer_SetError("Cannot use sample for streamed data because it must be rewindable: %s", Sound_GetError() );
				Sound_FreeSample(sample);
				_free(ret_data->buffer);
				_free(ret_data);
				return NULL;
			}
			

			/* If the user has selected access_data, we need to 
			 * keep copies of the queuing buffers around because
			 * OpenAL won't let us access the data.
			 * Allocate the memory for the buffers here
			 * and initialize the albuffer-index map
			 */
			if(access_data)
			{
				ALuint j;
				/* Create buffers for data access
				 * Should be the same number as the number of queue buffers
				 */
				ret_data->buffer_map_list = (ALmixer_Buffer_Map*)malloc( sizeof(ALmixer_Buffer_Map) * max_queue_buffers);
				if(NULL == ret_data->buffer_map_list)
				{
					ALmixer_SetError("Out of Memory");
					Sound_FreeSample(sample);
					_free(ret_data->buffer);
					_free(ret_data);
					return NULL;
				}

				ret_data->circular_buffer_queue = CircularQueueUnsignedInt_CreateQueue(max_queue_buffers);
				if(NULL == ret_data->circular_buffer_queue)
				{
					ALmixer_SetError("Out of Memory");
					_free(ret_data->buffer_map_list);
					Sound_FreeSample(sample);
					_free(ret_data->buffer);
					_free(ret_data);
					return NULL;
				}


				for(j=0; j<max_queue_buffers; j++)
				{
					ret_data->buffer_map_list[j].albuffer = ret_data->buffer[j];
					ret_data->buffer_map_list[j].index = j;
					ret_data->buffer_map_list[j].num_bytes = 0;
					ret_data->buffer_map_list[j].data = (ALbyte*)malloc( sizeof(ALbyte) * buffersize);
					if(NULL == ret_data->buffer_map_list[j].data)
					{
						ALmixer_SetError("Out of Memory");
						break;
					}
				}
				/* If an error happened, we have to clean up the memory */
				if(j < max_queue_buffers)
				{
					ALmixer_SetError("################## Buffer allocation failed");
					fprintf(stderr, "################## Buffer allocation failed\n");
					while(j>0)
					{
						_free(ret_data->buffer_map_list[j].data);
						j--;
					}
					/* Delete for j=0 because the while loop misses the last one */
					_free(ret_data->buffer_map_list[j].data);

					_free(ret_data->buffer_map_list);
					CircularQueueUnsignedInt_FreeQueue(ret_data->circular_buffer_queue);
					Sound_FreeSample(sample);
					_free(ret_data->buffer);
					_free(ret_data);
					return NULL;
				}

				/* The Buffer_Map_List must be sorted by albuffer for binary searches
	 			*/
				qsort(ret_data->buffer_map_list, max_queue_buffers, sizeof(ALmixer_Buffer_Map), Compare_Buffer_Map);
			} /* End if access_data==true */

			
		} /* End of do stream */
	} /* end of DECODE_STREAM */
	/* User requested decode all (easy, nothing to figure out) */
	else if(AL_TRUE == decode_mode_is_predecoded)
	{
#ifndef ALMIXER_DISABLE_PREDECODED_PRECOMPUTE_BUFFER_SIZE_OPTIMIZATION
		/* SDL_sound (behind the scenes) seems to loop on buffer_size chunks 
		 * until the buffer is filled. It seems like we can 
		 * do much better and precompute the size of the buffer
		 * so looping isn't needed.
		 * WARNING: Due to the way SDL_sound is currently implemented,
		 * this may waste a lot of memory up front.
		 * SDL_sound seems to pre-create a buffer of the requested size,
		 * but on DecodeAll, an entirely new buffer is created and 
		 * everything is memcpy'd into the new buffer in read chunks
		 * of the buffer_size. This means we need roughly twice the memory
		 * to load a file.
		 */
		ALint sound_duration = Sound_GetDuration(sample);
		if(sound_duration > 0)
		{
			size_t total_bytes = Compute_Total_Bytes_With_Frame_Padding(&sample->desired, (ALuint)sound_duration);
			int buffer_resize_succeeded = Sound_SetBufferSize(sample, (int) total_bytes);
			if(0 == buffer_resize_succeeded)
			{
				ALmixer_SetError("DoLoad: Sound_SetBufferSize: %s", Sound_GetError());
				Sound_FreeSample(sample);
				_free(ret_data);
				return NULL;
			}
		}
#endif /* ALMIXER_DISABLE_PREDECODED_PRECOMPUTE_BUFFER_SIZE_OPTIMIZATION */
		bytes_decoded = Sound_DecodeAll(sample);
		if(sample->flags & SOUND_SAMPLEFLAG_ERROR)
		{
			ALmixer_SetError("DoLoadL Sound_DecodeAll: %s", Sound_GetError());
			Sound_FreeSample(sample);
			_free(ret_data);
			return NULL;
		}

		/* If no data, return an error */
		if(0 == bytes_decoded)
		{
			ALmixer_SetError("File has no data");
			Sound_FreeSample(sample);
			_free(ret_data);
			return NULL;
		}
		

		ret_data->decoded_all = 1;
		/* Need to keep this information around for
		 * seek and rewind abilities.
		 */
		ret_data->total_bytes = bytes_decoded;
		/* For now, the loaded bytes is the same as total bytes, but
		 * this could change during a seek operation
		 */
		ret_data->loaded_bytes = bytes_decoded;

		/* Let's compute the total playing time 
		 * SDL_sound does not yet provide this (we're working on
		 * that at the moment...)
		 */
		ret_data->total_time = Compute_Total_Time(&sample->desired, bytes_decoded);

		/* Create one element in the buffer array for data for OpanAL */
		ret_data->buffer = (ALuint*)malloc( sizeof(ALuint) );
		if(NULL == ret_data->buffer)
		{
			ALmixer_SetError("Out of Memory");
			Sound_FreeSample(sample);
			_free(ret_data);
			return NULL;
		}
		/* Clear the error code */
		alGetError();
		/* Now generate an OpenAL buffer using that first element */
		_alGenBuffers(1, ret_data->buffer);
		if( (error = alGetError()) != AL_NO_ERROR)
		{
			ALmixer_SetError("alGenBuffers failed: %s\n", alGetString(error));
			Sound_FreeSample(sample);
			_free(ret_data->buffer);
			_free(ret_data);
			return NULL;
		}
		/*
		fprintf(stderr, "Actual rate=%d, desired=%d\n", sample->actual.rate, sample->desired.rate);
*/
		/* Now copy the data to the OpenAL buffer */
		/* We can't just set a pointer because the API needs
		 * its own copy to assist hardware acceleration */
		alBufferData(ret_data->buffer[0], 
			TranslateFormat(&sample->desired), 
			sample->buffer,
			bytes_decoded,
			sample->desired.rate
		);
		if( (error = alGetError()) != AL_NO_ERROR)
		{
			ALmixer_SetError("alBufferData failed: %s\n", alGetString(error));
			Sound_FreeSample(sample);
			_alDeleteBuffers(1, ret_data->buffer);
			_free(ret_data->buffer);
			_free(ret_data);
			return NULL;
		}

		/* We should be done with the sample since it's all 
		 * predecoded. So we can free the memory */
		/* Need to keep around because Seek() needs it */
		
		/* Additional notes:
		 * We need to keep data around in case Seek() is needed
		 * or other Sound_AudioInfo is needed.
		 * This can either be done by not deleting the sample,
		 * or it can be done by dynamically recreating it
		 * when we need it. 
		 * Update: I think now it's up to the user by passing the 
		 * access_data flag. If they set the flag, then they get 
		 * data callbacks and seek support. If not, then they can
		 * get all that stuff at the expense of keeping extra memory 
		 * around.
		 */
		if(0 == access_data)
		{
			Sound_FreeSample(sample);
			ret_data->sample = NULL;
		}
			
		/* Else, We keep a copy of the sample around.
		 * so don't do anything.
		 */
#if 0
#if defined(DISABLE_PREDECODED_SEEK) 
		Sound_FreeSample(sample);
		ret_data->sample = NULL;
#elif !defined(DISABLE_SEEK_MEMORY_OPTIMIZATION)
		Sound_FreeSample(sample);
		ret_data->sample = NULL;
#else
		/* We keep a copy of the sample around.
		 * so don't do anything.
		 */
#endif
#endif

		/* okay we're done here */
	}
	else
	{
		/* Shouldn't get here */
		ALmixer_SetError("Unknown decode mode");
		Sound_FreeSample(sample);
		_free(ret_data);
		return NULL;
	}
		
	/* Add the ALmixerData to an internal linked list so we can delete it on 
	 * quit and avoid messy dangling issues with Sound_Quit
	 */
	LinkedList_PushBack(s_listOfALmixerData, ret_data);
	return ret_data;
}


/* This will load a sample for us. Most of the uglyness is
 * error checking and the fact that streamed/predecoded files
 * must be treated differently.
 * I don't like the AudioInfo parameter. I removed it once,
 * but the system will fail on RAW samples because the user
 * must specify it, so I had to bring it back.
 * Remember I must close the rwops if there is an error before NewSample()
 */
ALmixer_Data* ALmixer_LoadSample_RW(ALmixer_RWops* rwops, const char* fileext, ALuint buffersize, ALboolean decode_mode_is_predecoded, ALuint max_queue_buffers, ALuint num_startup_buffers, ALuint suggested_number_of_buffers_to_queue_per_update_pass, ALuint access_data)
{
	Sound_Sample* sample = NULL;
	Sound_AudioInfo target;
	
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return NULL;
	}

	/* Initialize target values to defaults 
	 * 0 tells SDL_sound to use the "actual" values
	 */
	target.channels = 0;
	target.rate = 0;
#if 0
	/* This requires my new additions to SDL_sound. It will
	 * convert the sample to the proper endian order.
	 * If the actual is 8-bit, it will do unsigned, if 
	 * the actual is 16-bit, it will do signed.
	 * I'm told by Ryan Gordon that OpenAL prefers the signedness
	 * in this way.
	 */
	target.format = AUDIO_U8S16SYS;
#else
	target.format = AUDIO_S16SYS;
#endif
	
	/* Set a default buffersize if needed */
	if(0 == buffersize)
	{
		buffersize = ALMIXER_DEFAULT_BUFFERSIZE;
	}
	
	sample = Sound_NewSample(rwops, fileext, &target, buffersize);
	if(NULL == sample)
	{
		ALmixer_SetError("ALmixer_LoadSample_RW: Sound_NewSample: %s", Sound_GetError());
		return NULL;
	}

	return( DoLoad(sample, buffersize, decode_mode_is_predecoded, max_queue_buffers, num_startup_buffers, suggested_number_of_buffers_to_queue_per_update_pass, access_data));
}



/* This will load a sample for us from 
 * a file (instead of RWops). Most of the uglyness is
 * error checking and the fact that streamed/predecoded files
 * must be treated differently.
 */
ALmixer_Data* ALmixer_LoadSample(const char* filename, ALuint buffersize, ALboolean decode_mode_is_predecoded, ALuint max_queue_buffers, ALuint num_startup_buffers, ALuint suggested_number_of_buffers_to_queue_per_update_pass, ALuint access_data)
{
	Sound_Sample* sample = NULL;
	Sound_AudioInfo target;
	
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return NULL;
	}
	
	/* Initialize target values to defaults 
	 * 0 tells SDL_sound to use the "actual" values
	 */
	target.channels = 0;
	target.rate = 0;
	
	if(0 == buffersize)
	{
		buffersize = ALMIXER_DEFAULT_BUFFERSIZE;
	}
#if 0
	/* This requires my new additions to SDL_sound. It will
	 * convert the sample to the proper endian order.
	 * If the actual is 8-bit, it will do unsigned, if 
	 * the actual is 16-bit, it will do signed.
	 * I'm told by Ryan Gordon that OpenAL prefers the signedness
	 * in this way.
	 */
	target.format = AUDIO_U8S16SYS;
#else
	target.format = AUDIO_S16SYS;
#endif
	
#if 0	
	/* Okay, here's a messy hack. The problem is that we need
	 * to convert the sample to have the correct bitdepth,
	 * endian order, and signedness values. 
	 * The bit depth is 8 or 16.
	 * The endian order is the native order of the system.
	 * The signedness depends on what the original value
	 * of the sample. Unfortunately, we can't specify these
	 * values until we after we already know what the original
	 * values were for bitdepth and signedness. 
	 * So we must open the file once to get the values, 
	 * then close it, and then reopen it with the 
	 * correct desired target values.
	 * I tried changing the sample->desired field after 
	 * the NewSample call, but it had no effect, so 
	 * it looks like it must be set on open.
	 */
	/* Pick a small buffersize for the first open to not
	 * waste much time allocating memory */
	sample = Sound_NewSampleFromFile(filename, NULL, 512);
	if(NULL == sample)
	{
		ALmixer_SetError("ALmixer_LoadSample: Sound_NewSampleFromFile: %s", Sound_GetError());
		return NULL;
	}

	bit_depth = GetBitDepth(sample->actual.format);
	signedness_value = GetSignednessValue(sample->actual.format);
	if(8 == bit_depth)
	{
		/* If 8 bit, then we don't have to worry about 
		 * endian issues. We can just use the actual format
		 * value and it should do the right thing
		 */
		target.format = sample->actual.format;
	}
	else
	{
		/* We'll assume it's 16-bit, and if it's not
		 * hopefully SDL_sound will return an error, 
		 * or let us convert to 16-bit
		 */
		/* Now we need to get the correct signedness */
		if(ALMIXER_UNSIGNED_VALUE == signedness_value)
		{
			/* Set to Unsigned 16-bit, system endian order */
			target.format = AUDIO_U16SYS;
		}
		else
		{
			/* Again, we'll assume it's Signed 16-bit system order
			 * or force the conversion and hope it works out
			 */
			target.format = AUDIO_S16SYS;
		}
	}
	
	/* Now we have the correct info. We need to close and reopen */
	Sound_FreeSample(sample);
#endif

	sample = Sound_NewSampleFromFile(filename, &target, buffersize);
	if(NULL == sample)
	{
		ALmixer_SetError("ALmixer_LoadSample: Sound_NewSampleFromFile: %s", Sound_GetError());
		return NULL;
	}

	/*
	fprintf(stderr, "Correction test: Actual rate=%d, desired=%d, actual format=%d, desired format=%d\n", sample->actual.rate, sample->desired.rate, sample->actual.format, sample->desired.format);
	*/
	return( DoLoad(sample, buffersize, decode_mode_is_predecoded, max_queue_buffers, num_startup_buffers, suggested_number_of_buffers_to_queue_per_update_pass, access_data));
}


/* This is a back door for RAW samples or if you need the
 * AudioInfo field. Use at your own risk.
 */
ALmixer_Data* ALmixer_LoadSample_RAW_RW(ALmixer_RWops* rwops, const char* fileext, ALmixer_AudioInfo* desired, ALuint buffersize, ALboolean decode_mode_is_predecoded, ALuint max_queue_buffers, ALuint num_startup_buffers, ALuint suggested_number_of_buffers_to_queue_per_update_pass, ALuint access_data)
{
	Sound_Sample* sample = NULL;
	Sound_AudioInfo sound_desired;

	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return NULL;
	}
	
	/* Rather than copying the data from struct to struct, I could just
	 * cast the thing since the structs are meant to be identical. 
	 * But if SDL_sound changes it's implementation, bad things
	 * will probably happen. (Or if I change my implementation and 
	 * forget about the cast, same bad scenario.) Since this is a load
	 * function, performance of this is negligible.
	 */
	if(NULL == desired)
	{
		sample = Sound_NewSample(rwops, fileext, NULL, buffersize);
	}
	else
	{
	   sound_desired.format = desired->format;
	   sound_desired.channels = desired->channels;
	   sound_desired.rate = desired->rate;
	   sample = Sound_NewSample(rwops, fileext, &sound_desired, buffersize);
	}
	if(NULL == sample)
	{
		ALmixer_SetError("ALmixer_LoadSample_RAW_RW: Sound_NewSample: %s", Sound_GetError());
		return NULL;
	}
	return( DoLoad(sample, buffersize, decode_mode_is_predecoded, max_queue_buffers, num_startup_buffers,suggested_number_of_buffers_to_queue_per_update_pass, access_data));
}




/* This is a back door for RAW samples or if you need the
 * AudioInfo field. Use at your own risk.
 */
ALmixer_Data* ALmixer_LoadSample_RAW(const char* filename, ALmixer_AudioInfo* desired, ALuint buffersize, ALboolean decode_mode_is_predecoded, ALuint max_queue_buffers, ALuint num_startup_buffers, ALuint suggested_number_of_buffers_to_queue_per_update_pass, ALuint access_data)
{
	Sound_Sample* sample = NULL;
	Sound_AudioInfo sound_desired;

	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return NULL;
	}
	
	/* Rather than copying the data from struct to struct, I could just
	 * cast the thing since the structs are meant to be identical. 
	 * But if SDL_sound changes it's implementation, bad things
	 * will probably happen. (Or if I change my implementation and 
	 * forget about the cast, same bad scenario.) Since this is a load
	 * function, performance of this is negligible.
	 */
	if(NULL == desired)
	{
		sample = Sound_NewSampleFromFile(filename, NULL, buffersize);
	}
	else
	{
	   sound_desired.format = desired->format;
	   sound_desired.channels = desired->channels;
	   sound_desired.rate = desired->rate;
	   sample = Sound_NewSampleFromFile(filename, &sound_desired, buffersize);
	}

	if(NULL == sample)
	{
		ALmixer_SetError("ALmixer_LoadSample_RAW: Sound_NewSampleFromFile: %s", Sound_GetError());
		return NULL;
	}
	return( DoLoad(sample, buffersize, decode_mode_is_predecoded, max_queue_buffers, num_startup_buffers, suggested_number_of_buffers_to_queue_per_update_pass, access_data));
}


void ALmixer_FreeData(ALmixer_Data* data)
{
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return;
	}

	/* Bypass if in interruption event */
	/* FIXME: Buffers are connected to devices, sources are connected to contexts. I should still be able to delete the buffers even if there is no context. */
	if(NULL == alcGetCurrentContext())
	{
		fprintf(stderr, "ALmixer_FreeData: Programmer Error. You cannot delete data when the OpenAL content is currently NULL. You may have already called ALmixer_Quit() or are in an interruption event\n");
		return;
	}
	
	Internal_FreeData(data);
}

ALint ALmixer_GetTotalTime(ALmixer_Data* data)
{
	if(NULL == data)
	{
		return -1;
	}

	return data->total_time;
}

/* This function will look up the source for the corresponding channel */
/* Must return 0 on error instead of -1 because of unsigned int */
ALuint ALmixer_GetSource(ALint channel)
{
	ALuint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return 0;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_GetSource(channel);
#ifdef ENABLE_ALMIXER_THREADS	
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

/* This function will look up the channel for the corresponding source */
ALint ALmixer_GetChannel(ALuint source)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_GetChannel(source);
#ifdef ENABLE_ALMIXER_THREADS	
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_FindFreeChannel(ALint start_channel)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_FindFreeChannel(start_channel);
#ifdef ENABLE_ALMIXER_THREADS	
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}



/* API update function.
 * It should return the number of buffers that were 
 * queued during the call. The value might be
 * used to guage how long you might wait to
 * call the next update loop in case you are worried
 * about preserving CPU cycles. The idea is that
 * when a buffer is queued, there was probably some
 * CPU intensive looping which took awhile.
 * It's mainly provided as a convenience.
 * Timing the call with ALmixer_GetTicks() would produce
 * more accurate information.
 * Returns a negative value if there was an error,
 * the value being the number of errors.
 */
ALint ALmixer_Update()
{
#ifdef ENABLE_ALMIXER_THREADS
	/* The thread will handle all updates by itself.
	 * Don't allow the user to explicitly call update.
	 */
	return 0;
#else
	return( Update_ALmixer(NULL) );
#endif
}



void ALmixer_SetPlaybackFinishedCallback(void (*playback_finished_callback)(ALint which_channel, ALuint al_source, ALmixer_Data* almixer_data, ALboolean finished_naturally, void* user_data), void* user_data)
{
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	Channel_Done_Callback = playback_finished_callback;
	Channel_Done_Callback_Userdata = user_data;
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
}


void ALmixer_SetPlaybackDataCallback(void (*playback_data_callback)(ALint which_chan, ALuint al_source, ALbyte* data, ALuint num_bytes, ALuint frequency, ALubyte channels, ALubyte bit_depth, ALboolean is_unsigned, ALboolean decode_mode_is_predecoded, ALuint length_in_msec, void* user_data), void* user_data)
{
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	Channel_Data_Callback = playback_data_callback;
	Channel_Data_Callback_Userdata = user_data;
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
}





ALint ALmixer_PlayChannelTimed(ALint channel, ALmixer_Data* data, ALint loops, ALint ticks)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}		
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_PlayChannelTimed(channel, data, loops, ticks);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}


/* In case the user wants to specify a source instead of a channel,
 * they may use this function. This function will look up the 
 * source-to-channel map, and convert the call into a
 * PlayChannelTimed() function call.
 * Returns the channel it's being played on.
 * Note: If you are prefer this method, then you need to be careful
 * about using PlayChannel, particularly if you request the
 * first available channels because source and channels have 
 * a one-to-one mapping in this API. It is quite easy for 
 * a channel/source to already be in use because of this.
 * In this event, an error message will be returned to you.
 */
ALuint ALmixer_PlaySourceTimed(ALuint source, ALmixer_Data* data, ALint loops, ALint ticks)
{
	ALuint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return 0;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_PlaySourceTimed(source, data, loops, ticks);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}


/* Will return the number of channels halted
 * or -1 for error
 */
ALint ALmixer_HaltChannel(ALint channel)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_HaltChannel(channel, AL_FALSE);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

/* Will return the number of channels halted
 * or 0 for error
 */
ALint ALmixer_HaltSource(ALuint source)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_HaltSource(source, AL_FALSE);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}


/* This will rewind the SDL_Sound sample for streamed
 * samples and start buffering up the data for the next
 * playback. This may require samples to be halted
 */
ALboolean ALmixer_RewindData(ALmixer_Data* data)
{
	ALboolean retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return AL_FALSE;
	}	
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_RewindData(data);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_RewindChannel(ALint channel)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_RewindChannel(channel);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_RewindSource(ALuint source)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}	
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_RewindSource(source);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_PauseChannel(ALint channel)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_PauseChannel(channel);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_PauseSource(ALuint source)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_PauseSource(source);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_ResumeChannel(ALint channel)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_ResumeChannel(channel);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_ResumeSource(ALuint source)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_ResumeSource(source);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

/* Might consider setting eof to 0 as a "feature"
 * This will allow seek to end to stay there because
 * Play automatically rewinds if at the end */
ALboolean ALmixer_SeekData(ALmixer_Data* data, ALuint msec)
{
	ALboolean retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_SeekData(data, msec);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_SeekChannel(ALint channel, ALuint msec)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_SeekChannel(channel, msec);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_SeekSource(ALuint source, ALuint msec)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_SeekSource(source, msec);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_FadeInChannelTimed(ALint channel, ALmixer_Data* data, ALint loops, ALuint fade_ticks, ALint expire_ticks)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_FadeInChannelTimed(channel, data, loops, fade_ticks, expire_ticks);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALuint ALmixer_FadeInSourceTimed(ALuint source, ALmixer_Data* data, ALint loops, ALuint fade_ticks, ALint expire_ticks)
{
	ALuint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return 0;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_FadeInSourceTimed(source, data, loops, fade_ticks, expire_ticks);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_FadeOutChannel(ALint channel, ALuint ticks)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_FadeOutChannel(channel, ticks);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}
	
ALint ALmixer_FadeOutSource(ALuint source, ALuint ticks)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return 0;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_FadeOutSource(source, ticks);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_FadeChannel(ALint channel, ALuint ticks, ALfloat volume)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return 0;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_FadeChannel(channel, ticks, volume);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_FadeSource(ALuint source, ALuint ticks, ALfloat volume)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_FadeSource(source, ticks, volume);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}


ALboolean ALmixer_SetVolumeChannel(ALint channel, ALfloat volume)
{
	ALboolean retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return AL_FALSE;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_SetVolumeChannel(channel, volume);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALboolean ALmixer_SetVolumeSource(ALuint source, ALfloat volume)
{
	ALboolean retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return AL_FALSE;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_SetVolumeSource(source, volume);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALfloat ALmixer_GetVolumeChannel(ALint channel)
{
	ALfloat retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1.0f;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_GetVolumeChannel(channel);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;	
}

ALfloat ALmixer_GetVolumeSource(ALuint source)
{
	ALfloat retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1.0f;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_GetVolumeSource(source);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;	
}

ALboolean ALmixer_SetMaxVolumeChannel(ALint channel, ALfloat volume)
{
	ALboolean retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return AL_FALSE;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_SetMaxVolumeChannel(channel, volume);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALboolean ALmixer_SetMaxVolumeSource(ALuint source, ALfloat volume)
{
	ALboolean retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return AL_FALSE;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_SetMaxVolumeSource(source, volume);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALfloat ALmixer_GetMaxVolumeChannel(ALint channel)
{
	ALfloat retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1.0f;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_GetMaxVolumeChannel(channel);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;	
}

ALfloat ALmixer_GetMaxVolumeSource(ALuint source)
{
	ALfloat retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1.0f;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_GetMaxVolumeSource(source);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;	
}


ALboolean ALmixer_SetMinVolumeChannel(ALint channel, ALfloat volume)
{
	ALboolean retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return AL_FALSE;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_SetMinVolumeChannel(channel, volume);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALboolean ALmixer_SetMinVolumeSource(ALuint source, ALfloat volume)
{
	ALboolean retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return AL_FALSE;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_SetMinVolumeSource(source, volume);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALfloat ALmixer_GetMinVolumeChannel(ALint channel)
{
	ALfloat retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1.0f;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_GetMinVolumeChannel(channel);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;	
}

ALfloat ALmixer_GetMinVolumeSource(ALuint source)
{
	ALfloat retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1.0f;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_GetMinVolumeSource(source);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;	
}



ALboolean ALmixer_SetMasterVolume(ALfloat volume)
{
	ALboolean retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return AL_FALSE;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_SetMasterVolume(volume);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;	
}

ALfloat ALmixer_GetMasterVolume()
{
	ALfloat retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1.0f;
	}		
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_GetMasterVolume();
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;	
}

ALint ALmixer_ExpireChannel(ALint channel, ALint ticks)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_ExpireChannel(channel, ticks);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_ExpireSource(ALuint source, ALint ticks)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_ExpireSource(source, ticks);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_IsActiveChannel(ALint channel)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_QueryChannel(channel);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_IsActiveSource(ALuint source)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_QuerySource(source);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}


ALint ALmixer_IsPlayingChannel(ALint channel)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_PlayingChannel(channel);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_IsPlayingSource(ALuint source)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_PlayingSource(source);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}


ALint ALmixer_IsPausedChannel(ALint channel)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_PausedChannel(channel);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALint ALmixer_IsPausedSource(ALuint source)
{
	ALint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return -1;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_PausedSource(source);
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}


ALuint ALmixer_CountAllFreeChannels()
{
	ALuint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return 0;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_CountAllFreeChannels();
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALuint ALmixer_CountUnreservedFreeChannels()
{
	ALuint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return 0;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_CountUnreservedFreeChannels();
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALuint ALmixer_CountAllUsedChannels()
{
	ALuint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return 0;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_CountAllUsedChannels();
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALuint ALmixer_CountUnreservedUsedChannels()
{
	ALuint retval;
	if( (AL_FALSE == ALmixer_Initialized) || (AL_TRUE == g_inInterruption) )
	{
		return 0;
	}
#ifdef ENABLE_ALMIXER_THREADS
	SDL_LockMutex(s_simpleLock);
#endif
	retval = Internal_CountUnreservedUsedChannels();
#ifdef ENABLE_ALMIXER_THREADS
	SDL_UnlockMutex(s_simpleLock);
#endif
	return retval;
}

ALboolean ALmixer_IsPredecoded(ALmixer_Data* data)
{
	if(NULL == data)
	{
		return AL_FALSE;
	}
	return data->decoded_all;
}

ALboolean ALmixer_CompiledWithThreadBackend()
{
#ifdef ENABLE_ALMIXER_THREADS
	return AL_TRUE;
#else
	return AL_FALSE;
#endif
}




