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
#include "UITextChangedEventArgs.h"


namespace Interop { namespace UI {

#pragma region Constructors/Destructors
UITextChangedEventArgs::UITextChangedEventArgs(const UITextChangedEventArgs::Settings& settings)
:	fNewText(settings.NewText),
	fPreviousText(settings.PreviousText),
	fPreviousStartSelectionIndex(settings.PreviousStartSelectionIndex),
	fAddedCharacterCount(settings.AddedCharacterCount),
	fDeletedCharacterCount(settings.DeletedCharacterCount)
{
	// Ensure that string pointers are never null. Use an empty string instead.
	if (fNewText.GetUTF16() == nullptr)
	{
		fNewText.SetUTF16(L"");
	}
	if (fPreviousText.GetUTF16() == nullptr)
	{
		fPreviousText.SetUTF16(L"");
	}

	// Ensure that the selection index is valid.
	if (fPreviousStartSelectionIndex < 0)
	{
		fPreviousStartSelectionIndex = 0;
	}
	else if (fPreviousStartSelectionIndex > fPreviousText.GetLength())
	{
		fPreviousStartSelectionIndex = fPreviousText.GetLength();
	}

	// Ensure the character counts are never negative.
	if (fAddedCharacterCount < 0)
	{
		fAddedCharacterCount = 0;
	}
	if (fDeletedCharacterCount < 0)
	{
		fDeletedCharacterCount = 0;
	}
}

UITextChangedEventArgs::~UITextChangedEventArgs()
{
}

#pragma endregion


#pragma region Public Methods
const char* UITextChangedEventArgs::GetNewTextAsUtf8() const
{
	return fNewText.GetUTF8();
}

const wchar_t* UITextChangedEventArgs::GetNewTextAsUtf16() const
{
	return fNewText.GetUTF16();
}

const char* UITextChangedEventArgs::GetPreviousTextAsUtf8() const
{
	return fPreviousText.GetUTF8();
}

const wchar_t* UITextChangedEventArgs::GetPreviousTextAsUtf16() const
{
	return fPreviousText.GetUTF16();
}

int UITextChangedEventArgs::GetPreviousStartSelectionIndex() const
{
	return fPreviousStartSelectionIndex;
}

int UITextChangedEventArgs::GetAddedCharacterCount() const
{
	return fAddedCharacterCount;
}

int UITextChangedEventArgs::GetDeletedCharacterCount() const
{
	return fDeletedCharacterCount;
}

#pragma endregion

} }	// namespace Interop::UI
