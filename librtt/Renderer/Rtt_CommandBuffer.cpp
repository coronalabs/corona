//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
