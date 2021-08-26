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

struct CoronaHandle {
    void * internal[2];
};

typedef struct CoronaRenderer * CoronaRendererHandle;
typedef struct CoronaRenderData * CoronaRenderDataHandle;
typedef struct CoronaShader * CoronaShaderHandle;
typedef struct CoronaShaderData * CoronaShaderDataHandle;
typedef struct CoronaDisplayObject * CoronaDisplayObjectHandle;
typedef struct CoronaGroupObject * CoronaGroupObjectHandle;

#endif // _CoronaPublicTypes_H__
