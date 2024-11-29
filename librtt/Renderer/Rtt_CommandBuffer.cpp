//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_CommandBuffer.h"
#include "Display/Rtt_ShaderResource.h"

#include "Core/Rtt_Allocator.h"
#include <stddef.h>

#include "../Core/Rtt_Math.h"

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
	fBytesUsed( 0 ),
	fTimeTransform( NULL ),
    fLastTimeTransform( NULL ),
    fUsesTime( false ),
	fDefaultTimeTransform( Rtt_NEW( allocator, TimeTransform ) )
{

}

CommandBuffer::~CommandBuffer()
{
    if (fBuffer != NULL)
    {
        delete [] fBuffer;
    }

	Rtt_DELETE( fDefaultTimeTransform );
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
 
void
CommandBuffer::PrepareTimeTransforms( const TimeTransform* transform )
{
	*fDefaultTimeTransform = *transform;

	fTimeTransform = NULL;
	fLastTimeTransform = NULL;
	fUsesTime = false;
}

void
CommandBuffer::AcquireTimeTransform( ShaderResource* resource )
{
    fUsesTime = resource->UsesTime();

    if (fUsesTime)
    {
        fLastTimeTransform = fTimeTransform;
        fTimeTransform = resource->GetTimeTransform();

		if ( NULL == fTimeTransform && NULL != fDefaultTimeTransform->func )
		{
			fTimeTransform = fDefaultTimeTransform;
		}
    }
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
