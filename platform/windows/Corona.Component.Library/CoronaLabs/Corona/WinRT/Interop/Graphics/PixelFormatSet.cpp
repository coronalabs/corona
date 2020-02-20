//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "PixelFormatSet.h"
#include "PixelFormat.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

#pragma region Consructors/Destructors
PixelFormatSet::PixelFormatSet()
{
	fCollection = ref new Platform::Collections::Vector<PixelFormat^>();
}

#pragma endregion


#pragma region Public Methods/Properties
void PixelFormatSet::Add(PixelFormat^ item)
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

void PixelFormatSet::AddRange(Windows::Foundation::Collections::IIterable<PixelFormat^>^ items)
{
	// Validate argument.
	if (!items)
	{
		return;
	}

	// Add all items in the given collection to this collection.
	for (auto&& item : items)
	{
		Add(item);
	}
}

bool PixelFormatSet::Remove(PixelFormat^ item)
{
	// Do not continue if given null.
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

void PixelFormatSet::Clear()
{
	fCollection->Clear();
}

bool PixelFormatSet::Contains(PixelFormat^ item)
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

int PixelFormatSet::Count::get()
{
	return (int)(fCollection->Size);
}

Windows::Foundation::Collections::IIterator<PixelFormat^>^ PixelFormatSet::First()
{
	return fCollection->First();
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
