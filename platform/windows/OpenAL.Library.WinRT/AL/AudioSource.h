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
#include "AudioBufferQueue.h"
#include "AudioFormatInfo.h"
#include "AudioPlaybackPosition.h"
#include "AudioPlaybackState.h"
#include "Position.h"
#include "Vector.h"
#include <mutex>
#include <xaudio2.h>


namespace AL {

class AudioContext;
class XAudio2VoiceFactory;

/// <summary>Represents an audio source in 3D space that plays audio that can be heard by one listener.</summary>
class AudioSource
{
	public:
		#pragma region Public Constants
		/// <summary>Represents an invalid audio source ID.</summary>
		static const ALuint kInvalidId;

		#pragma endregion


		#pragma region CreationSettings Structure
		/// <summary>Settings to be passed into an AudioSource class' constructor.</summary>
		struct CreationSettings
		{
			/// <summary>Creates a new settings object.</summary>
			CreationSettings()
			: IntegerId(AudioSource::kInvalidId), ContextPointer(nullptr), VoiceFactoryPointer(nullptr)
			{ }

			/// <summary>Unique integer ID to be assigned to the audio source.</summary>
			ALuint IntegerId;

			/// <summary>Pointer to the audio context that will own the audio source.</summary>
			AudioContext *ContextPointer;

			/// <summary>Pointer to a factory needed to create an audio source's XAudio2 voice object.</summary>
			XAudio2VoiceFactory *VoiceFactoryPointer;
		};
		#pragma endregion


		#pragma region StreamAudioController Class
		/// <summary>
		///  <para>Provides access to an audio source's streaming audio queue.</para>
		///  <para>
		///   Allows external code to push audio buffers to be streamed out next and pop buffers to be re-used for
		///   the next set of streamed audio data.
		///  </para>
		///  <para>Instances of this class are returned by the AudioSource.GetStreamingAudioController() function.</para>
		/// </summary>
		class StreamingAudioController
		{
			public:
				/// <summary>
				///  <para>Creates an invalid streaming audio controller object.</para>
				///  <para>The controller's push and pop method will do nothing when not referencing an audio source.</para>
				/// </summary>
				StreamingAudioController();

				/// <summary>Creates a new streaming audio controller for the given audio source.</summary>
				/// <param name="audioSourcePointer">
				///  <para>Pointer to the audio source to take control of streaming audio queue.</para>
				///  <para>
				///   Setting this to null will create an invalid controller object whose push and pop methods will do nothing.
				///  </para>
				/// </param>
				StreamingAudioController(AudioSource *audioSourcePointer);

				/// <summary>
				///  Pushes the given audio buffer to the audio source's stream, to be played once the previous
				///  pushed buffers have been played first.
				/// </summary>
				/// <param name="bufferPointer">Pointer to the audio buffer to be pushed to the end of the queue.</param>
				void PushBuffer(AudioBuffer *bufferPointer);

				/// <summary>
				///  Pops an audio buffer off from the queue that has already been processed/played by the audio system.
				/// </summary>
				/// <returns>
				///  <para>Returns a pointer to the audio buffer that was popped off.</para>
				///  <para>
				///   Returns null if the queue is empty or if none of the audio buffers have been fully processed/played yet.
				///  </para>
				/// </returns>
				AudioBuffer* PopProcessedBuffer();

				/// <summary>
				///  <para>Fetches the number of audio buffers that have already been processed/played by the audio system.</para>
				///  <para>
				///   The caller is expected to pop these buffers off, copy the next stream of audio data to them,
				///   and then push them back in to this audio source.
				///  </para>
				/// </summary>
				/// <returns>
				///  <para>Returns the number of audio buffers that were processed/played by the audio system.</para>
				///  <para>Returns zero if none of the pushed audio buffers have been process or played yet.</para>
				/// </returns>
				int GetProcessedBufferCount();

				/// <summary>
				///  <para>Fetches the number of all buffers in the queue.</para>
				///  <para>This includes unprocessed, currently being processed, and fully processed buffers.</para>
				/// </summary>
				/// <returns>
				///  Returns all of the buffers that have been pushed in, including processed and unprocessed buffers.
				/// </returns>
				int GetBufferCount();

			private:
				/// <summary>The audio source this streaming audio controller will interact with.</summary>
				AudioSource *fAudioSourcePointer;
		};

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new audio source used to play audio in 3D space.</summary>
		/// <param name="settings">
		///  Assigns the audio source its unique integer ID, audio context which owns it,
		///  and a factory needed to create its XAudio2 voice object.
		/// </param>
		AudioSource(const AudioSource::CreationSettings &settings);

		/// <summary>Disposes of this source's resources.</summary>
		virtual ~AudioSource();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Gets the unique integer ID assigned to this audio source.</summary>
		/// <returns>Returns this audio source's unique integer ID.</returns>
		ALuint GetIntegerId();

		/// <summary>Gets the audio context that owns this audio source.</summary>
		/// <returns>
		///  <para>Returns a pointer to the audio context that owns this audio source.</para>
		///  <para>Returns null if the context was not given to the audio source upon construction.</para>
		/// </returns>
		AudioContext* GetContext();

		/// <summary>Gets the volume level applied to audio playback.</summary>
		/// <returns>Returns volume level as a scale where 1.0 means normal volume level and 0.0 means silence/mute.</returns>
		float GetVolumeScale();

		/// <summary>Sets the volume level to be applied to audio output.</summary>
		/// <param name="value">The volume level in scale form. Set to 1.0 for normal volume level. Set to 0.0 to mute.</param>
		void SetVolumeScale(float value);

		/// <summary>
		///  <para>Gets the minimum volume level to be applied to audio output.</para>
		///  <para>This minimum is set to 0.0 by default, but can be changed via the SetMinVolumeScale() function.</para>
		/// </summary>
		/// <returns>
		///  Returns the minimum volume level in scale form where 1.0 means normal volume level and 0.0 means silence/mute.
		/// </returns>
		float GetMinVolumeScale();

		/// <summary>
		///  <para>Sets the minimum volume level to be applied to audio output.</para>
		///  <para>
		///   This can be used to prevent audio output from being attenuated (due to distance) below a minimu
		///   volume level so that it can always be heard by the listener.
		///  </para>
		/// </summary>
		/// <param name="value">The volume level in scale form. 0.0 means mute and 1.0 means normal volume level.</param>
		void SetMinVolumeScale(float value);

		/// <summary>
		///  <para>Gets the maximum volume level to be applied to audio output.</para>
		///  <para>This maximum is set to 1.0 by default, but can be changed via the SetMaxVolumeScale() function.</para>
		/// </summary>
		/// <returns>
		///  Returns the maximum volume level in scale form where 1.0 means normal volume level and 0.0 means silence/mute.
		/// </returns>
		float GetMaxVolumeScale();

		/// <summary>Sets the maximum volume level to be applied to audio output.</summary>
		/// <param name="value">The volume level in scale form. 0.0 means mute and 1.0 means normal volume level.</param>
		void SetMaxVolumeScale(float value);

		/// <summary>Gets the pitch shift to be applied to audio output.</summary>
		/// <returns>
		///  <para>Returns the pitch shift in scale form.</para>
		///  <para>A value of 1.0 means no pitch shift is applied.</para>
		///  <para>A value of 0.5 reduces the pitch by 1 octave.</para>
		///  <para>A value of 2.0 increases the pitch by 1 octave.</para>
		/// </returns>
		float GetPitchShiftScale();

		/// <summary>Sets the pitch shift to be applied to audio output.</summary>
		/// <param name="value">
		///  <para>The pitch shift in scale form.</para>
		///  <para>Setting this to 1.0 means no pitch shift will be applied.</para>
		///  <para>
		///   Each reduction of 50% reduces the pitch by 1 octave. For example, 0.5 lowers the pitch by 1 octave
		///   and 0.25 lowers it by 2 octaves.
		///  </para>
		///  <para>
		///   Each doubling of this scale increases the pitch by 1 octave. For example, 2.0 increases the pitch by 1 octave
		///   and 4.0 increases it by 2 octaves.
		///  </para>
		/// </param>
		void SetPitchShiftScale(float value);

		/// <summary>
		///  <para>Gets the audio source's current position in 3D space.</para>
		///  <para>
		///   This is used to apply positional audio where audio sources near the listener will be louder than sources
		///   that are farther away.
		///  </para>
		/// </summary>
		/// <returns>Returns a copy of the audio source's position.</returns>
		Position GetPosition();

		/// <summary>
		///  <para>Sets the audio source's position in 3D space.</para>
		///  <para>
		///   This is used to apply positional audio where audio sources near the listener will be louder than sources
		///   that are farther away.
		///  </para>
		/// </summary>
		/// <param name="value">The position to set the audio source to.</param>
		void SetPosition(const Position &value);

		/// <summary>
		///  <para>Gets the current velocity of the audio source in 3D space.</para>
		///  <para>The velocity is only used to apply doppler effects and is not used to re-position this object over time.</para>
		/// </summary>
		/// <returns>Returns velocity as a vector.</returns>
		Vector GetVelocity();

		/// <summary>
		///  <para>Sets the current velocity of the audio source in 3D space.</para>
		///  <para>The velocity is only used to apply doppler effects and is not used to re-position this object over time.</para>
		/// </summary>
		/// <param name="value">The velocity vector to set the audio source to.</param>
		void SetVelocity(const Vector &value);

		/// <summary>
		///  <para>Determines if a static audio buffer will be looped or not.</para>
		///  <para>This setting is not applicable if this audio source is set up for streaming.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns true if this audio source is set up to loop when playing a static audio buffer.</para>
		///  <para>
		///   Returns false if this source will put itself in the "kStopped" playback state when reaching the end
		///   of a static audio buffer.
		///  </para>
		/// </returns>
		bool IsStaticBufferLoopingEnabled();

		/// <summary>
		///  <para>Enables or disables looping of a static audio buffer.</para>
		///  <para>This setting is not applicable if this audio source is set up for streaming.</para>
		/// </summary>
		/// <param name="value">
		///  <para>Set to true to have this audio source loop the playback of a static audio buffer.</para>
		///  <para>
		///   Set to false to stop playback when reaching the end of the static audio buffer,
		///   putting this source into the "kStopped" playback state.
		///  </para>
		/// </param>
		void SetIsStaticBufferLoopingEnabled(bool value);

		/// <summary>Gets the audio source's current playback state such as kPlaying, kPaused, etc.</summary>
		/// <returns>Returns kReseted, kPlaying, kPaused, or kStopped.</returns>
		AudioPlaybackState GetPlaybackState();

		/// <summary>Gets the current playback position in the audio track.</summary>
		/// <param name="units">
		///  <para>The units to get the playback position in such as:</para>
		///  <para>
		///   AudioPlaybackPosition::Units::kBytes, AudioPlaybackPosition::Units::kSeconds,
		///   or AudioPlaybackPosition::Units::kSamples.
		///  </para>
		/// </param>
		/// <returns>
		///  <para>Returns the current playback position in the audio track in the given units.</para>
		///  <para>Returns a position value of zero if the playback state is stopped or reseted/rewound.</para>
		/// </returns>
		AudioPlaybackPosition GetPlaybackPositionIn(AudioPlaybackPosition::Units units);

		/// <summary>
		///  <para>Sets the position in the audio track to play from.</para>
		///  <para>
		///   If this source is currently playing audio, then it'll immediately "seek" to the given position
		///   and play from there.
		///  </para>
		/// </summary>
		/// <param name="position">The audio track position to start playing from.</param>
		/// <returns>
		///  <para>Returns true if the given position was valid and accepted.</para>
		///  <para>
		///   Returns false if the given position exceeds the bounds/length of the audio buffer that is currently being played.
		///   Will never return false if no audio buffer was provided yet to the audio source yet.
		///  </para>
		/// </returns>
		bool SetPlaybackPosition(AudioPlaybackPosition position);

		/// <summary>Determines if this audio source is streaming audio or is using one static audio buffer.</summary>
		/// <returns>
		///  <para>Returns true if this audio source is set up for streaming audio.</para>
		///  <para>Returns false if this audio source is only using one static audio buffer.</para>
		/// </returns>
		bool IsStreaming();

		/// <summary>Determines if this audio source is streaming audio or is using one static audio buffer.</summary>
		/// <returns>
		///  <para>Returns true if this audio source is only using one static audio buffer.</para>
		///  <para>Returns false if this audio source is set up for streaming audio.</para>
		/// </returns>
		bool IsNotStreaming();

		/// <summary>Sets up this audio source for streaming audio or for one static audio buffer.</summary>
		/// <param name="value">
		///  <para>
		///   Set true to set up this audio source for streaming audio. In this case, it is up to the caller to submit
		///   audio buffers regularly to this source's StreamingAudioController returned by the GetStreamingBuffers() function.
		///  </para>
		///  <para>
		///   Set false to use one static audio buffer which contains the entire audio data to be played.
		///   This audio buffer is assigned to this source via the SetStaticBuffer() function.
		///  </para>
		/// </param>
		void SetIsStreaming(bool value);

		/// <summary>
		///  <para>Assigns this audio source one static buffer that contains the entire audio data to be played.</para>
		///  <para>The given audio buffer will only be used if this audio source is not set up for streaming audio.</para>
		/// </summary>
		/// <param name="bufferPointer">
		///  <para>Pointer to the audio buffer providing the audio data to be played by this source.</para>
		///  <para>This buffer is expected to have come from the audio context's buffer manager.</para>
		///  <para>Can be set to null, which would cause no audio to be played.</para>
		/// </param>
		void SetStaticBuffer(AudioBuffer *bufferPointer);

		/// <summary>Gets the audio buffer assigned to this audio source via the SetStaticBuffer() function.</summary>
		/// <returns>
		///  <para>Returns a pointer to the one audio buffer this source will play if not set up for streaming audio.</para>
		///  <para>Returns null if a static audio buffer has not been assigned to this source.</para>
		/// </returns>
		AudioBuffer* GetStaticBuffer();

		/// <summary>
		///  <para>Gets a controller used to push/pop streaming audio buffers to/from this audio source.</para>
		///  <para>The buffer pushed in will only be used if streaming audio is enabled via the SetIsStreaming() function.</para>
		/// </summary>
		/// <returns>
		///  Returns this audio source's stream audio controller used to push new audio buffers to the playback queue
		///  and pop off buffers that have been process by the audio system.
		/// </returns>
		StreamingAudioController& GetStreamingBuffers();

		void Play();

		void Pause();

		void Stop();

		void StopAndRewind();

		#pragma endregion

	private:
		#pragma region Private Methods
		void SubmitVolumeSettings();

		void SubmitPitchSettings();

		void SubmitAudioBuffers();

		#pragma endregion


#if 0
		class AudioBufferEventHandler
		{
			public:
				AudioBufferEventHandler();
				AudioBufferEventHandler(AudioSource *audioSourcePointer);

				void SubscribeTo(AudioBuffer *bufferPointer);
				void UnsubscribeFrom(AudioBuffer *bufferPointer);

			private:
				void OnUpdating(const AudioBuffer &buffer, const EventArgs &arguments);
				void OnUpdated(const AudioBuffer &buffer, const EventArgs &arguments);
				void OnDestroying(const AudioBuffer &buffer, const EventArgs &arguments);

				AudioBuffer::UpdatingEvent::MethodHandler<AudioBufferEventHandler> fUpdatingEventHandler;
				AudioBuffer::UpdatedEvent::MethodHandler<AudioBufferEventHandler> fUpdatedEventHandler;
				AudioBuffer::DestroyingEvent::MethodHandler<AudioBufferEventHandler> fDestroyingEventHandler;

				AudioSource *fAudioSourcePointer;
		};
#endif


		#pragma region VoiceEventHandler Class
		/// <summary>Private class used by the AudioSource class to handle XAudio2 voice events.</summary>
		class VoiceEventHandler : public IXAudio2VoiceCallback
		{
			public:
				/// <summary>Creates an invalid XAudio2 voice event handler that is not tied to an audio source.</summary>
				VoiceEventHandler();

				/// <summary>Creates an XAudio2 voice event handler for the given audio source.</summary>
				/// <param name="audioSourcePointer">Audio source that will own this event handler.</param>
				VoiceEventHandler(AudioSource *audioSourcePointer);

				/// <summary>Called when the XAudio2 system is just about to read an audio buffer.</summary>
				/// <param name="bufferContextPointer">
				///  Custom data provided by the audio source when the buffer was submitted to the XAudio2 voice.
				/// </param>
				virtual void STDMETHODCALLTYPE OnBufferStart(void *bufferContextPointer);

				/// <summary>
				///  <para>Called when the XAudio2 system has just finished processing an audio buffer.</para>
				///  <para>It is safe to delete the audio buffer at this point.</para>
				/// </summary>
				/// <param name="bufferContextPointer">
				///  Custom data provided by the audio source when the buffer was submitted to the XAudio2 voice.
				/// </param>
				virtual void STDMETHODCALLTYPE OnBufferEnd(void *bufferContextPointer);

				/// <summary>Called just before the XAudio2 system is about to loop the audio track.</summary>
				/// <param name="bufferContextPointer">
				///  Custom data provided by the audio source when the buffer was submitted to the XAudio2 voice.
				/// </param>
				virtual void STDMETHODCALLTYPE OnLoopEnd(void *bufferContextPointer);

				/// <summary>
				///  <para>Called when the XAudio2 system has just finished processing all audio buffers submitted to it.</para>
				///  <para>Audio stops playing at this point because there is no more audio buffers to play.</para>
				/// </summary>
				virtual void STDMETHODCALLTYPE OnStreamEnd();

				/// <summary>
				///  <para>Called just before the XAudio2 system attempts to read/process all audio buffers submitted to it.</para>
				///  <para>This event callback will be called before the OnBufferStart() callback.</para>
				/// </summary>
				/// <param name="bytesRequired">
				///  <para>The number of bytes that must be submitted to the XAudio2 system to prevent starvation.</para>
				///  <para>Only applicable when streaming audio buffers to the voice to prevent an audio skipping effect.</para>
				/// </param>
				virtual void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 bytesRequired);

				/// <summary>Called by the XAudio2 system after it completes a processing pass.</summary>
				virtual void STDMETHODCALLTYPE OnVoiceProcessingPassEnd();

				/// <summary>Called when the XAudio2 system encounters an error when attempting to process audio.</summary>
				/// <param name="bufferContextPointer">
				///  Custom data provided by the audio source when the buffer was submitted to the XAudio2 voice.
				/// </param>
				/// <param name="error">An HRESULT code indicating the error that was encountered.</param>
				virtual void STDMETHODCALLTYPE OnVoiceError(void *bufferContextPointer, HRESULT error);

			private:
				/// <summary>
				///  Pointer to the audio source that owns this event handler and the XAudio2 voice that is
				///  generating the events.
				/// </summary>
				AudioSource *fAudioSourcePointer;
		};

		#pragma endregion


		#pragma region StreamingAudioData Structure
		/// <summary>Private structure used to store buffers used for streaming audio.</summary>
		struct StreamingAudioData
		{
			StreamingAudioData()
			: UnprocessedSamplePosition(0)
			{ }

			/// <summary>Queue of audio buffers that have not been submitted to the audio system yet.</summary>
			AudioBufferQueue UnprocessedBuffers;

			/// <summary>
			///  <para>Queue of audio buffers that are currently being used/read by the XAudio2 system.</para>
			///  <para>These buffers cannot be deleted until the XAudio2 system is done with them.</para>
			/// </summary>
			AudioBufferQueue ActiveBuffers;

			/// <summary>
			///  <para>Queue of buffers that have already been submitted and processed by the XAudio2 system.</para>
			///  <para>These are expected to be popped off and either deleted or re-used for the next set of audio data.</para>
			/// </summary>
			AudioBufferQueue ProcessedBuffers;

			/// <summary>
			///  <para>
			///   References the first audio sample/frame belonging to the next "unprocessed" audio buffer in the
			///   UnprocessedBuffers queue to be submitted to the audio system.
			///  </para>
			///  <para>
			///   This position accumalates over time and is relative to first sample submitted since playback started.
			///  </para>
			///  <para>This position is used to "seek" to a particular point in a streaming audio track.</para>
			/// </summary>
			uint64 UnprocessedSamplePosition;
		};
		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Unique integer ID assigned to this audio source.</summary>
		ALuint fIntegerId;

		/// <summary>The audio context that owns this audio source.</summary>
		AudioContext *fContextPointer;

		/// <summary>
		///  <para>The volume level to apply to audio playback in scale form.</para>
		///  <para>A scale of 1.0 means normal volume level and 0.0 means silence/mute.</para>
		/// </summary>
		float fVolumeScale;

		/// <summary>The minimum volume scale that can be applied when outputting audio.</summary>
		float fMinVolumeScale;

		/// <summary>The maximum volume scale that can be applied when outputting audio.</summary>
		float fMaxVolumeScale;

		/// <summary>Scale used to pitch shift audio playback. A value of 1.0 means no pitch shift.</summary>
		float fPitchShiftScale;

		/// <summary>The audio source's current position in 3D space.</summary>
		Position fPosition;

		/// <summary>The audio source's current velocity used to apply doppler effects.</summary>
		Vector fVelocity;

		/// <summary>Set true to enable looping a static audio buffer. Set false to not loop.</summary>
		bool fIsStaticBufferLoopingEnabled;

		/// <summary>The current playback state such as kPlaying, kStopped, etc.</summary>
		AudioPlaybackState fPlaybackState;

		/// <summary>The position in the audio buffer to start playing audio from.</summary>
		AudioPlaybackPosition fPlaybackStartPosition;

		/// <summary>
		///  <para>Set true if the XAudio2 voice has been started and audio buffers have been submitted.</para>
		///  <para>
		///   Should remain true even if this audio source object has been paused because the voice is still
		///   referencing audio buffers.
		///  </para>
		///  <para>
		///   Must be set false if the Stop() function has been called to make this object stop submitted buffers
		///   to the XAudio2 voice.
		///  </para>
		/// </summary>
		bool fIsXAudioVoiceActive;

		/// <summary>
		///  <para>Set true if streaming audio via the queue buffers in the "fStreamingAudioData" member variable.</para>
		///  <para>
		///   Set false if the entire audio data was loaded into one buffer referenced by the "fStaticAudioBufferPointer"
		///   member variable
		///  </para>
		/// </summary>
		bool fIsStreaming;

		/// <summary>
		///  Pointer to the audio buffer to play when not streaming audio via the "fStreamingAudioData" member.
		/// </summary>
		AudioBuffer* fStaticAudioBufferPointer;

		/// <summary>Stores the unprocessed, active, and processed buffers needed for streaming audio.</summary>
		StreamingAudioData fStreamingAudioData;

		/// <summary>Public interface which allows external code to push and pop audio buffers used for streaming.</summary>
		StreamingAudioController fStreamingAudioController;

		/// <summary>The audio format currently being used by the "fSourceVoicePointer" XAudio2 voice.</summary>
		AudioFormatInfo fCurrentAudioFormat;

		/// <summary>The XAudio2 voice that this audio source wraps.</summary>
		IXAudio2SourceVoice *fXAudioVoicePointer;

		/// <summary>Factory needed to create the XAudio2 voice that this audio source wraps.</summary>
		XAudio2VoiceFactory *fVoiceFactoryPointer;

		/// <summary>Handles events received from an XAudio2 voice.</summary>
		VoiceEventHandler fVoiceEventHandler;

		/// <summary>Mutex used to synchronize access to this source's audio buffers between threads.</summary>
		std::recursive_mutex fMutex;

		#pragma endregion
};

}	// namespace AL
