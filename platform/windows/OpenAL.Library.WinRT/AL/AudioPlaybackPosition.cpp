// ----------------------------------------------------------------------------
// 
// AudioPlaybackPosition.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include <pch.h>
#include "AudioPlaybackPosition.h"
#include "AudioFormatInfo.h"


namespace AL {

#pragma region Constructors/Destructors
AudioPlaybackPosition::AudioPlaybackPosition()
:	fUnits(AudioPlaybackPosition::Units::kBytes),
	fValue(0)
{
}

AudioPlaybackPosition::AudioPlaybackPosition(AudioPlaybackPosition::Units units, uint32 value)
:	fUnits(units),
	fValue(value)
{
}

AudioPlaybackPosition::~AudioPlaybackPosition()
{
}

#pragma endregion


#pragma region Public Methods
AudioPlaybackPosition::Units AudioPlaybackPosition::GetUnits() const
{
	return fUnits;
}

uint32 AudioPlaybackPosition::GetValue() const
{
	return fValue;
}

AudioPlaybackPosition AudioPlaybackPosition::ToBytesUsing(const AudioFormatInfo &format)
{
	// Convert this object's position to bytes.
	uint32 positionInBytes;
	switch (fUnits)
	{
		case AudioPlaybackPosition::Units::kBytes:
			positionInBytes = fValue;
			break;
		case AudioPlaybackPosition::Units::kSeconds:
			positionInBytes = (fValue * (uint32)format.GetSampleRate()) * (uint32)format.GetSampleSizeInBytes();
			break;
		case AudioPlaybackPosition::Units::kSamples:
			positionInBytes = fValue * (uint32)format.GetSampleSizeInBytes();
			break;
		default:
			if (IsDebuggerPresent())
			{
				__debugbreak();
			}
			positionInBytes = fValue;
			break;
	}

	// Return a new playback position object measured in bytes.
	return AudioPlaybackPosition(AudioPlaybackPosition::Units::kBytes, positionInBytes);
}

AudioPlaybackPosition AudioPlaybackPosition::ToSecondsUsing(const AudioFormatInfo &format)
{
	// Convert this object's position to seconds.
	double positionInFractionalSeconds;
	switch (fUnits)
	{
		case AudioPlaybackPosition::Units::kBytes:
			positionInFractionalSeconds = (double)(fValue / format.GetSampleSizeInBytes()) / (double)format.GetSampleRate();
			break;
		case AudioPlaybackPosition::Units::kSeconds:
			positionInFractionalSeconds = (double)fValue;
			break;
		case AudioPlaybackPosition::Units::kSamples:
			positionInFractionalSeconds = (double)fValue / (double)format.GetSampleRate();
			break;
		default:
			if (IsDebuggerPresent())
			{
				__debugbreak();
			}
			positionInFractionalSeconds = (double)fValue;
			break;
	}

	// Found fractional seconds to the nearest whole number seconds.
	uint32 positionInSeconds = (uint32)(positionInFractionalSeconds + 0.5);

	// Return a new playback position object measured in seconds.
	return AudioPlaybackPosition(AudioPlaybackPosition::Units::kSeconds, positionInSeconds);
}

AudioPlaybackPosition AudioPlaybackPosition::ToSamplesUsing(const AudioFormatInfo &format)
{
	// Convert this object's position to samples/frames.
	uint32 positionInSamples;
	switch (fUnits)
	{
		case AudioPlaybackPosition::Units::kBytes:
			positionInSamples = fValue / (uint32)format.GetSampleSizeInBytes();
			break;
		case AudioPlaybackPosition::Units::kSeconds:
			positionInSamples = fValue * (uint32)format.GetSampleRate();
			break;
		case AudioPlaybackPosition::Units::kSamples:
			positionInSamples = fValue;
			break;
		default:
			if (IsDebuggerPresent())
			{
				__debugbreak();
			}
			positionInSamples = fValue;
			break;
	}

	// Return a new playback position object measured in samples/frames.
	return AudioPlaybackPosition(AudioPlaybackPosition::Units::kSamples, positionInSamples);
}

AudioPlaybackPosition AudioPlaybackPosition::ToUnitUsing(AudioPlaybackPosition::Units unit, const AudioFormatInfo &format)
{
	// Convert this object's position to the given measurement unit.
	AudioPlaybackPosition convertedPosition;
	switch (unit)
	{
		case AudioPlaybackPosition::Units::kBytes:
			convertedPosition = ToBytesUsing(format);
			break;
		case AudioPlaybackPosition::Units::kSeconds:
			convertedPosition = ToSecondsUsing(format);
			break;
		case AudioPlaybackPosition::Units::kSamples:
			convertedPosition = ToSamplesUsing(format);
			break;
		default:
			if (IsDebuggerPresent())
			{
				__debugbreak();
			}
			convertedPosition = *this;
			break;
	}

	// Return the converted playback position.
	return convertedPosition;
}

#pragma endregion


#pragma region Public Static Functions
AudioPlaybackPosition AudioPlaybackPosition::FromBytes(uint32 value)
{
	return AudioPlaybackPosition(AudioPlaybackPosition::Units::kBytes, value);
}

AudioPlaybackPosition AudioPlaybackPosition::FromSeconds(uint32 value)
{
	return AudioPlaybackPosition(AudioPlaybackPosition::Units::kSeconds, value);
}

AudioPlaybackPosition AudioPlaybackPosition::FromSamples(uint32 value)
{
	return AudioPlaybackPosition(AudioPlaybackPosition::Units::kSamples, value);
}

#pragma endregion

}	// namespace AL
