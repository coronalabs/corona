//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
