// ----------------------------------------------------------------------------
// 
// PageOrientation.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "PageOrientation.h"
#include "CoronaLabs\Corona\WinRT\Interop\RelativeOrientation2D.h"
#include "CoronaLabs\WinRT\NativeStringServices.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace UI {

#pragma region Pre-allocated PageOrientation Objects
const PageOrientation^ PageOrientation::kUnknown = ref new PageOrientation(Rtt::DeviceOrientation::kUnknown);

const PageOrientation^ PageOrientation::kPortraitUpright = ref new PageOrientation(Rtt::DeviceOrientation::kUpright);

const PageOrientation^ PageOrientation::kPortraitUpsideDown = ref new PageOrientation(Rtt::DeviceOrientation::kUpsideDown);

const PageOrientation^ PageOrientation::kLandscapeRight = ref new PageOrientation(Rtt::DeviceOrientation::kSidewaysRight);

const PageOrientation^ PageOrientation::kLandscapeLeft = ref new PageOrientation(Rtt::DeviceOrientation::kSidewaysLeft);

#pragma endregion


#pragma region Consructors/Destructors
PageOrientation::PageOrientation(Rtt::DeviceOrientation::Type coronaDeviceOrientation)
:	fCoronaDeviceOrientation(coronaDeviceOrientation)
{
	// Add this instance to the collection.
	// Used to fetch a pre-existing instance via this class' static From() functions.
	PageOrientation::MutableCollection->Append(this);
}

#pragma endregion


#pragma region Public Methods/Properties
bool PageOrientation::IsPortrait::get()
{
	bool isPortrait =
			(Rtt::DeviceOrientation::kUpright == fCoronaDeviceOrientation) ||
			(Rtt::DeviceOrientation::kUpsideDown == fCoronaDeviceOrientation);
	return isPortrait;
}

bool PageOrientation::IsLandscape::get()
{
	bool isLandscape =
			(Rtt::DeviceOrientation::kSidewaysRight == fCoronaDeviceOrientation) ||
			(Rtt::DeviceOrientation::kSidewaysLeft == fCoronaDeviceOrientation);
	return isLandscape;
}

PageOrientation^ PageOrientation::Unknown::get()
{
	return const_cast<PageOrientation^>(kUnknown);
}

PageOrientation^ PageOrientation::PortraitUpright::get()
{
	return const_cast<PageOrientation^>(kPortraitUpright);
}

PageOrientation^ PageOrientation::PortraitUpsideDown::get()
{
	return const_cast<PageOrientation^>(kPortraitUpsideDown);
}

PageOrientation^ PageOrientation::LandscapeRight::get()
{
	return const_cast<PageOrientation^>(kLandscapeRight);
}

PageOrientation^ PageOrientation::LandscapeLeft::get()
{
	return const_cast<PageOrientation^>(kLandscapeLeft);
}

RelativeOrientation2D^ PageOrientation::ToOrientationRelativeTo(PageOrientation^ orientation)
{
	// Return the "Unknown" orientation if this object or the given object is invalid.
	if ((nullptr == orientation) || (PageOrientation::Unknown == orientation))
	{
		return RelativeOrientation2D::Unknown;
	}

	// Return the relative orientation.
	auto relativeDeviceOrientation = Rtt::DeviceOrientation::GetRelativeOrientation(
				orientation->fCoronaDeviceOrientation, fCoronaDeviceOrientation);
	return RelativeOrientation2D::From(relativeDeviceOrientation);
}

Platform::String^ PageOrientation::ToString()
{
	auto utf8String = Rtt::DeviceOrientation::StringForType(fCoronaDeviceOrientation);
	auto utf16String = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(utf8String);
	return utf16String;
}

Windows::Foundation::Collections::IIterable<PageOrientation^>^ PageOrientation::Collection::get()
{
	return PageOrientation::MutableCollection->GetView();
}

#pragma endregion


#pragma region Internal Methods/Properties
Rtt::DeviceOrientation::Type PageOrientation::CoronaDeviceOrientation::get()
{
	return fCoronaDeviceOrientation;
}

#pragma endregion


#pragma region Private Methods/Properties
Platform::Collections::Vector<PageOrientation^>^ PageOrientation::MutableCollection::get()
{
	static Platform::Collections::Vector<PageOrientation^> sCollection;
	return %sCollection;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::UI
