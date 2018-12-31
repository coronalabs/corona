// ----------------------------------------------------------------------------
// 
// AudioFormatSettings.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

#include "AudioDataType.h"
#include <xaudio2.h>


namespace AL {

/// <summary>Configures the format to be used by an audio buffer such as sample rate, channels, data type, etc.</summary>
class AudioFormatSettings
{
	public:
		/// <summary>Create a new audio format configuration object.</summary>
		AudioFormatSettings();

		/// <summary>Destroys the audio format settings object.</summary>
		virtual ~AudioFormatSettings();

		/// <summary>Gets the number of channels the audio buffer is formatted to use.</summary>
		/// <returns>Returns for 1 for mono or 2 for stereo.</returns>
		int GetChannelCount() const;

		/// <summary>Sets the number of channels the audio buffer is formatted for.</summary>
		/// <param name="value">Set to 1 for mono or 2 for stereo.</param>
		void SetChannelCount(int value);

		/// <summary>Gets the frequency in Hertz that the audio buffer is formatted for.</summary>
		/// <returns>Returns the audio sample rate frequency in Hertz, such as 44100 Hz.</returns>
		int GetSampleRate() const;

		/// <summary>Sets the frequency the audio buffer is formatted for in Hertz.</summary>
		/// <param name="value">The frequency in Hertz, such as 44100 Hz.</param>
		void SetSampleRate(int value);

		/// <summary>Gets the number of bits that a channel consumes in memory, such as 8-bit or 16-bit.</summary>
		/// <remarks>The bit depth is based on the data type assigned to the configuration object.</remarks>
		/// <returns>Returns the number of bits a channel's data consumes in memory such as 8-bit or 16-bit.</returns>
		int GetBitDepth() const;

		/// <summary>
		///  Gets the data type used by each channel in the audio buffer, such as 8-bit integer or 16-bit integer.
		/// </summary>
		/// <returns>Returns AudioDataType::kInt8 or AudioDataType::kInt16.</returns>
		AudioDataType GetDataType() const;

		/// <summary>Sets the data type used by each channel in the audio buffer.</summary>
		/// <param name="value">
		///  The data type such as AudioDataType::kInt8 or AudioDataType::kInt16, which are the only types that
		///  OpenAL currently supports.
		/// </param>
		void SetDataType(AudioDataType value);

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
		bool Equals(const AudioFormatSettings &value) const;

		/// <summary>Determines if this format object does not match the given format.</summary>
		/// <param name="value">The format to compare with.</param>
		/// <returns>Returns true if the formats do not match. Returns true if they do match.</returns>
		bool NotEquals(const AudioFormatSettings &value) const;

		/// <summary>Determines if this format object matches the given format.</summary>
		/// <param name="value">The format to compare with.</param>
		/// <returns>Returns true if the formats match. Returns false if not.</returns>
		bool operator==(const AudioFormatSettings &value) const;

		/// <summary>Determines if this format object does not match the given format.</summary>
		/// <param name="value">The format to compare with.</param>
		/// <returns>Returns true if the formats do not match. Returns true if they do match.</returns>
		bool operator!=(const AudioFormatSettings &value) const;

		enum
		{
			/// <summary>The minimum number of channels that can be passed to the SetChannelCount() function.</summary>
			kMinChannels = 1,

			/// <summary>
			///  <para>The maximum number of channels that can be passed to the SetChannelCount() function.</para>
			///  <para>
			///   Note that this maximum is an OpenAL alBufferData() limitation. Audio sources in OpenAL only support
			///   up to 8 channels (7.1 surround sound) when using the non-standard extension formats defined in
			///   in the "alext.h" header file.
			///  </para>
			/// </summary>
			kMaxChannels = 8
		};

	private:
		/// <summary>Number of channels that the audio buffer is formatted for.</summary>
		int fChannelCount;

		/// <summary>The sample rate in Hertz thta the audio buffer is formatted for.</summary>
		int fSampleRate;

		/// <summary>The type of data that each channel in the audio buffer uses such as 8-bit or 16-bit integer.</summary>
		AudioDataType fDataType;
};

}	// namespace AL
