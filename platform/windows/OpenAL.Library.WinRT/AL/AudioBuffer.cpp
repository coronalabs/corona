//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <pch.h>
#include "AudioBuffer.h"


namespace AL {

#pragma region Public Constants
const ALuint AudioBuffer::kInvalidId = 0;

#pragma endregion


#pragma region Constructors/Destructors
AudioBuffer::AudioBuffer(ALuint id)
:	fIntegerId(id),
	fByteBufferPointer(nullptr),
	fByteCount(0)
{
}

AudioBuffer::~AudioBuffer()
{
	// Notify the system that this buffer is about to be destroyed.
	// This gives audio sources a chance to release their pointers to this buffer, if applicable.
	fDestroyingEvent.Raise(*this, EventArgs::kEmpty);

	// Delete this buffer's byte array.
	Clear();
}

#pragma endregion


#pragma region Public Methods
AudioBuffer::UpdatingEvent::HandlerManager& AudioBuffer::GetUpdatingEventHandlers()
{
	return fUpdatingEvent.GetHandlerManager();
}

AudioBuffer::UpdatedEvent::HandlerManager& AudioBuffer::GetUpdatedEventHandlers()
{
	return fUpdatedEvent.GetHandlerManager();
}

AudioBuffer::DestroyingEvent::HandlerManager& AudioBuffer::GetDestroyingEventHandlers()
{
	return fDestroyingEvent.GetHandlerManager();
}

ALuint AudioBuffer::GetIntegerId()
{
	return fIntegerId;
}

AudioFormatInfo AudioBuffer::GetFormatInfo()
{
	return fFormatInfo;
}

uint8* AudioBuffer::GetBytBuffer()
{
	return fByteBufferPointer;
}

uint32 AudioBuffer::GetByteCount()
{
	return fByteCount;
}

uint32 AudioBuffer::GetSampleCount()
{
	// Return zero if no audio data has been loaded yet.
	if (fByteCount < 1)
	{
		return 0;
	}

	// Return the number of samples/frames stored in the buffer.
	return fByteCount / fFormatInfo.GetSampleSizeInBytes();
}

double AudioBuffer::GetDurationInSeconds()
{
	// Return zero if no audio data has been loaded yet.
	if (fByteCount < 1)
	{
		return 0;
	}

	// Return the duration in fractional seconds.
	return (double)GetSampleCount() / (double)fFormatInfo.GetSampleRate();
}

bool AudioBuffer::UpdateWith(const AudioFormatInfo &formatInfo, const uint8 *byteBufferPointer, const uint32 byteCount)
{
	// Clear the byte buffer if given zero bytes to copy.
	if (byteCount < 1)
	{
		fFormatInfo = formatInfo;
		Clear();
		return true;
	}

	// Do not continue if given a null array.
	if (nullptr == byteBufferPointer)
	{
		return false;
	}

	// Notify the system that the buffer is about to be updated.
	fUpdatingEvent.Raise(*this, EventArgs::kEmpty);

	// Optimization:
	// If an allocated buffer already exists, then attempt to resize it to match the given buffer.
	if (fByteBufferPointer && (fByteCount > 0))
	{
		void *newByteBufferPointer = ::realloc(fByteBufferPointer, byteCount);
		if (newByteBufferPointer)
		{
			fByteBufferPointer = (uint8*)newByteBufferPointer;
			fByteCount = byteCount;
		}
	}

	// Copy the given byte buffer.
	if (byteCount == fByteCount)
	{
		::memcpy(fByteBufferPointer, byteBufferPointer, byteCount);
		fFormatInfo = formatInfo;
	}
	else
	{
		if (fByteBufferPointer)
		{
			::free(fByteBufferPointer);
			fByteBufferPointer = nullptr;
			fByteCount = 0;
		}
		fByteBufferPointer = (uint8*)::malloc(byteCount);
		if (fByteBufferPointer)
		{
			::memcpy(fByteBufferPointer, byteBufferPointer, byteCount);
			fByteCount = byteCount;
			fFormatInfo = formatInfo;
		}
	}

	// Do not continue if we've failed to copy the given byte buffer.
	if (fByteCount < 1)
	{
		return false;
	}

	// Update the audio format information in case it was changed.
	fFormatInfo = formatInfo;

	// Notify the system that the buffer was updated with new data.
	fUpdatedEvent.Raise(*this, EventArgs::kEmpty);
	return true;
}

void AudioBuffer::Clear()
{
	// Do not continue if there is no byte buffer to delete.
	if (nullptr == fByteBufferPointer)
	{
		return;
	}

	// Notify the system that the buffer is about to be deleted.
	fUpdatingEvent.Raise(*this, EventArgs::kEmpty);

	// Delete the byte buffer.
	::free(fByteBufferPointer);
	fByteBufferPointer = nullptr;
	fByteCount = 0;

	// Notify the system that the buffer was deleted.
	fUpdatedEvent.Raise(*this, EventArgs::kEmpty);
}

#pragma endregion

}	// namespace AL
