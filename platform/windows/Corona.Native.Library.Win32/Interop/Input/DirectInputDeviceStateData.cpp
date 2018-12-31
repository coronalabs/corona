//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DirectInputDeviceStateData.h"
#include "InputAxisInfo.h"
#include "InputDeviceInfo.h"
#include "Key.h"
#include <exception>

// Include the "InitGuid.h" header before including the DirectInput header.
// This allows us to use its GUIDs without linking to its lib at compile time.
#include <InitGuid.h>
#include <dinput.h>


namespace Interop { namespace Input {

#pragma region Constants
const size_t DirectInputDeviceStateData::kButtonValueByteCount = sizeof(BYTE);

const size_t DirectInputDeviceStateData::kAxisValueByteCount = sizeof(LONG);

const size_t DirectInputDeviceStateData::kHatValueByteCount = sizeof(DWORD);

static const wchar_t kBufferNotFormattedMessage[] = L"Data buffer not formatted.";

static const wchar_t kInvalidIndexMessage[] = L"Invalid index.";

static const wchar_t kInstanceIdNotFoundMessage[] = L"Instance ID not found.";

#pragma endregion


#pragma region Constructors/Destructors
DirectInputDeviceStateData::DirectInputDeviceStateData()
:	fDataFormatPointer(nullptr),
	fAxisCount(0),
	fButtonCount(0),
	fHatCount(0),
	fBufferPointer(nullptr),
	fBufferSize(0)
{
}

DirectInputDeviceStateData::~DirectInputDeviceStateData()
{
	ClearFormat();
}

#pragma endregion


#pragma region Public Methods
void DirectInputDeviceStateData::SetFormatUsing(const DirectInputDeviceStateData::FormatSettings& settings)
{
	// Delete the previous data format.
	// This also resets all member variables.
	ClearFormat();

	// Determine the number of inputs the device has.
	if (settings.AxisInstanceIdArray)
	{
		fAxisCount = (int)settings.AxisInstanceIdArraySize;
	}
	if (settings.HatInstanceIdArray)
	{
		fHatCount = (int)settings.HatInstanceIdArraySize;
	}
	if (settings.ButtonInstanceIdArray)
	{
		fButtonCount = (int)settings.ButtonInstanceIdArraySize;
	}
	int totalInputCount = fButtonCount + fAxisCount + fHatCount;

	// If the given device has no inputs, then stop here.
	if (totalInputCount <= 0)
	{
		return;
	}

	// Create a data buffer to be used by the DirectInputDevice::GetDeviceState() method to write data to.
	fBufferSize = fAxisCount * kAxisValueByteCount;
	fBufferSize += fHatCount * kHatValueByteCount;
	fBufferSize += fButtonCount * kButtonValueByteCount;
	auto byteCount = fBufferSize % sizeof(DWORD);
	if (byteCount > 0)
	{
		// We must pad the buffer with extra bytes to fit the 4 byte packing alignment required by DirectInput.
		fBufferSize += sizeof(DWORD) - byteCount;
	}
	fBufferPointer = new BYTE[fBufferSize];

	// Create and set up a data format object to be passed into the DirectInputDevice::SetFormat() method.
	// This tells DirectInput which device input channels we want to read data from.
	// Note: DirectInput will only accept a format with axes and hats that are DWORD aligned in the buffer.
	//       This means that axes and hats must be positioned first in the buffer since they each take a DWORD in memory.
	//       Buttons only take 1 byte and must be positioned last in the buffer. Buttons do not have to be DWORD aligned.
	fDataFormatPointer = new DIDATAFORMAT;
	memset(fDataFormatPointer, 0, sizeof(DIDATAFORMAT));
	fDataFormatPointer->dwSize = sizeof(DIDATAFORMAT);
	fDataFormatPointer->dwObjSize = sizeof(DIOBJECTDATAFORMAT);
	fDataFormatPointer->dwFlags = DIDF_ABSAXIS;
	fDataFormatPointer->dwDataSize = fBufferSize;
	fDataFormatPointer->dwNumObjs = totalInputCount;
	fDataFormatPointer->rgodf = new DIOBJECTDATAFORMAT[totalInputCount];
	for (int index = 0, byteOffset = 0; index < totalInputCount; index++)
	{
		auto objectFormatPointer = &(fDataFormatPointer->rgodf[index]);
		objectFormatPointer->pguid = 0;
		objectFormatPointer->dwOfs = byteOffset;
		objectFormatPointer->dwFlags = 0;
		if (index < fAxisCount)
		{
			auto instanceId = settings.AxisInstanceIdArray[index];
			objectFormatPointer->dwType = DIDFT_AXIS | DIDFT_MAKEINSTANCE(instanceId);
			byteOffset += kAxisValueByteCount;
		}
		else if (index < (fAxisCount + fHatCount))
		{
			auto instanceId = settings.HatInstanceIdArray[index - fAxisCount];
			objectFormatPointer->dwType = DIDFT_POV | DIDFT_MAKEINSTANCE(instanceId);
			byteOffset += kHatValueByteCount;
		}
		else
		{
			auto instanceId = settings.ButtonInstanceIdArray[index - (fAxisCount + fHatCount)];
			objectFormatPointer->dwType = DIDFT_BUTTON | DIDFT_MAKEINSTANCE(instanceId);
			byteOffset += kButtonValueByteCount;
		}
	}
}

LPDIDATAFORMAT DirectInputDeviceStateData::GetFormat() const
{
	return fDataFormatPointer;
}

void DirectInputDeviceStateData::ClearFormat()
{
	if (fDataFormatPointer)
	{
		if (fDataFormatPointer->rgodf)
		{
			delete[] fDataFormatPointer->rgodf;
		}
		delete fDataFormatPointer;
		fDataFormatPointer = nullptr;
	}
	if (fBufferPointer)
	{
		delete[] fBufferPointer;
		fBufferPointer = nullptr;
		fBufferSize = 0;
	}
	fAxisCount = 0;
	fButtonCount = 0;
	fHatCount = 0;
}

BYTE* DirectInputDeviceStateData::GetBufferPointer() const
{
	return fBufferPointer;
}

size_t DirectInputDeviceStateData::GetBufferSize() const
{
	return fBufferSize;
}

int DirectInputDeviceStateData::GetAxisCount() const
{
	return fAxisCount;
}

ValueResult<LONG> DirectInputDeviceStateData::GetAxisValueByIndex(int index) const
{
	// Validate.
	if (!fBufferPointer)
	{
		return ValueResult<LONG>::FailedWith(kBufferNotFormattedMessage);
	}
	if ((index < 0) || (index >= fAxisCount))
	{
		return ValueResult<LONG>::FailedWith(kInvalidIndexMessage);
	}

	// Fetch the axis' position within the byte buffer.
	auto result = GetAxisBufferOffsetByIndex(index);
	if (result.HasFailed())
	{
		return ValueResult<LONG>::FailedWith(result.GetMessage());
	}
	int byteOffset = result.GetValue();

	// Create a union used to convert the axis' data within the byte buffer to a LONG.
	union
	{
		LONG LongValue;
		BYTE ByteArray[kAxisValueByteCount];
	} binaryConverter;
	
	// Fetch the axis' data from the byte buffer.
	memcpy(binaryConverter.ByteArray, fBufferPointer + byteOffset, kAxisValueByteCount);
	return ValueResult<LONG>::SucceededWith(binaryConverter.LongValue);
}

ValueResult<WORD> DirectInputDeviceStateData::GetAxisInstanceIdByIndex(int index) const
{
	// Validate.
	if (!fDataFormatPointer)
	{
		return ValueResult<WORD>::FailedWith(kBufferNotFormattedMessage);
	}
	if ((index < 0) || (index >= fAxisCount))
	{
		return ValueResult<WORD>::FailedWith(kInvalidIndexMessage);
	}

	// Fetch and return the indexed axis' instance ID.
	int inputIndex = index;
	auto objectType = fDataFormatPointer->rgodf[inputIndex].dwType;
	return ValueResult<WORD>::SucceededWith(DIDFT_GETINSTANCE(objectType));
}

ValueResult<int> DirectInputDeviceStateData::GetAxisIndexByInstanceId(WORD instanceId) const
{
	// Validate.
	if (!fDataFormatPointer)
	{
		return ValueResult<int>::FailedWith(kBufferNotFormattedMessage);
	}

	// Search for the given instance ID.
	for (int index = 0; index < fAxisCount; index++)
	{
		auto formatObjectPointer = &(fDataFormatPointer->rgodf[index]);
		if (DIDFT_GETINSTANCE(formatObjectPointer->dwType) == instanceId)
		{
			// Found it. Return a zero based index to the input assigned the given ID.
			return ValueResult<int>::SucceededWith(index);
		}
	}

	// Instance ID not found. Returns a failure result.
	return ValueResult<int>::FailedWith(kInstanceIdNotFoundMessage);
}

ValueResult<int> DirectInputDeviceStateData::GetAxisBufferOffsetByIndex(int index) const
{
	// Validate.
	if (!fDataFormatPointer)
	{
		return ValueResult<int>::FailedWith(kBufferNotFormattedMessage);
	}
	if ((index < 0) || (index >= fAxisCount))
	{
		return ValueResult<int>::FailedWith(kInvalidIndexMessage);
	}

	// Return a byte offset within "fBufferPointer" where the indexed axis' data value is read/written to.
	int inputIndex = index;
	return ValueResult<int>::SucceededWith(fDataFormatPointer->rgodf[inputIndex].dwOfs);
}

ValueResult<int> DirectInputDeviceStateData::GetAxisBufferOffsetByInstanceId(WORD instanceId) const
{
	// Validate.
	if (!fDataFormatPointer)
	{
		return ValueResult<int>::FailedWith(kBufferNotFormattedMessage);
	}

	// Search for the given instance ID.
	for (int index = 0; index < fAxisCount; index++)
	{
		auto formatObjectPointer = &(fDataFormatPointer->rgodf[index]);
		if (DIDFT_GETINSTANCE(formatObjectPointer->dwType) == instanceId)
		{
			// Found it. Return the buffer offset within "fBufferPointer" that the referenced input writes data to.
			return ValueResult<int>::SucceededWith(formatObjectPointer->dwOfs);
		}
	}

	// Instance ID not found. Returns a failure result.
	return ValueResult<int>::FailedWith(kInstanceIdNotFoundMessage);
}

int DirectInputDeviceStateData::GetButtonCount() const
{
	return fButtonCount;
}

ValueResult<BYTE> DirectInputDeviceStateData::GetButtonValueByIndex(int index) const
{
	// Validate.
	if (!fBufferPointer)
	{
		return ValueResult<BYTE>::FailedWith(kBufferNotFormattedMessage);
	}
	if ((index < 0) || (index >= fButtonCount))
	{
		return ValueResult<BYTE>::FailedWith(kInvalidIndexMessage);
	}

	// Fetch the button's position within the byte buffer.
	auto result = GetButtonBufferOffsetByIndex(index);
	if (result.HasFailed())
	{
		return ValueResult<BYTE>::FailedWith(result.GetMessage());
	}
	int byteOffset = result.GetValue();

	// Return the indexed button's value within the byte buffer.
	return ValueResult<BYTE>::SucceededWith(fBufferPointer[byteOffset]);
}

ValueResult<WORD> DirectInputDeviceStateData::GetButtonInstanceIdByIndex(int index) const
{
	// Validate.
	if (!fDataFormatPointer)
	{
		return ValueResult<WORD>::FailedWith(kBufferNotFormattedMessage);
	}
	if ((index < 0) || (index >= fButtonCount))
	{
		return ValueResult<WORD>::FailedWith(kInvalidIndexMessage);
	}

	// Fetch and return the indexed button's instance ID.
	int inputIndex = fAxisCount + fHatCount + index;
	auto objectType = fDataFormatPointer->rgodf[inputIndex].dwType;
	return ValueResult<WORD>::SucceededWith(DIDFT_GETINSTANCE(objectType));
}

ValueResult<int> DirectInputDeviceStateData::GetButtonBufferOffsetByIndex(int index) const
{
	// Validate.
	if (!fDataFormatPointer)
	{
		return ValueResult<int>::FailedWith(kBufferNotFormattedMessage);
	}
	if ((index < 0) || (index >= fButtonCount))
	{
		return ValueResult<int>::FailedWith(kInvalidIndexMessage);
	}

	// Return a byte offset within "fBufferPointer" where the indexed button's data value is read/written to.
	int inputIndex = fAxisCount + fHatCount + index;
	return ValueResult<int>::SucceededWith(fDataFormatPointer->rgodf[inputIndex].dwOfs);
}

int DirectInputDeviceStateData::GetHatCount() const
{
	return fHatCount;
}

ValueResult<DWORD> DirectInputDeviceStateData::GetHatValueByIndex(int index) const
{
	// Validate.
	if (!fBufferPointer)
	{
		return ValueResult<DWORD>::FailedWith(kBufferNotFormattedMessage);
	}
	if ((index < 0) || (index >= fHatCount))
	{
		return ValueResult<DWORD>::FailedWith(kInvalidIndexMessage);
	}

	// Fetch the hat's position within the byte buffer.
	auto result = GetHatBufferOffsetByIndex(index);
	if (result.HasFailed())
	{
		return ValueResult<DWORD>::FailedWith(result.GetMessage());
	}
	int byteOffset = result.GetValue();

	// Create a union used to convert the hat's data within the byte buffer to a DWORD.
	union
	{
		DWORD DoubleWordValue;
		BYTE ByteArray[kHatValueByteCount];
	} binaryConverter;

	// Fetch the hat's data from the byte buffer.
	memcpy(binaryConverter.ByteArray, fBufferPointer + byteOffset, kHatValueByteCount);
	return ValueResult<DWORD>::SucceededWith(binaryConverter.DoubleWordValue);
}

ValueResult<WORD> DirectInputDeviceStateData::GetHatInstanceIdByIndex(int index) const
{
	// Validate.
	if (!fDataFormatPointer)
	{
		return ValueResult<WORD>::FailedWith(kBufferNotFormattedMessage);
	}
	if ((index < 0) || (index >= fHatCount))
	{
		return ValueResult<WORD>::FailedWith(kInvalidIndexMessage);
	}

	// Fetch and return the indexed hat's instance ID.
	int inputIndex = fAxisCount + index;
	auto objectType = fDataFormatPointer->rgodf[inputIndex].dwType;
	return ValueResult<WORD>::SucceededWith(DIDFT_GETINSTANCE(objectType));
}

ValueResult<int> DirectInputDeviceStateData::GetHatBufferOffsetByIndex(int index) const
{
	// Validate.
	if (!fDataFormatPointer)
	{
		return ValueResult<int>::FailedWith(kBufferNotFormattedMessage);
	}
	if ((index < 0) || (index >= fHatCount))
	{
		return ValueResult<int>::FailedWith(kInvalidIndexMessage);
	}

	// Return a byte offset within "fBufferPointer" where the indexed hat's data value is read/written to.
	int inputIndex = fAxisCount + index;
	return ValueResult<int>::SucceededWith(fDataFormatPointer->rgodf[inputIndex].dwOfs);
}

DirectInputDeviceStateData::BufferOffsetInfoResult DirectInputDeviceStateData::GetInfoFromBufferOffset(int offset) const
{
#if 0
	if (fDataFormatPointer)
	{
		for (int formatIndex = (int)fDataFormatPointer->dwNumObjs - 1; formatIndex >= 0; formatIndex--)
		{
			auto formatObjectPointer = &(fDataFormatPointer->rgodf[formatIndex]);
			if (offset == formatObjectPointer->dwOfs)
			{
				auto inputType = BufferOffsetInfoResult::InputType::kUnknown;
				int inputIndex = -1;
				if (formatObjectPointer->dwType & DIDFT_BUTTON)
				{
					inputType = BufferOffsetInfoResult::InputType::kButton;
					inputIndex = formatIndex;
				}
				else if (formatObjectPointer->dwType & DIDFT_AXIS)
				{
					inputType = BufferOffsetInfoResult::InputType::kAxis;
					inputIndex = formatIndex - fButtonCount;
				}
				else if (formatObjectPointer->dwType & DIDFT_POV)
				{
					inputType = BufferOffsetInfoResult::InputType::kHat;
					inputIndex = formatIndex - (fButtonCount + fAxisCount);
				}
				return BufferOffsetInfoResult(inputType, inputIndex);
			}
		}
	}
	return BufferOffsetInfoResult();
#else
	if (!fBufferPointer)
	{
		return BufferOffsetInfoResult::FailedWith(L"Buffer has not been formatted.");
	}
	if ((offset < 0) || (offset >= (int)fBufferSize))
	{
		return BufferOffsetInfoResult::FailedWith(L"Offset is out of range.");
	}

	if (offset < (fAxisCount * (int)kAxisValueByteCount))
	{
		if (offset % kAxisValueByteCount)
		{
			return BufferOffsetInfoResult::FailedWith(L"Offset is misaligned with axis data in buffer.");
		}
		auto inputType = BufferOffsetInfoResult::InputType::kAxis;
		int inputIndex = offset / kAxisValueByteCount;
		return BufferOffsetInfoResult(inputType, inputIndex);
	}

	offset -= fAxisCount * (int)kAxisValueByteCount;
	if (offset < (fHatCount * (int)kHatValueByteCount))
	{
		if (offset % kHatValueByteCount)
		{
			return BufferOffsetInfoResult::FailedWith(L"Offset is misaligned with hat switch data in buffer.");
		}
		auto inputType = BufferOffsetInfoResult::InputType::kHat;
		int inputIndex = offset / kHatValueByteCount;
		return BufferOffsetInfoResult(inputType, inputIndex);
	}

	offset -= fHatCount * (int)kHatValueByteCount;
	if (offset < fButtonCount)
	{
		auto inputType = BufferOffsetInfoResult::InputType::kButton;
		int inputIndex = offset;
		return BufferOffsetInfoResult(inputType, inputIndex);
	}

	return BufferOffsetInfoResult::FailedWith(L"Internal error handling given offset.");
#endif
}

#pragma endregion


#pragma region BufferOffsetInfoResult Class Methods
DirectInputDeviceStateData::BufferOffsetInfoResult::BufferOffsetInfoResult(const wchar_t* errorMessage)
:	OperationResult(false, errorMessage),
	fInputType(InputType::kUnknown),
	fInputIndex(-1)
{
}

DirectInputDeviceStateData::BufferOffsetInfoResult::BufferOffsetInfoResult(
	BufferOffsetInfoResult::InputType inputType, int inputIndex)
:	OperationResult((inputType != InputType::kUnknown) && (inputIndex >= 0), (const wchar_t*)nullptr),
	fInputType(inputType),
	fInputIndex(inputIndex)
{
}

DirectInputDeviceStateData::BufferOffsetInfoResult::~BufferOffsetInfoResult()
{
}

bool DirectInputDeviceStateData::BufferOffsetInfoResult::IsAxis() const
{
	return (InputType::kAxis == fInputType);
}

bool DirectInputDeviceStateData::BufferOffsetInfoResult::IsButton() const
{
	return (InputType::kButton == fInputType);
}

bool DirectInputDeviceStateData::BufferOffsetInfoResult::IsHat() const
{
	return (InputType::kHat == fInputType);
}

int DirectInputDeviceStateData::BufferOffsetInfoResult::GetIndex() const
{
	return fInputIndex;
}

DirectInputDeviceStateData::BufferOffsetInfoResult DirectInputDeviceStateData::BufferOffsetInfoResult::FailedWith(
	const wchar_t* errorMessage)
{
	return BufferOffsetInfoResult(errorMessage);
}

#pragma endregion

} }	// namespace Interop::Input
