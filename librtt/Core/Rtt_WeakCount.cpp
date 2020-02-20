//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Core/Rtt_WeakCount.h"
#include "Core/Rtt_SharedCount.h"

#include <stdio.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

WeakCount::WeakCount()
:	fCount( NULL )
{
}

WeakCount::WeakCount( const WeakCount& rhs )
:	fCount( rhs.fCount )
{
	if ( fCount )
	{
		fCount->RetainWeak();
	}
}

WeakCount::WeakCount( const SharedCount& rhs )
:	fCount( rhs.fCount )
{
	if ( fCount )
	{
		fCount->RetainWeak();
	}
}

WeakCount::~WeakCount()
{
	if ( fCount )
	{
		fCount->ReleaseWeak();
	}
}

WeakCount&
WeakCount::Assign( UseCount *newCount )
{
	if ( newCount != fCount )
	{
		// Increment new count
		if ( newCount ) { newCount->RetainWeak(); }

		// Decrement old count
		if ( fCount ) { fCount->ReleaseWeak(); }

		fCount = newCount;
	}

	return *this;
}

WeakCount&
WeakCount::operator=( const WeakCount& rhs )
{
	UseCount *newCount = rhs.fCount;

	return Assign( newCount );
}

WeakCount&
WeakCount::operator=( const SharedCount& rhs )
{
	UseCount *newCount = rhs.fCount;

	return Assign( newCount );
}

bool
WeakCount::IsValid() const
{
	return ( fCount ? fCount->IsValid() : false );
}

void
WeakCount::Log() const
{
	if ( fCount )
	{
		fCount->Log();
	}
	else
	{
		printf( "\t[WeakCount] UseCount is NULL." );
	}
}

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------
