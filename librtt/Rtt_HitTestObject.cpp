//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_HitTestObject.h"
#include "Display/Rtt_DisplayObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

HitTestObject::HitTestObject( DisplayObject& target, HitTestObject* parent )
:	fTarget( target ),
	fParent( parent ),
	fChild( NULL ),
	fSibling( NULL ),
	fNumChildren( 0 )
{
	target.SetUsedByHitTest( true );
}

HitTestObject::~HitTestObject()
{
	for ( HitTestObject* iCurrent = fChild, *iNext = NULL;
		  iCurrent;
		  iCurrent = iNext )
	{
		iNext = iCurrent->fSibling;
		Rtt_DELETE( iCurrent );
	}

	fTarget.SetUsedByHitTest( false );
}

/*
void
HitTestObject::Append( DisplayObject& child )
{
	Rtt_Allocator* pAllocator = fHitChildren.Allocator();
	HitTestObject* newChild = Rtt_NEW( pAllocator, HitTestObject( child, this, pAllocator ) );
	fHitChildren.Append( newChild );
}
*/

void
HitTestObject::Prepend( HitTestObject* child )
{
	Rtt_ASSERT( child );

	// make child new 'head' of list
	child->fSibling = fChild;
	fChild = child;

	++fNumChildren;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

