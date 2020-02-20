//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_IPhoneFont.h"

#import <UIKit/UIFont.h>
#import "Rtt_AppleFont.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

IPhoneFont::IPhoneFont( const char *fontName, Rtt_Real size, NSString *directory )
:	Super(),
	fFont( AppleFontHelper::AllocNativeFont(fontName, size, directory) )
{
}

IPhoneFont::IPhoneFont( PlatformFont::SystemFont fontType, Rtt_Real size )
:	Super(),
	fFont( nil )
{
	switch ( fontType )
	{
		case PlatformFont::kSystemFontBold:
			fFont = [UIFont boldSystemFontOfSize:size];
			break;
		default:	// case PlatformFont::kSystemFont:
			fFont = [UIFont systemFontOfSize:size];
			break;		
	}

	[fFont retain];
}

IPhoneFont::IPhoneFont( UIFont *font )
:	Super(),
	fFont( [font retain] )
{
}

IPhoneFont::IPhoneFont( const IPhoneFont &font )
:	Super(),
	fFont( [font.fFont retain] )
{
}

IPhoneFont::~IPhoneFont()
{
	[fFont release];
}

PlatformFont*
IPhoneFont::CloneUsing( Rtt_Allocator *allocator ) const
{
	if ( !allocator )
	{
		return NULL;
	}
	return Rtt_NEW( allocator, IPhoneFont( *this ) );
}

const char*
IPhoneFont::Name() const
{
	return [[fFont fontName] UTF8String];
}

void
IPhoneFont::SetSize( Real newValue )
{
	if ( Size() != newValue )
	{
		UIFont *oldFont = fFont;
		fFont = [[fFont fontWithSize:newValue] retain];
		[oldFont release];
	}
}

Real
IPhoneFont::Size() const
{
	return [fFont pointSize];
}

void*
IPhoneFont::NativeObject() const
{
	return fFont;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

