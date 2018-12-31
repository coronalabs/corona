// ----------------------------------------------------------------------------
// 
// AudioContext.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include <pch.h>
#include "AudioContext.h"
#include "AudioEnvironment.h"
#include "XAudio2VoiceFactory.h"


namespace AL {

#pragma region Static Member Variables
AudioContext::CreatedEvent AudioContext::sCreatedEvent;

AudioContext::DestroyingEvent AudioContext::sDestroyingEvent;

#pragma endregion


#pragma region Constructors/Destructors
AudioContext::AudioContext(IXAudio2 *xaudioEnginePointer, IXAudio2MasteringVoice *xaudioMasteringVoicePointer)
:	fXAudioEnginePointer(xaudioEnginePointer),
	fXAudioMasteringVoicePointer(xaudioMasteringVoicePointer),
	fListenerPointer(nullptr),
	fSourceManagerPointer(nullptr)
{
	// Create the audio listener.
	{
		AudioListener::CreationSettings settings;
		settings.ContextPointer = this;
		settings.XAudioMasteringVoicePointer = fXAudioMasteringVoicePointer;
		fListenerPointer = new AudioListener(settings);
	}

	// Set up the audio source manager.
	{
		AudioSourceManager::CreationSettings settings;
		settings.ContextPointer = this;
		settings.VoiceFactory = XAudio2VoiceFactory(fXAudioEnginePointer);
		fSourceManagerPointer = new AudioSourceManager(settings);
	}

	// Raise a static event notifying the system that a new audio context has been created.
	// Note: We must call the AddContextEventHandlers() function before raising any events in this class to ensure that
	//       the AudioEnvironment class will receive this class' events. I don't like tightly coupling it like this
	//       and should find a better way to subscribe to this class' events in the future.
	AudioEnvironment::AddContextEventHandlers();
	sCreatedEvent.Raise(*this, EventArgs::kEmpty);
}

AudioContext::~AudioContext()
{
	// Raise a static event notifying the system that this audio context is about to be destroyed.
	sDestroyingEvent.Raise(*this, EventArgs::kEmpty);

	// Destroy all audio sources first.
	// Note: XAudio2 requires the source voices to be deleted before the master voice.
	fSourceManagerPointer->DestroyAll();
	delete fSourceManagerPointer;

	// Destroy the audio listener.
	delete fListenerPointer;
}

#pragma endregion


#pragma region Public Instance Methods
AudioListener& AudioContext::GetListener()
{
	return *fListenerPointer;
}

AudioBufferManager& AudioContext::GetBufferManager()
{
	return fBufferManager;
}

AudioSourceManager& AudioContext::GetSourceManager()
{
	return *fSourceManagerPointer;
}

void AudioContext::Suspend()
{
	if (fXAudioEnginePointer)
	{
		fXAudioEnginePointer->StopEngine();
	}
}

void AudioContext::Resume()
{
	if (fXAudioEnginePointer)
	{
		fXAudioEnginePointer->StartEngine();
	}
}

#pragma endregion


#pragma region Public Static Functions
AudioContext::CreationResult AudioContext::Create(const AudioContext::CreationSettings &settings)
{
	// Attempt to create the XAudio2 engine.
	IXAudio2 *xaudioEnginePointer = nullptr;
	HRESULT result = ::XAudio2Create(&xaudioEnginePointer);
	if (nullptr == xaudioEnginePointer)
	{
		return CreationResult::FailedWith(result);
	}

	// Fetch the output sample rate from the given settings.
	// If the given sample rate is not valid, then make it valid.
	UINT32 outputSampleRate = settings.OutputSampleRate;
	if (outputSampleRate != XAUDIO2_DEFAULT_SAMPLERATE)
	{
		if (outputSampleRate < XAUDIO2_MIN_SAMPLE_RATE)
		{
			// Limit the sample rate to the minimum value allowed.
			outputSampleRate = XAUDIO2_MIN_SAMPLE_RATE;
		}
		else if (outputSampleRate > XAUDIO2_MAX_SAMPLE_RATE)
		{
			// Limit the sample rate to the maximum value allowed.
			outputSampleRate = XAUDIO2_MAX_SAMPLE_RATE;
		}
		else
		{
			// Ensure that the given sample rate is a multiple of XAUDIO2_QUANTUM_DENOMINATOR.
			UINT32 value = outputSampleRate % XAUDIO2_QUANTUM_DENOMINATOR;
			if (value)
			{
				outputSampleRate -= value;
			}
		}
	}

	// Create the XAudio2 mastering voice. This is equivalent to OpenAL's listener.
	IXAudio2MasteringVoice *xaudioMasteringVoicePointer = nullptr;
	result = xaudioEnginePointer->CreateMasteringVoice(
						&xaudioMasteringVoicePointer, XAUDIO2_DEFAULT_CHANNELS, outputSampleRate);
	if (nullptr == xaudioMasteringVoicePointer)
	{
		xaudioEnginePointer->Release();
		return CreationResult::FailedWith(result);
	}

	// Create an audio context which wraps the above XAudio2 objects and return it.
	auto contextPointer = new AudioContext(xaudioEnginePointer, xaudioMasteringVoicePointer);
	return CreationResult::SucceededWith(contextPointer);
}

void AudioContext::Destroy(AudioContext *contextPointer)
{
	// Validate argument.
	if (nullptr == contextPointer)
	{
		return;
	}

	// First, fetch the audio context's XAudio2 objects that were passed into it via this class' static Create() function.
	// Since the audio context's constructor did not create these object, its destructor will not destroy them.
	// Note: This follows the old school memory ownership rule of whoever creates an object is responsible for deleting it.
	IXAudio2 *xaudioEnginePointer = contextPointer->fXAudioEnginePointer;
	IXAudio2MasteringVoice *xaudioMasteringVoicePointer = contextPointer->fXAudioMasteringVoicePointer;

	// Destroy the given audio context. This will also destroy all of the audio buffers and XAudio2 source voices it owns.
	// Note: This must be done before destroying the XAudio2 mastering voice and engine.
	delete contextPointer;

	// Destroy the XAudio2 mastering voice. (Must be done before releasing the XAudio2 engine.)
	if (xaudioMasteringVoicePointer)
	{
		xaudioMasteringVoicePointer->DestroyVoice();
	}

	// Release the XAudio2 engine. (Must be done after destroying all XAudio2 voices.)
	if (xaudioEnginePointer)
	{
		xaudioEnginePointer->Release();
	}
}

AudioContext::CreatedEvent::HandlerManager& AudioContext::GetCreatedEventHandlers()
{
	return sCreatedEvent.GetHandlerManager();
}

AudioContext::DestroyingEvent::HandlerManager& AudioContext::GetDestroyingEventHandlers()
{
	return sDestroyingEvent.GetHandlerManager();
}

#pragma endregion


#pragma region CreationResult Class Methods
AudioContext::CreationResult::CreationResult(AudioContext *contextPointer, HRESULT resultCode)
:	fContextPointer(contextPointer),
	fResultCode(resultCode)
{
}

bool AudioContext::CreationResult::HasSucceeded()
{
	return fContextPointer != nullptr;
}

bool AudioContext::CreationResult::HasFailed()
{
	return fContextPointer == nullptr;
}

AudioContext* AudioContext::CreationResult::GetContext()
{
	return fContextPointer;
}

HRESULT AudioContext::CreationResult::GetHRESULT()
{
	return fResultCode;
}

AudioContext::CreationResult AudioContext::CreationResult::SucceededWith(AudioContext *contextPointer)
{
	if (nullptr == contextPointer)
	{
		if (IsDebuggerPresent())
		{
			__debugbreak();
		}
		return CreationResult::FailedWith(E_FAIL);
	}
	return CreationResult(contextPointer, S_OK);
}

AudioContext::CreationResult AudioContext::CreationResult::FailedWith(HRESULT errorCode)
{
	if (S_OK == errorCode)
	{
		if (IsDebuggerPresent())
		{
			__debugbreak();
		}
		errorCode = E_FAIL;
	}
	return AudioContext::CreationResult(nullptr, errorCode);
}

#pragma endregion

}	// namespace AL
