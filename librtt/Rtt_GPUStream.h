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

#ifndef _Rtt_GPUStream_H__
#define _Rtt_GPUStream_H__

// ----------------------------------------------------------------------------

#include "Rtt_RenderingStream.h"

#include "Display/Rtt_Paint.h"
#include "Rtt_GPU.h"

#if defined( Rtt_AUTHORING_SIMULATOR ) || defined( Rtt_ANDROID_ENV )
#define RTT_SURFACE_ROTATION
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

class SurfaceProperties;
class VertexCache;

// ----------------------------------------------------------------------------

class GPUStream : public RenderingStream
{
	public:
		typedef RenderingStream Super;
		typedef GPUStream Self;

#ifdef Rtt_DEBUG
	public:
		static void ToggleWireframe();
#endif

	public:
		enum
		{
			kMaxTextureStackDepth = 32
		};

	public:
		static int GetMaxTextureUnits();

	public:
		static GLenum GetDataType();

	public:
		GPUStream( Rtt_Allocator* );
		virtual ~GPUStream();

		virtual void Initialize(
						const PlatformSurface& surface,
						DeviceOrientation::Type contentOrientation );

		virtual void PrepareToRender();
		virtual void UpdateProjection( bool useOriginalOrientation );

		// Call whenever the actual window size you render to changes
		virtual void UpdateViewport( S32 windowWidth, S32 windowHeight );

		virtual S32 GetRenderedContentWidth() const;
		virtual S32 GetRenderedContentHeight() const;
		virtual Rtt_Real ViewableContentWidth() const;
		virtual Rtt_Real ViewableContentHeight() const;
		virtual Rtt_Real ActualContentWidth() const;
		virtual Rtt_Real ActualContentHeight() const;
		virtual void ContentToPixels( S32& x, S32& y, S32& w, S32& h ) const;

	protected:
		void UpdateProjection( S32 contentWidth, S32 contentHeight );

		static Real CalculateAlignmentOffset( Alignment alignment, Real contentLen, Real windowLen );
		void UpdateOffsets( S32 renderedContentWidth, S32 renderedContentHeight );

		virtual void Reshape( S32 contentWidth, S32 contentHeight );

	protected:
		bool UpdateContentOrientation( DeviceOrientation::Type newOrientation );

		virtual void SetContentOrientation( DeviceOrientation::Type newOrientation );
#ifdef RTT_SURFACE_ROTATION
		virtual void SetOrientation( DeviceOrientation::Type newOrientation, bool autoRotate );
#endif

	public:
#ifdef RTT_SURFACE_ROTATION
		// Returns relative orientation between content and surface factoring in
		// any modelview fRotation angle
		virtual DeviceOrientation::Type GetRelativeOrientation() const;
#endif

	protected:
		void PushMaskInternal( const BitmapPaint *mask, bool forceTextureCombiner );

	public:
		virtual void SetMatrixMode( MatrixMode mode );
		virtual void LoadIdentity();

	public:
		virtual void SetEnabled( CapabilityMask mask, bool value );

	public:
		virtual void SetTextureId( U32 textureId );
		virtual void SetTextureParameter( TextureParameter param, TextureValue value );
		virtual void SetTextureFunction( TextureFunction func );

	public:
		virtual void SetTextureCoordinates( const Vertex2 *coords, S32 numCoords );
		virtual void PushTexture( const BitmapPaint *bitmap );
		virtual const BitmapPaint* PopTexture();
		virtual int GetTextureDepth() const;
		int GetActiveTextureCount() const { return fTextureStackNumActiveFrames; }
		int GetActiveTextureIndex() const { return fTextureStackNumActiveFrames - 1; }

	public:
		virtual void SetClearColor( const Paint& paint );
		virtual RGBA GetClearColor();
		virtual void Clear();

	public:
		virtual void SetColor( const RGBA& color );

	public:
		virtual void BeginPaint( const Paint* paint );
		virtual void EndPaint();

	protected:
		const BitmapPaint* GetBitmap() const;

	public:
		virtual U8 GetAlpha() const;
		virtual U8 SetAlpha( U8 newValue, bool accumuluate );

		virtual void CaptureFrameBuffer( BufferBitmap& outBuffer, S32 x, S32 y, S32 w, S32 h );

	public:
		virtual void Begin3D( const Xform3D*, const Rect& bounds );
		virtual void End3D( const Xform3D* );

	public:
		virtual S32 GetRelativeRotation(){return fRotation;}
	

	protected:
		void DrawQuad( const Quad vertices );

	protected:
		const Paint* fCurrentPaint;
		const Vertex2 *fTextureCoords;
		S32 fNumTextureCoords;
		S32 fRotation; // Only used when Rtt_AUTHORING_SIMULATOR is defined
		bool fIsTexture;
		U8 fAlpha;
		U8 fAutoRotate;
		U8 fTextureStackSize;

	private:
		struct TextureStackFrame
		{
			const BitmapPaint *paint;
			int depth;
			bool requiresCombiner;
		};
		int fTextureStackDepth;
		int fTextureStackNumActiveFrames;
		TextureStackFrame fTextureStack[kMaxTextureStackDepth];

	private:
		GLint fWindowWidth;
		GLint fWindowHeight;
		S32 fRenderedContentWidth; // width of rect in which content is rendered (not necessarily same as content width)
		S32 fRenderedContentHeight; // height of rect in which content is rendered (not necessarily same as content height)

		// Frustum
		S32 fOrientedContentWidth; // Current width of the surface (can change due to orientation)
		S32 fOrientedContentHeight; // Current height of the surface (can change due to orientation)
		Real fNear;
		Real fFar;

		// Modelview
		Real fXVertexOffset;
		Real fYVertexOffset;

		// Current state
		RGBA fColor;
		TextureFunction fTextureFunction;

		// Clear color
		GLclampf fClearR;
		GLclampf fClearG;
		GLclampf fClearB;
		GLclampf fClearA;
		
	protected:
		Rtt_Allocator* fAllocator;
};

// ----------------------------------------------------------------------------

#ifdef OLD_GRAPHICS

class GPUMultisampleStream : public GPUStream
{
	public:
		typedef GPUStream Super;
		typedef GPUMultisampleStream Self;

	public:
		GPUMultisampleStream( Rtt_Allocator* );

	public:
		virtual void BeginPaint( const Paint* paint );
		virtual void EndPaint();

	public:
		virtual void Submit( RenderTypes::Mode mode, const Vertex2* vertices, const S32* counts, S32 numCounts );
		virtual void Submit( const Quad vertices );
		virtual void Submit( const Vertex2& origin, Real halfW, Real halfH, Real radius );
		virtual void Submit( const Vertex2& origin, Real radius );
		virtual void Submit( const Vertex2& origin, Real a, Real b );

	protected:
		const Paint* fPaint;
};
#endif

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_GPUStream_H__
