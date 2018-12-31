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

#include "Interop\ValueResult.h"
#include <memory>
#include <Windows.h>
#include <GdiPlus.h>


#pragma region Forward Declarations
namespace Interop { namespace Graphics {
	class FontSettings;
	class MFontLoader;
} }

#pragma endregion


namespace Interop { namespace Graphics {

/// <summary>
///  <para>Provides native Win32 GDI and GDI+ font objects for the given font settings.</para>
///  <para>This class was designed to be passed by value and used by multipled text handlers.</para>
///  <para>
///   This class' stored GDI/GDI+ resources are reference counted and will be deleted once all instances
///   of the same "Font" object are dereferenced.
///  </para>
/// </summary>
class Font
{
	public:
		#pragma region Public LogFontResult Class
		/// <summary>
		///  <para>The result object type returned by the Font::CreateLogFontUsing() method.</para>
		///  <para>
		///   If the HasSucceeded() method returns true, then the GetValue() method will provide
		///   the created LOGFONT struct.
		///  </para>
		/// </summary>
		typedef Interop::ValueResult<LOGFONTW> LogFontResult;

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates an invalid object that does not provide any GDI/GDI+ font objects.</summary>
		Font();

		/// <summary>Creates a new font using the given font settings and font loader.</summary>
		/// <param name="deviceContextHandle">
		///  <para>Handle to a device context needed to generate a GDI font resource.</para>
		///  <para>The new font object will be flagged as invalid if this handle is set to null.</para>
		/// </param>
		/// <param name="fontLoader">
		///  <para>Customizable font loader used by this class to load a native font.</para>
		///  <para>It's LoadUsing() method will be called with the given "fontSettings" parameter.</para>
		/// </param>
		/// <param name="fontSettings">Provides the font name, size, bold/italic styles, etc.</param>
		Font(HDC deviceContextHandle, MFontLoader& fontLoader, const FontSettings& fontSettings);

		/// <summary>Creates a new font object wrapping the given GDI+ font object.</summary>
		/// <param name="deviceContextHandle">
		///  <para>Handle to a device context needed to generate a GDI font resource.</para>
		///  <para>The new font object will be flagged as invalid if this handle is set to null.</para>
		/// </param>
		/// <param name="fontPointer">
		///  <para>Shared pointer to a GDI+ font this object will wrap.</para>
		///  <para>The new font object will be flagged as invalid if the given shared pointer is null.</para>
		/// </param>
		Font(HDC deviceContextHandle, const std::shared_ptr<Gdiplus::Font>& fontPointer);

		/// <summary>Creates a new font object from the given GDI LOGFONT struct settings.</summary>
		/// <param name="deviceContextHandle">
		///  <para>Handle to a device context needed to generate a GDI font resource.</para>
		///  <para>The new font object will be flagged as invalid if this handle is set to null.</para>
		/// </param>
		/// <param name="logFont">LOGFONT struct to create a GDI and GDI+ font object from.</param>
		Font(HDC deviceContextHandle, const LOGFONTW& logFont);

		/// <summary>
		///  <para>Decrements this objects reference count with its GDI and GDI+ font resources.</para>
		///  <para>
		///   Once decremented to zero, the GDI and GDI+ font references returned by this object will be destroyed.
		///  </para>
		/// </summary>
		virtual ~Font();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Determines if this object stores a valid reference to a native GDI and GDI+ font.</summary>
		/// <returns>
		///  <para>Returns true if this object provides valid references to native GDI and GDI+ font objects.</para>
		///  <para>Returns false if this font object was created with invalid font settings.</para>
		/// </returns>
		bool IsValid() const;

		/// <summary>Determines if this object stores does not provide valid reference to native GDI and GDI+ fonts.</summary>
		/// <returns>
		///  <para>Returns true if this font object was created with invalid font settings.</para>
		///  <para>Returns false if this object provides valid references to native GDI and GDI+ font objects.</para>
		/// </returns>
		bool IsInvalid() const;

		/// <summary>Gets a handle to a native GDI font object.</summary>
		/// <returns>
		///  <para>Returns a handle to a native GDI font object.</para>
		///  <para>
		///   Returns null if this font object is invalid, which can happen if the constructor was given
		///   invalid font settings or if it failed to load a font.
		///  </para>
		/// </returns>
		HFONT GetFontHandle() const;

		/// <summary>Gets a pointer to a GDI+ font object.</summary>
		/// <returns>
		///  <para>Returns a pointer to a native GDI+ font object.</para>
		///  <para>
		///   Returns null if this font object is invalid, which can happen if the constructor was given
		///   invalid font settings or if it failed to load a font.
		///  </para>
		/// </returns>
		const Gdiplus::Font* GetGdiPlusFont() const;

		/// <summary>
		///  Creates a new native GDI LOGFONT struct based on this object's font settings
		///  and for the given device context.
		/// </summary>
		/// <param name="deviceContextHandle">
		///  <para>Handle to a device context needed to generate the LOGFONT and to calculate a "logical" font size.</para>
		///  <para>This method will return a failure result if this handle is null.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a result object providing the new LOGFONT object if creation was successful.</para>
		///  <para>
		///   Returns a failure result if the given handle is null/invalid or if this font object is invalid.
		///   The result object's GetMessage() method will provide more details as to why it failed.
		///  </para>
		/// </returns>
		LogFontResult CreateLogFontUsing(HDC deviceContextHandle) const;

		#pragma endregion

	private:
		#pragma region Private GdiFont Class
		/// <summary>
		///  <para>Internal class used to store a native GDI font resource.</para>
		///  <para>
		///   Instances of this class are intended to be stored by the Font class' "fGdiFontPointer"
		///   shared pointer member variable.
		///  </para>
		/// </summary>
		class GdiFont
		{
			public:
				/// <summary>Creates an invalid object which provides a null HFONT handle.</summary>
				GdiFont();

				/// <summary>Creates a new GDI font using the given LOGFONT structure's settings.</summary>
				GdiFont(const LOGFONTW& logFont);

				/// <summary>Destroys this object's GDI font.</summary>
				virtual ~GdiFont();

				/// <summary>Gets a handle to the native GDI font resource.</summary>
				HFONT GetFontHandle() const;

				/// <summary>Creates a new GdiFont instance using the given GDI+ font settings.</summary>
				/// <param name="deviceContextHandle">
				///  <para>Handle to a device context needed to generate a GDI font resource.</para>
				///  <para>Will return null if this handle is set to null.</para>
				/// </param>
				/// <param name="gdiPlusFont">GDI+ font settings to be used to create the GDI font.</param>
				/// <returns>
				///  <para>Returns a new GDI font object wrapped in a shared pointer.</para>
				///  <para>Returns an invalid shared pointer if given invalid arguments.</para>
				/// </returns>
				static std::shared_ptr<GdiFont> CreateUsing(HDC deviceContextHandle, const Gdiplus::Font& gdiPlusFont);

			private:
				HFONT fHandle;
		};

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Shared pointer to a GDI+ font resource this font object wraps.</summary>
		std::shared_ptr<Gdiplus::Font> fGdiPlusFontPointer;

		/// <summary>Shared pointer to a GDI font resource this font object wraps.</summary>
		std::shared_ptr<GdiFont> fGdiFontPointer;

		#pragma endregion
};

} }	// namespace Interop::Graphics
