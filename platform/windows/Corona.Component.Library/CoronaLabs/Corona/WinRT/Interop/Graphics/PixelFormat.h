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


#include <collection.h>
#pragma warning(disable: Rtt_WIN_XML_COMMENT_COMPILER_WARNING_CODE)
#	include "Rtt_Build.h"
#	include "Display/Rtt_PlatformBitmap.h"
#pragma warning(default: Rtt_WIN_XML_COMMENT_COMPILER_WARNING_CODE)


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

/// <summary>Provides image pixel formats supported by Corona.</summary>
/// <remarks>
///  <para>
///   You cannot create instances of this class. Instead, you access pre-defined PixelFormat objects
///   via this class' static properties much like you would with an enum.
///  </para>
///  <para>Instances of this class are immutable.</para>
/// </remarks>
public ref class PixelFormat sealed
{
	private:
		/// <summary>Creates a new pixel format with the given settings.</summary>
		/// <remarks>This is a private constructor used to create this class's pre-defined pixel format objects.</remarks>
		/// <param name="coronaFormat">Corona's pixel format constant.</param>
		/// <param name="invariantName">Unique name to be returned by this pixel format object' ToString() method.</param>
		PixelFormat(Rtt::PlatformBitmap::Format coronaFormat, Platform::String^ invariantName);

	public:
		/// <summary>Gets the number of bytes one pixel consumes with this pixel format.</summary>
		/// <value>
		///  <para>The number of bytes one pixel consumes, such as 1 byte for grayscale or 4 bytes for RGBA.</para>
		///  <para>Returns 0 if the pixel format is undefined.</para>
		/// </value>
		property int ByteCount { int get(); }

		/// <summary>Gets an integer ID matching one of Corona's internal pixel format constants.</summary>
		/// <returns>Returns Corona's unique integer ID for this pixel format.</returns>
		int ToCoronaIntegerId();
		
		/// <summary>Gets the name of this pixel format.</summary>
		/// <returns>Returns the invariant (non-localized) name of this pixel format, such as "Grayscale" or "RGBA".</returns>
		Platform::String^ ToString();

		/// <summary>Represents an undefined pixel format.</summary>
		static property PixelFormat^ Undefined { PixelFormat^ get(); }

		/// <summary>The 8-bit grayscale pixel format.</summary>
		static property PixelFormat^ Grayscale { PixelFormat^ get(); }

		/// <summary>The 32-bit color pixel format supporting alpha.</summary>
		static property PixelFormat^ RGBA { PixelFormat^ get(); }

		/// <summary>The 32-bit color pixel format supporting alpha.</summary>
		static property PixelFormat^ BGRA { PixelFormat^ get(); }

		/// <summary>Gets a read-only collection of all pixel formats supported by Corona, including the undefined format.</summary>
		/// <value>Read-only collection of all known pixel format supported by Corona.</value>
		static property Windows::Foundation::Collections::IIterable<PixelFormat^>^ Collection
							{ Windows::Foundation::Collections::IIterable<PixelFormat^>^ get(); }

		/// <summary>Fetches a pixel format matching the given unique Corona integer ID.</summary>
		/// <param name="id">Unique integer ID defined by Corona for the requested pixel format.</param>
		/// <returns>
		///  <para>Returns a pixel format matching the given ID.</para>
		///  <para>Returns null if the given ID is unknown.</para>
		/// </returns>
		static PixelFormat^ FromCoronaIntegerId(int id);

	internal:
		/// <summary>Gets the unique ID for this pixel format matching a constant in Corona's PlatformBitmap::Format enum.</summary>
		/// <remarks>This is an internal method that is only accessible to Corona Labs.</remarks>
		/// <value>Unique ID matching a constant in Corona's PlatformBitmap::Format enum.</value>
		property Rtt::PlatformBitmap::Format CoronaBitmapFormat { Rtt::PlatformBitmap::Format get(); }

		/// <summary>Fetches a pixel format matching the given Corona constant.</summary>
		/// <remarks>This is an internal method that is only accessible to Corona Labs.</remarks>
		/// <param name="format">The pixel format to look for.</param>
		/// <returns>
		///  <para>Returns a pixel format matching the given Corona constant.</para>
		///  <para>Returns null if a matching PixelFormat object could not be found.</para>
		/// </returns>
		static PixelFormat^ From(Rtt::PlatformBitmap::Format format);

	private:
		/// <summary>Gets a private mutable collection of all known pixel formats.</summary>
		/// <remarks>
		///  This is used by this class' constructor to automatically store all pre-defined pixel format objects
		///  to be used by this class' static From() methods to do the equivalent of a Java/.NET reflection lookup
		///  for a pre-existing object.
		/// </remarks>
		static property Platform::Collections::Vector<PixelFormat^>^ MutableCollection
							{ Platform::Collections::Vector<PixelFormat^>^ get(); }

		static const PixelFormat^ kUndefined;
		static const PixelFormat^ kGrayscale;
		static const PixelFormat^ kRGBA;
		static const PixelFormat^ kBGRA;

		Rtt::PlatformBitmap::Format fCoronaBitmapFormat;
		Platform::String^ fInvariantName;
		int fByteCount;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
