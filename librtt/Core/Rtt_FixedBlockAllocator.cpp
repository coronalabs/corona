//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Core/Rtt_FixedBlockAllocator.h"
#include "Core/Rtt_New.h"

#if defined( Rtt_MAC_ENV ) || defined( Rtt_IPHONE_ENV ) || defined( Rtt_TVOS_ENV )
#include <stdlib.h>
#include <malloc/malloc.h>
#elif defined( Rtt_ANDROID_ENV )
#include <sys/sysconf.h>
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// MemoryPage constants and functions
// ----------------------------------------------------------------------------

namespace MemoryPage
{

const size_t kSize = 1 << 12;	// 4KB
// #ifdef Rtt_USE_64BIT
const size_t kAlignmentMask = ~kSize & ( kSize - 1 );

Rtt_INLINE static void*
Alloc( )
{
	void* result = NULL;

	#if defined( Rtt_MAC_ENV ) || defined( Rtt_IPHONE_ENV ) || defined( Rtt_TVOS_ENV )
		result = valloc( kSize );
		Rtt_ASSERT( kSize == malloc_size( result ) );
	#elif defined( Rtt_SYMBIAN_ENV )
		Rtt_STATIC_ASSERT( false );

		// Need a way to manage RChunk's!
		static TInt sPageSize = 0;
		static RChunk* sChunk = NULL;
		if ( ! sPageSize )
		{
			HAL::Get( HAL::EMemoryPageSize, sPageSize );
			Rtt_ASSERT( pageSize == kSize );

			sChunk = new RChunk;

			// Max of 8 pages
			sChunk->CreateLocal( sPageSize, sPageSize << 3 );
		}
		result = pChunk->Base();
	#elif defined( Rtt_ANDROID_ENV )
		result = memalign( sysconf( _SC_PAGESIZE ), kSize );
	#elif defined( Rtt_WEBOS_ENV ) || (defined( Rtt_EMSCRIPTEN_ENV ) && !defined(WIN32))
		result = valloc( kSize );
	#elif defined( Rtt_WIN_ENV ) || defined( Rtt_POWERVR_ENV ) || defined( Rtt_NINTENDO_ENV )
		result = malloc( kSize );
		// TODO: Need to return page-aligned memory block or else we'll crash
		Rtt_ASSERT_NOT_IMPLEMENTED();
		// result = _aligned_malloc() or VirtualAlloc()
	#else
		Rtt_STATIC_ASSERT( false );
		Rtt_ASSERT_NOT_IMPLEMENTED();
	#endif

	Rtt_ASSERT( result );

	return result;
}

} // MemoryPage


// ----------------------------------------------------------------------------

FixedBlock*
FixedBlock::Create( size_t blockSize, FixedBlock* pNext )
{
	FixedBlock* result = NULL;
	void* p = MemoryPage::Alloc();

	if ( p )
	{
		result = new( p ) FixedBlock( blockSize, pNext );
	}

	return result;
}

FixedBlock::FixedBlock( size_t blockSize, FixedBlock* pNext )
:	fBlockSize( blockSize & 0xFFFFFFFC ),	// 4 byte alignment
	fBumpPtrEnd( ((U8*)this) + MemoryPage::kSize ),
	fFreeList( NULL ),
	fThis( this ),
	fNext( pNext )
{
	fBumpPtr = fMemory;
	// Rtt_USE_64BIT
	Rtt_ASSERT( 0 == (((size_t)this) & MemoryPage::kAlignmentMask) );
	Rtt_ASSERT( fBumpPtr );
}

void*
FixedBlock::Alloc()
{
	void* result = NULL;

	if ( fBumpPtr )
	{
		result = fBumpPtr;

		void* pNext = ((U8*)result) + fBlockSize;
		fBumpPtr = ( pNext >= fBumpPtrEnd ? pNext : NULL );
	}
	else if ( fFreeList )
	{
		result = fFreeList;

		// The contents of fFreeList stores the next ptr in the free list
		fFreeList = *((void**)result); 
	}

	return result;
}

void
FixedBlock::Free( void* p )
{
	Rtt_STATIC_ASSERT( sizeof(MemoryPage::kAlignmentMask) == sizeof(p) );

	((FixedBlock*)(((U64)p) & ~MemoryPage::kAlignmentMask))->fThis->FreeObject( p );
}

void
FixedBlock::FreeObject( void* p )
{
	Rtt_ASSERT( fBlockSize >= sizeof( p ) );

	if ( fFreeList )
	{
		// The contents of p stores the next ptr in the free list
		*((void**)p) = fFreeList;
	}

	fFreeList = p;
}

// ----------------------------------------------------------------------------

const size_t FixedBlockAllocator::kMaxBlockSize = 32;

FixedBlockAllocator::FixedBlockAllocator( size_t blockSize )
:	fHead( FixedBlock::Create( blockSize, NULL ) ),
	fBlockSize( blockSize )
{
	Rtt_ASSERT( fHead );
	Rtt_ASSERT( fBlockSize <= kMaxBlockSize );
}

FixedBlockAllocator::~FixedBlockAllocator()
{
	Rtt_ASSERT( fHead );

	for ( FixedBlock *iCurrent = fHead, *iNext;
		  iCurrent;
		  iCurrent = iNext )
	{
		iNext = iCurrent->Next();

		FixedBlock::Destroy( iCurrent );
	}
}

void*
FixedBlockAllocator::Alloc()
{
	void* result = NULL;

	for ( FixedBlock *iCurrent = fHead, *iPrevious = NULL;
		  iCurrent;
		  iPrevious = iCurrent, iCurrent = iCurrent->Next() )
	{
		result = iCurrent->Alloc();

		if ( result )
		{
			// If we didn't allocate from head, swap head with iCurrent
			if ( iPrevious )
			{
				FixedBlock* iNext = iCurrent->Next();
				iPrevious->SetNext( fHead );
				iCurrent->SetNext( fHead->Next() );
				fHead->SetNext( iNext );
			}
			
			goto exit_gracefully;
		}
	}

	// Prevent goto from crossing initialization:
	{
		// No free mem in blocks
		FixedBlock* pBlock = FixedBlock::Create( fBlockSize, fHead );
		if ( Rtt_VERIFY( pBlock ) )
		{
			result = pBlock->Alloc(); Rtt_ASSERT( result );
			fHead = pBlock;
		}
	}
	
exit_gracefully:
	return result;
}

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------
