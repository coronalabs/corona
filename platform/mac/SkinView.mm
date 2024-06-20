//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "SkinView.h"

#import <QuartzCore/QuartzCore.h>

#include "Rtt_DeviceOrientation.h"
#include "Rtt_GPUStream.h"

// 10.8 finally gets the CGColor method
// But we need to run on 10.6/10.7.
// Adopted from Bill Dudney (Apache 2.0)
// http://bill.dudney.net/roller/objc/entry/nscolor_cgcolorref
@implementation NSColor(CGColorCompatibility)
- (CGColorRef) CoronaCGColor
{
	// Call Apple's (or I suppose any other CGColor defined in a category we accidentally picked up)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	if([self respondsToSelector:@selector(CGColor)])
	{
		return [self CGColor];
	}
#pragma GCC diagnostic pop
	
    CGColorSpaceRef colorSpace = [[self colorSpace] CGColorSpace];
    NSInteger componentCount = [self numberOfComponents];
    CGFloat *components = (CGFloat *)calloc(componentCount, sizeof(CGFloat));
    [self getComponents:components];
    CGColorRef color = CGColorCreate(colorSpace, components);
    free((void*)components);
	[(id)color autorelease];
    return color;
}

@end


#import <QuartzCore/QuartzCore.h>
#include <OpenGL/gl.h>
//#include <OpenGL/OpenGL.h>
// Fallback intended for Snow Leopard headless Macs. On Lion, we don't seem to have a problem creating the main pixel format. 
// (It just doesn't actually render which seems to be an Apple bug.)
// I also tried avoiding glBlitFramebuffer in this case, but it didn't help.
static CGLPixelFormatAttribute s_coronaFallbackPixelFormatAttributes[] =
{
	kCGLPFADoubleBuffer,
//	kCGLPFADepthSize, (CGLPixelFormatAttribute)16,
//	kCGLPFAColorSize, (CGLPixelFormatAttribute)24,
//	kCGLPFAAlphaSize, (CGLPixelFormatAttribute)8,
	kCGLPFARemotePBuffer, // DTS tells me this is needed for headless Macs
	// Need to investigate: kCGLPFAAccelerated + kCGLPFANoRecovery must be removed to work on headless Macs and I think we also need kCGLPFAAllowOfflineRenderers.
	// But this presumably will deny us hardware acceleration.
	//	kCGLPFAAllowOfflineRenderers,
	// Apple says 10.7 should always specify the OpenGL profile
	// However, testing on 10.6, this seems to cause an error.
	// Things seem to work fine for now on 10.7 without defining it, so I'll disable this for now.
	// Otherwise we need a runttime check.
	/*
	 #if MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_6
	 #if ESSENTIAL_GL_PRACTICES_SUPPORT_GL3
	 // Must specify the 3.2 Core Profile to use OpenGL 3.2
	 //		kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
	 #else
	 //		kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_Legacy,
	 #endif
	 #endif
	 */
	(CGLPixelFormatAttribute)0
};

// We need to create an OpenGL context to query the max texture size because CALayer is limited by that with no direct API to help us.
// We can't use the Rtt::GPUStream version because the SkinView may be created before an OpenGL context is created or the context may not be set to be the current context.
// Warning: This may be unreliable for multiple video cards, particulaly if the user moves across screens.
// Optimization to cache might need to be removed to allow relaunch on a new screen.
static size_t Internal_GetMaxTextureSize()
{
	static dispatch_once_t once;
	static size_t s_maxTextureSize = 2048;
	
	dispatch_once(&once, ^{
		
		CGLPixelFormatObj pixel_format = NULL;
		GLint num_pixel_formats = 0;
		
		CGLChoosePixelFormat(s_coronaFallbackPixelFormatAttributes, &pixel_format, &num_pixel_formats);
		if(NULL == pixel_format)
		{
			NSLog(@"Error: Could not choose fallback OpenGL pixel format for SkinView! OpenGL context cannot be created.");
			return;
		}
		
		
		CGLContextObj temp_context = NULL;
		CGLError error = CGLCreateContext(pixel_format, NULL, &temp_context);
		if(kCGLNoError != error)
		{
			NSLog(@"CGLCreateContext failed to create OpenGL context for SkinView max texture size");
			return;
		}
		
		CGLSetCurrentContext(temp_context);
		GLint retmax;

		glGetIntegerv( GL_MAX_TEXTURE_SIZE, &retmax );
		if ( glGetError() != GL_NO_ERROR )
		{
			NSLog(@"Failed to get GL_MAX_TEXTURE_SIZE for SkinView");

			s_maxTextureSize = 2048; // Default to 2048
		}
		else
		{
			s_maxTextureSize = retmax;
		}
		error = CGLDestroyContext(temp_context);
		if(kCGLNoError != error)
		{
			NSLog(@"CGLDestroyContext for SkinView failed");
		}
		
		error = CGLDestroyPixelFormat(pixel_format);
		if(kCGLNoError != error)
		{
			NSLog(@"CGLDestroyPixelFormat for SkinView failed");
		}
	});
	
	return s_maxTextureSize;
	
	
}

@implementation NSImage (ProportionalScaling)
// Scott Stevenson http://ns.treehouseideas.com/document.page/498
- (NSImage*)imageByScalingProportionallyToSize:(NSSize)targetSize
{
	NSImage* sourceImage = self;
	NSImage* newImage = nil;
	
	if ([sourceImage isValid])
	{
		NSSize imageSize = [sourceImage size];
		float width  = imageSize.width;
		float height = imageSize.height;
		
		float targetWidth  = targetSize.width;
		float targetHeight = targetSize.height;
		
		float scaleFactor  = 0.0;
		float scaledWidth  = targetWidth;
		float scaledHeight = targetHeight;
		
		NSPoint thumbnailPoint = NSZeroPoint;
		
		if ( NSEqualSizes( imageSize, targetSize ) == NO )
		{
			
			float widthFactor  = targetWidth / width;
			float heightFactor = targetHeight / height;
			
			if ( widthFactor < heightFactor )
				scaleFactor = widthFactor;
			else
				scaleFactor = heightFactor;
			
			scaledWidth  = width  * scaleFactor;
			scaledHeight = height * scaleFactor;
			
			if ( widthFactor < heightFactor )
				thumbnailPoint.y = (targetHeight - scaledHeight) * 0.5;
			
			else if ( widthFactor > heightFactor )
				thumbnailPoint.x = (targetWidth - scaledWidth) * 0.5;
		}
		
		newImage = [[NSImage alloc] initWithSize:targetSize];
		
		[newImage lockFocus];
		
		NSRect thumbnailRect;
		thumbnailRect.origin = thumbnailPoint;
		thumbnailRect.size.width = scaledWidth;
		thumbnailRect.size.height = scaledHeight;
		
		[sourceImage drawInRect: thumbnailRect
					   fromRect: NSZeroRect
					  operation: NSCompositeSourceOver
					   fraction: 1.0];
		
		[newImage unlockFocus];
		
	}
	
	return [newImage autorelease];
}

@end

#import <QuartzCore/QuartzCore.h>
@interface SkinView ()

@property(nonatomic, retain) CATiledLayer* layerView;
//@property(nonatomic, retain) CALayer* layerView;

@end

@implementation SkinView

@synthesize fAngle;
@synthesize nativeSize;
@synthesize nativeSkinImage;

- (id)initWithFrame:(NSRect)frameRect
	  isTransparent:(BOOL)isTransparent
{
	self = [super initWithFrame:frameRect];
	if ( self )
	{
		fSkinImage = nil;
		fTransform = nil;
		fAngle = 0;
		nativeSkinImage = nil;
		nativeSize = NSZeroSize;
		fScale = 1.f;

        [self setWantsLayer:YES];
		[self setLayer:[CALayer layer]];

		_layerView = [CATiledLayer layer];
		[_layerView retain];
		
		
		[[self layerView] setFrame:frameRect];
		
		[[self layerView] setBackgroundColor:[[NSColor clearColor] CoronaCGColor]];
		[[self layer] setBackgroundColor:[[NSColor clearColor] CoronaCGColor]];
		[[self layerView] setOpaque:NO];
		[[self layer] setOpaque:NO];

		if (isTransparent)
		{
			[[self layer] setOpacity:0.125];
		}
	
		[[self layer] addSublayer:[self layerView]];
	}

	return self;
}

- (void)dealloc
{
	[fTransform release];
	[fSkinImage release];

	[nativeSkinImage release];
	
	[_layerView release];
	[super dealloc];
}

- (NSImage*)image
{
	return fSkinImage;
}

- (NSSize) nativeSize
{
	return nativeSize;
}

- (NSSize) imageSize
{
	return nativeSize;
}

- (void)setImage:(NSImage*)image
{
	Rtt_ASSERT( image );

	[self setFrameSize:[image size]];
	[fSkinImage release];
	[nativeSkinImage release];
	fSkinImage = [image retain];
	nativeSkinImage = [fSkinImage retain];
	
	self.nativeSize = [nativeSkinImage size];
	
	[self setNeedsDisplay:YES];
}

- (void) scaleImageToSize:(NSSize)newsize
{
	[fSkinImage release];
	// Avoid image degradation with constant rescaling by resuing original image as the base.
	if( NSEqualSizes(newsize, nativeSize) )
	{
		fSkinImage = nativeSkinImage;
	}
	else
	{
		fSkinImage = [nativeSkinImage imageByScalingProportionallyToSize:newsize];
	}
	[fSkinImage retain];
	
//	[self setFrame:NSMakeRect(0, 0, newsize.width, newsize.height)];

	if(fAngle == 90.0 || fAngle == -90.0)
	{
		[self setFrameSize:NSMakeSize(newsize.height, newsize.width)];
//		[self setFrameSize:NSMakeSize([nativeSkinImage size].height, [nativeSkinImage size].width)];
	}
	else
	{
		[self setFrameSize:newsize];	
//		[self setFrameSize:nativeSize];	
	}
//	[self angleDidChange];
//	[self recomputeTransform];
	[self setNeedsDisplay:YES];
}

- (void)angleDidChange
{
#ifdef Rtt_AUTHORING_SIMULATOR
	using namespace Rtt;

	bool isUpright = DeviceOrientation::IsAngleUpright( fAngle );
	if ( isUpright )
	{
		fAngle = 0;
	}
	CGFloat newangle = fAngle;
	if(fAngle < 0)
	{
		newangle += 360.0;
	}
	else
	{
	}
	newangle = newangle * 2 * M_PI / 360.0;
	[self setRotationAngle:newangle];

	NSRect viewRect = [self frame];
	NSSize viewSize = viewRect.size;
	float w = viewSize.width;
	float h = viewSize.height;
	viewSize.width = h;
	viewSize.height = w;
	[self setFrameSize:viewSize];	

	
	if ( fTransform )
	{
		[fTransform release];
	}
	
	NSAffineTransform* t = nil;
	
	if ( ! isUpright )
	{
		// NOTE: the width and height need to be in terms of the original
		// upright size, so for upside down, we swap the w,h to recover
		// the proper upright w,h; the other cases, w,h start from the
		// pre-rotated orientation.
		float halfW = w / 2;
		float halfH = h / 2;
		t = [NSAffineTransform transform];
		if ( DeviceOrientation::IsAngleUpsideDown( fAngle ) )
		{
			float tmp = halfW;
			halfW = halfH;
			halfH = tmp;
		}
		
		[t translateXBy:halfW yBy:halfH];

		[t rotateByDegrees:fAngle];
		[t translateXBy:-halfW yBy:-halfH];
		
		/*
		 Rtt_TRACE( ( "half w,h (%g,%g)\n", halfW, halfH ) );
		 
		 NSAffineTransformStruct ts = [t transformStruct];
		 Rtt_TRACE( ("[\t%g\t%g\t%g]\n", ts.m11, ts.m12, ts.tX ) );
		 Rtt_TRACE( ("[\t%g\t%g\t%g]\n", ts.m21, ts.m22, ts.tY ) );
		 */
	}
	[t scaleBy:fScale];

	fTransform = [t retain];
	
	NSRect rect = [self frame];
	Rtt_TRACE( ( "viewRect origin(%g,%g) size(%g,%g)\n", rect.origin.x, rect.origin.y, rect.size.width, rect.size.height ) );

	[self setNeedsDisplay:YES];
#endif // Rtt_AUTHORING_SIMULATOR
}

- (void) recomputeTransform
{
#ifdef Rtt_AUTHORING_SIMULATOR

	using namespace Rtt;

	NSRect viewRect = [self frame];
	NSSize viewSize = viewRect.size;
	float w = viewSize.width;
	float h = viewSize.height;
	
	if ( fTransform )
	{
		[fTransform release];
	}

	NSAffineTransform* t = nil;
	bool isUpright = DeviceOrientation::IsAngleUpright( fAngle );

	if ( ! isUpright )
	{
		// NOTE: the width and height need to be in terms of the original
		// upright size, so for upside down, we swap the w,h to recover
		// the proper upright w,h; the other cases, w,h start from the
		// pre-rotated orientation.
		float halfW = w / 2;
		float halfH = h / 2;
		t = [NSAffineTransform transform];
		if ( DeviceOrientation::IsAngleUpsideDown( fAngle ) )
		{
			float tmp = halfW;
			halfW = halfH;
			halfH = tmp;
			[t translateXBy:halfW yBy:halfH];
		}
		else
		{
			[t translateXBy:halfH yBy:halfW];
		}

		[t rotateByDegrees:fAngle];
		[t translateXBy:-halfW yBy:-halfH];

		/*
		Rtt_TRACE( ( "half w,h (%g,%g)\n", halfW, halfH ) );

		NSAffineTransformStruct ts = [t transformStruct];
		Rtt_TRACE( ("[\t%g\t%g\t%g]\n", ts.m11, ts.m12, ts.tX ) );
		Rtt_TRACE( ("[\t%g\t%g\t%g]\n", ts.m21, ts.m22, ts.tY ) );
		*/
	}

	fTransform = [t retain];

	NSRect rect = [self frame];
	Rtt_TRACE( ( "viewRect origin(%g,%g) size(%g,%g)\n", rect.origin.x, rect.origin.y, rect.size.width, rect.size.height ) );
	
	[self setNeedsDisplay:YES];
#endif // Rtt_AUTHORING_SIMULATOR

}


- (S32)rotate:(bool)clockwise
{

	fAngle += (clockwise ? -90 : 90 );
	
	[self angleDidChange];
	
	return fAngle;
}

- (void)setOrientation:(Rtt::DeviceOrientation::Type)orientation
{
	using namespace Rtt;
	if ( DeviceOrientation::OrientationForAngle( fAngle ) != orientation )
	{
		if ( Rtt_VERIFY( DeviceOrientation::IsInterfaceOrientation( orientation ) ) )
		{
			fAngle = DeviceOrientation::CalculateRotation( DeviceOrientation::kUpright, orientation );
			[self angleDidChange];
		}
	}
}

- (BOOL) acceptsFirstResponder
{
	return NO;
}
- (BOOL) acceptsFirstMouse
{
	return NO;
}

- (void)mouseDown:(NSEvent*)event
{
//	NSLog( @"%@", event );
	[[self nextResponder] mouseDown:event];
//	NSLog(@"nextResponder: %@", [self nextResponder]);
}

- (void)mouseDragged:(NSEvent*)event
{
//	NSLog( @"%@", event );
	[[self nextResponder] mouseDragged:event];
//	NSLog(@"nextResponder: %@", [self nextResponder]);
}

- (void)mouseUp:(NSEvent*)event
{
//	NSLog( @"%@", event );
	//	 [super mouseDown:event];
	[[self nextResponder] mouseUp:event];
//	NSLog(@"nextResponder: %@", [self nextResponder]);
}

- (void) setFrame:(NSRect)frame_rect
{
	[super setFrame:frame_rect];
	[[self layer] setFrame:frame_rect];
	[[self layerView] setFrame:frame_rect];

}

- (void) setFrameSize:(NSSize)frame_size
{
	[super setFrameSize:frame_size];
	NSRect currentframe = [self frame];
	currentframe.size = frame_size;
	[[self layer] setFrame:currentframe];
	[[self layerView] setFrame:currentframe];
//	[[self layer] setBounds:currentframe];
	
}

- (BOOL) setImageWithURL:(NSURL*)the_url
{
	if(nil == the_url)
	{
		[[self layerView] setContents:nil];
		//		[[self layerView] setNeedsDisplay];
		
		//		[self setNeedsDisplay:YES];
		return NO;
	}
	CGImageSourceRef source_ref = CGImageSourceCreateWithURL((CFURLRef)the_url, NULL);
	if(NULL == source_ref)
	{
		[[self layerView] setContents:nil];
		return NO;
	}
	
	
	CFDictionaryRef fileinforef = CGImageSourceCopyPropertiesAtIndex(source_ref, 0, nil);
	NSDictionary* fileinfo = (NSDictionary*)fileinforef;
	size_t w = [[fileinfo valueForKey:(NSString*)kCGImagePropertyPixelWidth] integerValue];
	size_t h = [[fileinfo valueForKey:(NSString*)kCGImagePropertyPixelHeight] integerValue];	
	CFRelease(fileinforef);
	
	
	CGImageRef image_ref = NULL;
	
	CGRect image_rect = CGRectMake(0, 0, w, h);
	
	bool too_big = false;
	
	/* CALayer is tied to OpenGL so it also suffers from the GL_MAX_TEXTURE_SIZE. Core Animation doesn't provide an API for this, so we have to do it the hard way. If we exceed the max size, we can:
	 - use CATiledLayer
	 - tile it ourselves
	 - scale down the image
	 
	 CATiledLayer is the general solution to this problem but requires some work 
	 Manually cutting up the image into 2 or 4 pieces might work too…not sure if this is easier than CATiledLayer
	 Decimating the resolution to lower it might be an alternative but may not look good 
	 
	 Finding the MAX_TEXTURE_SIZE requires a call into OpenGL. At this point, we don't necessarily have a runtime going nor an OpenGL context. Caching this value might be useful.
	 
	 Dealing with multiple video cards with different max sizes might be a pain
	 Note: Intel GMA 950 and GMA X3100 are the only cards with MAX of 2048 (all others are larger). Right now iPad Retina is the only skin that requires larger. These GMA video cards were dropped in 10.8. 
	 
	 Alternatively, we could simply fallback to normal window without skin too in this case. Looks like Apple is doing this for the iOS simulator for the iPad Retina. Long standing feature requests for normal windows (and no-autozooming).

	// Use this for getting a sub image for a tile??? CGImageCreateWithImageInRect(image_ref, sub_rect)

	 */
	size_t max_texture_size = Internal_GetMaxTextureSize();
	if( (w > max_texture_size) || (h > max_texture_size) )
	{
		too_big = true;
	}

	if(too_big)
	{
		// This creates a smaller image ("thumbnail").
		// We will rely on CALayer to stretch/scale the image so it fits to the view.
		// CATiledLayer would probably be better, but because we play so many tricks with the frame rect to rotate/zoom ourselves, this might get even more complicated because we may need to coordinate the CATiledLayer's bounds with what's going on with our own rotation/zoom.
		// The skin will look blurry/lowres if this code is hit.
		CFDictionaryRef options = (CFDictionaryRef)[[NSDictionary alloc] initWithObjectsAndKeys:(id)kCFBooleanTrue, (id)kCGImageSourceCreateThumbnailWithTransform, (id)kCFBooleanTrue, (id)kCGImageSourceCreateThumbnailFromImageIfAbsent, (id)[NSNumber numberWithUnsignedInteger:max_texture_size], (id)kCGImageSourceThumbnailMaxPixelSize, nil];
			image_ref = CGImageSourceCreateThumbnailAtIndex(source_ref, 0, options); // Create scaled image
			CFRelease(options);

	}
	else
	{
		image_ref = CGImageSourceCreateImageAtIndex(source_ref, 0, NULL);
//		size_t w = CGImageGetWidth( image_ref );
//		size_t h = CGImageGetHeight( image_ref );

	}
			
			
	
	[[self layerView] setContents:(id)image_ref];
	
	[[self layerView] setFrame:image_rect];
	
	CFRelease(source_ref);

	CGImageRelease(image_ref);

	nativeSize = image_rect.size;
    
    return YES;
}

- (void) setRotationAngle:(CGFloat)angle_in_radians
{
	//	CATransform3D rotate_transform = CATransform3DMakeRotation(180.0 * M_PI / 180.0, 0, 0, 1.0);
	CATransform3D rotate_transform = CATransform3DMakeRotation(angle_in_radians, 0, 0, 1.0);
	[[self layerView] setTransform:rotate_transform];
}

@end
