// ----------------------------------------------------------------------------
// 
// BitmapSettings.h
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

#include "PixelFormat.h"

#pragma region Forward Declarations
namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop {
	ref class RelativeOrientation2D;
} } } }

#pragma endregion


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

/// <summary>Represents the configuration of one bitmap such as pixel width, height, format, etc.</summary>
public ref class BitmapSettings sealed
{
	public:
		#pragma region Constructors
		/// <summary>Creates a new settings object initialize to a zero width and height.</summary>
		BitmapSettings();

		/// <summary>Creates a new settings object having a copy of the given settings.</summary>
		/// <param name="settings">The settings to be copied. Cannot be null or else an exception will occur.</param>
		BitmapSettings(BitmapSettings^ settings);

		#pragma endregion


		#pragma region Public Methods/Properties
		/// <summary>Gets or sets the width of the bitmap in pixels.</summary>
		/// <value>
		///  <para>The width of the bitmap in pixels.</para>
		///  <para>Set to zero to indicate that no bitmap has been created or loaded.</para>
		/// </value>
		property int PixelWidth { int get(); void set(int value); }

		/// <summary>Gets or sets the height of the bitmap in pixels.</summary>
		/// <value>
		///  <para>The height of the bitmap in pixels.</para>
		///  <para>Set to zero to indicate that no bitmap has been created or loaded.</para>
		/// </value>
		property int PixelHeight  { int get(); void set(int value); }

		/// <summary>Gets or sets the format used by each pixel in the bitmap.</summary>
		/// <value>
		///  <para>The bitmap's pixel format such as Grayscale or RGBA.</para>
		///  <para>Set to <see cref="CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::Undefined">PixelFormat.Undefined</see> if the format is unknown or no bitmap was created/loaded.</para>
		///  <para>Setting this property to null will change the format to <see cref="CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::Undefined">PixelFormat.Undefined</see>.</para>
		/// </value>
		property CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat^ PixelFormat
		{
			CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat^ get();
			void set(CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat^ value);
		}

		/// <summary>
		///  Gets or sets whether or not the alpha channel has been multiplied against the RGB channels for all pixels.
		/// </summary>
		/// <remarks>This property is not applicable if the pixel format does not support alpha.</remarks>
		/// <value>
		///  <para>Set true if the RGB channels of all pixels have been premultiplied with their alpha channels.</para>
		///  <para>Set to false if the alpha channel has never been multiplied against the RGB channels.</para>
		/// </value>
		property bool PremultipliedAlphaApplied { bool get(); void set(bool value); }

		/// <summary>
		///  Gets or sets the orientation of the bitmap, which indicates if the bitmap was rotated
		///  when loaded from the image source.
		/// </summary>
		/// <value>
		///  <para>Set to Upright if the image has not been rotated.</para>
		///  <para>Set to SidewaysRight if the image has been rotated 90 degrees clockwise.</para>
		///  <para>Set to SidewaysLeft if the image has been rotated 90 degrees counter-clockwise.</para>
		///  <para>Set to UpsideDown if the image has been rotated 180 degrees.</para>
		///  <para>Setting this property to null or unknown will reset this property to Upright.</para>
		/// </value>
		property RelativeOrientation2D^ Orientation
		{
			RelativeOrientation2D^ get();
			void set(RelativeOrientation2D^ value);
		}

		/// <summary>Gets or sets the scale of this bitmap compared to the original image that was loaded.</summary>
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
		///  <para>Setting this to a value less than or equal to zero will cause an exception to be thrown.</para>
		/// </value>
		property float Scale { float get(); void set(float value); }

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

		#pragma endregion

	private:
		#pragma region Private Member Variables
		/// <summary>The bitmap's pixel width.</summary>
		int fPixelWidth;

		/// <summary>The bitmap's pixel height.</summary>
		int fPixelHeight;

		/// <summary>The format of the bitmap's pixels such as grayscale or RGBA.</summary>
		CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat^ fPixelFormat;

		/// <summary>Set to true if all pixels have been multiplied against their respective alpha channel.</summary>
		bool fPremultipliedAlphaApplied;

		/// <summary>Indicates if the bitmap is rotated.</summary>
		RelativeOrientation2D^ fOrientation;

		/// <summary>
		///  <para>Value indicating by how much the bitmap was downscaled compared to the source image.</para>
		///  <para>
		///   For examle, a value of 1.0f indicates that the image was not downscaled.
		///   A value of 0.5f indicates the image was downscaled by 50% compared to the original image.
		///  </para>
		///  <para>Cannot be set to less than or equal to zero.</para>
		/// </summary>
		float fScale;

		#pragma endregion
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
