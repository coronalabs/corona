// ----------------------------------------------------------------------------
// 
// AudioBuffer.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

#include "al.h"
#include "AudioFormatInfo.h"
#include "Event.h"
#include "EventArgs.h"


namespace AL {

/// <summary>
///  <para>Stores a byte buffer of audio data and provides information about its audio format.</para>
///  <para>An audio buffer is used by audio sources for playback.</para>
///  <para>One audio buffer can be used by multiple audio sources at the same time, which saves memory.</para>
/// </summary>
class AudioBuffer
{
	public:
		/// <summary>
		///  <para>Defines the "Updating" event type which is raised just before new data is copied into the audio buffer.</para>
		///  <para>Handlers such as audio sources are expected to stop playback if they're current using this buffer.</para>
		/// </summary>
		typedef Event<const AudioBuffer&, const EventArgs&> UpdatingEvent;

		/// <summary>
		///  Defines the "Updated" event type which is raised just after data has been copied to the audio buffer.
		/// </summary>
		typedef Event<const AudioBuffer&, const EventArgs&> UpdatedEvent;
		
		/// <summary>
		///  <para>
		///   Defines the "Destroying" event type which is raised when this audio buffer object is about to be
		///   destroyed via its destructor.
		///  </para>
		///  <para>Handlers are expected to let go of there reference to this buffer in this case.</para>
		/// </summary>
		typedef Event<const AudioBuffer&, const EventArgs&> DestroyingEvent;

		/// <summary>Represents an invalid audio buffer ID.</summary>
		static const ALuint kInvalidId;

		/// <summary>Creates a new buffer for storing audio data.</summary>
		/// <param name="id">Unique integer ID to be assigned to this buffer.</param>
		AudioBuffer(ALuint id);

		/// <summary>Destroys this audio buffer.</summary>
		virtual ~AudioBuffer();

		/// <summary>Gets an object used to add or remove an event handler for the "Updating" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		UpdatingEvent::HandlerManager& GetUpdatingEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "Updated" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		UpdatedEvent::HandlerManager& GetUpdatedEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "Destroying" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		DestroyingEvent::HandlerManager& GetDestroyingEventHandlers();

		/// <summary>Gets the unique integer ID assigned to this buffer by the audio system.</summary>
		/// <returns>Returns this buffer's unique integer ID.</returns>
		ALuint GetIntegerId();

		/// <summary>
		///  Gets information about how the audio buffer is formatted such as number of channels, data type, frequency, etc.
		/// </summary>
		/// <returns>Returns the audio buffer's format read-only information.</returns>
		AudioFormatInfo GetFormatInfo();

		/// <summary>Gets a pointer to this buffer's byte array.</summary>
		/// <returns>
		///  <para>Returns a pointer to this buffer's byte array.</para>
		///  <para>Returns null if audio data has not been loaded via the UpdateWith() function yet.</para>
		/// </returns>
		uint8* GetBytBuffer();

		/// <summary>Gets the number of bytes in the buffer.</summary>
		/// <returns>
		///  <para>
		///   Returns the number of bytes in the buffer. Expected to be used in conjuction with byte array returned
		///   by the GetByteBuffer() function.
		///  </para>
		///  <para>Returns zero if audio data has not been loaded via the UpdateWith() function yet.</para>
		/// </returns>
		uint32 GetByteCount();

		/// <summary>Gets the number of audio samples/frames in the buffer.</summary>
		/// <returns>
		///  <para>Returns the number of audio samples/frames stored in the buffer.</para>
		///  <para>Returns zero if audio data has not been loaded via the UpdateWith() function yet.</para>
		/// </returns>
		uint32 GetSampleCount();

		/// <summary>Gets how many seconds worth of audio data is stored in the buffer.</summary>
		/// <returns>
		///  <para>Returns the total duration of the audio buffer in fractional seconds.</para>
		///  <para>Returns zero if audio data has not been loaded via the UpdateWith() function yet.</para>
		/// </returns>
		double GetDurationInSeconds();

		/// <summary>Copies the given audio data into this byte buffer object.</summary>
		/// <param name="formatInfo">Describes the audio format of the given byte buffer.</param>
		/// <param name="byteBufferPointer">
		///  Pointer to the byte buffer to be copied to this buffer object. Cannot be null.
		/// </para>
		/// <param name="byteCount">
		///  <para>Number of bytes to be copied from the "byteBuffer" parameter.</para>
		///  <para>Will clear/delete this buffer's bytes if set to zero.</para>
		/// </param>
		/// <returns>
		///  <para>Returns true if the given byte buffer was successfully copied to this object.</para>
		///  <para>Returns false if given invalid arguments or if unable to allocate memory for the given buffer.</para>
		/// </returns>
		bool UpdateWith(const AudioFormatInfo &formatInfo, const uint8 *byteBufferPointer, const uint32 byteCount);

		/// <summary>
		///  <para>Deletes this buffer's audio data from memory.</para>
		///  <para>The GetByteBuffer() method will return null after calling this method.</para>
		/// </summary>
		void Clear();

	private:
		/// <summary>Manages the "Updating" event.</summary>
		UpdatingEvent fUpdatingEvent;

		/// <summary>Manages the "Updated" event.</summary>
		UpdatedEvent fUpdatedEvent;
		
		/// <summary>Manages the "Destroying" event.</summary>
		DestroyingEvent fDestroyingEvent;

		/// <summary>Describes the format of the audio data stored in "fByteBuffer".</summary>
		AudioFormatInfo fFormatInfo;

		/// <summary>Unique integer ID assigned to this buffer.</summary>
		ALuint fIntegerId;

		/// <summary>Byte array storing the audio data.</summary>
		uint8 *fByteBufferPointer;

		/// <summary>Number of bytes that "fByteBuffer" is using.</summary>
		uint32 fByteCount;
};

}	// namespace AL
