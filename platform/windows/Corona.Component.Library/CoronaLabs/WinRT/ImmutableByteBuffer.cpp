//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "ImmutableByteBuffer.h"


namespace CoronaLabs { namespace WinRT {

#pragma region Consructors/Destructors
ImmutableByteBuffer::ImmutableByteBuffer(Windows::Foundation::Collections::IVectorView<byte>^ bytes)
:	fBytes(bytes)
{
	if (nullptr == bytes)
	{
		throw ref new Platform::NullReferenceException();
	}
}

#pragma endregion


#pragma region Public Methods/Properties
byte ImmutableByteBuffer::GetByIndex(int index)
{
	// Validate index.
	if ((index < 0) || (index >= (int)(fBytes->Size)))
	{
		throw ref new Platform::OutOfBoundsException();
	}

	// Return the indexed byte.
	return fBytes->GetAt((unsigned int)index);
}

bool ImmutableByteBuffer::IsEmpty::get()
{
	return (fBytes->Size < 1);
}

bool ImmutableByteBuffer::IsNotEmpty::get()
{
	return (fBytes->Size > 0);
}

int ImmutableByteBuffer::Count::get()
{
	return (int)(fBytes->Size);
}

Windows::Foundation::Collections::IIterator<byte>^ ImmutableByteBuffer::First()
{
	return fBytes->First();
}

#pragma endregion


#pragma region Public Static Functions
ImmutableByteBuffer^ ImmutableByteBuffer::Empty::get()
{
	static ImmutableByteBuffer sImmutableByteBuffer(ref new Platform::Collections::VectorView<byte>());
	return %sImmutableByteBuffer;
}

ImmutableByteBuffer^ ImmutableByteBuffer::From(Windows::Foundation::Collections::IIterable<byte>^ bytes)
{
	// Validate argument.
	if (nullptr == bytes)
	{
		return nullptr;
	}

	// Copy the given collection's bytes to a new collection.
	auto mutableByteCollection = ref new Platform::Collections::Vector<byte>();
	for (auto&& nextByte : bytes)
	{
		mutableByteCollection->Append(nextByte);
	}

	// If no bytes were copied, then return the pre-allocated "empty" instance.
	if (mutableByteCollection->Size < 1)
	{
		return ImmutableByteBuffer::Empty;
	}

	// Wrap the copied bytes inside a new ImmutableByteBuffer and return it.
	return ref new ImmutableByteBuffer(mutableByteCollection->GetView());
}

ImmutableByteBuffer^ ImmutableByteBuffer::From(Windows::Foundation::Collections::IVector<byte>^ bytes)
{
	// Validate argument.
	if (nullptr == bytes)
	{
		return nullptr;
	}

	// If the given collection is empty, then return the pre-allocated "empty" instance.
	if (bytes->Size < 1)
	{
		return ImmutableByteBuffer::Empty;
	}

	// Copy the given collection bytes to a new array.
	auto byteArray = ref new Platform::Array<byte>(bytes->Size);
	bytes->GetMany(0, byteArray);

	// Wrap the array of copied bytes in a new ImmutableByteBuffer and return it.
	auto readOnlyByteCollection = ref new Platform::Collections::VectorView<byte>(byteArray);
	return ref new ImmutableByteBuffer(readOnlyByteCollection);
}

ImmutableByteBuffer^ ImmutableByteBuffer::From(const Platform::Array<byte>^ bytes)
{
	// Validate argument.
	if (nullptr == bytes)
	{
		return nullptr;
	}

	// If the given array is empty, then return the pre-allocated "empty" instance.
	if (bytes->Length < 1)
	{
		return ImmutableByteBuffer::Empty;
	}

	// Copy the given byte array to a new array.
	auto byteArray = ref new Platform::Array<byte>(bytes->Length);
	memcpy(byteArray->Data, bytes->Data, bytes->Length);

	// Wrap the array of copied bytes in a new ImmutableByteBuffer and return it.
	auto readOnlyByteCollection = ref new Platform::Collections::VectorView<byte>(byteArray);
	return ref new ImmutableByteBuffer(readOnlyByteCollection);
}

#pragma endregion


#pragma region Internal Static Functions
ImmutableByteBuffer^ ImmutableByteBuffer::From(const char *stringPointer)
{
	// Validate argument.
	if (nullptr == stringPointer)
	{
		return nullptr;
	}

	// If the given string is empty, then return the pre-allocated "empty" instance.
	if ('\0' == stringPointer[0])
	{
		return ImmutableByteBuffer::Empty;
	}

	// Copy the given string's characters to a new byte buffer and return it.
	return From(stringPointer, (int)strlen(stringPointer));
}

ImmutableByteBuffer^ ImmutableByteBuffer::From(const char *stringPointer, int count)
{
	// Validate arguments.
	if (nullptr == stringPointer)
	{
		return nullptr;
	}
	if (count < 0)
	{
		throw ref new Platform::OutOfBoundsException();
	}

	// Return the pre-allocated "empty" instance if there are no characters to copy.
	if (0 == count)
	{
		return ImmutableByteBuffer::Empty;
	}

	// Copy the given string's characters to a new array.
	auto byteArray = ref new Platform::Array<byte>(count);
	memcpy(byteArray->Data, stringPointer, count);

	// Wrap the array of copied characters in a new ImmutableByteBuffer and return it.
	auto readOnlyByteCollection = ref new Platform::Collections::VectorView<byte>(byteArray);
	return ref new ImmutableByteBuffer(readOnlyByteCollection);
}

#pragma endregion

} }	// namespace CoronaLabs::WinRT
