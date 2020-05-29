//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MFontLoader.h"
#include <GdiPlus.h>
#include <unordered_map>
#include <string>
#include <Windows.h>


namespace Interop { namespace Graphics {

/// <summary>Provides font loading and accessing features.</summary>
class FontServices : public MFontLoader
{
	public:
		#pragma region Constructors/Destructors
		/// <summary>Creates a new font services object.</summary>
		FontServices();

		/// <summary>Destroys the loaded FontFamily objects and other allocated resources.</summary>
		virtual ~FontServices();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>
		///  <para>Loads the given font file and returns a GDI+ FontFamily object used for text rendering.</para>
		///  <para>
		///   The font will be loaded privately by this class and will not be visible via the
		///   Win32 EnumFont() related C functions.
		///  </para>
		///  <para>The returned FontFamily object will exist for the lifetime of this FontServices object.</para>
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
		virtual Gdiplus::FontFamily* LoadFromFile(const wchar_t* filePath) override;

		/// <summary>Loads a GDI+ font using the given font settings.</summary>
		/// <param name="fontSettings">Provides the name, size, and styles of the font to be loaded.</param>
		/// <returns>Returns a shared pointer to a GDI+ font for the given font settings.</returns>
		virtual std::shared_ptr<Gdiplus::Font> LoadUsing(const FontSettings& fontSettings) override;

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
		virtual Gdiplus::FontFamily* FetchPrivateLoadedFontByName(const wchar_t* name) const override;

		/// <summary>
		///  <para>Safely fetches the GDI+ font famliy from the given GDI+ font.</para>
		///  <para>
		///   This works-around a severe Microsoft bug/crasher with the Gdiplus::Font::GetFamily() method.
		///   Never call that method! It's dangerous!
		///  </para>
		/// </summary>
		/// <param name="font">The GDI+ font to fetch the font family information from.</param>
		/// <returns>
		///  <para>Returns a shared pointer to a GDI+ font family that the given font is using.</para>
		///  <para>Returns null if the given font object is invalid. (ie: It's IsAvailable() method returns false.)</para>
		/// </returns>
		std::shared_ptr<Gdiplus::FontFamily> GetFamilyFrom(const Gdiplus::Font& font) const;

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>Copy constructor made private to prevent copies from being made.</summary>
		FontServices(const FontServices &value) {}

		/// <summary>Copy operator made private to prevent copies from being made.</summary>
		void operator=(const FontServices &value) {}

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>
		///  Defines a hash table collection type using a string for the key and a
		///  GDI+ FontFamily object pointer for the value.
		/// </summary>
		typedef std::unordered_map<std::wstring, Gdiplus::FontFamily*> StringFontFamilyMap;

		/// <summary>Defines a [wstring/FontFamily] pair for the "StringFontFamilyMap" type.</summary>
		typedef std::pair<std::wstring, Gdiplus::FontFamily*> StringFontFamilyPair;

		/// <summary>
		///  <para>Fast lookup table used to fetch loaded GDI+ FontFamily objects by their font file path.</para>
		///  <para>All font file path strings are expected to be in lower-case form.</para>
		///  <para>
		///   It is okay for multiple entries to reference the same FontFamily object, which handles
		///   the case where different paths reference the same font file.
		///  </para>
		/// </summary>
		StringFontFamilyMap fFontFilePathMap;

		/// <summary>
		///  <para>Fast lookup table used to fetch loaded GDI+ FontFamily objects by their font family name.</para>
		///  <para>
		///   The FontFamily objects stored in this table were allocated by this class and are expected
		///   to be deleted by this class' destructor.
		///  </para>
		/// </summary>
		StringFontFamilyMap fFontFamilyNameMap;

		/// <summary>
		///  <para>Microsoft GDI+ token received from the GdiplusStartup() function.</para>
		///  <para>Must be passed into the GdiplusShutdown() function when the runtime is terminated.</para>
		///  <para>Will be zero if GDI+ has not be started yet or it has already been shutdown.</para>
		/// </summary>
		ULONG_PTR fGdiPlusToken;

		#pragma endregion
};

} }	// namespace Interop::Graphics
