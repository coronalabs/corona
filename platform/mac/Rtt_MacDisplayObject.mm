//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_AppleTextAlignment.h"

#include "Rtt_MacDisplayObject.h"
#include "Rtt_MacPlatform.h"
#include "Rtt_MacSimulator.h"
#import "SimulatorDeviceWindow.h"

#import <AppKit/AppKit.h>
#import "AppDelegate.h"
#import "GLView.h"
#include "Rtt_Display.h"

#include "Rtt_Lua.h"
#include "Rtt_Runtime.h"
#include "Display/Rtt_LuaLibDisplay.h"
#include "Display/Rtt_StageObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

MacDisplayObject::MacDisplayObject( const Rect& bounds )
:	fSelfBounds( bounds),
	fView( nil ),
	fLayerHostSuperView( nil ),
	fIsHidden ( false ),
	fCachedSimulatorScale( Rtt_REAL_1 )
{
	// Note: Setting the reference point to center is not done in any of the other implementations because
	// fSelfBounds is already centered/converted unlike this implementation.
	// This solves the problem, but will possibly be a problem if/when we support object resizing.
	// Either this code should be converted to center fSelfBounds initially or the 
	// subclass virtual function will need to account for the offset.
	
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

	// If running in the Corona simulator, then fetch its current zoom level scale.
	CacheSimulatorScale();
	
	// Update DisplayObject so that it corresponds to the actual position of the UIView
	// where DisplayObject's self bounds will be centered around its local origin.
	Translate( fViewCenter.x, fViewCenter.y );

	// The self bounds needs to be centered around DisplayObject's local origin
	// even though UIView's bounds will not be.
	fSelfBounds.MoveCenterToOrigin();
	
}

MacDisplayObject::~MacDisplayObject()
{
	[fView removeFromSuperview];
	[fView release];
	// I don't think this is working quite right. Removing an element in the middle of the list seems to keep some intermediate focus.
	// But it doesn't crash.
	RecomputeNextKeyViews();
}

void
MacDisplayObject::InitializeView( NSView *view )
{
	Rtt_ASSERT( ! fView );
	Rtt_ASSERT( view );

	fView = [view retain];
	NSRect view_frame = [view frame];
	fNSViewFrame = view_frame;
	
	// Note: Setting the reference point to center is not done in any of the other implementations because
	// fSelfBounds is already centered/converted unlike this implementation.
	// This solves the problem, but will possibly be a problem if/when we support object resizing.
	// Either this code should be converted to center fSelfBounds initially or the 
	// subclass virtual function will need to account for the offset.
///	SetReferencePoint(NULL, kReferenceCenter );
}

void
MacDisplayObject::SetFocus()
{
	// Ugh. Why is this so hard? It seems we need a delay in our NativeKeyboard2 example or the responder won't change.
	// Probably due to being in the middle of manipulation of a different responder when hitting return which highlights all the text of the current textfield.
	// But then we need to avoid infinite recursion. So add a check to not call this if already the first responder.
	// This is very dangerous and unreliable. First, in AppKit, there isn't an easy/good way to figure out the current first responder. (You can find out the first responder for say a window, but you don't know if that is actually the current responder.)
	// Second, for NSTextField, the responder isn't the actual responder but it is a NSTextView. You need code like this:
/*
	if ( [[[self window] firstResponder] isKindOfClass:[NSTextView class]] &&
		[[self window] fieldEditor:NO forObject:nil] != nil ) {
        NSTextField *field = [[[self window] firstResponder] delegate];
        if (field != self) {
            // do something based upon first-responder status
			//			[[self window] performSelector:@selector(makeFirstResponder:) withObject:self afterDelay:0.1];
	
			
        }
	}
*/	
	[[fView window] performSelector:@selector(makeFirstResponder:) withObject:fView afterDelay:0.0];
}
void
MacDisplayObject::DidMoveOffscreen()
{
}

void
MacDisplayObject::WillMoveOnscreen()
{
}

bool
MacDisplayObject::CanCull() const
{
	// Disable culling for all native display objects.
	// Note: This is needed so that the Build() function will get called when a native object
	//       is being moved partially or completely offscreen.
	return false;
}


void
MacDisplayObject::PreInitialize( const Display& display )
{

	PlatformDisplayObject::Preinitialize( display );
    
    // We can access the GLView at this point via the display's runtime, so save it for later
    const Rtt::MacPlatform& platform = static_cast< const Rtt::MacPlatform& >( display.GetRuntime().Platform() );
    fLayerHostSuperView = platform.GetView();
}

void
MacDisplayObject::Translate( Real dx, Real dy )
{
	Super::Translate( dx, dy );
	
}

void
MacDisplayObject::Prepare( const Display& display )
{

	Super::Prepare( display );
	
	if ( ShouldPrepare() )
	{
		
		PreInitialize( display );
		
		Rect screenBounds;
		GetScreenBounds( screenBounds );

		NSRect r = NSMakeRect( screenBounds.xMin, screenBounds.yMin, screenBounds.Width(), screenBounds.Height() );
		CGFloat backingScaleFactor = [[fLayerHostSuperView window] backingScaleFactor];
		if (backingScaleFactor > 1.0)
		{
			r.origin.x /= backingScaleFactor;
			r.origin.y /= backingScaleFactor;
			r.size.width /= backingScaleFactor;
			r.size.height /= backingScaleFactor;
		}

		[fView setFrame:r];
		fNSViewFrame = r;
		if ( ! fIsHidden )
		{
			// Only restore the object if the user hasn't requested it hidden
			[fView setHidden:NO];
		}
		
		const Matrix& tSelf = GetMatrix();
		if ( tSelf.IsIdentity() )
		{
			// What is fSelfBounds? Is it our frame? What is the mapping from fSelfBounds to bounds or frame?
			//[fView setFrameOrigin:fNSViewFrame.origin];
			[fView setFrameOrigin:fNSViewFrame.origin];
			
		}
		else
		{
			[fView setFrameOrigin:r.origin];
			[fView setFrameSize:r.size];
		}
		
		// If running in the Corona simulator, then fetch its current zoom level scale.
		// This function will call the DidRescaleSimulator() if the zoom level has just changed.
		CacheSimulatorScale();
	}
	
	// We have a problem because this Build matrix is computed some time after the object was created.
	// If we insert the object when created, when this method is invoked shortly later, you can see
	// a visible jump in the object's position which looks bad.
	// The workaround is to defer adding the object to the view until after this method has been computed at least once.
	if ( nil == [fView superview] )
	{
		AddSubviewToLayerHostView();
	}

}

void
MacDisplayObject::Draw( Renderer& renderer ) const
{
}

void
MacDisplayObject::GetSelfBounds( Rect& rect ) const
{
	rect = fSelfBounds;
}
	
void
MacDisplayObject::SetSelfBounds( Real width, Real height )
{
	if ( width > Rtt_REAL_0 )
	{
		fSelfBounds.Initialize( Rtt_RealDiv2(width), Rtt_RealDiv2(GetGeometricProperty(kHeight)) );
	}
	
	if ( height > Rtt_REAL_0 )
	{
		fSelfBounds.Initialize( Rtt_RealDiv2(GetGeometricProperty(kWidth)), Rtt_RealDiv2(height) );
	}
	
	// Causes prepare to be called which does the actual resizing
	Invalidate( kGeometryFlag | kStageBoundsFlag | kTransformFlag );
}

NSView*
MacDisplayObject::GetLayerHostSuperView()
{
    Rtt_ASSERT(fLayerHostSuperView); // this is set in PreInitialize()
	return fLayerHostSuperView;
}

void
MacDisplayObject::AddSubviewToLayerHostView()
{
	GLView* layerhostview = (GLView *)GetLayerHostSuperView();
	
    [layerhostview addSubview:fView];

	// Setting the NSView's wantsLayer is necessary for the native controls to work with the OpenGL canvas
	[fView setWantsLayer:YES];

	// This block tries to set the nextkeyview's for all the objects in the layer host view
	RecomputeNextKeyViews();
}
	
void
MacDisplayObject::RecomputeNextKeyViews()
{
	NSView* layerhostview = GetLayerHostSuperView();
	
	
	// This block tries to set the nextkeyview's for all the subviews in the layer host view.
	// This is so tabbing to the next field works.
	// This is highly dependent on the order of how the subviews were added/arranged in the subviews array.
	// Our deferred insertion technique to avoid the Build/jumping problem may cause problems in the future.
	NSView* lastview = nil;
	NSView* firstview = nil;
	for(NSView* view in [layerhostview subviews])
	{
		if(nil == lastview)
		{
			lastview = view;
			firstview = view;
			continue;
		}
//		NSLog(@"lastview: %@, view:%@", lastview, view);
		
		[lastview setNextKeyView:view];
		lastview = view;
	}
	[lastview setNextKeyView:firstview];
}

bool
MacDisplayObject::IsInSimulator() const
{
#if Rtt_AUTHORING_SIMULATOR
	MacSimulator *simulator = [(AppDelegate*)[NSApp delegate] simulator];
	const StageObject *stageObject = GetStage();
	if ( simulator && stageObject )
	{
		const MacPlatform &platform = (const MacPlatform&)(stageObject->GetDisplay().GetRuntime().Platform());
		SimulatorDeviceWindow *simulatorWindow = (SimulatorDeviceWindow*)(simulator->GetWindow());
		if ( [platform.GetView() window] == simulatorWindow )
		{
			return true;
		}
	}
#endif // Rtt_AUTHORING_SIMULATOR
	return false;
}

float
MacDisplayObject::GetSimulatorScale() const
{
#if Rtt_AUTHORING_SIMULATOR
	return fCachedSimulatorScale;
#else
	return 1.0;
#endif
}

void
MacDisplayObject::CacheSimulatorScale()
{
#if Rtt_AUTHORING_SIMULATOR
	// If this object is running in Corona's simulator window, then fetch the window's current zoom level scale.
	float currentScale = Rtt_REAL_1;
	MacSimulator *simulator = [(AppDelegate*)[NSApp delegate] simulator];
	const StageObject *stageObject = GetStage();
	if ( simulator && stageObject )
	{
		const MacPlatform &platform = (const MacPlatform&)(stageObject->GetDisplay().GetRuntime().Platform());
		SimulatorDeviceWindow *simulatorWindow = (SimulatorDeviceWindow*)(simulator->GetWindow());
		if ( [platform.GetView() window] == simulatorWindow )
		{
			currentScale = [simulatorWindow scale];
		}
	}
	
	// Raise a notification if zoom level scale has changed.
	float delta = fabsf( fCachedSimulatorScale - currentScale );
	if ( delta > FLT_EPSILON )
	{
		float previousScale = fCachedSimulatorScale;
		fCachedSimulatorScale = currentScale;
		DidRescaleSimulator( previousScale, currentScale );
	}
#endif // Rtt_AUTHORING_SIMULATOR
}

void
MacDisplayObject::DidRescaleSimulator( float previousScale, float currentScale )
{
	// No-op by default. This event is intended for subclasses.
}

int
MacDisplayObject::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;

	if ( strcmp( "isVisible", key ) == 0 )
	{
		lua_pushboolean( L, ! fIsHidden );
	}
	else if ( strcmp( "alpha", key ) == 0 )
	{
		lua_pushnumber( L, [fView alphaValue] );
	}
	else if ( strcmp( "hasBackground", key ) == 0 )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
		lua_pushnil( L );
		/*
		// TODO: When we allow changing bkgd colors, we should cache this as
		// a separate property which would ignore the NSColor.
		NSColor *color = fView.backgroundColor;
		CGFloat alpha = CGColorGetAlpha( [color CGColor] );
		lua_pushboolean( L, alpha > FLT_EPSILON );
		*/
	}
	else
	{
		result = 0;
	}

	return result;
}

bool
MacDisplayObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	if ( strcmp( "isVisible", key ) == 0 )
	{
		[fView setHidden: ! lua_toboolean( L, valueIndex )];
		fIsHidden = ! lua_toboolean( L, valueIndex );
	}
	else if ( strcmp( "alpha", key ) == 0 )
	{
		[fView setAlphaValue: lua_tonumber( L, valueIndex )];
	}
	else if ( strcmp( "hasBackground", key ) == 0 )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
		/*
		NSColor *color = lua_toboolean( L, valueIndex ) ? [NSColor whiteColor] : [NSColor clearColor];
		fView.backgroundColor = color;
		*/
	}
	else
	{
		result = false;
	}

	return result;
}
    
    
int
MacDisplayObject::setReturnKey( lua_State *L )
{
    return 0;
}

// TODO: This code won't compile b/c @try depends on C++ exceptions being enabled
// which is a pretty big change that we're not prepared to make right now.
//
// When we move this stuff into a plugin, we can explore whether this is
// feasible to isolate to the plugin itself.
//
// NOTE: Must add #import "CoronaLuaObjC+NSObject.h" in order for this code to work.
#define Rtt_NATIVE_PROPERTIES_MAC 0

#if Rtt_NATIVE_PROPERTIES_MAC
id
MacDisplayObject::GetNativeTarget() const
{
	return GetView();
}

int
MacDisplayObject::GetNativeProperty( lua_State *L, const char key[] ) const
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
MacDisplayObject::SetNativeProperty( lua_State *L, const char key[], int valueIndex )
{
	id target = GetNativeTarget();
	bool result = [target set:L luaValue:valueIndex forKey:@(key)];

	if ( ! result )
	{
		result = Super::SetNativeProperty( L, key, valueIndex );
	}

	return result;
}
#endif // Rtt_NATIVE_PROPERTIES_MAC

// ----------------------------------------------------------------------------

NSColor*
MacTextObject::GetTextColor( lua_State *L, int index, bool isByteColorRange )
{
	Color c = LuaLibDisplay::toColor(L,index,isByteColorRange);
	RGBA rgba = ( (ColorUnion*)(& c) )->rgba;
	
	CGFloat r = (CGFloat)rgba.r / 255.0f;
	CGFloat g = (CGFloat)rgba.g / 255.0f;
	CGFloat b = (CGFloat)rgba.b / 255.0f;
	CGFloat a = (CGFloat)rgba.a / 255.0f;

	return [NSColor colorWithDeviceRed:r green:g blue:b alpha:a];
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

