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

#include "Display/Rtt_RectObject.h"
#include "Display/Rtt_ShaderResource.h"
#include "Display/Rtt_RectPath.h"
#include "Display/Rtt_TesselatorRect.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

RectObject*
RectObject::NewRect( Rtt_Allocator* pAllocator, Real width, Real height )
{
	RectPath* path = RectPath::NewRect( pAllocator, width, height );
	return Rtt_NEW( pAllocator, RectObject( path ) );
}

// ----------------------------------------------------------------------------

RectObject::RectObject( RectPath *path )
:	Super( path )
{
	Rtt_ASSERT( path );
}

void
RectObject::GetSelfBoundsForAnchor( Rect& rect ) const
{
	//Initial implementation used the OOP hierarchy to return the appropriate bounds for anchor
	//Per Walter's call though he preferred to cast, to keep unnecessary overhead out of the other
	//classes
	const ShapePath& path = static_cast< const ShapePath& >( GetPath() );
	const TesselatorRect* tesselator = static_cast< const TesselatorRect* >( path.GetTesselator() );
	
	Rtt_ASSERT(NULL != tesselator);
	if (tesselator)
	{
		tesselator->GetSelfBoundsForAnchor(rect);
	}
}

ShaderResource::ProgramMod
RectObject::GetProgramMod() const
{
	ShaderResource::ProgramMod result = ShaderResource::kDefault;
	
	const ShapePath& path = static_cast< const ShapePath& >( GetPath() );
	const TesselatorRect* tesselator = static_cast< const TesselatorRect* >( path.GetTesselator() );
	
	Rtt_ASSERT(NULL != tesselator);
	if (tesselator)
	{
		result = tesselator->HasOffset() ? ShaderResource::k25D : ShaderResource::kDefault;
	}
	return result;
}

const RectPath&
RectObject::GetRectPath() const
{
	return static_cast< const RectPath& >( GetPath() );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

