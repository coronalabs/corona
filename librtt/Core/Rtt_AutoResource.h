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

#ifndef Rtt_AutoResource_H
#define Rtt_AutoResource_H

#include "Core/Rtt_Allocator.h"
#include "Core/Rtt_RefCount.h"
#include "Core/Rtt_Traits.h"
#include "Core/Rtt_Types.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

template < typename T >
class NullPointerOrPrimitive
{
	public:
		static T Value()
		{
			Rtt_STATIC_ASSERT( Traits::IsPointer< T >::Value || Traits::IsPrimitive< T >::Value );

			return NULL;
		}
};

// ----------------------------------------------------------------------------

namespace Private
{
#if defined( Rtt_MINIMIZE_CODE_SIZE )
bool RefCountRelease( RefCount* pRefCount );
#endif
} // Private

#ifdef Rtt_DEBUG
RefCount* FreedRefCount();
#else
#define FreedRefCount()		NULL
#endif

// ----------------------------------------------------------------------------

template < typename T, typename Finalizer, typename NullResource = NullPointerOrPrimitive< T > >
class AutoResource
{
	private:
		typedef AutoResource< T, Finalizer, NullResource > Self;

	public:
		AutoResource() : fOwner( NULL ), fResource( NullResource::Value() ) { }

		AutoResource( const Self& rhs ) { Retain( rhs.fOwner, rhs.fResource ); }

		explicit AutoResource( Rtt_Allocator* pAllocator, T resource );

		~AutoResource() { Release(); }

	public:
		bool IsNull() const;
		bool NotNull() const { return ! IsNull(); }

	public:
		AutoResource& operator=( const Self& rhs );

		// Conversion operator, not address-of operator!
		// Facilitates *'ing AutoResource instance to obtain _reference_ to resource.
		// Also simplifies sub-class implementation
		operator T&() { return fResource; }
		operator const T&() const { return fResource; }

		bool operator==( const T& rhs ) const { return fResource == rhs; }
		bool operator!=( const T& rhs ) const { return fResource != rhs; }

#ifdef Rtt_DEBUG
		void Z() const
		{
			Rtt_ASSERT( fOwner != FreedRefCount() );
			Rtt_TRACE( ( "0x%x(", fResource ) );
			if ( fOwner )
			{
				Rtt_TRACE( ( "%d", * fOwner ) );
			}
			Rtt_TRACE( ( ")" ) );
		}
#endif

	private:
		void Retain( RefCount* pRefCount, T resource );
		void Release();

	private:
		// The ref count "owns" the resource
		RefCount* fOwner;
		T fResource;
};

// Implementation
// ----------------------------------------------------------------------------

template < typename T, typename Finalizer, typename NullResource >
AutoResource< T, Finalizer, NullResource >::AutoResource( Rtt_Allocator* pAllocator, T resource )
{
	Rtt_STATIC_ASSERT( Traits::IsPointer< T >::Value || Traits::IsPrimitive< T >::Value );

	RefCount* pRefCount = ( NullResource::Value() == resource ? NULL : Rtt_AllocatorAllocRefCount( pAllocator ) );

	Retain( pRefCount, resource );
}

template < typename T, typename Finalizer, typename NullResource >
bool
AutoResource< T, Finalizer, NullResource >::IsNull() const
{
	return NullResource::Value() == fResource;
}

template < typename T, typename Finalizer, typename NullResource >
AutoResource< T, Finalizer, NullResource >&
AutoResource< T, Finalizer, NullResource >::operator=( const Self& rhs )
{
	Release();
	Retain( rhs.fOwner, rhs.fResource );

	return *this;
}

template < typename T, typename Finalizer, typename NullResource >
void
AutoResource< T, Finalizer, NullResource >::Retain( RefCount* pRefCount, T resource )
{
	Rtt_STATIC_ASSERT( Traits::IsPointer< T >::Value || Traits::IsPrimitive< T >::Value );

	fOwner = pRefCount;
	fResource = resource;

	if ( pRefCount ) { ++(*pRefCount); }
}

template < typename T, typename Finalizer, typename NullResource >
void
AutoResource< T, Finalizer, NullResource >::Release()
{
	Rtt_ASSERT( fOwner != FreedRefCount() );

#if defined( Rtt_MINIMIZE_CODE_SIZE )
	if ( Private::RefCountRelease( fOwner ) )
	{
		Finalizer::Collect( fResource );
	}
#else
	RefCount* pRefCount = fOwner;

	if ( pRefCount )
	{
		--(*pRefCount);
				
		if ( 0 == *pRefCount )
		{
			Rtt_AllocatorFreeRefCount( pRefCount );
			Finalizer::Collect( fResource );
		}
	}
#endif

	fOwner = FreedRefCount();
	fResource = NullResource::Value();
}

// ----------------------------------------------------------------------------

} // Rtt

// ----------------------------------------------------------------------------

#endif // Rtt_AutoResource_H

