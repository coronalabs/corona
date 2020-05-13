TARGET_PLATFORM := android-14

# For this to work, you have to be in the right directory (corona/jni)
LOCAL_PATH := $(call my-dir)
CORONA_ROOT := ../../../..


########################################################################################################

include $(CLEAR_VARS)


LOCAL_MODULE     := lua
LOCAL_ARM_MODE   := arm
LIBLUA_DIR       := $(CORONA_ROOT)/external/lua-5.1.3
LOCAL_C_INCLUDES := $(LIBLUA_DIR)/src
LOCAL_SRC_FILES  := \
	$(LIBLUA_DIR)/src/lapi.c \
	$(LIBLUA_DIR)/src/lauxlib.c \
	$(LIBLUA_DIR)/src/lbaselib.c \
	$(LIBLUA_DIR)/src/lcode.c \
	$(LIBLUA_DIR)/src/ldblib.c \
	$(LIBLUA_DIR)/src/ldebug.c \
	$(LIBLUA_DIR)/src/ldo.c \
	$(LIBLUA_DIR)/src/ldump.c \
	$(LIBLUA_DIR)/src/lfunc.c \
	$(LIBLUA_DIR)/src/lgc.c \
	$(LIBLUA_DIR)/src/linit.c \
	$(LIBLUA_DIR)/src/liolib.c \
	$(LIBLUA_DIR)/src/llex.c \
	$(LIBLUA_DIR)/src/lmathlib.c \
	$(LIBLUA_DIR)/src/lmem.c \
	$(LIBLUA_DIR)/src/loadlib.c \
	$(LIBLUA_DIR)/src/lobject.c \
	$(LIBLUA_DIR)/src/lopcodes.c \
	$(LIBLUA_DIR)/src/loslib.c \
	$(LIBLUA_DIR)/src/lparser.c \
	$(LIBLUA_DIR)/src/lstate.c \
	$(LIBLUA_DIR)/src/lstring.c \
	$(LIBLUA_DIR)/src/lstrlib.c \
	$(LIBLUA_DIR)/src/ltable.c \
	$(LIBLUA_DIR)/src/ltablib.c \
	$(LIBLUA_DIR)/src/ltm.c \
	$(LIBLUA_DIR)/src/luac.c \
	$(LIBLUA_DIR)/src/lundump.c \
	$(LIBLUA_DIR)/src/lvm.c \
	$(LIBLUA_DIR)/src/lzio.c \
	$(LIBLUA_DIR)/src/print.c

# files that are not compiled
EXTERNAL_FILES_OLD  := \
	$(LIBLUA_DIR)/etc/min.c \
	$(LIBLUA_DIR)/src/lua.c \
	$(LIBLUA_DIR)/etc/lundump_swap.c

LOCAL_CFLAGS     := -DANDROID_NDK -DRtt_ANDROID_ENV -DNDEBUG -DLUA_USE_DLOPEN -DLUA_USE_POSIX
LOCAL_LDLIBS     := -llog -ldl

ifeq ($(TARGET_ARCH),arm)
LOCAL_CFLAGS+= -D_ARM_ASSEM_
endif

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
#	LOCAL_CFLAGS := $(LOCAL_CFLAGS) -DHAVE_NEON=0
#	LOCAL_ARM_NEON  := true	
endif

include $(BUILD_SHARED_LIBRARY)

########################################################################################################

include $(CLEAR_VARS)

LOCAL_MODULE     := openal
LOCAL_ARM_MODE   := arm
LIBOPENAL_DIR    := $(CORONA_ROOT)/external/openal-soft_apportable/jni/OpenAL
LOCAL_C_INCLUDES := $(LIBOPENAL_DIR)/include $(LIBOPENAL_DIR)/OpenAL32/Include $(LIBOPENAL_DIR)/Alc

LOCAL_SRC_FILES  := $(LIBOPENAL_DIR)/OpenAL32/alAuxEffectSlot.c \
                    $(LIBOPENAL_DIR)/OpenAL32/alBuffer.c        \
                    $(LIBOPENAL_DIR)/OpenAL32/alDatabuffer.c    \
                    $(LIBOPENAL_DIR)/OpenAL32/alEffect.c        \
                    $(LIBOPENAL_DIR)/OpenAL32/alError.c         \
                    $(LIBOPENAL_DIR)/OpenAL32/alExtension.c     \
                    $(LIBOPENAL_DIR)/OpenAL32/alFilter.c        \
                    $(LIBOPENAL_DIR)/OpenAL32/alListener.c      \
                    $(LIBOPENAL_DIR)/OpenAL32/alSource.c        \
                    $(LIBOPENAL_DIR)/OpenAL32/alState.c         \
                    $(LIBOPENAL_DIR)/OpenAL32/alThunk.c         \
                    $(LIBOPENAL_DIR)/Alc/ALc.c                  \
                    $(LIBOPENAL_DIR)/Alc/alcConfig.c            \
                    $(LIBOPENAL_DIR)/Alc/alcEcho.c              \
                    $(LIBOPENAL_DIR)/Alc/alcModulator.c         \
                    $(LIBOPENAL_DIR)/Alc/alcReverb.c            \
                    $(LIBOPENAL_DIR)/Alc/alcRing.c              \
                    $(LIBOPENAL_DIR)/Alc/alcThread.c            \
                    $(LIBOPENAL_DIR)/Alc/ALu.c                  \
                    $(LIBOPENAL_DIR)/Alc/android.c              \
                    $(LIBOPENAL_DIR)/Alc/audiotrack.c           \
                    $(LIBOPENAL_DIR)/Alc/bs2b.c                 \
                    $(LIBOPENAL_DIR)/Alc/mixer.c                \
                    $(LIBOPENAL_DIR)/Alc/null.c                 \
                    $(LIBOPENAL_DIR)/Alc/panning.c              \
                    $(LIBOPENAL_DIR)/Alc/opensles.c             \

LOCAL_CFLAGS     := -DAL_BUILD_LIBRARY -DAL_ALEXT_PROTOTYPES -DNDEBUG -DANDROID -DPOST_FROYO \
                    -fpic \
                    -ffunction-sections \
                    -funwind-tables \
                    -fstack-protector \
                    -fno-short-enums \
                    -DHAVE_GCC_VISIBILITY \
                    -O3

LOCAL_LDLIBS     := -llog -Wl,-s

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
#	LOCAL_CFLAGS := $(LOCAL_CFLAGS) -DHAVE_NEON=0
#	LOCAL_CFLAGS += -marm -DOPENAL_FIXED_POINT -DOPENAL_FIXED_POINT_SHIFT=16
#	Not sure if fixed point really helps with armv7. It was originally for armv5/6. Need to benchmark.
	LOCAL_CFLAGS += -DOPENAL_FIXED_POINT -DOPENAL_FIXED_POINT_SHIFT=16
#	LOCAL_ARM_NEON  := true	
endif


MAX_SOURCES_LOW ?= 32
MAX_SOURCES_START ?= 32
MAX_SOURCES_HIGH ?= 32
LOCAL_CFLAGS += -DMAX_SOURCES_LOW=$(MAX_SOURCES_LOW) -DMAX_SOURCES_START=$(MAX_SOURCES_START) -DMAX_SOURCES_HIGH=$(MAX_SOURCES_HIGH)



include $(BUILD_SHARED_LIBRARY)

########################################################################################################

# NOTE: This is incomplete as the mpg123 I downloaded doesn't compile.
# I'm leaving it in as a template for future implementation.

include $(CLEAR_VARS)

LOCAL_MODULE     := mpg123
LOCAL_ARM_MODE   := arm
LIBMPG_DIR    := $(CORONA_ROOT)/external/mpg123-1.13.1/src/libmpg123

LOCAL_C_INCLUDES := \
	$(LIBMPG_DIR) \

LIBMPG_FILES := \
	$(LIBMPG_DIR)/compat.c \
	$(LIBMPG_DIR)/dct64.c \
	$(LIBMPG_DIR)/equalizer.c \
	$(LIBMPG_DIR)/format.c \
	$(LIBMPG_DIR)/frame.c \
	$(LIBMPG_DIR)/icy.c \
	$(LIBMPG_DIR)/icy2utf8.c \
	$(LIBMPG_DIR)/id3.c \
	$(LIBMPG_DIR)/index.c \
	$(LIBMPG_DIR)/layer1.c \
	$(LIBMPG_DIR)/layer2.c \
	$(LIBMPG_DIR)/layer3.c \
	$(LIBMPG_DIR)/libmpg123.c \
	$(LIBMPG_DIR)/ntom.c \
	$(LIBMPG_DIR)/optimize.c \
	$(LIBMPG_DIR)/parse.c \
	$(LIBMPG_DIR)/readers.c \
	$(LIBMPG_DIR)/tabinit.c \
	$(LIBMPG_DIR)/stringbuf.c \
	$(LIBMPG_DIR)/synth.c \
	$(LIBMPG_DIR)/synth_arm.S \
	$(LIBMPG_DIR)/feature.c \
	$(LIBMPG_DIR)/synth_8bit.c \
	$(LIBMPG_DIR)/synth_s32.c \
#	$(LIBMPG_DIR)/synth_real.c \
#	$(LIBMPG_DIR)/synth_arm_accurate.S \
#	$(LIBMPG_DIR)/dither.c \
#	$(LIBMPG_DIR)/lfs_alias.c \
#	$(LIBMPG_DIR)/getcpuflags.S \
#	$(LIBMPG_DIR)/lfs_wrap.c \

LOCAL_SRC_FILES  := \
	$(LIBMPG_FILES) \

LOCAL_CFLAGS     := \
	-DMPG123_NO_CONFIGURE \
	-DHAVE_STDLIB_H \
	-DHAVE_STRERROR \
	-DHAVE_SYS_TYPES_H \
	-DFPM_ARM \
	-DFIXED_POINT \
	-DANDROID_NDK \
	-D_ARM_ASSEM_ \
	-DMPG123_NO_LARGENAME \
	-DNO_REAL \
	-DOPT_GENERIC \
	-DNDEBUG \

#	-DREAL_IS_FIXED \
#	-DOPT_ARM \ # triggers ASM errors
#	e.g. Error: selected processor does not support `smull fp,ip,r6,r7'

#	-DNO_8BIT \
#	-DHAVE_CONFIG_H \
#	-DHAVE_CONFIG_H 
#	-DFPM_ARM 
#LOCAL_LDLIBS     := -llog

LOCAL_LDLIBS     := -llog -Wl,-s

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
#	LOCAL_CFLAGS := $(LOCAL_CFLAGS) -DHAVE_NEON=0
#	LOCAL_ARM_NEON  := true	
endif

include $(BUILD_SHARED_LIBRARY)

########################################################################################################

include $(CLEAR_VARS)

LOCAL_MODULE     := almixer

LIBALMIXER_DIR   := $(CORONA_ROOT)/external/ALmixer
LIBOPENAL_DIR    := $(CORONA_ROOT)/external/openal-soft_apportable/jni/OpenAL
LIBMPG_DIR    := $(CORONA_ROOT)/external/mpg123-1.13.1/src/libmpg123
LIBVORBIS_DIR    := $(CORONA_ROOT)/external/tremor/Tremor

LOCAL_C_INCLUDES := \
	$(LIBOPENAL_DIR)/include/AL \
	$(LIBALMIXER_DIR) \
	$(LIBALMIXER_DIR)/Isolated \
	$(LIBALMIXER_DIR)/Isolated/LGPL \
	$(LIBVORBIS_DIR) \
	$(LIBMPG_DIR) \

LIBALMIXER_FILES := \
	$(LIBALMIXER_DIR)/ALmixer.c \
	$(LIBALMIXER_DIR)/CircularQueue.c \
	$(LIBALMIXER_DIR)/LinkedList.c \
	$(LIBALMIXER_DIR)/Isolated/ALmixer_RWops.c \
	$(LIBALMIXER_DIR)/Isolated/SimpleMutex.c \
	$(LIBALMIXER_DIR)/Isolated/SimpleThreadPosix.c \
	$(LIBALMIXER_DIR)/Isolated/SoundDecoder.c \
	$(LIBALMIXER_DIR)/Isolated/tErrorLib.c \
	$(LIBALMIXER_DIR)/Isolated/LGPL/wav.c \
	$(LIBALMIXER_DIR)/Isolated/LGPL/mpg123.c \
	$(LIBALMIXER_DIR)/Isolated/LGPL/oggtremor.c \
	$(LIBALMIXER_DIR)/Isolated/LGPL/SDL_sound_minimal.c \
	
LIBVORBIS_FILES = \
	$(LIBVORBIS_DIR)/bitwise.c \
	$(LIBVORBIS_DIR)/codebook.c \
	$(LIBVORBIS_DIR)/dsp.c \
	$(LIBVORBIS_DIR)/floor0.c \
	$(LIBVORBIS_DIR)/floor1.c \
	$(LIBVORBIS_DIR)/floor_lookup.c \
	$(LIBVORBIS_DIR)/framing.c \
	$(LIBVORBIS_DIR)/info.c \
	$(LIBVORBIS_DIR)/mapping0.c \
	$(LIBVORBIS_DIR)/mdct.c \
	$(LIBVORBIS_DIR)/misc.c \
	$(LIBVORBIS_DIR)/res012.c \
	$(LIBVORBIS_DIR)/vorbisfile.c

LOCAL_SRC_FILES  := \
	$(LIBALMIXER_FILES) \
	$(LIBVORBIS_FILES)

LOCAL_CFLAGS     := \
	-DSOUND_SUPPORTS_WAV \
	-DSOUND_SUPPORTS_OGG \
	-DANDROID_NDK \
	-DENABLE_ALMIXER_THREADS \
	-DALMIXER_COMPILE_WITHOUT_SDL \
	-DNDEBUG \
	-DALMIXER_USE_APPORTABLE_OPENAL_EXTENSIONS \
	-DOPENAL_FIXED_POINT -DOPENAL_FIXED_POINT_SHIFT=16 \
	$(MY_ALMIXER_CFLAGS)

#	-DDEBUG_CHATTER \
#	-DSOUND_SUPPORTS_OGG \
#LOCAL_STATIC_LIBRARIES  := libmpg

LOCAL_SHARED_LIBRARIES := libopenal $(MY_LINK_TO_LIBMPG)
LOCAL_LDLIBS     := -llog -ldl
LOCAL_LDLIBS     += -lOpenSLES

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
#	LOCAL_CFLAGS := $(LOCAL_CFLAGS) -DHAVE_NEON=0
#	LOCAL_ARM_NEON  := true	
endif

include $(BUILD_SHARED_LIBRARY)

########################################################################################################

include $(CLEAR_VARS)

LOCAL_MODULE    := jnlua5.1

LIBJNLUA_DIR    := $(CORONA_ROOT)/external/JNLua/src/main/c
LIBLUA_DIR      := $(CORONA_ROOT)/external/lua-5.1.3

LOCAL_C_INCLUDES := \
	$(LIBJNLUA_DIR) \
	$(LIBLUA_DIR)/src

LIBJNLUA_FILES = \
	$(LIBJNLUA_DIR)/jnlua.c

LOCAL_SRC_FILES  := \
	$(LIBJNLUA_FILES)

LOCAL_CFLAGS     := \
	-DANDROID_NDK \
	-DNDEBUG \
	-D_REENTRANT \
	-DRtt_ANDROID_ENV \
	-DLUA_USE_POSIX \
	-DLUA_USE_DLOPEN

LOCAL_SHARED_LIBRARIES := liblua
LOCAL_LDLIBS := -llog

ifeq ($(TARGET_ARCH),arm)
LOCAL_CFLAGS+= -D_ARM_ASSEM_
endif

LOCAL_ARM_MODE := arm

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
#	LOCAL_CFLAGS := $(LOCAL_CFLAGS) -DHAVE_NEON=0
#	LOCAL_ARM_NEON  := true	
endif

include $(BUILD_SHARED_LIBRARY)

########################################################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := corona

LOCAL_ARM_MODE   := arm

LOCAL_CFLAGS := -DANDROID_NDK \
                -DDISABLE_IMPORTGL \
                -DRtt_ANDROID_ENV \
				-DALMIXER_COMPILE_WITHOUT_SDL \
				-DRtt_FEATURES_ALL \
				-DNDEBUG \
				-DLUA_USE_POSIX \
				-DLUA_USE_DLOPEN \
				-DOPENAL_FIXED_POINT -DOPENAL_FIXED_POINT_SHIFT=16 \
				$(MY_CFLAGS)

# These flags are for libjpeg
LOCAL_CFLAGS += -DAVOID_TABLES -O3 -fstrict-aliasing -fprefetch-loop-arrays

# Flags to help reduce binary size.
# fvisibility=hidden will hide the elf symbpls which default to public
# Other ones work together so that each function is in its own output file then --gc-sections will remove unused code
LOCAL_CFLAGS += -fvisibility=hidden -ffunction-sections -fdata-sections
LOCAL_CPPFLAGS += -fvisibility=hidden -ffunction-sections -fdata-sections
ifeq ($(APP_OPTIM),release)
	LOCAL_LDFLAGS += -Wl,--gc-sections
endif
#                -DRtt_TRIAL
#                -DRtt_DEBUG \
#				-DRtt_REAL_FIXED 

LIBALMIXER_DIR := $(CORONA_ROOT)/external/ALmixer
LIBB2SEPARATORCPP_DIR := $(CORONA_ROOT)/external/b2Separator-cpp
LIBBOX2D_DIR := $(CORONA_ROOT)/external/Box2D/Box2D
LIBBOX2D_INC := $(CORONA_ROOT)/external/Box2D
LIBJPEG_DIR := $(CORONA_ROOT)/external/libjpeg
LIBLUA_DIR := $(CORONA_ROOT)/external/lua-5.1.3
LIBOPENAL_DIR := $(CORONA_ROOT)/external/openal-soft_apportable/jni/OpenAL
LIBPNG_DIR := $(CORONA_ROOT)/external/lpng1256
LIBSMOOTHPOLYGON_DIR := $(CORONA_ROOT)/external/smoothpolygon
LIBSQLITE_DIR := $(CORONA_ROOT)/external/sqlite3
LUAFILESYSTEM_DIR := $(CORONA_ROOT)/external/luafilesystem
LUALPEG_DIR := $(CORONA_ROOT)/external/lpeg
LUASOCKET_DIR := $(CORONA_ROOT)/external/luasocket
TACHYON_DIR  := $(CORONA_ROOT)/librtt/Renderer

LOCAL_C_INCLUDES := \
	$(CORONA_ROOT)/external/sqlite3 \
	$(LIBLUA_DIR)/src \
	$(CORONA_ROOT)/external/rsa \
	$(CORONA_ROOT)/external/LuaHashMap \
	$(LIBOPENAL_DIR)/include/AL \
	$(LIBALMIXER_DIR) \
	$(LIBALMIXER_DIR)/Isolated \
	$(LIBPNG_DIR) \
	$(LUASOCKET_DIR)/src \
	$(LUALPEG_DIR) \
	$(LUAFILESYSTEM_DIR)/src \
	$(CORONA_ROOT)/librtt \
	$(CORONA_ROOT)/librtt/Corona \
	$(CORONA_ROOT)/plugins/shared \
	$(CORONA_ROOT)/platform/android/ndk \
	$(CORONA_ROOT)/platform/shared \
	$(LIBBOX2D_INC) \
	$(CORONA_ROOT) \
    $(LIBJPEG_DIR) \
    $(LIBB2SEPARATORCPP_DIR) \
    $(LIBSMOOTHPOLYGON_DIR) \

# files that are not compiled
EXTERNAL_FILES_OLD  := \
	$(CORONA_ROOT)/external/hmac/hmac_sha2.c \
	$(CORONA_ROOT)/external/hmac/sha2.c \
	$(CORONA_ROOT)/external/lstrip/lstrip.c \
	$(CORONA_ROOT)/external/srlua/glue.c \
	$(CORONA_ROOT)/external/srlua/srlua.c \
	$(CORONA_ROOT)/platform/android/PngReader.cpp \

PLATFORM_FILES := \
	$(CORONA_ROOT)/platform/android/ndk/generated/init.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/runtime.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/easing.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/json.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/dkjson.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/loader_ccdata.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/loader_callback.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/launchpad.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/composer.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/composer_scene.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/timer.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/transition.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/transition_v1.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_button.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_momentumScrolling.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_pickerWheel.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_scrollview.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_slider.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_tabbar.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_tableview.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_progressView.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_searchField.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_segmentedControl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_spinner.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_stepper.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_switch.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_theme_android_sheet.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_theme_android.cpp \
    $(CORONA_ROOT)/platform/android/ndk/generated/widget_theme_android_holo_dark.cpp \
    $(CORONA_ROOT)/platform/android/ndk/generated/widget_theme_android_holo_dark_sheet.cpp \
    $(CORONA_ROOT)/platform/android/ndk/generated/widget_theme_android_holo_light.cpp \
    $(CORONA_ROOT)/platform/android/ndk/generated/widget_theme_android_holo_light_sheet.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_theme_ios_sheet.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_theme_ios.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_theme_ios7_sheet.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/widget_theme_ios7.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/shell.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidAudioPlayer.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_LuaLibOpenSLES.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_PlatformOpenSLESPlayer.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidAudioRecorder.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidBitmap.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidCrypto.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidData.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidDisplayObject.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidEventSound.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidFont.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidImageProvider.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidInputDevice.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidInputDeviceManager.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidLocation.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidMapViewObject.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidModalInteraction.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidDevice.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidPlatform.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidRuntimeDelegate.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidScreenSurface.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidStore.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidStoreProvider.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidStoreTransaction.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidSurface.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidSystemOpenEvent.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidTimer.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidTextFieldObject.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidVideoObject.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidVideoPlayer.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidVideoProvider.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidWebPopup.cpp \
	$(CORONA_ROOT)/platform/android/ndk/Rtt_AndroidWebViewObject.cpp \
	$(CORONA_ROOT)/platform/android/ndk/AndroidBinaryReader.cpp \
	$(CORONA_ROOT)/platform/android/ndk/AndroidBinaryReadResult.cpp \
	$(CORONA_ROOT)/platform/android/ndk/AndroidDisplayObjectRegistry.cpp \
	$(CORONA_ROOT)/platform/android/ndk/AndroidFileReader.cpp \
	$(CORONA_ROOT)/platform/android/ndk/AndroidFontSettings.cpp \
	$(CORONA_ROOT)/platform/android/ndk/AndroidGLView.cpp \
	$(CORONA_ROOT)/platform/android/ndk/AndroidBaseImageDecoder.cpp \
	$(CORONA_ROOT)/platform/android/ndk/AndroidBaseNativeImageDecoder.cpp \
	$(CORONA_ROOT)/platform/android/ndk/AndroidImageData.cpp \
	$(CORONA_ROOT)/platform/android/ndk/AndroidImageDecoder.cpp \
	$(CORONA_ROOT)/platform/android/ndk/AndroidJavaImageDecoder.cpp \
	$(CORONA_ROOT)/platform/android/ndk/AndroidKeyServices.cpp \
	$(CORONA_ROOT)/platform/android/ndk/AndroidNativePngDecoder.cpp \
	$(CORONA_ROOT)/platform/android/ndk/AndroidOperationResult.cpp \
	$(CORONA_ROOT)/platform/android/ndk/AndroidZipFileEntry.cpp \

LIBLUASOCKET_FILES := \
	$(LUASOCKET_DIR)/src/auxiliar.c \
	$(LUASOCKET_DIR)/src/compat.c \
	$(LUASOCKET_DIR)/src/except.c \
	$(LUASOCKET_DIR)/src/io.c \
	$(LUASOCKET_DIR)/src/mime.c	\
	$(LUASOCKET_DIR)/src/select.c \
	$(LUASOCKET_DIR)/src/timeout.c \
	$(LUASOCKET_DIR)/src/unix.c \
	$(LUASOCKET_DIR)/src/buffer.c \
	$(LUASOCKET_DIR)/src/inet.c \
	$(LUASOCKET_DIR)/src/luasocket.c \
	$(LUASOCKET_DIR)/src/options.c \
	$(LUASOCKET_DIR)/src/tcp.c \
	$(LUASOCKET_DIR)/src/udp.c \
	$(LUASOCKET_DIR)/src/usocket.c \
    $(CORONA_ROOT)/platform/android/ndk/generated/ftp.cpp \
    $(CORONA_ROOT)/platform/android/ndk/generated/headers.cpp \
    $(CORONA_ROOT)/platform/android/ndk/generated/http.cpp \
    $(CORONA_ROOT)/platform/android/ndk/generated/ltn12.cpp \
    $(CORONA_ROOT)/platform/android/ndk/generated/mbox.cpp \
    $(CORONA_ROOT)/platform/android/ndk/generated/mime.cpp \
    $(CORONA_ROOT)/platform/android/ndk/generated/smtp.cpp \
    $(CORONA_ROOT)/platform/android/ndk/generated/socket.cpp \
    $(CORONA_ROOT)/platform/android/ndk/generated/ssl.cpp \
    $(CORONA_ROOT)/platform/android/ndk/generated/tp.cpp \
    $(CORONA_ROOT)/platform/android/ndk/generated/url.cpp

LIBRTT_FILES := \
	$(CORONA_ROOT)/external/LuaHashMap/LuaHashMap.c \
	$(CORONA_ROOT)/librtt/Core/Rtt_Allocator.cpp \
	$(CORONA_ROOT)/librtt/Core/Rtt_Array.cpp \
	$(CORONA_ROOT)/librtt/Core/Rtt_ArrayTuple.cpp \
	$(CORONA_ROOT)/librtt/Core/Rtt_Assert.c \
	$(CORONA_ROOT)/librtt/Core/Rtt_AutoResource.cpp \
	$(CORONA_ROOT)/librtt/Core/Rtt_FileSystem.cpp \
	$(CORONA_ROOT)/librtt/Core/Rtt_Fixed.c \
	$(CORONA_ROOT)/librtt/Core/Rtt_FixedBlockAllocator.cpp \
	$(CORONA_ROOT)/librtt/Core/Rtt_FixedMath.c \
	$(CORONA_ROOT)/librtt/Core/Rtt_Geometry.cpp \
	$(CORONA_ROOT)/librtt/Core/Rtt_Math.c \
	$(CORONA_ROOT)/librtt/Core/Rtt_OperationResult.cpp \
	$(CORONA_ROOT)/librtt/Core/Rtt_RefCount.cpp \
	$(CORONA_ROOT)/librtt/Core/Rtt_ResourceHandle.cpp \
	$(CORONA_ROOT)/librtt/Core/Rtt_SharedCount.cpp \
	$(CORONA_ROOT)/librtt/Core/Rtt_String.cpp \
	$(CORONA_ROOT)/librtt/Core/Rtt_StringHash.cpp \
	$(CORONA_ROOT)/librtt/Core/Rtt_Time.c \
	$(CORONA_ROOT)/librtt/Core/Rtt_UseCount.cpp \
	$(CORONA_ROOT)/librtt/Core/Rtt_VersionTimestamp.c \
	$(CORONA_ROOT)/librtt/Core/Rtt_WeakCount.cpp \
	$(CORONA_ROOT)/librtt/Corona/CoronaAssert.c \
	$(CORONA_ROOT)/librtt/Corona/CoronaEvent.cpp \
	$(CORONA_ROOT)/librtt/Corona/CoronaLibrary.cpp \
	$(CORONA_ROOT)/librtt/Corona/CoronaLog.c \
	$(CORONA_ROOT)/librtt/Corona/CoronaLua.cpp \
	$(CORONA_ROOT)/librtt/Corona/CoronaVersion.c \
	$(CORONA_ROOT)/librtt/Corona/CoronaGraphics.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_BitmapMask.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_BitmapPaint.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_BitmapPaintAdapter.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ImageSheetPaintAdapter.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_BufferBitmap.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_CameraPaint.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ClosedPath.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_CompositeObject.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_CompositePaint.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ContainerObject.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_CPUResourcePool.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_Display.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_DisplayDefaults.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_DisplayObject.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_DisplayPath.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_DisplayV2.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_EmbossedTextObject.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_EmitterObject.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_GradientPaint.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_GradientPaintAdapter.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_GroupObject.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ImageFrame.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ImageSheet.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ImageSheetPaint.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ImageSheetUserdata.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_LineObject.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_LuaLibDisplay.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_LuaLibGraphics.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_OpenPath.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_Paint.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_PaintAdapter.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_PlatformBitmap.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_PlatformBitmapTexture.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_RectObject.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_RectPath.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_Scene.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_Shader.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShaderBuiltin.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShaderComposite.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShaderCompositeAdapter.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShaderData.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShaderDataAdapter.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShaderFactory.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShaderInput.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShaderName.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShaderProxy.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShaderResource.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShaderTypes.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShapeAdapter.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShapeAdapterCircle.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShapeAdapterMesh.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShapeAdapterPolygon.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShapeAdapterRect.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShapeAdapterRoundedRect.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShapeObject.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_ShapePath.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_SnapshotObject.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_SpriteObject.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_SpritePlayer.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_SpriteSourceFrame.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_StageObject.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_Tesselator.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_TesselatorCircle.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_TesselatorLine.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_TesselatorPolygon.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_TesselatorMesh.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_TesselatorRect.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_TesselatorRoundedRect.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_TesselatorShape.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_TextObject.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_TextureFactory.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_TextureResource.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_TextureResourceAdapter.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_TextureResourceBitmap.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_TextureResourceBitmapAdapter.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_TextureResourceCanvas.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_TextureResourceCanvasAdapter.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_TextureResourceExternal.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_TextureResourceExternalAdapter.cpp \
	$(CORONA_ROOT)/librtt/Display/Rtt_VertexCache.cpp \
	$(CORONA_ROOT)/librtt/Input/Rtt_InputAxisCollection.cpp \
	$(CORONA_ROOT)/librtt/Input/Rtt_InputAxisDescriptor.cpp \
	$(CORONA_ROOT)/librtt/Input/Rtt_InputAxisType.cpp \
	$(CORONA_ROOT)/librtt/Input/Rtt_InputDeviceCollection.cpp \
	$(CORONA_ROOT)/librtt/Input/Rtt_InputDeviceConnectionState.cpp \
	$(CORONA_ROOT)/librtt/Input/Rtt_InputDeviceDescriptor.cpp \
	$(CORONA_ROOT)/librtt/Input/Rtt_InputDeviceType.cpp \
	$(CORONA_ROOT)/librtt/Input/Rtt_PlatformInputAxis.cpp \
	$(CORONA_ROOT)/librtt/Input/Rtt_PlatformInputDevice.cpp \
	$(CORONA_ROOT)/librtt/Input/Rtt_PlatformInputDeviceManager.cpp \
	$(CORONA_ROOT)/librtt/Input/Rtt_ReadOnlyInputAxisCollection.cpp \
	$(CORONA_ROOT)/librtt/Input/Rtt_ReadOnlyInputDeviceCollection.cpp \
	$(CORONA_ROOT)/librtt/b2GLESDebugDraw.cpp \
	$(CORONA_ROOT)/librtt/Rtt_Archive.cpp \
	$(CORONA_ROOT)/librtt/Rtt_CKWorkflow.cpp \
	$(CORONA_ROOT)/librtt/Rtt_DeviceOrientation.cpp \
	$(CORONA_ROOT)/librtt/Rtt_DisplayObjectExtensions.cpp \
	$(CORONA_ROOT)/librtt/Rtt_Event.cpp \
	$(CORONA_ROOT)/librtt/Rtt_ExplicitTemplates.cpp \
	$(CORONA_ROOT)/librtt/Rtt_FilePath.cpp \
	$(CORONA_ROOT)/librtt/Rtt_GPUStream.cpp \
	$(CORONA_ROOT)/librtt/Rtt_HitTestObject.cpp \
	$(CORONA_ROOT)/librtt/Rtt_KeyName.cpp \
	$(CORONA_ROOT)/librtt/Rtt_Lua.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaAssert.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaAux.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaContainer.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaContext.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaCoronaBaseLib.c \
	$(CORONA_ROOT)/librtt/Rtt_LuaData.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaLibCrypto.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaLibFacebook.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaLibFlurry.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaLibGameNetwork.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaLibInAppStore.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaLibMedia.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaLibNative.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaLibOpenAL.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaLibPhysics.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaLibSQLite.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaLibSystem.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaProxy.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaProxyVTable.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaResource.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaResourceOwner.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaTableIterator.cpp \
	$(CORONA_ROOT)/librtt/Rtt_LuaUserdataProxy.cpp \
	$(CORONA_ROOT)/librtt/Rtt_Matrix.cpp \
	$(CORONA_ROOT)/librtt/Rtt_ParticleSystemObject.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PhysicsContact.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PhysicsContactListener.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PhysicsJoint.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PhysicsWorld.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformAudioPlayer.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformAudioRecorder.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformAudioSessionManager.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformData.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformDisplayObject.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformEventSound.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformExitCallback.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformFBConnect.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformFont.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformImageProvider.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformInAppStore.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformLocation.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformModalInteraction.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformNotifier.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformOpenALPlayer.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformReachability.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformSurface.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformTimer.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformVideoPlayer.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformVideoProvider.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PlatformWebPopup.cpp \
	$(CORONA_ROOT)/librtt/Rtt_Preference.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PreferenceCollection.cpp \
	$(CORONA_ROOT)/librtt/Rtt_PreferenceValue.cpp \
	$(CORONA_ROOT)/librtt/Rtt_RenderingStream.cpp \
	$(CORONA_ROOT)/librtt/Rtt_Resource.cpp \
	$(CORONA_ROOT)/librtt/Rtt_Runtime.cpp \
	$(CORONA_ROOT)/librtt/Rtt_RuntimeDelegate.cpp \
	$(CORONA_ROOT)/librtt/Rtt_RuntimeDelegatePlayer.cpp \
	$(CORONA_ROOT)/librtt/Rtt_Scheduler.cpp \
	$(CORONA_ROOT)/librtt/Rtt_Transform.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/CoronaLibrary.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/CoronaPrototype.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/CoronaProvider.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_add_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_average_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_color_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_colorBurn_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_colorDodge_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_darken_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_difference_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_exclusion_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_glow_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_hardLight_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_hardMix_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_hue_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_lighten_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_linearLight_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_luminosity_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_multiply_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_negation_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_normalMapWith1DirLight_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_normalMapWith1PointLight_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_overlay_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_phoenix_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_pinLight_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_reflect_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_saturation_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_screen_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_softLight_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_subtract_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_vividLight_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_composite_yuv420f_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_default_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_bloom_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_blur_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_blurGaussian_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_blurHorizontal_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_blurVertical_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_brightness_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_bulge_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_chromaKey_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_color_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_colorChannelOffset_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_colorMatrix_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_colorPolynomial_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_contrast_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_crosshatch_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_crystallize_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_desaturate_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_dissolve_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_duotone_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_emboss_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_exposure_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_frostedGlass_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_grayscale_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_hue_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_invert_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_iris_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_levels_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_linearWipe_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_median_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_monotone_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_opTile_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_pixelate_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_polkaDots_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_posterize_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_radialWipe_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_saturate_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_scatter_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_sepia_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_sharpenLuminance_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_sobel_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_step_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_straighten_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_swirl_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_vignette_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_vignetteMask_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_water_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_wobble_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_woodCut_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_filter_zoomBlur_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_generator_checkerboard_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_generator_lenticularHalo_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_generator_linearGradient_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_generator_marchingAnts_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_generator_perlinNoise_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_generator_radialGradient_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_generator_random_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_generator_stripes_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/kernel_generator_sunbeams_gl.cpp \
	$(CORONA_ROOT)/platform/android/ndk/generated/shell_default_gl.cpp \
    $(LIBALMIXER_DIR)/luaal.c \
	$(TACHYON_DIR)/Rtt_CommandBuffer.cpp \
	$(TACHYON_DIR)/Rtt_CPUResource.cpp \
	$(TACHYON_DIR)/Rtt_FrameBufferObject.cpp \
	$(TACHYON_DIR)/Rtt_Geometry_Renderer.cpp \
	$(TACHYON_DIR)/Rtt_GeometryPool.cpp \
	$(TACHYON_DIR)/Rtt_GL.cpp \
	$(TACHYON_DIR)/Rtt_GLCommandBuffer.cpp \
	$(TACHYON_DIR)/Rtt_GLFrameBufferObject.cpp \
	$(TACHYON_DIR)/Rtt_GLGeometry.cpp \
	$(TACHYON_DIR)/Rtt_GLProgram.cpp \
	$(TACHYON_DIR)/Rtt_GLRenderer.cpp \
	$(TACHYON_DIR)/Rtt_GLTexture.cpp \
	$(TACHYON_DIR)/Rtt_GPUResource.cpp \
	$(TACHYON_DIR)/Rtt_HighPrecisionTime.cpp \
	$(TACHYON_DIR)/Rtt_Matrix_Renderer.cpp \
	$(TACHYON_DIR)/Rtt_Program.cpp \
	$(TACHYON_DIR)/Rtt_ProgramFactory.cpp \
	$(TACHYON_DIR)/Rtt_RenderData.cpp \
	$(TACHYON_DIR)/Rtt_Renderer.cpp \
	$(TACHYON_DIR)/Rtt_RenderTypes.cpp \
	$(TACHYON_DIR)/Rtt_Texture.cpp \
	$(TACHYON_DIR)/Rtt_TextureBitmap.cpp \
	$(TACHYON_DIR)/Rtt_TextureVolatile.cpp \
	$(TACHYON_DIR)/Rtt_Uniform.cpp \
	$(TACHYON_DIR)/Rtt_VideoTexture.cpp \

LIBPLUGINS_DIR := $(CORONA_ROOT)/plugins/shared
LIBPLUGINS_FILES := \
	$(LIBPLUGINS_DIR)/CoronaLuaLibrary.cpp \
	$(LIBPLUGINS_DIR)/CoronaLuaLibraryMetadata.cpp

LIBSHARED_DIR := $(CORONA_ROOT)/platform/shared
LIBSHARED_FILES := \
	$(LIBSHARED_DIR)/Rtt_DependencyLoader.cpp \
	$(LIBSHARED_DIR)/Rtt_DependencyUtilsAndroid.cpp

LIBPNG_FILES := \
	$(LIBPNG_DIR)/png.c \
	$(LIBPNG_DIR)/pngerror.c \
	$(LIBPNG_DIR)/pnggccrd.c \
	$(LIBPNG_DIR)/pngget.c \
	$(LIBPNG_DIR)/pngmem.c \
	$(LIBPNG_DIR)/pngpread.c \
	$(LIBPNG_DIR)/pngread.c \
	$(LIBPNG_DIR)/pngrio.c \
	$(LIBPNG_DIR)/pngrtran.c \
	$(LIBPNG_DIR)/pngrutil.c \
	$(LIBPNG_DIR)/pngset.c \
	$(LIBPNG_DIR)/pngtrans.c \
	$(LIBPNG_DIR)/pngvcrd.c \
	$(LIBPNG_DIR)/pngwio.c \
	$(LIBPNG_DIR)/pngwrite.c \
	$(LIBPNG_DIR)/pngwtran.c \
	$(LIBPNG_DIR)/pngwutil.c \

LIBLPEG_FILES := \
  $(LUALPEG_DIR)/lpeg.c \
  $(CORONA_ROOT)/platform/android/ndk/generated/re.cpp

LIBLUAFILESYSTEM_FILES := \
  $(LUAFILESYSTEM_DIR)/src/lfs.c

LIBSQLITE_FILES := \
  $(LIBSQLITE_DIR)/sqlite3.c \
  $(CORONA_ROOT)/external/lsqlite3-7/lsqlite3.c

LIBBOX2D_FILES := \
	$(LIBBOX2D_DIR)/Collision/b2BroadPhase.cpp \
	$(LIBBOX2D_DIR)/Collision/b2CollideCircle.cpp \
	$(LIBBOX2D_DIR)/Collision/b2CollideEdge.cpp \
	$(LIBBOX2D_DIR)/Collision/b2CollidePolygon.cpp \
	$(LIBBOX2D_DIR)/Collision/b2Collision.cpp \
	$(LIBBOX2D_DIR)/Collision/b2Distance.cpp \
	$(LIBBOX2D_DIR)/Collision/b2DynamicTree.cpp \
	$(LIBBOX2D_DIR)/Collision/b2TimeOfImpact.cpp \
	$(LIBBOX2D_DIR)/Collision/Shapes/b2ChainShape.cpp \
	$(LIBBOX2D_DIR)/Collision/Shapes/b2CircleShape.cpp \
	$(LIBBOX2D_DIR)/Collision/Shapes/b2EdgeShape.cpp \
	$(LIBBOX2D_DIR)/Collision/Shapes/b2PolygonShape.cpp \
	$(LIBBOX2D_DIR)/Common/b2BlockAllocator.cpp \
	$(LIBBOX2D_DIR)/Common/b2Draw.cpp \
	$(LIBBOX2D_DIR)/Common/b2FreeList.cpp \
	$(LIBBOX2D_DIR)/Common/b2Math.cpp \
	$(LIBBOX2D_DIR)/Common/b2Settings.cpp \
	$(LIBBOX2D_DIR)/Common/b2StackAllocator.cpp \
	$(LIBBOX2D_DIR)/Common/b2Stat.cpp \
	$(LIBBOX2D_DIR)/Common/b2Timer.cpp \
	$(LIBBOX2D_DIR)/Common/b2TrackedBlock.cpp \
	$(LIBBOX2D_DIR)/Dynamics/b2Body.cpp \
	$(LIBBOX2D_DIR)/Dynamics/b2ContactManager.cpp \
	$(LIBBOX2D_DIR)/Dynamics/b2Fixture.cpp \
	$(LIBBOX2D_DIR)/Dynamics/b2Island.cpp \
	$(LIBBOX2D_DIR)/Dynamics/b2World.cpp \
	$(LIBBOX2D_DIR)/Dynamics/b2WorldCallbacks.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Contacts/b2ChainAndCircleContact.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Contacts/b2ChainAndPolygonContact.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Contacts/b2CircleContact.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Contacts/b2Contact.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Contacts/b2ContactSolver.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Contacts/b2EdgeAndCircleContact.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Contacts/b2PolygonAndCircleContact.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Contacts/b2PolygonContact.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Joints/b2DistanceJoint.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Joints/b2FrictionJoint.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Joints/b2GearJoint.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Joints/b2Joint.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Joints/b2MotorJoint.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Joints/b2MouseJoint.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Joints/b2PrismaticJoint.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Joints/b2PulleyJoint.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Joints/b2RevoluteJoint.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Joints/b2RopeJoint.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Joints/b2WeldJoint.cpp \
	$(LIBBOX2D_DIR)/Dynamics/Joints/b2WheelJoint.cpp \
	$(LIBBOX2D_DIR)/Particle/b2Particle.cpp \
	$(LIBBOX2D_DIR)/Particle/b2ParticleAssembly.cpp \
	$(LIBBOX2D_DIR)/Particle/b2ParticleGroup.cpp \
	$(LIBBOX2D_DIR)/Particle/b2ParticleSystem.cpp \
	$(LIBBOX2D_DIR)/Particle/b2VoronoiDiagram.cpp

LIBB2SEPARATORCPP_FILES := \
	$(LIBB2SEPARATORCPP_DIR)/b2Separator.cpp

LIBSMOOTHPOLYGON_FILES := \
	$(LIBSMOOTHPOLYGON_DIR)/SmoothPolygon.cpp

LIBJPEG_FILES =  \
	$(LIBJPEG_DIR)/jcapimin.c \
	$(LIBJPEG_DIR)/jcapistd.c \
	$(LIBJPEG_DIR)/jccoefct.c \
	$(LIBJPEG_DIR)/jccolor.c \
	$(LIBJPEG_DIR)/jcdctmgr.c \
	$(LIBJPEG_DIR)/jchuff.c \
	$(LIBJPEG_DIR)/jcinit.c \
	$(LIBJPEG_DIR)/jcmainct.c \
	$(LIBJPEG_DIR)/jcmarker.c \
	$(LIBJPEG_DIR)/jcmaster.c \
	$(LIBJPEG_DIR)/jcomapi.c \
	$(LIBJPEG_DIR)/jcparam.c \
	$(LIBJPEG_DIR)/jcphuff.c \
	$(LIBJPEG_DIR)/jcprepct.c \
	$(LIBJPEG_DIR)/jcsample.c \
	$(LIBJPEG_DIR)/jctrans.c \
	$(LIBJPEG_DIR)/jdapimin.c \
	$(LIBJPEG_DIR)/jdapistd.c \
	$(LIBJPEG_DIR)/jdatadst.c \
	$(LIBJPEG_DIR)/jdatasrc.c \
	$(LIBJPEG_DIR)/jdcoefct.c \
	$(LIBJPEG_DIR)/jdcolor.c \
	$(LIBJPEG_DIR)/jddctmgr.c \
	$(LIBJPEG_DIR)/jdhuff.c \
	$(LIBJPEG_DIR)/jdinput.c \
	$(LIBJPEG_DIR)/jdmainct.c \
	$(LIBJPEG_DIR)/jdmarker.c \
	$(LIBJPEG_DIR)/jdmaster.c \
	$(LIBJPEG_DIR)/jdmerge.c \
	$(LIBJPEG_DIR)/jdphuff.c \
	$(LIBJPEG_DIR)/jdpostct.c \
	$(LIBJPEG_DIR)/jdsample.c \
	$(LIBJPEG_DIR)/jdtrans.c \
	$(LIBJPEG_DIR)/jerror.c \
	$(LIBJPEG_DIR)/jfdctflt.c \
	$(LIBJPEG_DIR)/jfdctfst.c \
	$(LIBJPEG_DIR)/jfdctint.c \
	$(LIBJPEG_DIR)/jidctflt.c \
	$(LIBJPEG_DIR)/jidctred.c \
	$(LIBJPEG_DIR)/jquant1.c \
	$(LIBJPEG_DIR)/jquant2.c \
	$(LIBJPEG_DIR)/jutils.c \
	$(LIBJPEG_DIR)/jmemmgr.c \
	$(LIBJPEG_DIR)/jmem-android.c \
	$(LIBJPEG_DIR)/jidctint.c \
	$(LIBJPEG_DIR)/jidctfst.S \

LOCAL_SRC_FILES := \
    importgl.c \
    JavaToNativeBridge.cpp \
    JavaToNativeShim.cpp \
	NativeToJavaBridge.cpp \
    $(LIBBOX2D_FILES) \
    $(PLATFORM_FILES) \
    $(LIBRTT_FILES) \
    $(LIBPLUGINS_FILES) \
    $(LIBSHARED_FILES) \
    $(LIBLUASOCKET_FILES) \
	$(LIBLPEG_FILES) \
	$(LIBLUAFILESYSTEM_FILES) \
    $(LIBSQLITE_FILES) \
    $(LIBPNG_FILES) \
    $(LIBB2SEPARATORCPP_FILES) \
    $(LIBSMOOTHPOLYGON_FILES) \

LOCAL_SHARED_LIBRARIES := liblua libjnlua5.1 $(MY_LINK_TO_LIBMPG) libopenal libalmixer cpufeatures
LOCAL_LDLIBS := -lGLESv2 -lEGL -ljnigraphics -ldl -llog -lz -lOpenSLES

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
#	LOCAL_CFLAGS := $(LOCAL_CFLAGS) -DHAVE_NEON=0
#	LOCAL_ARM_NEON  := true	
endif

include $(BUILD_SHARED_LIBRARY)


########################################################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := plugins

LOCAL_ARM_MODE := arm

LOCAL_SHARED_LIBRARIES := ads analytics

include $(BUILD_SHARED_LIBRARY)


########################################################################################################

$(call import-module,ads)
$(call import-module,analytics)

$(call import-module,cpufeatures)


