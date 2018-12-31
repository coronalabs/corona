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
#include "Core/Rtt_String.h"

#include "Rtt_Resource.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

Resource::Resource()
:	fCount( 0 )
{
}

Resource::~Resource()
{
	Rtt_ASSERT( 0 == fCount );
}

void
Resource::Release()
{
	Rtt_ASSERT( fCount > 0 );
	if ( 0 >= --fCount )
	{
		Rtt_DELETE( this );
	}
}

// ----------------------------------------------------------------------------

CachedResource::CachedResource( MCachedResourceLibrary& library, const char* key )
:	Super(),
	fLibrary( library ),
	fKey( library.GetAllocator() ),
	fNext( this ), // Circular linked list
	fPrev( this ) // Circular linked list
{
	// Need to completely init this instance before adding to library
	RegisterResource( key );
}

CachedResource::~CachedResource()
{
	Remove();

	if ( fKey.GetString() )
	{
		fLibrary.SetResource( NULL, fKey.GetString() );
	}
}

void
CachedResource::RegisterResource( const char* key )
{
	if ( Rtt_VERIFY( ! fKey.GetString() ) && key )
	{
		fLibrary.SetResource( this, key );
		fKey.Set( key );
	}
}

void
CachedResource::Remove()
{
	Self* n = fNext; Rtt_ASSERT( n );
	Self* p = fPrev; Rtt_ASSERT( p );

	// When next and prev points to this, the node is by itself
	// Hence, we assert that if next does not point to this, 
	// then prev must not point to this either
	if ( n != this && Rtt_VERIFY( p != this ) )
	{
		// Remove node from circular linked list
		p->fNext = n;
		n->fPrev = p;

		// Restore node to initial state since it's been removed
		fPrev = this;
		fNext = this;
	}
}

void
CachedResource::Append( Self* newValue )
{
	Rtt_ASSERT( newValue );

	newValue->Remove();

	Self* oldNext = fNext;
	newValue->fNext = oldNext;
	newValue->fPrev = this;
	oldNext->fPrev = newValue;
	fNext = newValue;
}

void
CachedResource::Prepend( Self* newValue )
{
	Rtt_ASSERT( newValue );

	newValue->Remove();

	Self* oldPrev = fPrev;
	newValue->fNext = this;
	newValue->fPrev = oldPrev;
	oldPrev->fNext = newValue;
	fPrev = newValue;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

