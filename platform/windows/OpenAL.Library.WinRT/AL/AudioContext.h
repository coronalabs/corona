//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AudioBufferManager.h"
#include "AudioListener.h"
#include "AudioSourceManager.h"
#include "Event.h"
#include "EventArgs.h"
#include <xaudio2.h>


namespace AL {

class XAudio2VoiceFactory;

class AudioContext
{
	private:
		#pragma region Constructors/Destructors
		/// <summary>Creates a new audio context.</summary>
		/// <remarks>Constructor made private to force external code to call the static Create() function instead.</remarks>
		/// <param name="xaudioEnginePointer">
		///  <para>Pointer to the XAudio2 engine.</para>
		///  <para>Cannot be null or else a crash will occur.</para>
		///  <para>
		///   The new AudioContext object will not delete the given XAudio2 object in its destructor.
		///   That is the caller's responsibility.
		///  </para>
		/// </param>
		/// <param name="xaudioMasteringVoicePointer">
		///  <para>Pointer to an XAudio2 master voice created via the given "xaudioEnginePointer" argument.</para>
		///  <para>Cannot be null or else a crash will occur.</para>
		///  <para>
		///   The new AudioContext object will not delete the given XAudio2 object in its destructor.
		///   That is the caller's responsibility.
		///  </para>
		/// </param>
		AudioContext(IXAudio2 *xaudioEnginePointer, IXAudio2MasteringVoice *xaudioMasteringVoicePointer);

		/// <summary>Copy constructor made private to prevent it from being called.</summary>
		AudioContext(const AudioContext&);

		/// <summary>Destroys this context, its audio sources, and its audio buffers.</summary>
		/// <remarks>Destructor made private to force external code to call the static Destroy() function instead.</remarks>
		virtual ~AudioContext();

		#pragma endregion

	public:
		#pragma region Event Definitions
		/// <summary>
		///  Defines the "Created" event which is raised when an audio context has been created via its constructor.
		/// </summary>
		typedef Event<AudioContext&, const EventArgs&> CreatedEvent;

		/// <summary>
		///  Defines the "Destroying" event type which is raised when this audio context object is about to be
		///  destroyed via its destructor.
		/// </summary>
		typedef Event<AudioContext&, const EventArgs&> DestroyingEvent;

		#pragma endregion


		#pragma region CreationSettings Structure
		/// <summary>Settings to be passed into the AudioContext class' static Create() function.</summary>
		struct CreationSettings
		{
			/// <summary>Creates a new settings object.</summary>
			CreationSettings()
			: OutputSampleRate(XAUDIO2_DEFAULT_SAMPLERATE)
			{}

			/// <summary>
			///  <para>The sample rate in Hertz that all of audio sources should be mixed and outputted as, suchas 44100 Hz.</para>
			///  <para>
			///   Should be set to the sample rate that the majority of the audio sources will be playing to
			///   reduce the time to mix the audio to a single stream.
			///  </para>
			///  <para>Defaults to "XAUDIO2_DEFAULT_SAMPLERATE" which is the platform/device's default sample rate.</para>
			/// </summary>
			int OutputSampleRate;
		};

		#pragma endregion


		#pragma region CreationResult Class
		/// <summary>
		///  <para>Instances of this class are returned by the AudioContext::Create() static function.</para>
		///  <para>Provides an AudioContext object if creation was successful.</para>
		///  <para>Provides an HRESULT error code if failed to create the AudioContext object.</para>
		/// </summary>
		class CreationResult
		{
			private:
				/// <summary>Creates a result object with the given information.</summary>
				/// <remarks>
				///  Constructor made private to force external code to use this class' static SucceededWith()
				///  and FailedWith() functions.
				/// </remarks>
				/// <param name="contextPointer">
				///  <para>Pointer to the AudioContext that was created if successful.</para>
				///  <para>Set to null if creation failed.</para>
				/// </param>
				/// <param name="resultCode">
				///  <para>Set to the error code returned by the XAudio2Create() function.</para>
				///  <para>Expected to be S_OK if successful or an "E_*" error code if failed.</para>
				/// </param>
				CreationResult(AudioContext *contextPointer, HRESULT resultCode);

			public:
				/// <summary>Determines if an audio context was successfully created.</summary>
				/// <returns>
				///  <para>
				///   Returns true if an audio context was successfully created which can be retrieved via
				///   the GetContext() method.
				///  </para>
				///  <para>Returns false if creation failed, in which case the GetHRESULT() function will indicate why.</para>
				/// </returns>
				bool HasSucceeded();

				/// <summary>Determines if an audio context was not successfully created.</summary>
				/// <returns>
				///  <para>Returns true if creation failed, in which case the GetHRESULT() function will indicate why.</para>
				///  <para>
				///   Returns false if an audio context was successfully created which can be retrieved via
				///   the GetContext() method.
				///  </para>
				/// </returns>
				bool HasFailed();

				/// <summary>Gets a pointer to the audio context if successfully created.</summary>
				/// <returns>
				///  <para>Returns a pointer to the new audio context if successfully created.</para>
				///  <para>Returns null if creation failed, in which case GetHRESULT() should provide details as to why.</para>
				/// </returns>
				AudioContext* GetContext();

				/// <summary>Gets the success/error code </summary>
				/// <returns>
				///  <para>Returns S_OK if successfully created an audio context.</para>
				///  <para>Returns an "E_*" error code if creation failed.</para>
				/// </returns>
				HRESULT GetHRESULT();

				/// <summary>Creates a success result storing a pointer to the newly created audio context.</summary>
				/// <param name="contextPointer">
				///  <para>Pointer to the newly created audio context.</para>
				///  <para>Cannot be null or else a "std::invalid_argument" exception will be thrown.</para>
				/// </param>
				/// <returns>Returns a success result object storing the given pointer.</returns>
				static CreationResult SucceededWith(AudioContext *contextPointer);

				/// <summary>Creates a failure result object storing the given HRESULT error code.</summary>
				/// <param name="errorCode">
				///  <para>
				///   An HRESULT "E_*" error code returned by the XAudio2Create() function indicating why creation failed.
				///  </para>
				///  <para>Cannot be set to S_OK or else a "std::invalid_argument" exception will be thrown.</para>
				/// </param>
				static CreationResult FailedWith(HRESULT errorCode);

			private:
				/// <summary>Pointer to the audio context that was created or null if creation failed.</summary>
				AudioContext *fContextPointer;

				/// <summary>Set to S_OK if creation was successful or the "E_*" error code indicating why it failed.</summary>
				HRESULT fResultCode;
		};

		#pragma endregion


		#pragma region Public Methods
		AudioListener& GetListener();

		AudioBufferManager& GetBufferManager();

		AudioSourceManager& GetSourceManager();

		std::recursive_mutex& GetMutex();

		void Suspend();

		void Resume();

		#pragma endregion


		#pragma region Public Static Functions
		static CreationResult Create(const AudioContext::CreationSettings &settings);

		static void Destroy(AudioContext *contextPointer);

		/// <summary>Gets an object used to add or remove an event handler for the "Created" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		static CreatedEvent::HandlerManager& GetCreatedEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "Destroying" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		static DestroyingEvent::HandlerManager& GetDestroyingEventHandlers();

		#pragma endregion

	private:
		#pragma region Private Member Variables
		/// <summary>Pointer to the XAudio2 audio engine which manages everything audio related.</summary>
		IXAudio2 *fXAudioEnginePointer;

		/// <summary>
		///  <para>Pointer to the XAudio2 mastering voice which is the equivalent to OpenAL's listener.</para>
		///  <para>Member variable "fAudioListener" wraps this pointer.</para>
		/// </summary>
		IXAudio2MasteringVoice *fXAudioMasteringVoicePointer;

		/// <summary>
		///  <para>Listener in 3D space which "hears" the audio outputted from audio sources.</para>
		///  <para>This object wraps the XAudio2 mastering voice.</para>
		/// </summary>
		AudioListener *fListenerPointer;

		/// <summary>
		///  Creates and stores a collection of audio buffers which store raw PCM audio data that audio sources
		///  can use to play sound.
		/// </summary>
		AudioBufferManager fBufferManager;

		/// <summary>Creates and stores a collection of audio sources that output sound in 3D space.</summary>
		AudioSourceManager *fSourceManagerPointer;

		/// <summary>Manages the "Created" event.</summary>
		static CreatedEvent sCreatedEvent;

		/// <summary>Manages the "Destroying" event.</summary>
		static DestroyingEvent sDestroyingEvent;

		#pragma endregion
};

}	// namespace AL
