//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
BitmapMask::Create( Runtime& runtime, const FilePath& maskData, bool onlyForHitTests )
{
	BitmapPaint *paint = BitmapPaint::NewBitmap( runtime, maskData, PlatformBitmap::kIsBitsFullResolution, true, onlyForHitTests );
	BitmapMask *result = NULL;

	if ( Rtt_VERIFY( paint || onlyForHitTests ) )
	{
		result = Rtt_NEW( runtime.GetAllocator(), BitmapMask( paint, onlyForHitTests ) );
	}

	return result;
}

BitmapMask::BitmapMask( BitmapPaint *paint, bool onlyForHitTests, bool isTemporary )
:	fPaint( paint ),
	fTransform(),
	fContentWidth( Rtt_REAL_NEG_1 ),
	fContentHeight( Rtt_REAL_NEG_1 ),
	fOnlyForHitTests( onlyForHitTests ),
	fIsTemporary( isTemporary )
{
	Rtt_ASSERT( paint || onlyForHitTests );
}

BitmapMask::BitmapMask( BitmapPaint *paint, Real contentW, Real contentH )
:	fPaint( paint ),
	fTransform(),
	fContentWidth( contentW > Rtt_REAL_0 ? contentW : Rtt_REAL_NEG_1 ),
	fContentHeight( contentH > Rtt_REAL_0 ? contentH : Rtt_REAL_NEG_1 ),
	fOnlyForHitTests( false ),
	fIsTemporary( false )
{
}

BitmapMask::~BitmapMask()
{
	if ( !fIsTemporary )
	{
		Rtt_DELETE( fPaint );
	}
}

const char BitmapMask::kHitTestOnlyTable[] = "hitTestOnlyTableKey";

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

