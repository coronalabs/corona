//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Rtt_Build.h"
#	include "Rtt_PlatformAudioPlayer.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

class WinRTAudioPlayer : public PlatformAudioPlayer
{
	public:
		/// <summary>Creates an audio player used by Corona's "media" API's in Lua.</summary>
		/// <param name="handle">Provides the Lua listener to be invoked when playback ends.</param>
		/// <param name="allocator">Allocator needed to create memory for this player.</param>
		WinRTAudioPlayer(const ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator);

		/// <summary>Destroys this player and disposes its allocated resources.</summary>
		virtual ~WinRTAudioPlayer();

		/// <summary>Loads the given audio file into memory.</summary>
		/// <param name="filePath">Path and name of the audio file to be loaded. Cannot be NULL or empty.</param>
		/// <returns>
		///  <para>Returns true if the given audio file was loaded successfully.</para>
		///  <para>Returns false if failed to load the file or if given an invalid argument.</para>
		/// </returns>
		virtual bool Load(const char * filePath);

		/// <summary>Plays the audio file that was loaded via the Load() function.</summary>
		virtual void Play();

		/// <summary>Stops audio playback and resets back to the beginning of the audio track.</summary>
		virtual void Stop();

		/// <summary>Pauses audio playback.</summary>
		virtual void Pause();

		/// <summary>Resumes audio playback if currently paused.</summary>
		virtual void Resume();

		/// <summary>Sets the volume level.</summary>
		/// <param name="volume">
		///  <para>A value ranging between 0.0 and 1.0.</para>
		///  <para>Set to 0.0 to mute audio.</para>
		///  <para>Set to 1.0 to set the volume at the highest level.</para>
		/// </param>
		virtual void SetVolume(Rtt_Real volume);

		/// <summary>Gets the volume level for this audio track.</summary>
		/// <returns>Returns the current volume level. Ranges between 0.0 and 1.0, where 0.0 is muted.</returns>
		virtual Rtt_Real GetVolume() const;
};

} // namespace Rtt
