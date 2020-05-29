//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "ImageDecoderSettings.h"
#include "PixelFormat.h"
#include "PixelFormatSet.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

#pragma region Consructors/Destructors
ImageDecoderSettings::ImageDecoderSettings()
:	fMaxPixelWidth(0),
	fMaxPixelHeight(0),
	fPremultiplyAlphaEnabled(false),
	fPreferredPixelFormat(PixelFormat::RGBA)
{
	fSupportedPixelFormats = ref new PixelFormatSet();
	fSupportedPixelFormats->Add(fPreferredPixelFormat);
}

#pragma endregion


#pragma region Public Properties/Methods
void ImageDecoderSettings::CopyFrom(ImageDecoderSettings^ settings)
{
	// Validate argument.
	if (!settings)
	{
		return;
	}

	// Copy the given object's settings.
	fMaxPixelWidth = settings->fMaxPixelWidth;
	fMaxPixelHeight = settings->fMaxPixelHeight;
	fPremultiplyAlphaEnabled = settings->fPremultiplyAlphaEnabled;
	fPreferredPixelFormat = settings->fPreferredPixelFormat;
	fSupportedPixelFormats->Clear();
	fSupportedPixelFormats->AddRange(settings->fSupportedPixelFormats);
}

int ImageDecoderSettings::MaxPixelWidth::get()
{
	return fMaxPixelWidth;
}

void ImageDecoderSettings::MaxPixelWidth::set(int value)
{
	if (value < 0)
	{
		value = 0;
	}
	fMaxPixelWidth = value;
}

int ImageDecoderSettings::MaxPixelHeight::get()
{
	return fMaxPixelHeight;
}

void ImageDecoderSettings::MaxPixelHeight::set(int value)
{
	if (value < 0)
	{
		value = 0;
	}
	fMaxPixelHeight = value;
}

bool ImageDecoderSettings::PremultiplyAlphaEnabled::get()
{
	return fPremultiplyAlphaEnabled;
}

void ImageDecoderSettings::PremultiplyAlphaEnabled::set(bool value)
{
	fPremultiplyAlphaEnabled = value;
}

PixelFormat^ ImageDecoderSettings::PreferredPixelFormat::get()
{
	return fPreferredPixelFormat;
}

void ImageDecoderSettings::PreferredPixelFormat::set(PixelFormat^ value)
{
	// Validate argument.
	if (!value)
	{
		throw ref new Platform::NullReferenceException();
	}
	if (PixelFormat::Undefined == value)
	{
		throw ref new Platform::InvalidArgumentException("Pixel format cannot be set to 'Undefined'.");
	}

	// Store the given pixel format.
	fPreferredPixelFormat = value;
}

PixelFormatSet^ ImageDecoderSettings::SupportedPixelFormats::get()
{
	return fSupportedPixelFormats;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
