//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Core/Rtt_SharedCount.h"
#include "Core/Rtt_WeakCount.h"

#include <stdio.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

SharedCount::SharedCount()
:	fCount( NULL )
{
}

SharedCount::SharedCount( const SharedCount& rhs )
:	fCount( rhs.fCount )
{
	if ( fCount )
	{
		fCount->RetainStrong();
	}
}

SharedCount::SharedCount( const WeakCount& rhs )
:	fCount( rhs.fCount )
{
	if ( fCount )
	{
		fCount->RetainStrong();
	}
}

SharedCount::~SharedCount()
{
	if ( fCount )
	{
		fCount->ReleaseStrong();
	}
}

SharedCount&
SharedCount::Assign( UseCount *newCount )
{
	if ( newCount != fCount )
	{
		// Increment new count
		if ( newCount ) { newCount->RetainStrong(); }

		// Decrement old count
		if ( fCount ) { fCount->ReleaseStrong(); }

		fCount = newCount;
	}

	return *this;
}

SharedCount&
SharedCount::operator=( const SharedCount& rhs )
{
	UseCount *newCount = rhs.fCount;

	return Assign( newCount );
}

SharedCount&
SharedCount::operator=( const WeakCount& rhs )
{
	UseCount *newCount = rhs.fCount;

	return Assign( newCount );
}

bool
SharedCount::IsValid() const
{
	return ( fCount ? fCount->IsValid() : false );
}

void
SharedCount::Log() const
{
	if ( fCount )
	{
		fCount->Log();
	}
	else
	{
		printf( "\t[SharedCount] UseCount is NULL." );
	}
}

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------
