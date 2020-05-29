//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

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
