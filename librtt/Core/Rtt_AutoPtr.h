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


