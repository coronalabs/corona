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

