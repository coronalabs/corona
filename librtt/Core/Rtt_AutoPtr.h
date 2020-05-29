//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Rtt_AutoPtr_H
#define Rtt_AutoPtr_H

#include "Core/Rtt_AutoResource.h"
#include "Core/Rtt_Finalizer.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

template < typename T >
class AutoPtr : public AutoResource< T*, PtrFinalizer< T > >
{
	public:
		typedef AutoPtr< T > Self;
		typedef AutoResource< T*, PtrFinalizer< T > > Super;

	public:
		static const Self& Null();

	public:
		AutoPtr() : Super() {}
		AutoPtr( const Self& rhs ) : Super( rhs ) {}
		explicit AutoPtr( Rtt_Allocator* pZone, T* ptr ) : Super( pZone, ptr ) {}

	public:
		Rtt_FORCE_INLINE const T* operator->() const { return *this; }
		Rtt_FORCE_INLINE T* operator->() { return *this; }
};

// ----------------------------------------------------------------------------

template < typename T >
const AutoPtr< T >&
AutoPtr< T >::Null()
{
	static Self sNull;
	return sNull;
}

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_AutoPtr_H


