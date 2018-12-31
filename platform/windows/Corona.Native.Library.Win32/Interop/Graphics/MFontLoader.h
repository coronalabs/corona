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

#include "FontSettings.h"
#include <memory>
#include <Windows.h>
#include <GdiPlus.h>


namespace Interop { namespace Graphics {

/// <summary>Abstract class providing font loading features.</summary>
class MFontLoader
{
	public:
		/// <summary>Destroys this object.</summary>
		virtual ~MFontLoader() {}

		/// <summary>
		///  <para>Loads the given font file and returns a GDI+ FontFamily object used for text rendering.</para>
		///  <para>
		///   The font will be loaded privately by this class and will not be visible via the
		///   Win32 EnumFont() related C functions.
		///  </para>
		///  <para>The returned FontFamily object will exist for the lifetime of this font provider object.</para>
		///  <para>
		///   If the given font file has already been loaded by this class instance, then the last FontFamily
		///   object will be returned instead for best performance.
		///  </para>
		/// </summary>
		/// <param name="filePath">>Path to the font file to be loaded. Only supports *.ttf and *.otf files.</param>
		/// <returns>
		///  <para>Returns a pointer to a GDI+ FontFamily object for the loaded font file.</para>
		///  <para>Returns null if failed to load the given font file or given a null/empty path string.</para>
		/// </returns>
		virtual Gdiplus::FontFamily* LoadFromFile(const wchar_t* filePath) = 0;

		/// <summary>Loads a GDI+ font using the given font settings.</summary>
		/// <param name="fontSettings">Provides the name, size, and styles of the font to be loaded.</param>
		/// <returns>Returns a shared pointer to a GDI+ font for the given font settings.</returns>
		virtual std::shared_ptr<Gdiplus::Font> LoadUsing(const FontSettings& fontSettings) = 0;

		/// <summary>
		///  <para>Fetches a GDI+ font family that was privately loaded from file by its font family name.</para>
		///  <para>Font files can be privately lodaed via the LoadFromFile() and LoadUsing() methods.</para>
		///  <para>Fonts loaded privately will not be visible via the Win32 EnumFont() related C functions.</para>
		///  <para>The returned FontFamily object will exist for the lifetime of this FontServices object.</para>
		///  <para>This method will not return FontFamily objects for installed fonts.</para>
		/// </summary>
		/// <param name="name">The font family name to search for.</param>
		/// <returns>
		///  <para>Returns a pointer to a GDI+ FontFamily object for the given font family name.</para>
		///  <para>Returns null if the given family name was not privately loaded or if given a null/empty string.</para>
		/// </returns>
		virtual Gdiplus::FontFamily* FetchPrivateLoadedFontByName(const wchar_t* name) const = 0;
};

} }	// namespace Interop::Graphics
