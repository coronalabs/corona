//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_WeakPtr_H__
#define __Rtt_WeakPtr_H__

#include "Core/Rtt_SharedPtr.h"
#include "Core/Rtt_WeakCount.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

template<class T> class SharedPtr;

// ----------------------------------------------------------------------------

template < class T >
class WeakPtr
{
	public:
		typedef WeakPtr< T > Self;

	public:
		WeakPtr() : fResource( NULL ), fCount() {}
		WeakPtr( const SharedPtr< T >& rhs );

	public:
		bool IsNull() const;
		bool NotNull() const { return ! IsNull(); }

#ifdef Rtt_DEBUG_SHARED_PTR
		void Log( const char *label) const;
#endif

	private:
		template < class Y > friend class SharedPtr;

		T *fResource;
		WeakCount fCount;
};

// ----------------------------------------------------------------------------

template < class T >
WeakPtr< T >::WeakPtr( const SharedPtr< T >& rhs )
:	fResource( rhs.fResource ),
	fCount( rhs.fCount )
{
}

template < class T >
bool
WeakPtr< T >::IsNull() const
{
	return ! fCount.IsValid();
}

#ifdef Rtt_DEBUG_SHARED_PTR
template < class T >
void
WeakPtr< T >::Log( const char *label) const
{
	if ( ! label ) { label = "WeakPtr"; }
	printf( "[%s] IsNull(%d)\n", label, IsNull() );
	fCount.Log();
	printf( "\n" );
}
#endif

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_WeakPtr_H__

