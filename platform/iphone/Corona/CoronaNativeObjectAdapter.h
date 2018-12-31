// ----------------------------------------------------------------------------
// 
// CoronaNativeObjectAdapter.h
// Copyright (c) 2012 Corona Labs Inc. All rights reserved.
// 
// ----------------------------------------------------------------------------

#ifndef _CoronaNativeObjectAdapter_H__
#define _CoronaNativeObjectAdapter_H__

#include "CoronaMacros.h"

// ----------------------------------------------------------------------------

typedef struct lua_State lua_State;

// ----------------------------------------------------------------------------

@protocol CoronaNativeObjectAdapter< NSObject >

@property (nonatomic, readonly, assign) UIView *view;

- (int)indexForState:(lua_State *)L key:(const char *)key;
- (BOOL)newIndexForState:(lua_State *)L key:(const char *)key index:(int)index;

@end

CORONA_EXPORT int CoronaPushNativeObject( lua_State *L, CGRect bounds, id<CoronaNativeObjectAdapter>adapter );

// ----------------------------------------------------------------------------

#endif // _CoronaNativeObjectAdapter_H__
