// ----------------------------------------------------------------------------
// 
// AudioPlaybackPosition.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


namespace AL {

class AudioFormatInfo;

/// <summary>
///  <para>Stores a single position in an audio track in either bytes, seconds, or audio frames/samples.</para>
///  <para>Intended to be used to indicate current playback position in an audio track or where to start playback.</para>
/// </summary>
class AudioPlaybackPosition
{
	public:
		/// <summary>Indicates the units of an audio playback position such as bytes or seconds.</summary>
		enum class Units
		{
			/// <summary>
			///  Indicates that the audio track position is in bytes relative to the byte buffer(s) being played.
			/// </summary>
			kBytes,

			/// <summary>Indicates that the audio track position is in seconds.</summary>
			kSeconds,

			/// <summary>Indicates that the audio track position is measured in audio frames/samples.</summary>
			kSamples
		};

		/// <summary>Creates a default audio track position initialized to zero bytes.</summary>
		AudioPlaybackPosition();

		/// <summary>Creates a new audio track position with the given units and positional value.</summary>
		/// <param name="units">
		///  <para>The units that the "value" parameter is measured which must be one of the following:</para>
		///  <para>
		///   AudioPlaybackPosition::Units::kBytes, AudioPlaybackPosition::Units::kSeconds,
		///   or AudioPlaybackPosition::Units::kSamples.
		///  </para>
		/// </param>
		/// <param name="value">The audio track positional value. Zero would be the start of the audio track.</param>
		AudioPlaybackPosition(Units units, uint32 value);

		/// <summary>Destroys this object.</summary>
		virtual ~AudioPlaybackPosition();

		/// <summary>Gets the units that positional value is in such as bytes, seconds, or frames.</summary>
		/// <returns>
		///  Returns either AudioPlaybackPosition::Units::kBytes, AudioPlaybackPosition::Units::kSeconds,
		///  or AudioPlaybackPosition::Units::kSamples.
		/// </returns>
		Units GetUnits() const;

		/// <summary>Gets the positional value in the audio track in the units specified by the GetUnits() function.</summary>
		/// <returns>
		///  <para>Returns the positional value.</para>
		///  <para>Zero would indicate the start of the audio track.</para>
		/// </returns>
		uint32 GetValue() const;

		/// <summary>Creates a new position object providing this object's position converted to bytes.</summary>
		/// <param name="format">
		///  Audio format information needed to convert a position measured in seconds or samples into bytes.
		/// </param>
		/// <returns>Returns a new playback position measured in bytes.</returns>
		AudioPlaybackPosition ToBytesUsing(const AudioFormatInfo &format);

		/// <summary>Creates a new position object providing this object's position converted to seconds.</summary>
		/// <param name="format">
		///  Audio format information needed to convert a position measured in bytes or samples into seconds.
		/// </param>
		/// <returns>Returns a new playback position measured in seconds.</returns>
		AudioPlaybackPosition ToSecondsUsing(const AudioFormatInfo &format);

		/// <summary>Creates a new position object providing this object's position converted to samples/frames.</summary>
		/// <param name="format">
		///  Audio format information needed to convert a position measured in bytes or seconds into samples.
		/// </param>
		/// <returns>Returns a new playback position measured in samples/frames.</returns>
		AudioPlaybackPosition ToSamplesUsing(const AudioFormatInfo &format);

		/// <summary>Creates a new position object providing this object's position converted to the given unit.</summary>
		/// <param name="unit">
		///  <para>The unit to convert to which must be one of the following:</para>
		///  <para>
		///   AudioPlaybackPosition::Units::kBytes, AudioPlaybackPosition::Units::kSeconds,
		///   or AudioPlaybackPosition::Units::kSamples.
		///  </para>
		/// </param>
		/// <param name="format">
		///  Audio format information needed to convert a position measured in bytes or samples into seconds.
		/// </param>
		/// <returns>Returns a new playback position measured in the given units.</returns>
		AudioPlaybackPosition ToUnitUsing(AudioPlaybackPosition::Units unit, const AudioFormatInfo &format);

		/// <summary>Creates a new playback position specified in bytes.</summary>
		/// <param name="value">
		///  <para>The audio track positional value in bytes.</para>
		///  <para>Zero would indicate the start of the audio track.</para>
		/// </param>
		/// <returns>Returns a new audio track position object providing the given value in bytes.</returns>
		static AudioPlaybackPosition FromBytes(uint32 value);

		/// <summary>Creates a new playback position specified in seconds.</summary>
		/// <param name="value">
		///  <para>The audio track positional value in seconds.</para>
		///  <para>Zero would indicate the start of the audio track.</para>
		/// </param>
		/// <returns>Returns a new audio track position object providing the given value in seconds.</returns>
		static AudioPlaybackPosition FromSeconds(uint32 value);

		/// <summary>Creates a new playback position specified in audio frames/samples.</summary>
		/// <param name="value">
		///  <para>The audio track positional value in frames/samples.</para>
		///  <para>Zero would indicate the start of the audio track.</para>
		/// </param>
		/// <returns>Returns a new audio track position object providing the given value in samples.</returns>
		static AudioPlaybackPosition FromSamples(uint32 value);

	private:
		/// <summary>Indicates the units member variable "fValue" is measured in, such as bytes or seconds.</summary>
		Units fUnits;

		/// <summary>The audio track positional value using the measurement units indicates by member variable "fUnits".</summary>
		uint32 fValue;
};

}	// namespace AL
