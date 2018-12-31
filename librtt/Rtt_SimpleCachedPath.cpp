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

#include "Core/Rtt_Allocator.h"
#include "Display/Rtt_DisplayObject.h"
#include "Rtt_SimpleCachedPath.h"
#include "Rtt_FillTesselatorStream.h"
#include "Display/Rtt_ClosedPath.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

SimpleCachedPath::SimpleCachedPath( Rtt_Allocator* pAllocator, ClosedPath& path )
:	Super( path ),
	fCache( pAllocator )
{
}

void
SimpleCachedPath::Update( RenderData& data, const Matrix& srcToDstSpace )
{
#ifdef OLD_GRAPHICS
	// Super::Build( srcToDstSpace );

	// Generate Vertices
	FillTesselatorStream tesselator( & srcToDstSpace, fCache );

	// Ensure that Draw() goes through the non-cached path
//	Rtt_ASSERT( ! OriginalPath().IsCached() );
	OriginalPath().Draw( tesselator, NULL );
#endif
}

void
SimpleCachedPath::Translate( Real dx, Real dy )
{
	ArrayVertex2& vertices = fCache.Vertices();
	Rtt_ASSERT( ( vertices.Length() == 0 ) == ( fCache.Counts().Length() == 0 ) );

	if ( vertices.Length() > 0 )
	{
		Vertex2_Translate( vertices.WriteAccess(), vertices.Length(), dx, dy );
	}
}

void
SimpleCachedPath::Draw( Renderer& renderer ) const
{
#ifdef OLD_GRAPHICS
	Rtt_ASSERT( object );
	Quad stageQuad;
	const Quad* pSubmitQuad = object->GetSubmitQuad( stageQuad );

	const ArrayS32& counts = fCache.Counts();
	rStream.WillSubmitArray( * pSubmitQuad );
	rStream.Submit(
		RenderTypes::kFan,
		fCache.Vertices().ReadAccess(),
		counts.ReadAccess(),
		counts.Length() );
	rStream.DidSubmitArray();
#endif
}

void
SimpleCachedPath::Invalidate()
{
	fCache.Vertices().Empty();
	fCache.Counts().Empty();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
