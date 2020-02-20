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

/// <summary>Indicates if text/graphics should be horizontally aligned to the left, center, or right.</summary>
/// <remarks>
///  <para>
///   You cannot create instances of this class. Instead, you access pre-defined HorizontalAlignment objects
///   via this class' static properties like you would with an enum.
///  </para>
///  <para>Instances of this class are immutable.</para>
/// </remarks>
public ref class HorizontalAlignment sealed
{
	private:
		/// <summary>Creates a new alignment object with the given string ID.</summary>
		/// <remarks>
		///  This is a private constructor that is used to create the predefined alignment objects this class
		///  makes available publicly via its static properties.
		/// </remarks>
		/// <param name="coronaStringId">
		///  <para>Unique string ID used by Corona such as "left", "center", or "right".</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		HorizontalAlignment(Platform::String^ coronaStringId);

	public:
		/// <summary>Gets Corona's unique string ID for this alignment type.</summary>
		/// <value>
		///  Corona's unique string ID for this alignment type such as "left", "center", or "right".
		///  Used in Lua scripts when setting up alignment for text objects or text fields.
		/// </value>
		property Platform::String^ CoronaStringId { Platform::String^ get(); }
		Platform::String^ ToString();

		/// <summary>Indicates that the text/graphics should be left aligned.</summary>
		static property HorizontalAlignment^ Left { HorizontalAlignment^ get(); }

		/// <summary>Indicates that the text/graphics should be centered.</summary>
		static property HorizontalAlignment^ Center { HorizontalAlignment^ get(); }

		/// <summary>Indicates that the text/graphics should be right aligned.</summary>
		static property HorizontalAlignment^ Right { HorizontalAlignment^ get(); }

		/// <summary>Gets a read-only collection of all alignment types provided by this class.</summary>
		/// <value>Read-only collection of all alignment types provided by this class.</value>
		static property Windows::Foundation::Collections::IIterable<HorizontalAlignment^>^ Collection
		{
			Windows::Foundation::Collections::IIterable<HorizontalAlignment^>^ get();
		}

		/// <summary>Gets an alignment object matching the given Corona string ID.</summary>
		/// <param name="stringId">
		///  Unique string ID for the alignment such as "left", "center", or "right".
		///  This string ID typically comes from Lua when setting an alignment to a text object or native text field.
		/// </param>
		/// <returns>
		///  <para>Returns the alignment object matching the given string ID.</para>
		///  <para>Returns null if the given string ID is unknown.</para>
		/// </returns>
		static HorizontalAlignment^ FromCoronaStringId(Platform::String^ stringId);

	internal:
		/// <summary>Gets an alignment object matching the given Corona string ID.</summary>
		/// <param name="stringId">
		///  Unique string ID for the alignment such as "left", "center", or "right".
		///  This string ID typically comes from Lua when setting an alignment to a text object or native text field.
		/// </param>
		/// <returns>
		///  <para>Returns the alignment object matching the given string ID.</para>
		///  <para>Returns null if the given string ID is unknown.</para>
		/// </returns>
		static HorizontalAlignment^ FromCoronaStringId(const char *stringId);

	private:
		/// <summary>Gets a private mutable collection used to store all alignment types provided by this class.</summary>
		/// <remarks>
		///  This is used by this class' constructor to automatically store all pre-defined alignment objects
		///  to be used by this class' static From() methods to do the equivalent of a Java/.NET reflection lookup
		///  for pre-existing objects.
		/// </remarks>
		static property Platform::Collections::Vector<HorizontalAlignment^>^ MutableCollection
		{
			Platform::Collections::Vector<HorizontalAlignment^>^ get();
		}

		static const HorizontalAlignment^ kLeft;
		static const HorizontalAlignment^ kCenter;
		static const HorizontalAlignment^ kRight;

		Platform::String^ fCoronaStringId;
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
