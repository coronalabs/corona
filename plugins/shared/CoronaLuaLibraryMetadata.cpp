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

