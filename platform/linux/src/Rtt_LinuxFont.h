//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core/Rtt_String.h"
#include "Rtt_PlatformFont.h"

namespace Rtt
{
	class LinuxFont : public PlatformFont
	{
	public:
		typedef PlatformFont Super;

	public:
		LinuxFont(Rtt_Allocator &allocator, const char *fontName, Rtt_Real size, bool isBold);
		LinuxFont(Rtt_Allocator &allocator, PlatformFont::SystemFont fontType, Rtt_Real size);
		LinuxFont(const LinuxFont &font);
		virtual ~LinuxFont();
		virtual PlatformFont *CloneUsing(Rtt_Allocator *allocator) const;
		virtual const char *Name() const;
		virtual void SetSize(Rtt_Real newValue);
		virtual Rtt_Real Size() const;
		// This allocates a TTF_Font object
		virtual void *NativeObject() const;
		bool IsBold() const;

	private:
		String fName;
		Rtt_Real fSize;
		bool fIsBold;
	};
};
