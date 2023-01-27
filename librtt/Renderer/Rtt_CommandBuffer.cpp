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

#include "Rtt_Math.h"

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

void
CommandBuffer::ReadBytes( void * value, size_t size )
{
	Rtt_ASSERT( fOffset < fBuffer + fBytesAllocated );
	memcpy( value, fOffset, size );
	fOffset += size;
}

void
CommandBuffer::WriteBytes( const void * value, size_t size )
{
	U32 bytesNeeded = fBytesUsed + size;
	if( bytesNeeded > fBytesAllocated )
	{
		U32 doubleSize = fBytesUsed ? 2 * fBytesUsed : 4;
		U32 newSize = Max( bytesNeeded, doubleSize );
		U8* newBuffer = new U8[newSize];

		memcpy( newBuffer, fBuffer, fBytesUsed );
		delete [] fBuffer;

		fBuffer = newBuffer;
		fBytesAllocated = newSize;
	}

	memcpy( fBuffer + fBytesUsed, value, size );
	fBytesUsed += size;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
