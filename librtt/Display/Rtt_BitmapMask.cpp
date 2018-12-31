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

#include "Display/Rtt_BitmapMask.h"
#include "Display/Rtt_BitmapPaint.h"
#include "Renderer/Rtt_Texture.h"
#include "Rtt_Lua.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

BitmapMask*
BitmapMask::Create( Runtime& runtime, const FilePath& maskData )
{
	BitmapPaint *paint = BitmapPaint::NewBitmap( runtime, maskData, PlatformBitmap::kIsBitsFullResolution, true );
	BitmapMask *result = NULL;

	if ( Rtt_VERIFY( paint ) )
	{
		result = Rtt_NEW( runtime.GetAllocator(), BitmapMask( paint ) );
	}

	return result;
}

BitmapMask::BitmapMask( BitmapPaint *paint )
:	fPaint( paint ),
	fTransform(),
	fContentWidth( Rtt_REAL_NEG_1 ),
	fContentHeight( Rtt_REAL_NEG_1 )
{
	Rtt_ASSERT( paint );
}

BitmapMask::BitmapMask( BitmapPaint *paint, Real contentW, Real contentH )
:	fPaint( paint ),
	fTransform(),
	fContentWidth( contentW > Rtt_REAL_0 ? contentW : Rtt_REAL_NEG_1 ),
	fContentHeight( contentH > Rtt_REAL_0 ? contentH : Rtt_REAL_NEG_1 )
{
}

BitmapMask::~BitmapMask()
{
	Rtt_DELETE( fPaint );
}

void
BitmapMask::GetSelfBounds( Rect& rect ) const
{
	Real w, h;
	GetSelfBounds( w, h );

	Real halfW = Rtt_RealDiv2( w );
	Real halfH = Rtt_RealDiv2( h );
	rect.Initialize( halfW, halfH );

	// Mask's self bounds are relative to center of display object
	fTransform.GetMatrix( NULL ).Apply( rect );
}

void
BitmapMask::GetSelfBounds( Real& outW, Real& outH ) const
{
	Real w = fContentWidth;
	Real h = fContentHeight;

	if ( !( w > Rtt_REAL_0 && h > Rtt_REAL_0 ) )
	{
		Texture *texture = fPaint->GetTexture();
		w = texture->GetWidth();
		h = texture->GetHeight();
	}

	outW = w;
	outH = h;
}

void
BitmapMask::SetSelfBounds( Real contentW, Real contentH )
{
	if ( ! ( contentW < Rtt_REAL_0 ) ) // (width >= 0)
	{
		fContentWidth = contentW;
	}

	if ( ! ( contentH <  Rtt_REAL_0 ) ) // (height >= 0)
	{
		fContentHeight = contentH;
	}
}

bool
BitmapMask::HitTest( Rtt_Allocator *allocator, int i, int j ) const
{
	PlatformBitmap *bitmap = fPaint->GetBitmap();

	bool result = ( bitmap ? bitmap->HitTest( allocator, i, j ) : false );

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

