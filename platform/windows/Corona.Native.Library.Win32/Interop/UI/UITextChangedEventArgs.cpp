//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
