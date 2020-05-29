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
interface class IBitmap;
interface class IImageDecoder;
ref class PixelFormat;

/// <summary>
///  Class used to modify a bitmap's pixels such as converting its pixel format, apply premultiplied alpha,
///  and downsample the bitmap to fit within a maximum width and height constraint.
/// </summary>
/// <remarks>
///  For best performance and memory efficiency, you should call this class' Convert() function to modify
///  a bitmap's existing pixels, if possible. However, the Convert() function will return false when attempting
///  to convert to a pixel format of a different byte size (ie: RGBA to 8-bit Grayscale), in which case,
///  your only option is to create a new bitmap via this class' CreateBitmapFrom() function.
/// </remarks>
public ref class BitmapConverter sealed
{
	public:
		/// <summary>Creates a new bitmap converter.</summary>
		BitmapConverter();

		/// <summary>Gets or sets the maximum pixel width the given bitmap should be downsampled to.</summary>
		/// <remarks>
		///  <para>
		///   If the given bitmap is larger than this maximum, then this converter will have to create a new bitmap that
		///   is downsampled to fit. Otherwise, the resulting bitmap's dimensions will be the same as the given bitmap.
		///  </para>
		///  <para>
		///   When loading bitmaps as textures into Direct3D, this property should be set to the GPU's max texture size
		///   or else the bitmap will not be accepted by the GPU.
		///  </para>
		/// </remarks>
		/// <value>
		///  <para>The maximum pixel width the bitmap can be when converted.</para>
		///  <para>Set to zero if no maximum should be applied.</para>
		/// </value>
		property int MaxPixelWidth { int get(); void set(int value); }

		/// <summary>Gets or sets the maximum pixel height the given bitmap should be downsampled to.</summary>
		/// <remarks>
		///  <para>
		///   If the given bitmap is larger than this maximum, then this converter will have to create a new bitmap that
		///   is downsampled to fit. Otherwise, the resulting bitmap's dimensions will be the same as the given bitmap.
		///  </para>
		///  <para>
		///   When loading bitmaps as textures into Direct3D, this property should be set to the GPU's max texture size
		///   or else the bitmap will not be accepted by the GPU.
		///  </para>
		/// </remarks>
		/// <value>
		///  <para>The maximum pixel height the bitmap can be when converted.</para>
		///  <para>Set to zero if no maximum should be applied.</para>
		/// </value>
		property int MaxPixelHeight { int get(); void set(int value); }

		/// <summary>Gets or sets whether or not this convert should apply premultiplied alpha to the bitmap's pixels.</summary>
		/// <remarks>This property is ignored if the bitmap to be converted does not have an alpha channel.</remarks>
		/// <value>
		///  <para>Set to true to have the bitmap's alpha channel multiplied against the RGB channels for each pixel.</para>
		///  <para>Set to false to not multiply the alpha channel against the RGB channels.</para>
		/// </value>
		property bool PremultiplyAlphaEnabled { bool get(); void set(bool value); }

		/// <summary>
		///  Gets or sets the pixel format that a bitmap should be converted to when calling this converter's
		///  Convert() or CreateBitmapFrom() methods.
		/// </summary>
		/// <remarks>
		///  Note that if the bitmap to be converted has a different pixel format byte count than the format to
		///  be converted to, then the Convert() method cannot modify the given bitmap's bytes. In this case,
		///  a new bitmap has to be created by calling the CreateBitmapFrom() method.
		/// </remarks>
		/// <value>
		///  <para>The pixel format the given bitmap should be converted to, such as Grayscale or RGBA.</para>
		///  <para>Will throw an exception if set to null or Undefined.</para>
		/// </value>
		property CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat^ PixelFormat
		{
			CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat^ get();
			void set(CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat^ value);
		}

		/// <summary>
		///  Copies the given image decoder's maximum pixel width, height, and premultiplied alpha settings to this converter.
		/// </summary>
		/// <param name="decoder">
		///  <para>The image decoder to copy settings from.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		void CopySettingsFrom(IImageDecoder^ decoder);

		/// <summary>Determines if the given bitmap's bytes can be converted by this object's Convert() method.</summary>
		/// <returns>
		///  <para>Returns true if the given bitmap's bytes can be converted by the Convert() method.</para>
		///  <para>
		///   Returns false if the Convert() method cannot modify the given bitmap's bytes. This can happen when:
		///   <list type="bullet">
		///    <item>
		///     <description>The given bitmap has zero width and height.</description>
		///    </item>
		///    <item>
		///     <description>
		///      The given bitmap's pixel format has a different number of bytes than the pixel format this converter
		///      will be changing it to.
		///     </description>
		///    </item>
		///    <item>
		///     <description>The bitmap needs to be downsampled due to the maximum pixel width and height.</description>
		///    </item>
		///   </list>
		///   So, if this method returns false, then your only option is to call this converter's CreateBitmapFrom()
		///   method which generates a new bitmap formatted according to this converter's settings.
		///  </para>
		/// </returns>
		bool CanConvert(IBitmap^ bitmap);

		/// <summary>Converts the given bitmap's pixels using this converter's given settings.</summary>
		/// <param name="bitmap">Reference to the bitmap to be converted.</param>
		/// <returns>
		///  <para>Returns true if the given bitmap's pixels were successfully converted.</para>
		///  <para>
		///   Returns false if the given bitmap's pixels cannot be converted. This can happen when:
		///   <list type="bullet">
		///    <item>
		///     <description>
		///      The given bitmap's pixel format has a different number of bytes than the pixel format this converter
		///      will be changing it to.
		///     </description>
		///    </item>
		///    <item>
		///     <description>The bitmap needs to be downsampled due to the maximum pixel width and height.</description>
		///    </item>
		///    <item>
		///     <description>The given bitmap has zero width and height.</description>
		///    </item>
		///    <item>
		///     <description>The given bitmap is null.</description>
		///    </item>
		///   </list>
		///   So, if this method returns false, then your only option is to call this converter's CreateBitmapFrom()
		///   method which generates a new bitmap formatted according to this converter's settings.
		///  </para>
		/// </returns>
		bool Convert(IBitmap^ bitmap);

		/// <summary>
		///  Creates a new bitmap based on the given bitmap, formatted according to this converter's given settings.
		/// </summary>
		/// <param name="bitmap">The bitmap pixels to be used to create the new converted bitmap.</param>
		/// <returns>
		///  <para>Returns a new bitmap formatted according to this converter's given format settings.</para>
		///  <para>Returns a new empty bitmap if the given bitmap is empty.</para>
		///  <para>Returns null if the given bitmap's pixels have been freed or if given a null argument.</para>
		/// </returns>
		IBitmap^ CreateBitmapFrom(IBitmap^ bitmap);

	private:
		int fMaxPixelWidth;
		int fMaxPixelHeight;
		bool fPremultiplyAlphaEnabled;
		CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat^ fPixelFormat;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
