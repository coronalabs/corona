/* ----------------------------------------------------------------------------
//
// alc.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
//
// ----------------------------------------------------------------------------
*/

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifdef OPENAL_API_EXPORT
#	define ALC_API __declspec(dllexport)
#else
#	define ALC_API __declspec(dllimport)
#endif
#define ALC_APIENTRY __cdecl


#pragma region Definitions
/// <summary>Indicates that the referenced OpenAL library supports the 1.0 OpenAL ALC APIs.</summary>
#define ALC_VERSION_1_0

#pragma endregion


#pragma region Types
/// <summary>8-bit boolean type.</summary>
typedef int8_t ALCboolean;

/// <summary>8-bit character type.</summary>
typedef char ALCchar;

/// <summary>Signed 8-bit integer type.</summary>
typedef int8_t ALCbyte;

/// <summary>Unsigned 8-bit integer type.</summary>
typedef uint8_t ALCubyte;

/// <summary>Signed 16-bit integer type.</summary>
typedef int16_t ALCshort;

/// <summary>Unsigned 16-bit integer type.</summary>
typedef uint16_t ALCushort;

/// <summary>Signed 32-bit integer type.</summary>
typedef int32_t ALCint;

/// <summary>Unsigned 32-bit integer type.</summary>
typedef uint32_t ALCuint;

/// <summary>Unsigned 32-bit integer type.</summary>
typedef uint32_t ALCsizei;

/// <summary>32-bit IEEE754 single precision float.</summary>
typedef float ALCfloat;

/// <summary>32-bit IEEE754 double precision float.</summary>
typedef double ALCdouble;

/// <summary>Void type used to pass void* style pointers to "al" functions.</summary>
typedef void ALCvoid;

typedef void ALCdevice;

typedef void ALCcontext;

#pragma endregion


#pragma region Constants
/// <summary>Indicates that nothing was set.</summary>
#define ALC_NONE 0

/// <summary>The <b>false</b> value assignable to OpenAL's ALCboolean type.</summary>
#define ALC_FALSE ((ALCboolean)0)

/// <summary>The <b>true</b> value assignable to OpenAL's ALCboolean type.</summary>
#define ALC_TRUE ((ALCboolean)1)

/// <summary>OpenAL parameter type used by the library's "alc" functions.</summary>
typedef enum
{
	ALC_NO_ERROR = 0,
	ALC_FREQUENCY = 0x1007,
	ALC_REFRESH = 0x1008,
	ALC_SYNC = 0x1009,
	ALC_MONO_SOURCES = 0x1010,
	ALC_STEREO_SOURCES = 0x1011
} ALCenum;

#pragma endregion



ALC_API ALCcontext* ALC_APIENTRY alcCreateContext(ALCdevice *devicePointer, const ALCint *attributeArray);
ALC_API ALCboolean ALC_APIENTRY alcMakeContextCurrent(ALCcontext *contextPointer);
ALC_API void ALC_APIENTRY alcProcessContext(ALCcontext *contextPointer);
ALC_API void ALC_APIENTRY alcSuspendContext(ALCcontext *contextPointer);
ALC_API void ALC_APIENTRY alcDestroyContext(ALCcontext *contextPointer);
ALC_API ALCcontext* ALC_APIENTRY alcGetCurrentContext(void);
ALC_API ALCdevice* ALC_APIENTRY alcGetContextsDevice(ALCcontext *contextPointer);
ALC_API ALCenum ALC_APIENTRY alcGetError(ALCdevice *devicePointer);
ALC_API ALCdevice* ALC_APIENTRY alcOpenDevice(const ALCchar *deviceName);
ALC_API ALCboolean ALC_APIENTRY alcCloseDevice(ALCdevice *devicePointer);
ALC_API ALCboolean ALC_APIENTRY alcIsExtensionPresent(ALCdevice *devicePointer, const ALCchar *extensionName);
ALC_API void* ALC_APIENTRY alcGetProcAddress(ALCdevice *devicePointer, const ALCchar *functionName);
ALC_API ALCenum ALC_APIENTRY alcGetEnumValue(ALCdevice *devicePointer, const ALCchar *enumName);
ALC_API const ALCchar* ALC_APIENTRY alcGetString(ALCdevice *devicePointer, ALCenum parameter);
ALC_API void ALC_APIENTRY alcGetIntegerv(ALCdevice *devicePointer, ALCenum parameter, ALCsizei numberOfValues, ALCint *valueArray);


#ifdef __cplusplus
}	// extern "C"
#endif
