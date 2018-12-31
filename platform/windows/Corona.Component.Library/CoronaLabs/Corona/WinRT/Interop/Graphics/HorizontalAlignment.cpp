// ----------------------------------------------------------------------------
// 
// HorizontalAlignment.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "HorizontalAlignment.h"
#include "CoronaLabs\WinRT\NativeStringServices.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

#pragma region Pre-allocated HorizontalAlignment Objects
const HorizontalAlignment^ HorizontalAlignment::kLeft = ref new HorizontalAlignment("left");

const HorizontalAlignment^ HorizontalAlignment::kCenter = ref new HorizontalAlignment("center");

const HorizontalAlignment^ HorizontalAlignment::kRight = ref new HorizontalAlignment("right");

#pragma endregion


#pragma region Consructors/Destructors
HorizontalAlignment::HorizontalAlignment(Platform::String^ coronaStringId)
:	fCoronaStringId(coronaStringId)
{
	// Validate.
	if (!coronaStringId)
	{
		throw ref new Platform::NullReferenceException("coronaStringId");
	}

	// Add this instance to the collection.
	// Used to fetch a pre-existing instance via this class' static From() functions.
	HorizontalAlignment::MutableCollection->Append(this);
}

#pragma endregion


#pragma region Public Methods/Properties
HorizontalAlignment^ HorizontalAlignment::Left::get()
{
	return const_cast<HorizontalAlignment^>(kLeft);
}

HorizontalAlignment^ HorizontalAlignment::Center::get()
{
	return const_cast<HorizontalAlignment^>(kCenter);
}

HorizontalAlignment^ HorizontalAlignment::Right::get()
{
	return const_cast<HorizontalAlignment^>(kRight);
}

Platform::String^ HorizontalAlignment::CoronaStringId::get()
{
	return fCoronaStringId;
}

Platform::String^ HorizontalAlignment::ToString()
{
	return fCoronaStringId;
}

Windows::Foundation::Collections::IIterable<HorizontalAlignment^>^ HorizontalAlignment::Collection::get()
{
	return HorizontalAlignment::MutableCollection->GetView();
}

HorizontalAlignment^ HorizontalAlignment::FromCoronaStringId(Platform::String^ stringId)
{
	if (stringId && (stringId->Length() > 0))
	{
		for (auto&& item : HorizontalAlignment::MutableCollection)
		{
			if (item->fCoronaStringId->Equals(stringId))
			{
				return item;
			}
		}
	}
	return nullptr;
}

#pragma endregion


#pragma region Internal Functions
HorizontalAlignment^ HorizontalAlignment::FromCoronaStringId(const char *stringId)
{
	return FromCoronaStringId(CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(stringId));
}

#pragma endregion


#pragma region Private Methods/Properties
Platform::Collections::Vector<HorizontalAlignment^>^ HorizontalAlignment::MutableCollection::get()
{
	static Platform::Collections::Vector<HorizontalAlignment^> sCollection;
	return %sCollection;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
