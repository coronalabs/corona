//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "al.h"
#include "AudioBuffer.h"
#include <deque>


namespace AL {

/// <summary>Stores a queue of audio buffers.</summary>
/// <remarks>This class is intended to be used to queue buffers for audio streaming.</remarks>
class AudioBufferQueue
{
	public:
		/// <summary>Creates an empty queue for storing audio buffers.</summary>
		AudioBufferQueue();

		/// <summary>Destroys this queue.</summary>
		/// <remarks>
		///  This destructor will not destroy the audio buffers that were pushed into the queue.
		///  That is the responsibility of the caller thta pushed them into this queue.
		/// </remarks>
		virtual ~AudioBufferQueue();

		/// <summary>Pushes the given audio buffer to the end of the queue.</summary>
		/// <param name="bufferPointer">
		///  <para>Pointer to the audio buffer to be pushed into the queue.</para>
		///  <para>Will be ignored if given null.</para>
		/// </param>
		void Push(AudioBuffer *bufferPointer);

		/// <summary>Removes and returns the audio buffer at the front of the queue.</summary>
		/// <returns>
		///  <para>Returns a pointer to the audio buffer at the front of the queue.</para>
		///  <para>Returns null if the queue is empty.</para>
		/// </returns>
		AudioBuffer* Pop();

		/// <summary>
		///  <para>Removes the given audio buffer from the queue by its memory address.</para>
		///  <para>If the specified audio buffer was pushed in multiple times, then they will all be removed.</para>
		///  <para>This function will not delete the audio buffer from memory. That is the caller's responsibility.</para>
		/// </summary>
		/// <param name="bufferPointer">Memory address of the audio buffer to remove.</param>
		/// <returns>
		///  <para>Returns true if the specified audio buffer was removed from the queue.</para>
		///  <para>Returns false if the specified audio buffer was not found.</para>
		/// </returns>
		bool Remove(AudioBuffer *bufferPointer);

		/// <summary>
		///  <para>Removes an audio buffer from the queue by its unique integer ID.</para>
		///  <para>If the specified audio buffer was pushed in multiple times, then they will all be removed.</para>
		///  <para>This function will not delete the audio buffer from memory. That is the caller's responsibility.</para>
		/// </summary>
		/// <param name="bufferId">
		///  <para>Unique integer ID of the audio buffer to be removed.</para>
		///  <para>This is the integer ID returned by the audio buffer's GetIntegerId() function.</para>
		/// </param>
		/// <returns>
		///  <para>Returns true if the specified audio buffer was removed from the queue.</para>
		///  <para>Returns false if the specified audio buffer was not found.</para>
		/// </returns>
		bool RemoveById(ALuint bufferId);

		/// <summary>
		///  <para>Removes all audio buffers in the queue, making it empty.</para>
		///  <para>This function does not delete the audio buffers from memory. That is the caller's responsibility.</para>
		/// </summary>
		void Clear();

		/// <summary>Determine if the specified audio buffer exists in the queue.</summary>
		/// <param name="bufferPointer">The audio buffer to search for by memory address.</param>
		/// <returns>Returns true if the specified audio buffer was found in the queue. Returns false if not.</returns>
		bool Contains(AudioBuffer *bufferPointer);

		/// <summary>Determine if the specified audio buffer exists in the queue.</summary>
		/// <param name="bufferId">
		///  <para>Unique integer ID of the audio buffer to be search for.</para>
		///  <para>This is the integer ID returned by the audio buffer's GetIntegerId() function.</para>
		/// </param>
		/// <returns>Returns true if the specified audio buffer was found in the queue. Returns false if not.</returns>
		bool ContainsId(ALuint bufferId);

		/// <summary>
		///  <para>Gets the audio buffer at the front of the queue without popping it off.</para>
		///  <para>That is, the next buffer to be popped. (This is sometimes referred to as a "peek".)</para>
		/// </summary>
		/// <returns>
		///  <para>Returns a pointer to the audio buffer at the front of the queue.</para>
		/// </returns>
		AudioBuffer* GetFront();

		/// <summary>Fetches the number of audio buffers stored in the queue.</summary>
		/// <returns>
		///  <para>Returns the number of audio buffers stored in the queue.</para>
		///  <para>Returns zero if the queue is currently empty.</para>
		/// </returns>
		int GetCount();

		/// <summary>Gets the total number of bytes consumed by all of the audio buffers in the queue.</summary>
		/// <returns>
		///  <para>Returns the total number of bytes consumed by all of the audio buffers in the queue.</para>
		///  <para>Returns zero if the queue is empty or if the queue contains audio buffers having no data.</para>
		/// </returns>
		uint64 GetTotalBytes();

		/// <summary>Gets the total number of audio samples/frames provided by all of the audio buffers in the queue.</summary>
		/// <returns>
		///  <para>Returns the total number of audio samples/frames provided by all of the audio buffers in the queue.</para>
		///  <para>Returns zero if the queue is empty or if the queue contains audio buffers having no data.</para>
		/// </returns>
		uint64 GetTotalSamples();

		/// <summary>
		///  Gets the total playback time provided by all of the audio buffers in the queue in fractional seconds.
		/// </summary>
		/// <returns>
		///  <para>
		///   Returns the total playback time provided by all of the audio buffers in the queue in fractional seconds.
		///  </para>
		///  <para>Returns zero if the queue is empty or if the queue contains audio buffers having no data.</para>
		/// </returns>
		double GetTotalDurationInSeconds();

		/// <summary>Determines if this queue is currently empty.</summary>
		/// <returns>
		///  <para>Returns true if the queue is empty.</para>
		///  <para>Returns false if the queue contains at least one buffer in it.</para>
		/// </returns>
		bool IsEmpty();

		/// <summary>Determines if this queue contains at least one buffer in it.</summary>
		/// <returns>
		///  <para>Returns true if the queue contains at least one buffer in it.</para>
		///  <para>Returns false if the queue is empty.</para>
		/// </returns>
		bool IsNotEmpty();

	private:
		typedef std::deque<AudioBuffer*> CollectionType;

		/// <summary>Stores a collection of audio buffer pointers.</summary>
		CollectionType fCollection;
};

}	// namespace AL
