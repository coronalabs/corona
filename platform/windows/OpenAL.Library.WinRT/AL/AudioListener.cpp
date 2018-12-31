// ----------------------------------------------------------------------------
// 
// AudioListener.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include <pch.h>
#include "AudioListener.h"
#include "AudioContext.h"


namespace AL {

#pragma region Constructors/Destructors
AudioListener::AudioListener(const AudioListener::CreationSettings &settings)
:	fContextPointer(settings.ContextPointer),
	fXAudioMasteringVoicePointer(settings.XAudioMasteringVoicePointer)
{
}

AudioListener::~AudioListener()
{
}

#pragma endregion


#pragma region Public Methods
AudioContext* AudioListener::GetContext()
{
	return fContextPointer;
}

float AudioListener::GetVolumeScale()
{
	float value = 0.0f;
	fXAudioMasteringVoicePointer->GetVolume(&value);
	return value;
}

void AudioListener::SetVolumeScale(float value)
{
	// Do not allow the given value to exceed XAudio2's min/max volume levels.
	if (value > XAUDIO2_MAX_VOLUME_LEVEL)
	{
		value = XAUDIO2_MAX_VOLUME_LEVEL;
	}
	else if (value < -XAUDIO2_MAX_VOLUME_LEVEL)
	{
		value = -XAUDIO2_MAX_VOLUME_LEVEL;
	}

	// Apply the given volume setting.
	fXAudioMasteringVoicePointer->SetVolume(value);
}

Position AudioListener::GetPosition()
{
	return fPosition;
}

void AudioListener::SetPosition(const Position &value)
{
	fPosition = value;
}

Vector AudioListener::GetVelocity()
{
	return fVelocity;
}

void AudioListener::SetVelocity(const Vector &value)
{
	fVelocity = value;
}

#pragma endregion

}	// namespace AL
