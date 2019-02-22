cmake_minimum_required(VERSION 3.4.1)
project(mpg123)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMPG123_NO_CONFIGURE -DHAVE_STDLIB_H -DHAVE_STRERROR -DHAVE_SYS_TYPES_H -DFPM_ARM -DFIXED_POINT -DANDROID_NDK -D_ARM_ASSEM_ -DMPG123_NO_LARGENAME -DNO_REAL -DOPT_GENERIC -DNDEBUG")

add_library( mpg123 SHARED
	${LibMpg123Path}/compat.c
	${LibMpg123Path}/dct64.c
	${LibMpg123Path}/equalizer.c
	${LibMpg123Path}/format.c
	${LibMpg123Path}/frame.c
	${LibMpg123Path}/icy.c
	${LibMpg123Path}/icy2utf8.c
	${LibMpg123Path}/id3.c
	${LibMpg123Path}/index.c
	${LibMpg123Path}/layer1.c
	${LibMpg123Path}/layer2.c
	${LibMpg123Path}/layer3.c
	${LibMpg123Path}/libmpg123.c
	${LibMpg123Path}/ntom.c
	${LibMpg123Path}/optimize.c
	${LibMpg123Path}/parse.c
	${LibMpg123Path}/readers.c
	${LibMpg123Path}/tabinit.c
	${LibMpg123Path}/stringbuf.c
	${LibMpg123Path}/synth.c
	${LibMpg123Path}/synth_arm.S
	${LibMpg123Path}/feature.c
	${LibMpg123Path}/synth_8bit.c
	${LibMpg123Path}/synth_s32.c
)

target_link_libraries( mpg123 log )