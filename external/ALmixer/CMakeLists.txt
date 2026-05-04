cmake_minimum_required(VERSION 3.13)
project(almixer)

add_definitions(
	-DSOUND_SUPPORTS_WAV
	-DSOUND_SUPPORTS_OGG
	-DENABLE_ALMIXER_THREADS
	-DALMIXER_COMPILE_WITHOUT_SDL
#	-DALMIXER_USE_APPORTABLE_OPENAL_EXTENSIONS
	-DOPENAL_FIXED_POINT
	-DOPENAL_FIXED_POINT_SHIFT=16
)

# if not Corona Cards
add_definitions(
	-DMPG123_NO_CONFIGURE
	-DSOUND_SUPPORTS_MPG123
)

include_directories(
	../openal-soft/include/AL
	../ALmixer
	Isolated
	../tremor/Tremor
	../mpg123-1.13.1/src/libmpg123
	../lua-5.1.3/src
)

add_library( almixer SHARED 
	ALmixer.c
	CircularQueue.c
	LinkedList.c
	luaal.c
	Isolated/ALmixer_RWops.c
	Isolated/SimpleMutex.c
	Isolated/SimpleThreadPosix.c
	Isolated/SoundDecoder.c
	Isolated/tErrorLib.c
	Isolated/LGPL/wav.c
	Isolated/LGPL/mpg123.c
	Isolated/LGPL/oggtremor.c
	Isolated/LGPL/SDL_sound_minimal.c
	../tremor/Tremor/bitwise.c
	../tremor/Tremor/codebook.c
	../tremor/Tremor/dsp.c
	../tremor/Tremor/floor0.c
	../tremor/Tremor/floor1.c
	../tremor/Tremor/floor_lookup.c
	../tremor/Tremor/framing.c
	../tremor/Tremor/info.c
	../tremor/Tremor/mapping0.c
	../tremor/Tremor/mdct.c
	../tremor/Tremor/misc.c
	../tremor/Tremor/res012.c
	../tremor/Tremor/vorbisfile.c
)

if(ANDROID AND (ANDROID_ABI STREQUAL "arm64-v8a" OR ANDROID_ABI STREQUAL "x86_64"))
	# NDK version < 28
	target_link_options(almixer PRIVATE "-Wl,-z,max-page-size=16384")
	# NDK version <= 22
	target_link_options(almixer PRIVATE "-Wl,-z,common-page-size=16384")
endif()
target_link_libraries ( almixer OpenAL  lua log dl OpenSLES mpg123 )
