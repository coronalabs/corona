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

#include "Core\Rtt_Build.h"
#include "Core\Rtt_Real.h"
#include "Core\Rtt_String.h"
#include "Rtt_PlatformFont.h"

#pragma region Forward Declarations
namespace Interop
{
	class RuntimeEnvironment;
}

#pragma endregion


namespace Rtt
{

class WinFont : public PlatformFont
{
	public:
		typedef WinFont Self;
		typedef PlatformFont Super;

		/// <summary>Creates a new native Corona font configuration for Win32.</summary>
		/// <param name="environment">Corona runtime environment this font is associated with.</param>
		WinFont(Interop::RuntimeEnvironment& environment);

		/// <summary>Creates a new copy of the given native Corona font configuration.</summary>
		/// <param name="font">Objct to copy the font configuration from.</param>
		WinFont(const WinFont& font);

		/// <summary>Destroys this font object and its allocated resources.</summary>
		virtual ~WinFont();

		/// <summary>Creates a new copy of this native Corona font object.</summary>
		/// <param name="allocator">Pointer to an allocator needed to create the object. Cannot be null.</param>
		/// <returns>
		///  <para>Returns a new Rtt::WinFont object that is copy of this object.</para>
		///  <para>Returns null if given a null allocator.</para>
		/// </returns>
		virtual PlatformFont* CloneUsing(Rtt_Allocator *allocator) const
		{
			if (!allocator)
			{
				return nullptr;
			}
			return Rtt_NEW(allocator, WinFont(*this));
		}

		/// <summary>Sets the font family name or font file path to be used.</summary>
		/// <param name="name">
		///  <para>The font family name or font file name.</para>
		///  <para>Can be null or empty string, in which case the default system font should be used.</para>
		/// </param>
		void SetName(const char* name);

		/// <summary>Gets the font family name or font file name.</summary>
		/// <returns>
		///  <para>Returns the font name or font file path.</para>
		///  <para>Returns null or empty string if the font name was not set.</para>
		/// </returns>
		virtual const char* Name() const;

		/// <summary>Sets the font size.</summary>
		/// <remarks>
		///  The font size units are context sensitive and depend on what feature this font object will be used with.
		///  For example, display.newText() always interprets font size in Corona's scaled point system.
		///  But native TextFields and TextBoxes can interpret it in native points if their "isFontSizeScaled"
		///  property is set to false.
		/// </remarks>
		/// <param name="value">
		///  <para>The font size text will be rendered with.</para>
		///  <para>Set to less than or equal to zero to use the system's default font size.</para>
		/// </param>
		virtual void SetSize(Real value);

		/// <summary>Gets the font size.</summary>
		/// <remarks>
		///  The font size units are context sensitive and depend on what feature this font object will be used with.
		///  For example, display.newText() always interprets font size in Corona's scaled point system.
		///  But native TextFields and TextBoxes can interpret it in native points if their "isFontSizeScaled"
		///  property is set to false.
		/// </remarks>
		/// <returns>
		///  <para>Returns the font size to render text with.</para>
		///  <para>Returns a value less than or equal to zero if the font should use the system's default font size.</para>
		/// </returns>
		virtual Rtt_Real Size() const;

		/// <summary>Sets whether or not the font should use a bold style.</summary>
		/// <param name="value">Set true to use a bold style. Set false to not use this style.</param>
		void SetBold(bool value);

		/// <summary>Determines if the font should use a bold style.</summary>
		/// <returns>
		///  <para>Returns true if the font should use a bold style.</para>
		///  <para>Returns false to not use a bold style, which is the default.</para>
		/// </returns>
		bool IsBold() const;

		/// <summary>Sets whether or not the font should use an italic style.</summary>
		/// <param name="value">Set true to use an italic style. Set false to not use this style.</param>
		void SetItalic(bool value);

		/// <summary>Determines if the font should use an italic style.</summary>
		/// <returns>
		///  <para>Returns true if the font should use an italic style.</para>
		///  <para>Returns false to not use an italic style, which is the default.</para>
		/// </returns>
		bool IsItalic() const;

		/// <summary>
		///  <para>This function is not supported on Windows.</para>
		///  <para>
		///   You must use the Interop::Graphics::FontSettings and Interop::Graphics::FontServices derived class
		///   to generate a native Win32 GDI or GDI+ font object.
		///  </para>
		/// </summary>
		/// <returns>Always returns null.</returns>
		virtual void* NativeObject() const;

	private:
		Interop::RuntimeEnvironment& fEnvironment;
		String fName;
		Rtt_Real fSize;
		bool fIsBold;
		bool fIsItalic;
};

} // namespace Rtt
