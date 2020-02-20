//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "BitmapSettings.h"
#include "PixelFormat.h"
#include "CoronaLabs\Corona\WinRT\Interop\RelativeOrientation2D.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

#pragma region Consructors/Destructors
BitmapSettings::BitmapSettings()
:	fPixelWidth(0),
	fPixelHeight(0),
	fPixelFormat(CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::Undefined),
	fPremultipliedAlphaApplied(false),
	fOrientation(RelativeOrientation2D::Upright),
	fScale(1.0f)
{
}

BitmapSettings::BitmapSettings(BitmapSettings^ settings)
{
	// Validate.
	if (!settings)
	{
		throw ref new Platform::NullReferenceException();
	}

	// Copy the given settings to this object.
	fPixelWidth = settings->fPixelWidth;
	fPixelHeight = settings->fPixelHeight;
	fPixelFormat = settings->fPixelFormat;
	fPremultipliedAlphaApplied = settings->fPremultipliedAlphaApplied;
	fOrientation = settings->fOrientation;
	fScale = settings->fScale;
}

#pragma endregion


#pragma region Public Properties
int BitmapSettings::PixelWidth::get()
{
	return fPixelWidth;
}

void BitmapSettings::PixelWidth::set(int value)
{
	if (value < 0)
	{
		value = 0;
	}
	fPixelWidth = value;
}

int BitmapSettings::PixelHeight::get()
{
	return fPixelHeight;
}

void BitmapSettings::PixelHeight::set(int value)
{
	if (value < 0)
	{
		value = 0;
	}
	fPixelHeight = value;
}

CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat^ BitmapSettings::PixelFormat::get()
{
	return fPixelFormat;
}

void BitmapSettings::PixelFormat::set(CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat^ value)
{
	// Set the pixel format to undefined if given a null reference.
	if (!value)
	{
		value = CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::Undefined;
	}

	// Store the given format.
	fPixelFormat = value;
}

bool BitmapSettings::PremultipliedAlphaApplied::get()
{
	return fPremultipliedAlphaApplied;
}

void BitmapSettings::PremultipliedAlphaApplied::set(bool value)
{
	fPremultipliedAlphaApplied = value;
}

RelativeOrientation2D^ BitmapSettings::Orientation::get()
{
	return fOrientation;
}

void BitmapSettings::Orientation::set(RelativeOrientation2D^ value)
{
	// Set to upright if given a null or unknown orientation.
	if (!value || (RelativeOrientation2D::Unknown == value))
	{
		value = RelativeOrientation2D::Upright;
	}

	// Store the given orientation.
	fOrientation = value;
}

float BitmapSettings::Scale::get()
{
	return fScale;
}

void BitmapSettings::Scale::set(float value)
{
	if (value <= 0.0f)
	{
		throw ref new Platform::InvalidArgumentException("value");
	}
	fScale = value;
}

int BitmapSettings::StrideInBytes::get()
{
	// Calculate the number of bytes per row.
	int bytesPerRow = fPixelWidth * fPixelFormat->ByteCount;

	// Add additional bytes to the row so that it will meet the required byte packing alignment.
	const int kBytePackingAlignment = 4;
	if (fPixelFormat->ByteCount != kBytePackingAlignment)
	{
		int delta = bytesPerRow % kBytePackingAlignment;
		if (delta > 0)
		{
			bytesPerRow += kBytePackingAlignment - delta;
		}
	}

	// Return the total number of bytes per row, including the padded bytes.
	return bytesPerRow;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
