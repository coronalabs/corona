//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Rtt_Finalizer_H
#define Rtt_Finalizer_H

#include "Core/Rtt_Allocator.h"
#include "Core/Rtt_Assert.h"
#include "Core/Rtt_Traits.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

template < typename T >
class PtrFinalizer
{
	public:
		Rtt_INLINE static void Collect( T* p )
		{
			Rtt_DELETE( p );
		}
};

// ----------------------------------------------------------------------------

template < typename T >
class RttMallocCreatedPtrFinalizer
{
	public:
		Rtt_INLINE static void Collect( T* p )
		{
			Rtt_FREE( p );
		}
};

// ----------------------------------------------------------------------------

template < typename T >
class CppNewCreatedPtrFinalizer
{
	public:
		Rtt_INLINE static void Collect( T* p )
		{
			delete p;
		}
};

// ----------------------------------------------------------------------------

template < typename T >
class CppNewCreatedArrayFinalizer
{
	public:
		Rtt_INLINE static void Collect( T* p )
		{
			delete[] p;
		}
};

// ----------------------------------------------------------------------------

template < typename T >
class ObjectFinalizer
{
	public:
		static void Collect( const T& object )
		{
			Rtt_STATIC_ASSERT( ! Traits::IsPointer< T >::Value );
			object.~T();
		}
};

template < typename T >
class ObjectFinalizer< T* >
{
	public:
		static void Collect( T* object )
		{
			PtrFinalizer< T >::Collect( object );
		}
};

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_Finalizer_H


