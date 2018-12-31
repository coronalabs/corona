// ----------------------------------------------------------------------------
// 
// AudioBufferManager.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

#include "al.h"
#include <hash_map>


namespace AL {

class AudioBuffer;

/// <summary>Creates and stores a collection of audio buffers for one audio context.</summary>
class AudioBufferManager
{
	public:
		/// <summary>Creates a new audio buffer manager.</summary>
		AudioBufferManager();

		/// <summary>Destroys this manager along with all of the audio buffers it created.</summary>
		virtual ~AudioBufferManager();

		/// <summary>Creates a new audio buffer and store it in its collection.</summary>
		/// <returns>
		///  Returns a new empty audio buffer assigned a unique integer ID amongst all other buffers in this collection.
		/// </returns>
		AudioBuffer* Create();

		/// <summary>Destroys all audio buffers created by this manager.</summary>
		void DestroyAll();

		/// <summary>Destroys an audio buffer in this manager's collection by its unique integer ID.</summary>
		/// <param name="id">Unique integer ID assigned to the audio buffer by this manager.</param>
		/// <returns>
		///  <para>Returns true if the specified audio buffer was successfully deleted.</para>
		///  <para>Returns false if the given ID was not found in this manager's collection.</para>
		/// </returns>
		bool DestroyById(ALuint id);

		/// <summary>Determines if an audio buffer having the given ID exists in this manager's collection.</summary>
		/// <param name="id">Unique integer ID of the audio buffer to search for.</param>
		/// <returns>
		///  <para>Returns true if an audio buffer having the given ID exists in the manager's collection.</para>
		///  <para>Returns false if the given ID was not found.</para>
		/// </returns>
		bool ContainsId(ALuint id);

		/// <summary>Fetches an audio buffer that was created by this manager via its unique integer ID.</summary>
		/// <param name="id">Unique integer ID of the audio buffer to fetch.</param>
		/// <returns>
		///  <para>Returns a pointer to the specified audio buffer.</para>
		///  <para>Returns null if the given ID was not found in this manager's audio buffer collection.</para>
		/// </returns>
		AudioBuffer* GetById(ALuint id);

		/// <summary>Fetches an audio buffer contained in this manager's collection by its zero based index.</summary>
		/// <param name="index">
		///  <para>Zero based index of the audio buffer to fetch.</para>
		///  <para>Must be greater than or equal to zero and less than GetCount().</para>
		/// </param>
		/// <returns>
		///  <para>Returns a pointer to the indexed audio buffer.</para>
		///  <para>Returns null if the given index was out of bounds.</para>
		/// </returns>
		AudioBuffer* GetByIndex(int index);

		/// <summary>Gets the number of audio buffers stored within this manager.</summary>
		/// <returns>
		///  <para>Returns the number of audio buffers stored within this manager.</para>
		///  <para>Returns zero if no audio buffers are available.</para>
		/// </returns>
		int GetCount();

	private:
		/// <summary>Defines a hash table for storing audio buffers, using their unique integer IDs as the key.</summary>
		typedef std::hash_map<ALuint, AudioBuffer*> CollectionType;

		/// <summary>
		///  <para>Defines the dictionary "pair" used by this manager's "CollectionType".</para>
		///  <para>The first item in the pair is the audio buffer's unique integer ID.</para>
		///  <para>The second item is a pointer to the audio buffer assigned that unique integer ID.</para>
		/// </summary>
		typedef std::pair<ALuint, AudioBuffer*> CollectionPairType;

		/// <summary>Copy constructor made private to force new instances to be made.</summary>
		AudioBufferManager(const AudioBufferManager &manager);

		/// <summary>Copy operation made private to force new instances to be made.</summary>
		void operator=(const AudioBufferManager &manager);

		/// <summary>Stores a hash table of audio buffers which uses their unique integer IDs as the key.</summary>
		CollectionType fBufferCollection;

		/// <summary>Used to quickly assign a unique integer ID to an audio buffer by the Create() method.</summary>
		ALuint fNextBufferId;
};

}	// namespace AL
