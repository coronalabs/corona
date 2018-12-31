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


