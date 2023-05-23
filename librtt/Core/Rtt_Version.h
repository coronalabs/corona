//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_Version_H__
#define __Rtt_Version_H__

// ----------------------------------------------------------------------------

#include "Core/Rtt_Macros.h"

#define Rtt_VERSION_MAJOR	3
#define Rtt_VERSION_MINOR	0
#define Rtt_VERSION_REVISION	0

#define Rtt_LOCAL_BUILD_REVISION 9999

#ifndef Rtt_BUILD_YEAR
#define Rtt_BUILD_YEAR 2100
#endif

#define Rtt_BUILD_MONTH 1
#define Rtt_BUILD_DAY 1

#ifndef Rtt_BUILD_REVISION
#define Rtt_BUILD_REVISION Rtt_LOCAL_BUILD_REVISION
#endif

// If this is a public release, this should be 0
#define Rtt_IS_DAILYBUILD	1

// Used to determine if this is an internal/locally built version of Corona.
// This will never be defined for customer builds.
// #if Rtt_BUILD_REVISION == Rtt_LOCAL_BUILD_REVISION
#define Rtt_IS_LOCAL_BUILD
// #endif

#define Rtt_EXPIRATION_CUTOFF_INDIES_SIMULATOR_SKINS 1353196800

// 2.1.0
#define Rtt_STRING_VERSION	Rtt_MACRO_TO_STRING( Rtt_VERSION_MAJOR ) "." Rtt_MACRO_TO_STRING( Rtt_VERSION_MINOR ) "." Rtt_MACRO_TO_STRING( Rtt_VERSION_REVISION )

// 2010.1234
#define Rtt_STRING_BUILD	Rtt_MACRO_TO_STRING( Rtt_BUILD_YEAR ) "." Rtt_MACRO_TO_STRING( Rtt_BUILD_REVISION )

// 2010.9.26
#define Rtt_STRING_BUILD_DATE Rtt_MACRO_TO_STRING( Rtt_BUILD_YEAR ) "." Rtt_MACRO_TO_STRING( Rtt_BUILD_MONTH ) "." Rtt_MACRO_TO_STRING( Rtt_BUILD_DAY ) 

#define Rtt_STRING_COPYRIGHT	"Copyright (C) 2009-" Rtt_MACRO_TO_STRING( Rtt_BUILD_YEAR ) "  C o r o n a   L a b s   I n c ."

#define Rtt_STRING_CREDITS		"Walter Luh | Perry Clarke, Alex Frangeti, Sean Head, Ajay McCaleb, Tom Newman, Josh Quick, Vlad Shcherban, Michael Wallar"

// Per Gilbert, removing this string from the welcome screen.
#define Rtt_STRING_HUMAN_FRIENDLY_VERSION	""

// ----------------------------------------------------------------------------

#endif // __Rtt_Version_H__
