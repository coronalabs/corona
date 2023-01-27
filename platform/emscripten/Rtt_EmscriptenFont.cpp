//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "Core/Rtt_Build.h"
#include "Rtt_EmscriptenFont.h"

namespace Rtt
{

EmscriptenFont::EmscriptenFont(Rtt_Allocator &allocator, const char *fontName, Rtt_Real size, bool isBold)
:	Super(),
	fName( & allocator, fontName ? fontName : "" ),
	fSize( size ),
	fIsBold( isBold )
{
}

EmscriptenFont::EmscriptenFont(Rtt_Allocator &allocator, PlatformFont::SystemFont fontType, Rtt_Real size)
:	Super(),
	fName( & allocator, "" ),
	fSize( size ),
	fIsBold( false )
{
}

EmscriptenFont::EmscriptenFont(const EmscriptenFont &rhs)
:	Super(),
	fName(rhs.fName.GetAllocator(), rhs.fName.GetString()),
	fSize(rhs.fSize),
	fIsBold(rhs.fIsBold)
{
}

EmscriptenFont::~EmscriptenFont()
{
}

PlatformFont* EmscriptenFont::CloneUsing(Rtt_Allocator *allocator) const
{
	if (!allocator)
	{
		return NULL;
	}
	return Rtt_NEW(allocator, EmscriptenFont(*this));
}

const char* EmscriptenFont::Name() const
{
	return fName.GetString();
}

void EmscriptenFont::SetSize(Rtt_Real newValue)
{
	fSize = newValue;
}

Rtt_Real EmscriptenFont::Size() const
{
	return fSize;
}

void* EmscriptenFont::NativeObject() const
{
	// todo
	return NULL;
}

bool EmscriptenFont::IsBold() const
{
	return fIsBold;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
