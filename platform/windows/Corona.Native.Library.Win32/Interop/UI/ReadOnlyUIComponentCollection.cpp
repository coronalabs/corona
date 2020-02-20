//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ReadOnlyUIComponentCollection.h"
#include "UIComponentCollection.h"
#include "UIComponent.h"


namespace Interop { namespace UI {

#pragma region Constructors/Destructors
ReadOnlyUIComponentCollection::ReadOnlyUIComponentCollection()
:	fCollectionPointer(nullptr)
{
}

ReadOnlyUIComponentCollection::ReadOnlyUIComponentCollection(const UIComponentCollection* collectionPointer)
:	fCollectionPointer(collectionPointer)
{
}

ReadOnlyUIComponentCollection::ReadOnlyUIComponentCollection(const UIComponentCollection& collection)
:	fCollectionPointer(&collection)
{
}

ReadOnlyUIComponentCollection::~ReadOnlyUIComponentCollection()
{
}

#pragma endregion


#pragma region Public Methods
int ReadOnlyUIComponentCollection::GetCount() const
{
	if (fCollectionPointer)
	{
		return fCollectionPointer->GetCount();
	}
	return 0;
}

UIComponent* ReadOnlyUIComponentCollection::GetByIndex(int index) const
{
	if (fCollectionPointer)
	{
		return fCollectionPointer->GetByIndex(index);
	}
	return nullptr;
}

bool ReadOnlyUIComponentCollection::Contains(UIComponent* componentPointer) const
{
	if (fCollectionPointer)
	{
		return fCollectionPointer->Contains(componentPointer);
	}
	return false;
}

#pragma endregion

} }	// namespace Interop::UI
