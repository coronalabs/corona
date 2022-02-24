//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_LinuxFont.h"
#include <stdio.h>

namespace Rtt
{
	LinuxFont::LinuxFont(Rtt_Allocator &allocator, const char *fontName, Rtt_Real size, bool isBold)
		:	Super(),
		  fName(&allocator, fontName ? fontName : ""),
		  fSize(size),
		  fIsBold(isBold)
	{
	}

	LinuxFont::LinuxFont(Rtt_Allocator &allocator, PlatformFont::SystemFont fontType, Rtt_Real size)
		:	Super(),
		  fName(&allocator, ""),
		  fSize(size),
		  fIsBold(false)
	{
	}

	LinuxFont::LinuxFont(const LinuxFont &rhs)
		:	Super(),
		  fName(rhs.fName.GetAllocator(), rhs.fName.GetString()),
		  fSize(rhs.fSize),
		  fIsBold(rhs.fIsBold)
	{
	}

	LinuxFont::~LinuxFont()
	{
	}

	PlatformFont *LinuxFont::CloneUsing(Rtt_Allocator *allocator) const
	{
		if (!allocator)
		{
			return NULL;
		}

		return Rtt_NEW(allocator, LinuxFont(*this));
	}

	const char *LinuxFont::Name() const
	{
		return fName.GetString();
	}

	void LinuxFont::SetSize(Rtt_Real newValue)
	{
		fSize = newValue;
	}

	Rtt_Real LinuxFont::Size() const
	{
		return fSize;
	}

	void *LinuxFont::NativeObject() const
	{
		// todo
		return NULL;
	}

	bool LinuxFont::IsBold() const
	{
		return fIsBold;
	}
}; // namespace Rtt
