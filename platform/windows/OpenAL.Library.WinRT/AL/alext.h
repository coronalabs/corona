/* ----------------------------------------------------------------------------
//
// alext.h
// Copyright (c) 2016 Corona Labs Inc. All rights reserved.
//
// ----------------------------------------------------------------------------
*/

#pragma once


#ifdef __cplusplus
extern "C" {
#endif


#pragma region Constants
/// <summary>Non-standard OpenAL audio buffer formats supporting surround sound.</summary>
enum
{
	/// <summary>4 channel 8-bit sound format.</summary>
	AL_FORMAT_QUAD8 = 0x1204,

	/// <summary>4 channel 16-bit sound format.</summary>
	AL_FORMAT_QUAD16 = 0x1205,

	/// <summary>6 channel 8-bit sound format.</summary>
	AL_FORMAT_51CHN8 = 0x120A,

	/// <summary>6 channel 16-bit sound format.</summary>
	AL_FORMAT_51CHN16 = 0x120B,

	/// <summary>7 channel 8-bit sound format.</summary>
	AL_FORMAT_61CHN8 = 0x120D,

	/// <summary>7 channel 16-bit sound format.</summary>
	AL_FORMAT_61CHN16 = 0x120E,

	/// <summary>8 channel 8-bit sound format.</summary>
	AL_FORMAT_71CHN8 = 0x1210,

	/// <summary>8 channel 16-bit sound format.</summary>
	AL_FORMAT_71CHN16 = 0x1211,
};

#pragma endregion


#ifdef __cplusplus
}	// extern "C"
#endif
