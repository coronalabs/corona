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
#include "Rtt_WinFont.h"
#include "Core\Rtt_Build.h"
#include "Interop\RuntimeEnvironment.h"


namespace Rtt {

WinFont::WinFont(Interop::RuntimeEnvironment& environment)
:	Super(),
	fEnvironment(environment),
	fName(&environment.GetAllocator()),
	fSize(Rtt_REAL_0),
	fIsBold(false),
	fIsItalic(false)
{
}

WinFont::WinFont(const WinFont &font)
:	Super(),
	fEnvironment(font.fEnvironment),
	fName(font.fName.GetAllocator(), font.fName.GetString()),
	fSize(font.fSize),
	fIsBold(font.fIsBold),
	fIsItalic(font.fIsItalic)
{
}

WinFont::~WinFont()
{
}

void WinFont::SetName(const char* name)
{
	fName.Set(name);
}

const char* WinFont::Name() const
{
	return fName.GetString();
}

void WinFont::SetSize(Real value)
{
	fSize = value;
}

Rtt_Real WinFont::Size() const
{
	return fSize;
}

void WinFont::SetBold(bool value)
{
	fIsBold = value;
}

bool WinFont::IsBold() const
{
	return fIsBold;
}

void WinFont::SetItalic(bool value)
{
	fIsItalic = value;
}

bool WinFont::IsItalic() const
{
	return fIsItalic;
}

void* WinFont::NativeObject() const
{
	return nullptr;
}

}
