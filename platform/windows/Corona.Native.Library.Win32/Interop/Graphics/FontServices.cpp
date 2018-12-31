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

#include "stdafx.h"
#include "FontServices.h"
#include "FontSettings.h"
#include <algorithm>
#include <Shlwapi.h>


namespace Interop { namespace Graphics {

#pragma region Private WeakGdiPlusFontFamilyReference Class
/// <summary>
///  <para>Private/internal class which stores a weak reference to a GDI+ font family.</para>
///  <para>Intended to be passed to used when calling Microsoft's Gdiplus::Font::GetFamily() method.</para>
/// </summary>
/// <remarks>
///  <para>
///  This class works-around a severe Microsoft bug with the Gdiplus::Font::GetFamily() method. That method
///  will cause a crash if you pass it a Gdiplus::FontFamily instance to get a copy of the font family info
///  it uses. The reason it crashes is because the Font object's "nativeFamily" pointer is copied to
///  to the given temporary FontFamily instance, which will wrongly delete that "nativeFamily" pointer
///  that doesn't belong to it. This means that the Font object will be holding on to a wild pointer to
///  its native font family and will crash when it uses it.
///  </para>
///  <para>
///   This class works-around that bug by nulling out the "nativeFamily" pointer member variable before the
///   FontFamliy class' destructor has a chance to delete it. Hence, this class keeps a weak reference to it.
///  </para>
/// </remarks>
class WeakGdiPlusFontFamilyReference : public Gdiplus::FontFamily
{
	public:
		/// <summary>
		///  Safely releases the native GDI+ font family reference before the Gdiplus::FontFamily class'
		///  destructor has a chance to delete it.
		/// </summary>
		virtual ~WeakGdiPlusFontFamilyReference();
};

WeakGdiPlusFontFamilyReference::~WeakGdiPlusFontFamilyReference()
{
	this->nativeFamily = nullptr;
}

#pragma endregion


#pragma region Constructors/Destructors
FontServices::FontServices()
:	fGdiPlusToken(0)
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&fGdiPlusToken, &gdiplusStartupInput, nullptr);
}

FontServices::~FontServices()
{
	// Remove all privately loaded fonts from the old GDI system.
	for (auto&& pair : fFontFilePathMap)
	{
		::RemoveFontResourceExW(pair.first.c_str(), FR_NOT_ENUM, 0);
	}

	// Delete all of the FontFamily objects allocated by this class instance.
	Gdiplus::FontFamily* fontFamilyPointer;
	for (auto&& pair : fFontFamilyNameMap)
	{
		fontFamilyPointer = pair.second;
		if (fontFamilyPointer)
		{
			delete fontFamilyPointer;
		}
	}

	// Shutdown Microsoft GDI+.
	if (fGdiPlusToken)
	{
		Gdiplus::GdiplusShutdown(fGdiPlusToken);
		fGdiPlusToken = 0;
	}
}

#pragma endregion


#pragma region Public Methods
Gdiplus::FontFamily* FontServices::LoadFromFile(const wchar_t* filePath)
{
	// Validate argument.
	if (!filePath || (filePath[0] == L'\0'))
	{
		return nullptr;
	}

	// Make a lower-case copy of the given file path.
	std::wstring lowerCaseFilePath = filePath;
	std::transform(lowerCaseFilePath.begin(), lowerCaseFilePath.end(), lowerCaseFilePath.begin(), ::towlower);

	// Check if the given font file has already been loaded.
	auto iter = fFontFilePathMap.find(lowerCaseFilePath);
	if (iter != fFontFilePathMap.end())
	{
		return (*iter).second;
	}

	// Check if the given file exists.
	if (::PathFileExistsW(filePath) == FALSE)
	{
		return nullptr;
	}

	// Attempt to load the font file.
	Gdiplus::PrivateFontCollection privateFontCollection;
	auto result = privateFontCollection.AddFontFile(filePath);
	if (result != Gdiplus::Ok)
	{
		return nullptr;
	}
	INT loadedFontCount = privateFontCollection.GetFamilyCount();
	if (loadedFontCount <= 0)
	{
		return nullptr;
	}

	// Fetch the loaded font information.
	Gdiplus::FontFamily* loadedFontFamilyPointer = nullptr;
	wchar_t fontFamilyName[LF_FACESIZE];
	INT receivedFontCount = 0;
	auto fontFamilyArray = new Gdiplus::FontFamily[loadedFontCount];
	privateFontCollection.GetFamilies(loadedFontCount, fontFamilyArray, &receivedFontCount);
	for (int index = 0; index < receivedFontCount; index++)
	{
		// Fetch the next loaded font family object.
		Gdiplus::FontFamily& fontFamilyReference = fontFamilyArray[index];
		if (fontFamilyReference.IsAvailable() == FALSE)
		{
			continue;
		}

		// Fetch the font family name.
		fontFamilyName[0] = L'\0';
		result = fontFamilyReference.GetFamilyName(fontFamilyName);
		if ((result != Gdiplus::Ok) || (fontFamilyName[0] == L'\0'))
		{
			continue;
		}

		// Check if the loaded font's family name has already been loaded before.
		// This can happen if we've loaded a copy of a font file that exists in a different directory.
		iter = fFontFamilyNameMap.find(fontFamilyName);
		if (iter != fFontFamilyNameMap.end())
		{
			// Given font file was already loaded. Add its path to the collection for fast lookup next time.
			fFontFilePathMap.insert(StringFontFamilyPair(lowerCaseFilePath, (*iter).second));
			if (!loadedFontFamilyPointer)
			{
				loadedFontFamilyPointer = (*iter).second;
			}
			continue;
		}

		// Clone the loaded font family object and add it to our collection for quick lookup later.
		Gdiplus::FontFamily* clonedFontFamilyPointer = fontFamilyReference.Clone();
		if (!clonedFontFamilyPointer)
		{
			continue;
		}
		fFontFamilyNameMap.insert(StringFontFamilyPair(fontFamilyName, clonedFontFamilyPointer));
		fFontFilePathMap.insert(StringFontFamilyPair(lowerCaseFilePath, clonedFontFamilyPointer));
		if (!loadedFontFamilyPointer)
		{
			loadedFontFamilyPointer = clonedFontFamilyPointer;
		}
	}
	delete[] fontFamilyArray;
	fontFamilyArray = nullptr;

	// Load the font privately into GDI (not GDI+) so that it can by the older GDI features such as native UI.
	if (loadedFontFamilyPointer)
	{
		// First, remove all references to the existing font file, if they exist.
		// This is the only way to replace an existing loaded font with a new one in case it has changed.
		// Note: Adding/removing fonts with the API is reference counted.
		int removeCount = 0;
		for (; ::RemoveFontResourceExW(lowerCaseFilePath.c_str(), FR_NOT_ENUM, 0); removeCount++);

		// Load the font privately into GDI.
		// Note: We must add it as many times we have removed it above, plus one extra add for the caller.
		//       This is because the add/remove functions are reference counted by Windows.
		do
		{
			::AddFontResourceExW(lowerCaseFilePath.c_str(), FR_NOT_ENUM, 0);
			removeCount--;
		} while (removeCount >= 0);
	}

	// Return the loaded font family object. (Will return null if we failed to load the font.)
	return loadedFontFamilyPointer;
}

std::shared_ptr<Gdiplus::Font> FontServices::LoadUsing(const FontSettings& fontSettings)
{
	// Attempt to load the requested font.
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
				fontFamilyPointer = this->LoadFromFile(fontSettings.GetUtf16Name());
			}
		}
	}

	// If a font family hasn't been loaded up above, then load a default system font.
	if (!fontFamilyPointer)
	{
		NONCLIENTMETRICSW nonClientMetrics{};
		nonClientMetrics.cbSize = sizeof(nonClientMetrics);
		auto wasSuccessful = ::SystemParametersInfoW(
				SPI_GETNONCLIENTMETRICS, sizeof(nonClientMetrics), &nonClientMetrics, 0);
		if (wasSuccessful && (nonClientMetrics.lfMessageFont.lfFaceName[0] != L'\0'))
		{
			fontFamilyPointer = new Gdiplus::FontFamily(nonClientMetrics.lfMessageFont.lfFaceName);
			wasFontFamilyAllocated = true;
			if (fontFamilyPointer->IsAvailable() == FALSE)
			{
				delete fontFamilyPointer;
				fontFamilyPointer = nullptr;
				wasFontFamilyAllocated = false;
			}
		}
	}

	// If we still haven't loaded a font family, then just pick one that's guaranteed to exist.
	if (!fontFamilyPointer)
	{
		fontFamilyPointer = Gdiplus::FontFamily::GenericSansSerif();
	}

	// Create the GDI+ font object.
	auto fontPointer = std::make_shared<Gdiplus::Font>(
			fontFamilyPointer, fontSettings.GetPixelSize(), fontSettings.GetGdiPlusStyle(), Gdiplus::UnitPixel);

	// If a GDI+ font family object was allocated, then delete it now.
	if (wasFontFamilyAllocated && fontFamilyPointer)
	{
		delete fontFamilyPointer;
	}

	// Return the font as a shared pointer.
	return fontPointer;
}

Gdiplus::FontFamily* FontServices::FetchPrivateLoadedFontByName(const wchar_t* name) const
{
	if (name && (name[0] != L'\0'))
	{
		auto iter = fFontFamilyNameMap.find(name);
		if (iter != fFontFamilyNameMap.end())
		{
			return (*iter).second;
		}
	}
	return nullptr;
}

std::shared_ptr<Gdiplus::FontFamily> FontServices::GetFamilyFrom(const Gdiplus::Font& font) const
{
	// Fetch a "weak" reference to the given font's FontFamily object.
	// Note: This works-around a severe Microsoft bug with calling Font.GetFamily() with a FontFamily instance.
	//       This will cause a crash because GetFamily() copies the native font family pointer to the given
	//       FontFamily object which will be wrongly deleted by the FontFamily class' destructor, which means
	//       the Font object will have a wild pointer to a deleted native font famliy.
	std::shared_ptr<Gdiplus::FontFamily> fontFamilyPointer;
	if (font.IsAvailable())
	{
		fontFamilyPointer = std::shared_ptr<Gdiplus::FontFamily>(new WeakGdiPlusFontFamilyReference);
		auto result = font.GetFamily(fontFamilyPointer.get());
		if (result != Gdiplus::Ok)
		{
			fontFamilyPointer = nullptr;
		}
	}
	return fontFamilyPointer;
}

#pragma endregion

} }	// namespace Interop::Graphics
