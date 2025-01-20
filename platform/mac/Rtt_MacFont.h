//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacFont_H__
#define _Rtt_MacFont_H__

#include "Rtt_PlatformFont.h"

// Since we enumerate all the fonts in the project's directory path we need a limit
// in case they put a main.lua in their home directory
#define Rtt_MAX_FONTS_IN_APPLICATION 100

// ----------------------------------------------------------------------------

@class NSFont;
@class NSString;

namespace Rtt
{


// ----------------------------------------------------------------------------

class MacFont : public PlatformFont
{
	public:
		typedef MacFont Self;
		typedef PlatformFont Super;

	public:
		MacFont( const char *fontName, Rtt_Real size, NSString* directory );
		MacFont( PlatformFont::SystemFont fontType, Rtt_Real size );
		MacFont( NSFont *font );
		MacFont( const MacFont &font );
		virtual ~MacFont();

	public:
		// PlaformFont
		virtual PlatformFont* CloneUsing( Rtt_Allocator *allocator ) const;
		virtual const char* Name() const;
		virtual void SetSize( Real newValue );
		virtual Rtt_Real Size() const;
		virtual void* NativeObject() const;

		static void ActivateFontsForApplication(NSString* appDirStr, bool activate = true);

	private:
		NSFont *fFont;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MacFont_H__
