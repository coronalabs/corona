//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CoronaFontServices.h"
#include "Core\Rtt_Build.h"
#include "Core\Rtt_String.h"
#include "Corona\CoronaLog.h"
#include "Interop\RuntimeEnvironment.h"


namespace Interop { namespace Graphics {

#pragma region Constructors/Destructors
CoronaFontServices::CoronaFontServices(Interop::RuntimeEnvironment& environment)
:	FontServices(),
	fEnvironment(environment)
{
}

CoronaFontServices::~CoronaFontServices()
{
}

#pragma endregion


#pragma region Public Methods
Interop::RuntimeEnvironment& CoronaFontServices::GetRuntimeEnvironment() const
{
	return fEnvironment;
}

std::shared_ptr<Gdiplus::Font> CoronaFontServices::LoadUsing(const FontSettings& fontSettings)
{
	// Attempt to load the requested font.
	bool wasFontNameLoaded = false;
	bool wasFontFamilyAllocated = false;
	const Gdiplus::FontFamily* fontFamilyPointer = nullptr;
	if (fontSettings.GetUtf16Name() && (fontSettings.GetUtf16Name()[0] != L'\0'))
	{
		// A font name was provided.
		// First, check if this is a font family name that was privately loaded from file.
		fontFamilyPointer = this->FetchPrivateLoadedFontByName(fontSettings.GetUtf16Name());
		if (!fontFamilyPointer)
		{
			// Next, attempt to load an installed system font by font family name.
			Gdiplus::FontFamily testFontFamily(fontSettings.GetUtf16Name());
			if (testFontFamily.IsAvailable())
			{
				// An installed font family name was found. Clone it and flag it to be deleted later.
				fontFamilyPointer = testFontFamily.Clone();
				wasFontFamilyAllocated = true;
			}
			else
			{
				// Font family name not found. Assume we were given the name to a font file.
				std::wstring fontPath = fEnvironment.GetUtf16PathFor(Rtt::MPlatform::kResourceDir);
				fontPath.append(L"\\");
				fontPath.append(fontSettings.GetUtf16Name());
				fontFamilyPointer = this->LoadFromFile(fontPath.c_str());
				if (!fontFamilyPointer)
				{
					// Failed to load font file. Append a *.ttf extension and try again. (Matches our Android behavior.)
					fontPath.append(L".ttf");
					fontFamilyPointer = this->LoadFromFile(fontPath.c_str());
					if (!fontFamilyPointer)
					{
						// Failed to load font file. Append an *.otf extension and try again. (Matches our Android behavior.)
						fontPath.replace(fontPath.length() - 4, 4, L".otf");
						fontFamilyPointer = this->LoadFromFile(fontPath.c_str());
					}
				}
			}
		}
		wasFontNameLoaded = (fontFamilyPointer != nullptr);
	}

	// If a font family hasn't been loaded up above, then load a default system font.
	if (!fontFamilyPointer)
	{
		auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
		if (deviceSimulatorServicesPointer)
		{
			// We're running in the Corona Simulator.
			// To be backward compatible with older versions of the simulator, attempt to use an Arial font first.
			// Note: In the future, we should load an Android, iOS, or WP8 font instead.
			fontFamilyPointer = new Gdiplus::FontFamily(L"Arial");
			wasFontFamilyAllocated = true;
		}
		else
		{
			// We're running in desktop mode. Use the Window's default system font.
			NONCLIENTMETRICSW nonClientMetrics{};
			nonClientMetrics.cbSize = sizeof(nonClientMetrics);
			auto wasSuccessful = ::SystemParametersInfoW(
					SPI_GETNONCLIENTMETRICS, sizeof(nonClientMetrics), &nonClientMetrics, 0);
			if (wasSuccessful && (nonClientMetrics.lfMessageFont.lfFaceName[0] != L'\0'))
			{
				fontFamilyPointer = new Gdiplus::FontFamily(nonClientMetrics.lfMessageFont.lfFaceName);
				wasFontFamilyAllocated = true;
			}
		}
		if (fontFamilyPointer && (fontFamilyPointer->IsAvailable() == FALSE))
		{
			// We've failed to load a font up above.
			// Delete it so that we can use the final font fallback loader down below.
			delete fontFamilyPointer;
			fontFamilyPointer = nullptr;
			wasFontFamilyAllocated = false;
		}
	}

	// If we still haven't loaded a font family, then just pick one that's guaranteed to exist.
	if (!fontFamilyPointer)
	{
		fontFamilyPointer = Gdiplus::FontFamily::GenericSansSerif();
	}

	// If we've failed to load a font by name up above, then this is likely a developer error.
	// Log a warning so that the developer can correct the mistake.
	// Note: Null/empty font names are valid and always default to a system font.
	if (!wasFontNameLoaded && !Rtt_StringIsEmpty(fontSettings.GetUtf8Name()))
	{
		CORONA_LOG_WARNING("Could not load font %s. Using default font.", fontSettings.GetUtf8Name());
	}
	
	INT fontStyles[] = {
		fontSettings.GetGdiPlusStyle(),
		Gdiplus::FontStyleRegular,
		Gdiplus::FontStyleBold,
		Gdiplus::FontStyleItalic,
		Gdiplus::FontStyleBoldItalic,
		Gdiplus::FontStyleUnderline,
		Gdiplus::FontStyleStrikeout,
		Gdiplus::FontStyleRegular,
		-1 // last element must be -1
	};

	// Sometimes font styles are not available, so iterate to find if any available
	INT *fontStylePtr = fontStyles;
	while (!fontFamilyPointer->IsStyleAvailable(*fontStylePtr) && fontStylePtr[1] != -1) 
	{
		fontStylePtr++;
	}

	// Create the GDI+ font object.
	auto fontPointer = std::make_shared<Gdiplus::Font>(
			fontFamilyPointer, fontSettings.GetPixelSize(), *fontStylePtr, Gdiplus::UnitPixel);
	if (fontPointer->GetLastStatus() != Gdiplus::Ok)
	{
		CORONA_LOG_WARNING("Unknown error (%d) occurred while creating native font %s. Using default font.", fontPointer->GetLastStatus(), fontSettings.GetUtf8Name());
		fontPointer = std::make_shared<Gdiplus::Font>( 
			Gdiplus::FontFamily::GenericSansSerif(), fontSettings.GetPixelSize(), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	}

	// If a GDI+ font family object was allocated, then delete it now.
	if (wasFontFamilyAllocated && fontFamilyPointer)
	{
		delete fontFamilyPointer;
	}

	// Return the font as a shared pointer.
	return fontPointer;
}

#pragma endregion

} }	// namespace Interop::Graphics
