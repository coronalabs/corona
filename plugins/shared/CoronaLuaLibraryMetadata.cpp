//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "CoronaLuaLibraryMetadata.h"

#include "CoronaAssert.h"

#include <string.h>

// ----------------------------------------------------------------------------

namespace Corona
{

// ----------------------------------------------------------------------------

static char *
Copy( const char *src )
{
	char *result = NULL;

	if ( src )
	{
		size_t len = strlen( src );
		result = new char[len + 1];
		strncpy( result, src, len );
		result[len] = '\0';
	}

	return result;
}

// ----------------------------------------------------------------------------

LuaLibraryMetadata::LuaLibraryMetadata( const char *name, const char *publisherId, int version, int revision )
:	fName( Copy( name ) ),
	fPublisherId( Copy( publisherId ) ),
	fVersion( version ),
	fRevision( revision )
{
}

LuaLibraryMetadata::~LuaLibraryMetadata()
{
	delete [] fPublisherId;
	delete [] fName;
}

// ----------------------------------------------------------------------------

} // namespace Corona

// ----------------------------------------------------------------------------

