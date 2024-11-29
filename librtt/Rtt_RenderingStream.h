//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_RenderingStream_H__
#define _Rtt_RenderingStream_H__

// ----------------------------------------------------------------------------

#include "Core/Rtt_Geometry.h"
#include "Core/Rtt_Types.h"

#include "Display/Rtt_Display.h"
#include "Renderer/Rtt_RenderTypes.h"
#include "Rtt_DeviceOrientation.h"

#include "Rtt_SurfaceInfo.h"

#define Rtt_3D	1

// ----------------------------------------------------------------------------

namespace Rtt
{

class PlatformBitmap;
class BitmapPaint;
class BufferBitmap;
class MBitmapDelegate;
class Paint;
class PlatformSurface;
class SurfaceProperties;
struct Xform3D;
class VertexArray;

// ----------------------------------------------------------------------------

class RenderingStream
{
	public:
		typedef RenderingStream Self;

		typedef enum _Properties
		{
			kInitialized = 0x1,
			kNonZeroWinding = 0x2,
			kFlipHorizontalAxis = 0x4,
			kFlipVerticalAxis = 0x8,
			kInhibitSwap = 0x10
		}
		Properties;

		typedef enum _RotateMode
		{
			kResetRotate = 0,
			kAutoRotate,
			kRelativeRotate
		}
		RotateMode;


		typedef enum _MatrixMode
		{
			kMatrixModeModelView = 0,
			kMatrixModeProjection,
			kMatrixModeTexture,

			kNumMatrixMode
		}
		MatrixMode;

		typedef enum _CapabilityMask
		{
			kCapabilityTexture2D = 0x1
		}
		CapabilityMask;

		typedef enum _TextureParameter
		{
			kTextureUnknown = -1,
			kTextureMagFilter = 0,
			kTextureMinFilter,
			kTextureWrapS,
			kTextureWrapT,
			kTextureMipMap,

			kNumTextureParameter
		}
		TextureParameter;

		typedef enum _TextureValue
		{
			kTextureFalse = 0,
			kTextureTrue,
			kTextureNearest,
			kTextureLinear,
			kTextureNearestMipMapNearest,
			kTextureLinearMipMapNearest,
			kTextureNearestMipMapLinear,
			kTextureLinearMipMapLinear,
			kTextureRepeat,
			kTextureClampToEdge,
			
			kNumTextureValue
		}
		TextureValue;

		typedef enum _TextureFunction
		{
			kTextureFunctionUnknown = -1,
			kTextureFunctionAdd = 0,
			kTextureFunctionModulate,
			kTextureFunctionDecal,
			kTextureFunctionBlend,
			kTextureFunctionReplace,
			kTextureFunctionCombine,

			kNumTextureFunction
		}
		TextureFunction;

	public:
		RenderingStream();
		virtual ~RenderingStream() = 0;

		// Call to initialize geometry properties. Should be called before PrepareToRender().
		// 
		// To support dynamic content scaling, the rendering stream needs to know 
		// how to scale up the content so that it fills the window. The renderer
		// only needs to know the rendered content bounds (the window width in 
		// scaled-content units). The rendered bounds are defined by the window bounds
		// and the scale factors
		// renderedContentWidth = windowWidth * contentScaleX
		// renderedContentHeight = windowHeight * contentScaleY
		void Preinitialize( S32 contentW, S32 contentH );
		virtual void Initialize(
				const PlatformSurface& surface,
				DeviceOrientation::Type launchOrientation );
		virtual void Reinitialize(
				const PlatformSurface& surface,
				DeviceOrientation::Type launchOrientation );

		// Call prior to rendering, i.e. any submission to the stream
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

	public:
		// Returns rect of screen in content coordinates
		const Rect& GetScreenContentBounds() const { return fScreenContentBounds; }

	protected:
		Rect& GetScreenContentBounds() { return fScreenContentBounds; }

	protected:
		virtual void Reshape( S32 renderedContentWidth, S32 renderedContentHeight );

	public:
		static bool ShouldSwap( DeviceOrientation::Type srcOrientation, DeviceOrientation::Type dstOrientation );
		void SetSurfaceOrientation( DeviceOrientation::Type newOrientation ) 
		{ 
			Rtt_ASSERT( newOrientation >= DeviceOrientation::kUnknown && newOrientation <= DeviceOrientation::kNumTypes );
			fSurfaceOrientation = newOrientation; 
		}
		virtual void SetContentOrientation( DeviceOrientation::Type newOrientation );
		virtual void SetOrientation( DeviceOrientation::Type newOrientation, bool autoRotate );

	public:
		virtual DeviceOrientation::Type GetRelativeOrientation() const;

	public:
		DeviceOrientation::Type GetLaunchOrientation() const { return (DeviceOrientation::Type)fLaunchOrientation; }
		DeviceOrientation::Type GetContentOrientation() const { return (DeviceOrientation::Type)fContentOrientation; }
		DeviceOrientation::Type GetSurfaceOrientation() const { return (DeviceOrientation::Type)fSurfaceOrientation; }

	public:
		virtual void SetMatrixMode( MatrixMode mode );
		virtual void LoadIdentity();

	public:
		bool IsEnabled( CapabilityMask mask );
		virtual void SetEnabled( CapabilityMask mask, bool value );

	public:
		virtual void SetTextureId( U32 textureId );
		virtual void SetTextureParameter( TextureParameter param, TextureValue value );
		virtual void SetTextureFunction( TextureFunction func );

	public:
		virtual void SetTextureCoordinates( const Vertex2 *coords, S32 numCoords );
		virtual void PushTexture( const BitmapPaint *paint );
		virtual const BitmapPaint* PopTexture();
		virtual int GetTextureDepth() const;

	public:
		virtual void SetClearColor( const Paint& paint );
		virtual RGBA GetClearColor();
		virtual void Clear();

	public:
		virtual void SetColor( const RGBA& color );
		virtual void BeginPaint( const Paint* paint );
		virtual void EndPaint();

		// Currently it's the Paint's responsibility to apply the "global" alpha
		// to the Paint's color/texture/etc
		virtual U8 GetAlpha() const;

		// Each DisplayObject is responsible for saving the current alpha
		// and restoring it at the begin/end of each "Draw" call.
		virtual U8 SetAlpha( U8 newValue, bool accumulate );

		// Assumes you pass screen coordinates, not content coordinates (most code is in content coordinates)
		virtual void CaptureFrameBuffer( BufferBitmap& outBuffer, S32 xScreen, S32 yScreen, S32 wScreen, S32 hScreen );

	public:
		// Experimental
		virtual void Begin3D( const Xform3D*, const Rect& bounds );
		virtual void End3D( const Xform3D* );

	public:
		S32 DeviceWidth() const { return fDeviceWidth; }
		S32 DeviceHeight() const { return fDeviceHeight; }

	public:
		// Dynamic Content Scaling
		S32 ContentWidth() const { return fContentWidth; }
		S32 ContentHeight() const { return fContentHeight; }
		S32 ScreenWidth() const;
		S32 ScreenHeight() const;

		Real GetSx() const { return fSx; }
		Real GetSy() const { return fSy; }

		Real GetXOriginOffset() const { return fXOriginOffset; }
		Real GetYOriginOffset() const { return fYOriginOffset; }

		void SetXOriginOffset( Real newValue ) { fXOriginOffset = newValue; }
		void SetYOriginOffset( Real newValue ) { fYOriginOffset = newValue; }

		void SwapContentSize();
		void SwapContentScale();
        void SwapContentAlign();

		// Sets how the content is aligned
		void SetContentAlignment( Alignment xAlign, Alignment yAlign );
		Alignment GetXAlign() const { return (Alignment)fXAlign; }
		Alignment GetYAlign() const { return (Alignment)fYAlign; }

		// Sets the scaling mode and updates content scale factors based on window size
		void SetScaleMode( Display::ScaleMode mode, Rtt_Real screenWidth, Rtt_Real screenHeight );
		Display::ScaleMode GetScaleMode() const { return (Display::ScaleMode)fScaleMode; }

		void ContentToScreenUnrounded( float& x, float& y ) const;
		void ContentToScreenUnrounded( float& x, float& y, float& w, float& h ) const;

		void ContentToScreen( S32& x, S32& y ) const;
		void ContentToScreen( S32& x, S32& y, S32& w, S32& h ) const;
		void ContentToScreen( Rtt_Real& x, Rtt_Real& y, Rtt_Real& w, Rtt_Real& h ) const;
		void ContentToPixels( S32& x, S32& y ) const;
		virtual void ContentToPixels( S32& x, S32& y, S32& w, S32& h ) const;

		// Call this method when the window size changes
		void UpdateContentScale( Rtt_Real screenWidth, Rtt_Real screenHeight );

	protected:
		const Quad* SubmitBounds() const { return fSubmitBounds; }

	public:
		void WillSubmitArray( const Quad& submitBounds );
		void DidSubmitArray();

		Rtt_FORCE_INLINE bool IsProperty( U32 mask ) const { return (fProperties & mask) != 0; }
		Rtt_INLINE void ToggleProperty( U32 mask ) { fProperties ^= mask; }
		void SetProperty( U32 mask, bool value );

	public:
		virtual S32 GetRelativeRotation(){return 0;}


	protected:
//		const Quad* SetCurrentMask( const Quad* newMask );
//		friend class SimpleMasker;

/*
		virtual void Submit( const VertexArray& vertices ) = 0;

		// General case: submit vertex arrays, requires calling 
		virtual void Submit( RenderTypes::Mode mode, const Vertex2* vertices, const S32* counts, S32 numCounts ) = 0;
		// virtual void Submit( const Mode modes[], const VertexCache caches[], S32 len ) = 0;

	public:
		// Optimized primitives: rect, rounded rect, circle, ellipse
		// Intended to be called only when the srcToDst transform preserves the
		// aspect ratio (uniform scale along x and y) and preserves orientation 
		// (no rotation)
		virtual void Submit( const Quad vertices ) = 0;
		virtual void Submit( const Vertex2& origin, Real halfW, Real halfH, Real radius ) = 0;
		virtual void Submit( const Vertex2& origin, Real radius ) = 0;
		virtual void Submit( const Vertex2& origin, Real a, Real b ) = 0;
*/
	private:
		const Quad* fSubmitBounds;
		U8 fProperties;
		S8 fXAlign;
		S8 fYAlign;
		S8 fBlendMode;
		U8 fLaunchOrientation;
		U8 fContentOrientation;
		U8 fSurfaceOrientation;
		U8 fScaleMode;
//		const Quad* fCurrentMask;
		Real fSx;
		Real fSy;
		S32 fDeviceWidth;
		S32 fDeviceHeight;
		S32 fContentWidth;	// Content size is the size at which the content was originally authored
		S32 fContentHeight;
		Real fXOriginOffset; // x-distance (in content units) from screen origin to content origin.
		Real fYOriginOffset; // y-distance (in content units) from screen origin to content origin.
		Rect fScreenContentBounds; // The rect of the screen in content coordinates
		U32 fCapabilities;

	// Provide access to SetContentOrientation()
	friend class DisplayV1;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_RenderingStream_H__
