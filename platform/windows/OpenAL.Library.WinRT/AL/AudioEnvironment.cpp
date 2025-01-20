//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <pch.h>
#include "AudioEnvironment.h"
#include "AudioContext.h"
#include <mutex>


namespace AL {

#pragma region Static Member Variables
AudioContext *AudioEnvironment::sCurrentContext = nullptr;

std::recursive_mutex AudioEnvironment::sMutex;

AudioContextCollection AudioEnvironment::sContextCollection;

AudioContext::CreatedEvent::FunctionHandler AudioEnvironment::sCreatedContextEventHandler(&AudioEnvironment::OnCreatedContext);

AudioContext::DestroyingEvent::FunctionHandler AudioEnvironment::sDestroyingContextEventHandler(&AudioEnvironment::OnDestroyingContext);

#pragma endregion


#pragma region Public Static Functions
AudioContext* AudioEnvironment::GetCurrentContext()
{
	return sCurrentContext;
}

bool AudioEnvironment::SetCurrentContext(AudioContext *contextPointer)
{
	AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Do not continue if given a pointer to an audio context that no longer exists. (ie: A wild pointer.)
	// Note: A null pointer is okay.
	if (contextPointer && (sContextCollection.Contains(contextPointer) == false))
	{
		return false;
	}

	// Make the given audio context the current context.
	sCurrentContext = contextPointer;
	return true;
}

std::recursive_mutex& AudioEnvironment::GetMutex()
{
	return sMutex;
}

#pragma endregion


#pragma region Private Static Functions
void AudioEnvironment::AddContextEventHandlers()
{
	AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Add this class' event handlers, if not done already.
	if (AudioContext::GetCreatedEventHandlers().Contains(&sCreatedContextEventHandler) == false)
	{
		AudioContext::GetCreatedEventHandlers().Add(&sCreatedContextEventHandler);
		AudioContext::GetDestroyingEventHandlers().Add(&sDestroyingContextEventHandler);
	}
}

void AudioEnvironment::OnCreatedContext(AudioContext &context, const EventArgs &arguments)
{
	AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Add the newly created audio context to the collection.
	sContextCollection.Add(&context);
}

void AudioEnvironment::OnDestroyingContext(AudioContext &context, const EventArgs &arguments)
{
	AudioEnvironment::ScopedMutexLock scopedMutexLock;

	// Make the current context null if it is being destroyed. This prevents wild pointer issues.
	if (&context == sCurrentContext)
	{
		sCurrentContext = nullptr;
	}

	// Remove the audio context from the static collection.
	sContextCollection.Remove(&context);
}

#pragma endregion

}	// namespace AL
