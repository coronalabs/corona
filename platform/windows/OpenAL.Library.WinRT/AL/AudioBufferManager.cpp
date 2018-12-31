// ----------------------------------------------------------------------------
// 
// AudioBufferManager.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include <pch.h>
#include "AudioBufferManager.h"
#include "AudioBuffer.h"


namespace AL {

#pragma region Constructors/Destructors
AudioBufferManager::AudioBufferManager()
{
	fNextBufferId = 1;
}

AudioBufferManager::~AudioBufferManager()
{
	DestroyAll();
}

#pragma endregion


#pragma region Public Methods
AudioBuffer* AudioBufferManager::Create()
{
	// Generate a unique integer ID for the new audio buffer.
	while ((AudioBuffer::kInvalidId == fNextBufferId) || (fBufferCollection.find(fNextBufferId) != fBufferCollection.end()))
	{
		fNextBufferId++;
	}
	ALuint newBufferId = fNextBufferId;
	fNextBufferId++;

	// Create the new audio buffer and add it to the collection.
	auto bufferPointer = new AudioBuffer(newBufferId);
	if (nullptr == bufferPointer)
	{
		return nullptr;
	}
	fBufferCollection.insert(CollectionPairType(newBufferId, bufferPointer));

	// Return the new audio buffer.
	return bufferPointer;
}

void AudioBufferManager::DestroyAll()
{
	for (auto&& nextPair : fBufferCollection)
	{
		delete nextPair.second;
	}
	fBufferCollection.clear();
}

bool AudioBufferManager::DestroyById(ALuint id)
{
	// Fetch the audio buffer by its unique ID.
	auto iterator = fBufferCollection.find(id);
	if (iterator == fBufferCollection.end())
	{
		return false;
	}

	// Delete the buffer and remove it from the collection.
	delete (*iterator).second;
	fBufferCollection.erase(iterator);
	return true;
}

bool AudioBufferManager::ContainsId(ALuint id)
{
	return (GetById(id) != nullptr);
}

AudioBuffer* AudioBufferManager::GetById(ALuint id)
{
	auto iterator = fBufferCollection.find(id);
	if (iterator == fBufferCollection.end())
	{
		return nullptr;
	}
	return (*iterator).second;
}

AudioBuffer* AudioBufferManager::GetByIndex(int index)
{
	// Validate argument.
	if ((index < 0) || (index >= fBufferCollection.size()))
	{
		return nullptr;
	}

	// Fetch the indexed buffer.
	for (auto&& nextPair : fBufferCollection)
	{
		if (0 == index)
		{
			return nextPair.second;
		}
		index--;
	}
	return nullptr;
}

int AudioBufferManager::GetCount()
{
	return fBufferCollection.size();
}

#pragma endregion

}	// namespace AL
