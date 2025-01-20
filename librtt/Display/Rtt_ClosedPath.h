//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ClosedPath_H__
#define _Rtt_ClosedPath_H__

#include "Core/Rtt_Matrix.h"
#include "Core/Rtt_Geometry.h"
#include "Display/Rtt_DisplayPath.h"
#include "Renderer/Rtt_RenderData.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Paint;
class Renderer;

// ----------------------------------------------------------------------------

class ClosedPath : public DisplayPath
{
	Rtt_CLASS_NO_COPIES( ClosedPath )

	public:
		enum _Constants
		{
			kIsRectPath = 0x1,
			kIsFillWeakReference = 0x2,
			kIsFillLocked = 0x4,
		};
		typedef U8 Properties;

		enum _DirtyMask
		{
			kStroke = 0x1,					// Stroke vertices in renderdata
			kStrokeTexture = 0x2,			// Stroke tex coords in renderdata
			kStrokeSource = 0x4,			// Stroke tesselation
			kStrokeSourceTexture = 0x8,		// Stroke tex tesselation

			kFill = 0x10,					// Fill vertices in renderdata
			kFillTexture = 0x20,			// Fill tex coords in renderdata
			kFillSource = 0x40,				// Fill tesselation
			kFillSourceTexture = 0x80,		// Fill tex tesselation
			
			kFillIndices = 0x100,
			kFillSourceIndices = 0x200,

			kDefault = kFillSource,
		};
		typedef U16 DirtyFlags;

	public:
		ClosedPath( Rtt_Allocator* pAllocator );
		virtual ~ClosedPath();

	public:
		virtual void Update( RenderData& data, const Matrix& srcToDstSpace );
		virtual void Translate( Real dx, Real dy );

	public:
		// Returns true if bounds was actually changed; returns false if no-op.
		virtual bool SetSelfBounds( Real width, Real height );

	public:
		void UpdatePaint( RenderData& data );
		void UpdateColor( RenderData& data, U8 objectAlpha );

	public:
		void SetStrokeData( RenderData *data ) { fStrokeData = data; }

	protected:
		RenderData *GetStrokeData() { return ( HasStroke() ? fStrokeData : NULL ); }

	public:
		Paint* GetFill() { return fFill; }
		const Paint* GetFill() const { return fFill; }
		void SetFill( Paint* newValue );
		void SetFillWeakReference( bool newValue ) { SetProperty( kIsFillWeakReference, newValue ); }
		void SetFillLocked( bool newValue ) { SetProperty( kIsFillLocked, newValue ); }

	public:
		void SwapFill( ClosedPath& rhs );

		Paint* GetStroke() { return fStroke; }
		const Paint* GetStroke() const { return fStroke; }
		void SetStroke( Paint* newValue );

		Rtt_INLINE U8 GetStrokeWidth() const { return fInnerStrokeWidth + fOuterStrokeWidth; }
		Rtt_INLINE U8 GetInnerStrokeWidth() const { return fInnerStrokeWidth; }
		Rtt_INLINE U8 GetOuterStrokeWidth() const { return fOuterStrokeWidth; }

		void SetInnerStrokeWidth( U8 newValue );
		void SetOuterStrokeWidth( U8 newValue );

		bool HasFill() const { return ( NULL != fFill ); }
		bool HasStroke() const { return ( NULL != fStroke ); }
	
		bool IsFillVisible() const;
		bool IsStrokeVisible() const;

	public:
		void Invalidate( DirtyFlags flags ) { fDirtyFlags |= flags; }
		bool IsValid( DirtyFlags flags ) const { return 0 == (fDirtyFlags & flags); }

	protected:
		void SetValid( DirtyFlags flags ) { fDirtyFlags &= (~flags); }

	public:
		// Use the PropertyMask constants
		// Make properties only read-only to the public
		bool IsProperty( Properties mask ) const { return (fProperties & mask) != 0; }

	protected:
		void ToggleProperty( Properties mask ) { fProperties ^= mask; }
		void SetProperty( Properties mask, bool value )
		{
			const Properties p = fProperties;
			fProperties = ( value ? p | mask : p & ~mask );
		}

	private:
		Paint* fFill; // Only one fill color per path
		Paint* fStroke;

		RenderData *fStrokeData;

		Properties fProperties;
		DirtyFlags fDirtyFlags;
		U8 fInnerStrokeWidth;
		U8 fOuterStrokeWidth;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ClosedPath_H__
