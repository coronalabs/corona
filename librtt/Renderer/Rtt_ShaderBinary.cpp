//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_ShaderBinary.h"
#include "Core/Rtt_Config.h"
#include "Core/Rtt_Allocator.h"
#include <stdio.h>
#include <cstdlib>
#include <cstring>


namespace Rtt
{

// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

ShaderBinary::ShaderBinary(Rtt_Allocator *allocatorPointer)
:	fAllocatorPointer(allocatorPointer),
	fByteBufferPointer(NULL),
	fByteCount(0)
{
}

ShaderBinary::~ShaderBinary()
{
	Free();
}


// ----------------------------------------------------------------------------
// Public Functions
// ----------------------------------------------------------------------------

Rtt_Allocator* ShaderBinary::GetAllocator()
{
	return fAllocatorPointer;
}

const U8* ShaderBinary::GetBytes()
{
	return fByteBufferPointer;
}

size_t ShaderBinary::GetByteCount()
{
	return fByteCount;
}

bool ShaderBinary::CopyFrom(const ShaderBinary *binaryPointer)
{
	// Validate argument.
	if (!fAllocatorPointer || !binaryPointer)
	{
		return false;
	}

	// If the given binary is empty, then empty this binary.
	if (binaryPointer->fByteCount < 1)
	{
		Free();
		return true;
	}

	// Copy the given object's binary to this object.
	return CopyFrom(binaryPointer->fByteBufferPointer, binaryPointer->fByteCount);
}

bool ShaderBinary::CopyFrom(const U8 *byteBufferPointer, const size_t byteCount)
{
	// Validate.
	if (!fAllocatorPointer || !byteBufferPointer || (byteCount < 1))
	{
		return false;
	}

	// Optimization:
	// If an allocated buffer already exists, then attempt to resize it to match the given buffer.
	if (fByteBufferPointer && (fByteCount > 0))
	{
		void *newByteBufferPointer = Rtt_REALLOC(fAllocatorPointer, fByteBufferPointer, byteCount);
		if (newByteBufferPointer)
		{
			fByteBufferPointer = (U8*)newByteBufferPointer;
			fByteCount = byteCount;
		}
	}

	// Copy the given byte buffer.
	if (byteCount == fByteCount)
	{
		::memcpy(fByteBufferPointer, byteBufferPointer, byteCount);
	}
	else
	{
		Free();
		fByteBufferPointer = (U8*)Rtt_MALLOC(fAllocatorPointer, byteCount);
		if (fByteBufferPointer)
		{
			::memcpy(fByteBufferPointer, byteBufferPointer, byteCount);
			fByteCount = byteCount;
		}
	}

	// Return true if the given buffer was copied successfully.
	return (fByteCount > 0);
}

bool ShaderBinary::CopyFromHexadecimalString(const char *hexString)
{
	// Validate argument.
	if (!fAllocatorPointer || !hexString)
	{
		return false;
	}

	// Delete the previous byte buffer.
	Free();

	// Fetch the number of characters in the given string.
	// Note: The length is expected to be a multiple of 2 since it takes 2 hex characters to make 1 byte.
	//       If the length is an odd number, then ignore the last character in the string.
	int stringLength = (int)::strlen(hexString);
	if (stringLength % 2)
	{
		stringLength--;
	}

	// Do not continue if the given string is empty.
	if (stringLength <= 0)
	{
		return true;
	}

	// Create a temporary byte buffer to copy the converted hex characters to.
	size_t byteCount = stringLength / 2;
	if (byteCount < 1)
	{
		return false;
	}
	U8 *byteBufferPointer = (U8*)Rtt_MALLOC(fAllocatorPointer, byteCount);
	if (!byteBufferPointer)
	{
		return false;
	}

	// Traverse all characters in the given string.
	bool wasConverted = true;
	for (int index = 0; index < stringLength; index += 2)
	{
		// Convert the next 2 hex characters to one byte.
		// Note: Give up when encountering an invalid character. (ie: A non-hexadecimal character.)
		U8 highNibble = 0;
		U8 lowNibble = 0;
		wasConverted = ConvertHexCharacterToByte(hexString[index], &highNibble);
		if (!wasConverted)
		{
			break;
		}
		wasConverted = ConvertHexCharacterToByte(hexString[index + 1], &lowNibble);
		if (!wasConverted)
		{
			break;
		}
		U8 byteValue = (highNibble << 4) | lowNibble;

		// Copy the decoded byte to the buffer.
		byteBufferPointer[index / 2] = byteValue;
	}

	// If there was a conversion failure, then delete the byte buffer.
	if (!wasConverted)
	{
		Rtt_FREE(byteBufferPointer);
		byteBufferPointer = NULL;
		byteCount = 0;
		return false;
	}

	// Decoding the hex string to byte buffer form was successful.
	// Store the byte buffer and return true.
	fByteBufferPointer = byteBufferPointer;
	fByteCount = byteCount;
	return true;
}

void ShaderBinary::Free()
{
	// Delete this object's byte buffer, if it exists.
	if (fByteBufferPointer)
	{
		Rtt_FREE(fByteBufferPointer);
		fByteBufferPointer = NULL;
		fByteCount = 0;
	}
}


// ----------------------------------------------------------------------------
// Private Functions
// ----------------------------------------------------------------------------

bool ShaderBinary::ConvertHexCharacterToByte(char hexCharacter, U8 *valuePointer)
{
	// Validate pointer.
	if (!valuePointer)
	{
		return false;
	}

	// Attempt to convert the given character.
	U8 value;
	if ((hexCharacter >= '0') && (hexCharacter <= '9'))
	{
		value = (U8)(hexCharacter - '0');
	}
	else if ((hexCharacter >= 'A') && (hexCharacter <= 'F'))
	{
		value = (U8)(hexCharacter - 'A') + 10;
	}
	else if ((hexCharacter >= 'a') && (hexCharacter <= 'f'))
	{
		value = (U8)(hexCharacter - 'a') + 10;
	}
	else
	{
		return false;
	}

	// Conversion succeeded. Return the result.
	*valuePointer = value;
	return true;
}

} // namespace Rtt
