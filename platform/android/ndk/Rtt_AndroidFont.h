//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidFont_H__
#define _Rtt_AndroidFont_H__

#include "Rtt_PlatformFont.h"
#include "Core/Rtt_String.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AndroidFont : public PlatformFont
{
	public:
		typedef AndroidFont Self;
		typedef PlatformFont Super;

	public:
		AndroidFont( Rtt_Allocator &allocator, const char *fontName, Rtt_Real size, bool isBold );
		AndroidFont( Rtt_Allocator &allocator, PlatformFont::SystemFont fontType, Rtt_Real size );
		AndroidFont( const AndroidFont &font );
		virtual ~AndroidFont();

	public:
		// PlaformFont
		virtual PlatformFont* CloneUsing( Rtt_Allocator *allocator ) const;
		virtual const char * Name() const;
		virtual void SetSize( Real newValue );
		virtual Rtt_Real Size() const;
		virtual void* NativeObject() const;
		bool IsBold() const;

	private:
		String fFontName;
		Rtt_Real fFontSize;
		bool fIsBold;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidFont_H__
