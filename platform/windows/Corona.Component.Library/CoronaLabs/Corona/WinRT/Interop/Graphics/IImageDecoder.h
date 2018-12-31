// ----------------------------------------------------------------------------
// 
// IImageDecoder.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

// Forward declarations.
interface class IBitmap;
ref class BitmapResult;
ref class BitmapInfoResult;
ref class ImageDecoderSettings;


/// <summary>Decodes an image source such as a PNG or JPEG file to an uncompressed bitmap.</summary>
public interface class IImageDecoder
{
	/// <summary>Gets the configuration this image decoder will use when decoding an image to a bitmap.</summary>
	/// <value>
	///  Modifiable settings that the caller should set up before decoding an image. This tells the decoder
	///  what pixel formats the caller supports, maximum pixel width/height the resulting bitmap should be
	///  constrained to, whether or not premultiplied alpha should be applied, etc.
	/// </value>
	property ImageDecoderSettings^ Settings { ImageDecoderSettings^ get(); }

	/// <summary>
	///  Partially decodes the given image file and retrieves information about the bitmap that would be produced
	///  when calling the DecodeFromFile() method.
	/// </summary>
	/// <remarks>
	///  Note that if you have applied a maximum pixel width or height to this decoder, then the width and height
	///  returned by this method will reflect what the downscaled bitmap size will be and not the actual size
	///  of the original image file.
	/// </remarks>
	/// <param name="filePath">The name and path of the image file to be decoded.</param>
	/// <returns>
	///  <para>Returns the result of the image decoding operation.</para>
	///  <para>
	///   If the result object's HasSucceeded property is set true, then this decoder succeeded in decoding
	///   the given image file. The information about the bitmap is provided by the result object's BitmapInfo property.
	///  </para>
	///  <para>
	///   If the result object's HasSucceeded property is false, then this decoder has failed to decode the
	///   given image file. The reason why is provided by the result object's Message property.
	///  </para>
	/// </returns>
	BitmapInfoResult^ DecodeInfoFromFile(Platform::String^ filePath);
	
	/// <summary>Decodes the given image file and returns an uncompressed bitmap.</summary>
	/// <param name="filePath">The name and path of the image file to be decoded.</param>
	/// <returns>
	///  <para>Returns the result of the image decoding operation.</para>
	///  <para>
	///   If the result object's HasSucceeded property is set true, then this decoder succeeded in decoding
	///   the given image file and the result object's Bitmap property would provide the bitmap that was produced.
	///  </para>
	///  <para>
	///   If the result object's HasSucceeded property is false, then this decoder failed to decode the
	///   given image file. The reason why would then be provided by the result object's Message property.
	///  </para>
	/// </returns>
	BitmapResult^ DecodeFromFile(Platform::String^ filePath);
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
