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

#include "Display/Rtt_DisplayPath.h"

#include "Rtt_Matrix.h"
#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_VertexCache.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Rtt_LuaUserdataProxy.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

void
DisplayPath::UpdateGeometry( Geometry& dst, const VertexCache& src, const Matrix& srcToDstSpace, U32 flags, Array<U16> *indices )
{
	if ( 0 == flags ) { return; }

	const ArrayVertex2& vertices = src.Vertices();
	const ArrayVertex2& texVertices = src.TexVertices();
	U32 numVertices = vertices.Length();

	U32 numIndices = indices==NULL?0:indices->Length();
	if ( dst.GetVerticesAllocated() < numVertices || dst.GetIndicesAllocated() < numIndices)
	{
		dst.Resize( numVertices, numIndices, false );
	}
	Geometry::Vertex *dstVertices = dst.GetVertexData();

	bool updateVertices = ( flags & kVerticesMask );
	bool updateTexture = ( flags & kTexVerticesMask );

	Rtt_ASSERT( ! updateTexture || ( vertices.Length() == texVertices.Length() ) );

	for ( U32 i = 0, iMax = vertices.Length(); i < iMax; i++ )
	{
		Rtt_ASSERT( i < dst.GetVerticesAllocated() );

		Geometry::Vertex& dst = dstVertices[i];

		if ( updateVertices )
		{
			Vertex2 v = vertices[i];
			srcToDstSpace.Apply( v );

			dst.x = v.x;
			dst.y = v.y;
			dst.z = 0.f;
		}

		if ( updateTexture )
		{
			dst.u = texVertices[i].x;
			dst.v = texVertices[i].y;
			dst.q = 1.f;
		}
	}

	dst.SetVerticesUsed( numVertices );
	
	if(flags & kIndicesMask)
	{
		if(indices)
		{
			const U16* indicesData = indices->ReadAccess();
			U16* dstData = dst.GetIndexData();
			numIndices = indices->Length();
			for (U32 i=0; i<numIndices; i++)
			{
				dstData[i] = indicesData[i];
			}
			
			dst.Invalidate();
		}
		dst.SetIndicesUsed(numIndices);
	}
}

// ----------------------------------------------------------------------------

DisplayPath::DisplayPath()
:	fObserver( NULL ),
	fAdapter( NULL ),
	fProxy( NULL )
{
}

DisplayPath::~DisplayPath()
{
	if ( fProxy )
	{
		GetObserver()->QueueRelease( fProxy ); // Release native ref to Lua-side proxy
		fProxy->DetachUserdata(); // Notify proxy that object is invalid
	}
}

void
DisplayPath::PushProxy( lua_State *L ) const
{
	if ( ! fProxy )
	{
		fProxy = LuaUserdataProxy::New( L, const_cast< Self * >( this ) );
		fProxy->SetAdapter( GetAdapter() );
	}

	fProxy->Push( L );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

