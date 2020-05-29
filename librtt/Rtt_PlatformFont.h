//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformFont_H__
#define _Rtt_PlatformFont_H__

#include "Core/Rtt_Allocator.h"
#include "Core/Rtt_Real.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class PlatformFont
{
	public:
		// Generic platform-neutral fonts
		typedef enum _SystemFont
		{
			kSystemFont = 0,
			kSystemFontBold,

			kNumSystemFonts
		}
		SystemFont;

	public:
		PlatformFont();
		virtual ~PlatformFont();

	public:
		virtual PlatformFont* CloneUsing( Rtt_Allocator *allocator ) const = 0;
		virtual const char* Name() const = 0;
		virtual void SetSize( Real newValue ) = 0;
		virtual Real Size() const = 0;
		virtual void* NativeObject() const = 0;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformFont_H__
