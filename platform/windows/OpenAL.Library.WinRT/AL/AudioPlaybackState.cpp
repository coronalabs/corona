//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <pch.h>
#include "AudioPlaybackState.h"


namespace AL {

#pragma region Constants
const AudioPlaybackState AudioPlaybackState::kReseted(AL_INITIAL);

const AudioPlaybackState AudioPlaybackState::kPlaying(AL_PLAYING);

const AudioPlaybackState AudioPlaybackState::kPaused(AL_PAUSED);

const AudioPlaybackState AudioPlaybackState::kStopped(AL_STOPPED);

#pragma endregion


#pragma region Constructors/Destructors
AudioPlaybackState::AudioPlaybackState()
:	fOpenALStateId(AL_INITIAL)
{
}

AudioPlaybackState::AudioPlaybackState(ALint alStateId)
:	fOpenALStateId(alStateId)
{
}

AudioPlaybackState::~AudioPlaybackState()
{
}

#pragma endregion


#pragma region Public Methods
ALint AudioPlaybackState::ToOpenALStateId() const
{
	return fOpenALStateId;
}

bool AudioPlaybackState::Equals(const AudioPlaybackState &value) const
{
	return (fOpenALStateId == value.fOpenALStateId);
}

bool AudioPlaybackState::NotEquals(const AudioPlaybackState &value) const
{
	return !Equals(value);
}

bool AudioPlaybackState::operator==(const AudioPlaybackState &value) const
{
	return Equals(value);
}

bool AudioPlaybackState::operator!=(const AudioPlaybackState &value) const
{
	return !Equals(value);
}

#pragma endregion

}	// namespace AL
