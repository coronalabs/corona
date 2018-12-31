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
#include "WinString.h"
#include "WinFile.h"
#include <string>


WinString::WinString()
:	mBuffer(nullptr),
	mBufferNumChars(0),
	mUTF8Buffer(nullptr)
{
}

WinString::WinString(const char* utf8Text)
:	WinString()
{
	SetUTF8(utf8Text);
}

WinString::WinString(const wchar_t* utf16Text)
:	WinString()
{
	SetUTF16(utf16Text);
}

WinString::WinString(const WinString& string)
:	WinString(string.mBuffer)
{
}

WinString::~WinString()
{
	if (mBuffer)
	{
		delete[] mBuffer;
		mBuffer = nullptr;
	}
	if (mUTF8Buffer)
	{
		delete[] mUTF8Buffer;
		mUTF8Buffer = nullptr;
	}
}

void WinString::operator=(const WinString& string)
{
	SetUTF16(string.GetUTF16());
}

void 
WinString::Expand( int chars )
{
	if (chars <= mBufferNumChars)
	{
		return;
	}

	wchar_t *newBuf = new wchar_t[chars];
	if ( mBuffer )
	{
		wcscpy_s(newBuf, chars, mBuffer);
		delete[] mBuffer;
	}
	else
	{
		newBuf[0] = 0;
	}
	mBuffer = newBuf;
	mBufferNumChars = chars;
}

void
WinString::ResetUTF8()
{
	if (mUTF8Buffer)
	{
		delete[] mUTF8Buffer;
		mUTF8Buffer = nullptr;
	}
}

void
WinString::SetTCHAR( const TCHAR * newString )
{
	if (newString)
	{
		ResetUTF8();
		int newLen = _tcslen( newString ) + 1;
		if ( newLen > mBufferNumChars )
		{
			if (mBuffer)
			{
				delete[] mBuffer;
			}
			mBuffer = new wchar_t[newLen];
			mBufferNumChars = newLen;
		}
		_tcscpy_s(mBuffer, mBufferNumChars, newString);
	}
	else
	{
		Clear();
	}
}

void
WinString::SetUTF16(const wchar_t *newString)
{
	SetTCHAR(newString);
}

const char *
WinString::GetUTF8() const
{
	if (mUTF8Buffer)
	{
		return mUTF8Buffer;
	}
	if (nullptr == mBuffer)
	{
		return nullptr;
	}

	// Convert to UTF8.

	const int utf16Length = _tcslen( mBuffer ) + 1;

	//
	// Get size of destination UTF-8 buffer, in chars (= bytes)
	//
	int utf8Size = ::WideCharToMultiByte(
		CP_UTF8,		// convert to UTF-8
		0,				// default flags
		mBuffer,		// source UTF-16 string
		utf16Length,	// total source string length, in wchar_t's, 
						// including end-of-string \0
		NULL,			// unused - no conversion required in this step
		0,				// request buffer size
		NULL, NULL		// unused
		);
	if ( utf8Size == 0 )
		return "";

	mUTF8Buffer = new char[ utf8Size + 1];
	mUTF8Buffer[utf8Size] = 0;

	int result = ::WideCharToMultiByte(
		CP_UTF8,		// convert to UTF-8
		0,				// default flags
		mBuffer,		// source UTF-16 string
		utf16Length,	// total source string length, in wchar_t's, 
						// including end-of-string \0
		&mUTF8Buffer[0], // destination buffer
		utf8Size,		// destination buffer size, in bytes
		NULL, NULL		// unused
		);

	return mUTF8Buffer;
}

void 
WinString::SetUTF8( const char * utf8 )
{
	Clear();
	Append(utf8);
}

void WinString::Append(const wchar_t *text)
{
	// Do not continue if there is no text to append.
	if (!text)
	{
		return;
	}
	if (L'\0' == text[0])
	{
		if (GetLength() <= 0)
		{
			Append(L'\0');
		}
		return;
	}

	// If this object's string is currently null/empty, then simply assign it the given string.
	int currentLength = GetLength();
	if (currentLength <= 0)
	{
		this->SetUTF16(text);
		return;
	}

	// Append the given text.
	int textLength = (int)wcslen(text);
	this->Expand(currentLength + textLength + 1);
	wcscpy_s(mBuffer + currentLength, (size_t)(textLength + 1), text);
	ResetUTF8();
}

void WinString::Append(const wchar_t character)
{
	int length = GetLength();
	if (character == L'\0')
	{
		Expand(length + 1);
		mBuffer[length] = character;
	}
	else
	{
		Expand(length + 2);
		mBuffer[length] = character;
		mBuffer[length + 1] = L'\0';
	}
	ResetUTF8();
}

// Appends the given string to this object's string.
void WinString::Append(const char *text)
{
	int currentLength;
	int textLength;
	int conversionLength;

	// Validate argument.
	if (nullptr == text)
	{
		return;
	}

	// Get the length of the text to be appended. Do not continue if empty.
	textLength = strlen(text);
	if (textLength < 1)
	{
		if (GetLength() <= 0)
		{
			Append(L'\0');
		}
		return;
	}

	// Get the length of the string to be appended to.
	currentLength = 0;
	if (mBuffer)
	{
		currentLength = _tcsclen(mBuffer);
	}
	conversionLength = ::MultiByteToWideChar(CP_UTF8, 0, text, textLength + 1, nullptr, 0) - 1; // Subtract NULL char.

	// Expand this object's string buffer to make room for the text to be appended.
	if ((currentLength + conversionLength) >= mBufferNumChars)
	{
		Expand(currentLength + conversionLength + 1);   // Add one for the NULL character.
	}

	// Append the given text, converted to wchar.
	// The UTF8/ASCII string is not being appended, so clear it. It will be rebuilt on demand.
	ResetUTF8();
	::MultiByteToWideChar(CP_UTF8, 0, text, textLength + 1, &mBuffer[currentLength], mBufferNumChars);
}

// Appends a portion of the given string to this object's string. Will automatically append a NULL
// character to the end of this object's string if the given string portion does not contain one.
void WinString::Append(const char *text, int startIndex, int charCount)
{
	int currentLength;
	int conversionLength;

	// Validate argument.
	if ((nullptr == text) || (startIndex < 0) || (charCount < 1))
	{
		return;
	}

	// Get the length of the string to be appended to.
	currentLength = 0;
	if (mBuffer)
	{
		currentLength = _tcsclen(mBuffer);
	}
	conversionLength = ::MultiByteToWideChar(CP_UTF8, 0, text + startIndex, charCount, nullptr, 0);

	// Expand this object's string buffer to make room for the text to be appended.
	if ((currentLength + conversionLength) >= mBufferNumChars)
	{
		Expand(currentLength + conversionLength + 1);   // Add one for the NULL character.
	}

	// Append the given text, converted to wchar.
	// The UTF8/ASCII string is not being appended, so clear it. It will be rebuilt on demand.
	ResetUTF8();
	::MultiByteToWideChar(
				CP_UTF8, 0, text + startIndex, charCount, &mBuffer[currentLength], mBufferNumChars);
	mBuffer[currentLength + conversionLength] = TCHAR('\0');
}

// Replaces all instances of "oldSubstring" with "newSubstring" within this object.
void WinString::Replace(const char *oldSubstring, const char *newSubstring)
{
	std::string newStringBuffer;
	std::string oldSubstringBuffer;
	std::string newSubstringBuffer;
	size_t index;

	// Do not continue if substring to find is NULL or empty.
	if ((nullptr == oldSubstring) || ('\0' == oldSubstring[0]))
	{
		return;
	}

	// Do not continue if this object has an empty string.
	if (GetLength() <= 0)
	{
		return;
	}

	// Convert the given arguments into C++ strings.
	oldSubstringBuffer = oldSubstring;
	if (newSubstring)
	{
		newSubstringBuffer = newSubstring;
	}

	// Replace all substrings.
	bool wasModified = false;
	newStringBuffer = GetUTF8();
	for (index = 0; index < newStringBuffer.size(); index += newSubstringBuffer.size())
	{
		index = newStringBuffer.find(oldSubstringBuffer, index);
		if (index == std::string::npos)
		{
			break;
		}
		newStringBuffer.replace(index, oldSubstringBuffer.size(), newSubstringBuffer);
		wasModified = true;
	}
	if (wasModified)
	{
		SetUTF8(newStringBuffer.c_str());
	}
}

// Replaces all instances of "oldSubstring" with "newSubstring" within this object.
void WinString::Replace(const wchar_t *oldSubstring, const wchar_t *newSubstring)
{
	std::wstring newStringBuffer;
	std::wstring oldSubstringBuffer;
	std::wstring newSubstringBuffer;
	size_t index;

	// Do not continue if substring to find is NULL or empty.
	if ((nullptr == oldSubstring) || (L'\0' == oldSubstring[0]))
	{
		return;
	}

	// Do not continue if this object has an empty string.
	if (GetLength() <= 0)
	{
		return;
	}

	// Convert the given arguments into C++ strings.
	oldSubstringBuffer = oldSubstring;
	if (newSubstring)
	{
		newSubstringBuffer = newSubstring;
	}

	// Replace all substrings.
	bool wasModified = false;
	newStringBuffer = GetUTF16();
	for (index = 0; index < newStringBuffer.size(); index += newSubstringBuffer.size())
	{
		index = newStringBuffer.find(oldSubstringBuffer, index);
		if (index == std::string::npos)
		{
			break;
		}
		newStringBuffer.replace(index, oldSubstringBuffer.size(), newSubstringBuffer);
		wasModified = true;
	}
	if (wasModified)
	{
		SetUTF16(newStringBuffer.c_str());
	}
}

// Replaces all instances of "oldCharacter" with "newCharacter" within the string.
void WinString::Replace(const wchar_t oldCharacter, const wchar_t newCharacter)
{
	// Do not continue if given characters match. (String isn't changing.)
	if (oldCharacter == newCharacter)
	{
		return;
	}

	// Perform the UTF-16 character substitution.
	bool wasModified = false;
	for (int index = GetLength() - 1; index >= 0; index--)
	{
		if (mBuffer[index] == oldCharacter)
		{
			mBuffer[index] = newCharacter;
			wasModified = true;
		}
	}

	// Clear the UTF-8 string if at least 1 character was replaced.
	// To be transcoded later, but only on demand.
	if (wasModified)
	{
		ResetUTF8();
	}
}

// Converts all characters in the string to lower case.
void WinString::MakeLowerCase()
{
	// Do not continue if there are no characters to convert.
	if (mBufferNumChars <= 0)
	{
		return;
	}

	// Convert the "TCHAR" and "char" strings to lower case.
	_tcslwr_s(mBuffer, mBufferNumChars);
	_strlwr_s((char*)GetUTF8(), strlen(GetUTF8()) + 1);
}

// Converts all characters in the string to upper case.
void WinString::MakeUpperCase()
{
	// Do not continue if there are no characters to convert.
	if (mBufferNumChars <= 0)
	{
		return;
	}

	// Convert the "TCHAR" and "char" strings to upper case.
	_tcsupr_s(mBuffer, mBufferNumChars);
	_strupr_s((char*)GetUTF8(), strlen(GetUTF8()) + 1);
}

// Determines if this string ends with the given text. This is a case sensitive comparison.
bool WinString::EndsWith(const char *text)
{
	const char* utf8String;
	size_t utf8StringLength;
	size_t textLength;

	// Do not continue if this string object is empty.
	if (IsEmpty())
	{
		return false;
	}

	// Get the length of the given text. Do not continue if empty.
	if (nullptr == text)
	{
		return false;
	}
	textLength = strlen(text);
	if (textLength <= 0)
	{
		return false;
	}

	// Check if the given text exists at the end of this object's string.
	utf8String = GetUTF8();
	utf8StringLength = strlen(utf8String);
	if (utf8StringLength < textLength)
	{
		return false;
	}
	utf8String += utf8StringLength - textLength;
	return (strncmp(utf8String, text, textLength) == 0);
}

/// Removes characters from the end of this object's string if it contains any of the given characters.
/// @param characters Pointer to a null terminated array of characters to trim.
///                   The order of these characters does not matter since this is not a sub string search.
///                   Can be null or empty, in which case, this function will do thing.
void WinString::TrimEnd(const wchar_t *characters)
{
	// Do not continue if not given any characters to search for.
	if (!characters)
	{
		return;
	}
	int filterCharacterCount = (int)wcslen(characters);
	if (filterCharacterCount <= 0)
	{
		return;
	}

	// Do not continue if this string object is empty.
	int stringLength = GetLength();
	if (stringLength <= 0)
	{
		return;
	}

	// Find the index to the last character in the string buffer that should not be trimmed.
	int trimIndex;
	for (trimIndex = stringLength - 1; trimIndex >= 0; trimIndex--)
	{
		bool shouldCharacterBeTrimmed = false;
		for (int filterCharacterIndex = 0; filterCharacterIndex < filterCharacterCount; filterCharacterIndex++)
		{
			if (characters[filterCharacterIndex] == mBuffer[trimIndex])
			{
				shouldCharacterBeTrimmed = true;
				break;
			}
		}
		if (false == shouldCharacterBeTrimmed)
		{
			break;
		}
	}

	// Trim the string.
	if (trimIndex < (stringLength - 1))
	{
		if (trimIndex < 0)
		{
			Clear();
		}
		else
		{
			mBuffer[trimIndex + 1] = L'\0';
			ResetUTF8();
		}
	}
}

/// Removes the given range of characters from the string.
/// @param startIndex Zero based index to the character where removal should start from.
/// @param count Number of characters to be removed from the string. Must be greater than zero.
void WinString::DeleteRange(int startIndex, int count)
{
	// Do not continue if the string is empty. Nothing to delete.
	if (IsEmpty())
	{
		return;
	}

	// Validate arguments.
	if ((startIndex < 0) || (count <= 0))
	{
		return;
	}
	int stringLength = GetLength();
	if (startIndex >= stringLength)
	{
		return;
	}

	// Make sure the given character count does not exceed this string's length.
	if ((startIndex + count) > stringLength)
	{
		count = stringLength - startIndex;
	}

	// Delete the specific character range from the string.
	int trailingCharacterCount = stringLength - (startIndex + count);
	if (trailingCharacterCount <= 0)
	{
		mBuffer[startIndex] = L'\0';
	}
	else
	{
		wmemmove_s(
				mBuffer + startIndex, mBufferNumChars - startIndex,
				mBuffer + startIndex + count, trailingCharacterCount + 1);
	}
	ResetUTF8();
}

/// Case sensitive comparison between this object's string and the given string. Can be used for sorts.
/// @param text Pointer to a NULL terminated string to compare with.
///             Can be NULL which is considered an empty string.
/// @return Returns a negative number of this object's string is less than the given string.
///         Returns zero if this object's string matches the given string.
///         Returns a positive number of this object's string is greater than the given string.
int WinString::Compare(const wchar_t *text) const
{
	// Get the length of this object's string and the given string.
	size_t myLength = GetLength();
	size_t textLength = (text != nullptr) ? wcslen(text) : 0;

	// Quick test for empty strings. (Don't feed the string compare function NULL!)
	if ((0 == myLength) && (0 == textLength))
	{
		return 0;
	}
	else if ((0 == myLength) && (textLength > 0))
	{
		return -1;
	}
	else if ((myLength > 0) && (0 == textLength))
	{
		return 1;
	}

	// Do a case sensitive string comparison.
	return wcscmp(GetUTF16(), text);
}

/// Case sensitive comparison between this object's string and the given string. Can be used for sorts.
/// @param text Pointer to a NULL terminated string to compare with.
///             Can be NULL which is considered an empty string.
/// @return Returns a negative number of this object's string is less than the given string.
///         Returns zero if this object's string matches the given string.
///         Returns a positive number of this object's string is greater than the given string.
int WinString::Compare(const char *text) const
{
	// Get the length of this object's string and the given string.
	size_t myLength = GetLength();
	size_t textLength = (text != nullptr) ? strlen(text) : 0;

	// Quick test for empty strings. (Don't feed the string compare function NULL!)
	if ((0 == myLength) && (0 == textLength))
	{
		return 0;
	}
	else if ((0 == myLength) && (textLength > 0))
	{
		return -1;
	}
	else if ((myLength > 0) && (0 == textLength))
	{
		return 1;
	}

	// Do a case sensitive string comparison.
	return strcmp(GetUTF8(), text);
}

/// Non-case sensitive comparison between this object's string and the given string. Can be used for sorts.
/// @param text Pointer to a NULL terminated string to compare with.
///             Can be NULL which is considered an empty string.
/// @return Returns a negative number of this object's string is less than the given string.
///         Returns zero if this object's string matches the given string.
///         Returns a positive number of this object's string is greater than the given string.
int WinString::CompareNoCase(const wchar_t *text) const
{
	// Get the length of this object's string and the given string.
	size_t myLength = GetLength();
	size_t textLength = (text != nullptr) ? wcslen(text) : 0;

	// Quick test for empty strings. (Don't feed the string compare function NULL!)
	if ((0 == myLength) && (0 == textLength))
	{
		return 0;
	}
	else if ((0 == myLength) && (textLength > 0))
	{
		return -1;
	}
	else if ((myLength > 0) && (0 == textLength))
	{
		return 1;
	}

	// Do a case insensitive string comparison.
	return _wcsicmp(GetUTF16(), text);
}

/// Non-case sensitive comparison between this object's string and the given string. Can be used for sorts.
/// @param text Pointer to a NULL terminated string to compare with.
///             Can be NULL which is considered an empty string.
/// @return Returns a negative number of this object's string is less than the given string.
///         Returns zero if this object's string matches the given string.
///         Returns a positive number of this object's string is greater than the given string.
int WinString::CompareNoCase(const char *text) const
{
	// Get the length of this object's string and the given string.
	size_t myLength = GetLength();
	size_t textLength = (text != nullptr) ? strlen(text) : 0;

	// Quick test for empty strings. (Don't feed the string compare function NULL!)
	if ((0 == myLength) && (0 == textLength))
	{
		return 0;
	}
	else if ((0 == myLength) && (textLength > 0))
	{
		return -1;
	}
	else if ((myLength > 0) && (0 == textLength))
	{
		return 1;
	}

	// Do a case insensitive string comparison.
	return _stricmp(GetUTF8(), text);
}

/// Case sensitive match between this object's string and the given string.
/// @param text Reference to the string object to match with.
/// @return Returns true if the strings match.
bool WinString::Equals(const WinString& text) const
{
	// First, check if the 2 object pointers match. (This is an optimization.)
	if (&text == this)
	{
		return true;
	}

	// Compare strings.
	return (Compare(text.GetUTF16()) == 0);
}

/// Case sensitive match between this object's string and the given string.
/// @param text Pointer to a NULL terminated string to match with.
///             Can be NULL which is considered an empty string.
/// @return Returns true if the strings match.
bool WinString::Equals(const wchar_t *text) const
{
	return (Compare(text) == 0);
}

/// Case sensitive match between this object's string and the given string.
/// @param text Pointer to a NULL terminated string to match with.
///             Can be NULL which is considered an empty string.
/// @return Returns true if the strings match.
bool WinString::Equals(const char *text) const
{
	return (Compare(text) == 0);
}

/// Non-case sensitive match between this object's string and the given string.
/// @param text Reference to the string object to match with.
/// @return Returns true if the strings match.
bool WinString::EqualsNoCase(const WinString &text) const
{
	// First, check if the 2 object pointers match. (This is an optimization.)
	if (&text == this)
	{
		return true;
	}

	// Compare strings.
	return (CompareNoCase(text.GetUTF16()) == 0);
}

/// Non-case sensitive match between this object's string and the given string.
/// @param text Pointer to a NULL terminated string to match with.
///             Can be NULL which is considered an empty string.
/// @return Returns true if the strings match.
bool WinString::EqualsNoCase(const wchar_t *text) const
{
	return (CompareNoCase(text) == 0);
}

/// Non-case sensitive match between this object's string and the given string.
/// @param text Pointer to a NULL terminated string to match with.
///             Can be NULL which is considered an empty string.
/// @return Returns true if the strings match.
bool WinString::EqualsNoCase(const char *text) const
{
	return (CompareNoCase(text) == 0);
}

/// Case sensitive match between this object's string and the given string.
/// @param text Reference to the string to be compared against.
/// @return Returns true if the strings match.
bool WinString::operator==(const WinString &text) const
{
	return Equals(text);
}

/// Determines if the given string and this string do not match.
/// @param text Reference to the string to be compared against.
/// @return Returns true if the strings do not match. Returns false if they do match.
bool WinString::operator!=(const WinString &text) const
{
	return !Equals(text);
}

/// Read a file into a WinString (assumed to be a text file)
/// @param filename Name of desired file
/// @return Returns true if the file is successfully read.
bool WinString::GetFileContents(const char *filename)
{
	WinFile theFile;
		
	theFile.Open( filename );

	if (!theFile.IsOpen())
	{
		return false;
	}

	Clear();
	Append( (const char *) theFile.GetContents(), 0, theFile.GetFileSize() );

	theFile.Close();

	return true;
}

/// Write a file from a WinString (assumed to be a text file)
/// @param filename Name of desired file
/// @return Returns true if the file is successfully written.
bool WinString::WriteFileContents(const char *filename)
{
	WinString filepath;
	HANDLE fileHandle;
    DWORD dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
    DWORD dwShareMode = FILE_SHARE_READ;
	DWORD bytesWritten = 0;

	filepath.SetUTF8( filename );

	fileHandle = ::CreateFile(
		filepath.GetTCHAR(),
		dwDesiredAccess,
		dwShareMode,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);

	if ( fileHandle == INVALID_HANDLE_VALUE )
	{
		DWORD err = ::GetLastError();
		return false;
	}	

	// Make sure the UTF8 buffer is current
	GetUTF8();

	if (mUTF8Buffer)
	{
		if (! ::WriteFile(fileHandle, mUTF8Buffer, strlen(mUTF8Buffer), &bytesWritten, NULL))
		{
			DWORD err = ::GetLastError();
			return false;
		}	
	}

	::CloseHandle( fileHandle );

	return true;
}

int WinString::Format(const char *format, ...)
{
	int result = -1;

	va_list ap;
	va_start(ap, format);
	int length = _vscprintf(format, ap);
	if (length > 0)
	{
		char* utf8String = new char[length + 1];
		utf8String[0] = '\0';
		utf8String[length] = '\0';
		result = vsnprintf_s(utf8String, length + 1, length, format, ap);
		SetUTF8(utf8String);
		delete[] utf8String;
	}
	else
	{
		Clear();
	}
	va_end( ap );

	return result;
}

int WinString::Format(const wchar_t *format, ...)
{
	int result = -1;

	va_list ap;
	va_start(ap, format);
	int length = _vscwprintf(format, ap);
	if (length > 0)
	{
		auto utf16String = new wchar_t[length + 1];
		utf16String[0] = L'\0';
		utf16String[length] = L'\0';
		result = _vsnwprintf_s(utf16String, length + 1, length, format, ap);
		SetUTF16(utf16String);
		delete[] utf16String;
	}
	else
	{
		Clear();
	}
	va_end(ap);

	return result;
}
