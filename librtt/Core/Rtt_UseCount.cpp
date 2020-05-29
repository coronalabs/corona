//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Core/Rtt_UseCount.h"

#include <stdio.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

UseCount::UseCount()
:	fStrongCount( 1 ),
	fWeakCount( 1 )
{
}

UseCount::~UseCount()
{
	Rtt_ASSERT( 0 == fStrongCount );
	Rtt_ASSERT( 0 == fWeakCount );
}

void
UseCount::FinalizeWeak()
{
	delete this;
}

void
UseCount::ReleaseStrong()
{
	if ( --fStrongCount == 0 )
	{
		FinalizeStrong();
		ReleaseWeak();
	}
}

void
UseCount::ReleaseWeak()
{
	if ( --fWeakCount == 0 )
	{
		FinalizeWeak();
	}
}

void
UseCount::Log() const
{
	printf( "\t[%p]\tstrong(%d)\tweak(%d)", this, GetStrongCount(), GetWeakCount() );
}

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------
