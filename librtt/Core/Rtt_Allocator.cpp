//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Core/Rtt_Allocator.h"

#ifdef Rtt_OPTIMIZE_REFCOUNT
#include "Core/Rtt_FixedBlockAllocator.h"
#endif
#ifdef Rtt_ANDROID_ENV
#include <string.h>
#endif

// ----------------------------------------------------------------------------


struct Rtt_Allocator
{
	Rtt_CLASS_NO_COPIES( Rtt_Allocator )

	public:
		Rtt_Allocator();

	public:
		void* Userdata() const { return fUserdata; }
		void SetUserdata( void* userdata ) { fUserdata = userdata; }

#ifdef Rtt_USE_GLOBAL_VARIABLES
public:
	static Rtt_Allocator* GetDefault() { return sAllocator; }
	static void SetDefault( Rtt_Allocator* p ) { sAllocator = p; }
	static S32& RefCount();
private:
	static Rtt_Allocator* sAllocator;
#endif

#ifdef Rtt_OPTIMIZE_REFCOUNT
	public:
		Rtt::RefCount* AllocRefCount( ) { return (Rtt::RefCount*)fRefCountAllocator.Alloc(); }
		static void FreeRefCount( Rtt::RefCount* p ) { Rtt::FixedBlockAllocator::Free( p ); }
	private:
		Rtt::FixedBlockAllocator fRefCountAllocator;
#endif

	private:
		void* fUserdata;
};

Rtt_Allocator::Rtt_Allocator()
:	fUserdata( NULL )
#ifdef Rtt_OPTIMIZE_REFCOUNT
	, fRefCountAllocator( sizeof( Rtt::RefCount ) )
#endif
{
}

#ifdef Rtt_USE_GLOBAL_VARIABLES
Rtt_Allocator* Rtt_Allocator::sAllocator = NULL;

S32&
Rtt_Allocator::RefCount()
{
	static S32 sRefCount = 0;
	Rtt_ASSERT( sRefCount >= 0 );

	return sRefCount;
}

#endif


// Global malloc-based allocation
// ----------------------------------------------------------------------------

#if defined( Rtt_ALLOCATOR_SYSTEM )

#if ! defined( ANDROID_NDK )
//void* operator new( size_t numBytes ) throw()
//{
//	return malloc( numBytes );
//}
//
//void operator delete( void* p ) throw()
//{
//	free( p );
//}
#endif // ! defined( ANDROID_NDK )

/*
void* operator new[]( size_t numBytes ) throw()
{
	return malloc( numBytes );
}

void operator delete[]( void* p )
{
	free( p );
}
*/

namespace Rtt
{

typedef Rtt_Allocator Allocator;
	
} // Rtt

// Placement-based allocation
// ----------------------------------------------------------------------------

#else // !defined( Rtt_ALLOCATOR_SYSTEM )

void* operator new( size_t numBytes, Rtt_Allocator* pContext ) throw()
{
	return Rtt_AllocatorAlloc( pContext, numBytes );
}

/*
void* operator new[]( size_t numBytes, Rtt_Allocator* pContext ) throw()
{
	return Rtt_AllocatorAlloc( pContext, numBytes *  );
}
*/


// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#include <stdlib.h>

class QuotaAllocator : public Rtt_Allocator
{
	public:
		QuotaAllocator( size_t bytesMax );
#ifdef Rtt_DEBUG
		virtual ~QuotaAllocator( );
#endif

	public:
		enum Constants
		{
			kHeaderSize = sizeof(size_t)
		};

		Rtt_FORCE_INLINE static void* SetHeader( size_t* p, size_t numBytes )
		{
			*p++ = numBytes;
			return p;
		}
		
		Rtt_FORCE_INLINE static size_t* GetHeader( void* p )
		{
			size_t *sp = (size_t*)p;
			return --sp;
		}

	public:
		// Returns number of bytes consumed if alloc takes place
		// Also pads numBytes with space for header
		Rtt_FORCE_INLINE size_t WillConsumeNumBytes( size_t& numBytes ) const
		{
			Rtt_STATIC_ASSERT( kHeaderSize == sizeof(void*) );
			
			numBytes += kHeaderSize;			
			return fBytesUsed + numBytes;
		}

	public:
		void* Alloc( size_t numBytes );
		void Free( void* p );
		void* Realloc( void* p, size_t numBytes );

	private:
		size_t fBytesUsed;
		const size_t fBytesMax;
};

QuotaAllocator::QuotaAllocator( size_t bytesMax )
:	fBytesUsed( 0 ),
	fBytesMax( bytesMax )
{
	Rtt_STATIC_ASSERT( sizeof( Rtt_Allocator ) == 0 );
	Rtt_ASSERT( bytesMax > 0 );
}

#ifdef Rtt_DEBUG
QuotaAllocator::~QuotaAllocator( )
{
	Rtt_ASSERT( 0 == fBytesUsed );
}
#endif

void*
QuotaAllocator::Alloc( size_t numBytes )
{
	size_t bytesUsed = WillConsumeNumBytes( numBytes );

	if ( bytesUsed <= fBytesMax )
	{
		fBytesUsed = bytesUsed;
		size_t *result = (size_t*)malloc( numBytes );
		return SetHeader( result, numBytes );
	}

	return NULL;
}

void*
QuotaAllocator::Alloc( size_t numBytes )
{
	size_t bytesUsed = WillConsumeNumBytes( numBytes );

	if ( bytesUsed <= fBytesMax )
	{
		fBytesUsed = bytesUsed;
		size_t *result = (size_t*)calloc( numBytes );
		return SetHeader( result, numBytes );
	}

	return NULL;
}

void
QuotaAllocator::Free( void *p )
{
//	size_t *ptr = (size_t*)p;
//	fBytesUsed -= *(--ptr);
	size_t *ptr = GetHeader( p );
	fBytesUsed -= *ptr;
	free( ptr );
}

void*
QuotaAllocator::Realloc( void* p, size_t numBytes )
{
	size_t *ptr = GetHeader( p );
	fBytesUsed -= *ptr;

	size_t bytesUsed = WillConsumeNumBytes( numBytes );
	
	if ( bytesUsed <= fBytesMax )
	{
		fBytesUsed = bytesUsed;
		size_t* result = (size_t*)realloc( ptr, numBytes );
		return SetHeader( result, numBytes );
	}

	return NULL;
}

// ----------------------------------------------------------------------------

typedef QuotaAllocator Allocator;

// ----------------------------------------------------------------------------

} // Rtt

#endif // Rtt_ALLOCATOR_SYSTEM


// Exports
// ----------------------------------------------------------------------------

Rtt_EXPORT Rtt_Allocator*
Rtt_AllocatorCreate( Rtt_ALLOCATOR_CREATE_ARG_DEFN )
{
	#ifdef Rtt_USE_GLOBAL_VARIABLES
		Rtt_Allocator* result = Rtt_Allocator::GetDefault();

		if ( 0 == Rtt_Allocator::RefCount()++ ) // TODO: there is a four-byte memory leak due to unbalanced counts :)
		{
			result = new Rtt::Allocator( Rtt_ALLOCATOR_CREATE_ARG_LIST );
			Rtt_Allocator::SetDefault( result );
		}
	#else
		Rtt_Allocator* result = new Rtt::Allocator( Rtt_ALLOCATOR_CREATE_ARG_LIST );
	#endif

	return result;
}

Rtt_EXPORT void
Rtt_AllocatorDestroy( Rtt_Allocator* pContext )
{
	#ifdef Rtt_USE_GLOBAL_VARIABLES
		Rtt_ASSERT( Rtt_Allocator::GetDefault() == pContext );
		if ( 0 == --Rtt_Allocator::RefCount() )
		{
			delete (Rtt::Allocator*)pContext;
			Rtt_Allocator::SetDefault( NULL );
		}
	#else
		delete (Rtt::Allocator*)pContext;
	#endif
}

Rtt_EXPORT void*
Rtt_AllocatorGetUserdata( Rtt_Allocator* pContext )
{
	return ((Rtt::Allocator*)pContext)->Userdata();
}

Rtt_EXPORT void
Rtt_AllocatorSetUserdata( Rtt_Allocator* pContext, void* userdata )
{
	((Rtt::Allocator*)pContext)->SetUserdata( userdata );
}

#if !defined( Rtt_ALLOCATOR_SYSTEM )

#ifdef Rtt_USE_GLOBAL_VARIABLES

RTT_EXPORT Rtt_Allocator*
Rtt_AllocatorDefault()
{
	return Rtt_Allocator::GetDefault();
}

#endif

// ----------------------------------------------------------------------------

Rtt_EXPORT void*
Rtt_AllocatorAlloc( Rtt_Allocator* pContext, size_t numBytes )
{
	Rtt_ASSERT( pContext );
	
	#ifdef Rtt_USE_GLOBAL_VARIABLES
		Rtt_ASSERT( Rtt_Allocator::GetDefault() == pContext );
	#else
		numBytes += sizeof(void*);
	#endif

	void* result = ((Rtt::Allocator*)pContext)->Alloc( numBytes );

	#ifdef Rtt_USE_GLOBAL_VARIABLES
	#else
		Rtt_STATIC_ASSERT( sizeof(U32) == sizeof(void*) );

		U32* ptr = (U32 *)result;
		*ptr++ = (U32)pContext;
		result = ptr;
	#endif

	return result;
}

Rtt_EXPORT void*
Rtt_AllocatorCalloc( Rtt_Allocator* pContext, size_t numElem, size_t elemSize )
{
	size_t numBytes = numElem * elemSize;
	void* result = Rtt_AllocatorAlloc( pContext, numBytes );

	if ( result )
	{
		memset( result, 0, numBytes );
	}

	return result;
}

Rtt_EXPORT void
Rtt_AllocatorFree( void* p )
{
	Rtt_ASSERT( p );

	Rtt_Allocator* pContext;

	#ifdef Rtt_USE_GLOBAL_VARIABLES
		pContext = Rtt_Allocator::GetDefault();
	#else
		Rtt_STATIC_ASSERT( sizeof(U32) == sizeof(void*) );

		U32* ptr = ((U32*)p) - 1;
		pContext = (Rtt_Allocator*) *ptr;
		p = ptr;
	#endif

	Rtt_ASSERT( pContext );
	((Rtt::Allocator*)pContext)->Free( p );
}

Rtt_EXPORT void*
Rtt_AllocatorRealloc( Rtt_Allocator* pContext, void* p, size_t numBytes )
{
	void* result;

	if ( p )
	{
		#ifdef Rtt_USE_GLOBAL_VARIABLES
			Rtt_ASSERT( Rtt_Allocator::GetDefault() == pContext );
		#else
			Rtt_STATIC_ASSERT( sizeof(U32) == sizeof(void*) );
			
			U32* ptr = ((U32*)p) - 1;
			Rtt_ASSERT( ((Rtt_Allocator*) *ptr) == pContext );
			p = ptr;
		#endif

		result = ((Rtt:Allocator*)pContext)->Realloc( p );
	}
	else
	{
		result = Rtt_AllocatorAlloc( pContext, numBytes );
	}

	return result;
}

Rtt_EXPORT void*
Rtt_CntxAllocatorAlloc( Rtt_Allocator* pContext, size_t numBytes )
{
	return ((Rtt::Allocator*)pContext)->Alloc( numBytes );
}

Rtt_EXPORT void
Rtt_CntxAllocatorFree( Rtt_Allocator* pContext, void* p )
{
	((Rtt::Allocator*)pContext)->Free( p );
}

#endif // Rtt_ALLOCATOR_SYSTEM


#ifdef Rtt_OPTIMIZE_REFCOUNT

Rtt_EXPORT Rtt::RefCount*
Rtt_AllocatorAllocRefCount( Rtt_Allocator* pContext )
{
	Rtt_ASSERT( pContext );
	return ((Rtt::Allocator*)pContext)->AllocRefCount();
}

Rtt_EXPORT void
Rtt_AllocatorFreeRefCount( Rtt::RefCount* p )
{
	Rtt_ASSERT( p );
	Rtt_Allocator::FreeRefCount( p );
}

#endif // Rtt_OPTIMIZE_REFCOUNT

// ----------------------------------------------------------------------------


