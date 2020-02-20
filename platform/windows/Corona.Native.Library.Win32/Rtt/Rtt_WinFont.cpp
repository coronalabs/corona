//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
