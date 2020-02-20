//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_SharedCountImpl_H__
#define __Rtt_SharedCountImpl_H__

#include "Core/Rtt_Allocator.h"
#include "Core/Rtt_Macros.h"
#include "Core/Rtt_Traits.h"
#include "Core/Rtt_Types.h"
#include "Core/Rtt_UseCount.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

template < typename T, typename Finalizer >
class SharedCountImpl : public UseCount
{
	Rtt_CLASS_NO_COPIES( SharedCountImpl )

	private:
		typedef UseCount Super;
		typedef SharedCountImpl< T, Finalizer > Self;

	public:
		explicit SharedCountImpl( T *p ) : Super(), fPointer( p ) { }

	public:
		virtual void FinalizeStrong();

	public:
///		bool IsNull() const { return NULL == fCount; }
///		bool NotNull() const { return ! IsNull(); }

	private:
		T *fPointer;
};

// Implementation
// ----------------------------------------------------------------------------

template < typename T, typename Finalizer >
void
SharedCountImpl< T, Finalizer >::FinalizeStrong()
{
	Rtt_ASSERT( 0 == GetStrongCount() );
	Finalizer::Collect( fPointer );
	fPointer = NULL;
}

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_SharedCountImpl_H__

