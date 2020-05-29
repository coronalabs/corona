//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_SharedPtr_H__
#define __Rtt_SharedPtr_H__

#include "Core/Rtt_SharedCount.h"
#include "Core/Rtt_WeakPtr.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

template<class T> class WeakPtr;

// ----------------------------------------------------------------------------

template < class T >
class SharedPtr
{
	public:
		typedef SharedPtr< T > Self;

	public:
		SharedPtr() : fResource( NULL ), fCount() {}
		explicit SharedPtr( T *p ) : fResource( p ), fCount( p ) {}
		SharedPtr( const WeakPtr< T >& rhs );

		static SharedPtr< T > ForRttNewCreatedPtr( T *p )
		{
			return SharedPtr< T >( p );
		}

		static SharedPtr< T > ForRttMallocCreatedPtr( T *p )
		{
			return SharedPtr< T >::WithFinalizer< RttMallocCreatedPtrFinalizer< T > >( p );
		}

		static SharedPtr< T > ForCppNewCreatedPtr( T *p )
		{
			return SharedPtr< T >::WithFinalizer< CppNewCreatedPtrFinalizer< T > >( p );
		}

		template< class TFinalizer >
		static SharedPtr< T > WithFinalizer( T *p )
		{
			SharedPtr< T > sharedPointer;
			sharedPointer.fResource = p;
			sharedPointer.fCount = SharedCount::Create< T, TFinalizer >( p );
			return sharedPointer;
		}

	public:
		bool IsNull() const;
		bool NotNull() const { return ! IsNull(); }

#ifdef Rtt_DEBUG_SHARED_PTR
		void Log( const char *label ) const;
#endif

	public:
		T* operator->() const { return fResource; }
		T& operator*() const { return * fResource; }
		bool operator==( const SharedPtr< T >& pointer ) const { return ( pointer.fResource == fResource ); }
		bool operator!=( const SharedPtr< T >& pointer ) const { return ( pointer.fResource != fResource ); }

	private:
		template < class Y > friend class WeakPtr;

		T *fResource;
		SharedCount fCount;
};

// ----------------------------------------------------------------------------

template < class T >
SharedPtr< T >::SharedPtr( const WeakPtr< T >& rhs )
:	fResource( NULL ),
	fCount()
{
	if ( rhs.NotNull() )
	{
		fResource = rhs.fResource;
		fCount = rhs.fCount;
	}
}

template < class T >
bool
SharedPtr< T >::IsNull() const
{
	return ! fCount.IsValid();
}

#ifdef Rtt_DEBUG_SHARED_PTR
template < class T >
void
SharedPtr< T >::Log( const char *label ) const
{
	if ( ! label ) { label = "SharedPtr"; }
	printf( "[%s] IsNull(%d)\n", label, IsNull() );
	fCount.Log();
	printf( "\n" );
}
#endif

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_SharedPtr_H__

