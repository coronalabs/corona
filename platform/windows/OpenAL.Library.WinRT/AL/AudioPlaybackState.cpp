// ----------------------------------------------------------------------------
// 
// AudioPlaybackState.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

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
