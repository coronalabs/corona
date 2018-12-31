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

