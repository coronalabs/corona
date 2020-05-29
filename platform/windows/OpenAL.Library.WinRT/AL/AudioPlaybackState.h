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


namespace AL {

/// <summary>
///  <para>Indicates the current playback state of an audio source such as kPlaying, kStopped, etc.</para>
///  <para>
///   Provides a simple means of converting the state to OpenAL's equivalent integer ID constants such as
///   AL_PLAYING, AL_STOPPED, etc.
///  </para>
/// </summary>
class AudioPlaybackState
{
	private:
		/// <summary>Creates a new audio playback state.</summary>
		/// <remarks>This is a private constructor used to create this class' predefined states.</remarks>
		/// <param name="alStateId">OpenAL's unique integer ID for the state such as AL_PLAYING, AL_STOPPED, etc.</param>
		AudioPlaybackState(ALint alStateId);

	public:
		AudioPlaybackState();

		/// <summary>Destroys this object.</summary>
		virtual ~AudioPlaybackState();

		/// <summary>Gets OpenAL's unique integer ID for this playback state, such as AL_PLAYING, AL_STOPPED, etc.</summary>
		/// <returns>Returns AL_INITIAL, AL_PLAYING, AL_PAUSED, or AL_STOPPED.</returns>
		ALint ToOpenALStateId() const;

		/// <summary>Determines if this state matches the given state.</summary>
		/// <param name="value">The state to compare with.</param>
		/// <returns>Returns true if the states match. Returns false if not.</returns>
		bool Equals(const AudioPlaybackState &value) const;

		/// <summary>Determines if this state does not match the given state.</summary>
		/// <param name="value">The state to compare with.</param>
		/// <returns>Returns true if the states do not match. Returns true if they do match.</returns>
		bool NotEquals(const AudioPlaybackState &value) const;

		/// <summary>Determines if this state matches the given state.</summary>
		/// <param name="value">The state to compare with.</param>
		/// <returns>Returns true if the states match. Returns false if not.</returns>
		bool operator==(const AudioPlaybackState &value) const;

		/// <summary>Determines if this state does not match the given state.</summary>
		/// <param name="value">The state to compare with.</param>
		/// <returns>Returns true if the states do not match. Returns true if they do match.</returns>
		bool operator!=(const AudioPlaybackState &value) const;

		/// <summary>Indicates that an audio source is at the beginning of its audio track and is not playing.</summary>
		static const AudioPlaybackState kReseted;

		/// <summary>Indicates that the audio source is currently playing audio.</summary>
		static const AudioPlaybackState kPlaying;

		/// <summary>Indicates that an audio source has paused playback.</summary>
		static const AudioPlaybackState kPaused;

		/// <summary>Indicates that an audio source has stopped playback.</summary>
		static const AudioPlaybackState kStopped;

	private:
		/// <summary>OpenAL's unique integer ID for this state such as AL_PLAYING, AL_STOPPED, etc.</summary>
		ALint fOpenALStateId;
};

}	// namespace AL
