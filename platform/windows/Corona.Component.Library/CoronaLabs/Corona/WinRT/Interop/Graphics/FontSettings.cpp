// ----------------------------------------------------------------------------
// 
// FontSettings.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "FontSettings.h"
#include <stdio.h>
#include <wchar.h>


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

#pragma region Consructors/Destructors
FontSettings::FontSettings()
:	fFilePath(nullptr),
	fFamilyName(nullptr),
	fIsBold(false),
	fIsItalic(false),
	fPointSize(8.0f)
{
}

#pragma endregion


#pragma region Public Properties/Methods
void FontSettings::CopyFrom(FontSettings^ settings)
{
	// Validate.
	if (!settings)
	{
		return;
	}

	// Copy the given settings.
	fFilePath = settings->fFilePath;
	fFamilyName = settings->fFamilyName;
	fIsBold = settings->fIsBold;
	fIsItalic = settings->fIsItalic;
	fPointSize = settings->fPointSize;
}

Platform::String^ FontSettings::FilePath::get()
{
	return fFilePath;
}

void FontSettings::FilePath::set(Platform::String^ value)
{
	fFilePath = value;
}

Platform::String^ FontSettings::FamilyName::get()
{
	return fFamilyName;
}

void FontSettings::FamilyName::set(Platform::String^ value)
{
	fFamilyName = value;
}

bool FontSettings::IsBold::get()
{
	return fIsBold;
}

void FontSettings::IsBold::set(bool value)
{
	fIsBold = value;
}

bool FontSettings::IsItalic::get()
{
	return fIsItalic;
}

void FontSettings::IsItalic::set(bool value)
{
	fIsItalic = value;
}

float FontSettings::PointSize::get()
{
	return fPointSize;
}

void FontSettings::PointSize::set(float value)
{
	if (value < 1.0f)
	{
		value = 1.0f;
	}
	fPointSize = value;
}

Platform::String^ FontSettings::ToString()
{
	const wchar_t TRUE_STRING[] = L"true";
	const wchar_t FALSE_STRING[] = L"false";
	const int MAX_MESSAGE_LENGTH = 512;
	wchar_t message[MAX_MESSAGE_LENGTH];

	_snwprintf_s(
			message, MAX_MESSAGE_LENGTH,
			L"{ FilePath=\"%s\", FamilyName=\"%s\", IsBold=%s, IsItalic=%s, PointSize=%0.1f }",
			fFilePath ? fFilePath->Data() : L"",
			fFamilyName ? fFamilyName->Data() : L"",
			fIsBold ? TRUE_STRING : FALSE_STRING,
			fIsItalic ? TRUE_STRING : FALSE_STRING,
			fPointSize);
	return ref new Platform::String(message);
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
