//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <pch.h>
#include "AudioDataType.h"


namespace AL {

#pragma region Constants
const AudioDataType AudioDataType::kInt8(1, 8);

const AudioDataType AudioDataType::kInt16(2, 16);

#pragma endregion


#pragma region Constructors/Destructors
AudioDataType::AudioDataType(int typeId, int bitDepth)
:	fTypeId(typeId),
	fBitDepth(bitDepth)
{
}

AudioDataType::~AudioDataType()
{
}

#pragma endregion


#pragma region Public Methods
int AudioDataType::GetBitDepth() const
{
	return fBitDepth;
}

bool AudioDataType::Equals(const AudioDataType &value) const
{
	return (fTypeId == value.fTypeId);
}

bool AudioDataType::NotEquals(const AudioDataType &value) const
{
	return !Equals(value);
}

bool AudioDataType::operator==(const AudioDataType &value) const
{
	return Equals(value);
}

bool AudioDataType::operator!=(const AudioDataType &value) const
{
	return !Equals(value);
}

#pragma endregion

}	// namespace AL
