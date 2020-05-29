//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
	#error This header file cannot be included by an external library.
#endif


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

// Forward declarations.
ref class BitmapInfo;


/// <summary>Stores an uncompressed bitmap's pixels and information such as width, height, orientation, etc.</summary>
public interface class IBitmap
{
	/// <summary>Gets information about this bitmap such as its width, height, pixel format, etc.</summary>
	/// <value>
	///  <para>Read-only information about this bitmap.</para>
	///  <para>If the width and height is zero, then this bitmap object has no bytes.</para>
	/// </value>
	property BitmapInfo^ Info { BitmapInfo^ get(); }

	/// <summary>Gets the memory address of the first byte of the bitmap's byte buffer.</summary>
	/// <remarks>
	///  This property is intended to be read by native C/C++ code to access the bitmap's pixels as a byte array.
	/// </remarks>
	/// <returns>
	///  <para>
	///   The memory address of the first byte of the bitmap's contiguous byte buffer,
	///   which can be converted to a C/C++ pointer to easily access its bytes.
	///  </para>
	///  <para>Returns zero if the bitmap's byte buffer has been released, which is the equivalent to a null pointer.</para>
	/// </returns>
	property int64 ByteBufferMemoryAddress { int64 get(); }

	/// <summary>Deletes this bitmap's byte buffer and resets the pixel width and height to zero.</summary>
	/// <remarks>It is okay to call this method if the bitmap has already been released.</remarks>
	void ReleaseByteBuffer();
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
