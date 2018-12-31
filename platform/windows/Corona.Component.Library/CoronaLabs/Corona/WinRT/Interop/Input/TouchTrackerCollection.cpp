// ----------------------------------------------------------------------------
// 
// TouchTrackerCollection.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "TouchTrackerCollection.h"
#include "TouchTracker.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Input {

#pragma region Consructors/Destructors
TouchTrackerCollection::TouchTrackerCollection()
{
	fCollection = ref new Platform::Collections::Vector<TouchTracker^>();
}

#pragma endregion


#pragma region Public Methods/Properties
void TouchTrackerCollection::Add(TouchTracker^ item)
{
	// Do not continue if given null.
	if (!item)
	{
		return;
	}

	// Do not continue if the given item already exists in the collection.
	if (this->Contains(item))
	{
		return;
	}

	// Add the given item to the collection.
	fCollection->Append(item);
}

TouchTracker^ TouchTrackerCollection::GetByPointerId(int id)
{
	for (auto&& item : fCollection)
	{
		if (item->PointerId == id)
		{
			return item;
		}
	}
	return nullptr;
}

bool TouchTrackerCollection::Remove(TouchTracker^ item)
{
	// Validate.
	if (!item)
	{
		return false;
	}

	// Find the given item in the collection.
	unsigned int index = 0;
	bool wasFound = fCollection->IndexOf(item, &index);
	if (!wasFound)
	{
		return false;
	}

	// Remove the item from the collection.
	fCollection->RemoveAt(index);
	return true;
}

void TouchTrackerCollection::Clear()
{
	fCollection->Clear();
}

bool TouchTrackerCollection::Contains(TouchTracker^ item)
{
	// Do not continue if given null.
	if (!item)
	{
		return false;
	}

	// Determine if the given item exists in the collection.
	unsigned int index = 0;
	return fCollection->IndexOf(item, &index);
}

bool TouchTrackerCollection::ContainsPointerId(int id)
{
	return (this->GetByPointerId(id) != nullptr);
}

int TouchTrackerCollection::Count::get()
{
	return (int)(fCollection->Size);
}

void TouchTrackerCollection::ResetAll()
{
	for (auto&& item : fCollection)
	{
		item->Reset();
	}
}

Windows::Foundation::Collections::IIterator<TouchTracker^>^ TouchTrackerCollection::First()
{
	return fCollection->First();
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Input
