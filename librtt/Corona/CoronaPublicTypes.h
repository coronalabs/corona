//-----------------------------------------------------------------------------
//
// Corona Labs
//
// easing.lua
//
// Code is MIT licensed; see https://www.coronalabs.com/links/code/license
//
//-----------------------------------------------------------------------------

#ifndef _CoronaPublicTypes_H__
#define _CoronaPublicTypes_H__

/**
 Types used by certain "object" functions in the API.
 
 These should be considered opaque, but are made available here to be passed by
 value and declared on the stack.
*/
#define CORONA_DECLARE_PUBLIC_TYPE( TYPE ) typedef struct { unsigned char data[ sizeof( void* ) + 8 ]; } Corona ## TYPE

CORONA_DECLARE_PUBLIC_TYPE( Renderer );
CORONA_DECLARE_PUBLIC_TYPE( RenderData );
CORONA_DECLARE_PUBLIC_TYPE( Shader );
CORONA_DECLARE_PUBLIC_TYPE( ShaderData );
CORONA_DECLARE_PUBLIC_TYPE( DisplayObject );
CORONA_DECLARE_PUBLIC_TYPE( GroupObject );
CORONA_DECLARE_PUBLIC_TYPE( CommandBuffer );

#endif // _CoronaPublicTypes_H__
