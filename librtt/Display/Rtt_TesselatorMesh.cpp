//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_TesselatorMesh.h"

#include "Display/Rtt_TesselatorLine.h"
#include "Rtt_Matrix.h"
#include "Rtt_Transform.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

TesselatorMesh::TesselatorMesh( Rtt_Allocator *allocator, Geometry::PrimitiveType meshType )
:	Super()
,	fUVs( allocator )
,	fMesh( allocator )
,	fIndices( allocator )
,	fSelfBounds()
,	fIsMeshValid( false )
,	fMeshType(meshType)
,   fVertexOffset(kVertexOrigin)
,	fLowestIndex(0)
,	fStrokeCount( -1 )
{
}

void
TesselatorMesh::GenerateFill( ArrayVertex2& vertices )
{
	Update();
	for (int i=0; i<fMesh.Length(); i++) {
		vertices.Append(fMesh[i]);
	}
}
	
void
TesselatorMesh::GenerateFillIndices( ArrayIndex& indicies )
{
	for (int i=0; i<fIndices.Length(); i++) {
		indicies.Append(fIndices[i]);
	}
}
	
void
TesselatorMesh::GenerateFillTexture( ArrayVertex2& texCoords, const Transform& t )
{
	Update();
	for (int i=0; i<fUVs.Length(); i++) {
		texCoords.Append(fUVs[i]);
	}

	if( !t.IsIdentity() )
	{
		Matrix m;
		m.Translate( -Rtt_REAL_HALF, -Rtt_REAL_HALF );
		m.Scale( t.GetSx(), t.GetSy() );
		m.Rotate( -t.GetRotation() );
		m.Translate( t.GetX(), t.GetY() );
		m.Translate( Rtt_REAL_HALF, Rtt_REAL_HALF );
		m.Apply(texCoords.WriteAccess(), texCoords.Length());
	}
}


static inline bool CompareVertices( const Vertex2 &a, const Vertex2 &b)
{
	return Rtt_RealEqual(a.x, b.x) && Rtt_RealEqual(a.y, b.y);
}

// compares edges a1-a2 and b1-b2
static inline bool CompareEdges( const Vertex2 &a1, const Vertex2 &a2, const Vertex2 &b1, const Vertex2 &b2)
{
	return ( CompareVertices(a1, b1) && CompareVertices(a2, b2) )
		|| ( CompareVertices(a1, b2) && CompareVertices(a2, b1) );
}

static inline bool AddAndVerifyEdge( ArrayVertex2 &edges, U16 idx, const ArrayVertex2 &mesh)
{
	if ( idx < mesh.Length())
	{
		edges.Append( mesh[idx] );
		return true;
	}
	else
	{
		return false;
	}
}
	
void
TesselatorMesh::GenerateStroke( ArrayVertex2& vertices )
{
	if(fMesh.Length() < 3 || (Rtt_RealIsZero(GetInnerWidth()) || Rtt_RealIsZero(GetOuterWidth())) )
	{
		return;
	}
	
	if (fMeshType == Geometry::kTriangles || fMeshType == Geometry::kIndexedTriangles)
	{
		ArrayVertex2 edges(fMesh.Allocator());
		const int numVertices = fMesh.Length();
		if (fMeshType == Geometry::kTriangles)
		{
			const int triangleVertices = 3*(numVertices/3);
			edges.Reserve( fMesh.Length()*2 );
			for (int i=0; i<triangleVertices; i+=3)
			{
				edges.Append( fMesh[i+0] );
				edges.Append( fMesh[i+1] );
				
				edges.Append( fMesh[i+1] );
				edges.Append( fMesh[i+2] );
				
				edges.Append( fMesh[i+2] );
				edges.Append( fMesh[i+0] );
			}
		}
		else if (fMeshType == Geometry::kIndexedTriangles)
		{
			const int triangleVertices = 3*(fIndices.Length()/3);
			edges.Reserve( fIndices.Length()*2 );
			for (int i=0; i<triangleVertices; i+=3)
			{
				 if (!( AddAndVerifyEdge( edges, fIndices[i+0], fMesh )
					 && AddAndVerifyEdge( edges, fIndices[i+1], fMesh )
					
					 && AddAndVerifyEdge( edges, fIndices[i+1], fMesh )
					 && AddAndVerifyEdge( edges, fIndices[i+2], fMesh )
					
					 && AddAndVerifyEdge( edges, fIndices[i+2], fMesh )
					 && AddAndVerifyEdge( edges, fIndices[i+0], fMesh ) ))
				 {
					 break;
				 }
			}
		}
		// fun fact: interlal edges of a mesh would be here twice. So, removing duplicates would leave us only with outline!
		for (int i=0; i<edges.Length(); i+=2)
		{
			for (int j=i+2; j<edges.Length(); j+=2)
			{
				if(CompareEdges( edges[i], edges[i+1], edges[j], edges[j+1] ))
				{
					edges.Remove(j, 2, false);
					edges.Remove(i, 2, false);
					i-=2;
					break;
				}
			}
		}
		
		ArrayVertex2 lineLoop(fMesh.Allocator());
		while(edges.Length()>1) //actually edges would always be pairs of vertices, >1 it's just for sanity sake.
		{
			lineLoop.Append(edges[0]);
			lineLoop.Append(edges[1]);
			edges.Remove(0, 2);

			bool found;
			do {

				const Vertex2& last = lineLoop[lineLoop.Length()-1];
				found = false;
				for (int i=0; i<edges.Length(); i+=2)
				{
					if(CompareVertices( last, edges[i]))
					{
						lineLoop.Append(edges[i+1]);
						found = true;
					}
					else if(CompareVertices( last, edges[i+1]))
					{
						lineLoop.Append(edges[i]);
						found = true;
					}
					if(found)
					{
						edges.Remove(i, 2);
						break;
					}
				}
			} while (found);
			
			ArrayVertex2 *dst;
			if (vertices.Length() == 0)
			{
				dst = &vertices;
			}
			else
			{
				dst = Rtt_NEW( vertices.Allocator(), ArrayVertex2(vertices.Allocator()) );
			}
			
			if(lineLoop.Length() > 3 && CompareVertices(lineLoop[0], lineLoop[lineLoop.Length()-1]))
			{
				lineLoop.Remove(lineLoop.Length()-1, 1);
				TesselatorLine t( lineLoop, TesselatorLine::kLoopMode );
				t.SetInnerWidth( GetInnerWidth() );
				t.SetOuterWidth( GetOuterWidth() );
				t.GenerateStroke( *dst );
			}
			else
			{
				TesselatorLine t( lineLoop, TesselatorLine::kLineMode );
				t.SetInnerWidth( GetInnerWidth() );
				t.SetOuterWidth( GetOuterWidth() );
				t.GenerateStroke( *dst );
			}
			lineLoop.Clear();
			
			if( &vertices != dst)
			{
				if(dst->Length()>0)
				{
					vertices.Append( Vertex2(vertices[vertices.Length()-1]) );
					vertices.Append(dst->operator[](0));
					for(int i=0;i<dst->Length(); i++)
					{
						vertices.Append(dst->operator[](i));
					}
				}
				Rtt_DELETE(dst);
			}
		}
	}
	else if (fMeshType == Geometry::kTriangleFan)
	{
		TesselatorLine t( fMesh, TesselatorLine::kLoopMode );
		t.SetInnerWidth( GetInnerWidth() );
		t.SetOuterWidth( GetOuterWidth() );
		t.GenerateStroke( vertices );
	}
	else if (fMeshType == Geometry::kTriangleStrip )
	{
		ArrayVertex2 lineLoop(fMesh.Allocator());
		lineLoop.Append(fMesh[0]);
		for(int i = 1; i<fMesh.Length(); i+=2 )
		{
			lineLoop.Append(fMesh[i]);
		}
		for (int i = fMesh.Length() + fMesh.Length()%2 - 2; i>0; i-=2) {
			lineLoop.Append(fMesh[i]);
		}
		TesselatorLine t( lineLoop, TesselatorLine::kLoopMode );
		t.SetInnerWidth( GetInnerWidth() );
		t.SetOuterWidth( GetOuterWidth() );
		t.GenerateStroke( vertices );
	}
}

void
TesselatorMesh::GetSelfBounds( Rect& rect )
{
	Update();
	rect = fSelfBounds;
}

Geometry::PrimitiveType
TesselatorMesh::GetFillPrimitive() const
{
	return fMeshType;
}

U32
TesselatorMesh::FillVertexCount() const
{
	return fMesh.Length();
}

U32
TesselatorMesh::StrokeVertexCount() const
{
	if (-1 == fStrokeCount) // TODO: ideal would be to pick apart GenerateStroke() without actually adding indices / vertices or doing the final stroke
	{
		ArrayVertex2 verts( fMesh.Allocator() );
		TesselatorMesh dummy( fMesh.Allocator(), fMeshType );

		dummy.fMesh.Reserve( fMesh.Length() );

		for (int i = 0, iMax = fMesh.Length(); i < iMax; ++i)
		{
			dummy.fMesh.Append( fMesh[i] );
		}

		dummy.fIndices.Reserve( fIndices.Length() );

		for (int i = 0, iMax = fIndices.Length(); i < iMax; ++i)
		{
			dummy.fIndices.Append( fIndices[i] );
		}

		dummy.SetWidth( Rtt_REAL_1 );
		dummy.GenerateStroke( verts );

		fStrokeCount = S32( verts.Length() );
	}

	return U32( fStrokeCount );
}

void
TesselatorMesh::Invalidate()
{
	fIsMeshValid = false;
}

void
TesselatorMesh::Update()
{
	if (!fIsMeshValid)
	{
		fIsMeshValid = true;
		
		fSelfBounds.SetEmpty();
		for (int i=0; i<fMesh.Length(); i++)
		{
			fSelfBounds.Union( fMesh[i] );
		}
		
		if ( fUVs.Length() != fMesh.Length() )
		{
			fUVs.Empty();
			fUVs.Reserve( fMesh.Length() );
			Real invW = 0;
			if (!Rtt_RealIsZero(fSelfBounds.Width()))
			{
				invW = Rtt_RealDiv(1, fSelfBounds.Width());
			}
			Real invH = 0;
			if (!Rtt_RealIsZero(fSelfBounds.Height()))
			{
				invH = Rtt_RealDiv(1, fSelfBounds.Height());
			}
			for (int i=0; i<fMesh.Length(); i++)
			{
				Vertex2 v = fMesh[i];
				v.x = ( v.x - fSelfBounds.xMin ) * invW;
				v.y = ( v.y - fSelfBounds.yMin ) * invH;
				fUVs.Append(v);
			}
		}
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

