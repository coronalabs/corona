// ----------------------------------------------------------------------------
// 
// TouchPhase.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "TouchPhase.h"
#include "CoronaLabs\WinRT\NativeStringServices.h"
#pragma warning(disable: Rtt_WIN_XML_COMMENT_COMPILER_WARNING_CODE)
#	include "Core\Rtt_String.h"
#pragma warning(default: Rtt_WIN_XML_COMMENT_COMPILER_WARNING_CODE)


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Input {

#pragma region Pre-allocated TouchPhase Objects
const TouchPhase^ TouchPhase::kBegan = ref new TouchPhase(Rtt::TouchEvent::kBegan);

const TouchPhase^ TouchPhase::kMoved = ref new TouchPhase(Rtt::TouchEvent::kMoved);

const TouchPhase^ TouchPhase::kEnded = ref new TouchPhase(Rtt::TouchEvent::kEnded);

const TouchPhase^ TouchPhase::kCanceled = ref new TouchPhase(Rtt::TouchEvent::kCancelled);

#pragma endregion


#pragma region Consructors/Destructors
TouchPhase::TouchPhase(Rtt::TouchEvent::Phase coronaIntegerId)
:	fCoronaTouchEventPhase(coronaIntegerId)
{
	// Fetch Corona's string ID for this phase and store it in UTF-16 form.
	const char* utf8StringId = Rtt::TouchEvent::StringForPhase(fCoronaTouchEventPhase);
	fCoronaStringId = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(utf8StringId);

	// Add this instance to the collection.
	// Used to fetch a pre-existing instance via this class' static From() functions.
	TouchPhase::MutableCollection->Append(this);
}

#pragma endregion


#pragma region Public Functions/Properties
TouchPhase^ TouchPhase::Began::get()
{
	return const_cast<TouchPhase^>(kBegan);
}

TouchPhase^ TouchPhase::Moved::get()
{
	return const_cast<TouchPhase^>(kMoved);
}

TouchPhase^ TouchPhase::Ended::get()
{
	return const_cast<TouchPhase^>(kEnded);
}

TouchPhase^ TouchPhase::Canceled::get()
{
	return const_cast<TouchPhase^>(kCanceled);
}

int TouchPhase::CoronaIntegerId::get()
{
	return (int)fCoronaTouchEventPhase;
}

Platform::String^ TouchPhase::CoronaStringId::get()
{
	return fCoronaStringId;
}

Platform::String^ TouchPhase::ToString()
{
	return fCoronaStringId;
}

Windows::Foundation::Collections::IIterable<TouchPhase^>^ TouchPhase::Collection::get()
{
	return TouchPhase::MutableCollection->GetView();
}

TouchPhase^ TouchPhase::FromCoronaStringId(Platform::String^ stringId)
{
	if (stringId && (stringId->Length() > 0))
	{
		for (auto&& item : TouchPhase::MutableCollection)
		{
			if (item->fCoronaStringId->Equals(stringId))
			{
				return item;
			}
		}
	}
	return nullptr;
}

TouchPhase^ TouchPhase::FromCoronaIntegerId(int value)
{
	for (auto&& item : TouchPhase::MutableCollection)
	{
		if (value == (int)(item->fCoronaTouchEventPhase))
		{
			return item;
		}
	}
	return nullptr;
}

#pragma endregion


#pragma region Internal Functions/Properties
Rtt::TouchEvent::Phase TouchPhase::CoronaTouchEventPhase::get()
{
	return fCoronaTouchEventPhase;
}

TouchPhase^ TouchPhase::FromCoronaStringId(const char *stringId)
{
	if (Rtt_StringIsEmpty(stringId) == false)
	{
		for (auto&& item : TouchPhase::MutableCollection)
		{
			if (Rtt_StringCompare(stringId, Rtt::TouchEvent::StringForPhase(item->fCoronaTouchEventPhase)) == 0)
			{
				return item;
			}
		}
	}
	return nullptr;
}

TouchPhase^ TouchPhase::From(Rtt::TouchEvent::Phase value)
{
	return FromCoronaIntegerId((int)value);
}

#pragma endregion


#pragma region Private Functions/Properties
Platform::Collections::Vector<TouchPhase^>^ TouchPhase::MutableCollection::get()
{
	static Platform::Collections::Vector<TouchPhase^> sCollection;
	return %sCollection;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Input
