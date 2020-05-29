//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "CoronaVersion.h"

#include "Core/Rtt_Version.h"

// ----------------------------------------------------------------------------

CORONA_API
const char *CoronaVersionBuildString()
{
	static const char sString[] = Rtt_STRING_BUILD;
	return  sString;
}

// ----------------------------------------------------------------------------

