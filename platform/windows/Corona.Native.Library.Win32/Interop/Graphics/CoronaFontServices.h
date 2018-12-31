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

#include "FontServices.h"


#pragma region Forward Declarations
namespace Interop
{
	class RuntimeEnvironment;
}

#pragma endregion


namespace Interop { namespace Graphics {

/// <summary>
///  <para>Provides font loading and accessing features.</para>
///  <para>Supports loading fonts from Corona's resource directory and the ability to load device simualted fonts.</para>
/// </summary>
class CoronaFontServices : public FontServices
{
	public:
		#pragma region Constructors/Destructors
		/// <summary>Creates a new font services object.</summary>
		/// <param name="environment">
		///  The Corona runtime environment used to load fonts from Corona's standard sandbox directories.
		/// </param>
		CoronaFontServices(Interop::RuntimeEnvironment& environment);

		/// <summary>Destroys the loaded FontFamily objects and other allocated resources.</summary>
		virtual ~CoronaFontServices();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Gets the Corona runtime environment this font services object is associated with.</summary>
		/// <returns>Returns a reference to the Corona runtime environment.</returns>
		Interop::RuntimeEnvironment& GetRuntimeEnvironment() const;

		/// <summary>Loads a GDI+ font using the given font settings.</summary>
		/// <param name="fontSettings">Provides the name, size, and styles of the font to be loaded.</param>
		/// <returns>Returns a shared pointer to a GDI+ font for the given font settings.</returns>
		virtual std::shared_ptr<Gdiplus::Font> LoadUsing(const FontSettings& fontSettings) override;

		#pragma endregion

	private:
		#pragma region Private Member Variables
		/// <summary>Reference to the Corona runtime environment that this font handler is associated with.</summary>
		Interop::RuntimeEnvironment& fEnvironment;

		#pragma endregion
};

} }	// namespace Interop::Graphics
