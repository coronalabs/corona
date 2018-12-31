// ----------------------------------------------------------------------------
// 
// BitmapInfo.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
	#error This header file cannot be included by an external library.
#endif

#include "PixelFormat.h"

#pragma region Forward Declarations
namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop {
	namespace Graphics
	{
		ref class BitmapSettings;
	}
	ref class RelativeOrientation2D;
} } } }

#pragma endregion


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

/// <summary>Provides information about a bitmap such as pixel width, height, format, etc.</summary>
/// <remarks>Instances of this class are immutable.</remarks>
public ref class BitmapInfo sealed
{
	private:
		/// <summary>Creates an immutable information object for an empty bitmap having zero width and height.</summary>
		/// <remarks>
		///  This is a private constructor that should only be called once via this class' static Empty property.
		/// </remarks>
		BitmapInfo();

	public:
		/// <summary>Creates an immutable bitmap information object containing a copy of the given settings.</summary>
		/// <param name="settings">The settings to be copied. Cannot be null or else an exception will occur.</param>
		BitmapInfo(BitmapSettings^ settings);

		/// <summary>Gets the width of the bitmap in pixels.</summary>
		/// <value>
		///  <para>The width of the bitmap in pixels.</para>
		///  <para>Zero indicates that no bitmap has been created or loaded.</para>
		/// </value>
		property int PixelWidth { int get(); }

		/// <summary>Gets the height of the bitmap in pixels.</summary>
		/// <value>
		///  <para>The height of the bitmap in pixels.</para>
		///  <para>Zero indicates that no bitmap has been created or loaded.</para>
		/// </value>
		property int PixelHeight  { int get(); }
		
		/// <summary>Gets the format used by each pixel in the bitmap.</summary>
		/// <value>
		///  <para>The bitmap's pixel format such as Grayscale or RGBA.</para>
		///  <para>Returns <see cref="CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::Undefined">PixelFormat.Undefined</see> if the format is unknown or no bitmap was created/loaded.</para>
		/// </value>
		property CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat^ PixelFormat
		{
			CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat^ get();
		}

		/// <summary>
		///  Determines whether or not the alpha channel has been multiplied against the RGB channels for all pixels.
		/// </summary>
		/// <remarks>This property is not applicable if the pixel format does not support alpha.</remarks>
		/// <value>
		///  <para>Set true if the RGB channels of all pixels have been premultiplied with their alpha channels.</para>
		///  <para>Set to false if the alpha channel has never been multiplied against the RGB channels.</para>
		/// </value>
		property bool PremultipliedAlphaApplied { bool get(); }

		/// <summary>
		///  Gets the orientation of the bitmap, which indicates if the bitmap was rotated
		///  when loaded from the image source.
		/// </summary>
		/// <value>
		///  <para>Set to Upright if the image has not been rotated.</para>
		///  <para>Set to SidewaysRight if the image has been rotated 90 degrees clockwise.</para>
		///  <para>Set to SidewaysLeft if the image has been rotated 90 degrees counter-clockwise.</para>
		///  <para>Set to UpsideDown if the image has been rotated 180 degrees.</para>
		/// </value>
		property RelativeOrientation2D^ Orientation { RelativeOrientation2D^ get(); }

		/// <summary>Gets the scale of this bitmap compared to the original image that was loaded.</summary>
		/// <remarks>
		///  <para>
		///   This is expected to be set to less than 1.0 if the image loader had to downscale the image.
		///   This way, if the rendering system wanted to display the bitmap at its image source's orignal
		///   size, it would know by how much to scale it up to.
		///  </para>
		///  <para>
		///   Note that this object's PixelWidth and PixelHeight are expected to be the downscaled lengths.
		///   For example, if the original image width and height was 100x100 pixels and the scale is set to 0.5,
		///   then this object's PixelWidth and PixelHeight should be set to 50x50 pixels.
		///  </para>
		///  <para>The formula for calculating this scale is:</para>
		///  <para>scale = (loadedImageWidth / originalImageWidth)</para>
		/// </remarks>
		/// <value>
		///  <para>
		///   This value will be 1.0 if the bitmap was not scaled when loaded, meaning that the pixel width and height
		///   of the bitmap matches the image source it was loaded from.
		///  </para>
		///  <para>A value less than 1.0 indicates that the bitmap was downscaled compared to the original source.</para>
		/// </value>
		property float Scale { float get(); }

		/// <summary>
		///  Gets the number of bytes per row the bitmap will use, including the additional bytes that might be padded
		///  at the end to meet the required 4 byte packing alignment in memory.
		/// </summary>
		/// <value>
		///  <para>
		///   The bytes per row the bitmap will use. This value would be the pixel width multiplied by the number of bytes
		///   each pixel takes, rounded up to the next increment of 4.
		///  </para>
		///  <para>
		///   For example, if the bitmap is 10 pixels wide and has an 8-bit grayscale pixel format (ie: 1 byte per pixel),
		///   then the bytes per row taken by the pixels would be 10 bytes, but the stride would be 12 bytes round up
		///   to the next increment of 4.
		///  </para>
		///  <para>
		///   Also note that a bitmap using a 32-bit color pixel format (ie: 4 byte color format) will always fit
		///   within the system's byte packing alignment and will not require extra byte padding in the end.
		///  </para>
		///  <para>This property will return 0 if the pixel width is zero or if the pixel format is undefined.</para>
		/// </value>
		property int StrideInBytes { int get(); }

		/// <summary>Gets a reusable info object used to represent an empty zero with and height bitmap.</summary>
		/// <value>Reusable Information representing a zero width and height bitmap.</value>
		static property BitmapInfo^ Empty { BitmapInfo^ get(); }

	private:
		/// <summary>Copy of the bitmap settings that this immutable information object wraps.</summary>
		BitmapSettings^ fSettings;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
