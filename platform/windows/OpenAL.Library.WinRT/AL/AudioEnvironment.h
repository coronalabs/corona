// ----------------------------------------------------------------------------
// 
// AudioEnvironment.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

#include "AudioContext.h"
#include "AudioContextCollection.h"
#include <mutex>


namespace AL {

class AudioContext;

/// <summary>
///  <para>Stores the current audio context and a mutex needed to synchronize access to it between threads.</para>
///  <para>This class is intended to be used by the OpenAL library's "al.h" and "alc.h" stateful APIs.</para>
///  <para>You cannot create instances of this class. You are expected to call its static functions instead.</para>
/// </summary>
class AudioEnvironment final
{
	private:
		// This gives the AudioContext class access to the private AddContextEventHandlers() function.
		friend class AudioContext;

		/// <summary>Constructor made private to prevent instances from being made.</summary>
		AudioEnvironment() { };

	public:
		#pragma region Public Functions
		/// <summary>
		///  <para>Fetches the current audio context last set via the SetCurrentContext() function.</para>
		///  <para>Intended to be called by stateful APIs such as the functions provided by "al.h".</para>
		///  <para>This is called by the alcGetCurrentContext() function.</para>
		///  <para>Warning: The caller should lock the mutex returned by GetMutex() before using the returned context.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns a pointer to the current audio context.</para>
		///  <para>Returns null if no audio context is currently available.</para>
		/// </returns>
		static AudioContext* GetCurrentContext();

		/// <summary>
		///  <para>Sets the current audio context, which will be returnd via the GetCurrentContext() function.</para>
		///  <para>This is called by the alcMakeContextCurrent() function.</para>
		/// </summary>
		/// <param name="contextPointer">
		///  <para>The audio context to be made current. Will be returned by this class' GetCurrentContext() function.</para>
		///  <para>Can be null, in which case, the stateful "al.h" APIs will not have an audio context to work with.</para>
		/// </param>
		/// <returns>
		///  <para>Returns true if the current context or null was successfully assigned to the system.</para>
		///  <para>Returns false if the given audio context pointer is no longer valid (ie: wild pointer).</returns>
		/// </returns>
		static bool SetCurrentContext(AudioContext *contextPointer);

		/// <summary>
		///  <para>Gets a mutex needed to synchronize access to the current audio context between threads.</para>
		///  <para>Only intended to be used by the "al.h" and "alc.h" APIs.</para>
		/// </summary>
		static std::recursive_mutex& GetMutex();

		#pragma endregion


		#pragma region ScopedMutexLock Class
		/// <summary>
		///  <para>Object whose constructor locks the AudioEnvironment's shared mutex and its destructor unlocks it.</para>
		///  <para>
		///   Works just like C++ std::lock_guard&lt;&gt; object, except it defaults to the AudioEnvironment's
		///   shared mutex to make it easier to use.
		///  </para>
		/// </summary>
		class ScopedMutexLock final
		{
			public:
				/// <summary>
				///  <para>Creates an object which immediately locks the AudioEnvironment class' shared mutex.</para>
				///  <para>The shared mutex will be unlocked via this object's destructor when the object falls out of scope.</para>
				/// </summary>
				ScopedMutexLock() : fMutexLock(AudioEnvironment::GetMutex()) {}

			private:
				/// <summary>Copy constructor made private to prevent it from being used.</summary>
				ScopedMutexLock(const ScopedMutexLock &) : fMutexLock(AudioEnvironment::GetMutex()) {}

				/// <summary>Copy operator made private to prevent it from being used.</summary>
				void operator=(const ScopedMutexLock &) {}

				/// <summary>Object used to automatically lock/unlock the mutex.</summary>
				std::lock_guard<std::recursive_mutex> fMutexLock;
		};

		#pragma endregion

	private:
		#pragma region Private Functions
		/// <summary>
		///  <para>Adds event handlers to the AudioContext class' static Created and Deleting events.</para>
		///  <para>
		///   This function is only expected to be called by the AudioContext class' constructor,
		///   which is a "friend" of this class.
		///  </para>
		/// </summary>
		static void AddContextEventHandlers();

		/// <summary>Called when a new audio context has been created.</summary>
		/// <param name="context">The audio context that was just created.</param>
		/// <param name="arguments">Empty event arguments.</param>
		static void OnCreatedContext(AudioContext &context, const EventArgs &arguments);

		/// <summary>Called when an audio context is about to be destroyed.</summary>
		/// <param name="context">The audio context that is about to be destroyed.</param>
		/// <param name="arguments">Empty event arguments.</param>
		static void OnDestroyingContext(AudioContext &context, const EventArgs &arguments);

		#pragma endregion


		#pragma region Private Static Variables
		/// <summary>
		///  <para>The currently selected context to be used by the stateful "al.h" APIs.</para>
		///  <para>Will be null if no audio contexts are available or currently selected.</para>
		/// </summary>
		static AudioContext *sCurrentContext;

		/// <summary>Stores a collection of all audio contexts that exist in the system.</summary>
		static AudioContextCollection sContextCollection;

		/// <summary>
		///  Mutex to be used by the stateful "al.h" and "alc.h" APIs to synchronize access with the current audio context.
		/// </summary>
		static std::recursive_mutex sMutex;

		/// <summary>
		///  Handler to be given to the AudioContext "Created" event to invoke the OnCreatedContext() function.
		/// </summary>
		static AudioContext::CreatedEvent::FunctionHandler sCreatedContextEventHandler;

		/// <summary>
		///  Handler to be given to the AudioContext "Destroying" event to invoke the OnDestroyingContext() function.
		/// </summary>
		static AudioContext::DestroyingEvent::FunctionHandler sDestroyingContextEventHandler;

		#pragma endregion
};

}	// namespace AL
