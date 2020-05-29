//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

