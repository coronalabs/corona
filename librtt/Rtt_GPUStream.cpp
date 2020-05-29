//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_GPUStream.h"

#include "Display/Rtt_PlatformBitmap.h"
#include "Display/Rtt_BufferBitmap.h"
#include "Display/Rtt_Paint.h"
//#include "Rtt_GPU.h"
#include "Rtt_FillTesselatorStream.h"
#include "Rtt_PlatformSurface.h"
//#include "Rtt_VertexArray.h"
#include "Display/Rtt_VertexCache.h"
#include "Renderer/Rtt_RenderTypes.h"

#if defined( Rtt_WIN_DESKTOP_ENV ) && !defined( Rtt_POWERVR_ENV )
	#if defined(Rtt_EMSCRIPTEN_ENV)
		#include <GL/glew.h>
	#elif defined(Rtt_LINUX_ENV)
		#ifdef _WIN32
			#include <GL/glew.h>
		#else
			#include <GL/gl.h>
			#include <GL/glext.h>
		#endif
	#else
	#include "glext.h"
	static PFNGLACTIVETEXTUREPROC glActiveTexture;
	static PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture;
	#endif
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#ifdef Rtt_DEBUG

	static bool gShowWireframes = false;

	void
	GPUStream::ToggleWireframe()
	{
		gShowWireframes = ! gShowWireframes;
	}

	#define Rtt_PAINT_TRIANGLES( expr ) (!gShowWireframes && (expr))
#else
	#define Rtt_PAINT_TRIANGLES( expr ) (expr)
#endif

/*
GLenum
GPUStream::GetInternalPixelFormat( PlatformBitmap::Format format )
{
	#ifdef Rtt_OPENGLES
		switch ( format )
		{
			case PlatformBitmap::kRGBA:
				return GL_RGBA;
			case PlatformBitmap::kRGB:
				return GL_RGB;
			case PlatformBitmap::kMask:
				return GL_ALPHA;
			default:
				Rtt_ASSERT_NOT_REACHED();
				return GL_ALPHA;
		}
	#else
		switch ( format )
		{
			case PlatformBitmap::kRGBA:
				return GL_RGBA8;
			case PlatformBitmap::kRGB:
				return GL_RGB8;
			case PlatformBitmap::kMask:
				return GL_ALPHA;
			default:
				Rtt_ASSERT_NOT_REACHED();
				return GL_ALPHA;
		}
	#endif
}

#if !defined( Rtt_OPENGLES )
GLenum
GPUStream::GetPixelFormat( PlatformBitmap::Format format )
{
	switch ( format )
	{
		case PlatformBitmap::kRGBA:
			return GL_BGRA;
		case PlatformBitmap::kRGB:
			return GL_BGR;
		case PlatformBitmap::kMask:
			return GL_ALPHA;
		default:
			Rtt_ASSERT_NOT_REACHED();
			return GL_ALPHA;
	}
}
#endif // !defined( Rtt_OPENGLES )
*/
// ----------------------------------------------------------------------------

#if 0
static GLenum
GLModeForMode( RenderTypes::Mode mode )
{
	GLenum result = GL_TRIANGLE_STRIP;
	switch ( mode )
	{
		case RenderTypes::kFan:
			result = GL_TRIANGLE_FAN;
			break;
		case RenderTypes::kStrip:
			result = GL_TRIANGLE_STRIP;
			break;
		case RenderTypes::kTriangle:
			result = GL_TRIANGLES;
			break;
		default:
			Rtt_ASSERT_NOT_IMPLEMENTED();
			break;
	}

	return result;
}
#endif
// ----------------------------------------------------------------------------

int
GPUStream::GetMaxTextureUnits()
{
	GLint maxTextureUnits = 2;
#if defined( Rtt_ANDROID_ENV )
	// For some reason GL_MAX_TEXTURE_IMAGE_UNITS is not defined on Android...
	glGetIntegerv( GL_MAX_TEXTURE_UNITS, & maxTextureUnits );
	Rtt_ASSERT( maxTextureUnits > 1 ); // OpenGL-ES 1.x mandates at least 2
#elif !defined( Rtt_WIN_PHONE_ENV ) && !defined( Rtt_EMSCRIPTEN_ENV )
	glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, & maxTextureUnits );
	Rtt_ASSERT( maxTextureUnits > 1 ); // OpenGL-ES 1.x mandates at least 2
#endif
	return maxTextureUnits;
}

#ifdef Rtt_REAL_FIXED
	static const GLenum kDataType = GL_FIXED;
#else
	static const GLenum kDataType = GL_FLOAT;
#endif

#ifdef Rtt_DEBUG
	static int sTextureStackSize = -1;
#endif

GLenum
GPUStream::GetDataType()
{
	return kDataType;
}

GPUStream::GPUStream( Rtt_Allocator* pAllocator )
:	Super(),
	fCurrentPaint( NULL ),
	fTextureCoords( NULL ),
	fNumTextureCoords( 0 ),
	fRotation( 0 ),
	fIsTexture( false ),
	fAlpha( 0xFF ),
	fAutoRotate( 0 ),
	fTextureStackSize( 0 ),
	fTextureStackDepth( 0 ),
	fTextureStackNumActiveFrames( 0 ),
	fWindowWidth( 0 ),
	fWindowHeight( 0 ),
	fRenderedContentWidth( 0 ),
	fRenderedContentHeight( 0 ),
	fOrientedContentWidth( 0 ),
	fOrientedContentHeight( 0 ),
	fXVertexOffset( Rtt_REAL_0 ),
	fYVertexOffset( Rtt_REAL_0 ),
	fTextureFunction( kTextureFunctionUnknown ),
	fClearR( 0.f ),
	fClearG( 0.f ),
	fClearB( 0.f ),
	fClearA( 1.f ),
	fAllocator( pAllocator )
{
	memset( & fColor, 0xFF, sizeof( fColor ) );

#if defined( Rtt_WIN_DESKTOP_ENV ) && !defined( Rtt_POWERVR_ENV )
	if  ( glActiveTexture == NULL ) {
		glActiveTexture = (PFNGLACTIVETEXTUREPROC) wglGetProcAddress("glActiveTexture");
		glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC) wglGetProcAddress("glClientActiveTexture");
	}
#endif

	// Init mask pointer values to NULL
	memset( fTextureStack, 0, sizeof( fTextureStack[0] ) * kMaxTextureStackDepth );

#ifdef OLD_GRAPHICS
	// FIXME: GPUStream's constructor assumes a valid OpenGL context is set. This may not be a good assumption.
	GLint maxTextureUnits;
	glGetIntegerv( GL_MAX_TEXTURE_UNITS, & maxTextureUnits );
	Rtt_ASSERT( maxTextureUnits > 1 ); // OpenGL-ES 1.x mandates at least 2
	// Texture depth cannot be larger than:
	// (1) size of TextureStackFrame array
	// (2) number of total texture units available
	fTextureStackSize = (U8)Min( maxTextureUnits, (GLint)kMaxTextureStackDepth );
#endif

#ifdef Rtt_DEBUG
	if ( sTextureStackSize < 0 ) { sTextureStackSize = fTextureStackSize; }
#endif
}

GPUStream::~GPUStream()
{
}

void
GPUStream::Initialize(
				const PlatformSurface& surface,
				DeviceOrientation::Type contentOrientation )
{
	if ( Rtt_VERIFY( ! IsProperty( kInitialized ) ) )
	{
		Super::Initialize( surface, contentOrientation );

		fWindowWidth = surface.Width();
		fWindowHeight = surface.Height();

		// Must start with physical device width and height, not window w/h which
		// could have been zoomed in/out, when calculating the rendered content w/h.
		S32 w = surface.DeviceWidth();
		S32 h = surface.DeviceHeight();

		// Super::Initialize() already swapped GetSx/Sy() based on contentOrientation
		// so we need to swap w,h before applying the scale.
		// if ( contentOrientation != DeviceOrientation::kUpright )

		/// Only swap when the content and surface orientations differ
		//DeviceOrientation::Type surfaceOrientation = surface.GetOrientation();
		//S32 angle = DeviceOrientation::CalculateRotation( contentOrientation, surfaceOrientation );
		
		//#ifdef RTT_SURFACE_ROTATION
		//if ( 90 == angle || -90 == angle )
		{
			// Swap w,h for landscape. The original values are from the physical
			// dimensions, which we assume is portrait.
			if ( DeviceOrientation::IsSideways( contentOrientation ) )
			{
				Swap( w, h );
			}
		}
		//#endif

		Real sx = GetSx();
		if ( ! Rtt_RealIsOne( sx ) )
		{
			w = Rtt_RealToInt( Rtt_RealMul( sx, Rtt_IntToReal( w ) ) );
		}

		Real sy = GetSy();
		if ( ! Rtt_RealIsOne( sy ) )
		{
			h = Rtt_RealToInt( Rtt_RealMul( sy, Rtt_IntToReal( h ) ) );
		}

		fRenderedContentWidth = w;
		fRenderedContentHeight = h;
		fOrientedContentWidth = w;
		fOrientedContentHeight = h;

		// Initialize offsets. We need this init'd before PrepareToRender() calls Reshape()
		// so we can properly offset the status bar in the Corona simulator
		UpdateOffsets( w, h );
	}
}

void
GPUStream::PrepareToRender()
{
	if ( Rtt_VERIFY( IsProperty( kInitialized ) ) )
	{
		Reshape( fRenderedContentWidth, fRenderedContentHeight );

#if 0
		glClearColor( fClearR, fClearG, fClearB, fClearA );

		// Default to smooth so that per-vertex colors are interpolated
		glShadeModel( GL_SMOOTH );

		// Don't need to cull
		glDisable( GL_CULL_FACE );

		// Enable vertex arrays so we can submit fans/strips efficiently
		glEnableClientState( GL_VERTEX_ARRAY );

		const GLfloat kInv255 = 1.f / 255.0f;
		glColor4f( kInv255*fColor.r, kInv255*fColor.g, kInv255*fColor.b, kInv255*fColor.a );

		glDisableClientState( GL_COLOR_ARRAY );

		glEnable( GL_BLEND );
		SetBlendMode( RenderTypes::kNormalNonPremultiplied ); // glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
#endif
	}
}

void
GPUStream::UpdateProjection( bool useOriginalOrientation )
{
#if 0

#if defined( Rtt_DEBUG ) && !defined( Rtt_ANDROID_ENV )
	{
		GLint matrixMode;
		glGetIntegerv( GL_MATRIX_MODE, & matrixMode );
		Rtt_ASSERT( GL_MODELVIEW == matrixMode );
	}
#endif

	S32 width = useOriginalOrientation ? fRenderedContentWidth : fOrientedContentWidth;
	S32 height = useOriginalOrientation ? fRenderedContentHeight : fOrientedContentHeight;
	UpdateProjection( width, height );

	// Restore matrix mode
	glMatrixMode( GL_MODELVIEW );
#endif
}

void
GPUStream::UpdateProjection( S32 renderedContentWidth, S32 renderedContentHeight )
{
#if 0
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	// When the surface and content orientations differ, it is necessary to rotate
	// the projection so that the contents of the frustum (the content) are projected
	// onto the window (surface) correctly.
	// 
	// On the iPhone, the surface orientation is always upright (relative to the device)
	// so to support auto-rotation, we must rotate the frustum so the content appears 
	// upright (relative to the viewer). Hence, we call GPUStream::SetContentOrientation()
	// which sets the content orientation, possibly making it differ from the surface's.
	// This relative difference is the angle calculated below and is used in the projection
	// transformation.
	// 
	// In contrast, on actual desktops (e.g. Mac), the OS windowing system doesn't
	// physically rotate, so we shouldn't have differences in content and surface
	// orientations. Here, we call GPUStream::SetOrientation(). Here the surface 
	// orientation is the actual orientation of the device screen as it appears
	// on the OS screen. During auto-rotation, the content orientation matches the
	// surface's. When there is no auto-rotation, we update fRotation which rotates 
	// all vertices via the modelview transformation.
	DeviceOrientation::Type orientation = Super::GetContentOrientation();
	S32 angle = DeviceOrientation::CalculateRotation( orientation, Super::GetSurfaceOrientation() );
	if ( 0 != angle )
	{
		glRotatef( Rtt_IntToReal( angle ), 0, 0, 1 );
	}

	const Real w = Rtt_IntToReal( renderedContentWidth );
	const Real h = Rtt_IntToReal( renderedContentHeight );

#if Rtt_3D
	Real wMin = -w;
	Real wMax = w;
	Real hMin = -h;
	Real hMax = h;
#else
	Real wMin = Rtt_REAL_0;
	Real wMax = w;
	Real hMin = Rtt_REAL_0;
	Real hMax = h;
#endif

	if ( IsProperty( kFlipHorizontalAxis ) )
	{
		Real tmp = hMin;
		hMin = hMax;
		hMax = tmp;
	}

	if ( IsProperty( kFlipVerticalAxis ) )
	{
		Real tmp = wMin;
		wMin = wMax;
		wMax = tmp;
	}

#if Rtt_3D
#ifdef _WINDOWS
#undef near
#undef far
#endif
	// 3D
	const Real near = Rtt::Max( w, h );     fNear = near;
	const Real far = Rtt_IntToReal( 5 );	fFar = far;
	Rtt_glFrustum(wMin, wMax, hMin, hMax, near, far );
#else
	// 2D
	Rtt_glOrtho(
		wMin, wMax,
		hMin, hMax,
		Rtt_REAL_1, Rtt_REAL_NEG_1 );

	fNear = Rtt_REAL_1;
	fFar = Rtt_REAL_NEG_1;
#endif

#endif
}

void
GPUStream::UpdateViewport( S32 windowWidth, S32 windowHeight )
{
#if 0
	fWindowWidth = windowWidth;
	fWindowHeight = windowHeight;
	glViewport( 0, 0, windowWidth, windowHeight );
#endif
}

S32
GPUStream::GetRenderedContentWidth() const
{
	Rtt_ASSERT( fRenderedContentWidth > 0 );
	return fRenderedContentWidth;
}

S32
GPUStream::GetRenderedContentHeight() const
{
	Rtt_ASSERT( fRenderedContentHeight > 0 );
	return fRenderedContentHeight;
}

Rtt_Real
GPUStream::ViewableContentWidth() const
{
	// The viewable content width is the smaller of the rendered content width
	// or the content width itself. The rendered width is the window width 
	// in *scaled* units as determined by UpdateContentScale().
	// 
	// Depending on the relationship of aspect ratios between the window and
	// the content, the rendered width might be larger (kLetterbox)
	// the same (kFillStretch), or smaller (kFillEven).
	S32 renderedContentWidth = GetRenderedContentWidth();
	S32 contentWidth = ContentWidth();
	return Min( contentWidth, renderedContentWidth );
}

Rtt_Real
GPUStream::ViewableContentHeight() const
{
	// See comment in ViewableContentWidth()
	S32 renderedContentHeight = GetRenderedContentHeight();
	S32 contentHeight = ContentHeight();
	return Min( contentHeight, renderedContentHeight );
}

Rtt_Real
GPUStream::ActualContentWidth() const
{
	Rtt_Real result = Rtt_REAL_0;

	switch ( GetScaleMode() )
	{
		// Fills the screen, so it's same as viewable bounds,
		// i.e. the intersection of screen with stretched content
		case Display::kZoomEven:
			result = ViewableContentWidth();
			break;

		// Extra border, so calculate from device width in pixels
		case Display::kLetterbox:
		{
			DeviceOrientation::Type currentOrientation = Super::GetContentOrientation();
			if ( DeviceOrientation::IsSideways( currentOrientation ) )
			{
				result = DeviceHeight() * GetSy();
			}
			else
			{
				result = DeviceWidth() * GetSx();
			}
			break;
		}
		// Stretches content to fill screen so same value
		case Display::kZoomStretch:
		case Display::kAdaptive:
			result = ContentWidth();
			break;

		// Content takes on screen bounds
		case Display::kNone:
		default:
			Rtt_ASSERT( ScreenWidth() == DeviceWidth() );
			result = DeviceWidth();
			break;
	}

	return result;
}

Rtt_Real
GPUStream::ActualContentHeight() const
{
	Rtt_Real result = Rtt_REAL_0;

	switch ( GetScaleMode() )
	{
		// Fills the screen, so it's same as viewable bounds,
		// i.e. the intersection of screen with stretched content
		case Display::kZoomEven:
			result = ViewableContentHeight();
			break;

		// Extra border, so calculate from device width in pixels
		case Display::kLetterbox:
		{
			DeviceOrientation::Type currentOrientation = Super::GetContentOrientation();
			if ( DeviceOrientation::IsSideways( currentOrientation ) )
			{
				result = DeviceWidth() * GetSx();
			}
			else
			{
				result = DeviceHeight() * GetSy();
			}
			break;
		}
		// Stretches content to fill screen so same value
		case Display::kZoomStretch:
		case Display::kAdaptive:
			result = ContentHeight();
			break;

		// Content takes on screen bounds
		case Display::kNone:
		default:
			Rtt_ASSERT( ScreenHeight() == DeviceHeight() );
			result = DeviceHeight();
			break;
	}

	return result;
}

Real
GPUStream::CalculateAlignmentOffset( Alignment alignment, Real contentLen, Real windowLen )
{
	Real result = Rtt_REAL_0;

	switch ( alignment )
	{
		case kAlignmentLeft:
			Rtt_STATIC_ASSERT( kAlignmentTop == kAlignmentLeft );
			result = - Rtt_RealDiv2( windowLen );
			break;
		case kAlignmentCenter:
			result = - Rtt_RealDiv2( contentLen );
			break;
		case kAlignmentRight:
			Rtt_STATIC_ASSERT( kAlignmentBottom == kAlignmentRight );
			result = - Rtt_RealDiv2( Rtt_RealMul2( contentLen ) - windowLen );
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return result;
}

void
GPUStream::UpdateOffsets( S32 renderedContentWidth, S32 renderedContentHeight )
{
	S32 w = renderedContentWidth;
	S32 h = renderedContentHeight;
	S32 contentW = ContentWidth();
	S32 contentH = ContentHeight();

	Alignment xAlign = GetXAlign();
	Alignment yAlign = GetYAlign();

	// When the launch orientation and current orientation are perpendicular
	// (one is sideways and one is vertical), then swap so that x/yAlign are
	// oriented the same as w/h and contentW/H.
	DeviceOrientation::Type launchOrientation = GetLaunchOrientation();
	DeviceOrientation::Type orientation = Super::GetContentOrientation();
	if ( DeviceOrientation::IsSideways( launchOrientation ) != DeviceOrientation::IsSideways( orientation ) )
	{
		Swap( xAlign, yAlign );
	}

	// Location of origin
	Real xOrigin = - Rtt_RealDiv2( Rtt_IntToReal( w ) );
	Real yOrigin = - Rtt_RealDiv2( Rtt_IntToReal( h ) ); 

	fXVertexOffset = CalculateAlignmentOffset( xAlign, contentW, w );
	fYVertexOffset = CalculateAlignmentOffset( yAlign, contentH, h );

	// Update origin offset
	Real x = fXVertexOffset - xOrigin;
	Real y = fYVertexOffset - yOrigin;

#ifdef RTT_SURFACE_ROTATION
	// For no auto-rotation, the screen origin is pinned to a physical location
	// on the device skin. In this case, if angle is 90 or -90, we need to swap 
	// x,y to get the origin offset relative to that pinned location. For other
	// angles the x,y values happen to be correct. For auto-rotation, the x,y 
	// values are likewise correct.
	DeviceOrientation::Type angleOrientation = DeviceOrientation::OrientationForAngle( fRotation );
	if ( DeviceOrientation::IsSideways( angleOrientation ) )
	{
		Rtt_ASSERT( ! fAutoRotate );
		Swap( x, y );
	}
#endif

	SetXOriginOffset( x );
	SetYOriginOffset( y );

//	Rtt_TRACE( ( "origin offset (x,y) = (%g,%g) [%d]\n", x, y, fRotation) );

#ifdef RTT_SURFACE_ROTATION
	// Ensure proper w,h is used for screen content bounds
	if ( DeviceOrientation::IsSideways( angleOrientation ) )
	{
		Rtt_ASSERT( ! fAutoRotate );
		Swap( w, h );
	}
#endif
	// The bounds of the screen in content coordinates.
	Rect& bounds = GetScreenContentBounds();
	bounds.xMin = -GetXOriginOffset();
	bounds.yMin = -GetYOriginOffset();
	bounds.xMax = w - GetXOriginOffset();
	bounds.yMax = h - GetYOriginOffset();
}

void
GPUStream::Reshape( S32 renderedContentWidth, S32 renderedContentHeight )
{
	UpdateOffsets( renderedContentWidth, renderedContentHeight );
	
	#ifdef RTT_SURFACE_ROTATION
		if ( fRotation )
		{
			Real xOffset = GetXOriginOffset();
			Real yOffset = GetYOriginOffset();

			// For no auto-rotation, the screen origin is pinned to a physical location
			// on the device skin that's not the same as the surface origin. In this case, 
			// if angle is 90 or -90, we need to swap x,y to get the correct screen offset.
			// For other angles and also for auto-rotation, the x,y values happen to be correct.
			DeviceOrientation::Type angleOrientation = DeviceOrientation::OrientationForAngle( fRotation );
			if ( DeviceOrientation::IsSideways( angleOrientation ) )
			{
				Rtt_ASSERT( ! fAutoRotate );
				Swap( xOffset, yOffset );
			}

			Real halfW = Rtt_RealDiv2( Rtt_IntToReal( renderedContentWidth ) ) - xOffset;
			Real halfH = Rtt_RealDiv2( Rtt_IntToReal( renderedContentHeight ) ) - yOffset;

			//Rtt_glTranslate( halfW, halfH, Rtt_REAL_0 );

			// For landscape, swap w,h
			if ( ! DeviceOrientation::IsAngleUpsideDown( fRotation ) )
			{
				Real tmp = halfW;
				halfW = halfH;
				halfH = tmp;
			}

			//glRotatef( Rtt_IntToReal( fRotation ), 0., 0., 1. );
			//Rtt_glTranslate( -halfW, -halfH, Rtt_REAL_0 );
		}
	#endif

#if 0
//	GLint windowWidth = width;
//	GLint windowHeight = height;

	glViewport( 0, 0, fWindowWidth, fWindowHeight );

	UpdateProjection( renderedContentWidth, renderedContentHeight );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	UpdateOffsets( renderedContentWidth, renderedContentHeight );

	#if Rtt_3D
		// (0,0) is in the center of the viewing frustum. We want it at the top-left.
		// Also, the camera faces negative-z and the camera is at z=0, so we need to
		// shift all vertices inside the frustum. The frustum was constructed so that
		// the back plane is twice the size of the screen (in content units), so we
		// shift all vertices halfway in between the camera and the back plane, i.e. fNear/2
		Rtt_glTranslate( fXVertexOffset, fYVertexOffset, - Rtt_RealDiv2( fNear ) );
	#endif

	#ifdef RTT_SURFACE_ROTATION
		if ( fRotation )
		{
			Real xOffset = GetXOriginOffset();
			Real yOffset = GetYOriginOffset();

			// For no auto-rotation, the screen origin is pinned to a physical location
			// on the device skin that's not the same as the surface origin. In this case, 
			// if angle is 90 or -90, we need to swap x,y to get the correct screen offset.
			// For other angles and also for auto-rotation, the x,y values happen to be correct.
			DeviceOrientation::Type angleOrientation = DeviceOrientation::OrientationForAngle( fRotation );
			if ( DeviceOrientation::IsSideways( angleOrientation ) )
			{
				Rtt_ASSERT( ! fAutoRotate );
				Swap( xOffset, yOffset );
			}

			Real halfW = Rtt_RealDiv2( Rtt_IntToReal( renderedContentWidth ) ) - xOffset;
			Real halfH = Rtt_RealDiv2( Rtt_IntToReal( renderedContentHeight ) ) - yOffset;

			Rtt_glTranslate( halfW, halfH, Rtt_REAL_0 );

			// For landscape, swap w,h
			if ( ! DeviceOrientation::IsAngleUpsideDown( fRotation ) )
			{
				Real tmp = halfW;
				halfW = halfH;
				halfH = tmp;
			}

			glRotatef( Rtt_IntToReal( fRotation ), 0., 0., 1. );
			Rtt_glTranslate( -halfW, -halfH, Rtt_REAL_0 );
		}
	#endif
#endif
}

bool
GPUStream::UpdateContentOrientation( DeviceOrientation::Type newOrientation )
{
	DeviceOrientation::Type current = Super::GetContentOrientation();
	bool result = ( newOrientation != current );

	if ( result )
	{
		// We swap scale factors below so inhibit swapping in call to super method
		Rtt_ASSERT( ! IsProperty( kInhibitSwap ) );
		SetProperty( kInhibitSwap, true );
		Super::SetContentOrientation( newOrientation );
		SetProperty( kInhibitSwap, false );

		S32 w = fRenderedContentWidth;
		S32 h = fRenderedContentHeight;

		// Swap w,h if current is portrait and desired is landscape (or vice versa)
		if ( Super::ShouldSwap( current, newOrientation ) )
		{
			Swap( w, h );
			SwapContentSize();
			SwapContentScale();
			// Don't swap vertex offsets or origin offsets.
			// These are updated via a call to Reshape() which calls UpdateOffsets()
		}

		fRenderedContentWidth = w;
		fRenderedContentHeight = h;
		fOrientedContentWidth = w;
		fOrientedContentHeight = h;
	}

	return result;
}

// Used to support auto-rotate on devices. For simulator, use SetOrientation().
// 
// You should not call this directly. Call Runtime::SetContentOrientation() instead.
// This function updates the content orientation (swapping various values as necessary)
// and then updates the projection matrix/viewing frustum, including rotating if
// the content and surface orientations differ
void
GPUStream::SetContentOrientation( DeviceOrientation::Type newOrientation )
{
	Rtt_ASSERT( newOrientation >= DeviceOrientation::kUnknown && newOrientation <= DeviceOrientation::kNumTypes );

	if ( UpdateContentOrientation( newOrientation ) )
	{
		Reshape( fRenderedContentWidth, fRenderedContentHeight );
	}
}

#ifdef RTT_SURFACE_ROTATION

// Used to support changing Corona view orientation in the simulator, where the window
// remains upright, even though we try to simulate a physically rotated device window.
// 
// When autoRotate is true, that means we want the origin/coordinate system
// of the content to be upright, i.e. the origin is at the top-left of whatever
// orientation the screen is in.
// When it's false, then the content will rotate along with the screen, i.e. the 
// content's origin stays fixed *relative* to the screen as it rotates.
void
GPUStream::SetOrientation( DeviceOrientation::Type newOrientation, bool autoRotate )
{
	bool shouldReshape = UpdateContentOrientation( newOrientation );

	Rtt_ASSERT( DeviceOrientation::IsInterfaceOrientation( newOrientation ) );

	DeviceOrientation::Type oldSurfaceValue = Super::GetSurfaceOrientation();
	if ( newOrientation != oldSurfaceValue )
	{
		shouldReshape = true;
		SetSurfaceOrientation( newOrientation );

		// When mode is not auto-rotate, we want to rotate the model view matrix to preserve
		// the orientation of the content relative to oldSurfaceValue.
		if ( ! autoRotate )
		{
			// Relative to previous orientation
			S32 angle = fRotation - DeviceOrientation::CalculateRotation( oldSurfaceValue, newOrientation );;
			if ( angle > 270 ) { angle -= 360; }
			else if ( angle < -270 ) { angle += 360; }

			fRotation = angle;
		}
		else
		{
			// When we auto-rotate, we don't want any relative orientation angle
			// between content and surface.
			fRotation = 0;
		}

		if ( Super::ShouldSwap( oldSurfaceValue, newOrientation ) )
		{
			Swap( fWindowWidth, fWindowHeight );
		}
	}

	if ( shouldReshape )
	{
		Rtt_ASSERT( ! fAutoRotate );
		fAutoRotate = autoRotate;
		Reshape( fRenderedContentWidth, fRenderedContentHeight );
		fAutoRotate = false;
	}
}

DeviceOrientation::Type
GPUStream::GetRelativeOrientation() const
{
	S8 result = Super::GetRelativeOrientation();

	if ( 0 != fRotation )
	{
		S8 delta = 0;
		switch ( DeviceOrientation::OrientationForAngle( fRotation ) )
		{
			case DeviceOrientation::kSidewaysLeft:
				delta = -1;
				break;
			case DeviceOrientation::kSidewaysRight:
				delta = 1;
				break;
			case DeviceOrientation::kUpsideDown:
				delta = 2;
				break;
			default:
				Rtt_ASSERT_NOT_REACHED();
				break;
		}

		result += delta;
		if ( result > DeviceOrientation::kSidewaysLeft )
		{
			result -= DeviceOrientation::kSidewaysLeft;
		}
		else if ( result < DeviceOrientation::kUpright )
		{
			result += DeviceOrientation::kSidewaysLeft;
		}
	}

	return (DeviceOrientation::Type)result;
}

#endif

#if 0
void
GPUStream::SetBlendMode( RenderTypes::BlendMode mode )
{
	// Compare requested blend mode with current blend mode. Only set when
	// different to minimize gl state changes.
	if ( GetBlendMode() != mode )
	{
		Super::SetBlendMode( mode );

		GLenum sfactor = GL_SRC_ALPHA;
		GLenum dfactor = GL_ONE_MINUS_SRC_ALPHA;

		switch ( mode )
		{
			// Normal: pixel = src*alphaSrc + dst*(1-srcAlpha)
			case RenderTypes::kNormalNonPremultiplied:
				// (R,G,B,A) = (rSrc,gSrc,bSrc,aSrc)*aSrc + (rDst,gDst,bDst,aDst)*(1-aSrc)
				//                                     ^                              ^
				//                                GL_SRC_ALPHA      GL_ONE_MINUS_SRC_ALPHA
				sfactor = GL_SRC_ALPHA;
				dfactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case RenderTypes::kNormalPremultiplied:
				// (R,G,B,A) = (aSrc*[rSrc,gSrc,bSrc],aSrc)*1 + (rDst,gDst,bDst,aDst)*(1-aSrc)
				//                                          ^                             ^
				//                                        GL_ONE        GL_ONE_MINUS_SRC_ALPHA
				sfactor = GL_ONE;
				dfactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			// Additive: pixel = src + dst
			case RenderTypes::kAdditiveNonPremultiplied:
				// (R,G,B,A) = (rSrc,gSrc,bSrc,aSrc)*aSrc + (rDst,gDst,bDst,aDst)*1
				//                                    ^                           ^
				//                               GL_SRC_ALPHA                GL_ONE
				sfactor = GL_SRC_ALPHA;
				dfactor = GL_ONE;
				break;
			case RenderTypes::kAdditivePremultiplied:
				// (R,G,B,A) = (aSrc*[rSrc,gSrc,bSrc],aSrc)*1 + (rDst,gDst,bDst,aDst)*1
				//                                          ^                         ^
				//                                        GL_ONE                 GL_ONE
				sfactor = GL_ONE;
				dfactor = GL_ONE;
				break;
			// Screen: pixel = 1 - (1-src)*(1-dst) = src + dst*(1-src)
			case RenderTypes::kScreenNonPremultiplied:
				// (R,G,B,A) = Src + (1 - Src) * Dst
				// (R,G,B,A) = (rSrc,gSrc,bSrc,aSrc)*aSrc + (rDst,gDst,bDst,aDst)*((1 - Src)*aSrc)
				//                                    ^                           ^^^^^^^^^^^^^^^^
				//                         GL_SRC_ALPHA  GL_ONE_MINUS_SRC_COLOR_TIMES_SRC_ALPHA (?)
				sfactor = GL_SRC_ALPHA;
				dfactor = GL_ONE_MINUS_SRC_COLOR;
				break;
			case RenderTypes::kScreenPremultiplied:
				// (R,G,B,A) = Src + (1 - Src) * Dst
				// (R,G,B,A) = (aSrc*[rSrc,gSrc,bSrc],aSrc)*1 + (rDst,gDst,bDst,aDst)*(1 - aSrc*[rSrc,gSrc,bSrc],aSrc)
				//                                          ^                         ^
				//                                        GL_ONE      GL_ONE_MINUS_SRC_COLOR
				sfactor = GL_ONE;
				dfactor = GL_ONE_MINUS_SRC_COLOR;
				break;
			// Multiply: pixel = src * dst
			case RenderTypes::kMultiplyNonPremultiplied:
				// (R,G,B,A) = {foreground}*(rDst,gDst,bDst,aDst) + (rDst,gDst,bDst,aDst)*(1-aSrc)
				//                                    ^                           ^
				//                               GL_DST_COLOR          GL_ONE_MINUS_SRC_ALPHA
				sfactor = GL_DST_COLOR;
				dfactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case RenderTypes::kMultiplyPremultiplied:
				// (R,G,B,A) = {foreground}*(rDst,gDst,bDst,aDst) + (rDst,gDst,bDst,aDst)*(1-aSrc)
				//                                    ^                           ^
				//                               GL_DST_COLOR          GL_ONE_MINUS_SRC_ALPHA
				sfactor = GL_DST_COLOR;
				dfactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case RenderTypes::kDstAlphaZero:
				sfactor = GL_ZERO;
				dfactor = GL_ZERO;
				break;
			case RenderTypes::kDstAlphaAccumulate:
				sfactor = GL_ONE;
				dfactor = GL_ONE;
				break;
			case RenderTypes::kDstAlphaModulateSrc:
				sfactor = GL_DST_ALPHA;
				dfactor = GL_ZERO;
				break;
			case RenderTypes::kDstAlphaBlit:
				sfactor = GL_DST_ALPHA;
				dfactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			default:
				Rtt_ASSERT_NOT_REACHED();
				break;
		}

		glBlendFunc( sfactor, dfactor );
	}
}
#endif

void
GPUStream::SetClearColor( const Paint& paint )
{
#if 0
	RGBA rgba = paint.GetRGBA();
	GLfloat kInv255 = 1.0f / 255.0f;
	fClearR = kInv255 * rgba.r;
	fClearG = kInv255 * rgba.g;
	fClearB = kInv255 * rgba.b;
	fClearA = kInv255 * rgba.a;
	glClearColor( fClearR, fClearG, fClearB, fClearA );
#endif
}

RGBA
GPUStream::GetClearColor()
{
	RGBA color;
	color.r = (U8)(fClearR * 255.0f);
	color.g = (U8)(fClearG * 255.0f);
	color.b = (U8)(fClearB * 255.0f);
	color.a = (U8)(fClearA * 255.0f);
	return color;
}

void
GPUStream::Clear()
{
#if 0
	glClear( GL_COLOR_BUFFER_BIT );
#endif
}

void
GPUStream::SetTextureCoordinates( const Vertex2 *coords, S32 numCoords )
{
	fTextureCoords = coords;
	fNumTextureCoords = numCoords;
}

// Pushes paint onto texture stack and prevents changes to the state of the
// current texture unit. Sets active texture unit to next available one. 
// 
// This function is smart enough to accept NULL. It also will push up to
// a texture stack limit which is defined by the GPU hardware (the max number
// of texture units).
// 
// Assumptions:
// * Precondition: Assumes you called paint->Begin() so that texture state
//   is set for currently active texture unit.
// * Postcondition: sets active texture unit to be next available one
//   so future invocations to BitmapPaint::Begin() do not affect the state 
//   of the texture unit that was active prior to the call. 
void
GPUStream::PushTexture( const BitmapPaint *paint )
{
#if 0
	if ( paint )
	{
		// index of next empty slot
		int index = fTextureStackNumActiveFrames;

		if ( index < fTextureStackSize )
		{
			TextureStackFrame& frame = fTextureStack[index];
			frame.paint = paint;
			frame.depth = fTextureStackDepth;

			#ifdef Rtt_DEBUG
			{
				GLint maxTextureUnits;
				glGetIntegerv( GL_MAX_TEXTURE_UNITS, & maxTextureUnits );
				Rtt_ASSERT( maxTextureUnits > 1 );

				GLint curTextureUnit;
				glGetIntegerv( GL_ACTIVE_TEXTURE, &curTextureUnit );
				Rtt_ASSERT( curTextureUnit == (GLint)GetTextureUnit( index ) );
			}
			#endif

			// This is the "push"
			// All future texture state changes will apply to the next texture unit
			if ( glActiveTexture != NULL )
			{
				GLenum textureUnit = GetTextureUnit( index + 1 );
				glActiveTexture( textureUnit ); GPUError();
			}

/*
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE ); GPUError();

			glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE ); GPUError();
			glTexEnvf( GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS ); GPUError();
			glTexEnvf( GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR ); GPUError();

			glTexEnvf( GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE ); GPUError();
			glTexEnvf( GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PREVIOUS ); GPUError();
			glTexEnvf( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA ); GPUError();
			glTexEnvf( GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_TEXTURE ); GPUError();
			glTexEnvf( GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA ); GPUError();
*/

			// Make sure this comes at the end since this affects the texture depth
			++fTextureStackNumActiveFrames;
		}
	}

	// Always increment depth b/c this is paired with PopTexture
	// This is the nesting level of PushTexture() calls that have not had their
	// corresponding PopTexture() called.
	++fTextureStackDepth;
#endif
}

// Pops top-most paint off of texture stack and returns it. May return NULL.
// Also, sets active texture unit to previous available one. 
// 
// Assumptions:
// * Postcondition: Assumes you called paint->End() on the returned paint.
const BitmapPaint*
GPUStream::PopTexture()
{
	Rtt_ASSERT( fTextureStackDepth > 0 );

	const BitmapPaint *result = NULL;

#if 0
	// Always decrement first b/c we want the index of the top-most element on the stack
	--fTextureStackDepth;

	// index of top stack element
	int index = fTextureStackNumActiveFrames - 1;

	if ( index >= 0 && Rtt_VERIFY( index < fTextureStackSize ) )
	{
		TextureStackFrame& frame = fTextureStack[index];
		if ( frame.paint && frame.depth == fTextureStackDepth )
		{
			--fTextureStackNumActiveFrames;

			const BitmapPaint *paint = frame.paint; Rtt_ASSERT( paint );
			frame.paint = NULL;
			frame.depth = 0;

			// This is the "pop"
			// Texture state changes now apply to the previous texture unit
			if ( glActiveTexture != NULL )
			{
				GLenum textureUnit = GetTextureUnit( index );
				glActiveTexture( textureUnit ); GPUError();
			}

			result = paint;
		}
	}
#endif
	return result;
}

int
GPUStream::GetTextureDepth() const
{
	return GetActiveTextureCount();
}

void
GPUStream::SetMatrixMode( MatrixMode mode )
{
#if 0
	GLenum m = GLMatrixModeForMode( mode );
	glMatrixMode( m ); GPUError();
#endif
}

void
GPUStream::LoadIdentity()
{
#if 0
	glLoadIdentity(); GPUError();
#endif
}

#if 0
static GLenum
GLCapabilityForCapabilityMask( GPUStream::CapabilityMask mask )
{
	GLenum result = 0;
	switch ( mask )
	{
		case GPUStream::kCapabilityTexture2D:
			result = GL_TEXTURE_2D;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return result;
}
#endif // 0

void
GPUStream::SetEnabled( CapabilityMask mask, bool value )
{
#if 0
	Super::SetEnabled( mask, value );

	GLenum cap = GLCapabilityForCapabilityMask( mask );

	// TODO: Performance. We need to optimize state changes away.
	// For texture state, we need to account for the active texture unit.
	{
		if ( value )
		{
			glEnable( cap );
		}
		else
		{
			glDisable( cap );
		}
	}
#endif
}

void
GPUStream::SetTextureId( U32 textureId )
{
#if 0
	glBindTexture( GL_TEXTURE_2D, textureId ); GPUError();
#endif
}

void
GPUStream::SetTextureParameter( TextureParameter param, TextureValue value )
{
#if 0
	GLenum pname = GLTexParamNameForParam( param );
	GLint pvalue = GLTexParamValueForValue( value );
	glTexParameteri( GL_TEXTURE_2D, pname, pvalue ); GPUError();
#endif
}

void
GPUStream::SetTextureFunction( TextureFunction func )
{
#if 0
	// TODO: Account for multiple active texture units before optimizing this state change
	{
		fTextureFunction = func;

		GLfloat param = GLTexEnvParamForFunction( func );
		glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, param ); GPUError();
	}
#endif
}

void
GPUStream::SetColor( const RGBA& rgba )
{
#ifdef OLD_GRAPHICS
	U32 oldColor = *(const U32*)(&fColor);
	U32 newColor = *(const U32*)(&rgba);

	if ( newColor != oldColor  )
	{
		fColor = rgba;

#if defined( Rtt_ANDROID_ENV )
		// Some (all?) Android devices do not support glColor4ub
		GLfloat kInv255 = 1.f / 255.0f;
		glColor4f( kInv255*rgba.r, kInv255*rgba.g, kInv255*rgba.b, kInv255*rgba.a ); GPUError();
#else
		glColor4ub( rgba.r, rgba.g, rgba.b, rgba.a ); GPUError();
#endif
	}
#endif
}

void
GPUStream::BeginPaint( const Paint* paint )
{
#ifdef OLD_GRAPHICS
	Rtt_ASSERT( ! fCurrentPaint );
	Rtt_ASSERT( ! fTextureCoords );

	Super::BeginPaint( paint );
	fCurrentPaint = paint;

	if ( paint )
	{
		const BitmapPaint *bitmapPaint = (const BitmapPaint*)paint->AsPaint( Paint::kBitmap );
		Rtt_ASSERT( ( bitmapPaint && paint->IsType( Paint::kBitmap ) ) || ! bitmapPaint );
		bool isCurrentPaintTexture = ( NULL != bitmapPaint );
		fIsTexture = isCurrentPaintTexture;

		paint->Begin( * this );
		PushTexture( ( isCurrentPaintTexture ? bitmapPaint : NULL ) );
	}
#endif
}

void
GPUStream::EndPaint()
{
#ifdef OLD_GRAPHICS
	const Paint* paint = fCurrentPaint;

	if ( paint )
	{
		fCurrentPaint = NULL;
		fTextureCoords = NULL;
		fNumTextureCoords = 0;

		const BitmapPaint *bitmapPaint = PopTexture(); Rtt_UNUSED( bitmapPaint );
		Rtt_ASSERT( NULL == bitmapPaint || paint == (const Paint*)bitmapPaint );
		paint->End( * this );
	}
#endif
}

const BitmapPaint*
GPUStream::GetBitmap() const
{
	Rtt_ASSERT( fCurrentPaint->IsType( Paint::kBitmap ) == fIsTexture );
	const BitmapPaint *result = NULL;
	if ( fIsTexture )
	{
		result = (const BitmapPaint*)fCurrentPaint;
	}
	else if ( GetTextureDepth() > 0 )
	{
		result = fTextureStack[0].paint;
	}

	return result;
}

U8
GPUStream::GetAlpha() const
{
	return fAlpha;
}

U8
GPUStream::SetAlpha( U8 newValue, bool accumuluate )
{
	U8 result = fAlpha;

	if ( accumuluate )
	{
		if ( newValue < 0xFF )
		{
			fAlpha = (((U16)result) * newValue) >> 8;
		}

		// when newValue is 0xFF, complement is 0; in that case, no need to multiply
//		U32 inverse = ~newValue;
//		fAlpha = ( inverse ? (((U16)result) * newValue) >> 8 : newValue );
	}
	else
	{
		fAlpha = newValue;
	}

	return result;
}

GLenum
GPU_GetPixelFormat( PlatformBitmap::Format format )
{
#	ifdef Rtt_OPENGLES

		switch ( format )
		{
			case PlatformBitmap::kRGBA:
				return GL_RGBA;
			case PlatformBitmap::kMask:
				return GL_ALPHA;
			case PlatformBitmap::kRGB:
			case PlatformBitmap::kBGRA:
			case PlatformBitmap::kARGB:
			default:
				Rtt_ASSERT_NOT_IMPLEMENTED();
				return GL_ALPHA;
		}

#	else // Not Rtt_OPENGLES.

		switch ( format )
		{
			case PlatformBitmap::kBGRA:
			case PlatformBitmap::kARGB:
				return GL_BGRA;
			case PlatformBitmap::kRGBA:
				return GL_RGBA;
			case PlatformBitmap::kRGB:
				return GL_BGR;
			case PlatformBitmap::kMask:
				return GL_ALPHA;
			default:
				Rtt_ASSERT_NOT_IMPLEMENTED();
				return GL_ALPHA;
		}

#	endif // Rtt_OPENGLES
}

GLenum
GPU_GetPixelType( PlatformBitmap::Format format )
{
#	ifdef Rtt_OPENGLES

		switch( format )
		{
			case PlatformBitmap::kMask:
				return GL_UNSIGNED_BYTE;
			case PlatformBitmap::kBGRA:
			case PlatformBitmap::kARGB:
			case PlatformBitmap::kRGBA:
			default:
				Rtt_ASSERT_NOT_IMPLEMENTED();
				return GL_UNSIGNED_BYTE;
		}

#	else // Not Rtt_OPENGLES.

		switch( format )
		{
			case PlatformBitmap::kBGRA:
				#ifdef Rtt_BIG_ENDIAN
					return GL_UNSIGNED_INT_8_8_8_8_REV;
				#else
					return GL_UNSIGNED_INT_8_8_8_8;
				#endif
			case PlatformBitmap::kARGB:
				#ifdef Rtt_BIG_ENDIAN
					return GL_UNSIGNED_INT_8_8_8_8;
				#else
					return GL_UNSIGNED_INT_8_8_8_8_REV;
				#endif
			case PlatformBitmap::kRGBA:
				#ifdef Rtt_BIG_ENDIAN
					return GL_UNSIGNED_INT_8_8_8_8_REV;
				#else
					return GL_UNSIGNED_INT_8_8_8_8;
				#endif
			case PlatformBitmap::kMask:
				return GL_UNSIGNED_BYTE;
			default:
				Rtt_ASSERT_NOT_IMPLEMENTED();
				return GL_UNSIGNED_BYTE;
		}

#	endif // Rtt_OPENGLES
}

// Performs a screen capture and outputs the image to the given "outBuffer" bitmap.
void
GPUStream::CaptureFrameBuffer( BufferBitmap& outBuffer, S32 xScreen, S32 yScreen, S32 wScreen, S32 hScreen )
{
//	GLint x = Rtt_RealToInt( bounds.xMin );
//	GLint y = Rtt_RealToInt( bounds.yMin );
//	GLint w = Rtt_RealToInt( bounds.xMax ) - x;
//	GLint h = Rtt_RealToInt( bounds.yMax ) - y;
#ifdef Rtt_OPENGLES
	const GLenum kFormat = GL_RGBA;
	const GLenum kType = GL_UNSIGNED_BYTE;
#else
	PlatformBitmap::Format format = outBuffer.GetFormat();
	const GLenum kFormat = GPU_GetPixelFormat( format );
	GLenum kType = GPU_GetPixelType( format );
#endif

	glReadPixels( xScreen,
					yScreen,
					wScreen,
					hScreen,
					kFormat,
					kType,
					outBuffer.WriteAccess() );
}

void
GPUStream::Begin3D( const Xform3D* transform, const Rect& bounds )
{
#if 0
	#if defined( Rtt_DEBUG ) && !defined( Rtt_ANDROID_ENV )
		GLint matrixMode;
		glGetIntegerv( GL_MATRIX_MODE, & matrixMode );
		Rtt_ASSERT( matrixMode == GL_MODELVIEW );
		GLint maxStackDepth, stackDepth;
		glGetIntegerv( GL_MAX_MODELVIEW_STACK_DEPTH, & maxStackDepth );
		glGetIntegerv( GL_MODELVIEW_STACK_DEPTH, & stackDepth );
		Rtt_ASSERT( stackDepth < maxStackDepth );
	#endif

	if ( transform )
	{
		glPushMatrix(); GPUError();

		// Origin in OGL is the center of screen, so we need to center
		// the object's anchor point to the origin.  Currently, the anchor point
		// is the center of the object's bounds.
		//
		// Typically, only need to do this in a root-level Stage, b/c 
		// children will "inherit" this origin remapping
		const Real xOffset = Rtt_RealDiv2( bounds.Width() ) + bounds.xMin;
		const Real yOffset = Rtt_RealDiv2( bounds.Height() ) + bounds.yMin;

		Rtt_glTranslate( xOffset, yOffset, Rtt_REAL_0 ); GPUError();

		// Note that this translate consolidates both the offset and the transform values
		Rtt_glTranslate( transform->tx, transform->ty, transform->tz ); GPUError();
		Rtt_glRotate( transform->angle, transform->x, transform->y, transform->z ); GPUError();
		Rtt_glScale( transform->sx, transform->sy, Rtt_REAL_1 ); GPUError();

		Rtt_glTranslate( - xOffset, - yOffset, Rtt_REAL_0 ); GPUError();
	}
#endif
}

void
GPUStream::End3D( const Xform3D* transform )
{
#if 0
	if ( transform )
	{
		glPopMatrix(); GPUError();
	}
#endif
}

/*
void
GPUStream::SetScale( Real sx, Real sy )
{
	if ( ! Rtt_RealEqual( GetSx(), sx ) || ! Rtt_RealEqual( GetSy(), sy ) )
	{
		// TODO: This is incomplete. 
		Rtt_ASSERT_NOT_IMPLEMENTED();

		Super::SetScale( sx, sy );
		fRenderedContentWidth = Rtt_RealToInt( Rtt_RealMul( sx, fWindowWidth ) );
		fRenderedContentHeight = Rtt_RealToInt( Rtt_RealMul( sy, fWindowHeight ) );
		UpdateProjection( fRenderedContentWidth, fRenderedContentHeight );

		glMatrixMode( GL_MODELVIEW );
	}
}
*/

/*
static const Vertex2 kTex[] =
{
	{ Rtt_REAL_0, Rtt_REAL_0 },
	{ Rtt_REAL_0, Rtt_REAL_1 },
	{ Rtt_REAL_1, Rtt_REAL_1 },
	{ Rtt_REAL_1, Rtt_REAL_0 }
};
*/

// ----------------------------------------------------------------------------

class BitmapContext
{
	Rtt_CLASS_NO_DYNAMIC_ALLOCATION
	Rtt_CLASS_NO_COPIES( BitmapContext )

	public:
		BitmapContext( bool isBitmap, int numActiveTextures, const Vertex2 *texVertices, const Vertex2 *dstVertices );
		~BitmapContext();

	private:
		bool fIsBitmap; // true if fCurrentPaint is a bitmap; false otherwise
		int fNumActiveTextures; // index of current active texture unit
};

BitmapContext::BitmapContext( bool isBitmap, int numActiveTextures, const Vertex2 *texVertices, const Vertex2 *dstVertices )
:	fIsBitmap( isBitmap ),
	fNumActiveTextures( numActiveTextures )
{
#if 0
	bool hasMask = numActiveTextures > 0;
	if ( isBitmap || hasMask )
	{
		if ( numActiveTextures > 1 && glClientActiveTexture != NULL )
		{
			// The top most texture unit is not a mask, so feed it the
			// image's texture coordinates
			GLenum textureUnit = GetTextureUnit( numActiveTextures - 1 );
			glClientActiveTexture( textureUnit );
			glEnableClientState( GL_TEXTURE_COORD_ARRAY ); GPUError();
			glTexCoordPointer( 2, kDataType, 0, texVertices ); GPUError();

			// Any additional textures must use custom texture matrix mapping.
			// Their tex coords are derived via mapping dst object coords (dstVertices).
			for ( int i = numActiveTextures - 1; --i >= 0; )
			{
				GLenum textureUnit = GetTextureUnit( i );
				
				glClientActiveTexture( textureUnit );
				glEnableClientState( GL_TEXTURE_COORD_ARRAY ); GPUError();
				glTexCoordPointer( 2, kDataType, 0, dstVertices ); GPUError();				
			}

			glClientActiveTexture( GL_TEXTURE0 );
		}
		else
		{
			glEnableClientState( GL_TEXTURE_COORD_ARRAY ); GPUError();
			glTexCoordPointer( 2, kDataType, 0, texVertices ); GPUError();
		}
	}
#endif
}

BitmapContext::~BitmapContext()
{
#if 0
	bool hasMask = fNumActiveTextures > 0;

	if ( fIsBitmap || hasMask )
	{
		if ( fNumActiveTextures > 1 && glClientActiveTexture != NULL )
		{
			for ( int i = fNumActiveTextures; --i > 0; )
			{
				GLenum textureUnit = GetTextureUnit( i );
				
				glClientActiveTexture( textureUnit );
				glDisableClientState( GL_TEXTURE_COORD_ARRAY ); GPUError();
			}

			glClientActiveTexture( GL_TEXTURE0 );
		}

		glDisableClientState( GL_TEXTURE_COORD_ARRAY ); GPUError();
	}
#endif
}

// ----------------------------------------------------------------------------

#if 0

void
GPUStream::Submit( const VertexArray& vertices )
{
	for ( int i = 0, iMax = vertices.GetNumAttributes(); i < iMax; i++ )
	{
		int numComponents = vertices.GetNumComponents( i );
		int dataType = GLTypeForType( vertices.GetComponentType( i ) );
		size_t bytesPerStride = vertices.GetBytesPerStride( i );
		const void *ptr = vertices.ReadAccess( i );

		VertexArray::Attribute attribute = vertices.GetAttributeType( i );
		switch ( attribute )
		{
			case VertexArray::kPositionAttribute:
				// No-op: always enabled
				// glEnableClientState( GL_VERTEX_ARRAY );
				glVertexPointer( numComponents, dataType, bytesPerStride, ptr ); GPUError();
				break;
			case VertexArray::kTextureCoordAttribute:
				glEnableClientState( GL_TEXTURE_COORD_ARRAY ); GPUError();
				glTexCoordPointer( numComponents, dataType, bytesPerStride, ptr ); GPUError();
				break;
			case VertexArray::kColorAttribute:
				glEnableClientState( GL_COLOR_ARRAY ); GPUError();
				glColorPointer( numComponents, dataType, bytesPerStride, ptr ); GPUError();
				break;
			case VertexArray::kNormalAttribute:
				glEnableClientState( GL_NORMAL_ARRAY ); GPUError();
				glNormalPointer( dataType, bytesPerStride, ptr ); GPUError();
				break;
			default:
				Rtt_ASSERT_NOT_IMPLEMENTED();
				break;
		}
	}

	if ( Rtt_PAINT_TRIANGLES( fCurrentPaint ) )
	{
		GLenum mode = GLModeForMode( vertices.GetMode() );
		glDrawArrays( mode, 0, vertices.Length( 0 ) ); GPUError();
	}

	for ( int i = 0, iMax = vertices.GetNumAttributes(); i < iMax; i++ )
	{
		VertexArray::Attribute attribute = vertices.GetAttributeType( i );

		switch ( attribute )
		{
			case VertexArray::kTextureCoordAttribute:
				glDisableClientState( GL_TEXTURE_COORD_ARRAY );
				break;
			case VertexArray::kColorAttribute:
				glDisableClientState( GL_COLOR_ARRAY );
				break;
			case VertexArray::kNormalAttribute:
				glDisableClientState( GL_NORMAL_ARRAY );
				break;
			default:
				break;
		}
	}

#ifdef Rtt_DEBUG
	if ( ! Rtt_PAINT_TRIANGLES( fCurrentPaint ) )
	{
		bool overridePaint = ! fCurrentPaint->IsType( Paint::kColor );
		if ( overridePaint )
		{
			fCurrentPaint->End( * this);
			
			fColor.a = 0xFE; // Force color to be submitted
			Paint::White().Begin( * this );
		}

		GLenum modeGL = GL_LINE_LOOP;

		int attributeIndex = vertices.GetAttributeIndex( VertexArray::kPositionAttribute );
		const U8 *p = (U8*)vertices.ReadAccess( attributeIndex );
		size_t bytesPerStide = vertices.GetBytesPerStride( attributeIndex );
		// size_t numWords = vertices.GetNumComponents( attributeIndex );

		// For tight packing, the stride step is just the number of bytes per tuple
		if ( 0 == bytesPerStide )
		{
			ArrayTuple::Type t = vertices.GetComponentType( attributeIndex );
			size_t bytesPerComponent = ArrayTuple::GetTypeSize( t );
			int numComponents = vertices.GetNumComponents( attributeIndex );
			bytesPerStide = bytesPerComponent * numComponents;
		}

		// TODO: Only supports float positions for now
		Rtt_ASSERT( ArrayTuple::kFloat == vertices.GetComponentType( attributeIndex ) );

		const GLfloat *ptr = (const GLfloat*)p;

		switch ( vertices.GetMode() )
		{
			case RenderTypes::kStrip:
			{
				Vertex2 a = { ptr[0], ptr[1] };
				p += bytesPerStide; ptr = (const GLfloat*)p; // advance to next
				Vertex2 b = { ptr[0], ptr[1] };
				p += bytesPerStide; ptr = (const GLfloat*)p; // advance to next

				for ( int i = 2, iMax = vertices.Length(); i < iMax; i++ )
				{
					Vertex2 c = { ptr[0], ptr[1] };

					// Draw outlines of triangle strips
					Vertex2 tri[] = { a, b, c };
Rtt_TRACE( ( "[a] (%g,%g) %d\n", a.x, a.y, i-2 ) );
Rtt_TRACE( ( "[b] (%g,%g) %d\n", b.x, b.y, i-1 ) );
Rtt_TRACE( ( "[c] (%g,%g) %d\n", c.x, c.y, i ) );
Rtt_TRACE( ( "------------------\n" ) );
					glVertexPointer( 2, kDataType, 0, tri ); GPUError();
					glDrawArrays( modeGL, 0, sizeof(tri)/sizeof(tri[0]) ); GPUError();

					// Next iteration:
					a = b;
					b = c;
					p += bytesPerStide; ptr = (const GLfloat*)p; // advance to next
				}
			}
			break;

			case RenderTypes::kTriangle:
			{
				Rtt_ASSERT( 0 == ( vertices.Length() % 3 ) ); // should be multiple of 3

				for ( int i = 0, iMax = vertices.Length() / 3; i < iMax; i++ )
				{
					Vertex2 a = { ptr[0], ptr[1] };
					p += bytesPerStide; ptr = (const GLfloat*)p; // advance to next
					Vertex2 b = { ptr[0], ptr[1] };
					p += bytesPerStide; ptr = (const GLfloat*)p; // advance to next
					Vertex2 c = { ptr[0], ptr[1] };
					p += bytesPerStide; ptr = (const GLfloat*)p; // advance to next

					// Draw outlines of triangle strips
					Vertex2 tri[] = { a, b, c };
/*
Rtt_TRACE( ( "[a] (%g,%g) %d\n", a.x, a.y, i*3 ) );
Rtt_TRACE( ( "[b] (%g,%g) %d\n", b.x, b.y, i*3+1 ) );
Rtt_TRACE( ( "[c] (%g,%g) %d\n", c.x, c.y, i*3+2 ) );
Rtt_TRACE( ( "------------------\n" ) );
*/
					glVertexPointer( 2, kDataType, 0, tri ); GPUError();
					glDrawArrays( modeGL, 0, sizeof(tri)/sizeof(tri[0]) ); GPUError();
				}
			}
			break;

			default:
				break;
		}

		if ( overridePaint )
		{
			Paint::White().End( * this );
			fCurrentPaint->Begin( * this );
		}
	}
#endif
}

void
GPUStream::Submit( RenderTypes::Mode mode, const Vertex2* vertices, const S32* counts, S32 numCounts )
{
	if ( Rtt_PAINT_TRIANGLES( fCurrentPaint ) )
	{
		Rtt_ASSERT( RenderTypes::kFan == mode || RenderTypes::kStrip == mode );

		Rtt_ASSERT( fCurrentPaint->IsType( Paint::kBitmap ) == fIsTexture );
		bool isBitmap = fIsTexture;
		if ( isBitmap )
		{
			/*
			if ( ! textureVertices )
			{
				textureVertices = vertices;
			}
			*/

			Rtt_ASSERT_NOT_IMPLEMENTED();
			// TODO: Set up *texture* matrix transform as needed
			// bitmapPaint->SetBitmapTransform( * this );
		}

		// Begin non-zero
		const Quad* quad = SubmitBounds();
		bool isNonZero = quad && IsProperty( kNonZeroWinding );
		if ( isNonZero )
		{
			BeginNonZero( *quad );
			Rtt_glColor4( Rtt_REAL_0, Rtt_REAL_0, Rtt_REAL_0, Rtt_REAL_1 );
		}

		// Draw triangles
		glVertexPointer( 2, kDataType, 0, vertices ); GPUError();
		BitmapContext guard( fIsTexture, GetActiveTextureCount(), vertices, vertices );

		GLint first = 0;
		GLenum modeGL = GLModeForMode( mode );
		for ( int i = 0; i < numCounts; i++ )
		{
			GLint count = counts[i];
			glDrawArrays( modeGL, first, count ); GPUError();
			first += count;
		}

		// End non-zero
		if ( isNonZero )
		{
			Rtt_ASSERT( ! fCurrentPaint->IsType( Paint::kBitmap ) );

			EndNonZero( *quad, fCurrentPaint );
		}
	}
#ifdef Rtt_DEBUG
	else if ( fCurrentPaint )
	{
		// Bitmap colors treated as white
		bool overridePaint = ! fCurrentPaint->IsType( Paint::kColor );
		if ( overridePaint )
		{
			fCurrentPaint->End( * this);

			fColor.a = 0xFE; // Force color to be submitted
			Paint::White().Begin( * this );
		}

		GLenum modeGL = GL_LINE_LOOP;
		for ( int i = 0; i < numCounts; i++ )
		{
			const GLint count = counts[i];

			if ( RenderTypes::kFan == mode )
			{
				for ( int j = 1, jMax = count - 1; j < jMax; j++ )
				{
					Vertex2 tri[4];
					tri[0] = vertices[0];
					tri[1] = vertices[j];
					tri[2] = vertices[j+1];
					tri[3] = vertices[0];

					glVertexPointer( 2, kDataType, 0, tri ); GPUError();
					glDrawArrays( modeGL, 0, sizeof(tri)/sizeof(tri[0]) ); GPUError();
				}
			}
			else
			{
				Rtt_ASSERT( RenderTypes::kStrip == mode );

				for ( int j = 0, jMax = count - 2; j < jMax; j++ )
				{
					Vertex2 tri[4];
					tri[0] = vertices[j];
					tri[1] = vertices[j+1];
					tri[2] = vertices[j+2];
					tri[3] = vertices[j];

					glVertexPointer( 2, kDataType, 0, tri ); GPUError();
					glDrawArrays( modeGL, 0, sizeof(tri)/sizeof(tri[0]) ); GPUError();
				}
			}

			vertices += count;
		}

		if ( overridePaint )
		{
			Paint::White().End( * this );
			fCurrentPaint->Begin( * this );
		}
	}
#endif
}

/*
void
GPUStream::Submit( const Mode modes[], const VertexCache caches[], S32 len )
{
	for ( S32 i = 0; i < len; i++ )
	{
		const VertexCache& cache = caches[i];
		ArrayS32& counts = cache.Counts();
		Self::Submit( modes[i], cache.Vertices().ReadAccess(), counts.ReadAccess(), counts.Length() );
	}
}
*/

void
GPUStream::Submit( const Quad vertices )
{
	if ( Rtt_PAINT_TRIANGLES( fCurrentPaint ) )
	{
		const Vertex2 *textureCoords = Quad_Zero();
		Rtt_ASSERT( fCurrentPaint->IsType( Paint::kBitmap ) == fIsTexture );
		bool isBitmap = fIsTexture;

		// Begin non-zero
		const Quad* quad = SubmitBounds();
		bool isNonZero = quad && IsProperty( kNonZeroWinding );
		if ( isNonZero )
		{
			if ( isBitmap )
			{
				BeginNonZero( *quad );

				Rtt_glColor4( Rtt_REAL_0, Rtt_REAL_0, Rtt_REAL_0, Rtt_REAL_1 );
				glVertexPointer( 2, kDataType, 0, vertices ); GPUError();

				{
					BeginTexture( vertices );
					glDrawArrays( GL_TRIANGLE_STRIP, 0, sizeof(Quad) / sizeof(Vertex2) ); GPUError();
					EndTexture();
				}

				BitmapContext guard( fIsTexture, GetActiveTextureCount(), * quad, vertices );						
				EndNonZero( * quad, fCurrentPaint );
			}
			else
			{
				BeginNonZero( *quad );

				Rtt_glColor4( Rtt_REAL_0, Rtt_REAL_0, Rtt_REAL_0, Rtt_REAL_1 );
				glVertexPointer( 2, kDataType, 0, vertices ); GPUError();

				BeginTexture( vertices );
				glDrawArrays( GL_TRIANGLE_STRIP, 0, sizeof(Quad) / sizeof(Vertex2) ); GPUError();
				EndTexture();

				EndNonZero( * quad, fCurrentPaint );
			}
		}
		else
		{
			glVertexPointer( 2, kDataType, 0, vertices ); GPUError();

			BeginTexture( vertices );
			glDrawArrays( GL_TRIANGLE_STRIP, 0, sizeof(Quad) / sizeof(Vertex2) ); GPUError();
			EndTexture();
		}
	}
#ifdef Rtt_DEBUG
	else if ( fCurrentPaint )
	{
		S32 counts = sizeof(Quad) / sizeof(Vertex2);

		const Quad* oldSubmitBounds = SubmitBounds();
		if ( oldSubmitBounds ) { DidSubmitArray(); }

		Quad bounds; memcpy( bounds, vertices, sizeof(Quad) );
		WillSubmitArray( bounds );
		Submit( RenderTypes::kStrip, vertices, & counts, 1 );
		DidSubmitArray();

		if ( oldSubmitBounds ) { WillSubmitArray( * oldSubmitBounds); }
	}
#endif
}

void
GPUStream::Submit( const Vertex2& origin, Real halfW, Real halfH, Real radius )
{
	if ( fCurrentPaint )
	{
		VertexCache tmpCache( fAllocator );

		FillTesselatorStream tesselator( NULL, tmpCache );
		tesselator.Submit( origin, halfW, halfH, radius );

		SubmitCache( RenderTypes::kFan, tmpCache );
	}
}

void
GPUStream::Submit( const Vertex2& origin, Real radius )
{
	if ( fCurrentPaint )
	{
		VertexCache tmpCache( fAllocator );

		FillTesselatorStream tesselator( NULL, tmpCache );

		tesselator.Submit( origin, radius );
		SubmitCache( RenderTypes::kFan, tmpCache );
	}
}

void
GPUStream::Submit( const Vertex2& origin, Real xRadius, Real yRadius )
{
	if ( fCurrentPaint )
	{
/*
		bool isBitmap = fCurrentPaint->IsType( Paint::kBitmap );
		if ( isBitmap ) { WillSubmit( vertices ); }

		// Draw
		
		if ( isBitmap ) { DidSubmit(); }
*/
	}
}

void
GPUStream::SubmitCache( RenderTypes::Mode mode, const VertexCache& cache )
{
	Submit(
		mode,
		cache.Vertices().ReadAccess(),
		cache.Counts().ReadAccess(),
		cache.Counts().Length() );
}
#endif // 0

void
GPUStream::DrawQuad( const Quad vertices )
{
#if 0
	glVertexPointer( 2, kDataType, 0, vertices ); GPUError();
	glDrawArrays( GL_TRIANGLE_STRIP, 0, sizeof(Quad) / sizeof(Vertex2) ); GPUError();
#endif
}

/// Converts the given content coordinates to pixel coordinates relative to OpenGL's origin.
void
GPUStream::ContentToPixels( S32& x, S32& y, S32& w, S32& h ) const
{
	// First convert content coordinates to screen coordinates.
	ContentToScreen(x, y, w, h);
	
	// Rotate screen coordinates to match OpenGL's orientation.
	S32 screenWidth = ScreenWidth();
	S32 screenHeight = ScreenHeight();
	DeviceOrientation::Type orientation = GetRelativeOrientation();
	if (DeviceOrientation::IsSideways(orientation))
	{
		// Swap coordinates if hardware was rotated 90 or 270 degrees.
		Swap(x, y);
		Swap(w, h);
		Swap(screenWidth, screenHeight);
	}
	if (((fRotation - 90) % 180) == 0)
	{
		// Swap coordinates if simulator window was rotated 90 or 270 degrees.
		// This will undo the above swap on a simulator with fixed orientation and sideways hardware.
		Swap(x, y);
		Swap(w, h);
		Swap(screenWidth, screenHeight);
	}
	if ((DeviceOrientation::kUpsideDown == orientation) ||
	    (DeviceOrientation::kSidewaysRight == orientation))
	{
		x = (screenWidth - x) - w;
	}
	if ((DeviceOrientation::kUpsideDown == orientation) ||
	    (DeviceOrientation::kSidewaysLeft == orientation))
	{
		y = (screenHeight - y) - h;
	}
	
	// Next, convert screen coordinates to pixel coordinates.
	// Pixel coordinates will only differ when running on a simulator that is zoomed-in/out.
	Rtt_Real xScale = Rtt_RealDiv(Rtt_IntToReal(fWindowWidth), Rtt_IntToReal(screenWidth));
	Rtt_Real yScale = Rtt_RealDiv(Rtt_IntToReal(fWindowHeight), Rtt_IntToReal(screenHeight));
	x = Rtt_RealToInt(Rtt_RealMul(Rtt_IntToReal(x), xScale) + Rtt_REAL_HALF);
	y = Rtt_RealToInt(Rtt_RealMul(Rtt_IntToReal(y), yScale) + Rtt_REAL_HALF);
	w = Rtt_RealToInt(Rtt_RealMul(Rtt_IntToReal(w), xScale) + Rtt_REAL_HALF);
	h = Rtt_RealToInt(Rtt_RealMul(Rtt_IntToReal(h), yScale) + Rtt_REAL_HALF);

	// Flip the coordinates relative to OpenGL's origin, which is typically the bottom left corner.
	if (IsProperty(RenderingStream::kFlipVerticalAxis))
	{
		x = ((S32)fWindowWidth - x) - w;
	}
	if (IsProperty(RenderingStream::kFlipHorizontalAxis))
	{
		y = ((S32)fWindowHeight - y) - h;
	}
}

// ============================================================================

#ifdef OLD_GRAPHICS

GPUMultisampleStream::GPUMultisampleStream( Rtt_Allocator* p )
:	Super( p ),
	fPaint( NULL )
{
}

void
GPUMultisampleStream::BeginPaint( const Paint* paint )
{
	static const Paint* kDummy = & Paint::ZeroColor();

	// Only multisample pure colors that are not masked
	if ( paint && paint->IsType( Paint::kColor ) )
	{
		// Save off correct color into fPaint and use a dummy color to init Super's 
		// fCurrentPaint so we can still submit to it
		// If fPaint is set, then we are multisampling it
		fPaint = paint;
		paint = kDummy;
	}

	Super::BeginPaint( paint );
}

void
GPUMultisampleStream::EndPaint()
{
	fPaint = NULL;
	Super::EndPaint();
}

void
GPUMultisampleStream::Submit( RenderTypes::Mode mode, const Vertex2* vertices, const S32* counts, S32 numCounts )
{
	// Each offset is relative to the previous
	static const Vertex2 kOffsets[] =
	{
#if 0
		// Four corners of pixel moved in by 1/4
		{ -Rtt_REAL_FOURTH, -Rtt_REAL_FOURTH },
		{ Rtt_REAL_0, Rtt_REAL_HALF },
		{ Rtt_REAL_HALF, Rtt_REAL_0 },
		{ Rtt_REAL_0, -Rtt_REAL_HALF }
#elif 0
		// Rotated grid (points are along horizontal/vertical lines 1/4 from center)
		{ -Rtt_REAL_FOURTH, -Rtt_REAL_EIGHTH },
		{ Rtt_REAL_EIGHTH, (Rtt_REAL_EIGHTH + Rtt_REAL_EIGHTH + Rtt_REAL_EIGHTH) },
		{ (Rtt_REAL_EIGHTH + Rtt_REAL_EIGHTH + Rtt_REAL_EIGHTH), -Rtt_REAL_EIGHTH },
		{ -Rtt_REAL_EIGHTH, -(Rtt_REAL_EIGHTH + Rtt_REAL_EIGHTH + Rtt_REAL_EIGHTH) }
#elif 0
		// Four corners of pixel
		{ Rtt_REAL_0, Rtt_REAL_0 },
		{ Rtt_REAL_0, Rtt_REAL_0 },
		{ Rtt_REAL_0, Rtt_REAL_0 },
		{ Rtt_REAL_0, Rtt_REAL_0 }
#elif 0
		// Four corners of pixel
		{ Rtt_REAL_HALF, Rtt_REAL_HALF },
		{ Rtt_REAL_0, -Rtt_REAL_1 },
		{ -Rtt_REAL_1, Rtt_REAL_0 },
		{ Rtt_REAL_0, Rtt_REAL_1 }
#elif 1
		// Four corners of pixel
		{ -Rtt_REAL_HALF, -Rtt_REAL_HALF },
		{ Rtt_REAL_0, Rtt_REAL_1 },
		{ Rtt_REAL_1, Rtt_REAL_0 },
		{ Rtt_REAL_0, -Rtt_REAL_1 }
#else
		// Four corners of pixel moved in by 1/8
		{ -(Rtt_REAL_FOURTH + Rtt_REAL_EIGHTH), -(Rtt_REAL_FOURTH + Rtt_REAL_EIGHTH) },
		{ Rtt_REAL_0, (Rtt_REAL_HALF + Rtt_REAL_FOURTH) },
		{ (Rtt_REAL_HALF + Rtt_REAL_FOURTH), Rtt_REAL_0 },
		{ Rtt_REAL_0, -(Rtt_REAL_HALF + Rtt_REAL_FOURTH) }
#endif
	};

	if ( fPaint && Rtt_VERIFY( SubmitBounds() ) )
	{
		// Save previous winding value and set current winding to default to
		// ensure that all other GPUStream calls are in the default winding mode
		const bool oldWindingSetting = IsProperty( kNonZeroWinding );
		SetProperty( kNonZeroWinding, false );

		const Quad& kQuad = * SubmitBounds();

		// Calculate coverage mask (alpha)
		// NOTE: This is equivalent to non-zero winding

#if 0
		// (1) Zero out alpha channel
		glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE );
		glBlendFunc( GL_ZERO, GL_ZERO );
		Self::DrawQuad( kQuad );

		// (2) Accumulate jittered samples in alpha (coverage)
		glBlendFunc( GL_ONE, GL_ONE );
#else
		BeginNonZero( kQuad );
#endif
		glPushMatrix();
		{
//			Rtt_glTranslate( Rtt_REAL_HALF, Rtt_REAL_HALF, Rtt_REAL_0 );
//			Rtt_glColor4( Rtt_REAL_0, Rtt_REAL_0, Rtt_REAL_0, Rtt_REAL_HALF );
			Rtt_glColor4( Rtt_REAL_0, Rtt_REAL_0, Rtt_REAL_0, Rtt_REAL_HALF - Rtt_REAL_EIGHTH );
			Super::Submit( mode, vertices, counts, numCounts );

//			Rtt_glColor4( Rtt_REAL_0, Rtt_REAL_0, Rtt_REAL_0, Rtt_REAL_EIGHTH );
			Rtt_glColor4( Rtt_REAL_0, Rtt_REAL_0, Rtt_REAL_0, Rtt_REAL_EIGHTH + Rtt_REAL_32ND );
			const int kNumSamples = sizeof( kOffsets ) / sizeof( Vertex2 );
			for ( int i = 0; i < kNumSamples; i++ )
			{
				const Vertex2& offset = kOffsets[i];
				Rtt_glTranslate( offset.x, offset.y, Rtt_REAL_0 );
				Super::Submit( mode, vertices, counts, numCounts );
			}
		}
		glPopMatrix();

#if 0
		// (3) Apply alpha of fPaint to coverage mask
		fPaint->Begin( * this ); // Use correct color
		glBlendFunc( GL_DST_COLOR, GL_ZERO );
		Self::DrawQuad( kQuad );

		// (4) Set mask so that RGB src colors have an effect
		glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

		// (5) Apply coverage mask (alpha) to color buffer
		glBlendFunc( GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA );
		Self::DrawQuad( kQuad );
		fPaint->End( * this );

		// (6) Restore blend mode
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
#else
		// TODO: We need to figure out what to do here --- call glDrawArrays again?
		// That seems expensive.  So!  For now, we just don't support it!
		// Except oops on multisample rotated bitmaps, this will get called...
		// I think?  the only reason tho is so that rotated bitmaps get
		// an AA-edge --- maybe we should just not support that either
		// until we find a cheaper solution to the whole thing!!!
		Rtt_ASSERT( ! fCurrentPaint->IsType( Paint::kBitmap ) );

		EndNonZero( kQuad, fPaint );
#endif

		// Restore winding setting
		SetProperty( kNonZeroWinding, oldWindingSetting );
		Rtt_ASSERT( oldWindingSetting == IsProperty( kNonZeroWinding ) );
	}
	else
	{
		Super::Submit( mode, vertices, counts, numCounts );
	}
}

void
GPUMultisampleStream::Submit( const Quad vertices )
{
	if ( fPaint )
	{
		if ( Quad_IsAligned( vertices ) )
		{
			// Temporarily set fCurrentPaint to fPaint b/c we're calling
			// Super::Submit and we might be in non-zero mode. Otherwise,
			// we'll end up drawing this rect with Paint::ZeroColor().
			// The previous value of fCurrentPaint is restored below. 
			const Paint* currentPaint = fCurrentPaint;
			fCurrentPaint = fPaint;

			fPaint->Begin( * this );
			Super::Submit( vertices );
			fPaint->End( * this );

			fCurrentPaint = currentPaint;
		}
		else
		{
#if 1
			Quad bounds;
			bool shouldSubmitBounds = ! SubmitBounds();
			if ( shouldSubmitBounds )
			{
				Quad_Enlarge( bounds, vertices );//Quad_GetBoundsAxisAligned( bounds, vertices );
				Self::WillSubmitArray( bounds );
			}

			const S32 counts[] = { sizeof( Quad ) / sizeof( vertices[0] ) };
			const S32 numCounts = { sizeof( counts ) / sizeof( counts[0] ) };
			Submit( RenderTypes::kStrip, vertices, counts, numCounts );

			if ( shouldSubmitBounds )
			{
				Self::DidSubmitArray();
			}
#else
			Quad bounds;
			Quad_GetBoundsAxisAligned( bounds, vertices );

			Vertex2 tri1[] = { vertices[0], vertices[1], vertices[2] };
			const S32 counts[] = { sizeof( tri1 ) / sizeof( vertices[0] ) };
			const S32 numCounts = { sizeof( counts ) / sizeof( counts[0] ) };

			Self::WillSubmitArray( bounds );
			Submit( RenderTypes::kFan, tri1, counts, numCounts );
			Self::DidSubmitArray();

			Vertex2 tri2[] = { vertices[0], vertices[2], vertices[3] };
			Rtt_STATIC_ASSERT( sizeof( tri1 ) == sizeof( tri2 ) );

			Self::WillSubmitArray( bounds );
			Submit( RenderTypes::kFan, tri2, counts, numCounts );
			Self::DidSubmitArray();
#endif
		}
	}
	else
	{
		Super::Submit( vertices );
	}
}

void
GPUMultisampleStream::Submit( const Vertex2& origin, Real halfW, Real halfH, Real radius )
{
	if ( fPaint )
	{
		Quad quad;
		bool shouldSubmitBounds = ! SubmitBounds();
		if ( shouldSubmitBounds )
		{
			const Real x = origin.x - halfW - Rtt_REAL_1;
			const Real y = origin.y - halfH - Rtt_REAL_1;
			const Real w = Rtt_RealMul2( halfW + Rtt_REAL_1 );
			const Real h = Rtt_RealMul2( halfH + Rtt_REAL_1 );
			quad[0].x = x;
			quad[0].y = y;
			quad[1].x = x;
			quad[1].y = y + h;
			quad[3].x = x + w;
			quad[3].y = y + h;
			quad[2].x = x + w;
			quad[2].y = y;

			Rtt_ASSERT( Quad_IsAligned( quad ) );

			Self::WillSubmitArray( quad );
		}

		Super::Submit( origin, halfW, halfH, radius );

		if ( shouldSubmitBounds )
		{
			Self::DidSubmitArray();
		}
	}
	else
	{
		Super::Submit( origin, halfW, halfH, radius );
	}
}

void
GPUMultisampleStream::Submit( const Vertex2& origin, Real radius )
{
	if ( fPaint )
	{
		Quad quad;
		bool shouldSubmitBounds = ! SubmitBounds();
		if ( shouldSubmitBounds )
		{
			const Real x = origin.x - radius - Rtt_REAL_1;
			const Real y = origin.y - radius - Rtt_REAL_1;
			const Real length = Rtt_RealMul2( radius + Rtt_REAL_1 );
			quad[0].x = x;
			quad[0].y = y;
			quad[1].x = x;
			quad[1].y = y + length;
			quad[3].x = x + length;
			quad[3].y = y + length;
			quad[2].x = x + length;
			quad[2].y = y;

			Rtt_ASSERT( Quad_IsAligned( quad ) );

			Self::WillSubmitArray( quad );
		}

		Super::Submit( origin, radius );

		if ( shouldSubmitBounds )
		{
			Self::DidSubmitArray();
		}
	}
	else
	{
		Super::Submit( origin, radius );
	}
}

void
GPUMultisampleStream::Submit( const Vertex2& origin, Real a, Real b )
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
}

#endif

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

