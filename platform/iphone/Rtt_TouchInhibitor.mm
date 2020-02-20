//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "Rtt_TouchInhibitor.h"

#import "CoronaViewPrivate.h"

// ----------------------------------------------------------------------------

namespace Rtt
{
	
class MCallback;

// ----------------------------------------------------------------------------

TouchInhibitor::TouchInhibitor( CoronaView *view )
:	fView( view )
{
	Rtt_ASSERT( fView.inhibitCount >= 0 );
	++fView.inhibitCount;
}

TouchInhibitor::~TouchInhibitor()
{
	Rtt_ASSERT( fView.inhibitCount > 0 );
	--fView.inhibitCount;
}

// ----------------------------------------------------------------------------
	
} // Rtt

// ----------------------------------------------------------------------------
