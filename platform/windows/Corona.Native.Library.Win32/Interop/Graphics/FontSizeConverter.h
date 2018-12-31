//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Windows.h>
#include <GdiPlus.h>


namespace Interop { namespace Graphics {

/// <summary>Converts a font size from one unit of measure to another. Such as from points to pixels.</summary>
class FontSizeConverter
{
	public:
		/// <summary>Creates a new converter.</summary>
		FontSizeConverter();

		/// <summary>Creates a new converter initialized with the given font size and units.</summary>
		/// <param name="size">The font size to be converted. Can be less than or equal to zero.</param>
		/// <param name="unit">The units the font size is measure in such as UnitPixel, UnitPoint, etc.</param>
		FontSizeConverter(float size, Gdiplus::Unit unit);

		/// <summary>Destroys this converter.</summary>
		virtual ~FontSizeConverter();

		/// <summary>
		///  <para>Sets the font size to be converted.</para>
		///  <para>The GetSize() and GetUnit() methods will return these values after calling this method.</para>
		/// </summary>
		/// <param name="size">The font size to be converted. Can be less than or equal to zero.</param>
		/// <param name="unit">The units the font size is measure in such as UnitPixel, UnitPoint, etc.</param>
		void SetSize(float size, Gdiplus::Unit unit);

		/// <summary>
		///  <para>Sets the font size to be converted from the given LOGFONT structure.</para>
		///  <para>The GetSize() and GetUnit() methods will return these values if this method returns true.</para>
		/// </summary>
		/// <param name="deviceContextHandle">
		///  Device context needed to convert the given LOGFONT struct's logical units to points.
		/// </param>
		/// <param name="logFont">The GDI LOGFONT structure to fetch the font size from.</param>
		/// <returns>
		///  <para>Returns true if successully read-in the font size from the given LOGFONT.</para>
		///  <para>Returns false if the given arguments are invalid and the font size was not successfully read.</para>
		/// </returns>
		bool SetSizeUsing(HDC deviceContextHandle, const LOGFONTW& logFont);

		/// <summary>
		///  Gets the font size assigned to this converter via SetFont() or the ConvertTo() method's converted result.
		/// </summary>
		/// <returns>Returns the font size.</returns>
		float GetSize() const;

		/// <summary>Gets the font size units given to SetFont() or ConvertTo().</summary>
		/// <returns>Returns the font size units such as UnitPixel, UnitPoint, etc.</returns>
		Gdiplus::Unit GetUnit() const;

		/// <summary>
		///  <para>Converts this object's current font size and units to the given units.</para>
		///  <para>The GetSize() and GetUnit() methods will provide the converted results.</para>
		/// </summary>
		/// <param name="unit">The units to convert to.</param>
		void ConvertTo(Gdiplus::Unit unit);

	private:
		float fDpi;
		float fSize;
		Gdiplus::Unit fUnit;
};

} }	// namespace Interop::Graphics
