//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ImageFrame.h"

#include "Display/Rtt_ImageSheet.h"
#include "Renderer/Rtt_Texture.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

ImageFrame::ImageFrame( ImageSheet& owner, S32 x, S32 y, S32 w, S32 h, Real sx, Real sy, bool intrudeHalfTexel )
:	fOwner( owner ),
	fX( x ),
	fY( y ),
	fWidth( w ),
	fHeight( h ),
	fSx( sx ),
	fSy( sy ),
	fSrcWidth( w ),
	fSrcHeight( h ),
	fOffsetX( Rtt_REAL_0 ),
	fOffsetY( Rtt_REAL_0 ),
	fTextureCoords(),
	fIsTrimmed( false )
/*
	fSrcX( 0 ),
	fSrcY( 0 ),
*/
{
	const SharedPtr< TextureResource >& resource = owner.GetTextureResource();

	// Frame's offset/size inside the source image in pixels
	Real frameX = Rtt_IntToReal( fX );
	Real frameY = Rtt_IntToReal( fY );
	Real frameW = Rtt_IntToReal( fWidth );
	Real frameH = Rtt_IntToReal( fHeight );

	// Map content coordinate values to pixels, since we want the actual frame in the texture
	if ( ! Rtt_RealIsOne( sx ) )
	{
		frameX = Rtt_RealMul( sx, frameX );
		frameW = Rtt_RealMul( sx, frameW );
	}

	if ( ! Rtt_RealIsOne( sy ) )
	{
		frameY = Rtt_RealMul( sy, frameY );
		frameH = Rtt_RealMul( sy, frameH );
	}

	// Size of entire source image in pixels
	const Texture& texture = resource->GetTexture();
	Real sheetW = Rtt_IntToReal( texture.GetWidth() );
	Real sheetH = Rtt_IntToReal( texture.GetHeight() );

	// Get frame's texture rect normalized to [0, 1]
	fTextureCoords.xMin = Rtt_RealDiv( frameX, sheetW );
	fTextureCoords.yMin = Rtt_RealDiv( frameY, sheetH );
	fTextureCoords.xMax = Rtt_RealDiv( frameX + frameW, sheetW );
	fTextureCoords.yMax = Rtt_RealDiv( frameY + frameH, sheetH );
	
	
	if( intrudeHalfTexel && sheetW > 0 && sheetH > 0 )
	{
		// Make sheet points pixel perfect: add and substract half of initial pixel to
		// point to center of the pixel, not it's edge.
		Real halfTexelW = Rtt_RealDiv( Rtt_REAL_HALF, sheetW );
		fTextureCoords.xMin += halfTexelW;
		fTextureCoords.xMax -= halfTexelW;

		Real halfTexelH = Rtt_RealDiv( Rtt_REAL_HALF, sheetH );
		fTextureCoords.yMin += halfTexelH;
		fTextureCoords.yMax -= halfTexelH;
	}
	
#ifdef Rtt_DEBUG
//	fTextureCoords.Trace();
#endif
}
	
void
ImageFrame::UpdateUVVertex(Vertex2& vert ) const
{
	vert.x = fTextureCoords.xMin + fTextureCoords.Width() *vert.x;
	vert.y = fTextureCoords.yMin + fTextureCoords.Height()*vert.y;
}

void
ImageFrame::SetTrimData( S32 srcX, S32 srcY, S32 srcW, S32 srcH )
{
	// Trimmed sprites need to have an offset translation applied to actual RectPath
	if ( srcW > fWidth || srcH > fHeight )
	{
		fSrcWidth = srcW;
		fSrcHeight = srcH;

		// If the image is trimmed, then the rect path corresponding to the
		// frame is not centered at the origin in its local/self coordinates.
		// Rather, the rect path corresponding to the original source frame
		// is centered. Therefore, we need to calculate offsets to move
		// the rect path to the proper location in its local coordinates.
		// 
		// See diagram below. The larger rect is the original source centered
		// about the local origin. The smaller rect is the trimmed frame.
		// The offset is the amount need to translate the smaller rect's center
		// from 'o' to 'x'
		// +-----------------------------------+
		// |               +-------+           |
		// |               |   x   |           |
		// |               +-------+           |
		// |                 o                 |
		// |                                   |
		// |                                   |
		// |                                   |
		// +-----------------------------------+
		fOffsetX = srcX + Rtt_IntToReal( (fWidth - srcW) >> 1 );
		fOffsetY = srcY + Rtt_IntToReal( (fHeight - srcH) >> 1 );
		fIsTrimmed = ! Rtt_RealIsZero( fOffsetX ) || ! Rtt_RealIsZero( fOffsetY );
	}
}


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

