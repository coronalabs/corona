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

#ifndef __Rtt_ImageFrame__
#define __Rtt_ImageFrame__

#include "Core/Rtt_Geometry.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class ImageSheet;

// ----------------------------------------------------------------------------

class ImageFrame
{
	public:
		// location/size (x,y,w,h) in content coordinates
		// scale factor (sx,sy), the ratio of pixels coords to content coords
		ImageFrame( ImageSheet& owner, S32 x, S32 y, S32 w, S32 h, Real sx, Real sy, bool intrudeHalfTexel );

	public:
		void SetTrimData( S32 srcX, S32 srcY, S32 srcW, S32 srcH );
		bool IsTrimmed() const { return fIsTrimmed; }

	public:
		void UpdateUVVertex(Vertex2& vert ) const;

/*
	public:
		bool IsValid() const { return fTextureCoords[0].x != kCoordinateNegInfinity; }

	protected:
		void Invalidate() { fTextureCoords[0].x = kCoordinateNegInfinity; }

	public:
		void SetTrimData( S32 srcX, S32 srcY, S32 srcW, S32 srcH );
		S32 GetSrcX() const { return fSrcX; }
		S32 GetSrcY() const { return fSrcY; }
*/

	public:
		// Content coords
		S32 GetWidth() const { return fWidth; }
		S32 GetHeight() const { return fHeight; }

		// Pixels
		S32 GetPixelX() const { return (S32)( fX * fSx ); }
		S32 GetPixelY() const { return (S32)( fY * fSy ); }
		S32 GetPixelW() const { return (S32)( fWidth * fSx ); }
		S32 GetPixelH() const { return (S32)( fHeight * fSy ); }

	public:
		// Content coords
		S32 GetSrcWidth() const { return fSrcWidth; }
		S32 GetSrcHeight() const { return fSrcHeight; }
		Real GetOffsetX() const { return fOffsetX; }
		Real GetOffsetY() const { return fOffsetY; }

	private:
		ImageSheet& fOwner;

		// frame top-left position and size within texture
		S32 fX;
		S32 fY;
		S32 fWidth;
		S32 fHeight;
		Real fSx;
		Real fSy;

		// size of original source image for frame
		S32 fSrcWidth;
		S32 fSrcHeight;

		Real fOffsetX;
		Real fOffsetY;

		// texture coord rect
		Rect fTextureCoords;

		bool fIsTrimmed;
		// texture coordinates for frame
//		Quad fTextureCoords; 
/*
		// if frame is cropped/trimmed from original, then following
		// attributes are greater than zero:
		S32 fSrcX;
		S32 fSrcY;
*/
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_ImageFrame__
