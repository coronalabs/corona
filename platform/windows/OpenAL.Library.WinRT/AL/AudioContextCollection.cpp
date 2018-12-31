// ----------------------------------------------------------------------------
// 
// AudioContextCollection.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include <pch.h>
#include "AudioContextCollection.h"
#include "AudioContext.h"


namespace AL {

#pragma region Constructors/Destructors
AudioContextCollection::AudioContextCollection()
{
}

AudioContextCollection::~AudioContextCollection()
{
}

#pragma endregion


#pragma region Public Methods
void AudioContextCollection::Add(AudioContext* contextPointer)
{
	// Validate argument.
	if (nullptr == contextPointer)
	{
		return;
	}

	// Add the given context to the end of the collection.
	fCollection.push_back(contextPointer);
}

bool AudioContextCollection::Remove(AudioContext *contextPointer)
{
	CollectionType::iterator iterator;
	bool isRequestingRepeatSearch;
	bool wasRemoved = false;

	// Do not continue if given null.
	if (nullptr == contextPointer)
	{
		return false;
	}

	// Remove all audio contexts from the collection matching the given memory address.
	do
	{
		isRequestingRepeatSearch = false;
		for (iterator = fCollection.begin(); iterator != fCollection.end(); iterator++)
		{
			if (contextPointer == (*iterator))
			{
				fCollection.erase(iterator);
				wasRemoved = true;
				isRequestingRepeatSearch = true;
				break;
			}
		}
	} while (isRequestingRepeatSearch);

	// Returns true if at least 1 audio context was removed.
	return wasRemoved;
}

void AudioContextCollection::Clear()
{
	fCollection.clear();
}

bool AudioContextCollection::Contains(AudioContext *contextPointer)
{
	// Do not continue if given null.
	if (nullptr == contextPointer)
	{
		return false;
	}

	// Find the specified audio context in the collection.
	for (auto&& nextContextPointer : fCollection)
	{
		if (contextPointer == nextContextPointer)
		{
			// Found it!
			return true;
		}
	}

	// The audio context was not found.
	return false;
}

AudioContext* AudioContextCollection::GetByIndex(int index)
{
	// Validate argument.
	if ((index < 0) || (index >= fCollection.size()))
	{
		return nullptr;
	}

	// Fetch the indexed audio context.
	for (auto&& nextContextPointer : fCollection)
	{
		if (0 == index)
		{
			return nextContextPointer;
		}
		index--;
	}
	return nullptr;
}

int AudioContextCollection::GetCount()
{
	return fCollection.size();
}

#pragma endregion

}	// namespace AL
