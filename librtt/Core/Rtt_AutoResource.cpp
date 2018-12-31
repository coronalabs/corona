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

#include "Core/Rtt_Build.h"

#include "Core/Rtt_AutoResource.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

namespace Private
{

#if defined( Rtt_MINIMIZE_CODE_SIZE )

bool
RefCountRelease( RefCount* pRefCount )
{
	bool shouldCollect = false;

	if ( pRefCount )
	{
		--(*pRefCount);
		
		shouldCollect = ( 0 == *pRefCount );

		if ( shouldCollect )
		{
			Rtt_AllocatorFreeRefCount( pRefCount );
		}
	}

	return shouldCollect;
};

#endif // Rtt_MINIMIZE_CODE_SIZE

} // Private

#ifdef Rtt_DEBUG
RefCount* FreedRefCount()
{
	static RefCount sCount = -1;
	Rtt_ASSERT( sCount < 0 );
	return & sCount;
}
#endif

// ----------------------------------------------------------------------------

} // Rtt

