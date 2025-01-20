//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Position.h"
#include "Vector.h"
#include <xaudio2.h>


namespace AL {

class AudioContext;

/// <summary>
///  <para>Represents an audio listener that hears playback from audio sources in 3D space.</para>
///  <para>The closer the listener is to an audio source, the louder the sound it emits.</para>
///  <para>This listener is anologous to the camera in OpenGL.</para>
///  <para>There can only be one listener per audio context.</para>
/// </summary>
class AudioListener
{
	public:
		#pragma region CreationSettings Structure
		/// <summary>Settings to be passed into an AudioListener class' constructor.</summary>
		struct CreationSettings
		{
			/// <summary>Creates a new settings object.</summary>
			CreationSettings()
			: ContextPointer(nullptr), XAudioMasteringVoicePointer(nullptr)
			{ }

			/// <summary>Pointer to the audio context that will own the audio listener.</summary>
			AudioContext *ContextPointer;

			/// <summary>Pointer to the XAudio2 mastering voice which is analogous to the OpenAL listener.</summary>
			IXAudio2MasteringVoice *XAudioMasteringVoicePointer;
		};
		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new listener used to listen to multiple audio sources in 3D space.</summary>
		/// <remarks>Only one listener can exist per audio context.</remarks>
		/// <param name="settings">
		///  Assigns the new listener the audio context that owns it and the XAudio2 master voice it will wrap.
		/// </param>
		AudioListener(const AudioListener::CreationSettings &settings);

		/// <summary>Disposes of this listener's resources.</summary>
		/// <remarks>
		///  Will not destroy the XAudio2 master voice that was given to this object's constructor.
		///  That is the responsibility of the creator of this object.
		/// </remarks>
		virtual ~AudioListener();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Gets the audio context that owns this audio listener.</summary>
		/// <returns>
		///  <para>Returns a pointer to the audio context that owns this audio listener.</para>
		///  <para>Returns null if the context was not given to the audio listener upon construction.</para>
		/// </returns>
		AudioContext* GetContext();

		/// <summary>Gets the volume level to be applied to the final mixed audio outputed to the device.</summary>
		/// <returns>Returns volume level as a scale where 1.0 means normal volume level and 0.0 means silence/mute.</returns>
		float GetVolumeScale();

		/// <summary>Sets the volume level to be applied to the final mixed audio outputted to the device.</summary>
		/// <param name="value">The volume level in scale form. Set to 1.0 for normal volume level. Set to 0.0 to mute.</param>
		void SetVolumeScale(float value);

		/// <summary>
		///  <para>Gets the listener's current position in 3D space.</para>
		///  <para>
		///   This is used to apply positional audio where audio sources near the listener will be louder than sources
		///   that are farther away.
		///  </para>
		/// </summary>
		/// <returns>Returns a copy of the listener's position.</returns>
		Position GetPosition();

		/// <summary>
		///  <para>Sets the listener's position in 3D space.</para>
		///  <para>
		///   This is used to apply positional audio where audio sources near the listener will be louder than sources
		///   that are farther away.
		///  </para>
		/// </summary>
		/// <param name="value">The position to set the listener to.</param>
		void SetPosition(const Position &value);

		/// <summary>
		///  <para>Gets the current velocity of the listener in 3D space.</para>
		///  <para>The velocity is only used to apply doppler effects and is not used to re-position this object over time.</para>
		/// </summary>
		/// <returns>Returns velocity as a vector.</returns>
		Vector GetVelocity();

		/// <summary>
		///  <para>Sets the current velocity of the listener in 3D space.</para>
		///  <para>The velocity is only used to apply doppler effects and is not used to re-position this object over time.</para>
		/// </summary>
		/// <param name="value">The velocity vector to set the listener to.</param>
		void SetVelocity(const Vector &value);

		#pragma endregion

	private:
		#pragma region Private Member Variables
		/// <summary>The audio context that owns this audio listener.</summary>
		AudioContext *fContextPointer;

		/// <summary>Pointer to the XAudio2 mastering voice which is analogous to the OpenAL listener.</summary>
		IXAudio2MasteringVoice *fXAudioMasteringVoicePointer;

		/// <summary>The listener's current position in 3D space.</summary>
		Position fPosition;

		/// <summary>The listener's current velocity used to apply doppler effects.</summary>
		Vector fVelocity;

		#pragma endregion
};

}	// namespace AL
