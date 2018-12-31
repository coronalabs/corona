// ----------------------------------------------------------------------------
// 
// AudioFormatSettings.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include <pch.h>
#include "AudioFormatSettings.h"


namespace AL {

#pragma region Constructors/Destructors
AudioFormatSettings::AudioFormatSettings()
:	fChannelCount(2),
	fSampleRate(44100),
	fDataType(AudioDataType::kInt16)
{
}

AudioFormatSettings::~AudioFormatSettings()
{
}

#pragma endregion


#pragma region Public Methods
int AudioFormatSettings::GetChannelCount() const
{
	return fChannelCount;
}

void AudioFormatSettings::SetChannelCount(int value)
{
	if (value < kMinChannels)
	{
		value = kMinChannels;
	}
	else if (value > kMaxChannels)
	{
		value = kMaxChannels;
	}
	fChannelCount = value;
}

int AudioFormatSettings::GetSampleRate() const
{
	return fSampleRate;
}

void AudioFormatSettings::SetSampleRate(int value)
{
	if (value < 1)
	{
		value = 1;
	}
	fSampleRate = value;
}

AudioDataType AudioFormatSettings::GetDataType() const
{
	return fDataType;
}

void AudioFormatSettings::SetDataType(AudioDataType value)
{
	fDataType = value;
}

int AudioFormatSettings::GetBitDepth() const
{
	return fDataType.GetBitDepth();
}

int AudioFormatSettings::GetSampleSizeInBytes() const
{
	return (fDataType.GetBitDepth() / 8) * fChannelCount;
}

WAVEFORMATEX AudioFormatSettings::ToWAVEFORMATEX() const
{
	WAVEFORMATEX waveFormat;
	memset(&waveFormat, 0, sizeof(waveFormat));
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = fChannelCount;
	waveFormat.nSamplesPerSec = fSampleRate;
	waveFormat.wBitsPerSample = fDataType.GetBitDepth();
	waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	return waveFormat;
}

WAVEFORMATEXTENSIBLE AudioFormatSettings::ToWAVEFORMATEXTENSIBLE() const
{
	WAVEFORMATEXTENSIBLE waveFormat;
	memset(&waveFormat, 0, sizeof(waveFormat));
	waveFormat.Format = ToWAVEFORMATEX();
	waveFormat.Format.cbSize = sizeof(waveFormat) - sizeof(waveFormat.Format);
	waveFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	waveFormat.Samples.wValidBitsPerSample = waveFormat.Format.wBitsPerSample;
	waveFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
	switch (fChannelCount)
	{
		case 1:
			waveFormat.dwChannelMask = SPEAKER_FRONT_CENTER;
			break;
		case 2:
			waveFormat.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
			break;
		case 4:
			waveFormat.dwChannelMask =
					SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
			break;
		case 6:
			waveFormat.dwChannelMask =
					SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY |
					SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT |
					SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
			break;
		case 7:
			waveFormat.dwChannelMask =
					SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY |
					SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT |
					SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT |
					SPEAKER_BACK_CENTER;
			break;
		case 8:
			waveFormat.dwChannelMask =
					SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY |
					SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT |
					SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT |
					SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
			break;
	}
	return waveFormat;
}

bool AudioFormatSettings::Equals(const AudioFormatSettings &value) const
{
	if ((fChannelCount == value.fChannelCount) &&
	    (fSampleRate == value.fSampleRate) &&
	    (fDataType == value.fDataType))
	{
		return true;
	}
	return false;
}

bool AudioFormatSettings::NotEquals(const AudioFormatSettings &value) const
{
	return !Equals(value);
}

bool AudioFormatSettings::operator==(const AudioFormatSettings &value) const
{
	return Equals(value);
}

bool AudioFormatSettings::operator!=(const AudioFormatSettings &value) const
{
	return !Equals(value);
}

#pragma endregion

}	// namespace AL
