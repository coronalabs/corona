//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "WinString.h"
#include <GdiPlus.h>


#pragma region Forward Declarations
namespace Interop { namespace Graphics {
	class Font;
} }
namespace Rtt {
	class WinFont;
}

#pragma endregion


namespace Interop { namespace Graphics {

/// <summary>Stores the configuration for one font such as its font name, size, and bold/italic style.</summary>
class FontSettings
{
	public:
		#pragma region Constructors/Destructors
		/// <summary>Creates a new font settings object.</summary>
		FontSettings();

		/// <summary>Destroys this object.</summary>
		virtual ~FontSettings();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Gets the font family name or font file name as a UTF-16 encoded string.</summary>
		/// <returns>
		///  <para>Returns the font name in UTF-16 form.</para>
		///  <para>Returns null or empty string if the font name was not set.</para>
		/// </returns>
		const wchar_t* GetUtf16Name() const;

		/// <summary>Gets the font family name or font file name as a UTF-8 encoded string.</summary>
		/// <returns>
		///  <para>Returns the font name in UTF-8 form.</para>
		///  <para>Returns null or empty string if the font name was not set.</para>
		/// </returns>
		const char* GetUtf8Name() const;

		/// <summary>Sets the font family name or font file name.</summary>
		/// <param name="name">
		///  <para>The font family name or font file name.</para>
		///  <para>Can be null or empty string, in which case the default system font should be used.</para>
		/// </param>
		void SetName(const wchar_t* name);

		/// <summary>Sets the font family name or font file name.</summary>
		/// <param name="name">
		///  <para>The font family name or font file name.</para>
		///  <para>Can be null or empty string, in which case the default system font should be used.</para>
		/// </param>
		void SetName(const char* name);

		/// <summary>Determines whether or not the font should be rendered using a "bold" style.</summary>
		/// <returns>Returns true if the font should be bold. Returns false if not.</returns>
		bool IsBold() const;

		/// <summary>Sets whether or not the font should be rendered using a "bold" style.</summary>
		/// <param name="value">Set to true to use a bold font style. Set to false to not use a bold style.</param>
		void SetIsBold(bool value);

		/// <summary>Determines whether or not the font should be rendered using an "italic" style.</summary>
		/// <returns>Returns true if the font should be italic. Returns false if not.</returns>
		bool IsItalic() const;

		/// <summary>Sets whether or not the font should be rendered using an "italic" style.</summary>
		/// <param name="value">Set to true to use an italic font style. Set to false to not use an italic style.</param>
		void SetIsItalic(bool value);

		/// <summary>Gets the bold and italics font styles as a GDI+ font style enum type.</summary>
		/// <returns>Returns a GDI+ FontStyle enum type matching this settings object's bold and italic styles.</returns>
		Gdiplus::FontStyle GetGdiPlusStyle() const;

		/// <summary>Gets the font size in pixels.</summary>
		/// <returns>Returns the font size in pixels.</returns>
		float GetPixelSize() const;

		/// <summary>Sets the font size in pixels.</summary>
		/// <param name="value">The pixel size the font will render text with. Cannot be less than zero.</param>
		void SetPixelSize(float value);

		/// <summary>Copies the Win32 LOGFONT structure's settings to this object's settings.</summary>
		/// <param name="deviceContextHandle">
		///  Handle to a device context needed to calculate the LOGFONT's font size in pixels. Cannot be null.
		/// </param>
		/// <param name="logFont">The Win32 GDI LOGFONT structure to copy font settings from.</param>
		/// <returns>
		///  <para>Returns true if successfully copied the given LOGFONT settings.</para>
		///  <para>Returns false if given a null device context handle.</para>
		/// </returns>
		bool CopyFrom(HDC deviceContextHandle, const LOGFONTW& logFont);

		/// <summary>Copies the settings from the Win32 front to this settings object.</summary>
		/// <param name="deviceContextHandle">
		///  Handle to a device context needed to calculate the font object's size in pixels. Cannot be null.
		/// </param>
		/// <param name="logFont">The Win32 font object to copy font settings from.</param>
		/// <returns>
		///  <para>Returns true if successfully copied the given font object's settings.</para>
		///  <para>Returns false if given a null device context handle or if the given font object is invalid.</para>
		/// </returns>
		bool CopyFrom(HDC deviceContextHandle, const Font& font);

		/// <summary>Copies the given Corona Rtt::Font settings to this settings object.</summary>
		/// <param name="font">The Corona font object to copy settings from.</param>
		/// <param name="units">
		///  Species the units the given object's font size is measured in such as UnitPixel, UnitPoint, etc.
		/// </param>
		void CopyFrom(const Rtt::WinFont& font, Gdiplus::Unit units);

		/// <summary>Copies this object's settings to the given Corona Rtt::Font object.</summary>
		/// <param name="font">The font object to copy this object's settings to.</param>
		void CopyTo(Rtt::WinFont& font) const;

		#pragma endregion

	private:
		#pragma region Private Member Variables
		WinString fName;
		bool fIsBold;
		bool fIsItalic;
		float fPixelSize;

		#pragma endregion
};

} }	// namespace Interop::Graphics
