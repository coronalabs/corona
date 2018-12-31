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
