// ----------------------------------------------------------------------------
// 
// AudioBufferQueue.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include <pch.h>
#include "AudioBufferQueue.h"


namespace AL {

#pragma region Constructors/Destructors
AudioBufferQueue::AudioBufferQueue()
{
}

AudioBufferQueue::~AudioBufferQueue()
{
}

#pragma endregion


#pragma region Public Methods
void AudioBufferQueue::Push(AudioBuffer* bufferPointer)
{
	// Validate argument.
	if (nullptr == bufferPointer)
	{
		return;
	}

	// Push the given buffer to the end of the queue.
	fCollection.push_back(bufferPointer);
}

AudioBuffer* AudioBufferQueue::Pop()
{
	// Return null if the queue is currently empty.
	if (fCollection.empty())
	{
		return nullptr;
	}

	// Pop off and return the buffer at the front of the queue.
	auto bufferPointer = fCollection.front();
	fCollection.pop_front();
	return bufferPointer;
}

bool AudioBufferQueue::Remove(AudioBuffer *bufferPointer)
{
	CollectionType::iterator iterator;
	bool isRequestingRepeatSearch;
	bool wasRemoved = false;

	// Do not continue if given null.
	if (nullptr == bufferPointer)
	{
		return false;
	}

	// Remove all audio buffers from the queue matching the given memory address.
	do
	{
		isRequestingRepeatSearch = false;
		for (iterator = fCollection.begin(); iterator != fCollection.end(); iterator++)
		{
			if (bufferPointer == (*iterator))
			{
				fCollection.erase(iterator);
				wasRemoved = true;
				isRequestingRepeatSearch = true;
				break;
			}
		}
	} while (isRequestingRepeatSearch);

	// Returns true if at least 1 memory buffer was removed.
	return wasRemoved;
}

bool AudioBufferQueue::RemoveById(ALuint bufferId)
{
	CollectionType::iterator iterator;
	bool isRequestingRepeatSearch;
	bool wasRemoved = false;

	// Remove all audio buffers from the queue matching the given ID.
	do
	{
		isRequestingRepeatSearch = false;
		for (iterator = fCollection.begin(); iterator != fCollection.end(); iterator++)
		{
			if (bufferId == (*iterator)->GetIntegerId())
			{
				fCollection.erase(iterator);
				wasRemoved = true;
				isRequestingRepeatSearch = true;
				break;
			}
		}
	} while (isRequestingRepeatSearch);

	// Returns true if at least 1 memory buffer was removed.
	return wasRemoved;
}

void AudioBufferQueue::Clear()
{
	fCollection.clear();
}

bool AudioBufferQueue::Contains(AudioBuffer *bufferPointer)
{
	// Do not continue if given null.
	if (nullptr == bufferPointer)
	{
		return false;
	}

	// Find the specified buffer in the collection.
	for (auto&& nextBufferPointer : fCollection)
	{
		if (bufferPointer == nextBufferPointer)
		{
			// Found it!
			return true;
		}
	}

	// The buffer was not found.
	return false;
}

bool AudioBufferQueue::ContainsId(ALuint bufferId)
{
	// Find the specified buffer in the collection.
	for (auto&& nextBufferPointer : fCollection)
	{
		if (bufferId == nextBufferPointer->GetIntegerId())
		{
			// Found it!
			return true;
		}
	}

	// The buffer was not found.
	return false;
}

AudioBuffer* AudioBufferQueue::GetFront()
{
	if (fCollection.empty())
	{
		return nullptr;
	}
	return fCollection.front();
}

int AudioBufferQueue::GetCount()
{
	return fCollection.size();
}

uint64 AudioBufferQueue::GetTotalBytes()
{
	uint64 totalBytes = 0;
	for (auto&& nextBufferPointer : fCollection)
	{
		totalBytes += nextBufferPointer->GetByteCount();
	}
	return totalBytes;
}

uint64 AudioBufferQueue::GetTotalSamples()
{
	uint64 totalSamples = 0;
	for (auto&& nextBufferPointer : fCollection)
	{
		totalSamples += nextBufferPointer->GetSampleCount();
	}
	return totalSamples;
}

double AudioBufferQueue::GetTotalDurationInSeconds()
{
	double totalSeconds = 0;
	for (auto&& nextBufferPointer : fCollection)
	{
		totalSeconds += nextBufferPointer->GetDurationInSeconds();
	}
	return totalSeconds;
}

bool AudioBufferQueue::IsEmpty()
{
	return fCollection.empty();
}

bool AudioBufferQueue::IsNotEmpty()
{
	return !fCollection.empty();
}

#pragma endregion

}	// namespace AL
