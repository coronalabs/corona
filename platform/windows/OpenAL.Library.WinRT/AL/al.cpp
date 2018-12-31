// ----------------------------------------------------------------------------
//
// al.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
//
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include <pch.h>
#include "al.h"
#include "alext.h"
#include "AudioBuffer.h"
#include "AudioBufferManager.h"
#include "AudioContext.h"
#include "AudioEnvironment.h"
#include "AudioListener.h"
#include "AudioSource.h"
#include "AudioSourceManager.h"
#include <unordered_map>


#pragma region Static Member Variables
/// <summary>Stores the error code from the last OpenAL function that was called.</summary>
static ALenum sLastErrorCode = AL_NO_ERROR;

#pragma endregion


#pragma region Private Types
/// <summary>
///  <para>Executes an operation on an audio source such as Play, Pause, Stop, etc.</para>
///  <para>
///   This is an abstract class which implements the "Command" design pattern where derived classes
///   are expected to execute one particular operation on an audio source.
///  </para>
/// </summary>
class BaseAudioSourceOperation
{
	public:
		/// <summary>Executes the operation on the given audio source.</summary>
		/// <param name="sourcePointer">The audio source to execute the operation on. Will do nothing if given null.</param>
		virtual void ExecuteOn(AL::AudioSource *sourcePointer) = 0;
};

/// <summary>Calls an audio source's Play() method.</summary>
class PlayAudioSourceOperation : public BaseAudioSourceOperation
{
	public:
		/// <summary>Executes the operation on the given audio source.</summary>
		/// <param name="sourcePointer">The audio source to execute the operation on. Will do nothing if given null.</param>
		virtual void ExecuteOn(AL::AudioSource *sourcePointer)
		{
			if (sourcePointer)
			{
				sourcePointer->Play();
			}
		}
};

/// <summary>Calls an audio source's Stop() method.</summary>
class StopAudioSourceOperation : public BaseAudioSourceOperation
{
	public:
		/// <summary>Executes the operation on the given audio source.</summary>
		/// <param name="sourcePointer">The audio source to execute the operation on. Will do nothing if given null.</param>
		virtual void ExecuteOn(AL::AudioSource *sourcePointer)
		{
			if (sourcePointer)
			{
				sourcePointer->Stop();
			}
		}
};

/// <summary>Calls an audio source's Pause() method.</summary>
class PauseAudioSourceOperation : public BaseAudioSourceOperation
{
	public:
		/// <summary>Executes the operation on the given audio source.</summary>
		/// <param name="sourcePointer">The audio source to execute the operation on. Will do nothing if given null.</param>
		virtual void ExecuteOn(AL::AudioSource *sourcePointer)
		{
			if (sourcePointer)
			{
				sourcePointer->Pause();
			}
		}
};

/// <summary>Calls an audio source's StopAndRewind() method.</summary>
class RewindAudioSourceOperation : public BaseAudioSourceOperation
{
	public:
		/// <summary>Executes the operation on the given audio source.</summary>
		/// <param name="sourcePointer">The audio source to execute the operation on. Will do nothing if given null.</param>
		virtual void ExecuteOn(AL::AudioSource *sourcePointer)
		{
			if (sourcePointer)
			{
				sourcePointer->StopAndRewind();
			}
		}
};

/// <summary>
///  <para>Provides a resonably unique hash code for an ALchar formatted string.</para>
///  <para>Intended to be used by an STL hash table collection for storing/finding ALchar strings.</para>
/// </summary>
struct StlStringPointerHashProvider : public std::hash<const ALchar*>
{
	size_t operator()(const ALchar* stringPointer) const
	{
		size_t hashValue = 0;
		if (stringPointer)
		{
			// Hash all of the characters in the string, excluding the null termination character.
			// Note: Null and empty strings will have the same hash value.
			for (; *stringPointer != '\0'; stringPointer++)
			{
				// Rotate the hash's bits left by one character to ensure all bytes in the hash are used.
				_rotl(hashValue, 8);

				// XOR the next character in the string to the hash's first byte.
				hashValue ^= (size_t)(*stringPointer);
			}
		}
		return hashValue;
	}
};

/// <summary>
///  <para>Determines if two ALchar formatted strings are equal.</para>
///  <para>Intended to be used by an STL hash table collection for finding ALchar strings.</para>
/// </summary>
struct StlStringPointerEqualityComparer : public std::equal_to<const ALchar*>
{
	bool operator()(const ALchar* string1, const ALchar* string2) const
	{
		// If the string pointers match or they are both null, then they are equal.
		if (string1 == string2)
		{
			return true;
		}

		// If one of the string pointers are null and the other is not, then they are not equal.
		// Note: Both pointers can't be null at this point due to the above check.
		if (!string1 || !string2)
		{
			return false;
		}

		// Check if all characters match.
		return (strcmp((const char*)string1, (const char*)string2) == 0);
	}
};

#pragma endregion


#pragma region Private Functions
static void ExecuteOperationOnSources(
	ALsizei numberOfSources, const ALuint *sourceIdArray, BaseAudioSourceOperation &operation)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Do not continue if given zero.
	if (numberOfSources < 1)
	{
		sLastErrorCode = AL_NO_ERROR;
		return;
	}

	// Validate pointer argument.
	if (nullptr == sourceIdArray)
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the current context and its audio source manager.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return;
	}
	AL::AudioSourceManager &sourceManager = contextPointer->GetSourceManager();

	// Do not continue if the given array contains at least 1 audio source ID that does not exist in the system.
	for (int index = 0; index < numberOfSources; index++)
	{
		if (sourceManager.ContainsId(sourceIdArray[index]) == false)
		{
			sLastErrorCode = AL_INVALID_NAME;
			return;
		}
	}

	// Execute the given operation on all of the given audio sources.
	AL::AudioSource *sourcePointer;
	for (int index = 0; index < numberOfSources; index++)
	{
		sourcePointer = sourceManager.GetById(sourceIdArray[index]);
		operation.ExecuteOn(sourcePointer);
	}
	sLastErrorCode = AL_NO_ERROR;
}

/// <summary>
///  <para>This function is to be called by all audio buffer getter/setter functions that do not support any parameters.</para>
///  <para>This function will always flag an error that can be retrieved via the alGetLastError() function.</para>
///  <para>Note: Audio buffers do not have any settable parameters and should always call this function.</para>
/// </summary>
/// <param name="bufferId">Unique integer ID of the audio buffer that the caller is attempting to access.</param>
/// <param name="parameter">The invalid parameter that the caller is attempting to read/write.</param>
static void HandleInvalidBufferOperationFor(ALuint bufferId, ALenum parameter)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Fetch the current context.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return;
	}

	// Fetch the specified audio buffer by its unique integer ID.
	auto bufferPointer = contextPointer->GetBufferManager().GetById(bufferId);
	if (nullptr == bufferPointer)
	{
		sLastErrorCode = AL_INVALID_NAME;
		return;
	}

	// Flag the given parameter as an error.
	sLastErrorCode = AL_INVALID_ENUM;
}

#pragma endregion


#pragma region Public Functions
void alEnable(ALenum capability)
{
	// This library does not current support any extensions that can be enabled/disabled.
}

void alDisable(ALenum capability)
{
	// This library does not current support any extensions that can be enabled/disabled.
}

ALboolean alIsEnabled(ALenum capability)
{
	// This library does not current support any extensions that can be enabled/disabled.
	return AL_FALSE;
}

void alDopplerFactor(ALfloat value)
{
	//TODO: Add doppler effect support.
}

void alDopplerVelocity(ALfloat value)
{
	//TODO: Add doppler effect support.
}

void alSpeedOfSound(ALfloat value)
{
	//TODO: Add doppler effect support.
}

void alDistanceModel(ALenum modelType)
{
	//TODO: Add positional audio support.
}

const ALchar* alGetString(ALenum parameter)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Clear the last error code.
	sLastErrorCode = AL_NO_ERROR;

	// Fetch the requested string.
	ALchar *stringValue = nullptr;
	switch (parameter)
	{
		case AL_VERSION:
			stringValue = "1.1";
			break;
		case AL_RENDERER:
			stringValue = "XAudio2";
			break;
		case AL_VENDOR:
			stringValue = "Corona Labs Inc.";
			break;
		case AL_EXTENSIONS:
			stringValue = "";		// This library does not currently support any extensions.
			break;
		case AL_NO_ERROR:
			stringValue = "No Error";
			break;
		case AL_INVALID_NAME:
			stringValue = "Invalid Name";
			break;
		case AL_INVALID_ENUM:
			stringValue = "Invalid Enum";
			break;
		case AL_INVALID_VALUE:
			stringValue = "Invalid Value";
			break;
		case AL_INVALID_OPERATION:
			stringValue = "Invalid Operation";
			break;
		case AL_OUT_OF_MEMORY:
			stringValue = "Out of Memory";
			break;
		default:
			sLastErrorCode = AL_INVALID_ENUM;
			break;
	}
	return stringValue;
}

ALboolean alGetBoolean(ALenum parameter)
{
	//TODO: Add positional audio support.
	return AL_FALSE;
}

ALint alGetInteger(ALenum parameter)
{
	//TODO: Add positional audio support.
	return 0;
}

ALfloat alGetFloat(ALenum parameter)
{
	//TODO: Add positional audio support.
	return 0;
}

ALdouble alGetDouble(ALenum parameter)
{
	//TODO: Add positional audio support.
	return 0;
}

void alGetBooleanv(ALenum parameter, ALboolean *valueArray)
{
	//TODO: Add positional audio support.
}

void alGetIntegerv(ALenum parameter, ALint *valueArray)
{
	//TODO: Add positional audio support.
}

void alGetFloatv(ALenum parameter, ALfloat *valueArray)
{
	//TODO: Add positional audio support.
}

void alGetDoublev(ALenum parameter, ALdouble *valueArray)
{
	//TODO: Add positional audio support.
}

ALenum alGetError(void)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Do not continue if no audio context is currently selected.
	if (AL::AudioEnvironment::GetCurrentContext() == nullptr)
	{
		return AL_INVALID_OPERATION;
	}

	// Fetch the last error code
	ALenum lastErrorCode = sLastErrorCode;

	// Clear the error code for the next time this function gets called.
	sLastErrorCode = AL_NO_ERROR;

	// Return the last error code.
	return lastErrorCode;
}

ALboolean alIsExtensionPresent(const ALchar *extensionName)
{
	// This library does not currently support any extensions.
	return AL_FALSE;
}

/// <summary>
///  <para>
///   Macro used to insert an OpenAL function name and pointer into the alGetProcAddress() function's
///   static "sFunctionTable" collection.
///  </para>
///  <para>This macro can only be used within the alGetProcAddress() function.</para>
/// </summary>
/// <param name="alFunction">The "al*()" function to be inserted into the table.</param>
#define AL_FUNCTION_TABLE_INSERT(alFunction) sFunctionTable.insert(std::pair<const ALchar*, void*>(#alFunction, &alFunction))

void* alGetProcAddress(const ALchar *functionName)
{
	static std::unordered_map<const ALchar*, void*, StlStringPointerHashProvider, StlStringPointerEqualityComparer> sFunctionTable;

	// Do not continue if given a null or empty string.
	if ((nullptr == functionName) || ('\0' == functionName[0]))
	{
		return nullptr;
	}

	// Create the function name/pointer table, if not done already.
	if (sFunctionTable.size() < 1)
	{
		AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
		if (sFunctionTable.size() < 1)
		{
			AL_FUNCTION_TABLE_INSERT(alEnable);
			AL_FUNCTION_TABLE_INSERT(alDisable);
			AL_FUNCTION_TABLE_INSERT(alIsEnabled);
			AL_FUNCTION_TABLE_INSERT(alDopplerFactor);
			AL_FUNCTION_TABLE_INSERT(alDopplerVelocity);
			AL_FUNCTION_TABLE_INSERT(alSpeedOfSound);
			AL_FUNCTION_TABLE_INSERT(alDistanceModel);
			AL_FUNCTION_TABLE_INSERT(alGetString);
			AL_FUNCTION_TABLE_INSERT(alGetBoolean);
			AL_FUNCTION_TABLE_INSERT(alGetInteger);
			AL_FUNCTION_TABLE_INSERT(alGetFloat);
			AL_FUNCTION_TABLE_INSERT(alGetDouble);
			AL_FUNCTION_TABLE_INSERT(alGetBooleanv);
			AL_FUNCTION_TABLE_INSERT(alGetIntegerv);
			AL_FUNCTION_TABLE_INSERT(alGetFloatv);
			AL_FUNCTION_TABLE_INSERT(alGetDoublev);
			AL_FUNCTION_TABLE_INSERT(alGetError);
			AL_FUNCTION_TABLE_INSERT(alIsExtensionPresent);
			AL_FUNCTION_TABLE_INSERT(alGetProcAddress);
			AL_FUNCTION_TABLE_INSERT(alGetEnumValue);
			AL_FUNCTION_TABLE_INSERT(alListenerf);
			AL_FUNCTION_TABLE_INSERT(alListener3f);
			AL_FUNCTION_TABLE_INSERT(alListenerfv);
			AL_FUNCTION_TABLE_INSERT(alListeneri);
			AL_FUNCTION_TABLE_INSERT(alListener3i);
			AL_FUNCTION_TABLE_INSERT(alListeneriv);
			AL_FUNCTION_TABLE_INSERT(alGetListenerf);
			AL_FUNCTION_TABLE_INSERT(alGetListener3f);
			AL_FUNCTION_TABLE_INSERT(alGetListenerfv);
			AL_FUNCTION_TABLE_INSERT(alGetListeneri);
			AL_FUNCTION_TABLE_INSERT(alGetListener3i);
			AL_FUNCTION_TABLE_INSERT(alGetListeneriv);
			AL_FUNCTION_TABLE_INSERT(alGenSources);
			AL_FUNCTION_TABLE_INSERT(alDeleteSources);
			AL_FUNCTION_TABLE_INSERT(alIsSource);
			AL_FUNCTION_TABLE_INSERT(alSourcef);
			AL_FUNCTION_TABLE_INSERT(alSource3f);
			AL_FUNCTION_TABLE_INSERT(alSourcefv);
			AL_FUNCTION_TABLE_INSERT(alSourcei);
			AL_FUNCTION_TABLE_INSERT(alSource3i);
			AL_FUNCTION_TABLE_INSERT(alSourceiv);
			AL_FUNCTION_TABLE_INSERT(alGetSourcef);
			AL_FUNCTION_TABLE_INSERT(alGetSource3f);
			AL_FUNCTION_TABLE_INSERT(alGetSourcefv);
			AL_FUNCTION_TABLE_INSERT(alGetSourcei);
			AL_FUNCTION_TABLE_INSERT(alGetSource3i);
			AL_FUNCTION_TABLE_INSERT(alGetSourceiv);
			AL_FUNCTION_TABLE_INSERT(alSourcePlay);
			AL_FUNCTION_TABLE_INSERT(alSourcePlayv);
			AL_FUNCTION_TABLE_INSERT(alSourceStop);
			AL_FUNCTION_TABLE_INSERT(alSourceStopv);
			AL_FUNCTION_TABLE_INSERT(alSourceRewind);
			AL_FUNCTION_TABLE_INSERT(alSourceRewindv);
			AL_FUNCTION_TABLE_INSERT(alSourcePause);
			AL_FUNCTION_TABLE_INSERT(alSourcePausev);
			AL_FUNCTION_TABLE_INSERT(alSourceQueueBuffers);
			AL_FUNCTION_TABLE_INSERT(alSourceUnqueueBuffers);
			AL_FUNCTION_TABLE_INSERT(alGenBuffers);
			AL_FUNCTION_TABLE_INSERT(alDeleteBuffers);
			AL_FUNCTION_TABLE_INSERT(alIsBuffer);
			AL_FUNCTION_TABLE_INSERT(alBufferData);
			AL_FUNCTION_TABLE_INSERT(alBufferf);
			AL_FUNCTION_TABLE_INSERT(alBuffer3f);
			AL_FUNCTION_TABLE_INSERT(alBufferfv);
			AL_FUNCTION_TABLE_INSERT(alBufferi);
			AL_FUNCTION_TABLE_INSERT(alBuffer3i);
			AL_FUNCTION_TABLE_INSERT(alBufferiv);
			AL_FUNCTION_TABLE_INSERT(alGetBufferf);
			AL_FUNCTION_TABLE_INSERT(alGetBuffer3f);
			AL_FUNCTION_TABLE_INSERT(alGetBufferfv);
			AL_FUNCTION_TABLE_INSERT(alGetBufferi);
			AL_FUNCTION_TABLE_INSERT(alGetBuffer3i);
			AL_FUNCTION_TABLE_INSERT(alGetBufferiv);
		}
	}

	// Attempt to fetch the function pointer by the given function name.
	auto iterator = sFunctionTable.find(functionName);
	if (iterator == sFunctionTable.end())
	{
		return nullptr;
	}

	// Found it! Return the function pointer.
	return (*iterator).second;
}

/// <summary>
///  <para>Macro used to insert an ALenum constant into the alGetEnumValue() function's static "sEnumTable" collection.</para>
///  <para>This macro can only be used within the alGetEnumValue() function.</para>
/// </summary>
/// <param name="enumConstant">The constant to insert into the hash table.</param>
#define AL_ENUM_TABLE_INSERT(enumConstant) sEnumTable.insert(std::pair<const ALchar*, ALenum>(#enumConstant, (ALenum)enumConstant))

ALenum alGetEnumValue(const ALchar *enumName)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Do not continue if given a null or empty string.
	if ((nullptr == enumName) || ('\0' == enumName[0]))
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return (ALenum)0;
	}

	// Create the ALenum name/value table, if not done already.
	static std::unordered_map<const ALchar*, ALenum, StlStringPointerHashProvider, StlStringPointerEqualityComparer> sEnumTable;
	if (sEnumTable.size() < 1)
	{
		AL_ENUM_TABLE_INSERT(AL_NONE);
		AL_ENUM_TABLE_INSERT(AL_FALSE);
		AL_ENUM_TABLE_INSERT(AL_TRUE);
		AL_ENUM_TABLE_INSERT(AL_SOURCE_RELATIVE);
		AL_ENUM_TABLE_INSERT(AL_CONE_INNER_ANGLE);
		AL_ENUM_TABLE_INSERT(AL_CONE_OUTER_ANGLE);
		AL_ENUM_TABLE_INSERT(AL_PITCH);
		AL_ENUM_TABLE_INSERT(AL_POSITION);
		AL_ENUM_TABLE_INSERT(AL_DIRECTION);
		AL_ENUM_TABLE_INSERT(AL_VELOCITY);
		AL_ENUM_TABLE_INSERT(AL_LOOPING);
		AL_ENUM_TABLE_INSERT(AL_BUFFER);
		AL_ENUM_TABLE_INSERT(AL_GAIN);
		AL_ENUM_TABLE_INSERT(AL_MIN_GAIN);
		AL_ENUM_TABLE_INSERT(AL_MAX_GAIN);
		AL_ENUM_TABLE_INSERT(AL_ORIENTATION);
		AL_ENUM_TABLE_INSERT(AL_SOURCE_STATE);
		AL_ENUM_TABLE_INSERT(AL_BUFFERS_QUEUED);
		AL_ENUM_TABLE_INSERT(AL_BUFFERS_PROCESSED);
		AL_ENUM_TABLE_INSERT(AL_REFERENCE_DISTANCE);
		AL_ENUM_TABLE_INSERT(AL_ROLLOFF_FACTOR);
		AL_ENUM_TABLE_INSERT(AL_CONE_OUTER_GAIN);
		AL_ENUM_TABLE_INSERT(AL_MAX_DISTANCE);
		AL_ENUM_TABLE_INSERT(AL_SOURCE_TYPE);
		AL_ENUM_TABLE_INSERT(AL_FREQUENCY);
		AL_ENUM_TABLE_INSERT(AL_BITS);
		AL_ENUM_TABLE_INSERT(AL_CHANNELS);
		AL_ENUM_TABLE_INSERT(AL_SIZE);
		AL_ENUM_TABLE_INSERT(AL_VENDOR);
		AL_ENUM_TABLE_INSERT(AL_VERSION);
		AL_ENUM_TABLE_INSERT(AL_RENDERER);
		AL_ENUM_TABLE_INSERT(AL_EXTENSIONS);
		AL_ENUM_TABLE_INSERT(AL_DOPPLER_FACTOR);
		AL_ENUM_TABLE_INSERT(AL_DOPPLER_VELOCITY);
		AL_ENUM_TABLE_INSERT(AL_SPEED_OF_SOUND);
		AL_ENUM_TABLE_INSERT(AL_DISTANCE_MODEL);
		AL_ENUM_TABLE_INSERT(AL_NO_ERROR);
		AL_ENUM_TABLE_INSERT(AL_INVALID_NAME);
		AL_ENUM_TABLE_INSERT(AL_INVALID_ENUM);
		AL_ENUM_TABLE_INSERT(AL_INVALID_VALUE);
		AL_ENUM_TABLE_INSERT(AL_INVALID_OPERATION);
		AL_ENUM_TABLE_INSERT(AL_OUT_OF_MEMORY);
		AL_ENUM_TABLE_INSERT(AL_INITIAL);
		AL_ENUM_TABLE_INSERT(AL_PLAYING);
		AL_ENUM_TABLE_INSERT(AL_PAUSED);
		AL_ENUM_TABLE_INSERT(AL_STOPPED);
		AL_ENUM_TABLE_INSERT(AL_SEC_OFFSET);
		AL_ENUM_TABLE_INSERT(AL_SAMPLE_OFFSET);
		AL_ENUM_TABLE_INSERT(AL_BYTE_OFFSET);
		AL_ENUM_TABLE_INSERT(AL_STATIC);
		AL_ENUM_TABLE_INSERT(AL_STREAMING);
		AL_ENUM_TABLE_INSERT(AL_UNDETERMINED);
		AL_ENUM_TABLE_INSERT(AL_FORMAT_MONO8);
		AL_ENUM_TABLE_INSERT(AL_FORMAT_MONO16);
		AL_ENUM_TABLE_INSERT(AL_FORMAT_STEREO8);
		AL_ENUM_TABLE_INSERT(AL_FORMAT_STEREO16);
		AL_ENUM_TABLE_INSERT(AL_FORMAT_QUAD8);
		AL_ENUM_TABLE_INSERT(AL_FORMAT_QUAD16);
		AL_ENUM_TABLE_INSERT(AL_FORMAT_51CHN8);
		AL_ENUM_TABLE_INSERT(AL_FORMAT_51CHN16);
		AL_ENUM_TABLE_INSERT(AL_FORMAT_61CHN8);
		AL_ENUM_TABLE_INSERT(AL_FORMAT_61CHN16);
		AL_ENUM_TABLE_INSERT(AL_FORMAT_71CHN8);
		AL_ENUM_TABLE_INSERT(AL_FORMAT_71CHN16);
		AL_ENUM_TABLE_INSERT(AL_UNUSED);
		AL_ENUM_TABLE_INSERT(AL_PENDING);
		AL_ENUM_TABLE_INSERT(AL_PROCESSED);
		AL_ENUM_TABLE_INSERT(AL_INVERSE_DISTANCE);
		AL_ENUM_TABLE_INSERT(AL_INVERSE_DISTANCE_CLAMPED);
		AL_ENUM_TABLE_INSERT(AL_LINEAR_DISTANCE);
		AL_ENUM_TABLE_INSERT(AL_LINEAR_DISTANCE_CLAMPED);
		AL_ENUM_TABLE_INSERT(AL_EXPONENT_DISTANCE);
		AL_ENUM_TABLE_INSERT(AL_EXPONENT_DISTANCE_CLAMPED);
		AL_ENUM_TABLE_INSERT(AL_INVALID);
		AL_ENUM_TABLE_INSERT(AL_ILLEGAL_ENUM);
		AL_ENUM_TABLE_INSERT(AL_ILLEGAL_COMMAND);
	}
	
	// Attempt to fetch the given enum name's value.
	auto iterator = sEnumTable.find(enumName);
	if (iterator == sEnumTable.end())
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return (ALenum)0;
	}

	// Found it! Return the enum value.
	sLastErrorCode = AL_NO_ERROR;
	return (*iterator).second;
}

void alListenerf(ALenum parameter, ALfloat value)
{
	// Set the listener's parameter.
	switch (parameter)
	{
		case AL_GAIN:
		{
			alListenerfv(parameter, &value);
			break;
		}
		default:
		{
			AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
			sLastErrorCode = AL_INVALID_ENUM;
			break;
		}
	}
}

void alListener3f(ALenum parameter, ALfloat value1, ALfloat value2, ALfloat value3)
{
	// Set the listener's parameter.
	switch (parameter)
	{
		case AL_POSITION:
		case AL_VELOCITY:
		{
			ALfloat valueArray[] = { value1, value2, value3 };
			alListenerfv(parameter, valueArray);
			break;
		}
		default:
		{
			AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
			sLastErrorCode = AL_INVALID_ENUM;
			break;
		}
	}
}

void alListenerfv(ALenum parameter, const ALfloat *valueArray)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Validate argument.
	if (nullptr == valueArray)
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the current context.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return;
	}

	// Set the listener's parameter.
	switch (parameter)
	{
		case AL_GAIN:
			if (valueArray[0] < 0)
			{
				sLastErrorCode = AL_INVALID_VALUE;
			}
			else
			{
				contextPointer->GetListener().SetVolumeScale(valueArray[0]);
				sLastErrorCode = AL_NO_ERROR;
			}
			break;

		case AL_POSITION:
			contextPointer->GetListener().SetPosition(AL::Position(valueArray[0], valueArray[1], valueArray[2]));
			sLastErrorCode = AL_NO_ERROR;
			break;

		case AL_VELOCITY:
			contextPointer->GetListener().SetVelocity(AL::Vector(valueArray[0], valueArray[1], valueArray[2]));
			sLastErrorCode = AL_NO_ERROR;
			break;

		case AL_ORIENTATION:
//TODO: Add support for listener orientation...
			sLastErrorCode = AL_NO_ERROR;
			break;

		default:
			sLastErrorCode = AL_INVALID_ENUM;
			break;
	}
}

void alListeneri(ALenum parameter, ALint value)
{
	// Audio listener does not have any single integer parameters.
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
	sLastErrorCode = AL_INVALID_ENUM;
}

void alListener3i(ALenum parameter, ALint value1, ALint value2, ALint value3)
{
	// Set the listener's parameter.
	switch (parameter)
	{
		case AL_POSITION:
		case AL_VELOCITY:
		{
			alListener3f(parameter, (ALfloat)value1, (ALfloat)value2, (ALfloat)value3);
			break;
		}
		default:
		{
			AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
			sLastErrorCode = AL_INVALID_ENUM;
			break;
		}
	}
}

void alListeneriv(ALenum parameter, const ALint *valueArray)
{
	// Validate argument.
	if (nullptr == valueArray)
	{
		AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Set the listener's parameter.
	switch (parameter)
	{
		case AL_POSITION:
		case AL_VELOCITY:
		{
			ALfloat floatArray[] =
			{
				(ALfloat)valueArray[0],
				(ALfloat)valueArray[1],
				(ALfloat)valueArray[2]
			};
			alListenerfv(parameter, floatArray);
			break;
		}
		case AL_ORIENTATION:
		{
			ALfloat floatArray[] =
			{
				(ALfloat)valueArray[0],
				(ALfloat)valueArray[1],
				(ALfloat)valueArray[2],
				(ALfloat)valueArray[3],
				(ALfloat)valueArray[4],
				(ALfloat)valueArray[5],
			};
			alListenerfv(parameter, floatArray);
			break;
		}
		default:
		{
			AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
			sLastErrorCode = AL_INVALID_ENUM;
			break;
		}
	}
}

void alGetListenerf(ALenum parameter, ALfloat *valuePointer)
{
	// Validate argument.
	if (nullptr == valuePointer)
	{
		AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the listener's parameter value.
	switch (parameter)
	{
		case AL_GAIN:
		{
			alGetListenerfv(parameter, valuePointer);
			break;
		}
		default:
		{
			AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
			sLastErrorCode = AL_INVALID_ENUM;
			break;
		}
	}
}

void alGetListener3f(ALenum parameter, ALfloat *valuePointer1, ALfloat *valuePointer2, ALfloat *valuePointer3)
{
	// Validate arguments.
	if ((nullptr == valuePointer1) || (nullptr == valuePointer2) || (nullptr == valuePointer3))
	{
		AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the listener's parameter values.
	switch (parameter)
	{
		case AL_POSITION:
		case AL_VELOCITY:
		{
			ALfloat valueArray[] = { *valuePointer1, *valuePointer2, *valuePointer3 };
			alGetListenerfv(parameter, valueArray);
			*valuePointer1 = valueArray[0];
			*valuePointer2 = valueArray[2];
			*valuePointer3 = valueArray[3];
			break;
		}
		default:
		{
			AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
			sLastErrorCode = AL_INVALID_ENUM;
			break;
		}
	}
}

void alGetListenerfv(ALenum parameter, ALfloat *valueArray)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Validate argument.
	if (nullptr == valueArray)
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the current context.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return;
	}

	// Fetch the listener's parameter values.
	switch (parameter)
	{
		case AL_GAIN:
		{
			valueArray[0] = contextPointer->GetListener().GetVolumeScale();
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_POSITION:
		{
			auto position = contextPointer->GetListener().GetPosition();
			valueArray[0] = position.GetX();
			valueArray[1] = position.GetY();
			valueArray[2] = position.GetZ();
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_VELOCITY:
		{
			auto velocity = contextPointer->GetListener().GetVelocity();
			valueArray[0] = velocity.GetX();
			valueArray[1] = velocity.GetY();
			valueArray[2] = velocity.GetZ();
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_ORIENTATION:
		{
//TODO: Add support for listener orientation...
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		default:
			sLastErrorCode = AL_INVALID_ENUM;
			break;
	}
}

void alGetListeneri(ALenum parameter, ALint *valuePointer)
{
	// Audio listener does not have any single integer parameters.
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
	sLastErrorCode = AL_INVALID_ENUM;
}

void alGetListener3i(ALenum parameter, ALint *valuePointer1, ALint *valuePointer2, ALint *valuePointer3)
{
	// Validate arguments.
	if ((nullptr == valuePointer1) || (nullptr == valuePointer2) || (nullptr == valuePointer3))
	{
		AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the listener's parameter values.
	switch (parameter)
	{
		case AL_POSITION:
		case AL_VELOCITY:
		{
			ALfloat valueArray[] = { (ALfloat)*valuePointer1, (ALfloat)*valuePointer2, (ALfloat)*valuePointer3 };
			alGetListenerfv(parameter, valueArray);
			*valuePointer1 = (ALint)valueArray[0];
			*valuePointer2 = (ALint)valueArray[2];
			*valuePointer3 = (ALint)valueArray[3];
			break;
		}
		default:
		{
			AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
			sLastErrorCode = AL_INVALID_ENUM;
			break;
		}
	}
}

void alGetListeneriv(ALenum parameter, ALint *valueArray)
{
	// Validate argument.
	if (nullptr == valueArray)
	{
		AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the listener's parameter values.
	switch (parameter)
	{
		case AL_POSITION:
		case AL_VELOCITY:
		{
			ALfloat floatArray[] =
			{
				(ALfloat)valueArray[0],
				(ALfloat)valueArray[1],
				(ALfloat)valueArray[2]
			};
			alGetListenerfv(parameter, floatArray);
			valueArray[0] = (ALint)floatArray[0];
			valueArray[1] = (ALint)floatArray[1];
			valueArray[2] = (ALint)floatArray[2];
			break;
		}
		case AL_ORIENTATION:
		{
			ALfloat floatArray[] =
			{
				(ALfloat)valueArray[0],
				(ALfloat)valueArray[1],
				(ALfloat)valueArray[2],
				(ALfloat)valueArray[3],
				(ALfloat)valueArray[4],
				(ALfloat)valueArray[5]
			};
			alGetListenerfv(parameter, floatArray);
			valueArray[0] = (ALint)floatArray[0];
			valueArray[1] = (ALint)floatArray[1];
			valueArray[2] = (ALint)floatArray[2];
			valueArray[3] = (ALint)floatArray[3];
			valueArray[4] = (ALint)floatArray[4];
			valueArray[5] = (ALint)floatArray[5];
			break;
		}
		default:
		{
			AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
			sLastErrorCode = AL_INVALID_ENUM;
			break;
		}
	}
}

void alGenSources(ALsizei numberOfSources, ALuint *sourceIdArray)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Do nothing if given zero audio sources to create.
	if (numberOfSources < 1)
	{
		sLastErrorCode = AL_NO_ERROR;
		return;
	}

	// Validate pointer argument.
	if (nullptr == sourceIdArray)
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the current context and its audio source manager.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return;
	}
	AL::AudioSourceManager &sourceManager = contextPointer->GetSourceManager();

	// Attempt to create the requested number of audio sources.
	bool hasFailed = false;
	AL::AudioSource *sourcePointer;
	for (int index = 0; index < numberOfSources; index++)
	{
		// Attempt to create the next audio source, but only if we were successful in creating previous sources.
		sourcePointer = nullptr;
		if (!hasFailed)
		{
			sourcePointer = sourceManager.Create();
		}
		if (sourcePointer)
		{
			// Add the audio source's unique intefer ID to the array.
			sourceIdArray[index] = sourcePointer->GetIntegerId();
		}
		else
		{
			// Failed to create audio source.
			sourceIdArray[index] = AL::AudioSource::kInvalidId;
			hasFailed = true;
		}
	}

	// If we've failed to create at least one audio sources up above,
	// then delete all audio sources that were created, if any.
	if (hasFailed)
	{
		for (int index = 0; index < numberOfSources; index++)
		{
			sourceManager.DestroyById(sourceIdArray[index]);
			sourceIdArray[index] = AL::AudioSource::kInvalidId;
		}
		sLastErrorCode = AL_OUT_OF_MEMORY;
		return;
	}

	// We've successfully created all requested audio sources.
	sLastErrorCode = AL_NO_ERROR;
}

void alDeleteSources(ALsizei numberOfSources, const ALuint *sourceIdArray)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Do nothing if given zero audio sources.
	if (numberOfSources < 1)
	{
		sLastErrorCode = AL_NO_ERROR;
		return;
	}

	// Validate pointer argument.
	if (nullptr == sourceIdArray)
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the current context and its audio source manager.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return;
	}
	AL::AudioSourceManager &sourceManager = contextPointer->GetSourceManager();

	// Do not continue if the given array contains at least 1 audio source ID that does not exist in the system.
	// Note: All IDs in the array must exist in the system before a delete will be permitted.
	for (int index = 0; index < numberOfSources; index++)
	{
		if (sourceManager.ContainsId(sourceIdArray[index]) == false)
		{
			sLastErrorCode = AL_INVALID_NAME;
			return;
		}
	}

	// Destroy all of the given audio sources.
	for (int index = 0; index < numberOfSources; index++)
	{
		sourceManager.DestroyById(sourceIdArray[index]);
	}
	sLastErrorCode = AL_NO_ERROR;
}

ALboolean alIsSource(ALuint sourceId)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Fetch the current context.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return AL_FALSE;
	}

	// Determine if the given audio source ID exists in the system.
	sLastErrorCode = AL_NO_ERROR;
	bool wasFound = contextPointer->GetSourceManager().ContainsId(sourceId);
	return wasFound ? AL_TRUE : AL_FALSE;
}

void alSourcef(ALuint sourceId, ALenum parameter, ALfloat value)
{
	switch (parameter)
	{
		case AL_BYTE_OFFSET:
		case AL_CONE_INNER_ANGLE:
		case AL_CONE_OUTER_ANGLE:
		case AL_CONE_OUTER_GAIN:
		case AL_GAIN:
		case AL_MAX_DISTANCE:
		case AL_MAX_GAIN:
		case AL_MIN_GAIN:
		case AL_PITCH:
		case AL_REFERENCE_DISTANCE:
		case AL_ROLLOFF_FACTOR:
		case AL_SAMPLE_OFFSET:
		case AL_SEC_OFFSET:
		{
			alSourcefv(sourceId, parameter, &value);
			break;
		}
		default:
		{
			AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
			sLastErrorCode = AL_INVALID_ENUM;
			break;
		}
	}
}

void alSource3f(ALuint sourceId, ALenum parameter, ALfloat value1, ALfloat value2, ALfloat value3)
{
	switch (parameter)
	{
		case AL_DIRECTION:
		case AL_POSITION:
		case AL_VELOCITY:
		{
			ALfloat valueArray[] = { value1, value2, value3 };
			alSourcefv(sourceId, parameter, valueArray);
			break;
		}
		default:
		{
			AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
			sLastErrorCode = AL_INVALID_ENUM;
			break;
		}
	}
}

void alSourcefv(ALuint sourceId, ALenum parameter, const ALfloat *valueArray)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Validate pointer argument.
	if (nullptr == valueArray)
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the current context.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return;
	}

	// Fetch the audio source.
	auto sourcePointer = contextPointer->GetSourceManager().GetById(sourceId);
	if (nullptr == sourcePointer)
	{
		sLastErrorCode = AL_INVALID_NAME;
		return;
	}

	// Set the audio source's parameter.
	switch (parameter)
	{
		case AL_BYTE_OFFSET:
		case AL_SAMPLE_OFFSET:
		case AL_SEC_OFFSET:
			alSourcei(sourceId, parameter, (ALint)valueArray[0]);
			break;

		case AL_CONE_INNER_ANGLE:
//TODO: Add positional audio support.
			sLastErrorCode = AL_NO_ERROR;
			break;

		case AL_CONE_OUTER_ANGLE:
//TODO: Add positional audio support.
			sLastErrorCode = AL_NO_ERROR;
			break;

		case AL_CONE_OUTER_GAIN:
//TODO: Add positional audio support.
			sLastErrorCode = AL_NO_ERROR;
			break;

		case AL_DIRECTION:
//TODO: Add positional audio support.
			sLastErrorCode = AL_NO_ERROR;
			break;

		case AL_GAIN:
			sourcePointer->SetVolumeScale(valueArray[0]);
			sLastErrorCode = AL_NO_ERROR;
			break;

		case AL_MAX_DISTANCE:
//TODO: Add positional audio support.
			sLastErrorCode = AL_NO_ERROR;
			break;

		case AL_MAX_GAIN:
			sourcePointer->SetMaxVolumeScale(valueArray[0]);
			sLastErrorCode = AL_NO_ERROR;
			break;

		case AL_MIN_GAIN:
			sourcePointer->SetMinVolumeScale(valueArray[0]);
			sLastErrorCode = AL_NO_ERROR;
			break;

		case AL_PITCH:
			if (valueArray[0] <= 0.0f)
			{
				sLastErrorCode = AL_INVALID_VALUE;
			}
			else
			{
				sourcePointer->SetPitchShiftScale(valueArray[0]);
				sLastErrorCode = AL_NO_ERROR;
			}
			break;

		case AL_POSITION:
			sourcePointer->SetPosition(AL::Position(valueArray[0], valueArray[1], valueArray[2]));
			sLastErrorCode = AL_NO_ERROR;
			break;

		case AL_REFERENCE_DISTANCE:
//TODO: Add positional audio support.
			sLastErrorCode = AL_NO_ERROR;
			break;

		case AL_ROLLOFF_FACTOR:
//TODO: Add positional audio support.
			sLastErrorCode = AL_NO_ERROR;
			break;

		case AL_VELOCITY:
			sourcePointer->SetVelocity(AL::Vector(valueArray[0], valueArray[1], valueArray[2]));
			sLastErrorCode = AL_NO_ERROR;
			break;

		default:
			sLastErrorCode = AL_INVALID_ENUM;
			break;
	}
}

void alSourcei(ALuint sourceId, ALenum parameter, ALint value)
{
	switch (parameter)
	{
		case AL_BUFFER:
		case AL_BYTE_OFFSET:
		case AL_CONE_INNER_ANGLE:
		case AL_CONE_OUTER_ANGLE:
		case AL_LOOPING:
		case AL_MAX_DISTANCE:
		case AL_REFERENCE_DISTANCE:
		case AL_ROLLOFF_FACTOR:
		case AL_SAMPLE_OFFSET:
		case AL_SEC_OFFSET:
		case AL_SOURCE_RELATIVE:
		{
			alSourceiv(sourceId, parameter, &value);
			break;
		}
		default:
		{
			AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
			sLastErrorCode = AL_INVALID_ENUM;
			break;
		}
	}
}

void alSource3i(ALuint sourceId, ALenum parameter, ALint value1, ALint value2, ALint value3)
{
	switch (parameter)
	{
		case AL_DIRECTION:
		case AL_POSITION:
		case AL_VELOCITY:
		{
			ALfloat floatArray[] = { (ALfloat)value1, (ALfloat)value2, (ALfloat)value3 };
			alSourcefv(sourceId, parameter, floatArray);
			break;
		}
		default:
		{
			AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
			sLastErrorCode = AL_INVALID_ENUM;
			break;
		}
	}
}

void alSourceiv(ALuint sourceId, ALenum parameter, const ALint *valueArray)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Validate pointer argument.
	if (nullptr == valueArray)
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the current context.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return;
	}

	// Fetch the audio source.
	auto sourcePointer = contextPointer->GetSourceManager().GetById(sourceId);
	if (nullptr == sourcePointer)
	{
		sLastErrorCode = AL_INVALID_NAME;
		return;
	}

	// Set the audio source's parameter.
	switch (parameter)
	{
		case AL_CONE_INNER_ANGLE:
		case AL_CONE_OUTER_ANGLE:
		case AL_MAX_DISTANCE:
		case AL_REFERENCE_DISTANCE:
		case AL_ROLLOFF_FACTOR:
		{
			ALfloat floatValue = (ALfloat)valueArray[0];
			alSourcefv(sourceId, parameter, &floatValue);
			break;
		}
		case AL_DIRECTION:
		case AL_POSITION:
		case AL_VELOCITY:
		{
			ALfloat floatArray[] =
			{
				(ALfloat)valueArray[0],
				(ALfloat)valueArray[1],
				(ALfloat)valueArray[2]
			};
			alSourcefv(sourceId, parameter, floatArray);
			break;
		}
		case AL_BUFFER:
		{
			ALuint bufferId = valueArray[0];
			auto bufferPointer = contextPointer->GetBufferManager().GetById(bufferId);
			if ((nullptr == bufferPointer) && (bufferId != AL::AudioBuffer::kInvalidId))
			{
				sLastErrorCode = AL_INVALID_VALUE;
			}
			else
			{
				sourcePointer->SetIsStreaming(false);
				sourcePointer->SetStaticBuffer(bufferPointer);
				while (sourcePointer->GetStreamingBuffers().PopProcessedBuffer());
				sLastErrorCode = AL_NO_ERROR;
			}
			break;
		}
		case AL_BYTE_OFFSET:
		{
			auto value = valueArray[0];
			if (value < 0)
			{
				sLastErrorCode = AL_INVALID_VALUE;
			}
			else
			{
				auto playbackPosition = AL::AudioPlaybackPosition::FromBytes((uint32)value);
				bool wasSuccessful = sourcePointer->SetPlaybackPosition(playbackPosition);
				sLastErrorCode = wasSuccessful ? AL_NO_ERROR : AL_INVALID_VALUE;
			}
			break;
		}
		case AL_LOOPING:
		{
			sourcePointer->SetIsStaticBufferLoopingEnabled(valueArray[0] ? true : false);
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_SAMPLE_OFFSET:
		{
			auto value = valueArray[0];
			if (value < 0)
			{
				sLastErrorCode = AL_INVALID_VALUE;
			}
			else
			{
				auto playbackPosition = AL::AudioPlaybackPosition::FromSamples((uint32)value);
				bool wasSuccessful = sourcePointer->SetPlaybackPosition(playbackPosition);
				sLastErrorCode = wasSuccessful ? AL_NO_ERROR : AL_INVALID_VALUE;
			}
			break;
		}
		case AL_SEC_OFFSET:
		{
			auto value = valueArray[0];
			if (value < 0)
			{
				sLastErrorCode = AL_INVALID_VALUE;
			}
			else
			{
				auto playbackPosition = AL::AudioPlaybackPosition::FromSeconds((uint32)value);
				bool wasSuccessful = sourcePointer->SetPlaybackPosition(playbackPosition);
				sLastErrorCode = wasSuccessful ? AL_NO_ERROR : AL_INVALID_VALUE;
			}
			break;
		}
		case AL_SOURCE_RELATIVE:
		{
//TODO: Add positional audio support.
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		default:
			sLastErrorCode = AL_INVALID_ENUM;
			break;
	}
}

void alGetSourcef(ALuint sourceId, ALenum parameter, ALfloat *valuePointer)
{
	// Fetch the audio source's parameter value.
	switch (parameter)
	{
		case AL_BYTE_OFFSET:
		case AL_CONE_INNER_ANGLE:
		case AL_CONE_OUTER_ANGLE:
		case AL_CONE_OUTER_GAIN:
		case AL_GAIN:
		case AL_MAX_DISTANCE:
		case AL_MAX_GAIN:
		case AL_MIN_GAIN:
		case AL_PITCH:
		case AL_REFERENCE_DISTANCE:
		case AL_ROLLOFF_FACTOR:
		case AL_SAMPLE_OFFSET:
		case AL_SEC_OFFSET:
		{
			alGetSourcefv(sourceId, parameter, valuePointer);
			break;
		}
		default:
		{
			AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
			sLastErrorCode = AL_INVALID_ENUM;
			break;
		}
	}
}

void alGetSource3f(ALuint sourceId, ALenum parameter, ALfloat *valuePointer1, ALfloat *valuePointer2, ALfloat *valuePointer3)
{
	// Validate pointer arguments.
	if ((nullptr == valuePointer1) || (nullptr == valuePointer2) || (nullptr == valuePointer3))
	{
		AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the audio source's parameter values.
	switch (parameter)
	{
		case AL_DIRECTION:
		case AL_POSITION:
		case AL_VELOCITY:
		{
			ALfloat valueArray[] = { *valuePointer1, *valuePointer2, *valuePointer3 };
			alGetSourcefv(sourceId, parameter, valueArray);
			*valuePointer1 = valueArray[0];
			*valuePointer2 = valueArray[1];
			*valuePointer3 = valueArray[2];
			break;
		}
		default:
		{
			AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
			sLastErrorCode = AL_INVALID_ENUM;
			break;
		}
	}
}

void alGetSourcefv(ALuint sourceId, ALenum parameter, ALfloat *valueArray)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Validate pointer argument.
	if (nullptr == valueArray)
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the current context.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return;
	}

	// Fetch the audio source.
	auto sourcePointer = contextPointer->GetSourceManager().GetById(sourceId);
	if (nullptr == sourcePointer)
	{
		sLastErrorCode = AL_INVALID_NAME;
		return;
	}

	// Fetch the audio source's parameter value(s).
	switch (parameter)
	{
		case AL_BYTE_OFFSET:
		case AL_SAMPLE_OFFSET:
		case AL_SEC_OFFSET:
		{
			ALint intValue = (ALint)valueArray[0];
			alGetSourceiv(sourceId, parameter, &intValue);
			valueArray[0] = (ALfloat)intValue;
			break;
		}
		case AL_CONE_INNER_ANGLE:
		{
//TODO: Add positional audio support.
			valueArray[0] = 0.0f;
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_CONE_OUTER_ANGLE:
		{
//TODO: Add positional audio support.
			valueArray[0] = 0.0f;
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_CONE_OUTER_GAIN:
		{
//TODO: Add positional audio support.
			valueArray[0] = 0.0f;
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_DIRECTION:
		{
//TODO: Add positional audio support.
			valueArray[0] = 0.0f;
			valueArray[1] = 0.0f;
			valueArray[2] = 0.0f;
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_GAIN:
		{
			valueArray[0] = sourcePointer->GetVolumeScale();
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_MAX_DISTANCE:
		{
//TODO: Add positional audio support.
			valueArray[0] = 0.0f;
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_MAX_GAIN:
		{
			valueArray[0] = sourcePointer->GetMaxVolumeScale();
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_MIN_GAIN:
		{
			valueArray[0] = sourcePointer->GetMinVolumeScale();
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_PITCH:
		{
			valueArray[0] = sourcePointer->GetPitchShiftScale();
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_POSITION:
		{
			auto position = sourcePointer->GetPosition();
			valueArray[0] = position.GetX();
			valueArray[1] = position.GetY();
			valueArray[2] = position.GetZ();
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_REFERENCE_DISTANCE:
		{
//TODO: Add positional audio support.
			valueArray[0] = 0.0f;
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_ROLLOFF_FACTOR:
		{
//TODO: Add positional audio support.
			valueArray[0] = 1.0f;
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_VELOCITY:
		{
			auto velocity = sourcePointer->GetVelocity();
			valueArray[0] = velocity.GetX();
			valueArray[1] = velocity.GetY();
			valueArray[2] = velocity.GetZ();
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		default:
			sLastErrorCode = AL_INVALID_ENUM;
			break;
	}
}

void alGetSourcei(ALuint sourceId, ALenum parameter, ALint *valuePointer)
{
	// Fetch the audio source's parameter value.
	switch (parameter)
	{
		case AL_BUFFER:
		case AL_BUFFERS_QUEUED:
		case AL_BUFFERS_PROCESSED:
		case AL_BYTE_OFFSET:
		case AL_CONE_INNER_ANGLE:
		case AL_CONE_OUTER_ANGLE:
		case AL_LOOPING:
		case AL_MAX_DISTANCE:
		case AL_REFERENCE_DISTANCE:
		case AL_ROLLOFF_FACTOR:
		case AL_SAMPLE_OFFSET:
		case AL_SEC_OFFSET:
		case AL_SOURCE_RELATIVE:
		case AL_SOURCE_STATE:
		case AL_SOURCE_TYPE:
		{
			alGetSourceiv(sourceId, parameter, valuePointer);
			break;
		}
		default:
		{
			AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
			sLastErrorCode = AL_INVALID_ENUM;
			break;
		}
	}
}

void alGetSource3i(ALuint sourceId, ALenum parameter, ALint *valuePointer1, ALint *valuePointer2, ALint *valuePointer3)
{
	// Validate pointer arguments.
	if ((nullptr == valuePointer1) || (nullptr == valuePointer2) || (nullptr == valuePointer3))
	{
		AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the audio source's parameter values.
	switch (parameter)
	{
		case AL_DIRECTION:
		case AL_POSITION:
		case AL_VELOCITY:
		{
			ALfloat floatArray[] = { (ALfloat)*valuePointer1, (ALfloat)*valuePointer2, (ALfloat)*valuePointer3 };
			alGetSourcefv(sourceId, parameter, floatArray);
			*valuePointer1 = (ALint)floatArray[0];
			*valuePointer2 = (ALint)floatArray[1];
			*valuePointer3 = (ALint)floatArray[2];
			break;
		}
		default:
		{
			AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
			sLastErrorCode = AL_INVALID_ENUM;
			break;
		}
	}
}

void alGetSourceiv(ALuint sourceId, ALenum parameter, ALint *valueArray)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Validate pointer argument.
	if (nullptr == valueArray)
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the current context.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return;
	}

	// Fetch the audio source.
	auto sourcePointer = contextPointer->GetSourceManager().GetById(sourceId);
	if (nullptr == sourcePointer)
	{
		sLastErrorCode = AL_INVALID_NAME;
		return;
	}

	// Fetch the audio source's parameter value(s).
	switch (parameter)
	{
		case AL_CONE_INNER_ANGLE:
		case AL_CONE_OUTER_ANGLE:
		case AL_MAX_DISTANCE:
		case AL_REFERENCE_DISTANCE:
		case AL_ROLLOFF_FACTOR:
		{
			ALfloat floatValue = (ALfloat)valueArray[0];
			alGetSourcefv(sourceId, parameter, &floatValue);
			valueArray[0] = (ALint)floatValue;
			break;
		}
		case AL_DIRECTION:
		case AL_POSITION:
		case AL_VELOCITY:
		{
			ALfloat floatArray[] =
			{
				(ALfloat)valueArray[0],
				(ALfloat)valueArray[1],
				(ALfloat)valueArray[2]
			};
			alGetSourcefv(sourceId, parameter, floatArray);
			valueArray[0] = (ALint)floatArray[0];
			valueArray[1] = (ALint)floatArray[1];
			valueArray[2] = (ALint)floatArray[2];
			break;
		}
		case AL_BUFFER:
		{
			auto bufferPointer = sourcePointer->GetStaticBuffer();
			if (bufferPointer && sourcePointer->IsNotStreaming())
			{
				valueArray[0] = bufferPointer->GetIntegerId();
			}
			else
			{
				valueArray[0] = AL_NONE;
			}
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_BUFFERS_QUEUED:
		{
			if (sourcePointer->IsStreaming())
			{
				valueArray[0] = sourcePointer->GetStreamingBuffers().GetBufferCount();
			}
			else
			{
				valueArray[0] = sourcePointer->GetStaticBuffer() ? 1 : 0;
			}
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_BUFFERS_PROCESSED:
		{
			if (sourcePointer->IsStreaming())
			{
				valueArray[0] = sourcePointer->GetStreamingBuffers().GetProcessedBufferCount();
			}
			else
			{
				valueArray[0] = 0;
			}
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_BYTE_OFFSET:
		{
			valueArray[0] = sourcePointer->GetPlaybackPositionIn(AL::AudioPlaybackPosition::Units::kBytes).GetValue();
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_LOOPING:
		{
			valueArray[0] = sourcePointer->IsStaticBufferLoopingEnabled() ? AL_TRUE : AL_FALSE;
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_SAMPLE_OFFSET:
		{
			valueArray[0] = sourcePointer->GetPlaybackPositionIn(AL::AudioPlaybackPosition::Units::kSamples).GetValue();
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_SEC_OFFSET:
		{
			valueArray[0] = sourcePointer->GetPlaybackPositionIn(AL::AudioPlaybackPosition::Units::kSeconds).GetValue();
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_SOURCE_RELATIVE:
		{
//TODO: Add positional audio support.
			valueArray[0] = AL_FALSE;
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_SOURCE_STATE:
		{
			valueArray[0] = sourcePointer->GetPlaybackState().ToOpenALStateId();
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_SOURCE_TYPE:
		{
			if (sourcePointer->IsStreaming())
			{
				valueArray[0] = AL_STREAMING;
			}
			else if (sourcePointer->GetStaticBuffer())
			{
				valueArray[0] = AL_STATIC;
			}
			else
			{
				valueArray[0] = AL_UNDETERMINED;
			}
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		default:
			sLastErrorCode = AL_INVALID_ENUM;
			break;
	}
}

void alSourcePlay(ALuint sourceId)
{
	alSourcePlayv(1, &sourceId);
}

void alSourcePlayv(ALsizei numberOfSources, const ALuint *sourceIdArray)
{
	PlayAudioSourceOperation operation;
	ExecuteOperationOnSources(numberOfSources, sourceIdArray, operation);
}

void alSourceStop(ALuint sourceId)
{
	alSourceStopv(1, &sourceId);
}

void alSourceStopv(ALsizei numberOfSources, const ALuint *sourceIdArray)
{
	StopAudioSourceOperation operation;
	ExecuteOperationOnSources(numberOfSources, sourceIdArray, operation);
}

void alSourceRewind(ALuint sourceId)
{
	alSourceRewindv(1, &sourceId);
}

void alSourceRewindv(ALsizei numberOfSources, const ALuint *sourceIdArray)
{
	RewindAudioSourceOperation operation;
	ExecuteOperationOnSources(numberOfSources, sourceIdArray, operation);
}

void alSourcePause(ALuint sourceId)
{
	alSourcePausev(1, &sourceId);
}

void alSourcePausev(ALsizei numberOfSources, const ALuint *sourceIdArray)
{
	PauseAudioSourceOperation operation;
	ExecuteOperationOnSources(numberOfSources, sourceIdArray, operation);
}

void alSourceQueueBuffers(ALuint sourceId, ALsizei numberOfBuffers, const ALuint *bufferIdArray)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Validate pointer argument.
	if (nullptr == bufferIdArray)
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the current context and its audio buffer manager.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return;
	}
	AL::AudioBufferManager &bufferManager = contextPointer->GetBufferManager();

	// Fetch the audio source.
	auto sourcePointer = contextPointer->GetSourceManager().GetById(sourceId);
	if (nullptr == sourcePointer)
	{
		sLastErrorCode = AL_INVALID_NAME;
		return;
	}

	// Do not continue if the audio source is set up to use a static audio buffer.
	// Note: The caller is expected to set parameter AL_SOURCE_TYPE to AL_STREAMING before calling this function.
	//       However, we'll let it slide if the source does not have a static buffer assigned to it.
	if (sourcePointer->IsNotStreaming() && sourcePointer->GetStaticBuffer())
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return;
	}

	// Do not continue if given zero buffers to queue. (This is not considered an error.)
	if (numberOfBuffers < 1)
	{
		sLastErrorCode = AL_NO_ERROR;
		return;
	}

	// Do not continue if the given array contains at least 1 audio buffer ID that does not exist in the system.
	for (int index = 0; index < numberOfBuffers; index++)
	{
		if (bufferManager.ContainsId(bufferIdArray[index]) == false)
		{
			sLastErrorCode = AL_INVALID_NAME;
			return;
		}
	}

	// Queue the given audio buffers.
	AL::AudioBuffer *bufferPointer;
	sourcePointer->SetIsStreaming(true);
	sourcePointer->SetStaticBuffer(nullptr);
	for (int index = 0; index < numberOfBuffers; index++)
	{
		bufferPointer = bufferManager.GetById(bufferIdArray[index]);
		sourcePointer->GetStreamingBuffers().PushBuffer(bufferPointer);
	}
	sLastErrorCode = AL_NO_ERROR;
}

void alSourceUnqueueBuffers(ALuint sourceId, ALsizei numberOfBuffers, ALuint *bufferIdArray)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Validate pointer argument.
	if (nullptr == bufferIdArray)
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the current context.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return;
	}

	// Fetch the audio source.
	auto sourcePointer = contextPointer->GetSourceManager().GetById(sourceId);
	if (nullptr == sourcePointer)
	{
		sLastErrorCode = AL_INVALID_NAME;
		return;
	}

	// Do not continue if given zero buffers to pop off of the queue. (This is not considered an error.)
	if (numberOfBuffers < 1)
	{
		sLastErrorCode = AL_NO_ERROR;
		return;
	}

	// Pop off all audio buffers from the queue that have already been played/processed by the audio.
	// Also, copy "processed" audio buffers' unique integer IDs to the given array.
	AL::AudioBuffer *bufferPointer;
	for (int index = 0; index < numberOfBuffers; index++)
	{
		bufferPointer = sourcePointer->GetStreamingBuffers().PopProcessedBuffer();
		bufferIdArray[index] = bufferPointer ? bufferPointer->GetIntegerId() : AL::AudioBuffer::kInvalidId;
	}
}

void alGenBuffers(ALsizei numberOfBuffers, ALuint *bufferIdArray)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Validate pointer argument.
	if (nullptr == bufferIdArray)
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the current context and its audio buffer manager.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return;
	}
	AL::AudioBufferManager &bufferManager = contextPointer->GetBufferManager();

	// Do not continue if given zero. (This is not considered an error.)
	if (numberOfBuffers < 1)
	{
		sLastErrorCode = AL_NO_ERROR;
		return;
	}

	// Create requested number of audio buffers and copy their IDs to the given array.
	bool hasFailed = false;
	AL::AudioBuffer *bufferPointer;
	for (int index = 0; index < numberOfBuffers; index++)
	{
		// Attempt to create the next audio buffer, but only if we were successful in creating previous buffers.
		bufferPointer = nullptr;
		if (!hasFailed)
		{
			bufferPointer = bufferManager.Create();
		}
		if (bufferPointer)
		{
			// Copy the new audio buffer's unique integer ID to the array.
			bufferIdArray[index] = bufferPointer->GetIntegerId();
		}
		else
		{
			// Failed to create the audio buffer.
			bufferIdArray[index] = AL::AudioBuffer::kInvalidId;
			hasFailed = true;
		}
	}

	// If we've failed to create at least one audio buffer up above,
	// then delete all audio buffer that were created, if any.
	if (hasFailed)
	{
		for (int index = 0; index < numberOfBuffers; index++)
		{
			bufferManager.DestroyById(bufferIdArray[index]);
			bufferIdArray[index] = AL::AudioBuffer::kInvalidId;
		}
		sLastErrorCode = AL_OUT_OF_MEMORY;
		return;
	}

	// We've successfully created all requested audio buffers.
	sLastErrorCode = AL_NO_ERROR;
}

void alDeleteBuffers(ALsizei numberOfBuffers, const ALuint *bufferIdArray)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Validate pointer argument.
	if (nullptr == bufferIdArray)
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the current context and its audio buffer manager.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return;
	}
	AL::AudioBufferManager &bufferManager = contextPointer->GetBufferManager();

	// Do not continue if given zero buffers to delete. (This is not considered an error.)
	if (numberOfBuffers < 1)
	{
		sLastErrorCode = AL_NO_ERROR;
		return;
	}

	// Do not continue if the given array contains at least 1 audio buffer ID that does not exist in the system.
	for (int index = 0; index < numberOfBuffers; index++)
	{
		if (bufferManager.ContainsId(bufferIdArray[index]) == false)
		{
			sLastErrorCode = AL_INVALID_NAME;
			return;
		}
	}

	// Delete all of the given audio buffers.
	for (int index = 0; index < numberOfBuffers; index++)
	{
		bufferManager.DestroyById(bufferIdArray[index]);
	}
	sLastErrorCode = AL_NO_ERROR;
}

ALboolean alIsBuffer(ALuint bufferId)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Fetch the current context.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return AL_FALSE;
	}

	// Determine if the given audio buffer ID exists in the system.
	sLastErrorCode = AL_NO_ERROR;
	bool wasFound = contextPointer->GetBufferManager().ContainsId(bufferId);
	return wasFound ? AL_TRUE : AL_FALSE;
}

void alBufferData(ALuint bufferId, ALenum format, const ALvoid *dataArray, ALsizei byteCount, ALsizei frequency)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Validate pointer argument.
	if (nullptr == dataArray)
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the current context.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return;
	}

	// Fetch the specified audio buffer by its unique integer ID.
	auto bufferPointer = contextPointer->GetBufferManager().GetById(bufferId);
	if (nullptr == bufferPointer)
	{
		sLastErrorCode = AL_INVALID_NAME;
		return;
	}

	// Convert the given audio format settings.
	AL::AudioFormatSettings settings;
	switch (format)
	{
		case AL_FORMAT_MONO8:
			settings.SetChannelCount(1);
			settings.SetDataType(AL::AudioDataType::kInt8);
			break;
		case AL_FORMAT_MONO16:
			settings.SetChannelCount(1);
			settings.SetDataType(AL::AudioDataType::kInt16);
			break;
		case AL_FORMAT_STEREO8:
			settings.SetChannelCount(2);
			settings.SetDataType(AL::AudioDataType::kInt8);
			break;
		case AL_FORMAT_STEREO16:
			settings.SetChannelCount(2);
			settings.SetDataType(AL::AudioDataType::kInt16);
			break;
		case AL_FORMAT_QUAD8:
			settings.SetChannelCount(4);
			settings.SetDataType(AL::AudioDataType::kInt8);
			break;
		case AL_FORMAT_QUAD16:
			settings.SetChannelCount(4);
			settings.SetDataType(AL::AudioDataType::kInt16);
			break;
		case AL_FORMAT_51CHN8:
			settings.SetChannelCount(6);
			settings.SetDataType(AL::AudioDataType::kInt8);
			break;
		case AL_FORMAT_51CHN16:
			settings.SetChannelCount(6);
			settings.SetDataType(AL::AudioDataType::kInt16);
			break;
		case AL_FORMAT_61CHN8:
			settings.SetChannelCount(7);
			settings.SetDataType(AL::AudioDataType::kInt8);
			break;
		case AL_FORMAT_61CHN16:
			settings.SetChannelCount(7);
			settings.SetDataType(AL::AudioDataType::kInt16);
			break;
		case AL_FORMAT_71CHN8:
			settings.SetChannelCount(8);
			settings.SetDataType(AL::AudioDataType::kInt8);
			break;
		case AL_FORMAT_71CHN16:
			settings.SetChannelCount(8);
			settings.SetDataType(AL::AudioDataType::kInt16);
			break;
		default:
			sLastErrorCode = AL_INVALID_ENUM;
			return;
	}
	settings.SetSampleRate(frequency);

	// Verify that the given array's byte count is in increments of the given audio format's sample/frame size.
	if ((byteCount % settings.GetSampleSizeInBytes()) != 0)
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Copy the audio data to the buffer.
	// Note: Will clear the byte buffer if given a zero byte count.
	bool wasUpdated = bufferPointer->UpdateWith(AL::AudioFormatInfo(settings), (uint8*)dataArray, byteCount);
	if (false == wasUpdated)
	{
		sLastErrorCode = AL_OUT_OF_MEMORY;
		return;
	}

	// Audio data was successfully copied to the audio buffer.
	sLastErrorCode = AL_NO_ERROR;
}

void alBufferf(ALuint bufferId, ALenum parameter, ALfloat value)
{
	HandleInvalidBufferOperationFor(bufferId, parameter);
}

void alBuffer3f(ALuint bufferId, ALenum parameter, ALfloat value1, ALfloat value2, ALfloat value3)
{
	HandleInvalidBufferOperationFor(bufferId, parameter);
}

void alBufferfv(ALuint bufferId, ALenum parameter, const ALfloat *valueArray)
{
	// Validate pointer argument.
	if (nullptr == valueArray)
	{
		AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Handle the setter operation.
	HandleInvalidBufferOperationFor(bufferId, parameter);
}

void alBufferi(ALuint bufferId, ALenum parameter, ALint value)
{
	HandleInvalidBufferOperationFor(bufferId, parameter);
}

void alBuffer3i(ALuint bufferId, ALenum parameter, ALint value1, ALint value2, ALint value3)
{
	HandleInvalidBufferOperationFor(bufferId, parameter);
}

void alBufferiv(ALuint bufferId, ALenum parameter, const ALint *valueArray)
{
	// Validate pointer argument.
	if (nullptr == valueArray)
	{
		AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Handle the setter operation.
	HandleInvalidBufferOperationFor(bufferId, parameter);
}

void alGetBufferf(ALuint bufferId, ALenum parameter, ALfloat *valuePointer)
{
	alGetBufferfv(bufferId, parameter, valuePointer);
}

void alGetBuffer3f(ALuint bufferId, ALenum parameter, ALfloat *valuePointer1, ALfloat *valuePointer2, ALfloat *valuePointer3)
{
	HandleInvalidBufferOperationFor(bufferId, parameter);
}

void alGetBufferfv(ALuint bufferId, ALenum parameter, ALfloat *valueArray)
{
	// Validate argument.
	if (nullptr == valueArray)
	{
		AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Set the audio buffer's parameter.
	switch (parameter)
	{
		case AL_FREQUENCY:
		{
			// Apple's OpenAL implementation deviates from the standard and supports fetching frequency as a float.
			// So, we should too to maximize portability with apps built for iOS.
			ALint integerArray[] = { (ALint)valueArray[0] };
			alGetBufferiv(bufferId, parameter, integerArray);
			valueArray[0] = integerArray[0];
			break;
		}
		default:
		{
			AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
			sLastErrorCode = AL_INVALID_ENUM;
			break;
		}
	}
}

void alGetBufferi(ALuint bufferId, ALenum parameter, ALint *valuePointer)
{
	alGetBufferiv(bufferId, parameter, valuePointer);
}

void alGetBuffer3i(ALuint bufferId, ALenum parameter, ALint *valuePointer1, ALint *valuePointer2, ALint *valuePointer3)
{
	HandleInvalidBufferOperationFor(bufferId, parameter);
}

void alGetBufferiv(ALuint bufferId, ALenum parameter, ALint *valueArray)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Validate argument.
	if (nullptr == valueArray)
	{
		sLastErrorCode = AL_INVALID_VALUE;
		return;
	}

	// Fetch the current context.
	auto contextPointer = AL::AudioEnvironment::GetCurrentContext();
	if (nullptr == contextPointer)
	{
		sLastErrorCode = AL_INVALID_OPERATION;
		return;
	}

	// Fetch the audio buffer.
	auto bufferPointer = contextPointer->GetBufferManager().GetById(bufferId);
	if (nullptr == bufferPointer)
	{
		sLastErrorCode = AL_INVALID_NAME;
		return;
	}

	// Fetch the audio buffer's parameter values.
	switch (parameter)
	{
		case AL_BITS:
		{
			valueArray[0] = bufferPointer->GetFormatInfo().GetBitDepth();
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_CHANNELS:
		{
			valueArray[0] = bufferPointer->GetFormatInfo().GetChannelCount();
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_FREQUENCY:
		{
			valueArray[0] = bufferPointer->GetFormatInfo().GetSampleRate();
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		case AL_SIZE:
		{
			valueArray[0] = bufferPointer->GetByteCount();
			sLastErrorCode = AL_NO_ERROR;
			break;
		}
		default:
			sLastErrorCode = AL_INVALID_ENUM;
			break;
	}
}

#pragma endregion

