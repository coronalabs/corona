//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
    SetObjectDesc(kStageObjectDesc/*"StageObject"*/); // for introspection
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
	const /*LightPtrArray< DisplayObject >*/Array<ObjectAndId>& focusObjects = fFocusObjects;
	for ( int i = 0, iMax = fFocusObjects.Length(); i < iMax && ! found; i++ )
	{
		DisplayObject *o = focusObjects[i].fObject; Rtt_ASSERT( o );
		const void* id = fFocusObjects[i].fId;
		if ( o == newValue )
		{
			// In this case, newValue was non-NULL, so search by display object.
			if ( focusId )
			{
				// A non-NULL focusId means update since an older value is already in the array
				fFocusObjects[i].fId = focusId;//newValue->SetFocusId( focusId );
				o->SetHasFocusId( true );
			}
			else
			{
				// A NULL focusId means remove since NULL indicates no per-object focus
				fFocusObjects.Remove( i, 1, false );
				o->SetHasFocusId( false );//o->SetFocusId( NULL );
			}

			found = true;
		}
		else if ( shouldRemove && id/*o->GetFocusId()*/ == focusId )
		{
			// In this case, newValue was NULL, but we passed a non-NULL focusId
			// which means we search by focusId and remove it from the array
			fFocusObjects.Remove( i, 1, false );
			o->SetHasFocusId( false );//o->SetFocusId( NULL );
			found = true;
		}
	}

	// Not in array, so add newValue/focusId pair provided both are non-NULL.
	if ( ! found && newValue && focusId )
	{
		ObjectAndId pair;
	
		pair.fObject = newValue;
		pair.fId = focusId;
		
		fFocusObjects.Append( pair );
	
	//	fFocusObjects.Append( newValue );
		newValue->SetHasFocusId( true ); // newValue->SetFocusId( focusId );
	}
}

DisplayObject*
StageObject::GetFocus( const void *focusId )
{
	DisplayObject* result = NULL;

	if ( focusId )
	{
		/*LightPtrArray< DisplayObject >*/Array<ObjectAndId>& focusObjects = fFocusObjects;
	//	for ( int i = 0, iMax = fFocusObjects.Length(); i < iMax; i++ ) // ERROR: bug in Remove(), since we keep walking
		for ( int i = fFocusObjects.Length() - 1; i >= 0; i-- )
		{
		//	DisplayObject *item = focusObjects[i].fObject;
			const void * const itemId = fFocusObjects[i].fId;//item->GetFocusId();
			if ( ! itemId )
			{
				Rtt_ASSERT( fFocusObjects[i].fObject );
				Rtt_ASSERT( !fFocusObjects[i].fObject->GetHasFocusId() );
				
				// Object id was set to NULL for deferred removal, so remove it
				focusObjects.Remove( i, 1, false );

			}
			else if ( itemId == focusId )
			{
				result = focusObjects[i].fObject;//item;
			}
		}
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

