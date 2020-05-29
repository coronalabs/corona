//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////



#include "Core/Rtt_Build.h"
#include "Rtt_AndroidFont.h"

namespace Rtt {

AndroidFont::AndroidFont( Rtt_Allocator & allocator, const char *fontName, Rtt_Real size, bool isBold )
:	Super(), fFontName( & allocator )
{
	fFontName.Set( fontName );
	fFontSize = size;
	fIsBold = isBold;
}

AndroidFont::AndroidFont( Rtt_Allocator & allocator, PlatformFont::SystemFont fontType, Rtt_Real size )
:	Super(), fFontName( & allocator ), fFontSize( size ), fIsBold( false )
{
	switch ( fontType )
	{
		case PlatformFont::kSystemFontBold:
			fIsBold = true;
			break;
		default:	// case PlatformFont::kSystemFont:
			break;		
	}
}

AndroidFont::AndroidFont( const AndroidFont &font )
:	Super(),
	fFontName(font.fFontName.GetAllocator(), font.fFontName.GetString()),
	fFontSize(font.fFontSize),
	fIsBold(font.fIsBold)
{
}

AndroidFont::~AndroidFont()
{
}

PlatformFont*
AndroidFont::CloneUsing( Rtt_Allocator *allocator ) const
{
	if ( !allocator )
	{
		return NULL;
	}
	return Rtt_NEW( allocator, AndroidFont( *this ) );
}

const char * 
AndroidFont::Name() const
{
	return fFontName.GetString();
}

void 
AndroidFont::SetSize( Real newValue )
{
	fFontSize = newValue;
}

Rtt_Real 
AndroidFont::Size() const
{
	return fFontSize;
}

void * 
AndroidFont::NativeObject() const
{
	// TODO
	return NULL;
}

bool
AndroidFont::IsBold() const
{
	return fIsBold;
}

}
