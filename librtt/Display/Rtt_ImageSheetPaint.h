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

#ifndef _Rtt_ImageSheetPaint_H__
#define _Rtt_ImageSheetPaint_H__

#include "Display/Rtt_BitmapPaint.h"

#include "Core/Rtt_AutoPtr.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class ImageFrame;
class ImageSheet;

// ----------------------------------------------------------------------------

class ImageSheetPaint : public BitmapPaint
{
	Rtt_CLASS_NO_COPIES( ImageSheetPaint )

	public:
		typedef BitmapPaint Super;

	public:
		// Load bitmap from sheet
		static ImageSheetPaint* NewBitmap( Rtt_Allocator *allocator, const AutoPtr< ImageSheet >& sheet );

		// Load bitmap from sheet
		static ImageSheetPaint* NewBitmap( Rtt_Allocator *allocator, const AutoPtr< ImageSheet >& sheet, int frameIndex );

	public:
		ImageSheetPaint( const AutoPtr< ImageSheet >& sheet, int frameIndex = -1 );
		virtual ~ImageSheetPaint();

	public:
		// Paint
		virtual const Paint *AsPaint( Type t ) const;

	public:
		virtual const MLuaUserdataAdapter& GetAdapter() const override;
		virtual const AutoPtr< ImageSheet >& GetSheet() const;
		int GetNumFrames() const;	
		void SetFrame( int frameIndex );
		int GetFrame() const;
		const ImageFrame *GetImageFrame() const;
	
		virtual void ApplyPaintUVTransformations( ArrayVertex2& vertices ) const override;

	private:
		AutoPtr< ImageSheet > fSheet;
		int fFrameIndex; // 0-based index
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ImageSheetPaint_H__
