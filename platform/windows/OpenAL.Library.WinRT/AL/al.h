/* ----------------------------------------------------------------------------
//
// al.h
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
#	define AL_API __declspec(dllexport)
#else
#	define AL_API __declspec(dllimport)
#endif
#define AL_APIENTRY __cdecl


#pragma region Definitions
/// <summary>Indicates that the OpenAL library's "al.h" was included.</summary>
/// <remarks>This is a decprecated macro that is included for OpenAL 1.0 backward compatibility.</remarks>
#define OPENAL

/// <summary>Indicates that the referenced OpenAL library supports the 1.0 OpenAL APIs.</summary>
#define AL_VERSION_1_0

/// <summary>Indicates that the referenced OpenAL library supports the 1.1 OpenAL APIs.</summary>
#define AL_VERSION_1_1

#pragma endregion


#pragma region Types
/// <summary>8-bit boolean type.</summary>
typedef int8_t ALboolean;

/// <summary>8-bit character type.</summary>
typedef char ALchar;

/// <summary>Signed 8-bit integer type.</summary>
typedef int8_t ALbyte;

/// <summary>Unsigned 8-bit integer type.</summary>
typedef uint8_t ALubyte;

/// <summary>Signed 16-bit integer type.</summary>
typedef int16_t ALshort;

/// <summary>Unsigned 16-bit integer type.</summary>
typedef uint16_t ALushort;

/// <summary>Signed 32-bit integer type.</summary>
typedef int32_t ALint;

/// <summary>Unsigned 32-bit integer type.</summary>
typedef uint32_t ALuint;

/// <summary>Unsigned 32-bit integer type.</summary>
typedef uint32_t ALsizei;

/// <summary>32-bit IEEE754 single precision float.</summary>
typedef float ALfloat;

/// <summary>32-bit IEEE754 double precision float.</summary>
typedef double ALdouble;

/// <summary>Void type used to pass void* style pointers to "al" functions.</summary>
typedef void ALvoid;

#pragma endregion


#pragma region Constants
/// <summary>Indicates that nothing was set.</summary>
#define AL_NONE 0

/// <summary>The <b>false</b> value assignable to OpenAL's ALboolean type.</summary>
#define AL_FALSE ((int8_t)0)

/// <summary>The <b>true</b> value assignable to OpenAL's ALboolean type.</summary>
#define AL_TRUE ((int8_t)1)

/// <summary>OpenAL parameter type used by the library's "al" functions.</summary>
typedef enum
{
	/// <summary>
	///  <para>
	///   ALenum parameter type used to get or set whether or not an audio source's position is relative to the
	///   listener's position or is using absolute coordinates.
	///  </para>
	///  <para>If set to AL_TRUE, then the source uses relative coordinates.</para>
	///  <para>This parameter defaults to AL_FALSE.</para>
	///  <para>This parameter's value type is an ALboolean.</para>
	/// </summary>
	AL_SOURCE_RELATIVE = 0x202,

	/// <summary>
	///  <para>
	///   ALenum parameter type used to get or set the directional inner cone angle of an audio source where it
	///   will not be attenuated.
	///  </para>
	///  <para>This value is set in degrees, ranging from 0 to 360. This parameter defaults to 360 degrees.</para>
	/// </summary>
	AL_CONE_INNER_ANGLE = 0x1001,

	/// <summary>
	///  <para>
	///   ALenum parameter type used to get or set the directional outer cone angle of an audio source where the
	///   source will be fully attenuated.
	///  </para>
	///  <para>This parameter is set in degrees, ranging from 0 to 360. This parameter defaults to 360 degrees.</para>
	/// </summary>
	AL_CONE_OUTER_ANGLE = 0x1002,

	/// <summary>
	///  <para>ALenum parameter type used to get or set the pitch of an audio source or the audio mixer's result.</para>
	///  <para>This parameter ranges from 0.5 to 2.0. This parameter defaults to 1.0.</para>
	///  <para>This parameter's value type is an ALfloat.<para>
	/// </summary>
	AL_PITCH = 0x1003,

	/// <summary>
	///  <para>ALenum parameter type used to get or set the position of an audio source or listener in 3D space.</para>
	///  <para>Position is set in (x,y,z) coordinates, like OpenGL, using the right handed coordinate system.</para>
	///  <para>- The x-axis heads right in the positive direction (your thumb).</para>
	///  <para>- The y-axis heads up in the positive direction (your index finger).</para>
	///  <para>- The z-axis heads towards the viewer/camera in the positive direction (your middle finger).</para>
	///  <para>This parameter's default is (0,0,0).</para>
	///  <para>The parameter's value type can be an ALfloat[3] or ALint[3] array.</para>
	/// </summary>
	AL_POSITION = 0x1004,

	/// <summary>
	///  <para>ALenum paramter type used to get or set the direction the audio source or listener is facing in 3D space.</para>
	///  <para>
	///   This parameter's value is set to an &lt;x,y,z&gt; vector. A vector of &lt;0,0,0&gt; indicates that the audio
	///   source has no direction, meaning it is omni-directional and can be heard in all directions. The inner and outer
	///   cone parameters do not apply if the audio source is omni-directional.
	///  </para>
	///  <para>This parameter's default is &lt;0,0,0&gt;.</para>
	///  <para>The parameter's value type can be an ALfloat[3] or ALint[3] array.</para>
	/// </summary>
	AL_DIRECTION = 0x1005,

	/// <summary>
	///  <para>ALenum paramter type used to get or set the velocity of the audio source or listener in 3D space.</para>
	///  <para>Velocity is set as an &lt;x,y,z&gt; vector and is used to apply doppler effects.</para>
	///  <para>This parameter's default is &lt;0,0,0&gt;, which means it is not moving.</para>
	///  <para>The parameter's value type can be an ALfloat[3] or ALint[3] array.</para>
	/// </summary>
	AL_VELOCITY = 0x1006,

	/// <summary>
	///  <para>ALenum parameter type used to get or set whether or not a source should loop its audio.</para>
	///  <para>This parameter defaults to AL_FALSE.</para>
	///  <para>This parameter's value type is an ALboolean.</para>
	/// </summary>
	AL_LOOPING = 0x1007,

	/// <summary>
	///  <para>ALenum parameter type used to get or set an audio source's buffer ID.</para>
	///  <para>
	///   This parameter's value type is an ALuint integer ID which references a byte buffer used to store audio samples.
	///   This buffer integer ID is acquired via the alGenBuffers() function.
	///  </para>
	/// </summary>
	AL_BUFFER = 0x1009,

	/// <summary>
	///  <para>ALenum parameter type used to get or set the volume level of an audio source or listener.</para>
	///  <para>This parameter's value must be greater than or equal to zero.</para>
	///  <para>A value of 1.0 means the audio is un-attenuated (normal volume level).</para>
	///  <para>A value of 0.0 mutes the audio.</para>
	///  <para>Each division by 2 equals an attenuation (decrease in volume) of about -6dB.</para>
	///  <para>Each multiplication by 2 equals an amplification (increase in volume) of about +6dB.</para>
	///  <para>This parameter's value type is an ALfloat.</para>
	/// </summary>
	AL_GAIN = 0x100A,

	/// <summary>
	///  <para>ALenum parameter type used to get or set the minimum value an audio source's gain can have applied to it.</para>
	///  <para>This minimum takes affect after distance and cone attenuation have been applied.</para>
	///  <para>This parameter is a logarithmic value ranging from 0.0 to 1.0.</para>
	///  <para>This parameter's value type is an ALfloat.</para>
	/// </summary>
	AL_MIN_GAIN = 0x100D,

	/// <summary>
	///  <para>ALenum parameter type used to get or set the maximum value an audio source's gain can have applied to it.</para>
	///  <para>This maximum takes affect after distance and cone attenuation have been applied.</para>
	///  <para>This parameter is a logarithmic value ranging from 0.0 to 1.0.</para>
	///  <para>This parameter's value type is an ALfloat.</para>
	/// </summary>
	AL_MAX_GAIN = 0x100E,

	/// <summary>
	///  <para>ALenum parameter type used to get or set the orientation of the listener in 3D space.</para>
	///  <para>This parameter is made up of two 3-dimensional vectors named the "at" and "up" vectors.</para>
	///  <para>The "at" vector represents the direction the listener is facing.</para>
	///  <para>
	///   The "up" vector represents which direction is up in respect to the listener.
	///   For example, this is the direction the top of a person's heading.
	///  </para>
	///  <para>
	///   This parameter's value type is an ALflot[6] where the first 3 elements in the array is the &lt;x,y,z&gt; "at"
	///   vector and the last 3 elements in the array is the &lt;x,y,z&gt; "up" vector.
	///  </para>
	/// </summary>
	AL_ORIENTATION = 0x100F,

	/// <summary>
	///  <para>ALenum parameter type used to retrieve the current state of an audio source.</para>
	///  <para>This parameter is used for retrieval only and cannot be passed to OpenAL's setter functions.</para>
	///  <para>The possible states are AL_INITIAL, AL_PLAYING, AL_PAUSED, and AL_STOPPED.</para>
	///  <para>The parameter's value type is an ALint.</para>
	/// </summary>
	AL_SOURCE_STATE = 0x1010,

	/// <summary>
	///  <para>
	///   ALenum parameter type used to retrieve the number of buffers currently queued via the alSourceQueueBuffers() function,
	///   excluding the buffers removed from the queue via the alSourceUnqueueBuffers() function.
	///  </para>
	///  <para>This parameter is used for retrieval only and cannot be passed to OpenAL's setter functions.</para>
	///  <para>The parameter's value type is an ALint.</para>
	/// </summary>
	AL_BUFFERS_QUEUED = 0x1015,

	/// <summary>
	///  <para>
	///   ALenum parameter type used to retrieve the number of queued buffers that have been processed by the audio system
	///   and can now be removed via the alSourceUnqueueBuffers() function.
	///  </para>
	///  <para>Looping audio sources will never be flagged as "processed".</para>
	///  <para>This parameter is used for retrieval only and cannot be passed to OpenAL's setter functions.</para>
	///  <para>The parameter's value type is an ALint.</para>
	/// </summary>
	AL_BUFFERS_PROCESSED = 0x1016,

	/// <summary>
	///  <para>
	///   ALenum parameter type used to get or set how far the listener must be from the audio source until the listener
	///   hears sound. This makes it so that the farther the listener moves from the source, the harder it is to hear
	///   (lower volume). The closer the listener gets to the source, the louder the source can be heard.
	///  </para>
	///  <para>The parameter's value must be greater than or equal to zero.</para>
	///  <para>
	///   A value of 0.0 sets up the audio source to not apply any distance attenuation (volume level) to the sound it emits.
	///  </para>
	///  <para>The parameter's value type is an ALfloat.</para>
	/// </summary>
	AL_REFERENCE_DISTANCE = 0x1020,

	/// <summary>
	///  <para>ALenum parameter type used to get or set the factor applied to the AL_REFERENCE_DISTANCE.</para>
	///  <para>
	///   The parameter must greater than or equal to zero. This roll-off factor is used as a multiplier and is used
	///   to adjust the attenuation (volume level) in a non-linear fashion based on the audio sources distance.
	///  </para>
	///  <para>A roll-off factor of 1.0 means linear audio attenuation will be applied to the audio source.</para>
	///  <para>A roll-off factor of 0.0 means no distance attenuation will be applied to the audio source.</para>
	///  <para>This parameter defaults to 1.0.</para>
	///  <para>The parameter's value type is an ALfloat.</para>
	/// </summary>
	AL_ROLLOFF_FACTOR = 0x1021,

	/// <summary>
	///  <para>
	///   ALenum parameter type used to get or set the volume level attenuation when the listener is not within
	///   the audio source's direction.
	///  </para>
	///  <para>This parameter ranges between 0.0 and 1.0.</para>
	///  <para>The parameter defaults to 0.0.</para>
	///  <para>The parameter's value type is an ALfloat.</para>
	/// </summary>
	AL_CONE_OUTER_GAIN = 0x1022,

	/// <summary>
	///  <para>ALenum parameter type used to get or set the maximum distance an audio source can be heard from.</para>
	///  <para>This parameter's value must be greater than or equal to zero.</para>
	///  <para>The parameter defaults to positive infinity, which means no maximum is applied.</para>
	///  <para>The parameter's value type is an ALfloat.</para>
	/// </summary>
	AL_MAX_DISTANCE = 0x1023,

	/// <summary>
	///  <para>ALenum parameter type used to determine if the audio source is streaming audio.</para>
	///  <para>This parameter is used for retrieval only and cannot be passed to OpenAL's setter functions.</para>
	///  <para>Will be AL_STATIC if the entire audio buffer has been loaded.</para>
	///  <para>
	///   Will be AL_STREAMING if the entire audio buffer has not been loaded into memory. In this case, the system
	///   is expected to queue streamed in audio buffers to this library via the alSourceQueueBuffers() function.
	///  </para>
	///  <para>Will be AL_UNDETERMINED if the audio source was assigned a NULL buffer (no sound).</para>
	///  <para>The parameter's value type is an ALint.</para>
	/// </summary>
	AL_SOURCE_TYPE = 0x1027,

	/// <summary>
	///  <para>ALenum parameter type used to retrieve the frequency of the audio in Hertz, such as 44100 Hz.</para>
	///  <para>This parameter is used for retrieval only and cannot be passed to OpenAL's setter functions.</para>
	/// </summary>
	AL_FREQUENCY = 0x2001,

	/// <summary>
	///  <para>ALenum parameter type used to retrieve the bit quality of the audio, such as 16-bit or 32-bit.</para>
	///  <para>This parameter is used for retrieval only and cannot be passed to OpenAL's setter functions.</para>
	/// </summary>
	AL_BITS = 0x2002,

	/// <summary>
	///  <para>
	///   ALenum parameter type used to retrieve the number of channels the audio, such as 1 for mono and 2 for stereo.
	///  </para>
	///  <para>This parameter is used for retrieval only and cannot be passed to OpenAL's setter functions.</para>
	/// </summary>
	AL_CHANNELS = 0x2003,

	/// <summary>
	///  <para>ALenum parameter type used to retrieve the number of bytes an audio buffer uses.</para>
	///  <para>This parameter is used for retrieval only and cannot be passed to OpenAL's setter functions.</para>
	/// </summary>
	AL_SIZE = 0x2004,

	/// <summary>
	///  <para>
	///   ALenum parameter type used to retrieve the name of the company that created the OpenAL library that is
	///   currently in use.
	///  </para>
	///  <para>This parameter is used for retrieval only and cannot be passed to OpenAL's setter functions.</para>
	///  <para>This parameter's value is of type string.</para>
	/// </summary>
	AL_VENDOR = 0xB001,

	/// <summary>
	///  <para>
	///   ALenum parameter type used to retrieve the version string assigned to the OpenAL library that is currently in use.
	///  </para>
	///  <para>This parameter is used for retrieval only and cannot be passed to OpenAL's setter functions.</para>
	///  <para>This parameter's value is of type string.</para>
	/// </summary>
	AL_VERSION = 0xB002,

	/// <summary>
	///  <para>
	///   ALenum parameter type used to retrieve the name of the audio renderer used by the OpenAL library that is
	///   currently in use.
	///  </para>
	///  <para>This parameter is used for retrieval only and cannot be passed to OpenAL's setter functions.</para>
	///  <para>This parameter's value is of type string.</para>
	/// </summary>
	AL_RENDERER = 0xB003,

	/// <summary>
	///  <para>
	///   ALenum parameter type used to retrieve a list of space separated extension names supported by the OpenAL
	///   library that is currently in use as a single string.
	///  </para>
	///  <para>This parameter is used for retrieval only and cannot be passed to OpenAL's setter functions.</para>
	///  <para>This parameter's value is of type string.</para>
	/// </summary>
	AL_EXTENSIONS = 0xB004,

	/// <summary>
	///  <para>ALenum parameter type used to get or set a global scale applied to all velocities.</para>
	///  <para>This parameter can also be set via the alDopplerFactor() function.</para>
	///  <para>This parameter must be set to greater than or equal to zero.</para>
	///  <para>This parameter defaults to 1.0, which means the velocities are not scaled.</para>
	///  <para>This parameter's value is of type ALfloat.</para>
	/// </summary>
	AL_DOPPLER_FACTOR = 0xC000,

	/// <summary>
	///  <para>
	///   ALenum parameter type used to get or set a global scale applied to the speed of sound when calculating the
	///   doppler effect for all audio sources.
	///  </para>
	///  <para>This parameter can also be set via the alDopplerVelocity() function.</para>
	///  <para>This parameter is deprecated. You should use the AL_SPEED_OF_SOUND parameter instead.</para>
	/// </summary>
	AL_DOPPLER_VELOCITY = 0xC001,

	/// <summary>
	///  <para>
	///   ALenum parameter type used to get or set the speed of sound to be applied to all velocities when calculating
	///   the doppler effect.
	///  </para>
	///  <para>This parameter can also be set via the alSpeedOfSound() function.</para>
	///  <para>This parameter must be 0.0001 meters per second or higher.</para>
	///  <para>This parameter defaults to 343.3 meter per second.</para>
	///  <para>This parameter's value is of type ALfloat.</para>
	/// </summary>
	AL_SPEED_OF_SOUND = 0xC003,

	/// <summary>
	///  <para>ALenum parameter type used to get or set the distance attenuation model to be applied to all sources.</para>
	///  <para>This parameter can also be set via the alDistanceModel() function.</para>
	///  <para>Set to AL_NONE to have no distance attenuation applied.</para>
	///  <para>Set to AL_LINEAR_DISTANCE for a linear gain scaling between the reference and max distances.</para>
	///  <para>
	///   Set to AL_LINEAR_DISTANCE_CLAMPED for a linear gain scaling between the reference and max distances.
	///   The gain cannot exceeded those two distances.
	///  </para>
	///  <para>Set to AL_EXPONENT_DISTANCE for an exponential gain drop off between the reference and max distances.</para>
	///  <para>
	///   Set to AL_EXPONENT_DISTANCE_CLAMPED for an exponential gain drop off between the reference and max distances.
	///   The gain cannot exceeded those two distances.
	///  </para>
	///  <para>Set to AL_INVERSE_DISTANCE where doubling the distance will halve the souce gain.</para>
	///  <para>
	///   Set to AL_INVERSE_DISTANCE_CLAMPED where doubling the distance will halve the souce gain.
	///   The gain cannot exceeded the reference and max distances.
	///  </para>
	///  <para>This parameter defaults to AL_INVERSE_DISTANCE_CLAMPED.</para>
	///  <para>This parameter's value is of type ALint.</para>
	/// </summary>
	AL_DISTANCE_MODEL = 0xD000,

	/// <summary>Indicate that no error occurred. The operation succeeded.</summary>
	AL_NO_ERROR = 0,

	/// <summary>Indicates that an invalid name/string parameter was provided.</summary>
	AL_INVALID_NAME = 0xA001,

	/// <summary>Indicates that an invalid ALenum constant was provided for the getter or setter's value type.</summary>
	AL_INVALID_ENUM = 0xA002,

	/// <summary>Indicates the an invalid value parameter was provided.</summary>
	AL_INVALID_VALUE = 0xA003,

	/// <summary>Indicates that an OpenAL function was misused.</summary>
	AL_INVALID_OPERATION = 0xA004,

	/// <summary>Indicates than there was not enough memory to complete operation.</summary>
	AL_OUT_OF_MEMORY = 0xA005
} ALenum;


/// <summary>Audio playback states.</summary>
enum
{
	/// <summary>Indicates that an audio source is at the beginning of its audio track and is not playing.</summary>
	AL_INITIAL = 0x1011,

	/// <summary>Indicates that an audio source is currently playing audio.</summary>
	AL_PLAYING = 0x1012,

	/// <summary>Indicates that an audio source has paused playback.</summary>
	AL_PAUSED = 0x1013,

	/// <summary>Indicates that an audio source has stopped playback.</summary>
	AL_STOPPED = 0x1014,
};


/// <summary>Units used to indicate the audio positive such as in bytes, seconds, or frames.</summary>
enum
{
	/// <summary>Indicates that the audio source's position in the buffer is provided in seconds.</summary>
	AL_SEC_OFFSET = 0x1024,

	/// <summary>Indicates that the audio source's position in the buffer is provided in frames.</summary>
	AL_SAMPLE_OFFSET = 0x1025,

	/// <summary>Indicates that the audio source's position in the buffer is provided in bytes.</summary>
	AL_BYTE_OFFSET = 0x1026
};


/// <summary>Indicates the type of audio buffer assigned to an audio source.</summary>
enum
{
	/// <summary>Indicates that the entire audio buffer was loaded into memory.</summary>
	AL_STATIC = 0x1028,

	/// <summary>
	///  <para>
	///   Indicates that entire audio buffer has not been loaded into memory. In this case, the system is
	///   expected to queue streamed in audio buffers to this library via the alSourceQueueBuffers() function.
	///  </para>
	///  <para>This source type is appropriate when playing very large audio files, such as music or recordings.</para>
	/// </summary>
	AL_STREAMING = 0x1029,

	/// <summary>Indicates that no audio buffers (no sound) has been assigned to the audio source.</summary>
	AL_UNDETERMINED = 0x1030
};


/// <summary>The audio buffer's format.</summary>
enum
{
	/// <summary>1 channel 8-bit sound format.</summary>
	AL_FORMAT_MONO8 = 0x1100,

	/// <summary>1 channel 16-bit sound format.</summary>
	AL_FORMAT_MONO16 = 0x1101,

	/// <summary>2 channel 8-bit sound format.</summary>
	AL_FORMAT_STEREO8 = 0x1102,

	/// <summary>2 channel 16-bit sound format.</summary>
	AL_FORMAT_STEREO16 = 0x1103,
};


/// <summary>Indicates an audio buffer's current usage state.</summary>
enum
{
	/// <summary>Indicates that the audio buffer is not currently being used yet.</summary>
	/// <remarks>This constant is not for public use.</remarks>
	AL_UNUSED = 0x2010,

	/// <summary>Indicates thta the audio buffer is assigned to an audio source, but not used yet.</summary>
	/// <remarks>This constant is not for public use.</remarks>
	AL_PENDING = 0x2011,

	/// <summary>Indicates that the audio buffer is currently being played.</summary>
	/// <remarks>This constant is not for public use.</remarks>
	AL_PROCESSED = 0x2012,
};


/// <summary>Distance model types for calculating attenuation/volume.</summary>
enum
{
	/// <summary>Doubling the distance between the reference and max distances will halve the source gain.</summary>
	AL_INVERSE_DISTANCE = 0xD001,
	
	/// <summary>
	///  <para>Doubling the distance between the reference and max distances will halve the source gain.</para>
	///  <para>The gain cannot exceeded the reference and max distances.</para>
	/// </summary>
	AL_INVERSE_DISTANCE_CLAMPED = 0xD002,
	
	/// <summary>Linear gain scaling between the reference and max distances.</summary>
	AL_LINEAR_DISTANCE = 0xD003,
	
	/// <summary>
	///  <para>Linear gain scaling between the reference and max distances.</para>
	///  <para>The gain cannot exceeded the reference and max distances.</para>
	/// </summary>
	AL_LINEAR_DISTANCE_CLAMPED = 0xD004,
	
	/// <summary>Exponential gain drop off between the reference and max distances.</summary>
	AL_EXPONENT_DISTANCE = 0xD005,
	
	/// <summary>
	///  <para>Exponential gain drop off between the reference and max distances.</para>
	///  <para>The gain cannot exceeded the reference and max distances.</para>
	/// </summary>
	AL_EXPONENT_DISTANCE_CLAMPED = 0xD006,
};

#pragma endregion


#pragma region Deprecated OpenAL 1.0 Definitions
/// <summary>This is a decprecated macro that is included for OpenAL 1.0 backward compatibility.</summary>
#define ALAPI AL_API

/// <summary>This is a decprecated macro that is included for OpenAL 1.0 backward compatibility.</summary>
#define ALAPIENTRY AL_APIENTRY

/// <summary>Constant used to indicate a failure.</summary>
/// <remarks>This is a decprecated macro that is included for OpenAL 1.0 backward compatibility.</remarks>
#define AL_INVALID (-1)

/// <summary>Indicates that an invalid constant was passed into an "al" function.</summary>
/// <remarks>
///  This is a decprecated macro that is included for OpenAL 1.0 backward compatibility.
///  You should use the "AL_INVALID_ENUM" constant instead.
/// </remarks>
#define AL_ILLEGAL_ENUM AL_INVALID_ENUM

/// <summary>Indicates that an "al" function was called in an incorrect manner.</summary>
/// <remarks>
///  This is a decprecated macro that is included for OpenAL 1.0 backward compatibility.
///  You should use the "AL_INVALID_OPERATION" constant instead.
/// </remarks>
#define AL_ILLEGAL_COMMAND AL_INVALID_OPERATION

#pragma endregion


#pragma region Functions
/// <summary>Enables the given feature for the current audio context.</summary>
/// <param name="capability">The feature to enable, such as AL_SOURCE_DISTANCE_MODEL.</param>
AL_API void AL_APIENTRY alEnable(ALenum capability);

/// <summary>Disables the given feature for the current audio context.</summary>
/// <param name="capability">The feature to disable, such as AL_SOURCE_DISTANCE_MODEL.</param>
AL_API void AL_APIENTRY alDisable(ALenum capability);

/// <summary>Determines if the given feature is enabled for the current audio context.</summary>
/// <param name="capability">The feature to check, such as AL_SOURCE_DISTANCE_MODEL.</param>
/// <returns>Returns AL_TRUE if enabled. Returns AL_FALSE if disabled.</returns>
AL_API ALboolean AL_APIENTRY alIsEnabled(ALenum capability);

/// <summary>Sets the scale to be applied to all audio source/listener velocities for the current audio context.</summary>
/// <remarks>The alGetError() function will return AL_INVALID_VALUE if the given value is less than zero.</remarks>
/// <param name="value">
///  <para>Must be set to greater than or equal to zero.</para>
///  <para>A value of 1.0 (the default) means the velocities are not scaled.</para>
/// </param>
AL_API void AL_APIENTRY alDopplerFactor(ALfloat value);

/// <summary>
///  <para>Sets a global scale to be applied to the speed of sound for the current audio context.</para>
///  <para>This function is deprecated. You should use the alSpeedOfSound() function instead.</para>
/// </summary>
/// <remarks>The alGetError() function will return AL_INVALID_VALUE if the given value is less than zero.</remarks>
/// <param name="value">
///  <para>Must be set to greater than or equal to zero.</para>
///  <para>A value of 1.0 (the default) means the speed of sound will not be scaled.</para>
/// </param>
AL_API void AL_APIENTRY alDopplerVelocity(ALfloat value);

/// <summary>
///  <para>Changes the speed of sound from the default of 343.3 meters per second for the current audio context.</para>
///  <para>This value is applied to all velocities when calculating doppler effects.</para>
/// </summary>
/// <remarks>The alGetError() function will return AL_INVALID_VALUE if the given value is less than or equal to zero.</remarks>
/// <param name="value">This value is in meters per second and must be greater than zero.</param>
AL_API void AL_APIENTRY alSpeedOfSound(ALfloat value);

/// <summary>
///  <para>Sets the distance attenuation model to be applied to all sources for the current audio context.</para>
///  <para>A newly created audio context defaults to an AL_INVERSE_DISTANCE_CLAMPED model.</para>
/// </summary>
/// <remarks>The alGetError() function will return AL_INVALID_VALUE if the given model type is invalid.</remarks>
/// <param name="modelType">
///  <para>Set to AL_NONE to have no distance attenuation applied.</para>
///  <para>Set to AL_LINEAR_DISTANCE for a linear gain scaling between the reference and max distances.</para>
///  <para>
///   Set to AL_LINEAR_DISTANCE_CLAMPED for a linear gain scaling between the reference and max distances.
///   The gain cannot exceeded those two distances.
///  </para>
///  <para>Set to AL_EXPONENT_DISTANCE for an exponential gain drop off between the reference and max distances.</para>
///  <para>
///   Set to AL_EXPONENT_DISTANCE_CLAMPED for an exponential gain drop off between the reference and max distances.
///   The gain cannot exceeded those two distances.
///  </para>
///  <para>Set to AL_INVERSE_DISTANCE where doubling the distance will halve the souce gain.</para>
///  <para>
///   Set to AL_INVERSE_DISTANCE_CLAMPED where doubling the distance will halve the souce gain.
///   The gain cannot exceeded the reference and max distances.
///  </para>
/// </param>
AL_API void AL_APIENTRY alDistanceModel(ALenum modelType);

/// <summary>Gets a string constant from the given parameter.</summary>
/// <param name="parameter">The parameter to get a string for, such as AL_VENDOR, AL_VERSION, or AL_RENDERER.</param>
/// <returns>
///  <para>Returns the given parameter's string constant.</para>
///  <para>
///   Returns null if given a parameter that does not have a value of type string.
///   The alGetError() function will return AL_INVALID_ENUM in this case.
///  </para>
/// </returns>
AL_API const ALchar* AL_APIENTRY alGetString(ALenum parameter);

/// <summary>Gets a boolean flag from the given parameter for the current audio context.</summary>
/// <param name="parameter">The parameter to get a boolean for, such as AL_DISTANCE_MODEL.</param>
/// <returns>
///  <para>Returns the the AL_TRUE or AL_FALSE value belonging to the given parameter.</para>
///  <para>
///   The alGetError() function will return AL_INVALID_ENUM if given a parameter that does not provide a boolean value.
///  </para>
/// </returns>
AL_API ALboolean AL_APIENTRY alGetBoolean(ALenum parameter);

/// <summary>Gets a 32-bit signed integer value from the given parameter for the current audio context.</summary>
/// <param name="parameter">The parameter to get data from, such as AL_DOPPLER_FACTOR or AL_DOPPLER_VELOCITY.</param>
/// <returns>
///  <para>Returns the 32-bit signed integer value belonging to the given parameter.</para>
///  <para>
///   The alGetError() function will return AL_INVALID_ENUM if given a parameter that does not provide an integer value.
///  </para>
/// </returns>
AL_API ALint AL_APIENTRY alGetInteger(ALenum parameter);

/// <summary>Gets a single precision floating point value from the given parameter for the current audio context.</summary>
/// <param name="parameter">The parameter to get data from, such as AL_DOPPLER_FACTOR or AL_DOPPLER_VELOCITY.</param>
/// <returns>
///  <para>Returns the single precision floating point value belonging to the given parameter.</para>
///  <para>
///   The alGetError() function will return AL_INVALID_ENUM if given a parameter that does not provide a float.
///  </para>
/// </returns>
AL_API ALfloat AL_APIENTRY alGetFloat(ALenum parameter);

/// <summary>Gets a double precision floating point value from the given parameter for the current audio context.</summary>
/// <param name="parameter">The parameter to get data from, such as AL_DOPPLER_FACTOR or AL_DOPPLER_VELOCITY.</param>
/// <returns>
///  <para>Returns the double precision floating point value belonging to the given parameter.</para>
///  <para>
///   The alGetError() function will return AL_INVALID_ENUM if given a parameter that does not provide a double float.
///  </para>
/// </returns>
AL_API ALdouble AL_APIENTRY alGetDouble(ALenum parameter);

/// <summary>Gets a boolean flag from the given parameter from the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter does not provide a boolean value.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if "valuePointer" is null.</para>
/// </remarks>
/// <param name="parameter">The parameter to get a boolean for, such as AL_DISTANCE_MODEL.</param>
/// <param name="valuePointer">Pointer to a boolean that the parameter's value will be copied to.</param>
AL_API void AL_APIENTRY alGetBooleanv(ALenum parameter, ALboolean *valuePointer);

/// <summary>Gets a 32-bit signed integer value from the given parameter for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter does not provide integer data.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if "valuePointer" is null.</para>
/// </remarks>
/// <param name="parameter">The parameter to get data from, such as AL_DOPPLER_FACTOR or AL_DOPPLER_VELOCITY.</param>
/// <param name="valuePointer">Pointer to an integer that the parameter's value will be copied to.</param>
AL_API void AL_APIENTRY alGetIntegerv(ALenum parameter, ALint *valuePointer);

/// <summary>Gets a single precision floating point value from the given parameter for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter does not provide float data.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if "valuePointer" is null.</para>
/// </remarks>
/// <param name="parameter">The parameter to get data from, such as AL_DOPPLER_FACTOR or AL_DOPPLER_VELOCITY.</param>
/// <param name="valuePointer">Pointer to a single precision float that the parameter's value will be copied to.</param>
AL_API void AL_APIENTRY alGetFloatv(ALenum parameter, ALfloat *valuePointer);

/// <summary>Gets a double precision floating point value from the given parameter for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter does not provide float data.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if "valuePointer" is null.</para>
/// </remarks>
/// <param name="parameter">The parameter to get data from, such as AL_DOPPLER_FACTOR or AL_DOPPLER_VELOCITY.</param>
/// <param name="valuePointer">Pointer to a double precision float that the parameter's value will be copied to.</param>
AL_API void AL_APIENTRY alGetDoublev(ALenum parameter, ALdouble *valuePointer);

/// <summary>
///  <para>Gets the error code generated by the last OpenAL function function call.</para>
///  <para>Calling this function will reset the error code back to AL_NO_ERROR.</para>
/// </summary>
/// <returns>
///  <para>Returns AL_NO_ERROR if no error occurred.</para>
///  <para>Returns AL_INVALID_NAME if an invalid string was provided with a parameter.</para>
///  <para>Returns AL_INVALID_ENUM if an ALenum parameter was given to a function that doesn't support it.</para>
///  <para>Returns AL_INVALID_VALUE if an invalid argument was given to a function.</para>
///  <para>Returns AL_INVALID_OPERATION if a function calls were made in the wrong order, putting the system in a bad state.</para>
///  <para>Returns AL_OUT_OF_MEMORY if there was not enough memory to complete the function's operation.</para>
/// </returns>
AL_API ALenum AL_APIENTRY alGetError(void);

/// <summary>Determines if the given OpenAL extension API exists for the currently loaded OpenAL library.</summary>
/// <param name="extensionName">Name of the extension, such as "AL_EXT_vorbis" for ogg vorbis decoding support.</param>
/// <returns>
///  <para>Returns AL_TRUE if the given extension is supported.</para>
///  <para>Returns AL_FALSE if not supported or if given an unknown name.</para>
/// </returns>
AL_API ALboolean AL_APIENTRY alIsExtensionPresent(const ALchar *extensionName);

/// <summary>Gets a function pointer for the given OpenAL function name.</summary>
/// <param name="functionName">Name of the function to get a callback for.</param>
/// <returns>
///  <para>Returns a pointer to the specified function.</para>
///  <para>Returns null if given an unknown function name or a null pointer.</para>
/// </returns>
AL_API void* AL_APIENTRY alGetProcAddress(const ALchar *functionName);

/// <summary>Gets the ALenum integer constant value for the equivalent ALenum constant name.</summary>
/// <remarks>The alGetError() function will return AL_INVALID_VALUE if given a null argument.</remarks>
/// <param name="enumName">Name of the ALenum constant.</param>
/// <returns>
///  <para>Returns the specified constant's integer value.</para>
///  <para>Returns zero if the given name is unknown or if given a null pointer.</para>
/// </returns>
AL_API ALenum AL_APIENTRY alGetEnumValue(const ALchar *enumName);

/// <summary>Sets the given parameter for the current audio context's one listener.</summary>
/// <remarks>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</remarks>
/// <param name="parameter">The parameter to be set, such as AL_GAIN or AL_METERS_PER_UNIT.</param>
/// <param name="value">The value to be applied to the listener's parmeter.</param>
AL_API void AL_APIENTRY alListenerf(ALenum parameter, ALfloat value);

/// <summary>Sets the given parameter for the current audio context's one listener.</summary>
/// <remarks>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</remarks>
/// <param name="parameter">The parameter to be set, such as AL_POSITION or AL_VELOCITY.</param>
/// <param name="value1">The X component value to be applied to the listener's parmeter.</param>
/// <param name="value2">The Y component value to be applied to the listener's parmeter.</param>
/// <param name="value3">The Z component value to be applied to the listener's parmeter.</param>
AL_API void AL_APIENTRY alListener3f(ALenum parameter, ALfloat value1, ALfloat value2, ALfloat value3);

/// <summary>Sets the given parameter for the current audio context's one listener.</summary>
/// <remarks>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</remarks>
/// <param name="parameter">
///  The parameter to be set, such as AL_GAIN, AL_METERS_PER_UNIT, AL_POSITION, or AL_VELOCITY.
/// </param>
/// <param name="valueArray">
///  <para>An array of values to be assigned to the parameter.</para>
///  <para>Must be an array of at least 3 elements {x,y,z} for parameters AL_POSITION and AL_VELOCITY.</para>
///  <para>Must be a pointer to at least 1 value for parameters AL_GAIN and AL_METERS_PER_UNIT.</para>
/// </param>
AL_API void AL_APIENTRY alListenerfv(ALenum parameter, const ALfloat *valueArray);

/// <summary>Sets the given parameter for the current audio context's one listener.</summary>
/// <remarks>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</remarks>
/// <param name="parameter">The parameter to be set. (None are currently supported at this time.)</param>
/// <param name="value">The value to be applied to the listener's parmeter.</param>
AL_API void AL_APIENTRY alListeneri(ALenum parameter, ALint value);

/// <summary>Sets the given parameter for the current audio context's one listener.</summary>
/// <remarks>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</remarks>
/// <param name="parameter">The parameter to be set, such as AL_POSITION or AL_VELOCITY.</param>
/// <param name="value1">The X component value to be applied to the listener's parmeter.</param>
/// <param name="value2">The Y component value to be applied to the listener's parmeter.</param>
/// <param name="value3">The Z component value to be applied to the listener's parmeter.</param>
AL_API void AL_APIENTRY alListener3i(ALenum parameter, ALint value1, ALint value2, ALint value3);

/// <summary>Sets the given parameter for the current audio context's one listener.</summary>
/// <remarks>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</remarks>
/// <param name="parameter">The parameter to be set, such as AL_POSITION, AL_VELOCITY, or AL_ORIENTATION.</param>
/// <param name="valueArray">
///  <para>An array of values to be assigned to the parameter.</para>
///  <para>Must be an array of at least 3 elements {x,y,z} for parameters AL_POSITION and AL_VELOCITY.</para>
///  <para>Must be an array of at least 6 elements {x1,y1,z1,x2,y2,z2} for the AL_ORIENTATION parameter.</para>
/// </param>
AL_API void AL_APIENTRY alListeneriv(ALenum parameter, const ALint *valueArray);

/// <summary>Gets the given parameter's value from the current audio context's one listener.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given value argument is null.</para>
/// </remarks>
/// <param name="parameter">The parameter to fetch, such as AL_GAIN or AL_METERS_PER_UNIT.</param>
/// <param name="valuePointer">Pointer to a float that the parameter's value will be copied to.</param>
AL_API void AL_APIENTRY alGetListenerf(ALenum parameter, ALfloat *valuePointer);

/// <summary>Gets the given parameter's values from the current audio context's one listener.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if any of the given value arguments are null.</para>
/// </remarks>
/// <param name="parameter">The parameter to fetch, such as AL_POSITION or AL_VELOCITY.</param>
/// <param name="valuePointer1">Pointer to a float that the parameter's X component value will be copied to.</param>
/// <param name="valuePointer2">Pointer to a float that the parameter's Y component value will be copied to.</param>
/// <param name="valuePointer3">Pointer to a float that the parameter's Z component value will be copied to.</param>
AL_API void AL_APIENTRY alGetListener3f(ALenum parameter, ALfloat *valuePointer1, ALfloat *valuePointer2, ALfloat *valuePointer3);

/// <summary>Gets the given parameter's values from the current audio context's one listener.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given value argument is null.</para>
/// </remarks>
/// <param name="parameter">The parameter to fetch, such as AL_GAIN, AL_METERS_PER_UNIT, AL_POSITION, or AL_VELOCITY.</param>
/// <param name="valueArray">
///  <para>An array that the parameter's values will be copied to.</para>
///  <para>Must be an array of at least 3 elements {x,y,z} for parameters AL_POSITION and AL_VELOCITY.</para>
///  <para>Must be a pointer to at least 1 value for parameters AL_GAIN and AL_METERS_PER_UNIT.</para>
/// </param>
AL_API void AL_APIENTRY alGetListenerfv(ALenum parameter, ALfloat *valueArray);

/// <summary>Gets the given parameter's value from the current audio context's one listener.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given value argument is null.</para>
/// </remarks>
/// <param name="parameter">The parameter to be set. (None are currently supported at this time.)</param>
/// <param name="valuePointer">Pointer to an integer that the parameter's value will be copied to.</param>
AL_API void AL_APIENTRY alGetListeneri(ALenum parameter, ALint *valuePointer);

/// <summary>Gets the given parameter's values from the current audio context's one listener.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if any of the given value arguments are null.</para>
/// </remarks>
/// <param name="parameter">The parameter to fetch, such as AL_POSITION or AL_VELOCITY.</param>
/// <param name="valuePointer1">Pointer to an integer that the parameter's X component value will be copied to.</param>
/// <param name="valuePointer2">Pointer to an integer that the parameter's Y component value will be copied to.</param>
/// <param name="valuePointer3">Pointer to an integer that the parameter's Z component value will be copied to.</param>
AL_API void AL_APIENTRY alGetListener3i(ALenum parameter, ALint *valuePointer1, ALint *valuePointer2, ALint *valuePointer3);

/// <summary>Gets the given parameter's values from the current audio context's one listener.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given value argument is null.</para>
/// </remarks>
/// <param name="parameter">The parameter to fetch, such as AL_POSITION, AL_VELOCITY, or AL_ORIENTATION.</param>
/// <param name="valueArray">
///  <para>An array that the parameter's values will be copied to.</para>
///  <para>Must be an array of at least 3 elements {x,y,z} for parameters AL_POSITION and AL_VELOCITY.</para>
///  <para>Must be an array of at least 6 elements {x1,y1,z1,x2,y2,z2} for the AL_ORIENTATION parameter.</para>
/// </param>
AL_API void AL_APIENTRY alGetListeneriv(ALenum parameter, ALint *valueArray);

/// <summary>Generates multiple audio sources for the current context that can be used to output audio in 3D space.</summary>
/// <remarks>
///  The alGetError() function will return AL_OUT_OF_MEMORY if there isn't enough memory to create all of the
///  requested audio sources. Zero audio sources will be created and no IDs will be copied to the given
///  "sourceIdArray" argument if this is the case.
/// </remarks>
/// <param name="numberOfSources">
///  <para>Number of audio sources to create.</para>
///  <para>This function will do nothing if set to zero.</para>
/// </param>
/// <param name="sourceIdArray">
///  <para>The array that the newly created audio sources will have their unique integer IDs copied to.</para>
///  <para>
///   The number of elements in this array must greater than or equal to argument "numberOfSources"
///   or else a crash will occur.
///  </para>
/// </param>
AL_API void AL_APIENTRY alGenSources(ALsizei numberOfSources, ALuint *sourceIdArray);

/// <summary>Deletes the given audio source IDs from the current audio context.</summary>
/// <remarks>The alGetError() function will return AL_INVALID_VALUE if given a null array.</remarks>
/// <param name="numberOfSources">
///  <para>Number of audio source IDs in argument "sourceIdArray" that should be deleted.</para>
///  <para>This function will do nothing if set to zero.</para>
/// </param>
/// <param name="sourceIdArray">
///  <para>An array of audio source IDs to be deleted.</para>
///  <para>These unique integer IDs were assigned to the audio sources by the alGenSources() function.</para>
/// </param>
AL_API void AL_APIENTRY alDeleteSources(ALsizei numberOfSources, const ALuint *sourceIdArray);

/// <summary>Determines if the given integer ID belongs to an existing audio source for the current audio context.</summary>
/// <param name="sourceId">
///  <para>The audio source's unique integer ID.</para>
///  <para>This ID is assigned to the audio source via the alGenSources() function.</para>
/// </param>
/// <returns>
///  <para>Returns AL_TRUE if an existing audio source having the given ID was found in the system.</para>
///  <para>
///   Returns AL_FALSE if an audio source with the given ID was not found. This can happen if an audio source with the
///   given ID has never existed or it did exist but was later deleted via the alDeleteSources() function.
///  </para>
/// </returns>
AL_API ALboolean AL_APIENTRY alIsSource(ALuint sourceId);

/// <summary>Sets the given audio source's parameter for the current audio context.</summary>
/// <remarks>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</remarks>
/// <param name="sourceId">Unique integer ID of the audio source to be modified.</param>
/// <param name="parameter">
///  The parameter to set such as AL_PITCH, AL_CONE_INNER_ANGLE, AL_CONE_OUTER_ANGLE, AL_GAIN, AL_MAX_DISTANCE,
///  AL_ROLLOFF_FACTOR, AL_REFERENCE_DISTANCE, AL_MIN_GAIN, AL_MAX_GAIN, AL_CONE_OUTER_GAIN, AL_DOPPLER_FACTOR,
///  AL_SEC_OFFSET, AL_SAMPLE_OFFSET, or AL_BYTE_OFFSET.
/// </param>
/// <param name="value">Single precision floating point value to be assigned to the parameter.</param>
AL_API void AL_APIENTRY alSourcef(ALuint sourceId, ALenum parameter, ALfloat value);

/// <summary>Sets the given audio source's parameter for the current audio context.</summary>
/// <remarks>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</remarks>
/// <param name="sourceId">Unique integer ID of the audio source to be modified.</param>
/// <param name="parameter">The parameter to set such as AL_POSITION, AL_VELOCITY, or AL_DIRECTION.</param>
/// <param name="value1">The X component to be assignedt to the parameter.</param>
/// <param name="value2">The Y component to be assignedt to the parameter.</param>
/// <param name="value3">The Z component to be assignedt to the parameter.</param>
AL_API void AL_APIENTRY alSource3f(ALuint sourceId, ALenum parameter, ALfloat value1, ALfloat value2, ALfloat value3);

/// <summary>Sets the given audio source's parameter for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given value argument is null.</para>
/// </remarks>
/// <param name="sourceId">Unique integer ID of the audio source to be modified.</param>
/// <param name="parameter">The parameter to set such as AL_POSITION, AL_VELOCITY, or AL_DIRECTION.</param>
/// <param name="valueArray">Array of 3 elements to be copied to the parameter.</param>
AL_API void AL_APIENTRY alSourcefv(ALuint sourceId, ALenum parameter, const ALfloat *valueArray);

/// <summary>Sets the given audio source's parameter for the current audio context.</summary>
/// <remarks>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</remarks>
/// <param name="sourceId">Unique integer ID of the audio source to be modified.</param>
/// <param name="parameter">
///  The parameter to set such as AL_MAX_DISTANCE, AL_ROLLOFF_FACTOR, AL_REFERENCE_DISTANCE, AL_DOPPLER_FACTOR,
///  AL_SEC_OFFSET, AL_SAMPLE_OFFSET, or AL_BYTE_OFFSET.
/// </param>
/// <param name="value">Integer value to be assigned to the parameter.</param>
AL_API void AL_APIENTRY alSourcei(ALuint sourceId, ALenum parameter, ALint value);

/// <summary>Sets the given audio source's parameter for the current audio context.</summary>
/// <remarks>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</remarks>
/// <param name="sourceId">Unique integer ID of the audio source to be modified.</param>
/// <param name="parameter">The parameter to set such as AL_POSITION, AL_VELOCITY, or AL_DIRECTION.</param>
/// <param name="value1">The X component to be assignedt to the parameter.</param>
/// <param name="value2">The Y component to be assignedt to the parameter.</param>
/// <param name="value3">The Z component to be assignedt to the parameter.</param>
AL_API void AL_APIENTRY alSource3i(ALuint sourceId, ALenum parameter, ALint value1, ALint value2, ALint value3);

/// <summary>Sets the given audio source's parameter for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given value argument is null.</para>
/// </remarks>
/// <param name="sourceId">Unique integer ID of the audio source to be modified.</param>
/// <param name="parameter">The parameter to set such as AL_POSITION, AL_VELOCITY, or AL_DIRECTION.</param>
/// <param name="valueArray">Array of 3 elements to be copied to the parameter.</param>
AL_API void AL_APIENTRY alSourceiv(ALuint sourceId, ALenum parameter, const ALint *valueArray);

/// <summary>Gets the specified audio source's parameter value for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given value argument is null.</para>
/// </remarks>
/// <param name="sourceId">Unique integer ID of the audio source to fetch data from.</param>
/// <param name="parameter">
///  The parameter to fetch data from such as AL_PITCH, AL_GAIN, AL_MIN_GAIN, AL_MAX_GAIN, AL_MAX_DISTANCE,
///  AL_ROLLOFF_FACTOR, AL_DOPPLER_FACTOR, AL_CONE_OUTER_GAIN, AL_SEC_OFFSET, AL_SAMPLE_OFFSET, AL_BYTE_OFFSET,
///  AL_CONE_INNER_ANGLE, AL_CONE_OUTER_ANGLE, or AL_REFERENCE_DISTANCE.
/// </param>
/// <param name="valuePointer">Pointer to a single precision float that the parameter's value will be copied to.</param>
AL_API void AL_APIENTRY alGetSourcef(ALuint sourceId, ALenum parameter, ALfloat *valuePointer);

/// <summary>Gets the specified audio source's parameter value for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if any of the given value arguments are null.</para>
/// </remarks>
/// <param name="sourceId">Unique integer ID of the audio source to fetch data from.</param>
/// <param name="parameter">The parameter to fetch data from such as AL_POSITION, AL_VELOCITY, or AL_DIRECTION.</param>
/// <param name="valuePointer1">Pointer to a single precision float that the parameter's X component will be copied to.</param>
/// <param name="valuePointer2">Pointer to a single precision float that the parameter's Y component will be copied to.</param>
/// <param name="valuePointer3">Pointer to a single precision float that the parameter's Z component will be copied to.</param>
AL_API void AL_APIENTRY alGetSource3f(ALuint sourceId, ALenum parameter, ALfloat *valuePointer1, ALfloat *valuePointer2, ALfloat *valuePointer3);

/// <summary>Gets the specified audio source's parameter value for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given value argument is null.</para>
/// </remarks>
/// <param name="sourceId">Unique integer ID of the audio source to fetch data from.</param>
/// <param name="parameter">The parameter to fetch data from such as AL_POSITION, AL_VELOCITY, or AL_DIRECTION.</param>
/// <param name="valueArray">Array of 3 elements that the parameter's {x,y,z} values will be copied to.</param>
AL_API void AL_APIENTRY alGetSourcefv(ALuint sourceId, ALenum parameter, ALfloat *valueArray);

/// <summary>Gets the specified audio source's parameter value for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given value argument is null.</para>
/// </remarks>
/// <param name="sourceId">Unique integer ID of the audio source to fetch data from.</param>
/// <param name="parameter">
///  The parameter to fetch data from such as AL_MAX_DISTANCE, AL_ROLLOFF_FACTOR, AL_REFERENCE_DISTANCE, AL_CONE_INNER_ANGLE,
///  AL_CONE_OUTER_ANGLE, AL_LOOPING, AL_BUFFER, AL_SOURCE_STATE, AL_BUFFERS_QUEUED, AL_BUFFERS_PROCESSED, AL_SOURCE_TYPE,
///  AL_SEC_OFFSET, AL_SAMPLE_OFFSET, AL_BYTE_OFFSET, or AL_DOPPLER_FACTOR.
/// </param>
/// <param name="valuePointer">Pointer to a 32-bit signed integer that the parameter's value will be copied to.</param>
AL_API void AL_APIENTRY alGetSourcei(ALuint sourceId, ALenum parameter, ALint *valuePointer);

/// <summary>Gets the specified audio source's parameter value for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if any of the given value arguments are null.</para>
/// </remarks>
/// <param name="sourceId">Unique integer ID of the audio source to fetch data from.</param>
/// <param name="parameter">The parameter to fetch data from such as AL_POSITION, AL_VELOCITY, or AL_DIRECTION.</param>
/// <param name="valuePointer1">Pointer to a 32-bit signed integer that the parameter's X component will be copied to.</param>
/// <param name="valuePointer2">Pointer to a 32-bit signed integer that the parameter's Y component will be copied to.</param>
/// <param name="valuePointer3">Pointer to a 32-bit signed integer that the parameter's Z component will be copied to.</param>
AL_API void AL_APIENTRY alGetSource3i(ALuint sourceId, ALenum parameter, ALint *valuePointer1, ALint *valuePointer2, ALint *valuePointer3);

/// <summary>Gets the specified audio source's parameter value for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given value argument is null.</para>
/// </remarks>
/// <param name="sourceId">Unique integer ID of the audio source to fetch data from.</param>
/// <param name="parameter">The parameter to fetch data from such as AL_POSITION, AL_VELOCITY, or AL_DIRECTION.</param>
/// <param name="valueArray">Array of 3 elements that the parameter's {x,y,z} values will be copied to.</param>
AL_API void AL_APIENTRY alGetSourceiv(ALuint sourceId, ALenum parameter, ALint *valueArray);

/// <summary>
///  <para>Plays audio from the given source for the current audio context.</para>
///  <para>If the audio source was paused, then it will be resumed at its last position in the audio track.</para>
/// </summary>
/// <remarks>
///  The alGetError() function will return AL_INVALID_NAME if the given audio source ID does not exist
///  or was deleted from the system.
/// </remarks>
/// <param name="souceId">Unique integer ID of the audio source to start playing audio.</param>
AL_API void AL_APIENTRY alSourcePlay(ALuint sourceId);

/// <summary>
///  <para>Play audio from the given sources for the current audio context.</para>
///  <para>If the audio sources were paused, then they will resume at their last position in the audio track.</para>
/// </summary>
/// <remarks>
///  <para>
///   The alGetError() function will return AL_INVALID_NAME if the given audio source array contains at least one ID
///   that does not exist or was deleted from the system. None of the audio sources will be played in this case,
///   even if the rest of the audio source IDs are valid.
///  </para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given array argument is null.</para>
/// </remarks>
/// <param name="numberOfSources">
///  <para>Number of audio sources in argument "sourceIdArray" to play audio from.</para>
///  <para>This function will do nothing if set to zero.</para>
/// </param>
/// <param name="sourceIdArray">
///  <para>An array of audio source IDs to be played.</para>
///  <para>These unique integer IDs were assigned to the audio sources by the alGenSources() function.</para>
/// </param>
AL_API void AL_APIENTRY alSourcePlayv(ALsizei numberOfSources, const ALuint *sourceIdArray);

/// <summary>
///  <para>Stops playback from the given audio source for the current audio context, placing it in the AL_STOPPED state.</para>
///  <para>Stopping an audio source will cause it to reset back to the beginning of the audio track.</para>
/// </summary>
/// <remarks>
///  The alGetError() function will return AL_INVALID_NAME if the given audio source ID does not exist
///  or was deleted from the system.
/// </remarks>
/// <param name="souceId">Unique integer ID of the audio source to stop.</param>
AL_API void AL_APIENTRY alSourceStop(ALuint sourceId);

/// <summary>
///  <para>
///   Stops playback for all of the given audio sources for the current audio context, placing them in the AL_STOPPED state.
///  </para>
///  <para>Stopping an audio source will cause it to reset back to the beginning of the audio track.</para>
/// </summary>
/// <remarks>
///  <para>
///   The alGetError() function will return AL_INVALID_NAME if the given audio source array contains at least one ID
///   that does not exist or was deleted from the system. None of the audio sources will be stopped in this case,
///   even if the rest of the audio source IDs are valid.
///  </para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given array argument is null.</para>
/// </remarks>
/// <param name="numberOfSources">
///  <para>Number of audio sources in argument "sourceIdArray" to be stopped.</para>
///  <para>This function will do nothing if set to zero.</para>
/// </param>
/// <param name="sourceIdArray">
///  <para>An array of audio source IDs to be stopped.</para>
///  <para>These unique integer IDs were assigned to the audio sources by the alGenSources() function.</para>
/// </param>
AL_API void AL_APIENTRY alSourceStopv(ALsizei numberOfSources, const ALuint *sourceIdArray);

/// <summary>
///  Stops and rewinds the given audio source back to the beginning of its audio track for the current audio context,
///  placing it in the AL_INITIAL state.
/// </summary>
/// <remarks>
///  The alGetError() function will return AL_INVALID_NAME if the given audio source ID does not exist
///  or was deleted from the system.
/// </remarks>
/// <param name="souceId">Unique integer ID of the audio source to rewind.</param>
AL_API void AL_APIENTRY alSourceRewind(ALuint sourceId);

/// <summary>
///  Stops audio playback and rewinds the audio track back to the beginning for all of the given audio sources
///  for the current audio context, placing them into the AL_INITIAL state.
/// </summary>
/// <remarks>
///  <para>
///   The alGetError() function will return AL_INVALID_NAME if the given audio source array contains at least one ID
///   that does not exist or was deleted from the system. None of the audio sources will be rewound in this case,
///   even if the rest of the audio source IDs are valid.
///  </para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given array argument is null.</para>
/// </remarks>
/// <param name="numberOfSources">
///  <para>Number of audio sources in argument "sourceIdArray" to be rewound.</para>
///  <para>This function will do nothing if set to zero.</para>
/// </param>
/// <param name="sourceIdArray">
///  <para>An array of audio source IDs to be rewound.</para>
///  <para>These unique integer IDs were assigned to the audio sources by the alGenSources() function.</para>
/// </param>
AL_API void AL_APIENTRY alSourceRewindv(ALsizei numberOfSources, const ALuint *sourceIdArray);

/// <summary>
///  Pauses the given audio source's playback for the current audio context, placing it in the AL_PAUSED state.
/// </summary>
/// <remarks>
///  The alGetError() function will return AL_INVALID_NAME if the given audio source ID does not exist
///  or was deleted from the system.
/// </remarks>
/// <param name="souceId">Unique integer ID of the audio source to pause.</param>
AL_API void AL_APIENTRY alSourcePause(ALuint sourceId);

/// <summary>
///  Pauses audio playback for all of the given audio sources for the current audio context,
///  placing them in the AL_PAUSED state.
/// </summary>
/// <remarks>
///  <para>
///   The alGetError() function will return AL_INVALID_NAME if the given audio source array contains at least one ID
///   that does not exist or was deleted from the system. None of the audio sources will be paused in this case,
///   even if the rest of the audio source IDs are valid.
///  </para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given array argument is null.</para>
/// </remarks>
/// <param name="numberOfSources">
///  <para>Number of audio sources in argument "sourceIdArray" to be paused.</para>
///  <para>This function will do nothing if set to zero.</para>
/// </param>
/// <param name="sourceIdArray">
///  <para>An array of audio source IDs to be paused.</para>
///  <para>These unique integer IDs were assigned to the audio sources by the alGenSources() function.</para>
/// </param>
AL_API void AL_APIENTRY alSourcePausev(ALsizei numberOfSources, const ALuint *sourceIdArray);

/// <summary>Streams out audio by pushing the buffers to the specified audio source.</summary>
/// <remarks>
///  <para>
///   The alGetError() function will return AL_INVALID_NAME if the given audio source ID or if any of the buffer IDs
///   in argument "bufferIdArray" cannot be found in the system or have been deleted.
///  </para>
///  <para>
///   The alGetError() function will return AL_INVALID_VALUE if the given array argument is null or if the
///   given audio source's AL_SOURCE_TYPE parameter is not set to AL_STREAMING.
///  </para>
/// </remarks>
/// <param name="souceId">Unique integer ID of the audio source to push buffers to for streaming.</param>
/// <param name="numberOfBuffers">
///  <para>The number of audio buffers in argument "bufferIdArray" to push into the audio source.</para>
///  <para>This function will do nothing if set to zero.</para>
/// </param>
/// <param name="bufferIdArray">
///  <para>An array of audio buffer IDs to be pushed into the audio source for streaming.</para>
///  <para>
///   The number of elements in this array must be greater than or equal to argument "numberOfBuffers"
///   or else a crash will occur.
///  </para>
///  <para>These unique integer IDs were assigned to the buffers by the alGenBuffers() function.</para>
/// </param>
AL_API void AL_APIENTRY alSourceQueueBuffers(ALuint sourceId, ALsizei numberOfBuffers, const ALuint *bufferIdArray);

/// <summary>Pops audio buffers from the specified audio source's queue used for streaming audio.</summary>
/// <remarks>
///  <para>
///   The alGetError() function will return AL_INVALID_NAME if the given audio source ID cannot be found in the
///   system or it has been deleted.
///  </para>
///  <para>
///   The alGetError() function will return AL_INVALID_VALUE if the given array argument is null or if the
///   given audio source's AL_SOURCE_TYPE parameter is not set to AL_STREAMING.
///  </para>
/// </remarks>
/// <param name="souceId">Unique integer ID of the audio source to pop buffers from.</param>
/// <param name="numberOfBuffers">
///  <para>The number of audio buffers to pop off of the audio source's streaming audio queue.</para>
///  <para>
///   Expected to be set to the number of processed buffers returned by parameter AL_BUFFERS_PROCESSED.
///   This way these buffers can be re-used by copying the next set of audio samples to them and pushed
///   back to the audio source via the alSourceQueueBuffers() function.
///  </para>
///  <para>This function will do nothing if set to zero.</para>
/// </param>
/// <param name="bufferIdArray">
///  <para>An array of integers that the audio source will copy popped off buffer IDs to.</para>
///  <para>
///   The number of elements in this array must be greater than or equal to argument "numberOfBuffers"
///   or else a crash will occur.
///  </para>
///  <para>These unique integer IDs were assigned to the buffers by the alGenBuffers() function.</para>
/// </param>
AL_API void AL_APIENTRY alSourceUnqueueBuffers(ALuint sourceId, ALsizei numberOfBuffers, ALuint *bufferIdArray);

/// <summary>Creates buffers used to store audio data to be played by audio sources for the current audio context.</summary>
/// <remarks>
///  <para>
///   The alGetError() function will return AL_OUT_OF_MEMORY if there isn't enough memory to create all of the
///   requested audio buffers. Zero audio buffers will be created and no IDs will be copied to the given
///   "bufferIdArray" argument if this is the case.
///  </para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given array is null.</para>
/// </remarks>
/// <param name="numberOfBuffers">
///  <para>Number of buffers to create.</para>
///  <para>Setting this to zero will cause this function to do nothing.</para>
/// </param>
/// <param name="bufferIdArray">
///  <para>Array of integers that this function will copy the newly created buffers' IDs to.</para>
///  <para>
///   The number of elements in this array must be greater than or equal to argument "numberOfBuffers"
///   or else a crash will occur.
///  </para>
/// </para>
AL_API void AL_APIENTRY alGenBuffers(ALsizei numberOfBuffers, ALuint *bufferIdArray);

/// <summary>Deletes the given audio buffers from the current audio context.</summary>
/// <remarks>
///  <para>
///   The alGetError() function will return AL_INVALID_NAME if any of the buffer IDs in the given array do not
///   exist in the system or have already been deleted. None of the buffers will be deleted in this case,
///   even if some of the IDs reference existing buffers.
///  </para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given array is null.</para>
/// </remarks>
/// <param name="numberOfBuffers">
///  <para>Number of buffer references in argument "bufferIdArray" to be deleted.</para>
///  <para>Setting this to zero will cause this function to do nothing.</para>
/// </param>
/// <param name="bufferIdArray">
///  <para>Array of audio buffer IDs to be deleted.</para>
///  <para>These unique integer IDs were assigned to the buffers by the alGenBuffers() function.</para>
///  <para>
///   The number of elements in this array must be greater than or equal to argument "numberOfBuffers"
///   or else a crash will occur.
///  </para>
/// </param>
AL_API void AL_APIENTRY alDeleteBuffers(ALsizei numberOfBuffers, const ALuint *bufferIdArray);

/// <summary>Determines if the given audio buffer ID exists in the system for the current audio context.</summary>
/// <param name="bufferId">Unique integer ID of the audio buffer to search for.</param>
/// <returns>
///  <para>Returns AL_TRUE if an existing audio buffer having the given ID was found in the system.</para>
///  <para>
///   Returns AL_FALSE if the given audio buffer ID was not found. This can happen if an audio buffer with the
///   given ID has never existed or it did exist but was later deleted via the alDeleteBuffers() function.
///  </para>
/// </returns>
AL_API ALboolean AL_APIENTRY alIsBuffer(ALuint bufferId);

/// <summary>Copies audio data to the specified buffer for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_NAME if the given buffer ID does not exist in the system.</para>
///  <para>
///   The alGetError() function will return AL_OUT_OF_MEMORY if failed to allocate memory for the buffer
///   to copy the given data to.
///  </para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given array is null.</para>
/// </remarks>
/// <param name="bufferId">Unique integer ID of the audio buffer to copy data to.</param>
/// <param name="format">
///  Indicates the format of the audio data provided by argument "dataArray" such as
///  AL_FORMAT_MONO8, AL_FORMAT_MONO16, AL_FORMAT_STEREO8, or AL_FORMAT_STEREO16.
/// </param>
/// <param name="dataArray">Array of audio data to be copied to the buffer.</param>
/// <param name="byteCount">Number of bytes used by the given "dataArray" argument.</param>
/// <param name="frequency">The frequency of the audio data in Hertz, such as 44100 Hz.</param>
AL_API void AL_APIENTRY alBufferData(ALuint bufferId, ALenum format, const ALvoid *dataArray, ALsizei byteCount, ALsizei frequency);

/// <summary>Sets the given audio buffer's parameter for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_NAME if the given buffer ID does not exist in the system.</para>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
/// </remarks>
/// <param name="bufferId">Unique integer ID of the audio buffer to modify.</param>
/// <param name="parameter">The parameter to set. (None are currently supported at this time.)</param>
/// <param name="value">The single precision floating point value to be assigned to the parameter.</param>
AL_API void AL_APIENTRY alBufferf(ALuint bufferId, ALenum parameter, ALfloat value);

/// <summary>Sets the given audio buffer's parameter for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_NAME if the given buffer ID does not exist in the system.</para>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
/// </remarks>
/// <param name="bufferId">Unique integer ID of the audio buffer to modify.</param>
/// <param name="parameter">The parameter to set. (None are currently supported at this time.)</param>
/// <param name="value1">The X component to be assigned to the parameter.</param>
/// <param name="value2">The Y component to be assigned to the parameter.</param>
/// <param name="value3">The Z component to be assigned to the parameter.</param>
AL_API void AL_APIENTRY alBuffer3f(ALuint bufferId, ALenum parameter, ALfloat value1, ALfloat value2, ALfloat value3);

/// <summary>Sets the given audio buffer's parameter for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_NAME if the given buffer ID does not exist in the system.</para>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given array is null.</para>
/// </remarks>
/// <param name="bufferId">Unique integer ID of the audio buffer to modify.</param>
/// <param name="parameter">The parameter to set. (None are currently supported at this time.)</param>
/// <param name="valueArray">Array of 3 elements to be copied to the parameter.</param>
AL_API void AL_APIENTRY alBufferfv(ALuint bufferId, ALenum parameter, const ALfloat *valueArray);

/// <summary>Sets the given audio buffer's parameter for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_NAME if the given buffer ID does not exist in the system.</para>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
/// </remarks>
/// <param name="bufferId">Unique integer ID of the audio buffer to modify.</param>
/// <param name="parameter">The parameter to set. (None are currently supported at this time.)</param>
/// <param name="value">The 32-bit signed integer value to be assigned to the parameter.</param>
AL_API void AL_APIENTRY alBufferi(ALuint bufferId, ALenum parameter, ALint value);

/// <summary>Sets the given audio buffer's parameter for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_NAME if the given buffer ID does not exist in the system.</para>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
/// </remarks>
/// <param name="bufferId">Unique integer ID of the audio buffer to modify.</param>
/// <param name="parameter">The parameter to set. (None are currently supported at this time.)</param>
/// <param name="value1">The X component to be assigned to the parameter.</param>
/// <param name="value2">The Y component to be assigned to the parameter.</param>
/// <param name="value3">The Z component to be assigned to the parameter.</param>
AL_API void AL_APIENTRY alBuffer3i(ALuint bufferId, ALenum parameter, ALint value1, ALint value2, ALint value3);

/// <summary>Sets the given audio buffer's parameter for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_NAME if the given buffer ID does not exist in the system.</para>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given array is null.</para>
/// </remarks>
/// <param name="bufferId">Unique integer ID of the audio buffer to modify.</param>
/// <param name="parameter">The parameter to set. (None are currently supported at this time.)</param>
/// <param name="valueArray">Array of 3 elements to be copied to the parameter.</param>
AL_API void AL_APIENTRY alBufferiv(ALuint bufferId, ALenum parameter, const ALint *valueArray);

/// <summary>Gets the specified audio buffer's parameter value for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_NAME if the given buffer ID does not exist in the system.</para>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given value argument is null.</para>
/// </remarks>
/// <param name="bufferId">Unique integer ID of the audio buffer to fetch data from.</param>
/// <param name="parameter">The parameter to fetch data from. (None are currently supported at this time.)</param>
/// <param name="valuePointer">Pointer to a single precision float that the parameter's value will be copied to.</param>
AL_API void AL_APIENTRY alGetBufferf(ALuint bufferId, ALenum parameter, ALfloat *valuePointer);

/// <summary>Gets the specified audio buffer's parameter value for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_NAME if the given buffer ID does not exist in the system.</para>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given value arguments are null.</para>
/// </remarks>
/// <param name="bufferId">Unique integer ID of the audio buffer to fetch data from.</param>
/// <param name="parameter">The parameter to fetch data from. (None are currently supported at this time.)</param>
/// <param name="valuePointer1">Pointer to a single precision float that the parameter's X component will be copied to.</param>
/// <param name="valuePointer2">Pointer to a single precision float that the parameter's Y component will be copied to.</param>
/// <param name="valuePointer3">Pointer to a single precision float that the parameter's Z component will be copied to.</param>
AL_API void AL_APIENTRY alGetBuffer3f(ALuint bufferId, ALenum parameter, ALfloat *valuePointer1, ALfloat *valuePointer2, ALfloat *valuePointer3);

/// <summary>Gets the specified audio buffer's parameter value for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_NAME if the given buffer ID does not exist in the system.</para>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given value argument is null.</para>
/// </remarks>
/// <param name="bufferId">Unique integer ID of the audio buffer to fetch data from.</param>
/// <param name="parameter">The parameter to fetch data from. (None are currently supported at this time.)</param>
/// <param name="valueArray">Array of 3 elements that the parameter's {x,y,z} values will be copied to.</param>
AL_API void AL_APIENTRY alGetBufferfv(ALuint bufferId, ALenum parameter, ALfloat *valueArray);

/// <summary>Gets the specified audio buffer's parameter value for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_NAME if the given buffer ID does not exist in the system.</para>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given value argument is null.</para>
/// </remarks>
/// <param name="bufferId">Unique integer ID of the audio buffer to fetch data from.</param>
/// <param name="parameter">The parameter to fetch data from such as AL_FREQUENCY, AL_BITS, AL_CHANNELS, or AL_SIZE.</param>
/// <param name="valuePointer">Pointer to a 32-bit signed integer that the parameter's value will be copied to.</param>
AL_API void AL_APIENTRY alGetBufferi(ALuint bufferId, ALenum parameter, ALint *valuePointer);

/// <summary>Gets the specified audio buffer's parameter value for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_NAME if the given buffer ID does not exist in the system.</para>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given value arguments are null.</para>
/// </remarks>
/// <param name="bufferId">Unique integer ID of the audio buffer to fetch data from.</param>
/// <param name="parameter">The parameter to fetch data from. (None are currently supported at this time.)</param>
/// <param name="valuePointer1">Pointer to a 32-bit signed integer that the parameter's X component will be copied to.</param>
/// <param name="valuePointer2">Pointer to a 32-bit signed integer that the parameter's Y component will be copied to.</param>
/// <param name="valuePointer3">Pointer to a 32-bit signed integer that the parameter's Z component will be copied to.</param>
AL_API void AL_APIENTRY alGetBuffer3i(ALuint bufferId, ALenum parameter, ALint *valuePointer1, ALint *valuePointer2, ALint *valuePointer3);

/// <summary>Gets the specified audio buffer's parameter value for the current audio context.</summary>
/// <remarks>
///  <para>The alGetError() function will return AL_INVALID_NAME if the given buffer ID does not exist in the system.</para>
///  <para>The alGetError() function will return AL_INVALID_ENUM if the given parameter is not supported.</para>
///  <para>The alGetError() function will return AL_INVALID_VALUE if the given value argument is null.</para>
/// </remarks>
/// <param name="bufferId">Unique integer ID of the audio buffer to fetch data from.</param>
/// <param name="parameter">The parameter to fetch data from. (None are currently supported at this time.)</param>
/// <param name="valueArray">Array of 3 elements that the parameter's {x,y,z} values will be copied to.</param>
AL_API void AL_APIENTRY alGetBufferiv(ALuint bufferId, ALenum parameter, ALint *valueArray);

#pragma endregion


#pragma region Function Pointer Types
/// <summary>Function pointer type to the alEnable() function.</summary>
typedef void (AL_APIENTRY *LPALENABLE)(ALenum capability);

/// <summary>Function pointer type to the alDisable() function.</summary>
typedef void (AL_APIENTRY *LPALDISABLE)(ALenum capability);

/// <summary>Function pointer type to the alIsEnabled() function.</summary>
typedef ALboolean (AL_APIENTRY *LPALISENABLED)(ALenum capability);

/// <summary>Function pointer type to the alDopplerFactor() function.</summary>
typedef void (AL_APIENTRY *LPALDOPPLERFACTOR)(ALfloat value);

/// <summary>Function pointer type to the alDopplerVelocity() function.</summary>
typedef void (AL_APIENTRY *LPALDOPPLERVELOCITY)(ALfloat value);

/// <summary>Function pointer type to the alSpeedOfSound() function.</summary>
typedef void (AL_APIENTRY *LPALSPEEDOFSOUND)(ALfloat value);

/// <summary>Function pointer type to the alDistanceModel() function.</summary>
typedef void (AL_APIENTRY *LPALDISTANCEMODEL)(ALenum modelType);

/// <summary>Function pointer type to the alGetString() function.</summary>
typedef const ALchar* (AL_APIENTRY *LPALGETSTRING)(ALenum parameter);

/// <summary>Function pointer type to the alGetBoolean() function.</summary>
typedef ALboolean (AL_APIENTRY *LPALGETBOOLEAN)(ALenum parameter);

/// <summary>Function pointer type to the alGetInteger() function.</summary>
typedef ALint (AL_APIENTRY *LPALGETINTEGER)(ALenum parameter);

/// <summary>Function pointer type to the alGetFloat() function.</summary>
typedef ALfloat (AL_APIENTRY *LPALGETFLOAT)(ALenum parameter);

/// <summary>Function pointer type to the alGetDouble() function.</summary>
typedef ALdouble (AL_APIENTRY *LPALGETDOUBLE)(ALenum parameter);

/// <summary>Function pointer type to the alGetBooleanv() function.</summary>
typedef void (AL_APIENTRY *LPALGETBOOLEANV)(ALenum parameter, ALboolean *valueArray);

/// <summary>Function pointer type to the alGetIntegerv() function.</summary>
typedef void (AL_APIENTRY *LPALGETINTEGERV)(ALenum parameter, ALint *valueArray);

/// <summary>Function pointer type to the alGetFloatv() function.</summary>
typedef void (AL_APIENTRY *LPALGETFLOATV)(ALenum parameter, ALfloat *valueArray);

/// <summary>Function pointer type to the alGetDoublev() function.</summary>
typedef void (AL_APIENTRY *LPALGETDOUBLEV)(ALenum parameter, ALdouble *valueArray);

/// <summary>Function pointer type to the alGetError() function.</summary>
typedef ALenum (AL_APIENTRY *LPALGETERROR)(void);

/// <summary>Function pointer type to the alIsExtensionPresent() function.</summary>
typedef ALboolean(AL_APIENTRY *LPALISEXTENSIONPRESENT)(const ALchar *extensionName);

/// <summary>Function pointer type to the alGetProcAddress() function.</summary>
typedef void* (AL_APIENTRY *LPALGETPROCADDRESS)(const ALchar *functionName);

/// <summary>Function pointer type to the alGetEnumValue() function.</summary>
typedef ALenum (AL_APIENTRY *LPALGETENUMVALUE)(const ALchar *enumName);

/// <summary>Function pointer type to the alListenerf() function.</summary>
typedef void (AL_APIENTRY *LPALLISTENERF)(ALenum parameter, ALfloat value);

/// <summary>Function pointer type to the alListener3f() function.</summary>
typedef void (AL_APIENTRY *LPALLISTENER3F)(ALenum parameter, ALfloat value1, ALfloat value2, ALfloat value3);

/// <summary>Function pointer type to the alListenerfv() function.</summary>
typedef void (AL_APIENTRY *LPALLISTENERFV)(ALenum parameter, const ALfloat *valueArray);

/// <summary>Function pointer type to the alListeneri() function.</summary>
typedef void (AL_APIENTRY *LPALLISTENERI)(ALenum parameter, ALint value);

/// <summary>Function pointer type to the alListener3i() function.</summary>
typedef void (AL_APIENTRY *LPALLISTENER3I)(ALenum parameter, ALint value1, ALint value2, ALint value3);

/// <summary>Function pointer type to the alListeneriv() function.</summary>
typedef void (AL_APIENTRY *LPALLISTENERIV)(ALenum parameter, const ALint *valueArray);

/// <summary>Function pointer type to the alGetListenerf() function.</summary>
typedef void (AL_APIENTRY *LPALGETLISTENERF)(ALenum parameter, ALfloat *valuePointer);

/// <summary>Function pointer type to the alGetListener3f() function.</summary>
typedef void (AL_APIENTRY *LPALGETLISTENER3F)(ALenum parameter, ALfloat *valuePointer1, ALfloat *valuePointer2, ALfloat *valuePointer3);

/// <summary>Function pointer type to the alGetListenerfv() function.</summary>
typedef void (AL_APIENTRY *LPALGETLISTENERFV)(ALenum parameter, ALfloat *valueArray);

/// <summary>Function pointer type to the alGetListeneri() function.</summary>
typedef void (AL_APIENTRY *LPALGETLISTENERI)(ALenum parameter, ALint *valuePointer);

/// <summary>Function pointer type to the alGetListener3i() function.</summary>
typedef void (AL_APIENTRY *LPALGETLISTENER3I)(ALenum parameter, ALint *valuePointer1, ALint *valuePointer2, ALint *valuePointer3);

/// <summary>Function pointer type to the alGetListeneriv() function.</summary>
typedef void (AL_APIENTRY *LPALGETLISTENERIV)(ALenum parameter, ALint *valueArray);

/// <summary>Function pointer type to the alGenSources() function.</summary>
typedef void (AL_APIENTRY *LPALGENSOURCES)(ALsizei numberOfSources, ALuint *sourceIdArray);

/// <summary>Function pointer type to the alDeleteSources() function.</summary>
typedef void (AL_APIENTRY *LPALDELETESOURCES)(ALsizei numberOfSources, const ALuint *sourceIdArray);

/// <summary>Function pointer type to the alIsSource() function.</summary>
typedef ALboolean (AL_APIENTRY *LPALISSOURCE)(ALuint sourceId);

/// <summary>Function pointer type to the alSourcef() function.</summary>
typedef void (AL_APIENTRY *LPALSOURCEF)(ALuint sourceId, ALenum parameter, ALfloat value);

/// <summary>Function pointer type to the alSource3f() function.</summary>
typedef void (AL_APIENTRY *LPALSOURCE3F)(ALuint sourceId, ALenum parameter, ALfloat value1, ALfloat value2, ALfloat value3);

/// <summary>Function pointer type to the alSourcefv() function.</summary>
typedef void (AL_APIENTRY *LPALSOURCEFV)(ALuint sourceId, ALenum parameter, const ALfloat *valueArray);

/// <summary>Function pointer type to the alSourcei() function.</summary>
typedef void (AL_APIENTRY *LPALSOURCEI)(ALuint sourceId, ALenum parameter, ALint value);

/// <summary>Function pointer type to the alSource3i() function.</summary>
typedef void (AL_APIENTRY *LPALSOURCE3I)(ALuint sourceId, ALenum parameter, ALint value1, ALint value2, ALint value3);

/// <summary>Function pointer type to the alSourceiv() function.</summary>
typedef void (AL_APIENTRY *LPALSOURCEIV)(ALuint sourceId, ALenum parameter, const ALint *valueArray);

/// <summary>Function pointer type to the alGetSourcef() function.</summary>
typedef void (AL_APIENTRY *LPALGETSOURCEF)(ALuint sourceId, ALenum parameter, ALfloat *valuePointer);

/// <summary>Function pointer type to the alGetSource3f() function.</summary>
typedef void (AL_APIENTRY *LPALGETSOURCE3F)(ALuint sourceId, ALenum parameter, ALfloat *valuePointer1, ALfloat *valuePointer2, ALfloat *valuePointer3);

/// <summary>Function pointer type to the alGetSourcefv() function.</summary>
typedef void (AL_APIENTRY *LPALGETSOURCEFV)(ALuint sourceId, ALenum parameter, ALfloat *valueArray);

/// <summary>Function pointer type to the alGetSourcei() function.</summary>
typedef void (AL_APIENTRY *LPALGETSOURCEI)(ALuint sourceId, ALenum parameter, ALint *valuePointer);

/// <summary>Function pointer type to the alGetSource3i() function.</summary>
typedef void (AL_APIENTRY *LPALGETSOURCE3I)(ALuint sourceId, ALenum parameter, ALint *valuePointer1, ALint *valuePointer2, ALint *valuePointer3);

/// <summary>Function pointer type to the alGetSourceiv() function.</summary>
typedef void (AL_APIENTRY *LPALGETSOURCEIV)(ALuint sourceId, ALenum parameter, ALint *valueArray);

/// <summary>Function pointer type to the alSourcePlay() function.</summary>
typedef void (AL_APIENTRY *LPALSOURCEPLAY)(ALuint sourceId);

/// <summary>Function pointer type to the alSourcePlayv() function.</summary>
typedef void (AL_APIENTRY *LPALSOURCEPLAYV)(ALsizei numberOfSources, const ALuint *sourceIdArray);

/// <summary>Function pointer type to the alSourceStop() function.</summary>
typedef void (AL_APIENTRY *LPALSOURCESTOP)(ALuint sourceId);

/// <summary>Function pointer type to the alSourceStopv() function.</summary>
typedef void (AL_APIENTRY *LPALSOURCESTOPV)(ALsizei numberOfSources, const ALuint *sourceIdArray);

/// <summary>Function pointer type to the alSourceRewind() function.</summary>
typedef void (AL_APIENTRY *LPALSOURCEREWIND)(ALuint sourceId);

/// <summary>Function pointer type to the alSourceRewindv() function.</summary>
typedef void (AL_APIENTRY *LPALSOURCEREWINDV)(ALsizei numberOfSources, const ALuint *sourceIdArray);

/// <summary>Function pointer type to the alSourcePause() function.</summary>
typedef void (AL_APIENTRY *LPALSOURCEPAUSE)(ALuint sourceId);

/// <summary>Function pointer type to the alSourcePausev() function.</summary>
typedef void (AL_APIENTRY *LPALSOURCEPAUSEV)(ALsizei numberOfSources, const ALuint *sourceIdArray);

/// <summary>Function pointer type to the alSourceQueueBuffers() function.</summary>
typedef void (AL_APIENTRY *LPALSOURCEQUEUEBUFFERS)(ALuint sourceId, ALsizei numberOfBuffers, const ALuint *bufferIdArray);

/// <summary>Function pointer type to the alSourceUnqueueBuffers() function.</summary>
typedef void (AL_APIENTRY *LPALSOURCEUNQUEUEBUFFERS)(ALuint sourceId, ALsizei numberOfBuffers, ALuint *bufferIdArray);

/// <summary>Function pointer type to the alGenBuffers() function.</summary>
typedef void (AL_APIENTRY *LPALGENBUFFERS)(ALsizei numberOfBuffers, ALuint *bufferIdArray);

/// <summary>Function pointer type to the alDeleteBuffers() function.</summary>
typedef void (AL_APIENTRY *LPALDELETEBUFFERS)(ALsizei numberOfBuffers, const ALuint *bufferIdArray);

/// <summary>Function pointer type to the alIsBuffer() function.</summary>
typedef ALboolean (AL_APIENTRY *LPALISBUFFER)(ALuint bufferId);

/// <summary>Function pointer type to the alBufferData() function.</summary>
typedef void (AL_APIENTRY *LPALBUFFERDATA)(ALuint bufferId, ALenum format, const ALvoid *dataArray, ALsizei dataArraySize, ALsizei frequency);

/// <summary>Function pointer type to the alBufferf() function.</summary>
typedef void (AL_APIENTRY *LPALBUFFERF)(ALuint bufferId, ALenum parameter, ALfloat value);

/// <summary>Function pointer type to the alBuffer3f() function.</summary>
typedef void (AL_APIENTRY *LPALBUFFER3F)(ALuint bufferId, ALenum parameter, ALfloat value1, ALfloat value2, ALfloat value3);

/// <summary>Function pointer type to the alBufferfv() function.</summary>
typedef void (AL_APIENTRY *LPALBUFFERFV)(ALuint bufferId, ALenum parameter, const ALfloat *valueArray);

/// <summary>Function pointer type to the alBufferi() function.</summary>
typedef void (AL_APIENTRY *LPALBUFFERI)(ALuint bufferId, ALenum parameter, ALint value);

/// <summary>Function pointer type to the alBuffer3i() function.</summary>
typedef void (AL_APIENTRY *LPALBUFFER3I)(ALuint bufferId, ALenum parameter, ALint value1, ALint value2, ALint value3);

/// <summary>Function pointer type to the alBufferiv() function.</summary>
typedef void (AL_APIENTRY *LPALBUFFERIV)(ALuint bufferId, ALenum parameter, const ALint *valueArray);

/// <summary>Function pointer type to the alGetBufferf() function.</summary>
typedef void (AL_APIENTRY *LPALGETBUFFERF)(ALuint bufferId, ALenum parameter, ALfloat *valuePointer);

/// <summary>Function pointer type to the alGetBuffer3f() function.</summary>
typedef void (AL_APIENTRY *LPALGETBUFFER3F)(ALuint bufferId, ALenum parameter, ALfloat *valuePointer1, ALfloat *valuePointer2, ALfloat *valuePointer3);

/// <summary>Function pointer type to the alGetBufferfv() function.</summary>
typedef void (AL_APIENTRY *LPALGETBUFFERFV)(ALuint bufferId, ALenum parameter, ALfloat *valueArray);

/// <summary>Function pointer type to the alGetBufferi() function.</summary>
typedef void (AL_APIENTRY *LPALGETBUFFERI)(ALuint bufferId, ALenum parameter, ALint *valuePointer);

/// <summary>Function pointer type to the alGetBuffer3i() function.</summary>
typedef void (AL_APIENTRY *LPALGETBUFFER3I)(ALuint bufferId, ALenum parameter, ALint *valuePointer1, ALint *valuePointer2, ALint *valuePointer3);

/// <summary>Function pointer type to the alGetBufferiv() function.</summary>
typedef void (AL_APIENTRY *LPALGETBUFFERIV)(ALuint bufferId, ALenum parameter, ALint *valueArray);

#pragma endregion

#ifdef __cplusplus
}	// extern "C"
#endif
