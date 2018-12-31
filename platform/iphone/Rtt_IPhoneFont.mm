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

