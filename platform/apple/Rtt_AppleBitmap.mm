//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_AppleBitmap.h"

#include "Rtt_AppleTextAlignment.h"

#import <CoreFoundation/CFURL.h>

#ifdef Rtt_MAC_ENV
	#import <Cocoa/Cocoa.h>
	#import <ApplicationServices/ApplicationServices.h>

	#if (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5)
		#if __LP64__
			typedef double CGFloat;// 64-bit
		#else
			typedef float CGFloat;// 32-bit
		#endif
	#endif
	// Apple is retarded about forcing you to include ridiculously large header
	// files when these are the only ones I need:
	// 
	// #include <CoreGraphics/CoreGraphics.h>
	// #include <ImageIO/ImageIO.h>
#else
	#include <CoreGraphics/CoreGraphics.h>
	#include <ImageIO/ImageIO.h>
	#import <UIKit/UIKit.h>
#endif

#include <string.h>


#ifdef Rtt_TEST_BITMAP
	#include "Rtt_Matrix.h"
	#include "Display/Rtt_Paint.h"
	#include "Display/Rtt_ShapeObject.h"
	#include "Display/Rtt_PlatformBitmap.h"
	#include "Rtt_Runtime.h"
	#include "Rtt_MPlatform.h"
#endif

#include "Display/Rtt_Display.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static size_t
GetMaxTextureSize()
{
	return Display::GetMaxTextureSize();
}


#ifdef Rtt_TEST_BITMAP

void
AppleBitmap::Test( class Runtime& runtime )
{
/*
	Rtt_Allocator* pAllocator = runtime.Allocator();	
	StageObject& stage = runtime.GetScene().GetStage();

	Paint* paint = Paint::NewBitmap( runtime, "fish.jpeg" );
	Real w = Rtt_IntToReal( paint->GetBitmap().Width() );
	Real h = Rtt_IntToReal( paint->GetBitmap().Height() );
	ShapeObject* v = ShapeObject::NewRect( pAllocator, w, h );
	v->SetShapeColor( paint );
	v->Translate( w, Rtt_RealMul2( h ) + Rtt_RealDiv2( Rtt_RealDiv2( h ) ) );
	stage->Insert( 0, v );

	paint = Paint::NewBitmap( runtime, "aquariumbackground.jpeg" );
	w = Rtt_IntToReal( paint->GetBitmap().Width() );
	h = Rtt_IntToReal( paint->GetBitmap().Height() );
	v = ShapeObject::NewRect( pAllocator, w, h );
	v->SetShapeColor( paint );
	v->Translate( Rtt_RealDiv2( w ), Rtt_RealDiv2( h ) );
	stage->Insert( 0, v );
*/
}

#endif // Rtt_TEST_BITMAP

// ----------------------------------------------------------------------------

AppleBitmap::AppleBitmap()
:	fData( NULL )
{
}

AppleBitmap::~AppleBitmap()
{
	Self::FreeBits();
}

void
AppleBitmap::FreeBits() const
{
	if ( fData )
	{
		Rtt_FREE( fData );
		fData = NULL;
	}
}

// ----------------------------------------------------------------------------
    
    /* The intended display orientation of the image. If present, the value
     * of this key is a CFNumberRef with the same value as defined by the
     * TIFF and Exif specifications.  That is:
     *   1  =  0th row is at the top, and 0th column is on the left.
     *   2  =  0th row is at the top, and 0th column is on the right.
     *   3  =  0th row is at the bottom, and 0th column is on the right.
     *   4  =  0th row is at the bottom, and 0th column is on the left.
     *   5  =  0th row is on the left, and 0th column is the top.
     *   6  =  0th row is on the right, and 0th column is the top.
     *   7  =  0th row is on the right, and 0th column is the bottom.
     *   8  =  0th row is on the left, and 0th column is the bottom.
     * If not present, a value of 1 is assumed. */
    
    //Effectively converts the "top" edge of the image
    //to the PlatformBitmap orientation based on the EXIF orientation
    //flag, cases 2,4,5,7 are special mirrored orientations
    static S8
    ConvertCGOrientation( int cgImageOrienation )
    {
        S8 result = PlatformBitmap::kUp;
        
        switch( cgImageOrienation )
        {
            case 0:
            case 1:
                result = PlatformBitmap::kUp;
                break;
            case 2:
                result = PlatformBitmap::kUp;
                break;
            case 3:
                result = PlatformBitmap::kDown;
                break;
            case 4:
                result = PlatformBitmap::kDown;
                break;
            case 5:
                result = PlatformBitmap::kRight;
                break;
            case 6:
                result = PlatformBitmap::kRight;
                break;
            case 7:
                result = PlatformBitmap::kLeft;
                break;
            case 8:
                result = PlatformBitmap::kLeft;
                break;
            default:
                break;
        }
        
        return result;
    }

static U8
GetInitialPropertiesValue()
{
	// CGBitmapContext's have to use a colorspace with premultiplied alpha for some unknown reason...
	// http://developer.apple.com/mac/library/qa/qa2001/qa1037.html
	return PlatformBitmap::kIsPremultiplied;
}

AppleFileBitmap::AppleFileBitmap( const char* inPath, bool isMask )
:	Super(),
	fImage( NULL ),
	fOrientation( kUp ),
	fProperties( GetInitialPropertiesValue() ),
	fIsMask( isMask )
{
	size_t inPathLength = strlen( inPath );

	CFURLRef imageUrl = CFURLCreateFromFileSystemRepresentation( kCFAllocatorDefault, (UInt8*)inPath, inPathLength, false );
	CGImageSourceRef imageSource = NULL;

	if ( Rtt_VERIFY( imageUrl ) )
	{
		// CFShow( imageUrl );
		
		imageSource = CGImageSourceCreateWithURL( imageUrl, NULL );

		if ( Rtt_VERIFY( imageSource ) )
		{
			// CFShow( imageSource );
            
            CGImageSourceStatus status = CGImageSourceGetStatus(imageSource);
            if (status == kCGImageStatusComplete)
            {
                CFDictionaryRef imageSourceProperties = CGImageSourceCopyPropertiesAtIndex(imageSource,0,NULL);
                if (imageSourceProperties)
                {
                    id orientation = (id)CFDictionaryGetValue(imageSourceProperties, kCGImagePropertyOrientation);
                    if (orientation)
                    {
                        int cgOrientation = [(NSNumber*)orientation intValue];
                        fOrientation = ConvertCGOrientation(cgOrientation);
                    }
                    CFRelease( imageSourceProperties );
                }
            }
            
			fImage = CGImageSourceCreateImageAtIndex( imageSource, 0, NULL );

			// if ( fImage ) { CFShow( fImage ); }
			CFRelease( imageSource );
		}

		CFRelease( imageUrl );
	}

	if (fImage)
	{
		Initialize();
	}
}

#if defined( Rtt_IPHONE_ENV ) || defined( Rtt_TVOS_ENV )

static S8
ConvertOrientation( UIImageOrientation orientation )
{
	S8 result = PlatformBitmap::kUp;

	switch( orientation )
	{
		case UIImageOrientationDown:
		case UIImageOrientationDownMirrored:
			result = PlatformBitmap::kDown;
			break;
		case UIImageOrientationLeft:
		case UIImageOrientationLeftMirrored:
			result = PlatformBitmap::kLeft;
			break;
		case UIImageOrientationRight:
		case UIImageOrientationRightMirrored:
			result = PlatformBitmap::kRight;
			break;
		//case UIImageOrientationUp:
		//case UIImageOrientationUpMirrored:
		default:
			break;
	}

	return result;
}

AppleFileBitmap::AppleFileBitmap( UIImage* image, bool isMask )
:	Super(),
	fImage( (CGImageRef)CFRetain( image.CGImage ) ),
	fOrientation( ConvertOrientation( image.imageOrientation ) ),
	fProperties( GetInitialPropertiesValue() ),
	fIsMask( isMask )
{
	Initialize();
}
#endif

#ifdef Rtt_MAC_ENV
AppleFileBitmap::AppleFileBitmap( NSImage* image, bool isMask )
:	Super(),
	fImage( NULL ),
	fOrientation( PlatformBitmap::kUp ), // not sure how to get orientation from NSImage
	fProperties( GetInitialPropertiesValue() ),
	fIsMask( isMask )
{
	fImage = (CGImageRef)CFRetain( [image CGImageForProposedRect:NULL context:NULL hints:nil] );
	Initialize();
}
#endif
	
AppleFileBitmap::~AppleFileBitmap()
{
	if ( fImage )
	{
		CFRelease( fImage );
	}
}

void
AppleFileBitmap::Initialize()
{
	fScale = CalculateScale();
}

#if !defined( Rtt_WEB_PLUGIN )
	// Implemented on a per-platform basis
	Rtt_EXPORT CGSize Rtt_GetScreenSize();
#endif

float
AppleFileBitmap::CalculateScale() const
{
	float result = -1.0; // Default value to flag that no scale was applied

	if ( ! Rtt_VERIFY( fImage ) )
	{
		return result;
	}
	
	size_t w = CGImageGetWidth( fImage );
	size_t h = CGImageGetHeight( fImage );

#if !defined( Rtt_WEB_PLUGIN )
	// TODO: Rtt_GetScreenSize() returns (0,0) if no simulator instance is active.
	// However, we have CoronaViews that are created independent of simulator,
	// so for now, we just do a cheap check against 0 to ignore. Later, we should
	// determine a way to decouple.
	CGSize screenSize = Rtt_GetScreenSize();
	bool isScreenSizeValid = ( screenSize.width > 0 && screenSize.height > 0 ); 

	size_t wMax = isScreenSizeValid ? NextPowerOf2( screenSize.width ) : w;
	size_t hMax = isScreenSizeValid ? NextPowerOf2( screenSize.height ) : h;
#else
	// TODO: Figure out how to get "screen" bounds, i.e. content bounds in html page
	size_t wMax = w;
	size_t hMax = h;
#endif
	
	// Downscale the image if it exceeds OpenGL's maximum texture size.
	size_t maxBitmapSize = (size_t)GetMaxTextureSize();

	if ( IsProperty( kIsBitsFullResolution ) )
	{
		// We still have to scale if the bitmap is larger than the largest texture size
		if ( w <= maxBitmapSize && h <= maxBitmapSize )
		{
			return result;
		}
		else
		{
			Rtt_LogException( "WARNING: Image size (%ld,%ld) exceeds max texture dimension (%ld). Image will be resized to fit.\n", w, h, maxBitmapSize );
			wMax = maxBitmapSize;
			hMax = maxBitmapSize;
		}
	}
	else
	{
		wMax = Min( wMax, maxBitmapSize );
		hMax = Min( hMax, maxBitmapSize );
	}

	// Align longest image edge to the longest screen edge to calculate proper scale.
	// If image is landscape and screen size is portrait (or vice versa), 
	// then swap screen dimensions. 
	bool isImageLandscape = w > h;
	bool isScreenLandscape = wMax > hMax;
	if ( isImageLandscape != isScreenLandscape )
	{
		size_t tmp = wMax;
		wMax = hMax; hMax = tmp;
	}

	if ( w > wMax || h > hMax )
	{
		float scaleW = ((float)wMax)/(float)w;
		float scaleH = ((float)hMax)/(float)h;
		result = ( scaleH < scaleW ? scaleH : scaleW );
	}

	// Return the down-scaling factor to be applied to the image.
	// Returns -1 if no scaling should be applied.
	return result;
}

static Rtt_INLINE
double DegreesToRadians( double degrees )
{
	return degrees * M_PI/180;
}

// Transform CGContext to account for different origin depending on orientation
// Also return CGRect, appropriately rotated
CGRect
AppleFileBitmap::TransformCGContext( CGContextRef context, size_t width, size_t height ) const
{
	CGRect rect = { { 0.f, 0.f }, { (CGFloat)width, (CGFloat)height } };

	if ( IsProperty( kIsBitsAutoRotated ) )
	{
		Orientation orientation = GetOrientation();
		if ( kUp != orientation )
		{
			S32 angle = DegreesToUpright();

			bool flipDimensions = true;

			switch ( orientation )
			{
				case kRight:
					CGContextTranslateCTM( context, 0, height );
					CGContextRotateCTM( context, DegreesToRadians( -angle ) );
					break;
				case kLeft:
					CGContextTranslateCTM( context, width, 0 );
					CGContextRotateCTM( context, DegreesToRadians( -angle ) );
					break;
				case kDown:
					CGContextTranslateCTM( context, width, height );
					CGContextRotateCTM( context, DegreesToRadians( angle ) );
					flipDimensions = false;
					break;
				default:
					Rtt_ASSERT_NOT_REACHED();
					break;
			}

			if ( flipDimensions )
			{
				// height and width correspond to the upright dimensions,
				// but we need to draw the bounds of the source (i.e.
				// the pre-auto-rotated) dimensions
				rect.size.width = height;
				rect.size.height = width;
			}
		}
	}

	return rect;
}

void*
AppleFileBitmap::GetBitsGrayscale( Rtt_Allocator* context ) const
{
	Rtt_ASSERT( ! fData );

	void *result = NULL;

	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceGray();

	if ( colorSpace )
	{
		Rtt_ASSERT( 1 == CGColorSpaceGetNumberOfComponents( colorSpace ) );

		const size_t width = Width();
		const size_t height = Height();

		Rtt_ASSERT( CGImageGetBitsPerPixel( fImage ) >= CGImageGetBitsPerComponent( fImage ) );
//		const size_t bytesPerPixel = CGImageGetBitsPerPixel( fImage ) / CGImageGetBitsPerComponent( fImage );
		const size_t bytesPerRow = width; // * bytesPerPixel;
		const CGBitmapInfo bitmapInfo = kCGImageAlphaNone;
		void* data = Rtt_CALLOC( context, bytesPerRow, height );
		
		const size_t bitsPerChannel = 8;
		CGContextRef bmpContext = CGBitmapContextCreate(
										data,
										width, height, bitsPerChannel,
										bytesPerRow, colorSpace,
										bitmapInfo );

		if ( Rtt_VERIFY( bmpContext ) )
		{
			CGRect rect = TransformCGContext( bmpContext, width, height );

			CGContextDrawImage( bmpContext, rect, fImage );
			CGContextRelease( bmpContext );
			result = data;
		}
		else
		{
			Rtt_FREE( data );
		}

		CGColorSpaceRelease( colorSpace );
	}

	return result;
}

void*
AppleFileBitmap::GetBitsColor( Rtt_Allocator* context ) const
{
	Rtt_ASSERT( ! fData );
	
	void *result = NULL;

	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

	if ( colorSpace )
	{
		const size_t width = Width();
		const size_t height = Height();

		const size_t bytesPerRow = width << 2;
		const CGBitmapInfo bitmapInfo =
				#ifdef Rtt_MAC_ENV
					kCGImageAlphaPremultipliedFirst;
				#else
					kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big;
				#endif
		void* data = Rtt_CALLOC( context, bytesPerRow, height );
		
		const size_t bitsPerChannel = 8;
		CGContextRef bmpContext = CGBitmapContextCreate(
										data,
										width, height, bitsPerChannel,
										bytesPerRow, colorSpace,
										bitmapInfo );
		if ( Rtt_VERIFY( bmpContext ) )
		{
			CGRect rect = TransformCGContext( bmpContext, width, height );

			CGContextDrawImage( bmpContext, rect, fImage );
			CGContextRelease( bmpContext );
			result = data;

// Debug pixel data
#if 0
PrintChannel( (const U8 *)result, 0, 4 );
PrintChannel( (const U8 *)result, 1, 4 );
PrintChannel( (const U8 *)result, 2, 4 );
PrintChannel( (const U8 *)result, 3, 4 );
#endif

#if 0
CGDataProviderRef provider = CGImageGetDataProvider( fImage );
if ( provider )
{
	CFDataRef imageData = CGDataProviderCopyData( provider );

	const void* bytes = CFDataGetBytePtr( imageData );
	Rtt_ASSERT( 0 == memcmp( bytes, data, bytesPerRow*height ) );

	CFRelease( imageData );
	CGDataProviderRelease( provider );
}
#endif
		}
		else
		{
			Rtt_FREE( data );
		}
		
		CGColorSpaceRelease( colorSpace );
	}
	
	return result;
}

const void*
AppleFileBitmap::GetBits( Rtt_Allocator* context ) const
{
	if ( ! fData )
	{
		// We don't support these colorspaces because we cannot create
		// a bitmap context for them
		Rtt_ASSERT( CGImageGetColorSpace( fImage ) );
		Rtt_ASSERT( kCGColorSpaceModelPattern != CGColorSpaceGetModel( CGImageGetColorSpace( fImage ) ) );

		#if 0
		{
			CGColorSpace colorSpace = CGImageGetColorSpace( fImage );
			size_t numChannels = CGColorSpaceGetNumberOfComponents( colorSpace );
			CGColorSpaceModel model = CGColorSpaceGetModel( colorSpace );
			Rtt_TRACE_SIM( ( "AppleFileBitmap::GetBits() numChannels(%d) model(%d)\n", numChannels, model ) );
		}
		#endif

		fData = ( fIsMask ? GetBitsGrayscale( context ) : GetBitsColor( context ) );
	}

	return fData;
}

#ifdef Rtt_DEBUG
void
AppleFileBitmap::PrintChannel( const U8 *bytes, int channel, U32 bytesPerPixel ) const
{
	const U32 width = Width();
	const U32 height = Height();
	const U32 bytesPerRow = width * 4;

	printf( "---------------------------------------------------\n" );
	printf( "[Channel = %d]\n", channel );

	for ( U32 j = 0; j < height; j++ )
	{
		const U8 *rowBytes = bytes + j*bytesPerRow + channel;
		for ( U32 i = 0; i < width; i++ )
		{
			printf( "%02x", *rowBytes );

			rowBytes += bytesPerPixel;
		}
		printf( "\n" );
	}

}
#endif

U32
AppleFileBitmap::SourceWidth() const
{
	U32 len = (U32) CGImageGetWidth( fImage );
	return WasScaled() ? (fScale * len) : len;
}

U32
AppleFileBitmap::SourceHeight() const
{
	U32 len = (U32) CGImageGetHeight( fImage );
	return WasScaled() ? (fScale * len) : len;
}

U32
AppleFileBitmap::Width() const
{
	return ( ! IsPropertyInternal( kIsBitsAutoRotated ) ? SourceWidth() : UprightWidth() );
}

U32
AppleFileBitmap::Height() const
{
	return ( ! IsPropertyInternal( kIsBitsAutoRotated ) ? SourceHeight() : UprightHeight() );
}

U32
AppleFileBitmap::UprightWidth() const
{
	S32 angle = DegreesToUpright();
	return ( 0 == angle || 180 == Abs( angle ) ) ? SourceWidth() : SourceHeight();
}

U32
AppleFileBitmap::UprightHeight() const
{
	S32 angle = DegreesToUpright();
	return ( 0 == angle || 180 == Abs( angle ) ) ? SourceHeight() : SourceWidth();
}

bool
AppleFileBitmap::WasScaled() const
{
	return (fScale > 0.);
}

Real
AppleFileBitmap::GetScale() const
{
	return WasScaled() ? fScale : Rtt_REAL_1;
}

bool
AppleFileBitmap::IsProperty( PropertyMask mask ) const
{
	return IsPropertyInternal( mask );
}

void
AppleFileBitmap::SetProperty( PropertyMask mask, bool newValue )
{
	if ( ! Super::IsPropertyReadOnly( mask ) )
	{
		const U8 p = fProperties;
		const U8 propertyMask = (U8)mask;
		fProperties = ( newValue ? p | propertyMask : p & ~propertyMask );
	}

	switch ( mask )
	{
		case kIsBitsFullResolution:
			fScale = CalculateScale();
			break;
		case kIsBitsAutoRotated:
			break;
		default:
			break;
	}
}

PlatformBitmap::Format
AppleFileBitmap::GetFormat() const
{
	if ( fIsMask )
	{
		return PlatformBitmap::kMask;
	}

#ifdef Rtt_MAC_ENV
	return PlatformBitmap::kBGRA;
#else
	return PlatformBitmap::kABGR;
#endif
}

PlatformBitmap::Orientation
AppleFileBitmap::GetOrientation() const
{
	return (PlatformBitmap::Orientation)fOrientation;
}

// ----------------------------------------------------------------------------

#if defined( Rtt_IPHONE_ENV ) || defined( Rtt_TVOS_ENV )
IPhoneFileBitmap::IPhoneFileBitmap( UIImage *image, bool isMask )
:	Super( image, isMask ),
	fUIImage( [image retain] )
{
}

IPhoneFileBitmap::~IPhoneFileBitmap()
{
	// As a precaution, we autorelease to ensure that the CGImage in Super 
	// is released prior to UIImage releasing ownership of the CGImage.
	[fUIImage autorelease];
}
#endif

#ifdef Rtt_MAC_ENV
MacFileBitmap::MacFileBitmap( NSImage* image, bool isMask )
:	Super(image, isMask)
{
}	
#endif
	
// ----------------------------------------------------------------------------

#ifdef Rtt_MAC_ENV
static NSDictionary*
AppleTextBitmapCreateAttributes( NSFont* font, NSTextAlignment alignment)
{
	// On iOS, the line gap (leading) between lines is included. To match that behavior
	// and minimize line height differences, we include the line gap on Mac.
	NSMutableParagraphStyle *style = [[[NSMutableParagraphStyle alloc] init] autorelease];
	[style setParagraphStyle:[NSParagraphStyle defaultParagraphStyle]];
	[style setLineSpacing:[font leading]];

	// this makes third-party fonts (i.e. non-Apple fonts) render more closely to the
	// way they do on iOS (presumably there's some difference of opinion on the minutiae
	// of the interpretation of font metrics between the two platforms)
	[style setMaximumLineHeight:[font ascender] + Abs([font descender])];

	[style setAlignment:alignment];

	return [[NSDictionary alloc] initWithObjectsAndKeys:
		font, NSFontAttributeName,
		style, NSParagraphStyleAttributeName,
		nil];
}
#elif defined( Rtt_TVOS_ENV )
static NSDictionary*
AppleTextBitmapCreateAttributes( UIFont *font )
{
	return @{
		NSFontAttributeName:font,
	};
}

static NSDictionary*
AppleTextBitmapCreateAttributes( UIFont *font, NSTextAlignment alignment )
{
	NSMutableDictionary *result = [NSMutableDictionary dictionaryWithDictionary:AppleTextBitmapCreateAttributes( font )];

	NSMutableParagraphStyle *paragraph = [[NSMutableParagraphStyle alloc] init];
	paragraph.lineBreakMode = NSLineBreakByWordWrapping;
	paragraph.alignment = alignment;
	{
		[result setValue:paragraph forKey:NSParagraphStyleAttributeName];
	}
	[paragraph release];

	return result;
}
#endif

AppleTextBitmap::AppleTextBitmap( const char str[], const PlatformFont& font, Real w, Real h, const char alignment[], Real& baselineOffset )
:	Super(),
	fDimensions(),
	fIsMultiline( w > Rtt_REAL_0 ),
	fTextAlignment(AppleAlignment::AlignmentForString(alignment))
{
	fText = [[NSString alloc] initWithUTF8String:str];

	// If there is a newline in the text, switch to multi-line
	// Note [NSString containsString:] is iOS 8.0 and above
	if( !fIsMultiline && [fText rangeOfString:@"\n"].location != NSNotFound )
	{
		h = 0;
		fIsMultiline = true;
	}

	NSString *text = fText;
	AppleFont *f = (AppleFont*)font.NativeObject();

	fFont = [f retain];

	if ( ! fIsMultiline )
	{
		#ifdef Rtt_MAC_ENV
			NSDictionary *attributes = AppleTextBitmapCreateAttributes( f, fTextAlignment );

			fDimensions = [text sizeWithAttributes:attributes];
			fDimensions.width = ceil( fDimensions.width );

			// In the single line, the calculated height can sometimes be too small
			// compared to iOS, as it doesn't include the leading. We use an alternative
			// calculation and choose the *larger* as lineHeight can be too small 
			// for smaller font sizes.
			NSLayoutManager *mgr = [[NSLayoutManager alloc] init];
			CGFloat lineHeight = [mgr defaultLineHeightForFont:f];
			[mgr autorelease];
			fDimensions.height = ceil( Max( lineHeight, fDimensions.height ) );

			[attributes release];
		#elif defined( Rtt_IPHONE_ENV )
			fDimensions = [text sizeWithFont:f];
		#elif defined( Rtt_TVOS_ENV )
			NSDictionary *attributes = AppleTextBitmapCreateAttributes(fFont);
			fDimensions = [text sizeWithAttributes:attributes];
		#else
			Rtt_ASSERT_NOT_IMPLEMENTED();
		#endif
	}
	else
	{
		fDimensions.width = Rtt_RealToFloat( w );
		fDimensions.height = Rtt_RealToFloat( h );

		bool isFixedHeight = ( fDimensions.height > 0 );

		if ( ! isFixedHeight )
		{
			size_t hMax = GetMaxTextureSize();
			AppleSize size = fDimensions;
			size.height = hMax;

			if ( w <= Rtt_REAL_0 )
			{
				size.width = hMax;
			}

			#ifdef Rtt_MAC_ENV
				NSDictionary *attributes = AppleTextBitmapCreateAttributes( f, fTextAlignment);

				// NSStringDrawingUsesLineFragmentOrigin needed for multiline
				NSRect rect = [text boundingRectWithSize:size options:NSStringDrawingUsesLineFragmentOrigin attributes:attributes];
				fDimensions = rect.size;

				[attributes release];

			#elif defined( Rtt_IPHONE_ENV )
				fDimensions = [text sizeWithFont:f constrainedToSize:size];
			#elif defined( Rtt_TVOS_ENV )
				NSDictionary *attributes = AppleTextBitmapCreateAttributes( f, fTextAlignment); // Unlike Mac one, returns autoreleased object
				fDimensions = [text boundingRectWithSize:size options:NSStringDrawingUsesLineFragmentOrigin attributes:attributes context:nil].size;
			#else
				Rtt_ASSERT_NOT_IMPLEMENTED();
			#endif

			fDimensions.width = ceil(fDimensions.width);
			fDimensions.height = ceil(fDimensions.height);
		}
	}

	// If the requested width is non-zero, then that's the width
	if ( w > 0 )
	{
		fDimensions.width = Rtt_RealToFloat( w );
	}
	
	U32 width = fDimensions.width;
	if ( width & 0x3 )
	{
		// this is a mask, that is why we need it to be %4
		width = (width + 3) & -4;
		fDimensions.width = width;
	}

	U32 height = fDimensions.height;
	if ( height & 0x3 )
	{
		// this is a mask, that is why we need it to be %4
		height = (height + 3) & -4;
		fDimensions.height = height;
	}

	if ( fDimensions.width > GetMaxTextureSize() )
	{
		Rtt_LogException( "WARNING: The text object containing the following text will not render correctly because the width (%g) exceeds max texture dimensions (%ld): %s\n", fDimensions.width, GetMaxTextureSize(), [fText UTF8String] );

		fDimensions.width = GetMaxTextureSize();
	}

	if ( fDimensions.height > GetMaxTextureSize() )
	{
		Rtt_LogException( "WARNING: The text object containing the following text will not render correctly because the height (%g) exceeds max texture dimensions (%ld): %s\n", fDimensions.height, GetMaxTextureSize(), [fText UTF8String] );

		fDimensions.height = GetMaxTextureSize();
	}

    baselineOffset = fDimensions.height*0.5f - fFont.ascender;
}

AppleTextBitmap::~AppleTextBitmap()
{
	[fFont release];
	[fText release];
}

const void*
AppleTextBitmap::GetBits( Rtt_Allocator* context ) const
{
	if ( ! fData )
	{
		const size_t width = fDimensions.width;
		const size_t height = fDimensions.height;

		const size_t bytesPerRow = width;

#ifdef Rtt_MAC_ENV
		// [NSImage lockFocus] throws an exception when width and height are 0
		if ( 0 == width || 0 == height )
		{
			return NULL;
		}

		NSRect drawRect = NSMakeRect(0, 0, width, height);
		NSImage* image = [[NSImage alloc] initWithSize:drawRect.size];
		NSBitmapImageRep *bitmap = nil;
		NSDictionary* attributes = AppleTextBitmapCreateAttributes( fFont, fTextAlignment );

		[image lockFocus];
			[[NSGraphicsContext currentContext] setShouldAntialias:YES];
			[[NSColor redColor] set];

			
			if ( fIsMultiline )
			{
				[fText drawInRect:drawRect withAttributes:attributes];
			}
			else
			{
				[fText drawAtPoint:NSZeroPoint withAttributes:attributes];
			}
		[image unlockFocus];

		// Old way: fails on retina Macs because the resultant bitmap's pixel dimensions are twice the actual width/height
		// bitmap = [[NSBitmapImageRep alloc] initWithFocusedViewRect:drawRect];

		// Draw the image into a NSBitmapImageRep in such a way that the number of
		// pixels is not doubled on retina Macs
		bitmap = [[NSBitmapImageRep alloc]
				  initWithBitmapDataPlanes:NULL
				  pixelsWide:width
				  pixelsHigh:height
				  bitsPerSample:8
				  samplesPerPixel:4
				  hasAlpha:YES
				  isPlanar:NO
				  colorSpaceName:NSDeviceRGBColorSpace
				  bytesPerRow:(width*4)
				  bitsPerPixel:0];

		bitmap.size = drawRect.size;

		[NSGraphicsContext saveGraphicsState];
		[NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithBitmapImageRep:bitmap]];

		[image drawInRect:drawRect
				 fromRect:drawRect
				operation:NSCompositeSourceOver
				 fraction:1.0];

		[NSGraphicsContext restoreGraphicsState];

		const U32* srcData = (const U32*)[bitmap bitmapData];
		U8* dstData = (U8*)Rtt_CALLOC( context, bytesPerRow, height );
		fData = dstData;

		size_t srcWidth = bitmap.pixelsWide;

		for ( size_t j = 0; j < height; j++ )
		{
			const U32* srcRowData = srcData;
			U8* rowData = dstData;
			for ( size_t i = 0; i < width; i++ )
			{
				const U32 pixel = srcRowData[i];
				#ifdef Rtt_BIG_ENDIAN
				const U8 r = (pixel & 0xFF);
				#else
				const U8 r = (pixel & 0xFF000000) >> 24;
				#endif

                rowData[i] = r; //( r + g + b + b ) >> 2;
			}
			srcData += srcWidth;
			dstData += width;
		}
		// DEBUG NOTE: Both srcData and dstData point to bogus memory here

		[image release];
		[bitmap release];
        [attributes release];
#else // Rtt_IPHONE_ENV || Rtt_TVOS_ENV
		const size_t bitsPerChannel = 8;
		void* data = Rtt_CALLOC( context, bytesPerRow, height );
		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceGray();
		CGContextRef bmpContext = CGBitmapContextCreate(
										data,
										width, height, bitsPerChannel,
										bytesPerRow, colorSpace, 
										kCGImageAlphaNone );

		if ( Rtt_VERIFY( bmpContext ) )
		{
			CGContextSetGrayFillColor( bmpContext, 1.0, 1.0 );
			CGContextTranslateCTM( bmpContext, 0.0, height );
			CGContextScaleCTM( bmpContext, 1.0, -1.0 ); //NOTE: NSString draws in UIKit referential i.e. renders upside-down compared to CGBitmapContext referential
			UIGraphicsPushContext( bmpContext );
			{
				CGSize actualSize; Rtt_UNUSED( actualSize );
				if ( ! fIsMultiline )
				{
					CGPoint origin = { 0.f, 0.f };

					#ifdef Rtt_IPHONE_ENV
						actualSize = [fText drawAtPoint:origin withFont:fFont];
					#elif defined( Rtt_TVOS_ENV )
						// NOTE: On TVOS, you must specify the color white
						// b/c CGContextSetGrayFillColor has no effect on the text color
						NSDictionary *attributes = @{
							NSFontAttributeName:fFont,
							NSForegroundColorAttributeName:[UIColor whiteColor],
						};
						[fText drawAtPoint:origin withAttributes:attributes];
					#endif
				}
				else
				{
					const CGRect rect = { { 0.f, 0.f }, { (CGFloat)width, (CGFloat)height } };
					actualSize = rect.size;

					#ifdef Rtt_IPHONE_ENV
						[fText drawInRect:rect withFont:fFont lineBreakMode:NSLineBreakByWordWrapping alignment:fTextAlignment];
					#elif defined( Rtt_TVOS_ENV )
						NSMutableParagraphStyle *paragraph = [[NSMutableParagraphStyle alloc] init];
						paragraph.lineBreakMode = NSLineBreakByWordWrapping;
						paragraph.alignment = fTextAlignment;

						// NOTE: On TVOS, you must specify the color white
						// b/c CGContextSetGrayFillColor has no effect on the text color
						NSDictionary *attributes = @{
							NSFontAttributeName:fFont,
							NSParagraphStyleAttributeName:paragraph,
							NSForegroundColorAttributeName:[UIColor whiteColor],
						};
						[fText drawInRect:rect withAttributes:attributes];
					#endif
				}
				Rtt_ASSERT( actualSize.width <= width );
				Rtt_ASSERT( actualSize.height <= height );
			}
			UIGraphicsPopContext();

			CGContextRelease( bmpContext );
			fData = data;
		}
		else
		{
			Rtt_FREE( data );
		}

		CGColorSpaceRelease( colorSpace );
#endif
	}

	return fData;
}

#ifdef Rtt_DEBUG
void
AppleTextBitmap::PrintChannel( const U8 *bytes ) const
{
	int channel = 0;
	U32 bytesPerPixel = 1;
	const U32 width = Width();
	const U32 height = Height();
	const U32 bytesPerRow = width * bytesPerPixel;

	printf( "---------------------------------------------------\n" );
	printf( "[Channel = %d]\n", channel );

	for ( U32 j = 0; j < height; j++ )
	{
		const U8 *rowBytes = bytes + j*bytesPerRow + channel;
		for ( U32 i = 0; i < width; i++ )
		{
			printf( "%02x", *rowBytes );

			rowBytes += bytesPerPixel;
		}
		printf( "\n" );
	}
}
#endif

U32
AppleTextBitmap::Width() const
{
	return fDimensions.width;
}

U32
AppleTextBitmap::Height() const
{
	return fDimensions.height;
}

PlatformBitmap::Format
AppleTextBitmap::GetFormat() const
{
	return PlatformBitmap::kMask;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

