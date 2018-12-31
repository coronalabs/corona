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

#include "Rtt_AndroidDisplayObject.h"
#include "NativeToJavaBridge.h"
#include "AndroidDisplayObjectRegistry.h"
#include "Rtt_Lua.h"
#include "Rtt_Runtime.h"
#include "Rtt_RenderingStream.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

AndroidDisplayObject::AndroidDisplayObject(
		const Rect& bounds, AndroidDisplayObjectRegistry *displayObjectRegistry, NativeToJavaBridge *ntjb )
:	PlatformDisplayObject(),
	fSelfBounds( bounds),
	fView( NULL ),
	fDisplayObjectRegistry( displayObjectRegistry ),
	fId( AndroidDisplayObjectRegistry::INVALID_ID ),
	fNativeToJavaBridge(ntjb)
{
	// Fetch a unique ID for this display object from the registry.
	if (fDisplayObjectRegistry)
	{
		fId = fDisplayObjectRegistry->Register(this);
	}

	// Convert the given x/y coordinates from a top-left reference point to a center reference point.
	Translate(bounds.xMin + Rtt_RealDiv2(bounds.Width()), bounds.yMin + Rtt_RealDiv2(bounds.Height()));
	fSelfBounds.MoveCenterToOrigin();
}

AndroidDisplayObject::~AndroidDisplayObject()
{
	// Have the Java display object destroyed first.
	if (fId != AndroidDisplayObjectRegistry::INVALID_ID)
	{
		fNativeToJavaBridge->DisplayObjectDestroy(fId);
	}
	
	// Remove this display object from the registry.
	if (fDisplayObjectRegistry)
	{
		fDisplayObjectRegistry->Unregister(fId);
	}
}

void
AndroidDisplayObject::InitializeView( void * view )
{
	Rtt_ASSERT( ! fView );
	Rtt_ASSERT( view );

	fView = view;
}

void
AndroidDisplayObject::SetFocus()
{
	fNativeToJavaBridge->DisplayObjectSetFocus( fId, true );
}

void
AndroidDisplayObject::DidMoveOffscreen()
{
}

void
AndroidDisplayObject::WillMoveOnscreen()
{
}

bool
AndroidDisplayObject::CanCull() const
{
	// Disable culling for all native display objects.
	// Note: This is needed so that the Build() function will get called when a native object
	//       is being moved partially or completely offscreen.
	return false;
}

void
AndroidDisplayObject::Prepare( const Display& display )
{
	Super::Prepare( display );

	if ( ShouldPrepare() )
	{
		// First, update this object's cached scale factors in case the app window has been resized.
		Rtt::Runtime *runtimePointer = fNativeToJavaBridge->GetRuntime();
		Preinitialize(runtimePointer->GetDisplay());

		// Update the native object's screen bounds.
		Rect bounds;
		GetScreenBounds(bounds);
		fNativeToJavaBridge->DisplayObjectUpdateScreenBounds(
				fId, bounds.xMin, bounds.yMin, bounds.Width(), bounds.Height());
	}
}

void
AndroidDisplayObject::Draw( Renderer& renderer ) const
{
}

void
AndroidDisplayObject::GetSelfBounds( Rect& rect ) const
{
	rect = fSelfBounds;
}

void
AndroidDisplayObject::SetSelfBounds( Real width, Real height )
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

bool
AndroidDisplayObject::HasBackground() const
{
	return fNativeToJavaBridge->DisplayObjectGetBackground(fId);
}

void
AndroidDisplayObject::SetBackgroundVisible(bool isVisible)
{
	fNativeToJavaBridge->DisplayObjectSetBackground(fId, isVisible);
}

int
AndroidDisplayObject::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;

	if ( strcmp( "isVisible", key ) == 0 )
	{
		bool visible = fNativeToJavaBridge->DisplayObjectGetVisible( fId );
		lua_pushboolean( L, visible );
	}
	else if ( strcmp( "alpha", key ) == 0 )
	{
		float alpha = fNativeToJavaBridge->DisplayObjectGetAlpha( fId );
		lua_pushnumber( L, alpha );
	}
	else if ( strcmp( "hasBackground", key ) == 0 )
	{
		lua_pushboolean( L, HasBackground() ? 1 : 0 );
	}
	else
	{
		result = 0;
	}

	return result;
}

bool
AndroidDisplayObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	if ( strcmp( "isVisible", key ) == 0 )
	{
		bool visible = lua_toboolean( L, valueIndex );
		fNativeToJavaBridge->DisplayObjectSetVisible( fId, visible );
	}
	else if ( strcmp( "alpha", key ) == 0 )
	{
		float alpha = lua_tonumber( L, valueIndex );
		fNativeToJavaBridge->DisplayObjectSetAlpha( fId, alpha );
	}
	else if ( strcmp( "hasBackground", key ) == 0 )
	{
		bool isVisible = lua_toboolean( L, valueIndex ) ? true : false;
		SetBackgroundVisible(isVisible);
	}
	else
	{
		result = false;
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

