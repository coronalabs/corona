//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
