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

