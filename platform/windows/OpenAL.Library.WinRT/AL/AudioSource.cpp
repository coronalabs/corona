// ----------------------------------------------------------------------------
// 
// AudioSource.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include <pch.h>
#include "AudioSource.h"
#include "AudioContext.h"
#include "XAudio2VoiceFactory.h"
#include <thread>


//JLQ: This header provides the functions needed to implement 3D positional sound.
//#include <x3daudio.h>


namespace AL {

#pragma region Public Constants
const ALuint AudioSource::kInvalidId = 0;

#pragma endregion


#pragma region Constructors/Destructors
AudioSource::AudioSource(const AudioSource::CreationSettings &settings)
:	fIntegerId(settings.IntegerId),
	fContextPointer(settings.ContextPointer),
	fVolumeScale(1.0f),
	fMinVolumeScale(0.0f),
	fMaxVolumeScale(1.0f),
	fPitchShiftScale(1.0f),
	fIsStaticBufferLoopingEnabled(false),
	fPlaybackState(AudioPlaybackState::kReseted),
	fIsXAudioVoiceActive(false),
	fIsStreaming(false),
	fStaticAudioBufferPointer(nullptr),
	fStreamingAudioController(this),
	fXAudioVoicePointer(nullptr),
	fVoiceFactoryPointer(settings.VoiceFactoryPointer),
	fVoiceEventHandler(this)
{
}

AudioSource::~AudioSource()
{
	if (fXAudioVoicePointer)
	{
		fIsXAudioVoiceActive = false;
		fXAudioVoicePointer->DestroyVoice();
	}
}

#pragma endregion


#pragma region Public Methods
ALuint AudioSource::GetIntegerId()
{
	return fIntegerId;
}

AudioContext* AudioSource::GetContext()
{
	return fContextPointer;
}

float AudioSource::GetVolumeScale()
{
	return fVolumeScale;
}

void AudioSource::SetVolumeScale(float value)
{
	// Do not allow the given value to exceed XAudio2's min/max volume levels.
	if (value > XAUDIO2_MAX_VOLUME_LEVEL)
	{
		value = XAUDIO2_MAX_VOLUME_LEVEL;
	}
	else if (value < -XAUDIO2_MAX_VOLUME_LEVEL)
	{
		value = -XAUDIO2_MAX_VOLUME_LEVEL;
	}

	// Do not continue if the value isn't changing.
	float delta = fabs(fVolumeScale - value);
	if (delta < FLT_EPSILON)
	{
		return;
	}

	// Store the given volume settings and update the XAudio2 voice if active.
	fVolumeScale = value;
	SubmitVolumeSettings();
}

float AudioSource::GetMinVolumeScale()
{
	return fMinVolumeScale;
}

void AudioSource::SetMinVolumeScale(float value)
{
	// Do not allow the given value to exceed XAudio2's min/max volume levels.
	if (value > XAUDIO2_MAX_VOLUME_LEVEL)
	{
		value = XAUDIO2_MAX_VOLUME_LEVEL;
	}
	else if (value < -XAUDIO2_MAX_VOLUME_LEVEL)
	{
		value = -XAUDIO2_MAX_VOLUME_LEVEL;
	}

	// Do not continue if the value isn't changing.
	float delta = fabs(fMinVolumeScale - value);
	if (delta < FLT_EPSILON)
	{
		return;
	}

	// Store the given volume settings and update the XAudio2 voice if active.
	fMinVolumeScale = value;
	SubmitVolumeSettings();
}

float AudioSource::GetMaxVolumeScale()
{
	return fMaxVolumeScale;
}

void AudioSource::SetMaxVolumeScale(float value)
{
	// Do not allow the given value to exceed XAudio2's min/max volume levels.
	if (value > XAUDIO2_MAX_VOLUME_LEVEL)
	{
		value = XAUDIO2_MAX_VOLUME_LEVEL;
	}
	else if (value < -XAUDIO2_MAX_VOLUME_LEVEL)
	{
		value = -XAUDIO2_MAX_VOLUME_LEVEL;
	}

	// Do not continue if the value isn't changing.
	float delta = fabs(fMaxVolumeScale - value);
	if (delta < FLT_EPSILON)
	{
		return;
	}

	// Store the given volume settings and update the XAudio2 voice if active.
	fMaxVolumeScale = value;
	SubmitVolumeSettings();
}

float AudioSource::GetPitchShiftScale()
{
	return fPitchShiftScale;
}

void AudioSource::SetPitchShiftScale(float value)
{
	// Do not allow the given value to exceed XAudio2's min/max frequencies.
	if (value > XAUDIO2_MAX_FREQ_RATIO)
	{
		value = XAUDIO2_MAX_FREQ_RATIO;
	}
	else if (value < XAUDIO2_MIN_FREQ_RATIO)
	{
		value = XAUDIO2_MIN_FREQ_RATIO;
	}

	// Do not continue if the value isn't changing.
	float delta = fabs(fPitchShiftScale - value);
	if (delta < FLT_EPSILON)
	{
		return;
	}

	// Store the given pitch shift and update the XAudio2 voice if active.
	fPitchShiftScale = value;
	SubmitPitchSettings();
}

Position AudioSource::GetPosition()
{
	return fPosition;
}

void AudioSource::SetPosition(const Position &value)
{
	fPosition = value;
}

Vector AudioSource::GetVelocity()
{
	return fVelocity;
}

void AudioSource::SetVelocity(const Vector &value)
{
	fVelocity = value;
}

bool AudioSource::IsStaticBufferLoopingEnabled()
{
	return fIsStaticBufferLoopingEnabled;
}

void AudioSource::SetIsStaticBufferLoopingEnabled(bool value)
{
	// Do not continue if this setting isn't changing.
	if (value == fIsStaticBufferLoopingEnabled)
	{
		return;
	}

	// Store the new setting.
	fIsStaticBufferLoopingEnabled = value;

	// Disable looping now if this audio source is currently playing a static audio buffer.
	std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);
	if (fXAudioVoicePointer && !fIsStaticBufferLoopingEnabled && !fIsStreaming)
	{
		fXAudioVoicePointer->ExitLoop();
	}
}

AudioPlaybackState AudioSource::GetPlaybackState()
{
	return fPlaybackState;
}

AudioPlaybackPosition AudioSource::GetPlaybackPositionIn(AudioPlaybackPosition::Units units)
{
	std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);

	// Return a position of zero if stopped.
	if ((nullptr == fXAudioVoicePointer) ||
	    fPlaybackState.Equals(AudioPlaybackState::kStopped) ||
	    fPlaybackState.Equals(AudioPlaybackState::kReseted))
	{
		return AudioPlaybackPosition(units, 0);
	}

	// Fetch the current audio position.
	XAUDIO2_VOICE_STATE xaudioVoiceState;
	memset(&xaudioVoiceState, 0, sizeof(XAUDIO2_VOICE_STATE));
	fXAudioVoicePointer->GetState(&xaudioVoiceState);
	uint32 currentPositionInSamples = 0;
	uint32 startPositionInSamples = fPlaybackStartPosition.ToSamplesUsing(fCurrentAudioFormat).GetValue();
	if (fIsStreaming)
	{
		// For streaming audio, the position needs to be relative to the first buffer in the queue.
		uint64 firstQueuedSamplePosition = fStreamingAudioData.UnprocessedSamplePosition;
		firstQueuedSamplePosition -=
					fStreamingAudioData.ActiveBuffers.GetTotalSamples() +
					fStreamingAudioData.ProcessedBuffers.GetTotalSamples();
		currentPositionInSamples =
					(xaudioVoiceState.SamplesPlayed + startPositionInSamples) - firstQueuedSamplePosition;
	}
	else if (fStaticAudioBufferPointer)
	{
		// Calculate the current position for static audio buffer playback.
		currentPositionInSamples =
				(xaudioVoiceState.SamplesPlayed + startPositionInSamples) % fStaticAudioBufferPointer->GetSampleCount();
	}

	// Return the current playback position in the requested units.
	return AudioPlaybackPosition::FromSamples(currentPositionInSamples).ToUnitUsing(units, fCurrentAudioFormat);
}

bool AudioSource::SetPlaybackPosition(AudioPlaybackPosition position)
{
	// If this audio source is using a static buffer, then do not continue if the given position exceeds its bounds.
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);
		if (IsNotStreaming() && fStaticAudioBufferPointer)
		{
			uint32 positionInBytes = position.ToBytesUsing(fStaticAudioBufferPointer->GetFormatInfo()).GetValue();
			if (positionInBytes > fStaticAudioBufferPointer->GetByteCount())
			{
				return false;
			}
		}
	}

//TODO: This function does not correctly seek to the given position when playback is paused. To be fixed later...

	// Stop playback if currently playing.
	bool wasPlaying = fPlaybackState.Equals(AudioPlaybackState::kPlaying);
	if (wasPlaying)
	{
		// This blocks until the submitted audio buffers have been released by the XAudio2 voice.
		Stop();

		// If we're streaming audio, then push all audio buffers into the "unprocessed" queue.
		// This way, when playback restarts, this audio source will automatically "seek" to the given position.
		if (fIsStreaming)
		{
			std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);
			AudioBuffer *bufferPointer;
			while (fStreamingAudioData.ProcessedBuffers.IsNotEmpty())
			{
				bufferPointer = fStreamingAudioData.ProcessedBuffers.Pop();
				fStreamingAudioData.UnprocessedBuffers.Push(bufferPointer);
			}
		}
	}

	// Store the given position to be used the next time audio is played.
	fPlaybackStartPosition = position;

	// Restart playback at the given position now if this source was currently playing.
	if (wasPlaying)
	{
		Play();
	}
	return true;
}

bool AudioSource::IsStreaming()
{
	return fIsStreaming;
}

bool AudioSource::IsNotStreaming()
{
	return !fIsStreaming;
}

void AudioSource::SetIsStreaming(bool value)
{
	// Do not continue if the setting won't be changing.
	if (value == fIsStreaming)
	{
		return;
	}

	// Stop audio playback, if not done already.
	Stop();

	// Enable/disable streaming.
	fIsStreaming = value;
}

void AudioSource::SetStaticBuffer(AudioBuffer *bufferPointer)
{
	// Do not continue if the buffer isn't changing.
	if (bufferPointer == fStaticAudioBufferPointer)
	{
		return;
	}

	// Stop audio playback, if not done already.
	bool wasPlaying = (AudioPlaybackState::kPlaying == fPlaybackState);
	if (wasPlaying)
	{
		Stop();
	}

	// Store the given audio buffer.
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);
		fStaticAudioBufferPointer = bufferPointer;
	}

	// Restart playback if previously playing.
	if (bufferPointer && wasPlaying)
	{
		Play();
	}
}

AudioBuffer* AudioSource::GetStaticBuffer()
{
	return fStaticAudioBufferPointer;
}

AudioSource::StreamingAudioController& AudioSource::GetStreamingBuffers()
{
	return fStreamingAudioController;
}

void AudioSource::Play()
{
	// Stop playback if currently playing.
	// Note: OpenAL specification states that calling alSourcePlay() on a currently playing source will restart playback.
	if (fPlaybackState.Equals(AudioPlaybackState::kPlaying))
	{
		StopAndRewind();
	}

	// We must lock the thread after stopping playback up above.
	std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);

	// Fetch an audio buffer to play.
	AudioBuffer *bufferPointer = nullptr;
	if (fIsStreaming)
	{
		bufferPointer = fStreamingAudioData.UnprocessedBuffers.GetFront();
		if (nullptr == bufferPointer)
		{
			bufferPointer = fStreamingAudioData.ActiveBuffers.GetFront();
		}
	}
	else
	{
		bufferPointer = fStaticAudioBufferPointer;
	}
	if ((nullptr == bufferPointer) || (bufferPointer->GetByteCount() < 1))
	{
		// This audio source was not given an audio buffer to play. Stop here.
		return;
	}

	// If the audio format has changed since the last play, then we must recreate the XAudio2 voice with the new format.
	if (fCurrentAudioFormat.NotEquals(bufferPointer->GetFormatInfo()))
	{
		if (fXAudioVoicePointer)
		{
			fXAudioVoicePointer->DestroyVoice();
			fXAudioVoicePointer = nullptr;
			fPlaybackState = AudioPlaybackState::kReseted;
		}
		fCurrentAudioFormat = bufferPointer->GetFormatInfo();
	}

	// If an XAudio2 source voice has not been created yet, then do so now.
	// Note: This is created on demand and re-used when replaying sound.
	if (nullptr == fXAudioVoicePointer)
	{
		WAVEFORMATEXTENSIBLE waveFormat;
		if (bufferPointer->GetFormatInfo().GetChannelCount() <= 2)
		{
			// The WAVEFORMATEX structure can only be used for mono and stereo sound.
			waveFormat.Format = bufferPointer->GetFormatInfo().ToWAVEFORMATEX();
		}
		else
		{
			// We must use a WAVEFORMATEXTENSIBLE structure for surround sound support.
			waveFormat = bufferPointer->GetFormatInfo().ToWAVEFORMATEXTENSIBLE();
		}
		HRESULT result = fVoiceFactoryPointer->CreateSourceVoice(
								&fXAudioVoicePointer, (const WAVEFORMATEX*)&waveFormat,
								0, XAUDIO2_MAX_FREQ_RATIO, &fVoiceEventHandler);
		if (FAILED(result))
		{
			return;
		}
	}

	// Submit this source's configuratoin to the XAudio2 voice.
	SubmitVolumeSettings();
	SubmitPitchSettings();

	// Submit this source's audio buffer(s) to the XAudio2 voice, if not done already.
	if (fPlaybackState.NotEquals(AudioPlaybackState::kPaused))
	{
		SubmitAudioBuffers();
	}

	// Start playing sound.
	fPlaybackState = AudioPlaybackState::kPlaying;
	fIsXAudioVoiceActive = true;
	fXAudioVoicePointer->Start();
}

void AudioSource::Pause()
{
	std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);

	// Do not continue if not currently playing.
	if (fPlaybackState.NotEquals(AudioPlaybackState::kPlaying))
	{
		return;
	}

	// Pause playback.
	if (fXAudioVoicePointer)
	{
		fXAudioVoicePointer->Stop();
	}
	fPlaybackState = AudioPlaybackState::kPaused;
}

void AudioSource::Stop()
{
	// Do not continue if already stopped.
	if (fPlaybackState.Equals(AudioPlaybackState::kStopped) || fPlaybackState.Equals(AudioPlaybackState::kReseted))
	{
		return;
	}

	// Request the XAudio2 voice to stop playback.
	// Note: This is asynchronous and does not stop immediately.
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);
		fIsXAudioVoiceActive = false;
		if (fXAudioVoicePointer)
		{
			fXAudioVoicePointer->Stop();
			fXAudioVoicePointer->FlushSourceBuffers();
		}
	}

	// Block until the XAudio2 voice has released the audio buffer(s) that were submitted to it.
	// We must block because OpenAL allows audio buffers to be re-usable immediately after stopping an audio source.
	// The OnBufferEnd() callback will flag the playback state as "stopped" once these buffers have been released.
	auto abortTimeInTicks = ::GetTickCount64() + 1000;
	while (fPlaybackState.NotEquals(AudioPlaybackState::kStopped))
	{
		// If the XAudio2 voice is taking too long to release its audio buffers, then abort.
		// Note: This can happen if the XAudio2 engine has been suspended.
		if (::GetTickCount64() >= abortTimeInTicks)
		{
			std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);

			// Forcefully destroy th XAudio2 voice.
			if (fXAudioVoicePointer)
			{
				fXAudioVoicePointer->DestroyVoice();
				fXAudioVoicePointer = nullptr;
			}
			fPlaybackState = AudioPlaybackState::kStopped;
			fPlaybackStartPosition = AudioPlaybackPosition::FromSamples(0);

			// Push all streaming audio buffers into the "processed" queue.
			// This allows the caller to pop them off and re-use them.
			AudioBuffer *bufferPointer;
			while (fStreamingAudioData.ActiveBuffers.IsNotEmpty())
			{
				bufferPointer = fStreamingAudioData.ActiveBuffers.Pop();
				fStreamingAudioData.ProcessedBuffers.Push(bufferPointer);
			}
			while (fStreamingAudioData.UnprocessedBuffers.IsNotEmpty())
			{
				bufferPointer = fStreamingAudioData.UnprocessedBuffers.Pop();
				fStreamingAudioData.ProcessedBuffers.Push(bufferPointer);
			}
			fStreamingAudioData.UnprocessedSamplePosition = 0;
			break;
		}

		// Yield this thread to give the audio thread a chance to release its audio buffers.
		std::this_thread::yield();
	}
}

void AudioSource::StopAndRewind()
{
	Stop();
	fPlaybackState = AudioPlaybackState::kReseted;
}

#pragma endregion


#pragma region Private Methods
void AudioSource::SubmitVolumeSettings()
{
	std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);

	// Validate.
	if (nullptr == fXAudioVoicePointer)
	{
		return;
	}

	// Fetch the volume level to be applied to the XAudio2 voice.
	// This volume level must be within the min/max bounds applied to this audio source.
	float volumeScale = fVolumeScale;
	if (volumeScale > fMaxVolumeScale)
	{
		volumeScale = fMaxVolumeScale;
	}
	else if (volumeScale < fMinVolumeScale)
	{
		volumeScale = fMinVolumeScale;
	}

	// Update the XAudio2 voice's volume.
	fXAudioVoicePointer->SetVolume(fVolumeScale);
}

void AudioSource::SubmitPitchSettings()
{
	std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);

	// Validate.
	if (nullptr == fXAudioVoicePointer)
	{
		return;
	}

	// Update the XAudio2 voice's frequency adjustment ratio with this audio source's pitch shift scale.
	fXAudioVoicePointer->SetFrequencyRatio(fPitchShiftScale);
}

void AudioSource::SubmitAudioBuffers()
{
	std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);

	// Validate.
	if (nullptr == fXAudioVoicePointer)
	{
		return;
	}

	// Fetch the playback start position.
	// Note: This will be greater than zero if the system has "seeked" to a different position in the audio track.
	UINT32 startPositionInSamples = fPlaybackStartPosition.ToSamplesUsing(fCurrentAudioFormat).GetValue();

	// Submit audio buffer(s) to the XAudio2 voice.
	if (fIsStreaming)
	{
		// Submit as many "unprocessed" streaming audio buffers as we can.
		int buffersToSubmit = fStreamingAudioData.UnprocessedBuffers.GetCount();
		if (buffersToSubmit > XAUDIO2_MAX_QUEUED_BUFFERS)
		{
			buffersToSubmit = XAUDIO2_MAX_QUEUED_BUFFERS;
		}
		for (; buffersToSubmit > 0; buffersToSubmit--)
		{
			// Get the next unprocessed audio buffer.
			AudioBuffer *bufferPointer = fStreamingAudioData.UnprocessedBuffers.Pop();
			if (nullptr == bufferPointer)
			{
				continue;
			}

			// Determine if this audio buffer should be submitted.
			if (startPositionInSamples > (bufferPointer->GetSampleCount() + fStreamingAudioData.UnprocessedSamplePosition))
			{
				// The owner of this source is "seeking" to a position beyond the bounds of this audio buffer.
				// So, flag it as "processed" and check if the next upcoming audio buffer meets this "seek" position.
				fStreamingAudioData.ProcessedBuffers.Push(bufferPointer);
			}
			else
			{
				// Push the audio buffer into the "active" buffer queue.
				fStreamingAudioData.ActiveBuffers.Push(bufferPointer);

				// Submit the audio buffer.
				XAUDIO2_BUFFER xaudioBuffer;
				memset(&xaudioBuffer, 0, sizeof(xaudioBuffer));
				xaudioBuffer.pContext = bufferPointer;
				xaudioBuffer.pAudioData = bufferPointer->GetBytBuffer();
				xaudioBuffer.AudioBytes = bufferPointer->GetByteCount();
				if (startPositionInSamples > fStreamingAudioData.UnprocessedSamplePosition)
				{
					xaudioBuffer.PlayBegin = startPositionInSamples - fStreamingAudioData.UnprocessedSamplePosition;
				}
				fXAudioVoicePointer->SubmitSourceBuffer(&xaudioBuffer);
			}
			fStreamingAudioData.UnprocessedSamplePosition += bufferPointer->GetSampleCount();
		}
	}
	else if (fStaticAudioBufferPointer)
	{
		// Submit one static audio buffer.
		XAUDIO2_BUFFER xaudioBuffer;
		memset(&xaudioBuffer, 0, sizeof(xaudioBuffer));
		xaudioBuffer.Flags = XAUDIO2_END_OF_STREAM;
		xaudioBuffer.pContext = fStaticAudioBufferPointer;
		xaudioBuffer.pAudioData = fStaticAudioBufferPointer->GetBytBuffer();
		xaudioBuffer.AudioBytes = fStaticAudioBufferPointer->GetByteCount();
		xaudioBuffer.LoopCount = fIsStaticBufferLoopingEnabled ? XAUDIO2_LOOP_INFINITE : 0;
		xaudioBuffer.PlayBegin = startPositionInSamples;
		fXAudioVoicePointer->SubmitSourceBuffer(&xaudioBuffer);
	}
}

#pragma endregion


#pragma region StreamingAudioController Class Methods
AudioSource::StreamingAudioController::StreamingAudioController()
:	fAudioSourcePointer(nullptr)
{
}

AudioSource::StreamingAudioController::StreamingAudioController(AudioSource *audioSourcePointer)
:	fAudioSourcePointer(audioSourcePointer)
{
}

void AudioSource::StreamingAudioController::PushBuffer(AudioBuffer *bufferPointer)
{
	// Validate.
	if ((nullptr == fAudioSourcePointer) || (nullptr == bufferPointer))
	{
		return;
	}

	// Push the given buffer to the back of the "unprocessed" queue.
	std::lock_guard<std::recursive_mutex> scopedMutexLock(fAudioSourcePointer->fMutex);
	fAudioSourcePointer->fStreamingAudioData.UnprocessedBuffers.Push(bufferPointer);
}

AudioBuffer* AudioSource::StreamingAudioController::PopProcessedBuffer()
{
	// Validate.
	if (nullptr == fAudioSourcePointer)
	{
		return nullptr;
	}

	// Pop off a buffer in the "processed" queue.
	std::lock_guard<std::recursive_mutex> scopedMutexLock(fAudioSourcePointer->fMutex);
	return fAudioSourcePointer->fStreamingAudioData.ProcessedBuffers.Pop();
}

int AudioSource::StreamingAudioController::GetProcessedBufferCount()
{
	// Validate.
	if (nullptr == fAudioSourcePointer)
	{
		return 0;
	}

	// Return the number of audio buffers that have already been "processed" by the audio system.
	std::lock_guard<std::recursive_mutex> scopedMutexLock(fAudioSourcePointer->fMutex);
	return fAudioSourcePointer->fStreamingAudioData.ProcessedBuffers.GetCount();
}

int AudioSource::StreamingAudioController::GetBufferCount()
{
	// Validate.
	if (nullptr == fAudioSourcePointer)
	{
		return 0;
	}

	// Return the number of all audio buffers pushed into the source.
	std::lock_guard<std::recursive_mutex> scopedMutexLock(fAudioSourcePointer->fMutex);
	int count = fAudioSourcePointer->fStreamingAudioData.UnprocessedBuffers.GetCount();
	count += fAudioSourcePointer->fStreamingAudioData.ActiveBuffers.GetCount();
	count += fAudioSourcePointer->fStreamingAudioData.ProcessedBuffers.GetCount();
	return count;
}

#pragma endregion


#pragma region VoiceEventHandler Class Methods
AudioSource::VoiceEventHandler::VoiceEventHandler()
:	fAudioSourcePointer(nullptr)
{
}

AudioSource::VoiceEventHandler::VoiceEventHandler(AudioSource *audioSourcePointer)
:	fAudioSourcePointer(audioSourcePointer)
{
}

void AudioSource::VoiceEventHandler::OnBufferStart(void *bufferContextPointer)
{
}

void AudioSource::VoiceEventHandler::OnBufferEnd(void *bufferContextPointer)
{
	bool hasPlaybackEnded = false;

	// Validate.
	if (nullptr == fAudioSourcePointer)
	{
		return;
	}

	// Handle the fully processed/played audio buffer.
	std::lock_guard<std::recursive_mutex> scopedMutexLock(fAudioSourcePointer->fMutex);
	if (fAudioSourcePointer->fIsStreaming)
	{
		// Pop this streaming audio buffer off of the "active" queue and push it into the "processed" queue.
		AudioBuffer *bufferPointer = (AudioBuffer*)bufferContextPointer;
		if (fAudioSourcePointer->fStreamingAudioData.ActiveBuffers.GetFront() == bufferPointer)
		{
			fAudioSourcePointer->fStreamingAudioData.ActiveBuffers.Pop();
		}
		else
		{
			fAudioSourcePointer->fStreamingAudioData.ActiveBuffers.Remove(bufferPointer);
		}
		fAudioSourcePointer->fStreamingAudioData.ProcessedBuffers.Push(bufferPointer);

		// Handle the next set of "unprocessed" buffers in the queue.
		if (fAudioSourcePointer->fIsXAudioVoiceActive)
		{
			// Submit the next "unprocessed" buffer to the audio system.
			fAudioSourcePointer->SubmitAudioBuffers();
		}
		else
		{
			// This audio source is being stopped.
			// Move all of the "unprocessed" audio buffers to the "processed" queue once all active buffers have been popped.
			if (fAudioSourcePointer->fStreamingAudioData.ActiveBuffers.IsEmpty())
			{
				while (fAudioSourcePointer->fStreamingAudioData.UnprocessedBuffers.IsNotEmpty())
				{
					bufferPointer = fAudioSourcePointer->fStreamingAudioData.UnprocessedBuffers.Pop();
					fAudioSourcePointer->fStreamingAudioData.ProcessedBuffers.Push(bufferPointer);
				}
			}
		}

		// Flag this audio source as stopped if there are no more buffers in the queue.
		if (fAudioSourcePointer->fStreamingAudioData.UnprocessedBuffers.IsEmpty() &&
		    fAudioSourcePointer->fStreamingAudioData.ActiveBuffers.IsEmpty())
		{
			hasPlaybackEnded = true;
		}
	}
	else
	{
		// We'll end up here if one of the following has happened:
		// 1) We've reached the end of a static audio buffer with looping disabled.
		// 2) The FlushSourceBuffers() function was called and the audio system has just released the static audio buffer.
		if (fAudioSourcePointer->fIsXAudioVoiceActive)
		{
			if (fAudioSourcePointer->fIsStaticBufferLoopingEnabled)
			{
				// Looping was enabled while playing an audio buffer that had looping disabled.
				// We must re-submit the static audio buffer to make it loop.
				fAudioSourcePointer->fPlaybackStartPosition = AudioPlaybackPosition::FromSamples(0);
				fAudioSourcePointer->SubmitAudioBuffers();
			}
			else
			{
				// Flag this audio source as stopped now that we've reached the end of the buffer when looping is disabled.
				hasPlaybackEnded = true;
			}
		}
		else
		{
			// Flag this audio source as stopped now that the XAudio voice has released the audio buffer.
			hasPlaybackEnded = true;
		}
	}

	// Flag this audio source as stopped if requested up above.
	if (hasPlaybackEnded)
	{
		fAudioSourcePointer->fPlaybackState = AudioPlaybackState::kStopped;
		fAudioSourcePointer->fStreamingAudioData.UnprocessedSamplePosition = 0;
		fAudioSourcePointer->fPlaybackStartPosition = AudioPlaybackPosition::FromSamples(0);
		if (fAudioSourcePointer->fXAudioVoicePointer && fAudioSourcePointer->fIsStreaming)
		{
			fAudioSourcePointer->fXAudioVoicePointer->Discontinuity();
		}
	}
}

void AudioSource::VoiceEventHandler::OnLoopEnd(void *bufferContextPointer)
{
}

void AudioSource::VoiceEventHandler::OnStreamEnd()
{
}

void AudioSource::VoiceEventHandler::OnVoiceProcessingPassStart(UINT32 bytesRequired)
{
}

void AudioSource::VoiceEventHandler::OnVoiceProcessingPassEnd()
{
}

void AudioSource::VoiceEventHandler::OnVoiceError(void *bufferContextPointer, HRESULT error)
{
}

#pragma endregion

}	// namespace AL
