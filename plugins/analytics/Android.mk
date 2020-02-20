#----------------------------------------------------------------------------
#
# This file is part of the Corona game engine.
# For overview and more information on licensing please refer to README.md 
# Home page: https://github.com/coronalabs/corona
# Contact: support@coronalabs.com
#
#----------------------------------------------------------------------------

LOCAL_PATH:= $(call my-dir)
CORONA_ROOT:=$(LOCAL_PATH)/../..

########################################################################################################

include $(CLEAR_VARS)

LOCAL_MODULE    := analytics

PLUGIN_ADS_DIR  := shared
LIBRTT_DIR      := $(CORONA_ROOT)/librtt
CORONA_API_DIR  := $(CORONA_ROOT)/librtt/Corona
CORONA_API_PRIVATE_DIR  := $(CORONA_ROOT)/plugins/shared
LUA_API_DIR     := $(CORONA_ROOT)/external/lua-5.1.3/src

LOCAL_C_INCLUDES := \
	$(PLUGIN_ADS_DIR) \
	$(LIBRTT_DIR) \
	$(CORONA_API_DIR) \
	$(CORONA_API_PRIVATE_DIR) \
	$(LUA_API_DIR)

LOCAL_SRC_FILES  := \
	$(PLUGIN_ADS_DIR)/CoronaAnalyticsLibrary.cpp \
	$(PLUGIN_ADS_DIR)/../../../platform/android/ndk/generated/analytics.c \
	$(PLUGIN_ADS_DIR)/../../../platform/android/ndk/generated/CoronaProvider.analytics.c

LOCAL_CFLAGS     := \
	-DANDROID_NDK \
	-DNDEBUG \
	-D_REENTRANT \
	-DRtt_ANDROID_ENV

#	-DLUA_USE_LINUX

LOCAL_SHARED_LIBRARIES := libcorona liblua
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
