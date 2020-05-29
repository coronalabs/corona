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
#include "XAudio2VoiceFactory.h"
#include <hash_map>


namespace AL {

class AudioContext;
class AudioSource;

/// <summary>Creates and stores a collection of audio sources for one audio context.</summary>
class AudioSourceManager
{
	public:
		#pragma region CreationSettings Structure
		/// <summary>Settings to be passed into an AudioSourceManager class' constructor.</summary>
		struct CreationSettings
		{
			/// <summary>Creates a new settings object.</summary>
			CreationSettings()
			: ContextPointer(nullptr)
			{ }

			/// <summary>Pointer to the audio context that owns the audio source manager.</summary>
			AudioContext *ContextPointer;

			/// <summary>Factory needed to create an audio source's XAudio2 voice object.</summary>
			XAudio2VoiceFactory VoiceFactory;
		};

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new audio source manager.</summary>
		AudioSourceManager();

		/// <summary>Creates a new audio source manager.</summary>
		/// <param name="settings">
		///  Provides the audio context which will own the new manager and a factory needed to create
		///  the XAudio2 voice objects.
		/// </param>
		AudioSourceManager(const AudioSourceManager::CreationSettings &settings);

		/// <summary>Destroys this manager along with all of the audio sources it created.</summary>
		virtual ~AudioSourceManager();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Creates a new audio source and stores it in its collection.</summary>
		/// <returns>
		///  Returns a new empty audio souce assigned a unique integer ID amongst all other sources in the collection.
		/// </returns>
		AudioSource* Create();

		/// <summary>Destroys all audio sources created by this manager.</summary>
		void DestroyAll();

		/// <summary>Destroys an audio source in this manager's collection by its unique integer ID.</summary>
		/// <param name="id">Unique integer ID assigned to the audio source by this manager.</param>
		/// <returns>
		///  <para>Returns true if the specified audio source was successfully deleted.</para>
		///  <para>Returns false if the given ID was not found in this manager's collection.</para>
		/// </returns>
		bool DestroyById(ALuint id);

		/// <summary>Determines if an audio source having the given ID exists in this manager's collection.</summary>
		/// <param name="id">Unique integer ID of the audio source to search for.</param>
		/// <returns>
		///  <para>Returns true if an audio source having the given ID exists in the manager's collection.</para>
		///  <para>Returns false if the given ID was not found.</para>
		/// </returns>
		bool ContainsId(ALuint id);

		/// <summary>Fetches an audio source that was created by this manager via its unique integer ID.</summary>
		/// <param name="id">Unique integer ID of the audio source to fetch.</param>
		/// <returns>
		///  <para>Returns a pointer to the specified audio source.</para>
		///  <para>Returns null if the given ID was not found in this manager's audio source collection.</para>
		/// </returns>
		AudioSource* GetById(ALuint id);

		/// <summary>Fetches an audio source contained in this manager's collection by its zero based index.</summary>
		/// <param name="index">
		///  <para>Zero based index of the audio source to fetch.</para>
		///  <para>Must be greater than or equal to zero and less than GetCount().</para>
		/// </param>
		/// <returns>
		///  <para>Returns a pointer to the indexed audio source.</para>
		///  <para>Returns null if the given index was out of bounds.</para>
		/// </returns>
		AudioSource* GetByIndex(int index);

		/// <summary>Gets the number of audio sources stored within this manager.</summary>
		/// <returns>
		///  <para>Returns the number of audio sources stored within this manager.</para>
		///  <para>Returns zero if no audio sources are available.</para>
		/// </returns>
		int GetCount();

		/// <summary>Gets the audio context that owns this audio source manager.</summary>
		/// <returns>
		///  <para>Returns a pointer to the audio context that owns this manager.</para>
		///  <para>
		///   Returns null if this manager was not given an audio context upon construction.
		///   This manager's Create() method will always fail in this case.
		///  </para>
		/// </returns>
		AudioContext* GetContext();

		#pragma endregion

	private:
		#pragma region Private Definitions
		/// <summary>Defines a hash table for storing audio sources, using their unique integer IDs as the key.</summary>
		typedef std::hash_map<ALuint, AudioSource*> CollectionType;

		/// <summary>
		///  <para>Defines the dictionary "pair" used by this manager's "CollectionType".</para>
		///  <para>The first item in the pair is the audio source's unique integer ID.</para>
		///  <para>The second item is a pointer to the audio source assigned that unique integer ID.</para>
		/// </summary>
		typedef std::pair<ALuint, AudioSource*> CollectionPairType;

		#pragma endregion


		#pragma region Private Methods
		/// <summary>Copy constructor made private to force new instances to be made.</summary>
		AudioSourceManager(const AudioSourceManager &manager);

		/// <summary>Copy operation made private to force new instances to be made.</summary>
		void operator=(const AudioSourceManager &manager);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Stores a hash table of audio sources which uses their unique integer IDs as the key.</summary>
		CollectionType fSourceCollection;

		/// <summary>Used to quickly assign a unique integer ID to an audio source by the Create() method.</summary>
		ALuint fNextSourceId;

		/// <summary>Pointer to the audio context that owns this manager.</summary>
		AudioContext *fContextPointer;

		/// <summary>Factory needed to create an audio source's XAudio2 voice object.</summary>
		XAudio2VoiceFactory fVoiceFactory;

		#pragma endregion
};

}	// namespace AL
