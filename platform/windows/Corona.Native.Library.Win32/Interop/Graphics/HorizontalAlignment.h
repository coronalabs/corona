//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <gdiplus.h>


namespace Interop { namespace Graphics {

/// <summary>Indicates if text/graphics should be horizontally aligned to the left, center, or right.</summary>
/// <remarks>
///  <para>
///   You cannot create instances of this class. Instead, you access pre-defined HorizontalAlignment objects
///   via this class' static constants like you would with an enum.
///  </para>
///  <para>Instances of this class are immutable.</para>
/// </remarks>
class HorizontalAlignment
{
	private:
		/// <summary>Calling the empty constructor will cause an exception to be thrown.</summary>
		HorizontalAlignment();

		/// <summary>Creates a new alignment object with the given ID pair.</summary>
		/// <remarks>
		///  This is a private constructor that is used to create the predefined alignment objects this class
		///  makes available publicly via its static constants.
		/// </remarks>
		/// <param name="coronaStringId">
		///  <para>Unique string ID used by Corona such as "left", "center", or "right".</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		/// <param name="gdiPlusAlignmentId">The GDI+ enum ID for this alignment.</param>
		/// <param name="gdiEditControlStyle">
		///  A Win32 style for an "EDIT" control such as ES_LEFT, ES_CENTER, or ES_RIGHT.
		/// </param>
		/// <param name="gdiDrawTextFormatId">GDI format alignment ID used by the Win32 Drawtext() function.</param>
		HorizontalAlignment(
				const char* coronaStringId, Gdiplus::StringAlignment gdiPlusAlignmentId,
				LONG gdiEditControlStyle, UINT gdiDrawTextFormatId);

		/// <summary>Destroys this object.</summary>
		virtual ~HorizontalAlignment();

	public:
		/// <summary>Indicates that the text/graphics should be left aligned.</summary>
		static const HorizontalAlignment kLeft;

		/// <summary>Indicates that the text/graphics should be centered.</summary>
		static const HorizontalAlignment kCenter;

		/// <summary>Indicates that the text/graphics should be right aligned.</summary>
		static const HorizontalAlignment kRight;

		/// <summary>Gets Corona's unique string ID for this alignment type.</summary>
		/// <returns>
		///  Returns Corona's unique string ID for this alignment type such as "left", "center", or "right".
		///  Used in Lua scripts when setting up alignment for text objects or text fields.
		/// </returns>
		const char* GetCoronaStringId() const;

		/// <summary>Gets the GDI+ unique enum ID for this alignment type.</summary>
		/// <returns>Returns the GDI+ enum ID for this alignment.</returns>
		Gdiplus::StringAlignment GetGdiPlusStringAlignmentId() const;

		/// <summary>Gets a GDI "EDIT" control style for this alignment such as ES_LEFT, ES_CENTER, or ES_RIGHT.</summary>
		/// <returns>Returns a GDI "EDIT" control style such as ES_LEFT, ES_CENTER, or ES_RIGHT.</returns>
		LONG GetGdiEditControlStyle() const;

		/// <summary>
		///  Gets a GDI format alignment ID that can be used by the Win32 DrawText() function
		///  such as DT_LEFT, DT_CENTER, or DT_RIGHT.
		/// </summary>
		/// <returns>Returns GDI DrawText() format alignment ID such as DT_LEFT, DT_CENTER, or DT_RIGHT.</returns>
		UINT GetGdiDrawTextFormatId() const;

		/// <summary>Determines if this alignment type matches the given one.</summary>
		/// <param name="value">The alignment type to be compared with.</param>
		/// <returns>Return true if the alignment types match. Returns false if not.</returns>
		bool Equals(const HorizontalAlignment& value) const;

		/// <summary>Determines if this alignment type does not match the given one.</summary>
		/// <param name"value">The alignment type to be compared with.</param>
		/// <returns>Return true if the alignment types do not match. Returns false if they do match.</returns>
		bool NotEquals(const HorizontalAlignment& value) const;

		/// <summary>Determines if this alignment type matches the given one.</summary>
		/// <param name="value">The alignment type to be compared with.</param>
		/// <returns>Return true if the alignment types match. Returns false if not.</returns>
		bool operator==(const HorizontalAlignment& value) const;

		/// <summary>Determines if this alignment type does not match the given one.</summary>
		/// <param name"value">The alignment type to be compared with.</param>
		/// <returns>Return true if the alignment types do not match. Returns false if they do match.</returns>
		bool operator!=(const HorizontalAlignment& value) const;

		/// <summary>Fetches a predefined HorizontalAlignment object matching the given unique Corona string ID.</summary>
		/// <param name="coronaStringId">The unique Corona string ID such as "left", "center", or "right".</param>
		/// <returns>
		///  <para>Returns a pointer to the matching HorizontalAlignment object such as kLeft, kCenter, or kRight.</para>
		///  <para>Returns null if the given string ID was not recognized.</para>
		/// </returns>
		static const HorizontalAlignment* FromCoronaStringId(const char* coronaStringId);

		/// <summary>
		///  Fetches a predefined HorizontalAlignment object matching the given GDI edit control's current aignment.
		/// </summary>
		/// <param name="windowHandle">
		///  <para>The window handle to the edit control to query for its alignment.</para>
		///  <para>This function will determine if its ES_LEFT, ES_CENTER, or ES_RIGHT style is set.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a pointer to a HorizontalAlignment object for the edit control's current alignment.</para>
		///  <para>Returns null given a null handle or if the edit control is no longer available.</para>
		/// </returns>
		static const HorizontalAlignment* FromGdiEditControl(HWND windowHandle);

		/// <summary>Fetches a predefined HorizontalAlignment object matching the given GDI edit control's style.</summary>
		/// <param name="value">
		///  <para>The edit control's style flags retrieved via the GetWindowLong() or GetWindowLongPtr() functions.</para>
		///  <para>This function will determine if the ES_LEFT, ES_CENTER, or ES_RIGHT style is set.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a pointer to a HorizontalAlignment object for the edit control's current alignment.</para>
		///  <para>Returns null if a match HorizontalAlignment type was not found. (Should never happen.)</para>
		/// </returns>
		static const HorizontalAlignment* FromGdiEditControlStyles(LONG value);

		/// <summary>Fetches a predefined HorizontalAlignment object matching the given GDI+ alignment enum ID.</summary>
		/// <param name="value">
		///  The GDI+ alignment enum ID such as StringAlignmentNear, StringAlignmentCenter, or StringAlignmentFar.
		/// </param>
		/// <returns>
		///  <para>Returns a pointer to the matching HorizontalAlignment object such as kLeft, kCenter, or kRight.</para>
		///  <para>Returns null if a match HorizontalAlignment type was not found.</para>
		/// </returns>
		static const HorizontalAlignment* From(Gdiplus::StringAlignment value);

	private:
		/// <summary>The unique Corona string ID for this alignment.</summary>
		const char* fCoronaStringId;

		/// <summary>The GDI+ enum ID for this alignment.</summary>
		Gdiplus::StringAlignment fGdiPlusAlignmentId;

		/// <summary>GDI "EDIT" control style for this alignment.</summary>
		LONG fGdiEditControlStyle;

		/// <summary>GDI format alignment ID used by the Win32 Drawtext() function.</summary>
		UINT fGdiDrawTextFormatId;
};

} }	// namespace Interop::Graphics
