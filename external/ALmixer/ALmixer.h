/*
    ALmixer:  A library to make playing pre-loaded sounds and streams easier
	with high performance and potential access to OpenAL effects.
    Copyright 2002, 2010 Eric Wing <ewing . public @ playcontrol.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/


 /**
 * @mainpage
 * ALmixer (which I sometimes call "SDL-OpenAL-Mixer" or "SDL_ALmixer") is a cross-platform audio library built 
 * on top of OpenAL to make playing and managing sounds easier. 
 * ALmixer provides a simple API inspired by SDL_mixer to make playing sounds easy 
 * with having to worry about directly dealing with OpenAL sources, buffers, 
 * and buffer queuing directly.
 * ALmixer currently utilizes SDL_sound behind the scenes to decode 
 * various audio formats such as WAV, MP3, AAC, MP4, OGG, etc.
 *
 * This library is targeted towards two major groups:
 * - People who just want an easy, high performance, way to play audio (don't care if its OpenAL or not)
 * - People who want to an easy way to play audio in OpenAL but still want access to OpenAL directly.
 *  
 * ALmixer exposes OpenAL sources in the API so you can freely use ALmixer 
 * in larger OpenAL applications that need to apply OpenAL 3D effects and features 
 * to playing sounds.
 *
 * The API is heavily influenced and inspired by SDL_mixer, though there is one major
 * conceptual design difference. ALmixer doesn't divide sound and music playback into two
 * separate play APIs. Instead, there is one unified play API and you specify via the 
 * load API whether you want the audio resource loaded as a stream or completely preloaded.
 * This allows you to have any arbitrary number of streaming sources playing simultaneously
 * (such as music and speech) unlike SDL_mixer where you are limited to only one "music" 
 * channel.
 *
 * A less major conceptual design difference is every "Channel" API has a corresponding "Source" API.
 * Every "channel" (in the SDL_mixer definition context) maps to a corresponding OpenAL source id. You can use
 * this source ID directly with OpenAL API commands to utilize OpenAL effects such as position, Doppler, etc.
 * Convenience APIs are provided to let you convert channel numbers to source ids and vice-versa.
 *
 * Another change which is a pet-peev of mine with SDL_mixer is the lack of a user_data parameter in callbacks.
 * ALmixer callbacks allow you to pass user_data (aka context) pointers through the callback functions.
 *
 * @note There are some #defines you can set to change the behavior at compile time. Most you shouldn't touch.
 * The one worth noting is ENABLE_ALMIXER_THREADS. If enabled, ALmixer_Update() is automatically called on a 
 * background thread so you no longer have to explicitly call it. (The function turns into a no-op so your existing
 * code won't break.) Having Update run in a separate thread has some advantages, particularly for streaming
 * audio as all the OpenAL buffer queuing happens in this function. It is less likely the background thread will
 * be blocked for long periods and thus less likely your buffer queues will be starved. However, this means you 
 * need to be extra careful about what you do in callback functions as they are invoked from the background thread.
 * I still consider this feature a experimental (though I am starting to use it more myself) and there
 * may still be bugs.
 *
 * @author Eric Wing
 *
 * Home Page: http://playcontrol.net/opensource/ALmixer
 */

/**
 * @file
 * ALmixer (which I sometimes call "SDL-OpenAL-Mixer" or "SDL_ALmixer") is a cross-platform audio library built 
 * on top of OpenAL to make playing and managing sounds easier. 
 * ALmixer provides a simple API inspired by SDL_mixer to make playing sounds easy 
 * with having to worry about directly dealing with OpenAL sources, buffers, 
 * and buffer queuing directly.
 * ALmixer currently utilizes SDL_sound behind the scenes to decode 
 * various audio formats such as WAV, MP3, AAC, MP4, OGG, etc.
 *
 * This library is targeted towards two major groups:
 * - People who just want an easy, high performance, way to play audio (don't care if its OpenAL or not)
 * - People who want to an easy way to play audio in OpenAL but still want access to OpenAL directly.
 *  
 * ALmixer exposes OpenAL sources in the API so you can freely use ALmixer 
 * in larger OpenAL applications that need to apply OpenAL 3D effects and features 
 * to playing sounds.
 *
 * The API is heavily influenced and inspired by SDL_mixer, though there is one major
 * conceptual design difference. ALmixer doesn't divide sound and music playback into two
 * separate play APIs. Instead, there is one unified play API and you specify via the 
 * load API whether you want the audio resource loaded as a stream or completely preloaded.
 * This allows you to have any arbitrary number of streaming sources playing simultaneously
 * (such as music and speech) unlike SDL_mixer where you are limited to only one "music" 
 * channel.
 *
 * A less major conceptual design difference is every "Channel" API has a corresponding "Source" API.
 * Every "channel" (in the SDL_mixer definition context) maps to a corresponding OpenAL source id. You can use
 * this source ID directly with OpenAL API commands to utilize OpenAL effects such as position, Doppler, etc.
 * Convenience APIs are provided to let you convert channel numbers to source ids and vice-versa.
 *
 * Another change which is a pet-peev of mine with SDL_mixer is the lack of a user_data parameter in callbacks.
 * ALmixer callbacks allow you to pass user_data (aka context) pointers through the callback functions.
 *
 * @note There are some #defines you can set to change the behavior at compile time. Most you shouldn't touch.
 * The one worth noting is ENABLE_ALMIXER_THREADS. If enabled, ALmixer_Update() is automatically called on a 
 * background thread so you no longer have to explicitly call it. (The function turns into a no-op so your existing
 * code won't break.) Having Update run in a separate thread has some advantages, particularly for streaming
 * audio as all the OpenAL buffer queuing happens in this function. It is less likely the background thread will
 * be blocked for long periods and thus less likely your buffer queues will be starved. However, this means you 
 * need to be extra careful about what you do in callback functions as they are invoked from the background thread.
 * I still consider this feature a experimental (though I am starting to use it more myself) and there
 * may still be bugs.
 *
 * @author Eric Wing
 *
 * Home Page: http://playcontrol.net/opensource/ALmixer
 */


#ifndef _SDL_ALMIXER_H_
#define _SDL_ALMIXER_H_


#ifndef DOXYGEN_SHOULD_IGNORE_THIS
/** @cond DOXYGEN_SHOULD_IGNORE_THIS */

/* Note: For Doxygen to produce clean output, you should set the 
 * PREDEFINED option to remove ALMIXER_DECLSPEC, ALMIXER_CALL, and
 * the DOXYGEN_SHOULD_IGNORE_THIS blocks.
 * PREDEFINED = DOXYGEN_SHOULD_IGNORE_THIS=1 ALMIXER_DECLSPEC= ALMIXER_CALL=
 */

#ifdef ALMIXER_COMPILE_WITHOUT_SDL
	#if defined(_WIN32) || defined(WINAPI_FAMILY)
		#if defined(ALMIXER_BUILD_LIBRARY)
			#define ALMIXER_DECLSPEC __declspec(dllexport)
		#else
			#define ALMIXER_DECLSPEC
		#endif
	#else
		#if defined(ALMIXER_BUILD_LIBRARY)
			#if defined (__GNUC__) && __GNUC__ >= 4
				#define ALMIXER_DECLSPEC __attribute__((visibility("default")))
			#else
				#define ALMIXER_DECLSPEC
			#endif
		#else
			#define ALMIXER_DECLSPEC
		#endif
	#endif

	#if defined(_WIN32) || defined(WINAPI_FAMILY)
		#define ALMIXER_CALL __cdecl
	#else
		#define ALMIXER_CALL
	#endif
#else
	#include "SDL_types.h" /* will include begin_code.h which is what I really want */
	#define ALMIXER_DECLSPEC DECLSPEC
	#define ALMIXER_CALL SDLCALL
#endif

/** @endcond DOXYGEN_SHOULD_IGNORE_THIS */
#endif /* DOXYGEN_SHOULD_IGNORE_THIS */



/* Needed for OpenAL types since altypes.h was removed in 1.1 */
#include "al.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef ALMIXER_COMPILE_WITHOUT_SDL
	/**
	 * Struct that contains the version information of this library.
	 * This represents the library's version as three levels: major revision
	 * (increments with massive changes, additions, and enhancements),
	 * minor revision (increments with backwards-compatible changes to the
	 * major revision), and patchlevel (increments with fixes to the minor
	 * revision).
	 * @see ALMIXER_VERSION, ALmixer_GetLinkedVersion
	 */
	typedef struct ALmixer_version
	{
		ALubyte major;
		ALubyte minor;
		ALubyte patch;
	} ALmixer_version;
#else
	#include "SDL_version.h"
	#define ALmixer_version SDL_version
#endif

/* Printable format: "%d.%d.%d", MAJOR, MINOR, PATCHLEVEL
 */
#define ALMIXER_MAJOR_VERSION		0
#define ALMIXER_MINOR_VERSION		2
#define ALMIXER_PATCHLEVEL			0


/** 
 * @defgroup CoreOperation Initialization, Tear-down, and Core Operational Commands
 * @{
 * Functions for setting up and using ALmixer.
 */
 
 
/**
 * This macro fills in a version structure with the version of the
 * library you compiled against. This is determined by what header the
 * compiler uses. Note that if you dynamically linked the library, you might
 * have a slightly newer or older version at runtime. That version can be
 * determined with ALmixer_GetLinkedVersion(), which, unlike 
 * ALMIXER_GET_COMPILED_VERSION, is not a macro.
 *
 * @note When compiled with SDL, this macro can be used to fill a version structure 
 * compatible with SDL_version.
 *
 * @param X A pointer to a ALmixer_version struct to initialize.
 *
 * @see ALmixer_version, ALmixer_GetLinkedVersion
 */
#define ALMIXER_GET_COMPILED_VERSION(X)                                           \
	{                                                                       \
		(X)->major = ALMIXER_MAJOR_VERSION;                          \
		(X)->minor = ALMIXER_MINOR_VERSION;                          \
		(X)->patch = ALMIXER_PATCHLEVEL;                             \
	}

/**
 * Gets the library version of the dynamically linked ALmixer you are using.
 * This gets the version of ALmixer that is linked against your program.
 * If you are using a shared library (DLL) version of ALmixer, then it is
 * possible that it will be different than the version you compiled against.
 *
 * This is a real function; the macro ALMIXER_GET_COMPILED_VERSION 
 * tells you what version of tErrorLib you compiled against:
 *
 * @code
 * ALmixer_version compiled;
 * ALmixer_version linked;
 *
 * ALMIXER_GET_COMPILED_VERSION(&compiled);
 * ALmixer_GetLinkedVersion(&linked);
 * printf("We compiled against tError version %d.%d.%d ...\n",
 *           compiled.major, compiled.minor, compiled.patch);
 * printf("But we linked against tError version %d.%d.%d.\n",
 *           linked.major, linked.minor, linked.patch);
 * @endcode
 *
 * @see ALmixer_version, ALMIXER_GET_COMPILED_VERSION
 */
extern ALMIXER_DECLSPEC const ALmixer_version* ALMIXER_CALL ALmixer_GetLinkedVersion(void);

#ifdef ALMIXER_COMPILE_WITHOUT_SDL
	/**
	 * Gets the last error string that was set by the system and clears the error.
	 *
	 * @note When compiled with SDL, this directly uses SDL_GetError.
	 * 
	 * @return Returns a string containing the last error or "" when no error is set.
	 */
	extern ALMIXER_DECLSPEC const char* ALMIXER_CALL ALmixer_GetError(void);
	/**
	 * Sets an error string that can be retrieved by ALmixer_GetError.
	 *
	 * @note When compiled with SDL, this directly uses SDL_SetError.
	 * 
	 * param The error string to set.
	 */
	extern ALMIXER_DECLSPEC void ALMIXER_CALL ALmixer_SetError(const char *fmt, ...);
#else
	#include "SDL_error.h"
	/**
	 * Gets the last error string that was set by the system and clears the error.
	 *
	 * @note When compiled with SDL, this directly uses SDL_GetError.
	 * 
	 * @return Returns a string containing the last error or "" when no error is set.
	 */
	#define ALmixer_GetError 	SDL_GetError
	/**
	 * Sets an error string that can be retrieved by ALmixer_GetError.
	 *
	 * @note When compiled with SDL, this directly uses SDL_SetError.
	 * 
	 * param The error string to set.
	 */
	#define ALmixer_SetError 	SDL_SetError
#endif


#ifdef ALMIXER_COMPILE_WITHOUT_SDL
	#include "ALmixer_RWops.h"
#else
	#include "SDL_rwops.h"
	/**
	 * A struct that mimicks the SDL_RWops structure.
	 *
	 * @note When compiled with SDL, this directly uses SDL_RWops.
	 */
	#define ALmixer_RWops 	SDL_RWops
#endif


#define ALMIXER_DEFAULT_FREQUENCY 	0
#define ALMIXER_DEFAULT_REFRESH 	0
#define ALMIXER_DEFAULT_NUM_CHANNELS	16
#define ALMIXER_DEFAULT_NUM_SOURCES		ALMIXER_DEFAULT_NUM_CHANNELS

/** 
 * This is the recommended Init function. This will initialize the context, SDL_sound,
 * and the mixer system. You should call this in the setup of your code, after SDL_Init.
 * If you attempt to bypass this function, you do so at your own risk.
 *
 * @note ALmixer expects the SDL audio subsystem to be disabled. In some cases, an enabled
 * SDL audio subsystem will interfere and cause problems in your app. This Init method explicitly
 * disables the SDL subsystem if SDL is compiled in. 
 *
 * @note The maximum number of sources is OpenAL implementation dependent.
 * Currently 16 is lowest common denominator for all OpenAL implementations in current use.
 * 32 is currently the second lowest common denominator.
 * If you try to allocate more sources than are actually available, this function may return false depending
 * if the OpenAL implementation returns an error or not. It is possible for OpenAL to silently fail
 * so be very careful about picking too many sources.
 *
 * @param playback_frequency The sample rate you want OpenAL to play at, e.g. 44100
 * Note that OpenAL is not required to actually respect this value.
 * Pass in 0 or ALMIXER_DEFAULT_FREQUENCY to specify you want to use your implementation's default value.
 * @param num_sources The number of OpenAL sources (also can be thought of as 
 * SDL_mixer channels) you wish to allocate.
 * Pass in 0 or ALMIXER_DEFAULT_NUM_SOURCES to use ALmixer's default value.
 * @param refresh_rate The refresh rate you want OpenAL to operate at. 
 * Note that OpenAL is not required to respect this value.
 * Pass in 0 or ALMIXER_DEFAULT_REFRESH to use OpenAL default behaviors.
 * @return Returns AL_FALSE on a failure or AL_TRUE if successfully initialized.
 */
extern ALMIXER_DECLSPEC ALboolean ALMIXER_CALL ALmixer_Init(ALuint playback_frequency, ALuint num_sources, ALuint refresh_rate);

/** 
 * InitContext will only initialize the OpenAL context (and not the mixer part).
 * Note that SDL_Sound is also initialized here because load order matters
 * because SDL audio will conflict with OpenAL when using SMPEG. This is only 
 * provided as a backdoor and is not recommended.
 *
 * @note This is a backdoor in case you need to initialize the AL context and 
 * the mixer system separately. I strongly recommend avoiding these two functions
 * and use the normal Init() function.
 */
extern ALMIXER_DECLSPEC ALboolean ALMIXER_CALL ALmixer_InitContext(ALuint playback_frequency, ALuint refresh_rate);

/** 
 * InitMixer will only initialize the Mixer system. This is provided in the case 
 * that you need control over the loading of the context. You may load the context 
 * yourself, and then call this function. This is not recommended practice, but is 
 * provided as a backdoor in case you have good reason to 
 * do this. Be warned that if ALmixer_InitMixer() fails,
 * it will not clean up the AL context. Also be warned that Quit() still does try to 
 * clean up everything.
 *
 * @note This is a backdoor in case you need to initialize the AL context and 
 * the mixer system separately. I strongly recommend avoiding these two functions
 * and use the normal Init() function.
 */
extern ALMIXER_DECLSPEC ALboolean ALMIXER_CALL ALmixer_InitMixer(ALuint num_sources);

/**
 * (EXPERIMENTAL) Call to notify ALmixer that your device needs to handle an interruption.
 * (EXPERIMENTAL) For devices like iOS that need special handling for interruption events like phone calls and alarms,
 * this function will do the correct platform correct thing to handle the interruption w.r.t. OpenAL. This calls ALmixer_SuspendUpdates().
 */
extern ALMIXER_DECLSPEC void ALMIXER_CALL ALmixer_BeginInterruption(void);

/**
 * (EXPERIMENTAL) Call to notify ALmixer that your device needs to resume from an interruption.
 * (EXPERIMENTAL) For devices like iOS that need special handling for interruption events like phone calls and alarms,
 * this function will do the correct platform correct thing to resume from the interruption w.r.t. OpenAL. This calls ALmixer_ResumeUpdates().
 */
extern ALMIXER_DECLSPEC void ALMIXER_CALL ALmixer_EndInterruption(void);

/**
 * (EXPERIMENTAL) Call to determine if in an interruption.
 * (EXPERIMENTAL) For devices like iOS that need special handling for interruption events like phone calls and alarms,
 * this function will do the correct platform correct thing to determine if in an interruption.
 */	
extern ALMIXER_DECLSPEC ALboolean ALmixer_IsInInterruption(void);


/**
 * (EXPERIMENTAL) Destroys the background update thread (ENABLE_ALMIXER_THREADS only). 
 * (EXPERIMENTAL) Destroys the background update thread (ENABLE_ALMIXER_THREADS only). BeginInterruption used to do this internally, but this was split off due to an iOS OpenAL race condition bug (10081775). Being able to manipulate the thread without manipulating the context was useful for suspend/resume backgrounding when not dealing with a full-blown interruption event.
 */
extern ALMIXER_DECLSPEC void ALMIXER_CALL ALmixer_SuspendUpdates(void);

/**
 * (EXPERIMENTAL) Recreates the background update thread (ENABLE_ALMIXER_THREADS only). 
 * (EXPERIMENTAL) Recreates the background update thread (ENABLE_ALMIXER_THREADS only). EndInterruption used to do this internally, but this was split off due to an iOS OpenAL race condition bug (10081775). Being able to manipulate the thread without manipulating the context was useful for suspend/resume backgrounding when not dealing with a full-blown interruption event.
 */
extern ALMIXER_DECLSPEC void ALMIXER_CALL ALmixer_ResumeUpdates(void);

/**
 * (EXPERIMENTAL) Call to determine if in ALmixer_SuspendUpdates(). (ENABLE_ALMIXER_THREADS only.)
 */
extern ALMIXER_DECLSPEC ALboolean ALmixer_AreUpdatesSuspended(void);

	
/**
 * This shuts down ALmixer. Please remember to free your ALmixer_Data* instances
 * before calling this method.
 */
extern ALMIXER_DECLSPEC void ALMIXER_CALL ALmixer_Quit(void);
/**
 * Returns whether ALmixer has been initializatized (via Init) or not.
 * @return Returns true for initialized and false for not initialized.
 */
extern ALMIXER_DECLSPEC ALboolean ALMIXER_CALL ALmixer_IsInitialized(void);

/**
 * Returns the frequency that OpenAL is set to.
 * @note This function is not guaranteed to give correct information and is OpenAL implementation dependent.
 * @return Returns the frequency, e.g. 44100.
 */
extern ALMIXER_DECLSPEC ALuint ALMIXER_CALL ALmixer_GetFrequency(void);

/**
 * Let's you change the maximum number of channels/sources available.
 * This function is not heavily tested. It is probably better to simply initialize
 * ALmixer with the number of sources you want when you initialize it instead of 
 * dynamically changing it later.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_AllocateChannels(ALint num_chans);

/**
 * Allows you to reserve a certain number of channels so they won't be automatically
 * allocated to play on.
 * This function will effectively block off a certain number of channels so they won't
 * be automatically assigned to be played on when you call various play functions
 * (applies to both play-channel and play-source functions since they are the same under the hood).
 * The lowest number channels will always be blocked off first.
 * For example, if there are 16 channels available, and you pass 2 into this function,
 * channels 0 and 1 will be reserved so they won't be played on automatically when you specify
 * you want to play a sound on any available channel/source. You can 
 * still play on channels 0 and 1 if you explicitly designate you want to play on their channel
 * number or source id.
 * Setting back to 0 will clear all the reserved channels so all will be available again for 
 * auto-assignment.
 * As an example, this feature can be useful if you always want your music to be on channel 0 and
 * speech on channel 1 and you don't want sound effects to ever occupy those channels. This allows
 * you to build in certain assumptions about your code, perhaps for deciding which data you want
 * to analyze in a data callback.
 * Specifying the number of reserve channels to the maximum number of channels will effectively
 * disable auto-assignment.
 * @param number_of_reserve_channels The number of channels/sources to reserve.
 * Or pass -1 to find out how many channels are currently reserved.
 * @return Returns the number of currently reserved channels.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_ReserveChannels(ALint number_of_reserve_channels);


/**
 * The update function that allows ALmixer to update its internal state.
 * If not compiled with/using threads, this function must be periodically called
 * to poll ALmixer to force streamed music and other events to
 * take place. 
 * The typical place to put this function is in your main-loop. 
 * If threads are enabled, then this function just
 * returns 0 and is effectively a no-op. With threads, it is not necessary to call this function
 * because updates are handled internally on another thread. However, because threads are still considered
 * experimental, it is recommended you call this function in a proper place in your code in case
 * future versions of this library need to abandon threads.
 * @return Returns 0 if using threads. If not using threads, for debugging purposes, it returns
 * the number of buffers queued during the loop, or a negative value indicating the numer of errors encountered.
 * This is subject to change and should not be relied on.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_Update(void);

/**
 * @}
 */
 
/** 
 * @defgroup LoadAPI Load Audio Functions
 * @{
 * Functions for loading and unloading audio data.
 */



/*
#define ALmixer_AudioInfo 	Sound_AudioInfo
*/

/*
#define ALMIXER_DEFAULT_BUFFERSIZE 32768
#define ALMIXER_DEFAULT_BUFFERSIZE 16384 
*/
#define ALMIXER_DEFAULT_BUFFERSIZE 8192

/* You probably never need to use these macros directly. */
#ifndef ALMIXER_DISABLE_PREDECODED_PRECOMPUTE_BUFFER_SIZE_OPTIMIZATION
	#define ALMIXER_DEFAULT_PREDECODED_BUFFERSIZE ALMIXER_DEFAULT_BUFFERSIZE * 4
#else
	/* I'm picking a smaller buffer because ALmixer will try to create a new larger buffer
	 * based on the length of the audio. So creating a large block up-front might just be a waste.
	 * However, if my attempts fail for some reason, this buffer size becomes a fallback.
	 * Having too small of a buffer might cause performance bottlenecks.
	 */
	#define ALMIXER_DEFAULT_PREDECODED_BUFFERSIZE 4096
#endif

/**
 * Specifies the maximum number of queue buffers to use for a sound stream.
 * Default Queue Buffers must be at least 2.
 */
/*
#define ALMIXER_DEFAULT_QUEUE_BUFFERS 5
*/
#define ALMIXER_DEFAULT_QUEUE_BUFFERS 12
/**
 * Specifies the number of queue buffers initially filled when first loading a stream.
 * Default startup buffers should be at least 1. */
#define ALMIXER_DEFAULT_STARTUP_BUFFERS 4
/*
#define ALMIXER_DEFAULT_STARTUP_BUFFERS 2 
*/
#define ALMIXER_DEFAULT_BUFFERS_TO_QUEUE_PER_UPDATE_PASS 2

/*
#define ALMIXER_DECODE_STREAM 	0
#define ALMIXER_DECODE_ALL 		1
*/

/* This is a trick I picked up from Lua. Doing the typedef separately 
* (and I guess before the definition) instead of a single 
* entry: typedef struct {...} YourName; seems to allow me
* to use forward declarations. Doing it the other way (like SDL)
* seems to prevent me from using forward declarions as I get conflicting
* definition errors. I don't really understand why though.
*/
typedef struct ALmixer_Data ALmixer_Data;
typedef struct ALmixer_AudioInfo ALmixer_AudioInfo;

/**
 * Roughly the equvialent to the Sound_AudioInfo struct in SDL_sound.
 * Types have been changed to use AL types because I know those are available.
 * This is different than SDL which uses fixed types so there might be subtle
 * things you need to pay attention to..
 * @note Originally, I just used the Sound_AudioInfo directly, but
 * I've been trying to reduce the header dependencies for this file.
 * But more to the point, I've been interested in dealing with the 
 * WinMain override problem Josh faced when trying to use SDL components
 * in an MFC app which didn't like losing control of WinMain. 
 * My theory is that if I can purge the header of any thing that 
 * #include's SDL_main.h, then this might work.
 * So I am now introducing my own AudioInfo struct.
 */
struct ALmixer_AudioInfo
{
	ALushort format;  /**< Equivalent of SDL_AudioSpec.format. */
	ALubyte channels; /**< Number of sound channels. 1 == mono, 2 == stereo. */
	ALuint rate;    /**< Sample rate; frequency of sample points per second. */
};



/**
 * This is a general loader function to load an audio resource from an RWops.
 * Generally, you should use the LoadStream and LoadAll specializations of this function instead which call this.
 * @param rw_ops The rwops pointing to the audio resource you want to load.
 * @param file_ext The file extension of your audio type which is used as a hint by the backend to decide which
 * decoder to use.
 * @param buffer_size The size of a buffer to allocate for read chunks. This number should be in quantized with 
 * the valid frame sizes of your audio data. If the data is streamed, the data will be read in buffer_size chunks.
 * If the file is to be predecoded, optimizations may occur and this value might be ignored.
 * @param decode_mode_is_predecoded Specifies whether you want to completely preload the data or stream the data in chunks.
 * @param max_queue_buffers For streamed data, specifies the maximum number of buffers that can be queued at any given time.
 * @param num_startup_buffers For streamed data, specifies the number of buffers to fill before playback starts.
 * Buffer underrun conditions will also attempt to restart using this value.
 * @param suggested_number_of_buffers_to_queue_per_update_pass For each ALmixer_Update() pass, this is the targeted number 
 * of buffers that will be queued.
 * @param access_data A boolean that specifies if you want the data contained in the currently playing buffer to be handed
 * to you in a callback function. Note that for predecoded data, you get back the entire buffer in one callback when the 
 * audio first starts playing. With streamed data, you get the data in buffer_size chunks. Callbacks are not guarnanteed
 * to be perfectly in-sync as this is a best-effort implementaiton. There are memory and performance considerations for 
 * using this feature, so if you don't need data callbacks, you should pass false to this function.
 * @return Returns an ALmixer_Data* of the loaded sample or NULL if failed.
 */
extern ALMIXER_DECLSPEC ALmixer_Data* ALMIXER_CALL ALmixer_LoadSample_RW(ALmixer_RWops* rw_ops, const char* file_ext, ALuint buffer_size, ALboolean decode_mode_is_predecoded, ALuint max_queue_buffers, ALuint num_startup_buffers, ALuint suggested_number_of_buffers_to_queue_per_update_pass, ALuint access_data);

#ifdef DOXYGEN_ONLY
/**
 * This is the loader function to load an audio resource from an RWops as a stream.
 * @param rw_ops The rwops pointing to the audio resource you want to load.
 * @param file_ext The file extension of your audio type which is used as a hint by the backend to decide which
 * decoder to use.
 * @param buffer_size The size of a buffer to allocate for read chunks. This number should be in quantized with 
 * the valid frame sizes of your audio data. If the data is streamed, the data will be read in buffer_size chunks.
 * @param max_queue_buffers For streamed data, specifies the maximum number of buffers that can be queued at any given time.
 * @param num_startup_buffers For streamed data, specifies the number of buffers to fill before playback starts.
 * Buffer underrun conditions will also attempt to restart using this value. 
 * @param suggested_number_of_buffers_to_queue_per_update_pass For each ALmixer_Update() pass, this is the targeted number
 * of buffers that will be queued. 
 * @param access_data A boolean that specifies if you want the data contained in the currently playing buffer to be handed
 * to you in a callback function. Note that for predecoded data, you get back the entire buffer in one callback when the 
 * audio first starts playing. With streamed data, you get the data in buffer_size chunks. Callbacks are not guarnanteed
 * to be perfectly in-sync as this is a best-effort implementaiton. There are memory and performance considerations for 
 * using this feature, so if you don't need data callbacks, you should pass false to this function.
 * @return Returns an ALmixer_Data* of the loaded sample or NULL if failed.
 */
ALmixer_Data* ALmixer_LoadStream_RW(ALmixer_RWops* rw_ops, const char* file_ext, ALuint buffer_size, ALuint max_queue_buffers, ALuint num_startup_buffers, ALuint suggested_number_of_buffers_to_queue_per_update_pass, ALuint access_data);
#else
#define ALmixer_LoadStream_RW(rw_ops, file_ext, buffer_size, max_queue_buffers, num_startup_buffers, suggested_number_of_buffers_to_queue_per_update_pass, access_data) ALmixer_LoadSample_RW(rw_ops,file_ext, buffer_size, AL_FALSE, max_queue_buffers, num_startup_buffers, suggested_number_of_buffers_to_queue_per_update_pass, access_data)
#endif

#ifdef DOXYGEN_ONLY
/**
 * This is the loader function to completely preload an audio resource from an RWops into RAM.
 * @param rw_ops The rwops pointing to the audio resource you want to load.
 * @param file_ext The file extension of your audio type which is used as a hint by the backend to decide which
 * decoder to use.
 * @param access_data A boolean that specifies if you want the data contained in the currently playing buffer to be handed
 * to you in a callback function. Note that for predecoded data, you get back the entire buffer in one callback when the 
 * audio first starts playing. With streamed data, you get the data in buffer_size chunks. Callbacks are not guarnanteed
 * to be perfectly in-sync as this is a best-effort implementaiton. There are memory and performance considerations for 
 * using this feature, so if you don't need data callbacks, you should pass false to this function.
 * @return Returns an ALmixer_Data* of the loaded sample or NULL if failed.
 */
ALmixer_Data* ALmixer_LoadAll_RW(ALmixer_RWops* rw_ops, const char* file_ext, ALuint access_data);
#else
#define ALmixer_LoadAll_RW(rw_ops, file_ext, access_data) ALmixer_LoadSample_RW(rw_ops, fileext, ALMIXER_DEFAULT_PREDECODED_BUFFERSIZE, AL_TRUE, 0, 0, 0, access_data)
#endif

/**
 * This is a general loader function to load an audio resource from a file.
 * Generally, you should use the LoadStream and LoadAll specializations of this function instead which call this.
 * @param file_name The file of the audio resource you want to load.
 * @param buffer_size The size of a buffer to allocate for read chunks. This number should be in quantized with 
 * the valid frame sizes of your audio data. If the data is streamed, the data will be read in buffer_size chunks.
 * If the file is to be predecoded, optimizations may occur and this value might be ignored.
 * @param decode_mode_is_predecoded Specifies whether you want to completely preload the data or stream the data in chunks.
 * @param max_queue_buffers For streamed data, specifies the maximum number of buffers that can be queued at any given time.
 * @param num_startup_buffers For streamed data, specifies the number of buffers to fill before playback starts.
 * Buffer underrun conditions will also attempt to restart using this value. 
 * @param suggested_number_of_buffers_to_queue_per_update_pass For each ALmixer_Update() pass, this is the targeted number
 * of buffers that will be queued. 
 * @param access_data A boolean that specifies if you want the data contained in the currently playing buffer to be handed
 * to you in a callback function. Note that for predecoded data, you get back the entire buffer in one callback when the 
 * audio first starts playing. With streamed data, you get the data in buffer_size chunks. Callbacks are not guarnanteed
 * to be perfectly in-sync as this is a best-effort implementaiton. There are memory and performance considerations for 
 * using this feature, so if you don't need data callbacks, you should pass false to this function.
 * @return Returns an ALmixer_Data* of the loaded sample or NULL if failed.
 */
extern ALMIXER_DECLSPEC ALmixer_Data * ALMIXER_CALL ALmixer_LoadSample(const char* file_name, ALuint buffer_size, ALboolean decode_mode_is_predecoded, ALuint max_queue_buffers, ALuint num_startup_buffers, ALuint suggested_number_of_buffers_to_queue_per_update_pass, ALuint access_data);

#ifdef DOXYGEN_ONLY
/**
 * This is the loader function to load an audio resource from a file.
 * @param file_name The file to the audio resource you want to load.
 * @param buffer_size The size of a buffer to allocate for read chunks. This number should be in quantized with 
 * the valid frame sizes of your audio data. If the data is streamed, the data will be read in buffer_size chunks.
 * @param max_queue_buffers For streamed data, specifies the maximum number of buffers that can be queued at any given time.
 * @param num_startup_buffers For streamed data, specifies the number of buffers to fill before playback starts.
 * Buffer underrun conditions will also attempt to restart using this value. 
 * @param suggested_number_of_buffers_to_queue_per_update_pass For each ALmixer_Update() pass, this is the targeted number
 * of buffers that will be queued.  * @param num_startup_buffers For streamed data, specifies the number of buffers to fill before playback starts.
 * Buffer underrun conditions will also attempt to restart using this value. 
 * @param suggested_number_of_buffers_to_queue_per_update_pass For each ALmixer_Update() pass, this is the targeted number
 * of buffers that will be queued. 
 * @param access_data A boolean that specifies if you want the data contained in the currently playing buffer to be handed
 * to you in a callback function. Note that for predecoded data, you get back the entire buffer in one callback when the 
 * audio first starts playing. With streamed data, you get the data in buffer_size chunks. Callbacks are not guarnanteed
 * to be perfectly in-sync as this is a best-effort implementaiton. There are memory and performance considerations for 
 * using this feature, so if you don't need data callbacks, you should pass false to this function.
 * @return Returns an ALmixer_Data* of the loaded sample or NULL if failed.
 */
ALmixer_Data* ALmixer_LoadStream(const char* file_name, ALuint buffer_size, ALuint max_queue_buffers, ALuint num_startup_buffers, ALuint suggested_number_of_buffers_to_queue_per_update_pass, ALuint access_data);
#else
#define ALmixer_LoadStream(file_name, buffer_size, max_queue_buffers, num_startup_buffers, suggested_number_of_buffers_to_queue_per_update_pass, access_data) ALmixer_LoadSample(file_name, buffer_size, AL_FALSE, max_queue_buffers, num_startup_buffers, suggested_number_of_buffers_to_queue_per_update_pass, access_data)
#endif

#ifdef DOXYGEN_ONLY
/**
 * This is the loader function to completely preload an audio resource from a file into RAM.
 * @param file_name The file to the audio resource you want to load.
 * @param access_data A boolean that specifies if you want the data contained in the currently playing buffer to be handed
 * to you in a callback function. Note that for predecoded data, you get back the entire buffer in one callback when the 
 * audio first starts playing. With streamed data, you get the data in buffer_size chunks. Callbacks are not guarnanteed
 * to be perfectly in-sync as this is a best-effort implementaiton. There are memory and performance considerations for 
 * using this feature, so if you don't need data callbacks, you should pass false to this function.
 * @return Returns an ALmixer_Data* of the loaded sample or NULL if failed.
 */
ALmixer_Data* ALmixer_LoadAll(const char* file_name, ALuint access_data);
#else
#define ALmixer_LoadAll(file_name, access_data) ALmixer_LoadSample(file_name, ALMIXER_DEFAULT_PREDECODED_BUFFERSIZE, AL_TRUE, 0, 0, 0, access_data)
#endif

/**
 * This is a back door general loader function for RAW samples or if you need to specify the ALmixer_AudioInfo field.
 * Use at your own risk.
 * Generally, you should use the LoadStream and LoadAll specializations of this function instead which call this.
 * @param rw_ops The rwops pointing to the audio resource you want to load.
 * @param file_ext The file extension of your audio type which is used as a hint by the backend to decide which
 * decoder to use. Pass "raw" for raw formats.
 * @param desired_format The format you want audio decoded to. NULL will pick a default for you.
 * @param buffer_size The size of a buffer to allocate for read chunks. This number should be in quantized with 
 * the valid frame sizes of your audio data. If the data is streamed, the data will be read in buffer_size chunks.
 * If the file is to be predecoded, optimizations may occur and this value might be ignored.
 * @param decode_mode_is_predecoded Specifies whether you want to completely preload the data or stream the data in chunks.
 * @param max_queue_buffers For streamed data, specifies the maximum number of buffers that can be queued at any given time.
 * @param num_startup_buffers For streamed data, specifies the number of buffers to fill before playback starts.
 * Buffer underrun conditions will also attempt to restart using this value. 
 * @param suggested_number_of_buffers_to_queue_per_update_pass For each ALmixer_Update() pass, this is the targeted number
 * of buffers that will be queued. 
 * @param access_data A boolean that specifies if you want the data contained in the currently playing buffer to be handed
 * to you in a callback function. Note that for predecoded data, you get back the entire buffer in one callback when the 
 * audio first starts playing. With streamed data, you get the data in buffer_size chunks. Callbacks are not guarnanteed
 * to be perfectly in-sync as this is a best-effort implementaiton. There are memory and performance considerations for 
 * using this feature, so if you don't need data callbacks, you should pass false to this function.
 * @return Returns an ALmixer_Data* of the loaded sample or NULL if failed.
 */
extern ALMIXER_DECLSPEC ALmixer_Data * ALMIXER_CALL ALmixer_LoadSample_RAW_RW(ALmixer_RWops* rw_ops, const char* file_ext, ALmixer_AudioInfo* desired_format, ALuint buffer_size, ALboolean decode_mode_is_predecoded, ALuint max_queue_buffers, ALuint num_startup_buffers, ALuint suggested_number_of_buffers_to_queue_per_update_pass, ALuint access_data);

#ifdef DOXYGEN_ONLY
/**
 * This is a back door stream loader function for RAW samples or if you need to specify the ALmixer_AudioInfo field.
 * Use at your own risk.
 * @param rw_ops The rwops pointing to the audio resource you want to load.
 * @param file_ext The file extension of your audio type which is used as a hint by the backend to decide which
 * decoder to use. Pass "raw" for raw formats.
 * @param desired_format The format you want audio decoded to. NULL will pick a default for you.
 * @param buffer_size The size of a buffer to allocate for read chunks. This number should be in quantized with 
 * the valid frame sizes of your audio data. If the data is streamed, the data will be read in buffer_size chunks.
 * If the file is to be predecoded, optimizations may occur and this value might be ignored.
 * @param max_queue_buffers For streamed data, specifies the maximum number of buffers that can be queued at any given time.
 * @param num_startup_buffers For streamed data, specifies the number of buffers to fill before playback starts.
 * Buffer underrun conditions will also attempt to restart using this value. 
 * @param suggested_number_of_buffers_to_queue_per_update_pass For each ALmixer_Update() pass, this is the targeted number
 * of buffers that will be queued. 
 * @param access_data A boolean that specifies if you want the data contained in the currently playing buffer to be handed
 * to you in a callback function. Note that for predecoded data, you get back the entire buffer in one callback when the 
 * audio first starts playing. With streamed data, you get the data in buffer_size chunks. Callbacks are not guarnanteed
 * to be perfectly in-sync as this is a best-effort implementaiton. There are memory and performance considerations for 
 * using this feature, so if you don't need data callbacks, you should pass false to this function.
 * @return Returns an ALmixer_Data* of the loaded sample or NULL if failed.
 */
ALmixer_Data* ALmixer_LoadStream_RAW_RW(ALmixer_RWops* rw_ops, const char* file_ext, ALmixer_AudioInfo* desired_format, ALuint buffer_size, ALuint max_queue_buffers, ALuint num_startup_buffers, ALuint suggested_number_of_buffers_to_queue_per_update_pass, ALuint access_data);
#else
#define ALmixer_LoadStream_RAW_RW(rw_ops, file_ext, desired_format, buffer_size, max_queue_buffers, num_startup_buffers, suggested_number_of_buffers_to_queue_per_update_pass, access_data) ALmixer_LoadSample_RAW_RW(rw_ops, file_ext, desired_format, buffer_size, AL_FALSE, max_queue_buffers, num_startup_buffers, suggested_number_of_buffers_to_queue_per_update_pass, access_data)
#endif

#ifdef DOXYGEN_ONLY
/**
 * This is a back door loader function for complete preloading RAW samples into RAM or if you need to specify the ALmixer_AudioInfo field.
 * Use at your own risk.
 * @param rw_ops The rwops pointing to the audio resource you want to load.
 * @param file_ext The file extension of your audio type which is used as a hint by the backend to decide which
 * decoder to use. Pass "raw" for raw formats.
 * @param desired_format The format you want audio decoded to. NULL will pick a default for you.
 * @param access_data A boolean that specifies if you want the data contained in the currently playing buffer to be handed
 * to you in a callback function. Note that for predecoded data, you get back the entire buffer in one callback when the 
 * audio first starts playing. With streamed data, you get the data in buffer_size chunks. Callbacks are not guarnanteed
 * to be perfectly in-sync as this is a best-effort implementaiton. There are memory and performance considerations for 
 * using this feature, so if you don't need data callbacks, you should pass false to this function.
 * @return Returns an ALmixer_Data* of the loaded sample or NULL if failed.
 */
ALmixer_Data* ALmixer_LoadAll_RAW_RW(ALmixer_RWops* rw_ops, const char* file_ext, ALmixer_AudioInfo* desired_format, ALuint access_data);
#else
#define ALmixer_LoadAll_RAW_RW(rw_ops, file_ext, desired_format, access_data) ALmixer_LoadSample_RAW_RW(rw_ops, file_ext, desired_format, ALMIXER_DEFAULT_PREDECODED_BUFFERSIZE, AL_TRUE, 0, 0, 0, access_data)
#endif

/**
 * This is a back door general loader function for RAW samples or if you need to specify the ALmixer_AudioInfo field.
 * Use at your own risk.
 * Generally, you should use the LoadStream and LoadAll specializations of this function instead which call this.
 * @param file_name The file to the audio resource you want to load. Extension should be "raw" for raw formats.
 * @param desired_format The format you want audio decoded to. NULL will pick a default for you.
 * @param buffer_size The size of a buffer to allocate for read chunks. This number should be in quantized with 
 * the valid frame sizes of your audio data. If the data is streamed, the data will be read in buffer_size chunks.
 * If the file is to be predecoded, optimizations may occur and this value might be ignored.
 * @param decode_mode_is_predecoded Specifies whether you want to completely preload the data or stream the data in chunks.
 * @param max_queue_buffers For streamed data, specifies the maximum number of buffers that can be queued at any given time.
 * @param num_startup_buffers For streamed data, specifies the number of buffers to fill before playback starts.
 * Buffer underrun conditions will also attempt to restart using this value. 
 * @param suggested_number_of_buffers_to_queue_per_update_pass For each ALmixer_Update() pass, this is the targeted number
 * of buffers that will be queued. 
 * @param access_data A boolean that specifies if you want the data contained in the currently playing buffer to be handed
 * to you in a callback function. Note that for predecoded data, you get back the entire buffer in one callback when the 
 * audio first starts playing. With streamed data, you get the data in buffer_size chunks. Callbacks are not guarnanteed
 * to be perfectly in-sync as this is a best-effort implementaiton. There are memory and performance considerations for 
 * using this feature, so if you don't need data callbacks, you should pass false to this function.
 * @return Returns an ALmixer_Data* of the loaded sample or NULL if failed.
 */
extern ALMIXER_DECLSPEC ALmixer_Data * ALMIXER_CALL ALmixer_LoadSample_RAW(const char* file_name, ALmixer_AudioInfo* desired_format, ALuint buffer_size, ALboolean decode_mode_is_predecoded, ALuint max_queue_buffers, ALuint num_startup_buffers, ALuint suggested_number_of_buffers_to_queue_per_update_pass, ALuint access_data);

#ifdef DOXYGEN_ONLY
/**
 * This is a back door stream loader function for RAW samples or if you need to specify the ALmixer_AudioInfo field.
 * Use at your own risk.
 * @param file_name The file to the audio resource you want to load.Extension should be "raw" for raw formats.
 * @param desired_format The format you want audio decoded to. NULL will pick a default for you.
 * @param buffer_size The size of a buffer to allocate for read chunks. This number should be in quantized with 
 * the valid frame sizes of your audio data. If the data is streamed, the data will be read in buffer_size chunks.
 * If the file is to be predecoded, optimizations may occur and this value might be ignored.
 * @param max_queue_buffers For streamed data, specifies the maximum number of buffers that can be queued at any given time.
 * @param num_startup_buffers For streamed data, specifies the number of buffers to fill before playback starts.
 * Buffer underrun conditions will also attempt to restart using this value. 
 * @param suggested_number_of_buffers_to_queue_per_update_pass For each ALmixer_Update() pass, this is the targeted number
 * of buffers that will be queued. 
 * @param access_data A boolean that specifies if you want the data contained in the currently playing buffer to be handed
 * to you in a callback function. Note that for predecoded data, you get back the entire buffer in one callback when the 
 * audio first starts playing. With streamed data, you get the data in buffer_size chunks. Callbacks are not guarnanteed
 * to be perfectly in-sync as this is a best-effort implementaiton. There are memory and performance considerations for 
 * using this feature, so if you don't need data callbacks, you should pass false to this function.
 * @return Returns an ALmixer_Data* of the loaded sample or NULL if failed.
 */
ALmixer_Data* ALmixer_LoadStream_RAW(const char* file_name, ALmixer_AudioInfo* desired_format, ALuint buffer_size, ALuint max_queue_buffers, ALuint num_startup_buffers, ALuint suggested_number_of_buffers_to_queue_per_update_pass, ALuint access_data);
#else
#define ALmixer_LoadStream_RAW(file_name, desired_format, buffer_size, max_queue_buffers, num_startup_buffers, suggested_number_of_buffers_to_queue_per_update_pass, access_data) ALmixer_LoadSample_RAW(file_name, desired_format, buffer_size, AL_FALSE, max_queue_buffers, num_startup_buffers, suggested_number_of_buffers_to_queue_per_update_pass, access_data)
#endif

#ifdef DOXYGEN_ONLY
/**
 * This is a back door loader function for complete preloading RAW samples into RAM or if you need to specify the ALmixer_AudioInfo field.
 * Use at your own risk.
 * @param file_name The file to the audio resource you want to load. Extension should be "raw" for raw formats.
 * @param desired_format The format you want audio decoded to. NULL will pick a default for you.
 * @param access_data A boolean that specifies if you want the data contained in the currently playing buffer to be handed
 * to you in a callback function. Note that for predecoded data, you get back the entire buffer in one callback when the 
 * audio first starts playing. With streamed data, you get the data in buffer_size chunks. Callbacks are not guarnanteed
 * to be perfectly in-sync as this is a best-effort implementaiton. There are memory and performance considerations for 
 * using this feature, so if you don't need data callbacks, you should pass false to this function.
 * @return Returns an ALmixer_Data* of the loaded sample or NULL if failed.
 */
ALmixer_Data* ALmixer_LoadAll_RAW(const char* file_name, ALmixer_AudioInfo* desired_format, ALuint access_data);
#else
#define ALmixer_LoadAll_RAW(file_name, desired_format, access_data) ALmixer_LoadSample_RAW(file_name, desired_format, ALMIXER_DEFAULT_PREDECODED_BUFFERSIZE, AL_TRUE, 0, 0, 0, access_data)
#endif

/**
 * Frees an ALmixer_Data.
 * Releases the memory associated with a ALmixer_Data. Use this when you are done playing the audio sample
 * and wish to release the memory.
 * @warning Do not try releasing data that is currently in use (e.g. playing, paused).
 * @warning Make sure to free your data before calling ALmixer_Quit. Do not free data aftter ALmixer_Quit().
 * @param almixer_data The ALmixer_Data* you want to free.
 */
extern ALMIXER_DECLSPEC void ALMIXER_CALL ALmixer_FreeData(ALmixer_Data* almixer_data);


/**
 * Returns true if the almixer_data was completely loaded into memory or false if it was loaded
 * as a stream.
 * @param almixer_data The audio resource you want to know about.
 * @return AL_TRUE is predecoded, or AL_FALSE if streamed.
 */
extern ALMIXER_DECLSPEC ALboolean ALMIXER_CALL ALmixer_IsPredecoded(ALmixer_Data* almixer_data);

/**
 * @}
 */
 
/** 
 * @defgroup CallbackAPI Callbacks
 * @{
 * Functions for callbacks
 */

/**
 * Allows you to set a callback for when a sound has finished playing on a channel/source.
 * @param playback_finished_callback The function you want to be invoked when a sound finishes.
 * The callback function will pass you back the channel number which just finished playing,
 * the OpenAL source id associated with the channel, the ALmixer_Data* that was played,
 * a boolean telling you whether a sound finished playing because it ended normally or because
 * something interrupted the playback (such as the user calling ALmixer_Halt*), and the
 * user_data supplied as the second parameter to this function.
 * @param which_chan The ALmixer channel that the data is currently playing on.
 * @param al_source The OpenAL source that the data is currently playing on.
 * @param almixer_data The ALmixer_Data that was played.
 * @param finished_naturally AL_TRUE if the sound finished playing because it ended normally 
 * or AL_FALSE because something interrupted playback (such as the user calling ALmixer_Halt*).
 * @param user_data This will be passed back to you in the callback.
 *
 * @warning You should not call other ALmixer functions in this callback. 
 * Particularly in the case of when compiled with threads, recursive locking
 * will occur which will lead to deadlocks. Also be aware that particularly in the 
 * threaded case, the callbacks may (and currently do) occur on a background thread.
 * One typical thread safe strategy is to set flags or schedule events to occur on the
 * main thread.
 * One possible exception to the no-calling ALmixer functions rule is ALmixer_Free. ALmixer_Free
 * currently does not lock so it might okay to call this to free your data. However, this is not
 * tested and not the expected pattern to be used.
 */
extern ALMIXER_DECLSPEC void ALMIXER_CALL ALmixer_SetPlaybackFinishedCallback(void (*playback_finished_callback)(ALint which_channel, ALuint al_source, ALmixer_Data* almixer_data, ALboolean finished_naturally, void* user_data), void* user_data);

/**
 * Allows you to set a callback for getting audio data.
 * This is a callback function pointer that when set, will trigger a function
 * anytime there is new data loaded for a sample. The appropriate load 
 * parameter must be set in order for a sample to appear here.
 * Keep in mind the the current backend implementation must do an end run
 * around OpenAL because OpenAL lacks support for this kind of thing.
 * As such, buffers are copied at decode time, and there is no attempt to do
 * fine grained timing syncronization. You will be provided the entire buffer
 * that is decoded regardless of length. So if you predecoded the entire 
 * audio file, the entire data buffer will be provided in a single callback.
 * If you stream the data, you will be getting chunk sizes that are the same as
 * what you specified the decode size to be. Unfortunely, this means if you 
 * pick smaller buffers, you get finer detail at the expense/risk of buffer 
 * underruns. If you decode more data, you have to deal with the syncronization
 * issues if you want to display the data during playback in something like an
 * oscilloscope.
 *
 * @warning You should not call other ALmixer functions in this callback. 
 * Particularly in the case of when compiled with threads, recursive locking
 * will occur which will lead to deadlocks. Also be aware that particularly in the 
 * threaded case, the callbacks may (and currently do) occur on a background thread.
 * One typical thread safe strategy is to set flags or schedule events to occur on the
 * main thread.
 * 
 * @param playback_data_callback The function you want called back.
 * @param which_channel The ALmixer channel that the data is currently playing on.
 * @param al_source The OpenAL source that the data is currently playing on.
 * @param pcm_data This is a pointer to the data buffer containing ALmixer's 
 * version of the decoded data. Consider this data as read-only. In the 
 * non-threaded backend, this data will persist until potentially the next call
 * to Update(). Currently, data buffers are preallocated and not destroyed
 * until FreeData() is called (though this behavior is subject to change),
 * but the contents will change when the buffer needs to be reused for a 
 * future callback. The buffer reuse is tied to the amount of buffers that
 * may be queued.
 * But assuming I don't change this, this may allow for some optimization
 * so you can try referencing data from these buffers without worrying 
 * about crashing. (You still need to be aware that the data could be 
 * modified behind the scenes on an Update().)
 * The data type listed is an signed 8-bit format, but the real data may
 * not actually be this. ALbyte was chosen as a convenience. If you have 
 * a 16 bit format, you will want to cast the data and divide the num_bytes by 2.
 * Typically, data is either Sint16. This seems to be a 
 * convention audio people seem to follow though I'm not sure what the 
 * underlying reasons (if any) are for this. I suspect that there may be 
 * some nice alignment/conversion property if you need to cast between ALbyte
 * and ALubyte.
 * 
 * @param num_bytes This is the total length of the data buffer. It presumes
 * that this length is measured for ALbyte. So if you have Sint16 data, you
 * should divide num_bytes by two if you access the data as Sint16.
 * 
 * @param frequency The frequency the data was decoded at.
 *
 * @param num_channels_in_sample 1 for mono, 2 for stereo. Not to be confused with the ALmixer which_channel.
 *
 * @param bit_depth Bits per sample. This is expected to be 8 or 16. This 
 * number will tell you if you if you need to treat the data buffer as 
 * 16 bit or not.
 * 
 * @param is_unsigned 1 if the data is unsigned, 0 if signed. Using this
 * combined with bit_depth will tell you if you need to treat the data
 * as ALubyte, ALbyte, ALuint, or ALint.
 *
 * @param decode_mode_is_predecoded This is here to tell you if the data was totally 
 * predecoded or loaded as a stream. If predecoded, you will only get 
 * one data callback per playback instance. (This might also be true for 
 * looping the same sample...I don't remember how it was implemented. 
 * Maybe this should be fixed.)
 * 0 (ALMIXER_DECODE_STREAM) for streamed.
 * 1 (ALMIXER_DECODE_ALL) for predecoded.
 *
 * @param length_in_msec This returns the total length (time) of the data 
 * buffer in milliseconds. This could be computed yourself, but is provided
 * as a convenince.
 *
 * @param user_data The user data you pass in will be passed back to you in the callback. 
 */
extern ALMIXER_DECLSPEC void ALMIXER_CALL ALmixer_SetPlaybackDataCallback(void (*playback_data_callback)(ALint which_channel, ALuint al_source, ALbyte* pcm_data, ALuint num_bytes, ALuint frequency, ALubyte num_channels_in_sample, ALubyte bit_depth, ALboolean is_unsigned, ALboolean decode_mode_is_predecoded, ALuint length_in_msec, void* user_data), void* user_data);

/**
 * @}
 */
 
 /** 
 * @defgroup PlayAPI Functions useful for playback.
 * @{
 * These are core functions that are useful for controlling playback.
 * Also see the Volume functions for additional playback functions and Query functions for additional information.
 */

/**
 * Returns the total time in milliseconds of the audio resource.
 * Returns the total time in milliseconds of the audio resource.
 * If the total length cannot be determined, -1 will be returned.
 * @param almixer_data The audio sample you want to know the total time of.
 * @return The total time in milliseconds or -1 if some kind of failure.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_GetTotalTime(ALmixer_Data* almixer_data);

/** 
 * This function will look up the OpenAL source id for the corresponding channel number.
 * @param which_channel The channel which you want to find the corresponding OpenAL source id for.
 * If -1 was specified, an available source for playback will be returned.
 * @return The OpenAL source id corresponding to the channel. 0 if you specified an illegal channel value.
 * Or 0 if you specified -1 and no sources were currently available.
 * @note ALmixer assumes your OpenAL implementation does not use 0 as a valid source ID. While the OpenAL spec
 * does not disallow 0 for valid source ids, as of now, there are no known OpenAL implementations in use that 
 * use 0 as a valid source id (partly due to problems this has caused developers in the past).
 */
extern ALMIXER_DECLSPEC ALuint ALMIXER_CALL ALmixer_GetSource(ALint which_channel);

/**
 * This function will look up the channel for the corresponding source.
 * @param al_source The source id you want to find the corresponding channel number for.
 * If 0 is supplied, it will try to return the first channel not in use. 
 * Returns -1 on error, or the channel.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_GetChannel(ALuint al_source);

/**
 * Will look for a channel available for playback.
 * Given a start channel number, the search will increase to the highest channel until it finds one available.
 * @param start_channel The channel number you want to start looking at.
 * @return A channel available or -1 if none could be found.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_FindFreeChannel(ALint start_channel);



/**
 * Play a sound on a channel with a time limit.
 * Plays a sound on a channel and will auto-stop after a specified number of milliseconds.
 * @param which_channel Allows you to specify the specific channel you want to play on. 
 * Channels range from 0 to the (Number of allocated channels - 1). If you specify -1, 
 * an available channel will be chosen automatically for you.
 * @note While paused, the auto-stop clock will also be paused. This makes it easy to always stop
 * a sample by a predesignated length without worrying about whether the user paused playback which would 
 * throw off your calculations.
 * @param almixer_data The audio resource you want to play.
 * @param number_of_loops The number of times to loop (repeat) playing the data. 
 * 0 means the data will play exactly once without repeat. -1 means infinitely loop.
 * @param number_of_milliseconds The number of milliseconds that should be played until the sample is auto-stopped.
 * -1 means don't auto-stop playing and let the sample finish playing normally (or if looping is set to infinite, 
 * the sample will never stop playing).
 * @return Returns the channel that was selected for playback or -1 if no channels were available.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_PlayChannelTimed(ALint which_channel, ALmixer_Data* almixer_data, ALint number_of_loops, ALint number_of_milliseconds);

#ifdef DOXYGEN_ONLY
/**
 * The same as ALmixer_PlayChannelTimed, but the sound is played without time limits.
 * @see ALmixer_PlayChannelTimed.
 */ 
ALint ALmixer_PlayChannelTimed(ALint which_channel, ALmixer_Data* almixer_data, ALint number_of_loops);
#else
#define ALmixer_PlayChannel(channel,data,loops) ALmixer_PlayChannelTimed(channel,data,loops,-1)
#endif


/**
 * Play a sound on an OpenAL source with a time limit.
 * Plays a sound on an OpenAL source and will auto-stop after a specified number of milliseconds.
 * @param al_source Allows you to specify the OpenAL source you want to play on. 
 * If you specify 0, an available source will be chosen automatically for you.
 * @note Source values are not necessarily continguous and their values are implementation dependent.
 * Always use ALmixer functions to determine source values.
 * @note While paused, the auto-stop clock will also be paused. This makes it easy to always stop
 * a sample by a predesignated length without worrying about whether the user paused playback which would 
 * throw off your calculations.
 * @param almixer_data The audio resource you want to play.
 * @param number_of_loops The number of times to loop (repeat) playing the data. 
 * 0 means the data will play exactly once without repeat. -1 means infinitely loop.
 * @param number_of_milliseconds The number of milliseconds that should be played until the sample is auto-stopped.
 * -1 means don't auto-stop playing and let the sample finish playing normally (or if looping is set to infinite, 
 * the sample will never stop playing).
 * @return Returns the OpenAL source that was selected for playback or 0 if no sources were available.
 */
extern ALMIXER_DECLSPEC ALuint ALMIXER_CALL ALmixer_PlaySourceTimed(ALuint al_source, ALmixer_Data* almixer_data, ALint number_of_loops, ALint number_of_milliseconds);

#ifdef DOXYGEN_ONLY
/**
 * The same as ALmixer_PlaySourceTimed, but the sound is played without time limits.
 * @see ALmixer_PlaySourceTimed.
 */ 
ALint ALmixer_PlayChannelTimed(ALuint al_source, ALmixer_Data* almixer_data, ALint number_of_loops);
#else
#define ALmixer_PlaySource(al_source, almixer_data, number_of_loops) ALmixer_PlaySourceTimed(al_source, almixer_data, number_of_loops, -1)
#endif

/**
 * Stops playback on a channel.
 * Stops playback on a channel and clears the channel so it can be played on again.
 * @note Callbacks will still be invoked, but the finished_naturally flag will be set to AL_FALSE.
 * @param which_channel The channel to halt or -1 to halt all channels.
 * @return The actual number of channels halted on success or -1 on error.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_HaltChannel(ALint which_channel);

/**
 * Stops playback on a channel.
 * Stops playback on a channel and clears the channel so it can be played on again.
 * @note Callbacks will still be invoked, but the finished_naturally flag will be set to AL_FALSE.
 * @param al_source The source to halt or 0 to halt all sources.
 * @return The actual number of sources halted on success or -1 on error.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_HaltSource(ALuint al_source);

/**
 * Rewinds the sound to the beginning for a given data.
 * Rewinds the actual data, but the effect
 * may not be noticed until the currently buffered data is played.
 * @param almixer_data The data to rewind.
 * @returns true on success or false on error.
 */
extern ALMIXER_DECLSPEC ALboolean ALMIXER_CALL ALmixer_RewindData(ALmixer_Data* almixer_data);

/**
 * Rewinds the sound to the beginning that is playing on a specific channel.
 * If decoded all, rewind will instantly rewind it. Data is not 
 * affected, so it will start at the "Seek"'ed positiond.
 * Streamed data will rewind the actual data, but the effect
 * may not be noticed until the currently buffered data is played.
 * @param which_channel The channel to rewind or -1 to rewind all channels.
 * @return The actual number of channels rewound on success or -1 on error.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_RewindChannel(ALint which_channel);
/**
 * Rewinds the sound to the beginning that is playing on a specific source.
 * If decoded all, rewind will instantly rewind it. Data is not 
 * affected, so it will start at the "Seek"'ed positiond.
 * Streamed data will rewind the actual data, but the effect
 * may not be noticed until the currently buffered data is played.
 * @param al_source The source to rewind or 0 to rewind all sources.
 * @return The actual number of sources rewound on success or -1 on error.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_RewindSource(ALuint al_source);

/**
 * Seek the sound for a given data.
 * Seeks the actual data to the given millisecond. It
 * may not be noticed until the currently buffered data is played.
 * @param almixer_data The data to seek on.
 * @param msec_pos The time position to seek to in the audio in milliseconds.
 * @returns true on success or false on error.
 */
extern ALMIXER_DECLSPEC ALboolean ALMIXER_CALL ALmixer_SeekData(ALmixer_Data* almixer_data, ALuint msec_pos);

/**
 * Seeks the sound to the beginning that is playing on a specific channel.
 * If decoded all, seek will instantly seek it. Data is not 
 * affected, so it will start at the "Seek"'ed positiond.
 * Streamed data will seek the actual data, but the effect
 * may not be noticed until the currently buffered data is played.
 * @param which_channel The channel to seek or -1 to seek all channels.
 * @return The actual number of channels rewound on success or -1 on error.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_SeekChannel(ALint which_channel, ALuint msec_pos);
/**
 * Seeks the sound to the beginning that is playing on a specific source.
 * If decoded all, seek will instantly seek it. Data is not 
 * affected, so it will start at the "Seek"'ed positiond.
 * Streamed data will seek the actual data, but the effect
 * may not be noticed until the currently buffered data is played.
 * @param al_source The source to seek or 0 to seek all sources.
 * @return The actual number of sources rewound on success or -1 on error.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_SeekSource(ALuint al_source, ALuint msec_pos);

/**
 * Pauses playback on a channel.
 * Pauses playback on a channel. Should have no effect on channels that aren't playing.
 * @param which_channel The channel to pause or -1 to pause all channels.
 * @return The actual number of channels paused on success or -1 on error.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_PauseChannel(ALint which_channel);
/**
 * Pauses playback on a source.
 * Pauses playback on a source. Should have no effect on source that aren't playing.
 * @param al_source The source to pause or -1 to pause all source.
 * @return The actual number of source paused on success or -1 on error.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_PauseSource(ALuint al_source);

/**
 * Resumes playback on a channel that is paused.
 * Resumes playback on a channel that is paused. Should have no effect on channels that aren't paused.
 * @param which_channel The channel to resume or -1 to resume all channels.
 * @return The actual number of channels resumed on success or -1 on error.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_ResumeChannel(ALint which_channel);

/**
 * Resumes playback on a source that is paused.
 * Resumes playback on a source that is paused. Should have no effect on sources that aren't paused.
 * @param al_source The source to resume or -1 to resume all sources.
 * @return The actual number of sources resumed on success or -1 on error.
 */
 extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_ResumeSource(ALuint al_source);

 
/**
 * Will cause a currently playing channel to stop playing in the specified number of milliseconds.
 * Will cause a currently playing channel to stop playing in the specified number of milliseconds.
 * This will override the value that was set when PlayChannelTimed or PlaySourceTimed was called
 * or override any previous calls to ExpireChannel or ExpireSource.
 * @param which_channel The channel to expire or -1 to apply to all channels.
 * @param number_of_milliseconds How many milliseconds from now until the expire triggers.
 * @return The actual number of channels this action is applied to on success or -1 on error.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_ExpireChannel(ALint which_channel, ALint number_of_milliseconds);
/**
 * Will cause a currently playing source to stop playing in the specified number of milliseconds.
 * Will cause a currently playing source to stop playing in the specified number of milliseconds.
 * This will override the value that was set when PlayChannelTimed or PlaySourceTimed was called
 * or override any previous calls to ExpireChannel or ExpireSource.
 * @param al_source The source to expire or 0 to apply to all sources.
 * @param number_of_milliseconds How many milliseconds from now until the expire triggers.
 * @return The actual number of sources this action is applied to on success or -1 on error.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_ExpireSource(ALuint al_source, ALint number_of_milliseconds);

/**
 * @}
 */

/** 
 * @defgroup VolumeAPI Volume and Fading
 * @{
 * Fade and volume functions directly call OpenAL functions related to AL_GAIN.
 * These functions are provided mostly for those who just want to play audio but are not planning
 * to use OpenAL features directly.
 * If you are using OpenAL directly (e.g. for 3D effects), you may want to be careful about using these as
 * they may fight/override values you directly set yourself.
 */

/**
 * Similar to ALmixer_PlayChannelTimed except that sound volume fades in from the minimum volume to the current AL_GAIN over the specified amount of time.
 * @see ALmixer_PlayChannelTimed.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_FadeInChannelTimed(ALint which_channel, ALmixer_Data* almixer_data, ALint number_of_loops, ALuint fade_ticks, ALint expire_ticks);

#ifdef DOXYGEN_ONLY
/**
 * The same as ALmixer_FadeInChannelTimed, but the sound is played without time limits.
 * @see ALmixer_FadeInChannelTimed, ALmixer_PlayChannel.
 */ 
ALint ALmixer_FadeInChannel(ALint which_channel, ALmixer_Data* almixer_data, ALint number_of_loops, ALuint fade_ticks);
#else
#define ALmixer_FadeInChannel(which_channel, almixer_data, number_of_loops, fade_ticks) ALmixer_FadeInChannelTimed(which_channel, almixer_data, number_of_loops, fade_ticks, -1)
#endif

/**
 * Similar to ALmixer_PlaySourceTimed except that sound volume fades in from the minimum volume to the max volume over the specified amount of time.
 * @see ALmixer_PlaySourceTimed.
 */
extern ALMIXER_DECLSPEC ALuint ALMIXER_CALL ALmixer_FadeInSourceTimed(ALuint al_source, ALmixer_Data* almixer_data, ALint number_of_loops, ALuint fade_ticks, ALint expire_ticks);

#ifdef DOXYGEN_ONLY
/**
 * The same as ALmixer_FadeInSourceTimed, but the sound is played without time limits.
 * @see ALmixer_FadeInSourceTimed, ALmixer_PlaySource.
 */ 
extern ALuint ALmixer_FadeInSource(ALuint al_source, ALmixer_Data* almixer_data, ALint number_of_loops, ALuint fade_ticks);
#else
#define ALmixer_FadeInSource(al_source, almixer_data, number_of_loops, fade_ticks) ALmixer_FadeInSourceTimed(al_source, almixer_data, number_of_loops, fade_ticks, -1)
#endif

/**
 * Fade out a current playing channel.
 * Will fade out a currently playing channel over the specified period of time starting from now. 
 * The volume will be changed from the current AL_GAIN level to the AL_MIN_GAIN. 
 * The volume fade will interpolate over the specified period of time. 
 * The playback will halt at the end of the time period.
 * @param which_channel The channel to fade or -1 to fade all playing channels.
 * @param fade_ticks In milliseconds, the amount of time the fade out should take to complete.
 * @return Returns -1 on error or the number of channels faded.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_FadeOutChannel(ALint which_channel, ALuint fade_ticks);

/**
 * Fade out a current playing source.
 * Will fade out a currently playing source over the specified period of time starting from now. 
 * The volume will be changed from the current AL_GAIN level to the AL_MIN_GAIN. 
 * The volume fade will interpolate over the specified period of time. 
 * The playback will halt at the end of the time period.
 * @param al_source The source to fade or -1 to fade all playing sources.
 * @param fade_ticks In milliseconds, the amount of time the fade out should take to complete.
 * @return Returns -1 on error or the number of sources faded.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_FadeOutSource(ALuint al_source, ALuint fade_ticks);

/**
 * Gradually changes the volume from the current AL_GAIN to the specified volume.
 * Gradually changes the volume from the current AL_GAIN to the specified volume over the specified period of time.
 * This is some times referred to as volume ducking. 
 * Note that this function works for setting the volume higher as well as lower.
 * @param which_channel The channel to fade or -1 to fade all playing channels.
 * @param fade_ticks In milliseconds, the amount of time the volume change should take to complete.
 * @param volume The volume to change to. Valid values are 0.0 to 1.0.
 * @return Returns -1 on error or the number of channels faded.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_FadeChannel(ALint which_channel, ALuint fade_ticks, ALfloat volume);

/**
 * Gradually changes the volume from the current AL_GAIN to the specified volume.
 * Gradually changes the volume from the current AL_GAIN to the specified volume over the specified period of time.
 * This is some times referred to as volume ducking. 
 * Note that this function works for setting the volume higher as well as lower.
 * @param al_source The source to fade or -1 to fade all playing sources.
 * @param fade_ticks In milliseconds, the amount of time the volume change should take to complete.
 * @param volume The volume to change to. Valid values are 0.0 to 1.0.
 * @return Returns -1 on error or the number of sources faded.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_FadeSource(ALuint al_source, ALuint fade_ticks, ALfloat volume);

/**
 * Sets the volume via the AL_GAIN source property.
 * Sets the volume for a given channel via the AL_GAIN source property.
 * @param which_channel The channel to set the volume to or -1 to set on all channels.
 * @param volume The new volume to use. Valid values are 0.0 to 1.0.
 * @return AL_TRUE on success or AL_FALSE on error.
 */
extern ALMIXER_DECLSPEC ALboolean ALMIXER_CALL ALmixer_SetVolumeChannel(ALint which_channel, ALfloat volume);

/**
 * Sets the volume via the AL_GAIN source property.
 * Sets the volume for a given source via the AL_GAIN source property.
 * @param al_source The source to set the volume to or 0 to set on all sources.
 * @param volume The new volume to use. Valid values are 0.0 to 1.0.
 * @return AL_TRUE on success or AL_FALSE on error.
 */
extern ALMIXER_DECLSPEC ALboolean ALMIXER_CALL ALmixer_SetVolumeSource(ALuint al_source, ALfloat volume);

/**
 * Gets the volume via the AL_GAIN source property.
 * Gets the volume for a given channel via the AL_GAIN source property.
 * @param which_channel The channel to get the volume from.
 * -1 will return the average volume set across all channels.
 * @return Returns the volume for the specified channel, or the average set volume for all channels, or -1.0 on error.
 */
extern ALMIXER_DECLSPEC ALfloat ALMIXER_CALL ALmixer_GetVolumeChannel(ALint which_channel);

/**
 * Gets the volume via the AL_GAIN source property.
 * Gets the volume for a given source via the AL_GAIN source property.
 * @param al_source The source to get the volume from.
 * -1 will return the average volume set across all source.
 * @return Returns the volume for the specified source, or the average set volume for all sources, or -1.0 on error.
 */
extern ALMIXER_DECLSPEC ALfloat ALMIXER_CALL ALmixer_GetVolumeSource(ALuint al_source);

/**
 * Sets the maximum volume via the AL_MAX_GAIN source property.
 * Sets the maximum volume for a given channel via the AL_MAX_GAIN source property.
 * Max volumes will be clamped to this value.
 * @param which_channel The channel to set the volume to or -1 to set on all channels.
 * @param volume The new volume to use. Valid values are 0.0 to 1.0.
 * @return AL_TRUE on success or AL_FALSE on error.
 */
extern ALMIXER_DECLSPEC ALboolean ALMIXER_CALL ALmixer_SetMaxVolumeChannel(ALint which_channel, ALfloat volume);

/**
 * Sets the maximum volume via the AL_MAX_GAIN source property.
 * Sets the maximum volume for a given source via the AL_MAX_GAIN source property.
 * @param al_source The source to set the volume to or 0 to set on all sources.
 * @param volume The new volume to use. Valid values are 0.0 to 1.0.
 * @return AL_TRUE on success or AL_FALSE on error.
 */
extern ALMIXER_DECLSPEC ALboolean ALMIXER_CALL ALmixer_SetMaxVolumeSource(ALuint al_source, ALfloat volume);

/**
 * Gets the max volume via the AL_MAX_GAIN source property.
 * Gets the max volume for a given channel via the AL_MAX_GAIN source property.
 * @param which_channel The channel to get the volume from.
 * -1 will return the average volume set across all channels.
 * @return Returns the volume for the specified channel, or the average set volume for all channels, or -1.0 on error.
 */
extern ALMIXER_DECLSPEC ALfloat ALMIXER_CALL ALmixer_GetMaxVolumeChannel(ALint which_channel);

/**
 * Gets the maximum volume via the AL_MAX_GAIN source property.
 * Gets the maximum volume for a given source via the AL_MAX_GAIN source property.
 * @param al_source The source to set the volume to or 0 to set on all sources.
 * 0 will return the average volume set across all channels.
 * @return Returns the volume for the specified channel, or the average set volume for all channels, or -1.0 on error.
 */
extern ALMIXER_DECLSPEC ALfloat ALMIXER_CALL ALmixer_GetMaxVolumeSource(ALuint al_source);

/**
 * Sets the minimum volume via the AL_MIN_GAIN source property.
 * Sets the minimum volume for a given channel via the AL_MIN_GAIN source property.
 * Min volumes will be clamped to this value.
 * @param which_channel The channel to set the volume to or -1 to set on all channels.
 * @param volume The new volume to use. Valid values are 0.0 to 1.0.
 * @return AL_TRUE on success or AL_FALSE on error.
 */
extern ALMIXER_DECLSPEC ALboolean ALMIXER_CALL ALmixer_SetMinVolumeChannel(ALint which_channel, ALfloat volume);

/**
 * Sets the minimum volume via the AL_MIN_GAIN source property.
 * Sets the minimum volume for a given source via the AL_MIN_GAIN source property.
 * @param al_source The source to set the volume to or 0 to set on all sources.
 * @param volume The new volume to use. Valid values are 0.0 to 1.0.
 * @return AL_TRUE on success or AL_FALSE on error.
 */
extern ALMIXER_DECLSPEC ALboolean ALMIXER_CALL ALmixer_SetMinVolumeSource(ALuint al_source, ALfloat volume);

/**
 * Gets the min volume via the AL_MIN_GAIN source property.
 * Gets the min volume for a given channel via the AL_MIN_GAIN source property.
 * @param which_channel The channel to get the volume from.
 * -1 will return the average volume set across all channels.
 * @return Returns the volume for the specified channel, or the average set volume for all channels, or -1.0 on error.
 */
extern ALMIXER_DECLSPEC ALfloat ALMIXER_CALL ALmixer_GetMinVolumeChannel(ALint which_channel);

/**
 * Gets the min volume via the AL_MIN_GAIN source property.
 * Gets the min volume for a given source via the AL_MIN_GAIN source property.
 * @param al_source The source to set the volume to or 0 to set on all sources.
 * 0 will return the average volume set across all channels.
 * @return Returns the volume for the specified channel, or the average set volume for all channels, or -1.0 on error.
 */
extern ALMIXER_DECLSPEC ALfloat ALMIXER_CALL ALmixer_GetMinVolumeSource(ALuint al_source);

/**
 * Sets the OpenAL listener AL_GAIN which can be thought of as the "master volume".
 * Sets the OpenAL listener AL_GAIN which can be thought of as the "master volume".
 * @param new_volume The new volume level to be set. Range is 0.0 to 1.0 where 1.0 is the max volume.
 * @return AL_TRUE on success or AL_FALSE on an error.
 */
extern ALMIXER_DECLSPEC ALboolean ALMIXER_CALL ALmixer_SetMasterVolume(ALfloat new_volume);

/**
 * Gets the OpenAL listener AL_GAIN which can be thought of as the "master volume".
 * Gets the OpenAL listener AL_GAIN which can be thought of as the "master volume".
 * @return The current volume level on the listener. -1.0 will be returned on an error.
 */
 extern ALMIXER_DECLSPEC ALfloat ALMIXER_CALL ALmixer_GetMasterVolume(void);

/**
 * @}
 */
 
/**
 * @defgroup QueryAPI Query APIs
 * @{
 * Functions to query ALmixer.
 */
 

/**
 * Returns true if the specified channel is currently playing or paused, 
 * or if -1 is passed the number of channels that are currently playing or paused.
 * @param which_channel The channel you want to know about or -1 to get the count of
 * currently playing/paused channels.
 * @return For a specific channel, 1 if the channel is playing or paused, 0 if not.
 * Or returns the count of currently playing/paused channels.
 * Or -1 on an error.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_IsActiveChannel(ALint which_channel);

/**
 * Returns true if the specified source is currently playing or paused,
 * or if -1 is passed the number of sources that are currently playing or paused.
 * @param al_source The channel you want to know about or -1 to get the count of
 * currently playing/paused sources.
 * @return For a specific sources, 1 if the channel is playing or paused, 0 if not.
 * Or returns the count of currently playing/paused sources.
 * Or -1 on an error.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_IsActiveSource(ALuint al_source);

/**
 * Returns true if the specified channel is currently playing. 
 * or if -1 is passed the number of channels that are currently playing.
 * @param which_channel The channel you want to know about or -1 to get the count of
 * currently playing channels.
 * @return For a specific channel, 1 if the channel is playing, 0 if not.
 * Or returns the count of currently playing channels.
 * Or -1 on an error.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_IsPlayingChannel(ALint which_channel);

/**
 * Returns true if the specified sources is currently playing. 
 * or if -1 is passed the number of sources that are currently playing.
 * @param al_source The sources you want to know about or -1 to get the count of
 * currently playing sources.
 * @return For a specific source, 1 if the source is playing, 0 if not.
 * Or returns the count of currently playing sources.
 * Or -1 on an error.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_IsPlayingSource(ALuint al_source);

/**
 * Returns true if the specified channel is currently paused. 
 * or if -1 is passed the number of channels that are currently paused.
 * @param which_channel The channel you want to know about or -1 to get the count of
 * currently paused channels.
 * @return For a specific channel, 1 if the channel is paused, 0 if not.
 * Or returns the count of currently paused channels.
 * Or -1 on an error.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_IsPausedChannel(ALint which_channel);

/**
 * Returns true if the specified sources is currently paused. 
 * or if -1 is passed the number of sources that are currently paused.
 * @param al_source The source you want to know about or -1 to get the count of
 * currently paused sources.
 * @return For a specific source, 1 if the source is paused, 0 if not.
 * Or returns the count of currently paused sources.
 * Or -1 on an error.
 */
extern ALMIXER_DECLSPEC ALint ALMIXER_CALL ALmixer_IsPausedSource(ALuint al_source);

/**
 * Returns the number of channels that are currently available for playback (not playing, not paused).
 * @return The number of channels that are currently free.
 */
extern ALMIXER_DECLSPEC ALuint ALMIXER_CALL ALmixer_CountAllFreeChannels(void);

/**
 * Returns the number of channels that are currently available for playback (not playing, not paused),
 * excluding the channels that have been reserved.
 * @return The number of channels that are currently in free, excluding the channels that have been reserved.
 * @see ALmixer_ReserveChannels
 */
extern ALMIXER_DECLSPEC ALuint ALMIXER_CALL ALmixer_CountUnreservedFreeChannels(void);

/**
 * Returns the number of channels that are currently in use (playing/paused).
 * @return The number of channels that are currently in use.
 * @see ALmixer_ReserveChannels
 */
extern ALMIXER_DECLSPEC ALuint ALMIXER_CALL ALmixer_CountAllUsedChannels(void);

/**
 * Returns the number of channels that are currently in use (playing/paused),
 * excluding the channels that have been reserved.
 * @return The number of channels that are currently in use excluding the channels that have been reserved.
 * @see ALmixer_ReserveChannels
 */
extern ALMIXER_DECLSPEC ALuint ALMIXER_CALL ALmixer_CountUnreservedUsedChannels(void);


#ifdef DOXYGEN_ONLY
/**
 * Returns the number of allocated channels.
 * This is just a convenience alias to ALmixer_AllocateChannels(-1).
 * @see ALmixer_AllocateChannels
 */ 
ALuint ALmixer_CountTotalChannels(void);
#else
#define ALmixer_CountTotalChannels() ALmixer_AllocateChannels(-1)
#endif




#ifdef DOXYGEN_ONLY
/**
 * Returns the number of reserved channels.
 * This is just a convenience alias to ALmixer_ReserveChannels(-1).
 * @see ALmixer_ReserveChannels
 */ 
ALuint ALmixer_CountReservedChannels(void);
#else
#define ALmixer_CountReservedChannels() ALmixer_ReserveChannels(-1)
#endif


/**
 * @}
 */

/**
 * @defgroup DebugAPI Debug APIs
 * @{
 * Functions for debugging purposes. These may be removed in future versions.
 */
 

/* For testing */
#if 0
extern ALMIXER_DECLSPEC void ALMIXER_CALL ALmixer_OutputAttributes(void);
#endif
/** This function may be removed in the future. For debugging. Prints to stderr. Lists the decoders available. */ 
extern ALMIXER_DECLSPEC void ALMIXER_CALL ALmixer_OutputDecoders(void);
/** This function may be removed in the future. For debugging. Prints to stderr. */ 
extern ALMIXER_DECLSPEC void ALMIXER_CALL ALmixer_OutputOpenALInfo(void);

/** This function may be removed in the future. Returns true if compiled with threads, false if not. */ 
extern ALMIXER_DECLSPEC ALboolean ALMIXER_CALL ALmixer_CompiledWithThreadBackend(void);

/**
 * @}
 */




/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif


#endif /* _SDL_ALMIXER_H_ */

/* end of SDL_ALmixer.h ... */


