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

