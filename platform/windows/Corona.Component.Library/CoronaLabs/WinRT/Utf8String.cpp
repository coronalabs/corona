//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Utf8String.h"
#include <string.h>
#include <stdlib.h>


namespace CoronaLabs { namespace WinRT {

#pragma region Consructors/Destructors
Utf8String::Utf8String()
{
	fStringBuffer = Utf8String::EmptyStringBuffer;
	fCharacterCount = 0;
}

Utf8String::Utf8String(const char* utf8String)
{
	// First, initialize member variable for an empty UTF-8 string.
	fStringBuffer = Utf8String::EmptyStringBuffer;
	fCharacterCount = 0;

	// Copy the given string's characters to this object, if any.
	if (utf8String)
	{
		int byteCount = (int)strlen(utf8String) + 1;
		if (byteCount > 0)
		{
			fStringBuffer = ref new Platform::Array<unsigned char>(byteCount);
			memcpy(fStringBuffer->Data, utf8String, byteCount);
			fCharacterCount = _mbstrlen(utf8String);
		}
	}
}

Utf8String::Utf8String(const wchar_t* utf16String)
{
	fStringBuffer = CreateUtf8ArrayFrom(utf16String);
	fCharacterCount = fStringBuffer ? _mbstrlen((const char*)(fStringBuffer->Data)) : 0;
}

Utf8String::Utf8String(Platform::String^ utf16String)
{
	fStringBuffer = utf16String ? CreateUtf8ArrayFrom(utf16String->Data()) : nullptr;
	fCharacterCount = fStringBuffer ? _mbstrlen((const char*)(fStringBuffer->Data)) : 0;
}

#pragma endregion


#pragma region Public Instance Functions/Properties
const char* Utf8String::Data::get()
{
	if (fStringBuffer)
	{
		return (const char*)(fStringBuffer->Data);
	}
	return NULL;
}

int Utf8String::CharacterCount::get()
{
	return fCharacterCount;
}

int Utf8String::ByteCount::get()
{
	if (fStringBuffer)
	{
		return fStringBuffer->Length;
	}
	return 0;
}

bool Utf8String::IsEmpty::get()
{
	return (fCharacterCount <= 0);
}

bool Utf8String::IsNotEmpty::get()
{
	return !IsEmpty;
}

Utf8String^ Utf8String::ToLower()
{
	// No need to convert if this string is empty.
	if (this->IsEmpty)
	{
		return this;
	}

	// Convert to UTF-16.
	int conversionLength = ::MultiByteToWideChar(CP_UTF8, 0, this->Data, this->ByteCount, NULL, 0);
	if (conversionLength <= 0)
	{
		return Utf8String::Empty;
	}
	wchar_t* utf16String = new wchar_t[conversionLength];
	::MultiByteToWideChar(CP_UTF8, 0, this->Data, this->ByteCount, utf16String, conversionLength);

	// Convert to lowercase.
	_wcslwr_s(utf16String, conversionLength);
	auto utf8LowercaseString = ref new Utf8String(utf16String);
	delete[] utf16String;

	// Return a new UTF-8 string object converted to lowercase.
	return utf8LowercaseString;
}

Utf8String^ Utf8String::ToUpper()
{
	// No need to convert if this string is empty.
	if (this->IsEmpty)
	{
		return this;
	}

	// Convert to UTF-16.
	int conversionLength = ::MultiByteToWideChar(CP_UTF8, 0, this->Data, this->ByteCount, NULL, 0);
	if (conversionLength <= 0)
	{
		return Utf8String::Empty;
	}
	wchar_t* utf16String = new wchar_t[conversionLength];
	::MultiByteToWideChar(CP_UTF8, 0, this->Data, this->ByteCount, utf16String, conversionLength);

	// Convert to uppercase.
	_wcsupr_s(utf16String, conversionLength);
	auto utf8UppercaseString = ref new Utf8String(utf16String);
	delete[] utf16String;

	// Return a new UTF-8 string object converted to uppercase.
	return utf8UppercaseString;
}

Platform::String^ Utf8String::ToString()
{
	// Convert this UTF-8 string to UTF-16 and then return it as a standard string object.
	if (this->IsNotEmpty)
	{
		int conversionLength = ::MultiByteToWideChar(CP_UTF8, 0, this->Data, this->ByteCount, NULL, 0);
		if (conversionLength > 0)
		{
			wchar_t* utf16String = new wchar_t[conversionLength];
			::MultiByteToWideChar(CP_UTF8, 0, this->Data, this->ByteCount, utf16String, conversionLength);
			auto platformString = ref new Platform::String(utf16String);
			delete[] utf16String;
			return platformString;
		}
	}

	// This string object is empty.
	// Return an empty UTF-16 platform string.
	return ref new Platform::String(L"");
}

bool Utf8String::Equals(Utf8String^ value)
{
	// Not equal if given null.
	if (nullptr == value)
	{
		return false;
	}

	// Equal if the references match.
	if (Platform::Object::ReferenceEquals(this, value))
	{
		return true;
	}

	// Equal if both strings are empty.
	if (this->IsEmpty && value->IsEmpty)
	{
		return true;
	}

	// Not equal if the byte counts don't match.
	if (this->ByteCount != value->ByteCount)
	{
		return false;
	}

	// Compare characters in both string.
	for (int index = this->ByteCount - 1; index >= 0; index--)
	{
		if (this->fStringBuffer[index] != value->fStringBuffer[index])
		{
			// A different character was found. Not equal.
			return false;
		}
	}

	// Both strings match.
	return true;
}

bool Utf8String::Equals(Platform::Object^ value)
{
	// Not equal if given null.
	if (nullptr == value)
	{
		return false;
	}

	// If given a UTF-16 string, then convert this UTF-8 string to UTF-16 and then compare them.
	auto utf16String = dynamic_cast<Platform::String^>(value);
	if (utf16String)
	{
		return this->ToString()->Equals(utf16String);
	}

	// If given an instance of this class, then compare the UTF-8 characters.
	auto utf8String = dynamic_cast<Utf8String^>(value);
	if (utf8String)
	{
		return Equals(utf8String);
	}

	// The given object type cannot be compared with this class. Not equal.
	return false;
}

#pragma endregion


#pragma region Public Static Functions
Utf8String^ Utf8String::Empty::get()
{
	static Utf8String kEmpty;
	return %kEmpty;
}

Utf8String^ Utf8String::From(Platform::String^ utf16String)
{
	// Return an empty string if given a null argument.
	// Note: A null "Platform::String" object is projected to .NET as a "String.Empty" object.
	//       Returning an empty Utf8String here will duplicate this same behavior.
	//       This is especially important since "Plaform::String" can never be set to an empty L"" string
	//       and attempting to do so via 'ref new Platform::String(L"")' will return a nullptr instead.
	//       See MSDN's documentation about HSTRING for more details.
	if (nullptr == utf16String)
	{
		return Utf8String::Empty;
	}

	// Convert the given string to UTF-8.
	return Utf8String::From(utf16String->Data());
}

Utf8String^ Utf8String::From(const wchar_t* utf16String)
{
	// Return null if given a null argument.
	if (nullptr == utf16String)
	{
		return nullptr;
	}

	// Return the empty string constant if given an empty string.
	if (L'\0' == utf16String[0])
	{
		return Utf8String::Empty;
	}

	// Convert the given string to UTF-8.
	return ref new Utf8String(utf16String);
}

#pragma endregion


#pragma region Private Static Functions/Properties
Platform::Array<unsigned char>^ Utf8String::EmptyStringBuffer::get()
{
	static bool sWasEmptryStringBufferInitialized = false;
	static Platform::Array<unsigned char> sEmptyStringBuffer(1);

	if (!sWasEmptryStringBufferInitialized)
	{
		sEmptyStringBuffer.set(0, '\0');
		sWasEmptryStringBufferInitialized = true;
	}
	return %sEmptyStringBuffer;
}

Platform::Array<unsigned char>^ Utf8String::CreateUtf8ArrayFrom(const wchar_t* utf16String)
{
	// If given a null or empty string, then return a shared reference to the empty string.
	if (!utf16String || (L'\0' == utf16String[0]))
	{
		return Utf8String::EmptyStringBuffer;
	}

	// Calculate the number of bytes the given string will be when converted to UTF-8.
	int conversionLength = ::WideCharToMultiByte(CP_UTF8, 0, utf16String, -1, NULL, 0, NULL, NULL);

	// If the given string converts to an empty UTF-8 string, then return a shared reference to an empty string.
	if (conversionLength <= 0)
	{
		return Utf8String::EmptyStringBuffer;
	}

	// Convert the given string to UTF-8.
	auto stringBuffer = ref new Platform::Array<unsigned char>(conversionLength);
	::WideCharToMultiByte(CP_UTF8, 0, utf16String, -1, (LPSTR)(stringBuffer->Data), conversionLength, NULL, NULL);
	return stringBuffer;
}

#pragma endregion


#pragma region LessThanComparer Class Functions
Utf8String::LessThanComparer::LessThanComparer()
{
}

bool Utf8String::LessThanComparer::operator()(const Utf8String^ x, const Utf8String^ y) const
{
	// Do null reference checks first.
	if (!x && !y)
	{
		// Not less-than because both references are null, meaning equal.
		return false;
	}
	else if (x && !y)
	{
		// Not less-than because x is not null and y is null.
		return false;
	}
	else if (!x && y)
	{
		// Is less-than because x is null and y is not.
		return true;
	}

	// If given objects have the same reference, then they are equal. Not less-than.
	auto nonConstantX = const_cast<Utf8String^>(x);
	auto nonConstantY = const_cast<Utf8String^>(y);
	if (Platform::Object::ReferenceEquals(nonConstantX, nonConstantY))
	{
		return false;
	}

	// Compare the values.
	return (strcmp(nonConstantX->Data, nonConstantY->Data) < 0);
}

#pragma endregion

} }	// namespace CoronaLabs::WinRT
