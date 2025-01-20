//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
