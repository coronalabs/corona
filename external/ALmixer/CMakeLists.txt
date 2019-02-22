cmake_minimum_required(VERSION 3.4.1)
project(almixer)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSOUND_SUPPORTS_WAV -DSOUND_SUPPORTS_OGG -DANDROID_NDK -DENABLE_ALMIXER_THREADS -DALMIXER_COMPILE_WITHOUT_SDL -DNDEBUG -DALMIXER_USE_APPORTABLE_OPENAL_EXTENSIONS -DOPENAL_FIXED_POINT -DOPENAL_FIXED_POINT_SHIFT=16")

include_directories(
	${LibOpenALPath}/include/AL
	${LibAlMixerPath}
	${LibAlMixerPath}/Isolated
	${LibAlMixerPath}/Isolated/LGPL
	${LibVorbisPath}
	${LibMpg123Path}
	${LibLuaSoPath}
)

add_library( almixer SHARED 
	${LibAlMixerPath}/ALmixer.c
	${LibAlMixerPath}/CircularQueue.c
	${LibAlMixerPath}/LinkedList.c
	${LibAlMixerPath}/luaal.c
	${LibAlMixerPath}/Isolated/ALmixer_RWops.c
	${LibAlMixerPath}/Isolated/SimpleMutex.c
	${LibAlMixerPath}/Isolated/SimpleThreadPosix.c
	${LibAlMixerPath}/Isolated/SoundDecoder.c
	${LibAlMixerPath}/Isolated/tErrorLib.c
	${LibAlMixerPath}/Isolated/LGPL/wav.c
	${LibAlMixerPath}/Isolated/LGPL/mpg123.c
	${LibAlMixerPath}/Isolated/LGPL/oggtremor.c
	${LibAlMixerPath}/Isolated/LGPL/SDL_sound_minimal.c
	${LibVorbisPath}/bitwise.c
	${LibVorbisPath}/codebook.c
	${LibVorbisPath}/dsp.c
	${LibVorbisPath}/floor0.c
	${LibVorbisPath}/floor1.c
	${LibVorbisPath}/floor_lookup.c
	${LibVorbisPath}/framing.c
	${LibVorbisPath}/info.c
	${LibVorbisPath}/mapping0.c
	${LibVorbisPath}/mdct.c
	${LibVorbisPath}/misc.c
	${LibVorbisPath}/res012.c
	${LibVorbisPath}/vorbisfile.c
)

target_link_libraries ( almixer openal mpg123 lua log dl OpenSLES )