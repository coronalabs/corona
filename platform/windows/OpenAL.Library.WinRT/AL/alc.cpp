//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <pch.h>
#include "alc.h"
#include "AudioContext.h"
#include "AudioEnvironment.h"
#include <mutex>


#pragma region Public Functions
ALCcontext* alcCreateContext(ALCdevice *devicePointer, const ALCint *attributeArray)
{
	// Create the context settings initialized to its defaults.
	AL::AudioContext::CreationSettings settings;

	// Override the above default settings with the given attributes, if any were provided.
	// This attribute array must contain elements in increments of 2 as follows:
	// {
	//    attributeName1, attributeValue1,
	//    attributeName2, attributeValue2,
	//    0                                   // <- The zero terminates the array just like how it is done with a string.
	// }
	if (attributeArray)
	{
		for (int index = 0; attributeArray[index] != 0; index += 2)
		{
			switch (attributeArray[index])
			{
				case ALC_FREQUENCY:
					settings.OutputSampleRate = attributeArray[index + 1];
					break;
			}
		}
	}

	// Attempt to creat the audio context.
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
	auto result = AL::AudioContext::Create(settings);
	if (result.HasFailed())
	{
		//TODO: Log the AudioContext creation error here.
	}

	// Return a pointer to the context or null if creation failed.
	return (ALCcontext*)result.GetContext();
}

ALCboolean alcMakeContextCurrent(ALCcontext *contextPointer)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;

	bool wasSet = AL::AudioEnvironment::SetCurrentContext((AL::AudioContext*)contextPointer);
	return wasSet ? ALC_TRUE : ALC_FALSE;
}

void alcProcessContext(ALCcontext *contextPointer)
{
	if (contextPointer)
	{
		AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
		((AL::AudioContext*)contextPointer)->Resume();
	}
}

void alcSuspendContext(ALCcontext *contextPointer)
{
	if (contextPointer)
	{
		AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
		((AL::AudioContext*)contextPointer)->Suspend();
	}
}

void alcDestroyContext(ALCcontext *contextPointer)
{
	if (contextPointer)
	{
		AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
		AL::AudioContext::Destroy((AL::AudioContext*)contextPointer);
	}
}

ALCcontext* alcGetCurrentContext(void)
{
	AL::AudioEnvironment::ScopedMutexLock scopedMutexLock;
	auto contextPointer = (ALCcontext*)AL::AudioEnvironment::GetCurrentContext();
	return contextPointer;
}

ALCdevice* alcGetContextsDevice(ALCcontext *contextPointer)
{
	return nullptr;
}

ALCenum alcGetError(ALCdevice *devicePointer)
{
	return ALC_NO_ERROR;
}

ALCdevice* alcOpenDevice(const ALCchar *deviceName)
{
	return (ALCdevice*)1;
}

ALCboolean alcCloseDevice(ALCdevice *devicePointer)
{
	return ALC_FALSE;
}

ALCboolean alcIsExtensionPresent(ALCdevice *devicePointer, const ALCchar *extensionName)
{
	return ALC_FALSE;
}

void* alcGetProcAddress(ALCdevice *devicePointer, const ALCchar *functionName)
{
	return nullptr;
}

ALCenum alcGetEnumValue(ALCdevice *devicePointer, const ALCchar *enumName)
{
	return ALC_NO_ERROR;
}

const ALCchar* alcGetString(ALCdevice *devicePointer, ALCenum parameter)
{
	return "";
}

void alcGetIntegerv(ALCdevice *devicePointer, ALCenum parameter, ALCsizei numberOfValues, ALCint *valueArray)
{
}

#pragma endregion

