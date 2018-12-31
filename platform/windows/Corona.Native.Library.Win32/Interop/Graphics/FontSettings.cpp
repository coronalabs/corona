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
#include "FontSettings.h"
#include "Rtt\Rtt_WinFont.h"
#include "Font.h"
#include "FontSizeConverter.h"


namespace Interop { namespace Graphics {

#pragma region Constructors/Destructors
FontSettings::FontSettings()
:	fIsBold(false),
	fIsItalic(false),
	fPixelSize(8.0f)
{
}

FontSettings::~FontSettings()
{
}

#pragma endregion


#pragma region Public Methods
const wchar_t* FontSettings::GetUtf16Name() const
{
	return fName.GetUTF16();
}

const char* FontSettings::GetUtf8Name() const
{
	return fName.GetUTF8();
}

void FontSettings::SetName(const wchar_t* name)
{
	fName.SetUTF16(name);
}

void FontSettings::SetName(const char* name)
{
	fName.SetUTF8(name);
}

bool FontSettings::IsBold() const
{
	return fIsBold;
}

void FontSettings::SetIsBold(bool value)
{
	fIsBold;
}

bool FontSettings::IsItalic() const
{
	return fIsItalic;
}

void FontSettings::SetIsItalic(bool value)
{
	fIsItalic = value;
}

Gdiplus::FontStyle FontSettings::GetGdiPlusStyle() const
{
	Gdiplus::FontStyle fontStyle = Gdiplus::FontStyleRegular;
	if (fIsBold && fIsItalic)
	{
		fontStyle = Gdiplus::FontStyleBoldItalic;
	}
	else if (fIsBold)
	{
		fontStyle = Gdiplus::FontStyleBold;
	}
	else if (fIsItalic)
	{
		fontStyle = Gdiplus::FontStyleItalic;
	}
	return fontStyle;
}

float FontSettings::GetPixelSize() const
{
	return fPixelSize;
}

void FontSettings::SetPixelSize(float value)
{
	if (value < 1.0f)
	{
		value = 1.0f;
	}
	fPixelSize = value;
}

bool FontSettings::CopyFrom(HDC deviceContextHandle, const LOGFONTW& logFont)
{
	// Validate.
	if (!deviceContextHandle)
	{
		return false;
	}

	// Validate font family name string by making sure it contains a null character.
	bool isFontNameValid = false;
	for (int index = 0; index < LF_FACESIZE; index++)
	{
		if (L'\0' == logFont.lfFaceName[index])
		{
			isFontNameValid = true;
			break;
		}
	}
	if (!isFontNameValid)
	{
		return false;
	}

	// Convert font size from logical units to pixels, excluding line spacing/padding.
	float fontSize = (float)logFont.lfHeight;
	if (logFont.lfHeight != 0)
	{
		if (logFont.lfHeight > 0)
		{
			auto fontHandle = ::CreateFontIndirectW(&logFont);
			if (fontHandle)
			{
				auto previousFontHandle = ::SelectObject(deviceContextHandle, fontHandle);
				TEXTMETRICW textMetrics{};
				::GetTextMetricsW(deviceContextHandle, &textMetrics);
				fontSize = (float)(logFont.lfHeight - textMetrics.tmInternalLeading);
				::SelectObject(deviceContextHandle, previousFontHandle);
				::DeleteObject(fontHandle);
			}
		}
		else if (logFont.lfHeight < 0)
		{
			fontSize *= -1.0f;
		}
	}

	// Copy the given settings.
	fName.SetUTF16(logFont.lfFaceName);
	fPixelSize = fontSize;
	fIsBold = (logFont.lfWeight >= FW_BOLD);
	fIsItalic = (logFont.lfItalic != 0);

	// Copy was successful.
	return true;
}

bool FontSettings::CopyFrom(HDC deviceContextHandle, const Font& font)
{
	// Do not continue if the given font object is invalid.
	if (font.IsInvalid())
	{
		return false;
	}

	// Fetch the Win32 GDI+ font object from the given font reference.
	auto gdiPlusFontPointer = font.GetGdiPlusFont();
	if (!gdiPlusFontPointer || !gdiPlusFontPointer->IsAvailable())
	{
		return false;
	}

	// Create a Win32 LOGFONT struct from the given font.
	auto result = font.CreateLogFontUsing(deviceContextHandle);
	if (result.HasFailed())
	{
		return false;
	}

	// Copy the LOGFONT's settings first.
	// This is the most reliable way to acquire the font family name.
	// Note: The Gdiplus::Font::GetFamily() method can trash memory and cause a crash. Don't use it!!!
	bool wasCopied = CopyFrom(deviceContextHandle, result.GetValue());
	if (!wasCopied)
	{
		return false;
	}

	// Fetch the fractional font size from the GDI+ font object.
	// Note: The LOGFONT above stores font size as an integer, but GDI+ stores it as a float.
	FontSizeConverter fontSizeConverter(gdiPlusFontPointer->GetSize(), gdiPlusFontPointer->GetUnit());
	fontSizeConverter.ConvertTo(Gdiplus::UnitPixel);
	fPixelSize = fontSizeConverter.GetSize();

	// Copy was successful.
	return true;
}

void FontSettings::CopyFrom(const Rtt::WinFont& font, Gdiplus::Unit units)
{
	// Copy the core font settings.
	fName.SetUTF8(font.Name());
	fIsBold = font.IsBold();
	fIsItalic = font.IsItalic();

	// Convert the given font size to pixel and store it.
	FontSizeConverter fontSizeConverter(font.Size(), units);
	fontSizeConverter.ConvertTo(Gdiplus::UnitPixel);
	fPixelSize = fontSizeConverter.GetSize();
}

void FontSettings::CopyTo(Rtt::WinFont& font) const
{
	font.SetName(fName.GetUTF8());
	font.SetSize(fPixelSize);
	font.SetBold(fIsBold);
	font.SetItalic(fIsItalic);
}

#pragma endregion

} }	// namespace Interop::Graphics
