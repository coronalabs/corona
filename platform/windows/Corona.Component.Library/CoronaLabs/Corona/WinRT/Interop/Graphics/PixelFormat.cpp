// ----------------------------------------------------------------------------
// 
// PixelFormat.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "PixelFormat.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

#pragma region Pre-allocated PixelFormat Objects
/// <summary>Indicates that the pixel format is undefined.</summary>
const PixelFormat^ PixelFormat::kUndefined = ref new PixelFormat(
									Rtt::PlatformBitmap::kUndefined, ref new Platform::String(L"Undefined"));

/// <summary>Indicates that the pixel format is 8-bit grayscale.</summary>
const PixelFormat^ PixelFormat::kGrayscale = ref new PixelFormat(
									Rtt::PlatformBitmap::kMask, ref new Platform::String(L"Grayscale"));

/// <summary>Indicates that the the pixel format is 32-bit color with an alpha channel.</summary>
const PixelFormat^ PixelFormat::kRGBA = ref new PixelFormat(
									Rtt::PlatformBitmap::kRGBA, ref new Platform::String(L"RGBA"));

/// <summary>Indicates that the the pixel format is 32-bit color with an alpha channel.</summary>
const PixelFormat^ PixelFormat::kBGRA = ref new PixelFormat(
									Rtt::PlatformBitmap::kBGRA, ref new Platform::String(L"BGRA"));

#pragma endregion


#pragma region Consructors/Destructors
PixelFormat::PixelFormat(Rtt::PlatformBitmap::Format coronaFormat, Platform::String^ invariantName)
:	fCoronaBitmapFormat(coronaFormat),
	fInvariantName(invariantName),
	fByteCount(0)
{
	if (fCoronaBitmapFormat != Rtt::PlatformBitmap::kUndefined)
	{
		fByteCount = Rtt::PlatformBitmap::BytesPerPixel(fCoronaBitmapFormat);
	}

	PixelFormat::MutableCollection->Append(this);
}

#pragma endregion


#pragma region Public Methods/Properties
PixelFormat^ PixelFormat::Undefined::get()
{
	return const_cast<PixelFormat^>(kUndefined);
}

PixelFormat^ PixelFormat::Grayscale::get()
{
	return const_cast<PixelFormat^>(kGrayscale);
}

PixelFormat^ PixelFormat::RGBA::get()
{
	return const_cast<PixelFormat^>(kRGBA);
}

PixelFormat^ PixelFormat::BGRA::get()
{
	return const_cast<PixelFormat^>(kBGRA);
}

int PixelFormat::ByteCount::get()
{
	return fByteCount;
}

int PixelFormat::ToCoronaIntegerId()
{
	return (int)fCoronaBitmapFormat;
}

Platform::String^ PixelFormat::ToString()
{
	return fInvariantName;
}

Windows::Foundation::Collections::IIterable<PixelFormat^>^ PixelFormat::Collection::get()
{
	return PixelFormat::MutableCollection->GetView();
}

PixelFormat^ PixelFormat::FromCoronaIntegerId(int id)
{
	return PixelFormat::From((Rtt::PlatformBitmap::Format)id);
}

#pragma endregion


#pragma region Internal Methods/Properties
Rtt::PlatformBitmap::Format PixelFormat::CoronaBitmapFormat::get()
{
	return fCoronaBitmapFormat;
}

PixelFormat^ PixelFormat::From(Rtt::PlatformBitmap::Format format)
{
	for (auto&& item : PixelFormat::MutableCollection)
	{
		if (format == item->fCoronaBitmapFormat)
		{
			return item;
		}
	}
	return nullptr;
}

#pragma endregion


#pragma region Private Methods/Properties
Platform::Collections::Vector<PixelFormat^>^ PixelFormat::MutableCollection::get()
{
	static Platform::Collections::Vector<PixelFormat^> sCollection;
	return %sCollection;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
