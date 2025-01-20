//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FontSizeConverter.h"


namespace Interop { namespace Graphics {

#pragma region Constructors/Destructors
FontSizeConverter::FontSizeConverter()
:	fSize(8.0f),
	fUnit(Gdiplus::UnitPixel),
	fDpi(96.0f)
{
}

FontSizeConverter::FontSizeConverter(float size, Gdiplus::Unit unit)
:	FontSizeConverter()
{
	SetSize(size, unit);
}

FontSizeConverter::~FontSizeConverter()
{
}

#pragma endregion


#pragma region Public Methods
void FontSizeConverter::SetSize(float size, Gdiplus::Unit unit)
{
	fSize = size;
	fUnit = unit;
	fDpi = 96.0f;
}

bool FontSizeConverter::SetSizeUsing(HDC deviceContextHandle, const LOGFONTW& logFont)
{
	// Validate.
	if (!deviceContextHandle)
	{
		return false;
	}

	// Fetch the device context's DPI.
	fDpi = (float)::GetDeviceCaps(deviceContextHandle, LOGPIXELSY);
	if (fDpi <= 0)
	{
		fDpi = 96.0f;
	}

	// Convert the given LOGFONT's font size from logical units to points.
	double fontSize = (double)logFont.lfHeight;
	if (logFont.lfHeight > 0)
	{
		// A positive LOGFONT size has line spacing included in the height. Strip it off.
		TEXTMETRICW textMetrics{};
		BOOL wasSuccessful = ::GetTextMetricsW(deviceContextHandle, &textMetrics);
		if (!wasSuccessful)
		{
			return false;
		}
		fontSize = (double)(logFont.lfHeight - textMetrics.tmInternalLeading);
	}
	else if (logFont.lfHeight < 0)
	{
		// A negative font size has no line spacing included. Make it positive.
		fontSize *= -1.0;
	}
	fontSize *= 72.0 / (double)fDpi;

	// Store the font size in points.
	fSize = (float)fontSize;
	fUnit = Gdiplus::UnitPoint;
	return true;
}

float FontSizeConverter::GetSize() const
{
	return fSize;
}

Gdiplus::Unit FontSizeConverter::GetUnit() const
{
	return fUnit;
}

void FontSizeConverter::ConvertTo(Gdiplus::Unit unit)
{
	// No conversion is necessary if this object's units match the given units. (This is an optimization.)
	if (unit == fUnit)
	{
		return;
	}

	// Make sure that our stored DPI value is valid.
	if (fDpi <= 0)
	{
		fDpi = 96.0f;
	}

	// Normalize font size to pixels.
	double fontSize = (double)fSize;
	switch (fUnit)
	{
		case Gdiplus::UnitPoint:
			fontSize *= (double)fDpi / 72.0;
			break;
		case Gdiplus::UnitInch:
			fontSize *= (double)fDpi;
			break;
		case Gdiplus::UnitMillimeter:
			fontSize *= (double)fDpi / 25.4;
			break;
		case Gdiplus::UnitDocument:
			fontSize *= (double)fDpi / 300.0;
			break;
		case Gdiplus::UnitWorld:
		case Gdiplus::UnitDisplay:
		case Gdiplus::UnitPixel:
		default:
			break;
	}

	// Convert from pixels to the given units.
	switch (unit)
	{
		case Gdiplus::UnitPoint:
			fontSize *= 72.0 / (double)fDpi;
			break;
		case Gdiplus::UnitInch:
			fontSize /= (double)fDpi;
			break;
		case Gdiplus::UnitMillimeter:
			fontSize *= 25.4 / (double)fDpi;
			break;
		case Gdiplus::UnitDocument:
			fontSize *= 300.0 / (double)fDpi;
			break;
		case Gdiplus::UnitWorld:
		case Gdiplus::UnitDisplay:
		case Gdiplus::UnitPixel:
			break;
	}

	// Store the converted units.
	fSize = (float)fontSize;
	fUnit = unit;
}

#pragma endregion

} }	// namespace Interop::Graphics
