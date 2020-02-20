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


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

// Forward declarations.
ref class PixelFormat;
ref class PixelFormatSet;

/// <summary>Provides the configuration for an IImageDecoder implemented object.</summary>
public ref class ImageDecoderSettings sealed
{
	public:
		/// <summary>Creates a new configuration for an decoder that implements the IImageDecoder interface.</summary>
		ImageDecoderSettings();

		/// <summary>Copies the given object's settings to this object.</summary>
		/// <param name="settings">The settings to be copied. Will be ignored if null.</param>
		void CopyFrom(ImageDecoderSettings^ settings);

		/// <summary>
		///  Gets or sets the maximum pixel width of the bitmap the decoder will produce when loading an image.
		/// </summary>
		/// <remarks>
		///  If the image source is larger than this maximum, then the decoder will downscale/downsample the image to fit.
		/// </remarks>
		/// <value>
		///  <para>The maximum pixel width the resulting bitmap can be when loaded from the image source.</para>
		///  <para>Set to zero if no maximum should be applied.</para>
		/// </value>
		property int MaxPixelWidth { int get(); void set(int value); }

		/// <summary>
		///  Gets or sets the maximum pixel height of the bitmap the decoder will produce when loading an image.
		/// </summary>
		/// <remarks>
		///  If the image source is larger than this maximum, then the decoder will downscale/downsample the image to fit.
		/// </remarks>
		/// <value>
		///  <para>The maximum pixel height the resulting bitmap can be when loaded from the image source.</para>
		///  <para>Set to zero if no maximum should be applied.</para>
		/// </value>
		property int MaxPixelHeight { int get(); void set(int value); }

		/// <summary>
		///  Gets or sets whether or not the decoded image's pixels will have premultiplied alpha applied to the resuling bitmap.
		/// </summary>
		/// <remarks>This property is ignored if the image source does not have an alpha channel.</remarks>
		/// <value>
		///  <para>Set to true if the resulting bitmap will have premultiplied alpha applied to it.</para>
		///  <para>
		///   Set to false if the decoded image's alpha channel will not be multiplied against the resulting
		///   bitmap's RGB channels.
		///  </para>
		/// </value>
		property bool PremultiplyAlphaEnabled { bool get(); void set(bool value); }

		/// <summary>
		///  Gets or sets the pixel format that the image decoder should use if the image source is not using a format
		///  specified in the SupportedPixelFormats collection.
		/// </summary>
		/// <value>
		///  <para>
		///   The pixel format of the bitmap the image decoder will produce if the image source does not
		///   match any pixel formats specified in the SupportedPixelFormats collection. In this case,
		///   the image decoder has no choice but to convert the decoded pixels which can cause a performance penalty.
		///  </para>
		///  <para>Will throw an exception if set to null or Undefined.</para>
		/// </value>
		property PixelFormat^ PreferredPixelFormat { PixelFormat^ get(); void set(PixelFormat^ value); }

		/// <summary>
		///  Gets a modifiable collection of pixel formats, such as Grayscale and RGBA, that informs the image decoder
		///  which formats the caller supports.
		/// </summary>
		/// <remarks>
		///  <para>
		///   This property returns a modifiable collection. The caller is expected to add only the formats that it
		///   supports. For example, if the caller only supports 8-bit grayscale bitmaps when loading an image from file,
		///   then only the Grayscale pixel format should be added to this collection. This is an optimization for the
		///   image decoder, because if the image being decoded is using one of these supported formats, then the decoder
		///   does not need to convert the pixels.
		///  </para>
		///  <para>
		///   If the decoded image's pixels does not match a format in this collection, then the decoder will convert the
		///   decoded pixels to the format specific by the PreferredPixelFormat property.
		///  </para>
		/// </remarks>
		/// <value>Collection of pixel formats that the image decoder will support.</value>
		property PixelFormatSet^ SupportedPixelFormats { PixelFormatSet^ get(); }

	private:
		/// <summary>The maximum pixel width the resulting bitmap can be. Set to zero for no maximum.</summary>
		int fMaxPixelWidth;

		/// <summary>The maximum pixel height the resulting bitmap can be. Set to zero for no maximum.</summary>
		int fMaxPixelHeight;

		/// <summary>Set to true to apply premultiplied alpha to the RGB channels. Otherwise, set to false.</summary>
		bool fPremultiplyAlphaEnabled;

		/// <summary>
		///  The pixel format to use if the source image is of a format not contained in the
		///  "fSupportedPixelFormats" collection.
		/// </summary>
		PixelFormat^ fPreferredPixelFormat;

		/// <summary>Collection of all pixel formats the owner of the image decoder supports.</summary>
		PixelFormatSet^ fSupportedPixelFormats;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
