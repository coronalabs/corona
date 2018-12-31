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

#ifndef _Rtt_OpenPath_H__
#define _Rtt_OpenPath_H__

#include "Display/Rtt_DisplayPath.h"
#include "Renderer/Rtt_RenderData.h"
#include "Rtt_VertexCache.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Paint;

// ----------------------------------------------------------------------------

class OpenPath : public DisplayPath
{
	Rtt_CLASS_NO_COPIES( OpenPath )

	protected:
		enum _Constants
		{
			kIsValid = 0x1,
			kIsStrokeCacheAllocated = 0x2,

			kNumConstants
		};

	public:
		enum _DirtyMask
		{
			kStroke = 0x1,					// Stroke vertices in renderdata
			kStrokeTexture = 0x2,			// Stroke tex coords in renderdata
			kStrokeSource = 0x4,			// Stroke tesselation
			kStrokeSourceTexture = 0x8,		// Stroke tex tesselation

			kDefault = kStrokeSource,
		};
		typedef U8 DirtyFlags;

	public:
		OpenPath( Rtt_Allocator* pAllocator );
		virtual ~OpenPath();

	protected:
		void Tesselate();

	public:
		virtual void Update( RenderData& data, const Matrix& srcToDstSpace );
		virtual void UpdateResources( Renderer& renderer ) const;
		virtual void Translate( Real dx, Real dy );
		virtual void GetSelfBounds( Rect& rect ) const;

	public:
		void UpdatePaint( RenderData& data );
		void UpdateColor( RenderData& data, U8 objectAlpha );

	public:
		Rtt_INLINE Paint* GetStroke() const { return fStroke; }
		void SetStroke( Paint* newValue );

		Rtt_INLINE Real GetWidth() const { return fWidth; }
		void SetWidth( Real newValue );

		Rtt_INLINE bool HasStroke() const { return fStroke && GetWidth() > Rtt_REAL_0; }

	public:
		void Insert( S32 index, const Vertex2& p );
		void Remove( S32 index );
		Rtt_INLINE void Append( const Vertex2& p ) { Insert( -1, p ); }
		Rtt_INLINE S32 NumVertices() const { return fStrokePoints.Length(); }
		ArrayVertex2 &GetVertices(){ return fStrokePoints; }

	public:
		void Invalidate( DirtyFlags flags ) { fDirtyFlags |= flags; }
		bool IsValid( DirtyFlags flags ) const { return 0 == (fDirtyFlags & flags); }

	protected:
		void SetValid( DirtyFlags flags ) { fDirtyFlags &= (~flags); }

	public:
		// Use the PropertyMask constants
		// Make properties only read-only to the public
		Rtt_INLINE bool IsProperty( U32 mask ) const { return (fProperties & mask) != 0; }

	protected:
		Rtt_INLINE void ToggleProperty( U32 mask ) { fProperties ^= mask; }
		Rtt_INLINE void SetProperty( U16 mask, bool value )
		{
			const U16 p = fProperties;
			fProperties = ( value ? p | mask : p & ~mask );
		}

	public:
		bool IsStrokeVisible() const;

	private:
		Paint* fStroke;
		Geometry *fStrokeGeometry;
		VertexCache fStrokeSource;
		ArrayVertex2 fStrokePoints;
		Rect fBounds; // self bounds
		Real fWidth;
		U8 fProperties;
		U8 fDirtyFlags;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_OpenPath_H__
