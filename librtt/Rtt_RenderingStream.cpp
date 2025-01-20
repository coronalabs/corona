//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"

#include "Rtt_RenderingStream.h"

#include "Rtt_PlatformSurface.h"

#include "Display/Rtt_BitmapPaint.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

RenderingStream::RenderingStream()
:	fSubmitBounds( NULL ),
	fProperties( 0 ),
	fXAlign( kAlignmentCenter ),
	fYAlign( kAlignmentCenter ),
	fLaunchOrientation( DeviceOrientation::kUpright ),
	fContentOrientation( DeviceOrientation::kUpright ),
	fSurfaceOrientation( DeviceOrientation::kUpright ),
	fScaleMode( Display::kNone ),
	fSx( Rtt_REAL_1 ), // default to identity
	fSy( Rtt_REAL_1 ), // default to identity
	fContentWidth( -1 ), // uninitialized
	fContentHeight( -1 ), // uninitialized
	fXOriginOffset( Rtt_REAL_0 ),
	fYOriginOffset( Rtt_REAL_0 ),
	fScreenContentBounds(),
	fCapabilities( 0 )
{
}

RenderingStream::~RenderingStream()
{
}

void
RenderingStream::Preinitialize( S32 contentW, S32 contentH )
{
	fContentWidth = contentW;
	fContentHeight = contentH;
}

void
RenderingStream::Initialize(
					const PlatformSurface& surface,
					DeviceOrientation::Type launchOrientation )
{
	SetProperty( kInitialized, true );

	fDeviceWidth = surface.DeviceWidth();
	fDeviceHeight = surface.DeviceHeight();

	// If content w,h are not specified in config.lua, then default to dimensions of surface.
	// Device w,h is assumed to be unrotated, i.e. dimensions of device when upright.
	if ( fContentWidth < 0 || fContentHeight < 0 )
	{
		if ( Display::kAdaptive == fScaleMode )
		{
			fContentWidth = surface.AdaptiveWidth();
			fContentHeight = surface.AdaptiveHeight();
		}
		else
		{
			fContentWidth = surface.DeviceWidth();
			fContentHeight = surface.DeviceHeight();
		}
	}

	// If app is landscape, swap fContentWidth and fContentHeight b/c these values
	// are the dimensions of the upright virtual screen that the content was created for. 
	// Also swap fSx, fSy for landscape. The original values are from the physical
	// dimensions, which we assume is portrait.
	if ( DeviceOrientation::IsSideways( launchOrientation ) )
	{
		SwapContentSize();
		SwapContentScale();
		SwapContentAlign();
	}
	
	fLaunchOrientation = launchOrientation;
	fContentOrientation = launchOrientation;
	SetSurfaceOrientation( surface.GetOrientation() );
}

void
RenderingStream::Reinitialize(
					const PlatformSurface& surface,
					DeviceOrientation::Type launchOrientation )
{
	SetProperty( kInitialized, false );

	Rtt_ASSERT( ! IsProperty( kInhibitSwap ) );
	SetProperty( kInhibitSwap, true );
	Initialize( surface, launchOrientation );
	SetProperty( kInhibitSwap, false );
}

void
RenderingStream::PrepareToRender()
{
}

void
RenderingStream::UpdateProjection( bool )
{
}

void
RenderingStream::UpdateViewport( S32 windowWidth, S32 windowHeight )
{
}

S32
RenderingStream::GetRenderedContentWidth() const
{
	Rtt_ASSERT_NOT_REACHED();
	return -1;
}

S32
RenderingStream::GetRenderedContentHeight() const
{
	Rtt_ASSERT_NOT_REACHED();
	return -1;
}

Rtt_Real
RenderingStream::ViewableContentWidth() const
{
	Rtt_ASSERT_NOT_REACHED();
	return Rtt_REAL_0;
}

Rtt_Real
RenderingStream::ViewableContentHeight() const
{
	Rtt_ASSERT_NOT_REACHED();
	return Rtt_REAL_0;
}

Rtt_Real
RenderingStream::ActualContentWidth() const
{
	Rtt_ASSERT_NOT_REACHED();
	return Rtt_REAL_0;
}

Rtt_Real
RenderingStream::ActualContentHeight() const
{
	Rtt_ASSERT_NOT_REACHED();
	return Rtt_REAL_0;
}

void
RenderingStream::Reshape( S32 renderedContentWidth, S32 renderedContentHeight )
{
}

bool
RenderingStream::ShouldSwap( DeviceOrientation::Type srcOrientation, DeviceOrientation::Type dstOrientation )
{
	bool isSrcLandscape = DeviceOrientation::IsSideways( srcOrientation );
	bool isDstLandscape = DeviceOrientation::IsSideways( dstOrientation );

	Rtt_ASSERT( isSrcLandscape || DeviceOrientation::IsUpright( srcOrientation ) );
	Rtt_ASSERT( isDstLandscape || DeviceOrientation::IsUpright( dstOrientation ) );

	// If they are not the same, then swap
	return ( isSrcLandscape != isDstLandscape );
}

void
RenderingStream::SetContentOrientation( DeviceOrientation::Type newOrientation )
{
	Rtt_ASSERT( newOrientation >= DeviceOrientation::kUnknown && newOrientation <= DeviceOrientation::kNumTypes );

	Rtt_ASSERT( IsProperty( kInitialized ) );

	if ( ShouldSwap( GetContentOrientation(), newOrientation ) )
	{
		SwapContentScale();
	}

	fContentOrientation = newOrientation;
}

void
RenderingStream::SetOrientation( DeviceOrientation::Type newOrientation, bool autoRotate )
{
	Rtt_ASSERT_NOT_REACHED();
}

DeviceOrientation::Type
RenderingStream::GetRelativeOrientation() const
{
	return DeviceOrientation::GetRelativeOrientation( GetContentOrientation(), GetSurfaceOrientation() );
}

void
RenderingStream::SetTextureCoordinates( const Vertex2 *coords, S32 numCoords )
{
}

void
RenderingStream::PushTexture( const BitmapPaint *paint )
{
}

const BitmapPaint*
RenderingStream::PopTexture()
{
	return NULL;
}

int
RenderingStream::GetTextureDepth() const
{
	return 0;
}

void
RenderingStream::SetClearColor( const Paint& paint )
{
}

RGBA
RenderingStream::GetClearColor()
{
	RGBA color;
	color.r = 0;
	color.g = 0;
	color.b = 0;
	color.a = 0;
	return color;
}

void
RenderingStream::Clear()
{
}

void
RenderingStream::SetMatrixMode( MatrixMode mode )
{
}

void
RenderingStream::LoadIdentity()
{
}

void
RenderingStream::SetEnabled( CapabilityMask mask, bool value )
{
	const U32 p = fCapabilities;
	fCapabilities = ( value ? p | mask : p & ~mask );
}

void
RenderingStream::SetTextureId( U32 textureId )
{
}

void
RenderingStream::SetTextureParameter( TextureParameter param, TextureValue value )
{
}

void
RenderingStream::SetTextureFunction( TextureFunction func )
{
}

void
RenderingStream::SetColor( const RGBA& color )
{
}

void
RenderingStream::BeginPaint( const Paint* paint )
{
#ifdef OLD_GRAPHICS
	static bool sShouldWarnLimitation = true;

	if ( paint )
	{
		const BitmapPaint *bitmapPaint = (const BitmapPaint*)paint->AsPaint( Paint::kBitmap );
		Rtt_ASSERT( ( bitmapPaint && paint->IsType( Paint::kBitmap ) ) || ! bitmapPaint );
		bool isCurrentPaintTexture = ( NULL != bitmapPaint );

		RenderTypes::BlendType paintBlend = paint->GetBlend();

		bool isPremultiplied = isCurrentPaintTexture && bitmapPaint->GetBitmap().IsPremultiplied();
		RenderTypes::BlendMode mode = RenderTypes::BlendModeForBlendType( paintBlend, isPremultiplied );

		// LIMITATION: with screen blend modes and non-premultiplied alpha
		if ( sShouldWarnLimitation
			 && isCurrentPaintTexture
			 && ( RenderTypes::kScreenNonPremultiplied == mode || RenderTypes::kMultiplyNonPremultiplied == mode ) )
		{
			const PlatformBitmap& bitmap = bitmapPaint->GetBitmap();
			if ( bitmap.HasAlphaChannel() )
			{
				// Only warn once
				sShouldWarnLimitation = false;

				const char *str = RenderTypes::StringForBlendType( paintBlend ); Rtt_ASSERT( str );
				Rtt_LogException( "WARNING: Because the image has an alpha channel but the platform OS could not load it with premultiplied alpha, '%s' blending will not work correctly. (This message will only appear once).\n", ( str ? str : "" ) );
			}
		}

		// Set blend mode *before* calling Paint::Begin()
		SetBlendMode( mode );
	}
#endif
}

void
RenderingStream::EndPaint()
{
}

U8
RenderingStream::GetAlpha() const
{
	return 0xFF;
}

U8
RenderingStream::SetAlpha( U8, bool )
{
	return 0xFF;
}

void
RenderingStream::CaptureFrameBuffer( BufferBitmap&, S32 x, S32 y, S32 w, S32 h )
{
	Rtt_ASSERT_NOT_REACHED();
}

void
RenderingStream::Begin3D( const Xform3D*, const Rect& bounds )
{
	Rtt_ASSERT_NOT_REACHED();
}

void
RenderingStream::End3D( const Xform3D* )
{
	Rtt_ASSERT_NOT_REACHED();
}

/*
void
RenderingStream::SetScale( Real sx, Real sy )
{
	fSx = sx;
	fSy = sy;
}
*/

void
RenderingStream::SwapContentSize()
{
	if ( ! IsProperty( kInhibitSwap ) )
	{
		Swap( fContentWidth, fContentHeight );
	}
}

void
RenderingStream::SwapContentScale()
{
	if ( ! IsProperty( kInhibitSwap ) )
	{
		Swap( fSx, fSy );
	}
}
    
void
RenderingStream::SwapContentAlign()
{
    if ( ! IsProperty( kInhibitSwap ) )
    {
        Swap( fXAlign, fYAlign );
    }
}

void
RenderingStream::SetContentAlignment( Alignment xAlign, Alignment yAlign )
{
	fXAlign = xAlign;
	fYAlign = yAlign;
}

void
RenderingStream::SetScaleMode( Display::ScaleMode mode, Rtt_Real screenWidth, Rtt_Real screenHeight )
{
	Rtt_ASSERT( fContentWidth > 0 && fContentHeight > 0 );

	fScaleMode = mode;

	UpdateContentScale( screenWidth, screenHeight );
}

// Gets the width of the screen.
S32
RenderingStream::ScreenWidth() const
{
	Rtt_Real originOffset = GetXOriginOffset();
	Rtt_Real margins = Rtt_RealMul(Rtt_IntToReal(2), originOffset);
	S32 result = Rtt_RealToInt(Rtt_RealDiv(Rtt_IntToReal(ContentWidth()) + margins, GetSx()) + Rtt_REAL_HALF);

	// TODO: Does this account for Alignment? Let's assert for now:
	Rtt_ASSERT( DeviceWidth() == result );

	return result;
}

// Gets the height of the screen.
S32
RenderingStream::ScreenHeight() const
{
	Rtt_Real originOffset = GetYOriginOffset();
	Rtt_Real margins = Rtt_RealMul(Rtt_IntToReal(2), originOffset);
	S32 result = Rtt_RealToInt(Rtt_RealDiv(Rtt_IntToReal(ContentHeight()) + margins, GetSy()) + Rtt_REAL_HALF);
	// TODO: Does this account for Alignment? Let's assert for now:
	Rtt_ASSERT( DeviceHeight() == result );

	return result;
}

void
RenderingStream::ContentToScreenUnrounded( float& x, float& y ) const
{
	float w = 0;
	float h = 0;
	ContentToScreenUnrounded(x, y, w, h);
}

void
RenderingStream::ContentToScreenUnrounded( float& x, float& y, float& w, float& h ) const
{
	Rtt_Real xScreen = GetSx();
	x = Rtt_RealDiv(GetXOriginOffset() + x, xScreen);
	w = Rtt_RealDiv(w, xScreen);

	Rtt_Real yScreen = GetSy();
	y = Rtt_RealDiv(GetYOriginOffset() + y, yScreen);
	h = Rtt_RealDiv(h, yScreen);
}

// Converts the given content coordinates to pixel coordinates.
void
RenderingStream::ContentToScreen( S32& x, S32& y ) const
{
	S32 w = 0;
	S32 h = 0;
	ContentToScreen(x, y, w, h);
}

/// Converts the given content coordinates to screen coordinates with origin at top left corner of screen.
void
RenderingStream::ContentToScreen( S32& x, S32& y, S32& w, S32& h ) const
{
	Rtt_Real xScreen = GetSx();
	x = Rtt_RealToInt(Rtt_RealDiv(GetXOriginOffset() + Rtt_IntToReal(x), xScreen) + Rtt_REAL_HALF);
	w = Rtt_RealToInt(Rtt_RealDiv(Rtt_IntToReal(w), xScreen) + Rtt_REAL_HALF);

	Rtt_Real yScreen = GetSy();
	y = Rtt_RealToInt(Rtt_RealDiv(GetYOriginOffset() + Rtt_IntToReal(y), yScreen) + Rtt_REAL_HALF);
	h = Rtt_RealToInt(Rtt_RealDiv(Rtt_IntToReal(h), yScreen) + Rtt_REAL_HALF);

}

void
RenderingStream::ContentToScreen( Rtt_Real& x, Rtt_Real& y, Rtt_Real& w, Rtt_Real& h ) const
{
	Rtt_Real xScreen = GetSx();
	x = Rtt_RealToInt(Rtt_RealDiv(GetXOriginOffset() + Rtt_IntToReal(x), xScreen));
	w = Rtt_RealToInt(Rtt_RealDiv(Rtt_RealToFloat(w), xScreen));

	Rtt_Real yScreen = GetSy();
	y = Rtt_RealToInt(Rtt_RealDiv(GetYOriginOffset() + Rtt_IntToReal(y), yScreen));
	h = Rtt_RealToInt(Rtt_RealDiv(Rtt_RealToFloat(h), yScreen));

}

void
RenderingStream::ContentToPixels( S32& x, S32& y ) const
{
	S32 w = 0;
	S32 h = 0;
	ContentToPixels(x, y, w, h);
}

void
RenderingStream::ContentToPixels( S32& x, S32& y, S32& w, S32& h ) const
{
	ContentToScreen(x, y, w, h);
}

void
RenderingStream::UpdateContentScale( Rtt_Real screenWidth, Rtt_Real screenHeight )
{
	Rtt_ASSERT( fContentWidth > 0 && fContentHeight > 0 );

	Display::UpdateContentScale( screenWidth, screenHeight, fContentWidth, fContentHeight, GetScaleMode(), fSx, fSy );
}

void
RenderingStream::WillSubmitArray( const Quad& submitBounds )
{
	Rtt_ASSERT( ! fSubmitBounds );
	fSubmitBounds = & submitBounds;
}

void
RenderingStream::DidSubmitArray()
{
	Rtt_ASSERT( fSubmitBounds );
	fSubmitBounds = NULL;
}

void
RenderingStream::SetProperty( U32 mask, bool value )
{
	const U8 p = fProperties;
	fProperties = ( value ? p | mask : p & ~mask );
}
/*
const Quad*
RenderingStream::SetCurrentMask( const Quad* newMask )
{
	const Quad* result = fCurrentMask;
	fCurrentMask = newMask;
	return result;
}
*/

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

