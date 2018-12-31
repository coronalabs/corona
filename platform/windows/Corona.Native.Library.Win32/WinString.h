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

/**
 * Encapsulates a Windows string. This is a TCHAR, used by many windows system functions, and
 * can be accessed as a utf8 string.
 *
 * This implementation currently assumes TCHAR is UTF16.
 */

#ifndef _WinString_H_
#define _WinString_H_

#include <windows.h>
#include <string.h>
#include <tchar.h>

class WinString
{
private:
	mutable int				mBufferNumChars;
	mutable wchar_t *		mBuffer;
	mutable char *			mUTF8Buffer;

public:
	WinString();
	WinString(const char* utf8Text);
	WinString(const wchar_t* utf16Text);
	WinString(const WinString& string);
	virtual ~WinString();

	// 
    // operator to cast to a const char *
    // 
    operator const char *()
    {
		return GetUTF8();
    }

	void operator=(const WinString& string);

	void Expand( int chars );

	const char * GetUTF8() const;

	void SetUTF8( const char * newString );

	void ResetUTF8();

	void Append(const wchar_t *text);
	void Append(const wchar_t character);
	void Append(const char *text);
	void Append(const char *text, int startIndex, int charCount);
	void Replace(const char *oldSubstring, const char *newSubstring);
	void Replace(const wchar_t *oldSubstring, const wchar_t *newSubstring);
	void Replace(const wchar_t oldCharacter, const wchar_t newCharacter);
	void MakeLowerCase();
	void MakeUpperCase();
	bool EndsWith(const char *text);
	void TrimEnd(const wchar_t *characters);
	void DeleteRange(int startIndex, int count);
	int Compare(const wchar_t *text) const;
	int Compare(const char *text) const;
	int CompareNoCase(const wchar_t *text) const;
	int CompareNoCase(const char *text) const;
	bool Equals(const WinString &text) const;
	bool Equals(const wchar_t *text) const;
	bool Equals(const char *text) const;
	bool EqualsNoCase(const WinString &text) const;
	bool EqualsNoCase(const wchar_t *text) const;
	bool EqualsNoCase(const char *text) const;
	bool operator==(const WinString &text) const;
	bool operator!=(const WinString &text) const;
	bool GetFileContents(const char *filename);
	bool WriteFileContents(const char *filename);
	int Format(const char *format, ...);
	int Format(const wchar_t *format, ...);

	void Clear()
	{
		if (mBuffer)
		{
			mBuffer[0] = 0;
		}
		ResetUTF8();
	}

	bool IsEmpty() const
	{
		if ((nullptr == mBuffer) || (0 == mBuffer[0]))
		{
			return true;
		}
		return false;
	}

	int GetLength() const
	{
		if (nullptr == mBuffer)
		{
			return 0;
		}
		return wcslen(mBuffer);
	}

	int GetBufferNumChars() const 
	{
		return mBufferNumChars;
	}

	const TCHAR * GetTCHAR() const
	{
		return mBuffer;
	}

	const wchar_t* GetUTF16() const
	{
		return mBuffer;
	}

	void SetTCHAR( const TCHAR * newString );

	void SetUTF16( const wchar_t * newString );

	TCHAR * GetBuffer()
	{
		return mBuffer;
	}
};

#endif
