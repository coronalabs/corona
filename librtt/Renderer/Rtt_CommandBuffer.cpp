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

#include "Renderer/Rtt_CommandBuffer.h"

#include "Core/Rtt_Allocator.h"
#include <stddef.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

CommandBuffer::CommandBuffer( Rtt_Allocator* allocator )
:	fAllocator( allocator ),
	fBuffer( NULL ), 
	fOffset( NULL ), 
	fNumCommands( 0 ), 
	fBytesAllocated( 0 ), 
	fBytesUsed( 0 )
{

}

CommandBuffer::~CommandBuffer()
{
    if (fBuffer != NULL)
    {
        delete [] fBuffer;
    }
}

const char *
CommandBuffer::GetNameForStencilFunc( S32 func )
{
	switch (func)
	{
	case 0:
		return "always";
	case 1:
		return "never";
	case 2:
		return "lessThan";
	case 3:
		return "lessThanOrEqual";
	case 4:
		return "greaterThan";
	case 5:
		return "greaterThanOrEqual";
	case 6:
		return "equal";
	case 7:
		return "notEqual";
	default:
		return NULL;
	}
}

S32
CommandBuffer::GetStencilFuncForName( const char *name )
{
	if (strcmp( name, "always" ) == 0)
	{
		return 0;
	}

	else if (strcmp( name, "never" ) == 0)
	{
		return 1;
	}

	else if (strcmp( name, "lessThan" ) == 0)
	{
		return 2;
	}

	else if (strcmp( name, "lessThanOrEqual" ) == 0)
	{
		return 3;
	}
	
	else if (strcmp( name, "greaterThan" ) == 0)
	{
		return 4;
	}
	
	else if (strcmp( name, "greaterThanOrEqual" ) == 0)
	{
		return 5;
	}
	
	else if (strcmp( name, "equal" ) == 0)
	{
		return 6;
	}
	
	else if (strcmp( name, "notEqual" ) == 0)
	{
		return 7;
	}

	else
	{
		return -1;
	}
}

const char *
CommandBuffer::GetNameForStencilOpAction( S32 action )
{
	switch (action)
	{
	case 0:
		return "keep";
	case 1:
		return "zero";
	case 2:
		return "replace";
	case 3:
		return "increment";
	case 4:
		return "incrementWrap";
	case 5:
		return "decrement";
	case 6:
		return "decrementWrap";
	case 7:
		return "invert";
	default:
		return NULL;
	}
}

S32
CommandBuffer::GetStencilOpActionForName( const char *name )
{
	if (strcmp( name, "keep" ) == 0)
	{
		return 0;
	}

	else if (strcmp( name, "zero" ) == 0)
	{
		return 1;
	}

	else if (strcmp( name, "replace" ) == 0)
	{
		return 2;
	}

	else if (strcmp( name, "increment" ) == 0)
	{
		return 3;
	}
	
	else if (strcmp( name, "incrementWrap" ) == 0)
	{
		return 4;
	}
	
	else if (strcmp( name, "decrement" ) == 0)
	{
		return 5;
	}
	
	else if (strcmp( name, "decrementWrap" ) == 0)
	{
		return 6;
	}
	
	else if (strcmp( name, "invert" ) == 0)
	{
		return 7;
	}

	else
	{
		return -1;
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
