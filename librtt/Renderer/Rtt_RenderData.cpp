//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_RenderData.h"

#include <cstdio>
#include "Renderer/Rtt_Geometry_Renderer.h"

#include "Core/Rtt_Assert.h"
#include "Corona/CoronaGraphics.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

RenderData::RenderData()
:	fGeometry( NULL ),
	fProgram( NULL ),
	fFillTexture0( NULL ),
	fFillTexture1( NULL ),
	fMaskTexture( NULL ),
	fMaskUniform( NULL ),
	fUserUniform0( NULL ),
	fUserUniform1( NULL ),
	fUserUniform2( NULL ),
	fUserUniform3( NULL ),
//	fMinDepthRange( 0.0 ),
//	fMaxDepthRange( 1.0 ),
	fBlendMode(),
	fBlendEquation( RenderTypes::kAddEquation )
{
}

void
RenderData::Log( const char *msg ) const
{
	if ( ! msg )
	{
		msg = "";
	}

	Rtt_LogException( "RenderData(%p) (%s)\n", this, msg );
	Rtt_LogException( "{\n" );
	Rtt_LogException( "\tblendColor [src,dst] = [%s,%s]\n",
		BlendMode::StringForParam( fBlendMode.fSrcColor ),
		BlendMode::StringForParam( fBlendMode.fDstColor ) );
	Rtt_LogException( "\tblendAlpha [src,dst] = [%s,%s]\n",
		BlendMode::StringForParam( fBlendMode.fSrcColor ),
		BlendMode::StringForParam( fBlendMode.fDstColor ) );
	Rtt_LogException( "\tblendEq =   %s\n", RenderTypes::StringForBlendEquation( fBlendEquation ) );
//	Rtt_LogException( "\tdepth =   [%g,%g]\n", fMinDepthRange, fMaxDepthRange );
	Rtt_LogException( "\tprogram = %p\n", fProgram );
	Rtt_LogException( "\tfillTex0 = %p\n", fFillTexture0 );
	Rtt_LogException( "\tfillTex1 = %p\n", fFillTexture1 );
	Rtt_LogException( "\tmaskTex = %p\n", fMaskTexture );
	Rtt_LogException( "\tmaskUniform = %p\n", fMaskUniform );
	Rtt_LogException( "\tgeometry: numVertices(%d)\n", fGeometry->GetVerticesUsed() );
	Geometry::Vertex *vertices = fGeometry->GetVertexData();
	for ( U32 i = 0, iMax = fGeometry->GetVerticesUsed(); i < iMax; i++ )
	{
		Geometry::Vertex& v = vertices[i];
		Rtt_LogException( "\t[%d] = {\n", i );
		Rtt_LogException( "\t\t(x,y,z) =       (%g,%g,%g)\n", v.x, v.y, v.z );
		Rtt_LogException( "\t\t(u,v,q) =       (%g,%g,%g)\n", v.u, v.v, v.q );
		Rtt_LogException( "\t\t(rs,gs,bs,as) = (%02x,%02x,%02x,%02x)\n", v.rs, v.gs, v.bs, v.as );
		Rtt_LogException( "\t\t(ux,uy,uz,uw) = (%g,%g,%g,%g)\n", v.ux, v.uy, v.uz, v.uw );
		Rtt_LogException( "\t}\n" );
	}
	Rtt_LogException( "}\n" );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
