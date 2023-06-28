//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Allocator_H__
#define _Rtt_Allocator_H__

#include "Core/Rtt_Config.h"
#include "Core/Rtt_Macros.h"
#include "Core/Rtt_Types.h"

// ----------------------------------------------------------------------------

// Allocator Interface
// ----------------------------------------------------------------------------

#if !defined( Rtt_ALLOCATOR_SYSTEM ) && !defined( Rtt_USE_GLOBAL_VARIABLES )
	#define Rtt_ALLOCATOR_ENABLE

	#define Rtt_ALLOCATOR_TYPE			Rtt_Allocator*
	#define Rtt_ALLOCATOR_VAR( name )	Rtt_ALLOCATOR_TYPE name
#else
	#define Rtt_ALLOCATOR_TYPE
	#define Rtt_ALLOCATOR_VAR( name )
#endif

#if defined( Rtt_ALLOCATOR_SYSTEM )
	#define Rtt_ALLOCATOR_CREATE_ARG_DEFN
	#define Rtt_ALLOCATOR_CREATE_ARG_LIST
#else
	#define Rtt_ALLOCATOR_CREATE_ARG_DEFN	size_t poolSize
	#define Rtt_ALLOCATOR_CREATE_ARG_LIST	poolSize
#endif

Rtt_EXPORT Rtt_Allocator*
Rtt_AllocatorCreate( Rtt_ALLOCATOR_CREATE_ARG_DEFN );

Rtt_EXPORT void
Rtt_AllocatorDestroy( Rtt_Allocator* pAllocator );

Rtt_EXPORT void*
Rtt_AllocatorGetUserdata( Rtt_Allocator* pAllocator );

Rtt_EXPORT void
Rtt_AllocatorSetUserdata( Rtt_Allocator* pAllocator, void* userdata );


// Placement new
// ----------------------------------------------------------------------------
#if defined( __cplusplus ) && ! defined( Rtt_PLACEMENT_NEW_PREDEFINED )

//	Rtt_FORCE_INLINE void* operator new( size_t, void* p ) throw() { return p; }

#endif


// Deliberately not implemented
// ----------------------------------------------------------------------------

#ifdef __cplusplus

#ifdef Rtt_PLACEMENT_NEW

// See Rtt_New.h

void* operator new[]( size_t numBytes ) throw();
void operator delete[]( void* p ) throw();

#endif

#endif // __cplusplus


// General Allocator
// ----------------------------------------------------------------------------

#if defined( Rtt_ALLOCATOR_SYSTEM )

	#include <stdlib.h>

	#define Rtt_MALLOC( null, size )			malloc( (size) )
	#define Rtt_CALLOC( null, numElem, size )	calloc( (numElem), (size) )
	#define Rtt_MALLOC_TYPE( null, T )			(T*)Rtt_MALLOC( sizeof(T) )
	#define Rtt_FREE( p )						free( (p) )
	#define Rtt_REALLOC( null, p, size )		realloc( (p), (size) )

	#define Rtt_CNTX_MALLOC( null, size )		Rtt_MALLOC( null, (size) )
	#define Rtt_CNTX_FREE( null, p )			Rtt_FREE(p)
	#define Rtt_CNTX_REALLOC( null, p, size )	Rtt_REALLOC( null, (p), (size) )

	#ifdef __cplusplus

	#if defined(Rtt_WIN_ENV) && defined(Rtt_DEBUG) && defined(Rtt_CHECK_MEMORY)
		#define Rtt_NEW( pUnused, T )			new ( _CLIENT_BLOCK, __FILE__, __LINE__) T
	#else
		#define Rtt_NEW( pUnused, T )			new T
	#endif

	#define Rtt_DELETE( p )						delete (p)

#ifdef Rtt_PLACEMENT_NEW
// From Rtt_New.h, this conflicts with the include of <new>
// I'm not sure this use of the define is good
	void* operator new( size_t numBytes ) throw();
	void operator delete( void* p ) throw();
#endif

	#endif // __cplusplus

#else // !defined( Rtt_ALLOCATOR_SYSTEM )

	#define Rtt_MALLOC( cntx, size )			Rtt_AllocatorAlloc( (cntx), (size) )
	#define Rtt_CALLOC( cntx, numElem, size )	Rtt_AllocatorCalloc( (cntx), (numElem), (size) )
	#define Rtt_MALLOC_TYPE( cntx, T )			(T*)Rtt_MALLOC( (cntx), sizeof(T) )
	#define Rtt_FREE( p )						Rtt_AllocatorFree( (p) )
	#define Rtt_REALLOC( null, p, size )		Rtt_AllocatorRealloc( (p), (size) )

	#define Rtt_CNTX_MALLOC( cntx, size )		Rtt_CntxAllocatorAlloc( (cntx), (size) )
	#define Rtt_CNTX_FREE( cntx, p )			Rtt_CntxAllocatorFree( (cntx), (p) )
	#define Rtt_CNTX_REALLOC( cntx, p, size )	Rtt_CntxAllocatorRealloc( cntx, (p), (size) )

	#ifdef __cplusplus

	#define Rtt_NEW( pAllocator, T )			new( pAllocator ) T
	#define Rtt_DELETE( p )						::Rtt::Delete( (p) )

	#endif // __cplusplus

	#ifdef Rtt_USE_GLOBAL_VARIABLES
		RTT_EXPORT Rtt_Allocator* Rtt_AllocatorDefault();
	#endif

	Rtt_EXPORT void*
	Rtt_AllocatorAlloc( Rtt_Allocator* pContext, size_t numBytes );

	Rtt_EXPORT void*
	Rtt_AllocatorCalloc( Rtt_Allocator* pContext, size_t numElem, size_t elemSize );

	Rtt_EXPORT void
	Rtt_AllocatorFree( void* p );

	Rtt_EXPORT void*
	Rtt_AllocatorRealloc( Rtt_Allocator* pContext, void* p, size_t numBytes );

	Rtt_EXPORT void*
	Rtt_CntxAllocatorAlloc( Rtt_Allocator* pContext, size_t numBytes );

	Rtt_EXPORT void
	Rtt_CntxAllocatorFree( Rtt_Allocator* pContext, void* p );

	Rtt_EXPORT void*
	Rtt_CntxAllocatorRealloc( Rtt_Allocator* pContext, void* p, size_t numBytes );

	#ifdef __cplusplus

	// Not implemented b/c placement versions should be used
	#if !defined( Rtt_Allocator_CPP )
	void* operator new( size_t numBytes ) throw();
	void operator delete( void* p );
	#endif // Rtt_Allocator_CPP
	
	void* operator new( size_t numBytes, Rtt_Allocator* pAllocator ) throw();
//	void* operator new[]( size_t numBytes, Rtt_Allocator* pAllocator ) throw();

	namespace Rtt
	{

	template < class T, class A >
	Rtt_FORCE_INLINE
	void Delete( T* p )
	{
		if ( p )
		{
			p->~T();
			Rtt_AllocatorFree( p );
		}
	}

	} // Rtt

	#endif // __cplusplus

	#define Rtt_SafeFree( p ) \
		if( p ) \
		{ \
			free( p ); \
			p = NULL; \
		}

	#define Rtt_SafeDelete( p ) \
		if( p ) \
		{ \
			delete p; \
			p = NULL; \
		}

	#define Rtt_SafeDeleteArray( p ) \
		if( p ) \
		{ \
			delete [] p; \
			p = NULL; \
		}

#endif // Rtt_ALLOCATOR_SYSTEM

// ----------------------------------------------------------------------------

#endif // _Rtt_Allocator_H__
