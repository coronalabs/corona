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
#	error This header file cannot be included by an external library.
#endif

#include "IBitmap.h"
#include <collection.h>


// External forward delcarations.
namespace CoronaLabs { namespace WinRT {
	ref class OperationResult;
} }


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

// Forward declarations.
ref class BitmapInfo;
ref class BitmapSettings;


/// <summary>Stores an uncompressed bitmap's pixels and information such as width, height, orientation, etc.</summary>
/// <remarks>
///  This bitmap can be reformatted to various pixel formats that Corona supports such as RGBA, BGRA, and Grayscale.
///  This class is useful if the platform's image decoder does not support decoding to a pixel format that Corona
///  wants, such as 8-bit grayscale.
/// </remarks>
public ref class Bitmap sealed : public IBitmap
{
	public:
		/// <summary>Creates a new object used to store the pixel of an uncompressed bitmap.</summary>
		Bitmap();

		/// <summary>Sets up this bitmap's byte buffer for the given settings.</summary>
		/// <param name="info">
		///  <para>The settings to use when creating this bitmap's byte buffer such as pixel width, height, and format.</para>
		///  <para>Setting this to BitmapInfo.Empty will release this bitmap's byte buffer.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a success result if the bitmap was formatted with the given settings successfully.</para>
		///  <para>
		///   Returns a failure result if given invalid settings or if failed to allocate the byte buffer.
		///   In this case, the result object's Message property will indicate what exactly went wrong.
		///  </para>
		/// </returns>
		CoronaLabs::WinRT::OperationResult^ FormatUsing(BitmapInfo^ info);

		/// <summary>Deletes this bitmap's byte buffer and resets the pixel width and height to zero.</summary>
		/// <remarks>It is okay to call this method if the bitmap has already been released.</remarks>
		virtual void ReleaseByteBuffer();

		/// <summary>Gets information about this bitmap such as its width, height, pixel format, etc.</summary>
		/// <value>
		///  <para>Read-only information about this bitmap.</para>
		///  <para>If the width and height is zero, then this bitmap object has no bytes.</para>
		/// </value>
		virtual property BitmapInfo^ Info { BitmapInfo^ get(); }

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
		virtual property int64 ByteBufferMemoryAddress { int64 get(); }

	private:
		/// <summary>Stores information about this bitmap such as width, height, format, orientation, etc.</summary>
		BitmapInfo^ fBitmapInfo;

		/// <summary>Stores this bitmap's pixels as a byte array.</summary>
		Platform::Array<uint8>^ fBitmapBytes;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
