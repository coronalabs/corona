//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIComponentCollection.h"
#include "UIComponent.h"


namespace Interop { namespace UI {

#pragma region Constructors/Destructors
UIComponentCollection::UIComponentCollection()
{
}

UIComponentCollection::UIComponentCollection(const UIComponentCollection& collection)
{
	Add(collection);
}

UIComponentCollection::~UIComponentCollection()
{
}

#pragma endregion


#pragma region Public Methods
void UIComponentCollection::Add(UIComponent* componentPointer)
{
	// Validate.
	if (!componentPointer)
	{
		return;
	}

	// Do not continue if the given object was already added to the collection.
	if (Contains(componentPointer))
	{
		return;
	}

	// Add the given object to the collection.
	fCollection.push_back(componentPointer);
}

void UIComponentCollection::Add(const UIComponentCollection& collection)
{
	for (auto&& componentPointer : collection.fCollection)
	{
		Add(componentPointer);
	}
}

bool UIComponentCollection::Remove(UIComponent* componentPointer)
{
	if (componentPointer)
	{
		for (auto iterator = fCollection.begin(); iterator != fCollection.end(); iterator++)
		{
			if (componentPointer == *iterator)
			{
				fCollection.erase(iterator);
				return true;
			}
		}
	}
	return false;
}

bool UIComponentCollection::RemoveByIndex(int index)
{
	if ((index >= 0) && (index < (int)fCollection.size()))
	{
		for (auto iterator = fCollection.begin(); iterator != fCollection.end(); iterator++)
		{
			if (0 == index)
			{
				fCollection.erase(iterator);
				return true;
			}
			index--;
		}
	}
	return false;
}

void UIComponentCollection::Clear()
{
	fCollection.clear();
}

int UIComponentCollection::GetCount() const
{
	return (int)fCollection.size();
}

UIComponent* UIComponentCollection::GetByIndex(int index) const
{
	if ((index >= 0) && (index < (int)fCollection.size()))
	{
		for (auto iterator = fCollection.begin(); iterator != fCollection.end(); iterator++)
		{
			if (0 == index)
			{
				return *iterator;
			}
			index--;
		}
	}
	return nullptr;
}

bool UIComponentCollection::Contains(UIComponent* componentPointer) const
{
	if (componentPointer)
	{
		for (auto iterator = fCollection.begin(); iterator != fCollection.end(); iterator++)
		{
			if (componentPointer == *iterator)
			{
				return true;
			}
		}
	}
	return false;
}

UIComponentCollection& UIComponentCollection::operator=(const UIComponentCollection& collection)
{
	Clear();
	Add(collection);
	return *this;
}

#pragma endregion

} }	// namespace Interop::UI
