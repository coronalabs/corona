LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES = \
	Tremor/bitwise.c \
	Tremor/codebook.c \
	Tremor/dsp.c \
	Tremor/floor0.c \
	Tremor/floor1.c \
	Tremor/floor_lookup.c \
	Tremor/framing.c \
	Tremor/info.c \
	Tremor/mapping0.c \
	Tremor/mdct.c \
	Tremor/misc.c \
	Tremor/res012.c \
	Tremor/vorbisfile.c

LOCAL_CFLAGS+= -O2

ifeq ($(TARGET_ARCH),arm)
LOCAL_CFLAGS+= -D_ARM_ASSEM_
endif
	
LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/Tremor

LOCAL_ARM_MODE := arm

LOCAL_MODULE := libvorbisidec

include $(BUILD_SHARED_LIBRARY)
