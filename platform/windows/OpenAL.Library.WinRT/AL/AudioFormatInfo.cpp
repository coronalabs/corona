// ----------------------------------------------------------------------------
// 
// AudioFormatInfo.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include <pch.h>
#include "AudioFormatInfo.h"


namespace AL {

#pragma region Constructors/Destructors
AudioFormatInfo::AudioFormatInfo()
{
}

AudioFormatInfo::AudioFormatInfo(const AudioFormatSettings &settings)
:	fSettings(settings)
{
}

AudioFormatInfo::~AudioFormatInfo()
{
}

#pragma endregion


#pragma region Public Methods
int AudioFormatInfo::GetChannelCount() const
{
	return fSettings.GetChannelCount();
}

int AudioFormatInfo::GetSampleRate() const
{
	return fSettings.GetSampleRate();
}

AudioDataType AudioFormatInfo::GetDataType() const
{
	return fSettings.GetDataType();
}

int AudioFormatInfo::GetBitDepth() const
{
	return fSettings.GetDataType().GetBitDepth();
}

int AudioFormatInfo::GetSampleSizeInBytes() const
{
	return fSettings.GetSampleSizeInBytes();
}

WAVEFORMATEX AudioFormatInfo::ToWAVEFORMATEX() const
{
	return fSettings.ToWAVEFORMATEX();
}

WAVEFORMATEXTENSIBLE AudioFormatInfo::ToWAVEFORMATEXTENSIBLE() const
{
	return fSettings.ToWAVEFORMATEXTENSIBLE();
}

bool AudioFormatInfo::Equals(const AudioFormatInfo &value) const
{
	return fSettings.Equals(value.fSettings);
}

bool AudioFormatInfo::NotEquals(const AudioFormatInfo &value) const
{
	return fSettings.NotEquals(value.fSettings);
}

bool AudioFormatInfo::operator==(const AudioFormatInfo &value) const
{
	return fSettings.Equals(value.fSettings);
}

bool AudioFormatInfo::operator!=(const AudioFormatInfo &value) const
{
	return fSettings.NotEquals(value.fSettings);
}

#pragma endregion

}	// namespace AL
