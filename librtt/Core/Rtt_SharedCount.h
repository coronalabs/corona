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

