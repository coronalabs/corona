//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_GradientPaint.h"

#include "Display/Rtt_BufferBitmap.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_GradientPaintAdapter.h"
#include "Display/Rtt_TextureFactory.h"
#include "Display/Rtt_TextureResource.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

GradientPaint::Direction
GradientPaint::StringToDirection( const char *str )
{
	Direction result = kDefaultDirection;

	if ( str )
	{
		if ( 0 == strcmp( str, "up" ) )
		{
			result = kUpDirection;
		}
		else if ( 0 == strcmp( str, "left" ) )
		{
			result = kLeftDirection;
		}
		else if ( 0 == strcmp( str, "right" ) )
		{
			result = kRightDirection;
		}
	}

	return result;
}

// ----------------------------------------------------------------------------

namespace { // anonymous
	enum {
		kBufferWidth = 1,
		kBufferHeight = 32,
		kEnd = 0,
		kStart = kBufferHeight - 1
	};
}

// Create an 1x2 bitmap
static BufferBitmap *
NewBufferBitmap(
	Rtt_Allocator *pAllocator,
	Color start,
	Color end,
	GradientPaint::Direction direction )
{
	const PlatformBitmap::Format kFormat = PlatformBitmap::kRGBA;
	PlatformBitmap::Orientation orientation = PlatformBitmap::kDown;

	switch ( direction )
	{
		case GradientPaint::kUpDirection:
			orientation = PlatformBitmap::kUp;
			break;
		case GradientPaint::kRightDirection:
			orientation = PlatformBitmap::kRight;
			break;
		case GradientPaint::kLeftDirection:
			orientation = PlatformBitmap::kLeft;
			break;
		default:
			break;
	}

	BufferBitmap *result =
		Rtt_NEW( pAllocator, BufferBitmap( pAllocator, kBufferWidth, kBufferHeight, kFormat, orientation ) );
	result->SetProperty( PlatformBitmap::kIsPremultiplied, true );

	Color *pixels = (Color *)result->WriteAccess();

	// Premultiply alpha
	ColorUnion color0; color0.pixel = start;
	float a0 = color0.rgba.a;
	float alpha0 = a0 / 255.f;
	float r0 = color0.rgba.r * alpha0;
	float g0 = color0.rgba.g * alpha0;
	float b0 = color0.rgba.b * alpha0;

	ColorUnion color1; color1.pixel = end;
	float a1 = color1.rgba.a;
	float alpha1 = a1 / 255.f;
	float r1 = color1.rgba.r * alpha1;
	float g1 = color1.rgba.g * alpha1;
	float b1 = color1.rgba.b * alpha1;

	// Assign boundary pixels
	color0.rgba.r = r0;
	color0.rgba.g = g0;
	color0.rgba.b = b0;
	color0.rgba.a = a0;
	pixels[kStart] = color0.pixel;

	color1.rgba.r = r1;
	color1.rgba.g = g1;
	color1.rgba.b = b1;
	color1.rgba.a = a1;
	pixels[kEnd] = color1.pixel;

	// Rtt_TRACE( ( "(r,g,b,a) = (%g, %g, %g, %g)\n", r1, g1, b1, a1 ) );

	// Interpolate in between
	for ( int i = 1, iMax = (int)kBufferHeight - 1; i < iMax; i++ )
	{
		float x0 = ((float)i) / iMax;
		float x1 = 1.f - x0;
		float r = r0*x0 + r1*x1;
		float g = g0*x0 + g1*x1;
		float b = b0*x0 + b1*x1;
		float a = a0*x0 + a1*x1;

		ColorUnion c;
		c.rgba.r = r;
		c.rgba.g = g;
		c.rgba.b = b;
		c.rgba.a = a;
		pixels[i] = c.pixel;
		// Rtt_TRACE( ( "[%d] (r,g,b,a) = (%g, %g, %g, %g)\n", i, r, g, b, a ) );
	}

	// Rtt_TRACE( ( "(r,g,b,a) = (%g, %g, %g, %g)\n", r0, g0, b0, a0 ) );

	const Real kScale = ((float)(kBufferHeight - 1)) / kBufferHeight; // 0.5;
	result->SetNormalizationScaleY( kScale );

	return result;
}

// ----------------------------------------------------------------------------

GradientPaint *
GradientPaint::New( TextureFactory& factory, Color start, Color end, Direction direction, Rtt_Real angle )
{
	Rtt_Allocator *allocator = factory.GetDisplay().GetAllocator();
	BufferBitmap *bitmap = NewBufferBitmap( allocator, start, end, direction );

	SharedPtr< TextureResource > resource = factory.FindOrCreate( bitmap, true );
	Rtt_ASSERT( resource.NotNull() );

	GradientPaint *result = Rtt_NEW( allocator, GradientPaint( resource, angle ) );

	return result;
}

// ----------------------------------------------------------------------------

GradientPaint::GradientPaint( const SharedPtr< TextureResource >& resource, Rtt_Real angle )
:	Super( resource )
{
	if(!Rtt_RealIsZero(angle))
	{
		Transform & t = GetTransform();
		t.SetProperty(kRotation, angle);
	}
	Initialize( kGradient );
	
	Invalidate( Paint::kTextureTransformFlag );
}

const Paint*
GradientPaint::AsPaint( Super::Type type ) const
{
	return ( kGradient == type || kBitmap == type ? this : NULL );
}

const MLuaUserdataAdapter&
GradientPaint::GetAdapter() const
{
	return GradientPaintAdapter::Constant();
}

Color
GradientPaint::GetStart() const
{
	BufferBitmap *bufferBitmap = static_cast< BufferBitmap * >( GetBitmap() );

	const Color *pixels = (const Color *)bufferBitmap->ReadAccess();

	return pixels[kStart];
}

Color
GradientPaint::GetEnd() const
{
	BufferBitmap *bufferBitmap = static_cast< BufferBitmap * >( GetBitmap() );

	const Color *pixels = (const Color *)bufferBitmap->ReadAccess();

	return pixels[kEnd];
}

void
GradientPaint::SetStart( Color color )
{
	BufferBitmap *bufferBitmap = static_cast< BufferBitmap * >( GetBitmap() );

	Color *pixels = (Color *)bufferBitmap->WriteAccess();

	pixels[kStart] = color;

	GetTexture()->Invalidate(); // Force Renderer to update GPU texture

	GetObserver()->InvalidateDisplay(); // Force reblit
}

void
GradientPaint::SetEnd( Color color )
{
	BufferBitmap *bufferBitmap = static_cast< BufferBitmap * >( GetBitmap() );

	Color *pixels = (Color *)bufferBitmap->WriteAccess();

	pixels[kEnd] = color;

	GetTexture()->Invalidate(); // Force Renderer to update GPU texture

	GetObserver()->InvalidateDisplay(); // Force reblit
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

