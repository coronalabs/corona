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

#include "Rtt_PreferenceCollection.h"
#include "Rtt_Preference.h"
#include "Rtt_PreferenceValue.h"


namespace Rtt
{

// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

PreferenceCollection::PreferenceCollection()
{
}

PreferenceCollection::PreferenceCollection(const PreferenceCollection& collection)
{
	CopyFrom(collection);
}

PreferenceCollection::~PreferenceCollection()
{
	// Delete preference objects allocated by this collection.
	Clear();
}


// ----------------------------------------------------------------------------
// Public Methods
// ----------------------------------------------------------------------------

bool PreferenceCollection::Add(const char* keyName, const PreferenceValue& value)
{
	return Add(Preference(keyName, value));
}

bool PreferenceCollection::Add(const Rtt::SharedConstStdStringPtr& keyName, const PreferenceValue& value)
{
	return Add(Preference(keyName, value));
}

bool PreferenceCollection::Add(const Preference& preference)
{
	// Do not continue if the given preference's key already exists in the collection.
	if (ContainsKey(preference.GetKeyName()))
	{
		return false;
	}

	// Create a copy of the given preference.
	Preference* clonedPreferencePointer = new Preference(preference);
	if (!clonedPreferencePointer)
	{
		return false;
	}

	// Add our copy of the preference to the collection.
	fList.push_back(clonedPreferencePointer);
	fMap.insert(std::pair<const char*, Preference*>(clonedPreferencePointer->GetKeyName(), clonedPreferencePointer));
	return true;
}

bool PreferenceCollection::RemoveByIndex(int index)
{
	// Validate argument.
	if ((index < 0) || (index >= (int)fList.size()))
	{
		return false;
	}

	// Remove the indexed preference from the collection.
	PreferenceCollection::ListType::iterator listIterator = fList.begin() + index;
	Preference* preferencePointer = *listIterator;
	if (preferencePointer)
	{
		PreferenceCollection::MapType::iterator mapIterator = fMap.find(preferencePointer->GetKeyName());
		if (mapIterator != fMap.end())
		{
			fMap.erase(mapIterator);
		}
		delete preferencePointer;
	}
	fList.erase(listIterator);
	return true;
}

bool PreferenceCollection::RemoveByKey(const char* keyName)
{
	// Fetch the preference from the map by its unique key name.
	MapType::iterator mapIterator = fMap.find(keyName);
	if (fMap.end() == mapIterator)
	{
		return false;
	}
	Preference* preferencePointer = mapIterator->second;

	// Remove the preference from the list collection.
	for (ListType::iterator listIterator = fList.begin(); listIterator != fList.end(); listIterator++)
	{
		if (preferencePointer == *listIterator)
		{
			fList.erase(listIterator);
			break;
		}
	}

	// Remove the preference from the map collection.
	fMap.erase(mapIterator);

	// Delete the preference.
	delete preferencePointer;
	return true;
}

void PreferenceCollection::Clear()
{
	for (ListType::iterator iterator = fList.begin(); iterator != fList.end(); iterator++)
	{
		delete *iterator;
	}
	fList.clear();
	fMap.clear();
}

int PreferenceCollection::GetCount() const
{
	return (int)fList.size();
}

Preference* PreferenceCollection::GetByIndex(int index) const
{
	// Validate argument.
	if ((index < 0) || (index >= (int)fList.size()))
	{
		return NULL;
	}

	// Fetch the preference by its zero based index.
	PreferenceCollection::ListType::const_iterator iterator = fList.begin() + index;
	return *iterator;
}

Preference* PreferenceCollection::GetByKey(const char* keyName) const
{
	PreferenceCollection::MapType::const_iterator iterator = fMap.find(keyName);
	if (fMap.end() == iterator)
	{
		return NULL;
	}
	return iterator->second;
}

bool PreferenceCollection::ContainsKey(const char* keyName) const
{
	return (GetByKey(keyName) != NULL);
}

void PreferenceCollection::CopyFrom(const PreferenceCollection& collection)
{
	// Do not continue if given an empty collection.
	const int kCount = collection.GetCount();
	if (kCount <= 0)
	{
		return;
	}

	// For best performance, increase the size of our internal collections to just-fit the ones to add.
	// Note: To keep things simple, we'll only do this if this collection is empty.
	//       Otherwise we would have to calculate how many of the given preferences already exist in this collection.
	if (GetCount() <= 0)
	{
		fList.reserve((ListType::size_type)kCount);
#ifdef Rtt_PREFERENCE_COLLECTION_USES_UNORDERED_MAP
		fMap.reserve((MapType::size_type)kCount);
#endif
	}

	// Copy the preferences from the given collection to this collection.
	for (int index = 0; index <= kCount; index++)
	{
		Preference* sourcePreferencePointer = collection.GetByIndex(index);
		if (sourcePreferencePointer)
		{
			Preference* targetPreferencePointer = GetByKey(sourcePreferencePointer->GetKeyName());
			if (targetPreferencePointer)
			{
				targetPreferencePointer->SetValue(sourcePreferencePointer->GetValue());
			}
			else
			{
				Add(*sourcePreferencePointer);
			}
		}
	}
}

void PreferenceCollection::operator=(const PreferenceCollection& collection)
{
	Clear();
	CopyFrom(collection);
}


// ----------------------------------------------------------------------------
// Custom STL Hashing/Comparing Functions 
// * Allows us to use "const char*" as a key in an STL map.
// * Hashes/compares by string instead of by pointer.
// * Faster than using "std::string" because it avoid a string copy.
// ----------------------------------------------------------------------------

#ifdef Rtt_PREFERENCE_COLLECTION_USES_UNORDERED_MAP
size_t PreferenceCollection::StlStringPointerHashProvider::operator()(const char* stringPointer) const
{
	static const size_t kHashValueBitCount = sizeof(size_t) * 8;
	size_t hashValue = 0;
	if (stringPointer)
	{
		// Hash all of the characters in the string, excluding the null termination character.
		// Note: Null and empty strings will have the same hash value.
		for (; *stringPointer != '\0'; stringPointer++)
		{
			// Rotate the hash's bits left by one character to ensure all bytes in the hash are used.
			hashValue = (hashValue << 8) | (hashValue >> (kHashValueBitCount - 8));

			// XOR the next character in the string to the hash's first byte.
			hashValue ^= (size_t)(*stringPointer);
		}
	}
	return hashValue;
}
#endif

#ifdef Rtt_PREFERENCE_COLLECTION_USES_UNORDERED_MAP
bool PreferenceCollection::StlStringPointerEqualityComparer::operator()(
	const char* string1, const char* string2) const
{
	// If the string pointers match or they are both null, then they are equal.
	if (string1 == string2)
	{
		return true;
	}

	// If one of the string pointers are null and the other is not, then they are not equal.
	// Note: Both pointers can't be null at this point due to the above check.
	if (!string1 || !string2)
	{
		return false;
	}

	// Check if all characters match.
	return (strcmp(string1, string2) == 0);
}
#endif

#ifndef Rtt_PREFERENCE_COLLECTION_USES_UNORDERED_MAP
bool PreferenceCollection::StlMapStringPointerIsLessThanComparer::operator()(
	const char* string1, const char* string2) const
{
	// Do null pointer checks first.
	if (!string1 && !string2)
	{
		// Not less-than because both pointer are null, meaning equal.
		return false;
	}
	else if (string1 && !string2)
	{
		// Not less-than because string1 is not null and string2 is null.
		return false;
	}
	else if (!string1 && string2)
	{
		// Is less-than because string1 is null and string2 is not.
		return true;
	}

	// If the pointers match, then they are equal (not less than).
	if (string1 == string2)
	{
		return false;
	}

	// Compare strings. (This is a binary comparison. It is not locale aware.)
	return (strcmp(string1, string2) < 0);
}
#endif

}	// namespace Rtt
