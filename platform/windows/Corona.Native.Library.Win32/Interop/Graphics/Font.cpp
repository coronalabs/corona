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
#include "Font.h"
#include "FontSettings.h"
#include "MFontLoader.h"


namespace Interop { namespace Graphics {

#pragma region Constructors/Destructors
Font::Font()
{
}

Font::Font(HDC deviceContextHandle, MFontLoader& fontLoader, const FontSettings& fontSettings)
{
	// Validate.
	if (!deviceContextHandle)
	{
		return;
	}

	// Load a GDI+ font using the given settings.
	fGdiPlusFontPointer = fontLoader.LoadUsing(fontSettings);
	if (!fGdiPlusFontPointer || !fGdiPlusFontPointer->IsAvailable())
	{
		return;
	}

	// Load an old GDI font using the GDI+ LOGFONT settings.
	fGdiFontPointer = Font::GdiFont::CreateUsing(deviceContextHandle, *fGdiPlusFontPointer);
}

Font::Font(HDC deviceContextHandle, const std::shared_ptr<Gdiplus::Font>& fontPointer)
{
	// Validate.
	if (!deviceContextHandle || !fontPointer || !fontPointer->IsAvailable())
	{
		return;
	}

	// Store the given GDI+ font.
	fGdiPlusFontPointer = fontPointer;

	// Load an old GDI font using the GDI+ LOGFONT settings.
	fGdiFontPointer = Font::GdiFont::CreateUsing(deviceContextHandle, *fGdiPlusFontPointer);
}

Font::Font(HDC deviceContextHandle, const LOGFONTW& logFont)
{
	// Load a GDI+ font using the given settings.
	fGdiPlusFontPointer = std::make_shared<Gdiplus::Font>(deviceContextHandle, &logFont);
	if (fGdiPlusFontPointer->IsAvailable() == false)
	{
		return;
	}

	// Load an old GDI font using the GDI+ LOGFONT settings.
	fGdiFontPointer = Font::GdiFont::CreateUsing(deviceContextHandle, *fGdiPlusFontPointer);
}

Font::~Font()
{
}

#pragma endregion


#pragma region Public Methods
bool Font::IsValid() const
{
	return (fGdiPlusFontPointer && fGdiPlusFontPointer->IsAvailable());
}

bool Font::IsInvalid() const
{
	return !IsValid();
}

HFONT Font::GetFontHandle() const
{
	return fGdiFontPointer ? fGdiFontPointer->GetFontHandle() : nullptr;
}

const Gdiplus::Font* Font::GetGdiPlusFont() const
{
	return fGdiPlusFontPointer ? fGdiPlusFontPointer.get() : nullptr;
}

Font::LogFontResult Font::CreateLogFontUsing(HDC deviceContextHandle) const
{
	// Validate.
	if (!deviceContextHandle)
	{
		return LogFontResult::FailedWith(L"Cannot create a LOGFONT with a null device context handle.");
	}
	if (IsInvalid())
	{
		return LogFontResult::FailedWith(L"Cannot create with an invalid/null font object.");
	}

	// Create a graphics context that converts from whatever the GDI+ font size units are to GDI logical units.
	Gdiplus::Graphics graphics(deviceContextHandle);

	// Create a GDI LOGFONT struct from this object's GDI+ font object.
	LOGFONTW logFont{};
	auto result = fGdiPlusFontPointer->GetLogFontW(&graphics, &logFont);
	if (result != Gdiplus::Ok)
	{
		const size_t kMaxMessageLength = 256;
		wchar_t message[kMaxMessageLength];
		_snwprintf_s(message, kMaxMessageLength, _TRUNCATE, L"Failed to create LOGFONT with GDI+ error code: %d", result);
		return LogFontResult::FailedWith(message);
	}
	return LogFontResult::SucceededWith(logFont);
}

#pragma endregion


#pragma region InternalData Struct
Font::GdiFont::GdiFont()
:	fHandle(nullptr)
{
}

Font::GdiFont::GdiFont(const LOGFONTW& logFont)
:	GdiFont()
{
	fHandle = ::CreateFontIndirectW(&logFont);
}

Font::GdiFont::~GdiFont()
{
	if (fHandle)
	{
		::DeleteObject(fHandle);
		fHandle = nullptr;
	}
}

std::shared_ptr<Font::GdiFont> Font::GdiFont::CreateUsing(
	HDC deviceContextHandle, const Gdiplus::Font& gdiPlusFont)
{
	// Create a GDI font using the given GDI+ font's settings.
	std::shared_ptr<Font::GdiFont> gdiFontPointer;
	if (deviceContextHandle && gdiPlusFont.IsAvailable())
	{
		// Create a graphics context that converts from whatever the GDI+ font size units are to GDI logical units.
		Gdiplus::Graphics graphics(deviceContextHandle);

		// Create a GDI LOGFONT structure from the GDI+ font object.
		LOGFONTW logFont{};
		gdiPlusFont.GetLogFontW(&graphics, &logFont);

		// Create the GDI font object from the LOGFONT.
		gdiFontPointer = std::make_shared<Font::GdiFont>(logFont);
		if (!gdiFontPointer->fHandle)
		{
			gdiFontPointer = nullptr;
		}
	}
	return gdiFontPointer;
}

HFONT Font::GdiFont::GetFontHandle() const
{
	return fHandle;
}

#pragma endregion

} }	// namespace Interop::Graphics
