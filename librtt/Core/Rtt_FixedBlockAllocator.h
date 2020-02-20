//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Rtt_FixedBlockAllocator_H
#define Rtt_FixedBlockAllocator_H

#include "Core/Rtt_Types.h"
#include "Core/Rtt_Allocator.h"

// TODO: Move MemoryPage::Free() into .cpp file so we can get rid of this include
#ifdef Rtt_ANDROID_ENV
#include <malloc.h>
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{
	
// ----------------------------------------------------------------------------

namespace MemoryPage
{

Rtt_INLINE static void
Free( void* p )
{
	free( p );
}

} // MemoryPage

// ----------------------------------------------------------------------------

class FixedBlock
{
	Rtt_CLASS_NO_COPIES( FixedBlock )

	public:
		static FixedBlock* Create( size_t blockSize, FixedBlock* pNext );
		Rtt_INLINE static void Destroy( FixedBlock* p )
		{
			Rtt_ASSERT( p->fThis == p );
			
			MemoryPage::Free( p );
		}

	private:
		FixedBlock( size_t blockSize, FixedBlock* pNext );

	public:
		void* Alloc();
		static void Free( void* p );

		FixedBlock* Next() { return fNext; }
		void SetNext( FixedBlock* pNext ) { fNext = pNext; }

	protected:
		void FreeObject( void* p );

	protected:
		void* fBumpPtr;
		const size_t fBlockSize;
		const void* const fBumpPtrEnd;
		void* fFreeList;
		FixedBlock* fThis;
		FixedBlock* fNext;
		U32 fMemory[1];
};

// ----------------------------------------------------------------------------

class FixedBlockAllocator
{
	Rtt_CLASS_NO_COPIES( FixedBlockAllocator )

	public:
		FixedBlockAllocator( size_t blockSize );
		~FixedBlockAllocator();

	public:
		void* Alloc();
		Rtt_INLINE static void Free( void* p ) { FixedBlock::Free( p ); }

	protected:
		FixedBlock* fHead;
		const size_t fBlockSize;

	protected:
		static const size_t kMaxBlockSize;
};

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_FixedBlockAllocator_H

