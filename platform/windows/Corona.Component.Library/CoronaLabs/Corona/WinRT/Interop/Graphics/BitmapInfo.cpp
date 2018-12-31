// ----------------------------------------------------------------------------
// 
// BitmapInfo.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "BitmapInfo.h"
#include "BitmapSettings.h"
#include "PixelFormat.h"
#include "CoronaLabs\Corona\WinRT\Interop\RelativeOrientation2D.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

#pragma region Consructors/Destructors
BitmapInfo::BitmapInfo()
{
	fSettings = ref new BitmapSettings();
}

BitmapInfo::BitmapInfo(BitmapSettings^ settings)
{
	// Validate.
	if (!settings)
	{
		throw ref new Platform::NullReferenceException();
	}

	// Copy the given settings.
	fSettings = ref new BitmapSettings(settings);
}

#pragma endregion


#pragma region Public Properties
int BitmapInfo::PixelWidth::get()
{
	return fSettings->PixelWidth;
}

int BitmapInfo::PixelHeight::get()
{
	return fSettings->PixelHeight;
}

CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat^ BitmapInfo::PixelFormat::get()
{
	return fSettings->PixelFormat;
}

bool BitmapInfo::PremultipliedAlphaApplied::get()
{
	return fSettings->PremultipliedAlphaApplied;
}

RelativeOrientation2D^ BitmapInfo::Orientation::get()
{
	return fSettings->Orientation;
}

float BitmapInfo::Scale::get()
{
	return fSettings->Scale;
}

int BitmapInfo::StrideInBytes::get()
{
	return fSettings->StrideInBytes;
}

#pragma endregion


#pragma region Public Static Properties
BitmapInfo^ BitmapInfo::Empty::get()
{
	static BitmapInfo sEmptyInfo;
	return %sEmptyInfo;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
