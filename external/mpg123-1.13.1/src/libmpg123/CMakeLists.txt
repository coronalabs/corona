cmake_minimum_required(VERSION 3.4.1)
project(mpg123)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,max-page-size=16384")
add_definitions(
	-DMPG123_NO_CONFIGURE
	-DHAVE_STDLIB_H
	-DHAVE_STRERROR
	-DHAVE_SYS_TYPES_H
	-DMPG123_NO_LARGENAME
	-DOPT_GENERIC
	-DFIXED_POINT
	-DNO_REAL
)

add_definitions(
	-DFPM_ARM
	-D_ARM_ASSEM_
)


add_library( mpg123 SHARED
	compat.c
	dct64.c
	equalizer.c
	format.c
	frame.c
	icy.c
	icy2utf8.c
	id3.c
	index.c
	layer1.c
	layer2.c
	layer3.c
	libmpg123.c
	ntom.c
	optimize.c
	parse.c
	readers.c
	tabinit.c
	stringbuf.c
	synth.c
	synth_arm.S
	feature.c
	synth_8bit.c
	synth_s32.c
)

target_link_libraries( mpg123 log )
