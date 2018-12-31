// ----------------------------------------------------------------------------
// 
// AudioSourceManager.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include <pch.h>
#include "AudioSourceManager.h"
#include "AudioSource.h"
#include "AudioContext.h"
#include "XAudio2VoiceFactory.h"


namespace AL {

#pragma region Constructors/Destructors
AudioSourceManager::AudioSourceManager()
:	fNextSourceId(1),
	fContextPointer(nullptr)
{
}

AudioSourceManager::AudioSourceManager(const AudioSourceManager::CreationSettings &settings)
:	fNextSourceId(1),
	fContextPointer(settings.ContextPointer),
	fVoiceFactory(settings.VoiceFactory)
{
}

AudioSourceManager::~AudioSourceManager()
{
	DestroyAll();
}

#pragma endregion


#pragma region Public Methods
AudioSource* AudioSourceManager::Create()
{
	// Do not continue if this manager was not given the objects needed to create a source upon construction.
	if (nullptr == fContextPointer)
	{
		return nullptr;
	}

	// Generate a unique integer ID for the new audio source.
	while ((AudioSource::kInvalidId == fNextSourceId) || (fSourceCollection.find(fNextSourceId) != fSourceCollection.end()))
	{
		fNextSourceId++;
	}
	ALuint newSourceId = fNextSourceId;
	fNextSourceId++;

	// Create the new audio source.
	AudioSource::CreationSettings settings;
	settings.IntegerId = newSourceId;
	settings.ContextPointer = fContextPointer;
	settings.VoiceFactoryPointer = &fVoiceFactory;
	auto sourcePointer = new AudioSource(settings);
	if (nullptr == sourcePointer)
	{
		return nullptr;
	}

	// Add the new audio source to the collection.
	fSourceCollection.insert(CollectionPairType(newSourceId, sourcePointer));

	// Return the new audio source.
	return sourcePointer;
}

void AudioSourceManager::DestroyAll()
{
	for (auto&& nextPair : fSourceCollection)
	{
		delete nextPair.second;
	}
	fSourceCollection.clear();
}

bool AudioSourceManager::DestroyById(ALuint id)
{
	// Fetch the audio source by its unique ID.
	auto iterator = fSourceCollection.find(id);
	if (iterator == fSourceCollection.end())
	{
		return false;
	}

	// Delete the source and remove it from the collection.
	delete (*iterator).second;
	fSourceCollection.erase(iterator);
	return true;
}

bool AudioSourceManager::ContainsId(ALuint id)
{
	return (GetById(id) != nullptr);
}

AudioSource* AudioSourceManager::GetById(ALuint id)
{
	auto iterator = fSourceCollection.find(id);
	if (iterator == fSourceCollection.end())
	{
		return nullptr;
	}
	return (*iterator).second;
}

AudioSource* AudioSourceManager::GetByIndex(int index)
{
	// Validate argument.
	if ((index < 0) || (index >= fSourceCollection.size()))
	{
		return nullptr;
	}

	// Fetch the indexed source.
	for (auto&& nextPair : fSourceCollection)
	{
		if (0 == index)
		{
			return nextPair.second;
		}
		index--;
	}
	return nullptr;
}

int AudioSourceManager::GetCount()
{
	return fSourceCollection.size();
}

AudioContext* AudioSourceManager::GetContext()
{
	return fContextPointer;
}

#pragma endregion

}	// namespace AL
