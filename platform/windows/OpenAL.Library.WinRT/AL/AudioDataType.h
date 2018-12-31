// ----------------------------------------------------------------------------
// 
// AudioDataType.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


namespace AL {

/// <summary>Represents the type of data an audio buffer is formatted in such as 8-bit integer or 16-bit integer.</summary>
/// <remarks>
///  You cannot create instances of this class. Instead, you fetch predefined types via this class' constants
///  much like an enum, such as the kInt8 and kInt16 constants.
/// </remarks>
class AudioDataType
{
	private:
		/// <summary>Creates a new data type.</summary>
		/// <remarks>This is a private constructor used to create this class' predefined data types.</remarks>
		/// <param name="typeId">
		///  <para>Unique integer ID assigned to the data type used to check for equality.</para>
		///  <para>This ID is not exposed publicly.</para>
		/// </param>
		/// <param name="bitDepth">Number of bits the data type uses such as 8-bit or 16-bit.</param>
		AudioDataType(int typeId, int bitDepth);

	public:
		/// <summary>Destroys this object.</summary>
		virtual ~AudioDataType();

		/// <summary>Gets the number of bits the data type uses in memory such as 8 or 16.</summary>
		/// <returns>Returns the number of bits the data type uses in memory such as 8 or 16.</returns>
		int GetBitDepth() const;

		/// <summary>Determines if this data type matches the given type.</summary>
		/// <param name="value">The type to compare with.</param>
		/// <returns>Returns true if the types match. Returns false if not.</returns>
		bool Equals(const AudioDataType &value) const;

		/// <summary>Determines if this data type does not match the given type.</summary>
		/// <param name="value">The type to compare with.</param>
		/// <returns>Returns true if the types do not match. Returns true if they do match.</returns>
		bool NotEquals(const AudioDataType &value) const;

		/// <summary>Determines if this data type matches the given type.</summary>
		/// <param name="value">The type to compare with.</param>
		/// <returns>Returns true if the types match. Returns false if not.</returns>
		bool operator==(const AudioDataType &value) const;

		/// <summary>Determines if this data type does not match the given type.</summary>
		/// <param name="value">The type to compare with.</param>
		/// <returns>Returns true if the types do not match. Returns true if they do match.</returns>
		bool operator!=(const AudioDataType &value) const;

		/// <summary>Represents an 8-bit signed integer audio format.</summary>
		static const AudioDataType kInt8;

		/// <summary>Represents a 16-bit signed integer audio format.</summary>
		static const AudioDataType kInt16;

	private:
		/// <summary>Unique integer ID assigned to the type used for equality checks. Not exposed publicly.</summary>
		int fTypeId;

		/// <summary>The number of bits the data type consumes in memory such as 8-bit or 16-bit.</summary>
		int fBitDepth;
};

}	// namespace AL
