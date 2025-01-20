//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "KeyDataCollection.h"


namespace Pthreads {

#pragma region Constructors/Destructors
KeyDataCollection::KeyDataCollection()
{
}

KeyDataCollection::~KeyDataCollection()
{
}

#pragma endregion


#pragma region Public Functions
void KeyDataCollection::Clear()
{
	fCollection.clear();
}

const void* KeyDataCollection::GetData(pthread_key_t key)
{
	const void* data = nullptr;

	auto iterator = fCollection.find(key);
	if (iterator != fCollection.end())
	{
		data = (*iterator).second;
	}
	return data;
}

void KeyDataCollection::SetData(pthread_key_t key, const void* data)
{
	// Do not continue if given null data.
	if (nullptr == data)
	{
		return;
	}

	// Add or replace the value in the table under the given key.
	auto iterator = fCollection.find(key);
	if (iterator != fCollection.end())
	{
		(*iterator).second = data;
	}
	else
	{
		fCollection.insert(CollectionPairType(key, data));
	}
}

#pragma endregion

}	// namespace Pthreads
