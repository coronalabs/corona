//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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
