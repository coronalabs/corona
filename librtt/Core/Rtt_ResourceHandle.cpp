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

#include "Core/Rtt_ResourceHandle.h"

#include "Core/Rtt_RefCount.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

BaseResourceHandle::BaseResourceHandle()
:	fCount( NULL )
{
}

BaseResourceHandle::BaseResourceHandle( const Self& rhs )
{
	Retain( rhs.fCount );
}
		   
BaseResourceHandle::~BaseResourceHandle()
{
	Release();
}

BaseResourceHandle&
BaseResourceHandle::operator=( const Self& rhs )
{
	if ( & rhs != this )
	{
		Release();
		Retain( rhs.fCount );
	}

	return * this;
}

void
BaseResourceHandle::Initialize( Rtt_Allocator* pAllocator )
{
	Rtt_ASSERT( pAllocator );

	Retain( Rtt_AllocatorAllocRefCount( pAllocator ) );
}

// Bottom (least significant) 8 bits are reserved. Top 24 bits used for ref count.
static const int kNumReservedBits = 8;
static const RefCount kIncrement = 1 << kNumReservedBits;

bool
BaseResourceHandle::IsValid() const
{
	Rtt_ASSERT( fCount );

	// Valid only if bottom 8 bits must be zero
	return (fCount && (0 == ((*fCount) & 0xFF)));
}

void
BaseResourceHandle::Invalidate()
{
	// Ensure fCount is valid and > 0
	Rtt_ASSERT( IsValid() && (*fCount >> kNumReservedBits) > 0 );

	// Turn on bottom 8 bits 
	*fCount = (*fCount) | 0xFF;
}

void
BaseResourceHandle::Retain( RefCount *pCount )
{
	fCount = pCount;
	if ( pCount )
	{
		// ++(*pCount);
		*pCount = *pCount + kIncrement;
	}
}

void
BaseResourceHandle::Release()
{
	RefCount *pCount = fCount;
	if ( pCount )
	{
		// --(*pCount);
		RefCount oldCount = *pCount;
		RefCount c = (oldCount >> kNumReservedBits) - 1;
		if ( 0 >= c )
		{
			Rtt_AllocatorFreeRefCount( pCount );
		}
		else
		{
			*pCount = oldCount - kIncrement;
		}
	}
}

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

