//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifdef Rtt_MetalANGLE
#import <MetalANGLE/MGLKit.h>

typedef MGLContext Rtt_EAGLContext;
typedef MGLKViewController Rtt_GLKViewController;
typedef MGLKView Rtt_GLKView;

#define Rtt_API_GLES2 kMGLRenderingAPIOpenGLES2
#define Rtt_DrawableDepth24 MGLDrawableDepthFormat24

#else
#import <GLKit/GLKit.h>

typedef EAGLContext Rtt_EAGLContext;
typedef GLKViewController Rtt_GLKViewController;
typedef GLKView Rtt_GLKView;

#define Rtt_API_GLES2 kEAGLRenderingAPIOpenGLES2
#define Rtt_DrawableDepth24 GLKViewDrawableDepthFormat24

#endif


// ----------------------------------------------------------------------------
