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


#include "AndroidFontSettings.h"


// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new font settings object.
AndroidFontSettings::AndroidFontSettings(Rtt_Allocator *allocatorPointer)
:	fName(allocatorPointer),
	fSize(18.0f),
	fIsBold(false)
{
}

/// Destroys this font settings object.
AndroidFontSettings::~AndroidFontSettings()
{
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Gets the font family name.
/// @return Returns the font family name to be used.
///         <br>
///         Returns null or empty string if using a system default font.
const char* AndroidFontSettings::GetName() const
{
	return fName.GetString();
}

/// Sets the font family name to be used.
/// @param name The font family name.
///             <br>
///             Set to null or empty string to use the system's default font.
void AndroidFontSettings::SetName(const char *name)
{
	fName.Set(name);
}

/// Gets the font size in pixels to be used.
/// @return Returns the font size in pixels.
float AndroidFontSettings::GetSize() const
{
	return fSize;
}

/// Sets the font size to be used.
/// @param size The font size in pixels. Set to zero to use the system's default font size.
void AndroidFontSettings::SetSize(float size)
{
	fSize = size;
}

/// Determines if the font should be bold or not.
/// @return Returns true for a bold font. Returns false if not.
bool AndroidFontSettings::IsBold() const
{
	return fIsBold;
}

/// Sets whether or not the font should be bold.
/// @param value Set true to use a bold font. Set false to not use bold.
void AndroidFontSettings::SetIsBold(bool value)
{
	fIsBold = value;
}
