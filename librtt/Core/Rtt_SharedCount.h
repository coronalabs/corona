//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_SharedCount_H__
#define __Rtt_SharedCount_H__

#include "Core/Rtt_Finalizer.h"
#include "Core/Rtt_SharedCountImpl.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class WeakCount;

// ----------------------------------------------------------------------------

class SharedCount
{
	public:
		typedef SharedCount Self;

	public:
		SharedCount();

		template < class T >
		explicit SharedCount( T *p )
		{
			fCount = new SharedCountImpl< T, PtrFinalizer< T > >( p );
		}

		SharedCount( const SharedCount& rhs );
		SharedCount( const WeakCount& rhs );
		~SharedCount();

		template < class T, class TFinalizer >
		static SharedCount Create( T *p )
		{
			SharedCount sharedCount;
			sharedCount.fCount = new SharedCountImpl< T, TFinalizer >( p );
			return sharedCount;
		}

	protected:
		SharedCount& Assign( UseCount *newCount );

	public:
		SharedCount& operator=( const SharedCount& rhs );
		SharedCount& operator=( const WeakCount& rhs );

	public:
		bool IsValid() const;
//		bool IsNull() const { return NULL == fCount; }
//		bool NotNull() const { return ! IsNull(); }

		void Log() const;

	public:
		friend inline bool operator==( const SharedCount &lhs, const SharedCount &rhs)
		{
			return lhs.fCount == rhs.fCount;
		}

	private:
		UseCount *fCount;

	friend class WeakCount;
};

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_SharedCount_H__

