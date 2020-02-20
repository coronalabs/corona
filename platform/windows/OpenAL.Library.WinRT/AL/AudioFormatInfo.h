//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "AudioFormatSettings.h"
#include "AudioDataType.h"
#include <xaudio2.h>


namespace AL {

/// <summary>Stores read-only information about an audio buffer's format such as frequency, number of channels, etc.</summary>
class AudioFormatInfo
{
	public:
		/// <summary>Creates a read-only audio format information object initialized to its defaults.</summary>
		AudioFormatInfo();

		/// <summary>Create a new read-only audio format information object using the given settings.</summary>
		/// <param name="settings">The settings to be copied to the new info object.</param>
		AudioFormatInfo(const AudioFormatSettings &settings);

		/// <summary>Destroys the audio format info object.</summary>
		virtual ~AudioFormatInfo();

		/// <summary>Gets the number of channels the audio buffer is formatted to use.</summary>
		/// <returns>Returns for 1 for mono or 2 for stereo.</returns>
		int GetChannelCount() const;

		/// <summary>Gets the frequency in Hertz that the audio buffer is formatted for.</summary>
		/// <returns>Returns the audio sample rate frequency in Hertz, such as 44100 Hz.</returns>
		int GetSampleRate() const;

		/// <summary>Gets the number of bits that a channel consumes in memory, such as 8-bit or 16-bit.</summary>
		/// <remarks>The bit depth is based on the data type assigned to the configuration object.</remarks>
		/// <returns>Returns the number of bits a channel's data consumes in memory such as 8-bit or 16-bit.</returns>
		int GetBitDepth() const;

		/// <summary>
		///  Gets the data type used by each channel in the audio buffer, such as 8-bit integer or 16-bit integer.
		/// </summary>
		/// <returns>Returns AudioDataType::kInt8 or AudioDataType::kInt16.</returns>
		AudioDataType GetDataType() const;

		/// <summary>
		///  <para>Calculates the number of bytes that will be used by a single audio frame/sample.</para>
		///  <para>This is based on the data type's bit depth and number of channels used.</para>
		/// </summary>
		/// <returns>
		///  Returns the number of bytes that will be used by a single sample based on this object's current settings.
		/// </returns>
		int GetSampleSizeInBytes() const;

		/// <summary>Creates a windows WAVEFORMATEX structure based on the configuration of this object.</summary>
		/// <returns>Returns a windows WAVEFORMATEX structure that matching the settings of this object.</returns>
		WAVEFORMATEX ToWAVEFORMATEX() const;

		/// <summary>Creates a windows WAVEFORMATEXTENSIBLE structure based on the configuration of this object.</summary>
		/// <returns>Returns a windows WAVEFORMATEXTENSIBLE structure that matching the settings of this object.</returns>
		WAVEFORMATEXTENSIBLE ToWAVEFORMATEXTENSIBLE() const;

		/// <summary>Determines if this format object matches the given format.</summary>
		/// <param name="value">The format to compare with.</param>
		/// <returns>Returns true if the formats match. Returns false if not.</returns>
		bool Equals(const AudioFormatInfo &value) const;

		/// <summary>Determines if this format object does not match the given format.</summary>
		/// <param name="value">The format to compare with.</param>
		/// <returns>Returns true if the formats do not match. Returns true if they do match.</returns>
		bool NotEquals(const AudioFormatInfo &value) const;

		/// <summary>Determines if this format object matches the given format.</summary>
		/// <param name="value">The format to compare with.</param>
		/// <returns>Returns true if the formats match. Returns false if not.</returns>
		bool operator==(const AudioFormatInfo &value) const;

		/// <summary>Determines if this format object does not match the given format.</summary>
		/// <param name="value">The format to compare with.</param>
		/// <returns>Returns true if the formats do not match. Returns true if they do match.</returns>
		bool operator!=(const AudioFormatInfo &value) const;

	private:
		/// <summary>Copy of the audio format settings that this read-only container wraps.</summary>
		AudioFormatSettings fSettings;
};

}	// namespace AL
