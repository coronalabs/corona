//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_IPhoneDisplayObject.h"
#include "Rtt_IPhonePlatformBase.h"

#import <UIKit/UIView.h>
#import <UIKit/UIScreen.h>
#import "CoronaLuaObjC.h"
#import "CoronaLuaObjC+NSObject.h"

#include "Display/Rtt_Display.h"
#include "Rtt_Runtime.h"


#include "Rtt_Lua.h"
#include "Display/Rtt_GroupObject.h"
#include "Display/Rtt_LuaLibDisplay.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

IPhoneDisplayObject::IPhoneDisplayObject( const Rect& bounds )
:	fSelfBounds( bounds ),
	fView( nil ),
	fCoronaView( nil ),
	fHidden( NO )
{
	Real w = bounds.Width();
	Real h = bounds.Height();
	Real halfW = Rtt_RealDiv2( w );
	Real halfH = Rtt_RealDiv2( h );

	// The UIView's self transform is relative to it's center, but the DisplayObject's
	// transform included that translation, so we need to factor this out during Build()
	// NOTE: The incoming bounds are in content coordinates, not native UIView coordinates,
	// so we must record these separately instead of relying on the values of [fView center]
	fViewCenter.x = bounds.xMin + halfW;
	fViewCenter.y = bounds.yMin + halfH;

	// Update DisplayObject so that it corresponds to the actual position of the UIView
	// where DisplayObject's self bounds will be centered around its local origin.
	Translate( fViewCenter.x, fViewCenter.y );

	// The self bounds needs to be centered around DisplayObject's local origin
	// even though UIView's bounds will not be.
	fSelfBounds.MoveCenterToOrigin();
}

IPhoneDisplayObject::~IPhoneDisplayObject()
{
	[fView removeFromSuperview];
	[fView release];
}

void
IPhoneDisplayObject::Preinitialize( const Display& display )
{
	Rtt_ASSERT( display.PointsWidth() == [UIScreen mainScreen].bounds.size.width );
	Rtt_ASSERT( display.PointsHeight() == [UIScreen mainScreen].bounds.size.height );

	PlatformDisplayObject::Preinitialize( display );

    // We can access the GLView at this point via the display's runtime, so save it for later
    const Rtt::IPhonePlatformBase& platform = static_cast< const Rtt::IPhonePlatformBase& >( display.GetRuntime().Platform() );
    fCoronaView = platform.GetView();
}

void
IPhoneDisplayObject::InitializeView( UIView *view )
{
	Rtt_ASSERT( ! fView );
	Rtt_ASSERT( view );

	fView = [view retain];
}

void
IPhoneDisplayObject::SetFocus()
{
	[fView becomeFirstResponder];
}

void
IPhoneDisplayObject::DidMoveOffscreen()
{
	fHidden = fView.hidden; // Store original value while offscreen
	fView.hidden = YES;		// Force view to be hidden
}

void
IPhoneDisplayObject::WillMoveOnscreen()
{
	fView.hidden = fHidden; // Restore view's hidden parameter
	fHidden = NO;			// Restore to default value
}

void
IPhoneDisplayObject::Prepare( const Display& display )
{
	Super::Prepare( display );

	const Matrix &transf = GetSrcToDstMatrix();

	CGAffineTransform xfm = CGAffineTransformIdentity;

	const Real *x_row = transf.Row0();
	const Real *y_row = transf.Row1();

	// We have to invert "b" and "c" because our rotation
	// direction is opposite of the one in a UIView.
	xfm.a = x_row[0]; // x.
	xfm.b = ( - x_row[1] ); // y.

	xfm.c = ( - y_row[0] ); // x.
	xfm.d = y_row[1]; // y.

	// Take into account content-scaling.
	float content_offset_x = 0.0f;
	float content_offset_y = 0.0f;
	GetContentOffsets( content_offset_x,
						content_offset_y );

	CGPoint c;
	c.x = ( GetContentToScreenSx() * ( transf.Tx() + content_offset_x ) );
	c.y = ( GetContentToScreenSy() * ( transf.Ty() + content_offset_y ) );
	fView.center = c;

	fView.transform = xfm;
}

void
IPhoneDisplayObject::Translate( Real dx, Real dy )
{
	Super::Translate( dx, dy );

}

void
IPhoneDisplayObject::Draw( Renderer& renderer ) const
{
}

void
IPhoneDisplayObject::GetSelfBounds( Rect& rect ) const
{
	rect = fSelfBounds;
}

void
IPhoneDisplayObject::SetSelfBounds( Real width, Real height )
{
	CGRect newFrame = fView.frame;
	
	if ( !( width < Rtt_REAL_0 ) ) // (width >= 0)
	{
		//SelfBounds needs to represent Corona coordinates
		//whereas the newFrame size is in content units, so we calculate
		//both to keep everything in sync
		float newPointWidth = width * GetContentToScreenSx();
		float coronaWidth	= width;
		float coronaHeight	= newFrame.size.height / GetContentToScreenSy();
		
		fSelfBounds.Initialize(coronaWidth/2.0f, coronaHeight/2.0f);
		Invalidate( kGeometryFlag | kStageBoundsFlag | kTransformFlag );
		
		newFrame.size.width = newPointWidth;
		
	}
	if ( !( height < Rtt_REAL_0 ) ) // (height >= 0)
	{
	
		//SelfBounds needs to represent Corona coordinates
		//whereas the newFrame size is in content units, so we calculate
		//both to keep everything in sync
		float newPointHeight	= height * GetContentToScreenSy();
		float coronaWidth		= newFrame.size.width / GetContentToScreenSx();
		float coronaHeight		= height;
		
		fSelfBounds.Initialize(coronaWidth/2.0f, coronaHeight/2.0f);
		Invalidate( kGeometryFlag | kStageBoundsFlag | kTransformFlag );
		
		newFrame.size.height = newPointHeight;
	}
	
	[fView setFrame:newFrame];

}

int
IPhoneDisplayObject::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;

	if ( strcmp( "isVisible", key ) == 0 )
	{
		lua_pushboolean( L, ! fView.hidden );
	}
	else if ( strcmp( "alpha", key ) == 0 )
	{
		lua_pushnumber( L, fView.alpha );
	}
	else if ( strcmp( "hasBackground", key ) == 0 )
	{
		// TODO: When we allow changing bkgd colors, we should cache this as
		// a separate property which would ignore the UIColor.
		
		if (fView.backgroundColor == nil)
		{
			// We've never set "hasBackground", default is true
			lua_pushboolean( L, true );
		}
		else
		{
			UIColor *color = fView.backgroundColor;
			CGFloat alpha = CGColorGetAlpha( [color CGColor] );
			lua_pushboolean( L, alpha > FLT_EPSILON );
		}
	}
	else
	{
		result = 0;
	}

	return result;
}

bool
IPhoneDisplayObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	if ( strcmp( "isVisible", key ) == 0 )
	{
		fView.hidden = ! lua_toboolean( L, valueIndex );
	}
	else if ( strcmp( "alpha", key ) == 0 )
	{
		fView.alpha = lua_tonumber( L, valueIndex );
	}
	else if ( strcmp( "hasBackground", key ) == 0 )
	{
		UIColor *color = lua_toboolean( L, valueIndex ) ? [UIColor whiteColor] : [UIColor clearColor];
		fView.backgroundColor = color;
	}
	else
	{
		result = false;
	}

	return result;
}

id
IPhoneDisplayObject::GetNativeTarget() const
{
	return GetView();
}

int
IPhoneDisplayObject::GetNativeProperty( lua_State *L, const char key[] ) const
{
	id target = GetNativeTarget();
	int result = [target pushLuaValue:L forKey:@(key)];

	if ( 0 == result )
	{
		result = Super::GetNativeProperty( L, key );
	}

	return result;
}

bool
IPhoneDisplayObject::SetNativeProperty( lua_State *L, const char key[], int valueIndex )
{
	id target = GetNativeTarget();
	bool result = [target set:L luaValue:valueIndex forKey:@(key)];

	if ( ! result )
	{
		result = Super::SetNativeProperty( L, key, valueIndex );
	}

	return result;
}

CoronaView*
IPhoneDisplayObject::GetCoronaView() const
{
    Rtt_ASSERT(fCoronaView); // this is set in PreInitialize()
	return fCoronaView;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

