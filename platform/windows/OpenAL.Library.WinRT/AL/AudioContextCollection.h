// ----------------------------------------------------------------------------
// 
// AudioContextCollection.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

#include <list>


namespace AL {

class AudioContext;

/// <summary>Stores a collection of AudioContext objects.</summary>
class AudioContextCollection
{
	public:
		/// <summary>Creates an empty collection used to store audio context objects.</summary>
		AudioContextCollection();

		/// <summary>Destroys this collection.</summary>
		/// <remarks>
		///  This destructor will not destroy the audio context objects that were added to the collection
		///  That is the responsibility of the caller that added them to the collection.
		/// </remarks>
		virtual ~AudioContextCollection();

		/// <summary>Adds the given audio context pointer to the end of the collection.</summary>
		/// <param name="contextPointer">
		///  <para>Pointer to the audio context to be added to the collection.</para>
		///  <para>Will be ignored if given null.</para>
		/// </param>
		void Add(AudioContext *contextPointer);

		/// <summary>
		///  <para>Removes the given audio context from the collection by its memory address.</para>
		///  <para>If the specified audio context was pushed in multiple times, then they will all be removed.</para>
		///  <para>This function will not delete the audio context from memory. That is the caller's responsibility.</para>
		/// </summary>
		/// <param name="contextPointer">Memory address of the audio context to remove.</param>
		/// <returns>
		///  <para>Returns true if the specified audio context was removed from the collection.</para>
		///  <para>Returns false if the specified audio context was not found.</para>
		/// </returns>
		bool Remove(AudioContext *contextPointer);

		/// <summary>
		///  <para>Removes all audio contexts in the collection, making it empty.</para>
		///  <para>This function does not delete the audio contexts from memory. That is the caller's responsibility.</para>
		/// </summary>
		void Clear();

		/// <summary>Determine if the specified audio context exists in the collection.</summary>
		/// <param name="contextPointer">The audio collection to search for by memory address.</param>
		/// <returns>Returns true if the specified audio context was found in the collection. Returns false if not.</returns>
		bool Contains(AudioContext *contextPointer);

		/// <summary>Fetches an audio context from the collection by its zero based index.</summary>
		/// <param name="index">Zero based index to the audio context to fetch.</param>
		/// <returns>
		///  <para>Returns a pointer to the indexed audio context.</para>
		///  <para>Returns null if the given index is out of bounds.</para>
		/// </returns>
		AudioContext* GetByIndex(int index);

		/// <summary>Fetches the number of audio contexts stored in the collection.</summary>
		/// <returns>
		///  <para>Returns the number of audio contexts stored in the collection.</para>
		///  <para>Returns zero if the collection is currently empty.</para>
		/// </returns>
		int GetCount();

	private:
		/// <summary>Defines a collection type for storing audio context pointers.</summary>
		typedef std::list<AudioContext*> CollectionType;

		/// <summary>Stores a collection of audio context pointers.</summary>
		CollectionType fCollection;
};

}	// namespace AL
