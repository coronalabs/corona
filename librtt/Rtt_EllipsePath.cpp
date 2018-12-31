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

#ifdef OLD_GRAPHICS

#include "Core/Rtt_Build.h"

#include "Rtt_EllipsePath.h"
#include "Rtt_RenderingStream.h"
#include "Display/Rtt_DisplayObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

EllipsePath::EllipsePath( Rtt_Allocator* pAllocator, Real a, Real b )
:	Super( pAllocator )
{
	fHalfW = a;
	fHalfH = b;
}

void
EllipsePath::Draw( Renderer& renderer, const DisplayObject* object ) const
{
	DisplayObjectSubmitContext context( rStream, object );

	if ( ! IsProperty( kIsCached ) || ! object )
	{
		Real a = fHalfW;
		Real b = fHalfH;
		if ( Rtt_RealEqual( a, b ) )
		{
			rStream.Submit( fDstOrigin, a );
		}
		else
		{
			rStream.Submit( fDstOrigin, a, b );
		}
	}
	else
	{
		fCache.Draw( rStream, object );
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif