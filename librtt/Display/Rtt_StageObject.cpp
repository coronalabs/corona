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

#include "Display/Rtt_StageObject.h"

#include "Display/Rtt_Display.h"
#include "Display/Rtt_Scene.h"
#include "Rtt_Matrix.h"
#include "Rtt_RenderingStream.h"
#include "Rtt_Runtime.h"
#include "Rtt_LuaProxyVTable.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

StageObject::StageObject( Rtt_Allocator* pAllocator, Scene& owner )
:	Super( pAllocator, this ),
	fOwner( owner ),
	fDisplay( owner.GetDisplay() ),
	fNext( NULL ),
	fFocus( NULL ),
	fFocusObjects( pAllocator ),
	fSnapshotBounds( NULL )
{
    SetObjectDesc("StageObject"); // for introspection
}

StageObject::~StageObject()
{
	if ( fNext ) { Rtt_DELETE( fNext ); }
}

void
StageObject::GetSelfBounds( Rect& rect ) const
{
	// Stage bounds are always the window bounds
	fDisplay.GetContentRect( rect );
}

bool
StageObject::ShouldOffsetWithAnchor() const
{
	return ( IsV1Compatibility() || Super::ShouldOffsetWithAnchor() );
}

void
StageObject::InvalidateDisplay()
{
	fOwner.Invalidate();
}

const LuaProxyVTable&
StageObject::ProxyVTable() const
{
	return LuaStageObjectProxyVTable::Constant();
}

void
StageObject::SetV1Compatibility( bool newValue )
{
	Super::SetV1Compatibility( newValue );

	if ( newValue )
	{
		SetAnchorX( 0.f );
		SetAnchorY( 0.f );
	}
}

bool
StageObject::IsOnscreen() const
{
	return & GetScene().CurrentStage() == this;
}

void
StageObject::SetNext( StageObject* pNext )
{
	Self* oldNext = fNext;
	fNext = pNext;
	if ( pNext )
	{
		pNext->fNext = oldNext;
	}
}

// Pass non-NULL newValue and non-NULL focusId to add/update per-object focus.
// Pass non-NULL newValue and NULL focusId to remove focus via searching by display object.
// Pass NULL newValue and non-NULL focusId to remove focus via searching by focus id.
void
StageObject::SetFocus( DisplayObject *newValue, const void *focusId )
{
	bool found = false;

	// Cache value outside loop
	bool shouldRemove = ( ! newValue && focusId );

	// See if newValue is already in array
	const LightPtrArray< DisplayObject >& focusObjects = fFocusObjects;
	for ( int i = 0, iMax = fFocusObjects.Length(); i < iMax && ! found; i++ )
	{
		DisplayObject *o = focusObjects[i]; Rtt_ASSERT( o );
		if ( o == newValue )
		{
			// In this case, newValue was non-NULL, so search by display object.
			if ( focusId )
			{
				// A non-NULL focusId means update since an older value is already in the array
				newValue->SetFocusId( focusId );
			}
			else
			{
				// A NULL focusId means remove since NULL indicates no per-object focus
				fFocusObjects.Remove( i, 1, false );
				o->SetFocusId( NULL );
			}

			found = true;
		}
		else if ( shouldRemove && o->GetFocusId() == focusId )
		{
			// In this case, newValue was NULL, but we passed a non-NULL focusId
			// which means we search by focusId and remove it from the array
			fFocusObjects.Remove( i, 1, false );
			o->SetFocusId( NULL );
			found = true;
		}
	}

	// Not in array, so add newValue/focusId pair provided both are non-NULL.
	if ( ! found && newValue && focusId )
	{
		fFocusObjects.Append( newValue );
		newValue->SetFocusId( focusId );
	}
}

DisplayObject*
StageObject::GetFocus( const void *focusId )
{
	DisplayObject* result = NULL;

	if ( focusId )
	{
		LightPtrArray< DisplayObject >& focusObjects = fFocusObjects;
		for ( int i = 0, iMax = fFocusObjects.Length(); i < iMax; i++ )
		{
			DisplayObject *item = focusObjects[i];
			const void * const itemId = item->GetFocusId();
			if ( ! itemId )
			{
				// Object id was set to NULL for deferred removal, so remove it
				focusObjects.Remove( i, 1, false );
			}
			else if ( itemId == focusId )
			{
				result = item;
			}
		}
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

