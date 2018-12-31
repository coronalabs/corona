// ----------------------------------------------------------------------------
// 
// RelativeOrientation2D.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "RelativeOrientation2D.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop {

#pragma region Pre-allocated RelativeOrientation2D Objects
/// <summary>Indicates that the orientation is unknown.</summary>
const RelativeOrientation2D^ RelativeOrientation2D::kUnknown = ref new RelativeOrientation2D(
				Rtt::DeviceOrientation::kUnknown, Rtt::PlatformBitmap::kUp, "Unknown", 0);

/// <summary>Indicates that the orientation is upright.</summary>
const RelativeOrientation2D^ RelativeOrientation2D::kUpright = ref new RelativeOrientation2D(
				Rtt::DeviceOrientation::kUpright , Rtt::PlatformBitmap::kUp, "Upright", 0);

/// <summary>Indicates that the orientation is upside down.</summary>
const RelativeOrientation2D^ RelativeOrientation2D::kUpsideDown = ref new RelativeOrientation2D(
				Rtt::DeviceOrientation::kUpsideDown, Rtt::PlatformBitmap::kDown, "UpsideDown", 180);

/// <summary>Indicates that the orientation is rotated 90 degrees counter-clockwise.</summary>
const RelativeOrientation2D^ RelativeOrientation2D::kSidewaysRight = ref new RelativeOrientation2D(
				Rtt::DeviceOrientation::kSidewaysRight, Rtt::PlatformBitmap::kRight, "SidewaysRight", 90);

/// <summary>Indicates that the orientation is rotated 90 degrees clockwise.</summary>
const RelativeOrientation2D^ RelativeOrientation2D::kSidewaysLeft = ref new RelativeOrientation2D(
				Rtt::DeviceOrientation::kSidewaysLeft, Rtt::PlatformBitmap::kLeft, "SidewaysLeft", 270);

#pragma endregion


#pragma region Consructors/Destructors
RelativeOrientation2D::RelativeOrientation2D(
	Rtt::DeviceOrientation::Type coronaDeviceOrientation,
	Rtt::PlatformBitmap::Orientation coronaBitmapOrientation,
	Platform::String^ invariantName,
	int degreesClockwise)
:	fCoronaDeviceOrientation(coronaDeviceOrientation),
	fCoronaBitmapOrientation(coronaBitmapOrientation),
	fInvariantName(invariantName),
	fDegreesClockwise(degreesClockwise)
{
	// Validate.
	if ((degreesClockwise < 0) || (degreesClockwise >= 360))
	{
		throw ref new Platform::InvalidArgumentException("degreesClockwise");
	}

	// Add this instance to the collection.
	// Used to fetch a pre-existing instance via this class' static From() functions.
	RelativeOrientation2D::MutableCollection->Append(this);
}

#pragma endregion


#pragma region Public Methods/Properties
RelativeOrientation2D^ RelativeOrientation2D::Unknown::get()
{
	return const_cast<RelativeOrientation2D^>(kUnknown);
}

RelativeOrientation2D^ RelativeOrientation2D::Upright::get()
{
	return const_cast<RelativeOrientation2D^>(kUpright);
}

RelativeOrientation2D^ RelativeOrientation2D::UpsideDown::get()
{
	return const_cast<RelativeOrientation2D^>(kUpsideDown);
}

RelativeOrientation2D^ RelativeOrientation2D::SidewaysRight::get()
{
	return const_cast<RelativeOrientation2D^>(kSidewaysRight);
}

RelativeOrientation2D^ RelativeOrientation2D::SidewaysLeft::get()
{
	return const_cast<RelativeOrientation2D^>(kSidewaysLeft);
}

int RelativeOrientation2D::DegreesClockwise::get()
{
	return fDegreesClockwise;
}

int RelativeOrientation2D::DegreesCounterClockwise::get()
{
	return 360 - fDegreesClockwise;
}

bool RelativeOrientation2D::IsSideways::get()
{
	bool isSideways =
			(Rtt::DeviceOrientation::kSidewaysRight == fCoronaDeviceOrientation) ||
			(Rtt::DeviceOrientation::kSidewaysLeft == fCoronaDeviceOrientation);
	return isSideways;
}

Platform::String^ RelativeOrientation2D::ToString()
{
	return fInvariantName;
}

Windows::Foundation::Collections::IIterable<RelativeOrientation2D^>^ RelativeOrientation2D::Collection::get()
{
	return RelativeOrientation2D::MutableCollection->GetView();
}

#pragma endregion


#pragma region Internal Methods/Properties
Rtt::DeviceOrientation::Type RelativeOrientation2D::CoronaDeviceOrientation::get()
{
	return fCoronaDeviceOrientation;
}

Rtt::PlatformBitmap::Orientation RelativeOrientation2D::CoronaBitmapOrientation::get()
{
	return fCoronaBitmapOrientation;
}

RelativeOrientation2D^ RelativeOrientation2D::From(Rtt::DeviceOrientation::Type value)
{
	for (auto&& item : RelativeOrientation2D::MutableCollection)
	{
		if (value == item->fCoronaDeviceOrientation)
		{
			return item;
		}
	}
	return nullptr;
}

RelativeOrientation2D^ RelativeOrientation2D::From(Rtt::PlatformBitmap::Orientation value)
{
	for (auto&& item : RelativeOrientation2D::MutableCollection)
	{
		if (value == item->fCoronaBitmapOrientation)
		{
			return item;
		}
	}
	return nullptr;
}

#pragma endregion


#pragma region Private Methods/Properties
Platform::Collections::Vector<RelativeOrientation2D^>^ RelativeOrientation2D::MutableCollection::get()
{
	static Platform::Collections::Vector<RelativeOrientation2D^> sCollection;
	return %sCollection;
}

#pragma endregion

} } } }	// namespace CoronaLabs::Corona::WinRT::Interop
