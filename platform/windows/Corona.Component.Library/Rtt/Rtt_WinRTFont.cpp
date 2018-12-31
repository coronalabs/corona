// ----------------------------------------------------------------------------
// 
// Rtt_WinRTFont.cpp
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "Rtt_WinRTFont.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\FontSettings.h"
#include "CoronaLabs\Corona\WinRT\CoronaRuntimeEnvironment.h"
#include "CoronaLabs\WinRT\NativeStringServices.h"
#include "Rtt_WinRTPlatform.h"


namespace Rtt
{

#pragma region Constructors/Destructors
WinRTFont::WinRTFont(CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ environment, const char *fontName)
:	PlatformFont(),
	fEnvironment(environment),
	fFontName(&environment->NativePlatformPointer->GetAllocator(), fontName)
{
	// Attempt to fetch a "font family name" and "font file path" from the given font name, if provided.
	Platform::String^ utf16FontFilePath = nullptr;
	Platform::String^ utf16FontFamilyName = nullptr;
	if (!Rtt_StringIsEmpty(fontName))
	{
		// Fetch font information.
		const size_t MAX_FONT_FILE_NAME_LENGTH = 512;
		auto platformPointer = environment->NativePlatformPointer;
		Rtt::String utf8FontFilePath(&platformPointer->GetAllocator());
		const char* utf8FontFamilyName = strrchr(fontName, '#');
		if (utf8FontFamilyName)
		{
			// The given font name contains a '#' character. Assume it's formatted as "<FontFilePath>#<FontFamilyName>".
			char utf8FontFileName[MAX_FONT_FILE_NAME_LENGTH];
			strncpy_s(utf8FontFileName, MAX_FONT_FILE_NAME_LENGTH, fontName, utf8FontFamilyName - fontName);
			platformPointer->PathForFile(
					utf8FontFileName, MPlatform::kResourceDir, MPlatform::kDefaultPathFlags, utf8FontFilePath);
		}
//TODO: Add the ability to extract font family name from a font file below.
//      Until then, there is no point in attempting to fetch a font file path since XAML requires a font family name to use it.
#if 0
		else if (Rtt_StringEndsWithNoCase(fontName, ".ttf") || Rtt_StringEndsWithNoCase(fontName, ".otf"))
		{
			// The given font name has a standard font file extension.
			// Change the name to an absolute path to the referenced font file.
			platformPointer->PathForFile(fontName, MPlatform::kResourceDir, MPlatform::kDefaultPathFlags, utf8FontFilePath);
		}
		else
		{
			// Check if the font name is reference a *.ttf file in the Resource directory.
			char utf8FontFileName[MAX_FONT_FILE_NAME_LENGTH];
			strcpy_s(utf8FontFileName, MAX_FONT_FILE_NAME_LENGTH, fontName);
			auto result = strcat_s(utf8FontFileName, MAX_FONT_FILE_NAME_LENGTH, ".ttf");
			if (0 == result)
			{
				platformPointer->PathForFile(
						utf8FontFileName, MPlatform::kResourceDir, MPlatform::kTestFileExists, utf8FontFilePath);
				if (utf8FontFilePath.IsEmpty())
				{
					// Check if the font name is referencing a *.otf file in the Resource directory.
					utf8FontFileName[strlen(utf8FontFileName) - 4] = '\0';
					strcat_s(utf8FontFileName, MAX_FONT_FILE_NAME_LENGTH, ".otf");
					platformPointer->PathForFile(
							utf8FontFileName, MPlatform::kResourceDir, MPlatform::kTestFileExists, utf8FontFilePath);
				}
			}
		}
#endif

		// Convert the UTF-8 paths and names to UTF-16.
		if (utf8FontFilePath.IsEmpty() == false)
		{
			utf16FontFilePath = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(utf8FontFilePath.GetString());
		}
		if (utf8FontFamilyName)
		{
			utf16FontFamilyName = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(utf8FontFamilyName);
		}
		else
		{
			utf16FontFamilyName = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(fontName);
		}
	}

	// Create the font settings.
	fSettings = ref new CoronaLabs::Corona::WinRT::Interop::Graphics::FontSettings();
	fSettings->FilePath = utf16FontFilePath;
	fSettings->FamilyName = utf16FontFamilyName;
}

WinRTFont::WinRTFont(const WinRTFont &font)
:	PlatformFont(),
	fEnvironment(font.fEnvironment),
	fSettings(nullptr),
	fFontName(font.fFontName.GetAllocator(), font.fFontName.GetString())
{
	fSettings = ref new CoronaLabs::Corona::WinRT::Interop::Graphics::FontSettings();
	fSettings->CopyFrom(font.fSettings);
}

WinRTFont::~WinRTFont()
{
}

#pragma endregion


#pragma region Public Member Functions
PlatformFont* WinRTFont::CloneUsing(Rtt_Allocator *allocator) const
{
	if (!allocator)
	{
		return nullptr;
	}
	return Rtt_NEW(allocator, WinRTFont(*this));
}

const char* WinRTFont::Name() const
{
	return fFontName.GetString();
}

void WinRTFont::SetSize(Rtt_Real newValue)
{
	fSettings->PointSize = Rtt_RealToFloat(newValue);
}

Rtt_Real WinRTFont::Size() const
{
	return Rtt_FloatToReal(fSettings->PointSize);
}

void* WinRTFont::NativeObject() const
{
	return NULL;
}

CoronaLabs::Corona::WinRT::Interop::Graphics::FontSettings^ WinRTFont::GetSettings()
{
	return fSettings;
}

#pragma endregion

}
