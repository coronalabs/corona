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

#ifndef _Rtt_DisplayV2_H__
#define _Rtt_DisplayV2_H__

#if 0

#include "Display/Rtt_Display.h"


// ----------------------------------------------------------------------------

namespace Rtt
{

class PlatformSurface;
class Renderer;

// ----------------------------------------------------------------------------

class DisplayV2 : public Display
{
	public:
		typedef Display Super;
	public:
		DisplayV2( Runtime& owner );
		virtual ~DisplayV2();

	public:
		virtual bool Initialize( lua_State *L, int configIndex, DeviceOrientation::Type orientation );

	protected:
		void ReadRenderingConfig( lua_State *L, int index );
		lua_State *GetL() const;

	protected:
		void Reshape();

	public:
		virtual void Start();
		virtual void Restart();
		virtual void Restart( DeviceOrientation::Type orientation );

	public:
		virtual void Render();
		virtual void Blit();
		virtual void Invalidate();
		virtual StageObject *GetStage();
		virtual StageObject *GetStageOffscreen();
		virtual BitmapPaint *Capture( DisplayObject *object,
										Rect *screenBounds,
										bool will_be_saved_to_file );

	public:
		virtual void UnloadResources();
		virtual void ReloadResources();

	public:
		virtual void GetImageSuffix( String& outSuffix ) const;
		virtual bool GetImageFilename( const char *filename, MPlatform::Directory baseDir, String& outFilename ) const;

	public:
		virtual GroupObject *Overlay();
		virtual GroupObject *Orphanage();
		virtual GroupObject *HitTestOrphanage();

	public:
		virtual S32 RenderedContentWidth() const;
		virtual S32 RenderedContentHeight() const;
		virtual S32 ViewableContentWidth() const;
		virtual S32 ViewableContentHeight() const;
		virtual Rtt_Real ActualContentWidth() const;
		virtual Rtt_Real ActualContentHeight() const;
		virtual S32 WindowWidth() const;
		virtual S32 WindowHeight() const;

	public:
		virtual S32 DeviceWidth() const;
		virtual S32 DeviceHeight() const;
		virtual S32 ScaledWidth() const;
		virtual S32 ScaledHeight() const;

	public:
		// Dynamic Content Scaling
		virtual S32 ContentWidth() const;
		virtual S32 ContentHeight() const;
		virtual S32 ScreenWidth() const;
		virtual S32 ScreenHeight() const;

		virtual Real GetSx() const;
		virtual Real GetSy() const;

		virtual Real GetXOriginOffset() const;
		virtual Real GetYOriginOffset() const;

		// Width and height in OS-defined points
		virtual Real PointsWidth() const;
		virtual Real PointsHeight() const;

		virtual void SetScaleMode( ScaleMode mode, Rtt_Real screenWidth, Rtt_Real screenHeight );
		virtual ScaleMode GetScaleMode() const;
		virtual void ContentToScreen( S32& x, S32& y, S32& w, S32& h ) const;
		virtual void ContentToPixels( S32& x, S32& y, S32& w, S32& h ) const;

		virtual void GetViewProjectionMatrix(glm::mat4 &viewMatrix, glm::mat4 &projMatrix);

	public:
		// Generalized function for calculating proper content scaling factors
		static void UpdateContentScale(
						Rtt_Real screenWidth, Rtt_Real screenHeight,
						S32 contentWidth, S32 contentHeight,
						ScaleMode scaleMode,
						Rtt_Real& outSx, Rtt_Real& outSy );

		virtual void CalculateContentToScreenScale( Real& outSx, Real& outSy ) const;
		virtual void GetContentRect( Rect& outRect ) const;
		virtual const Rect& GetScreenContentBounds() const;

		virtual void SetContentOrientation( DeviceOrientation::Type newOrientation );

		virtual void WindowDidRotate( DeviceOrientation::Type newOrientation, bool isNewOrientationSupported );

		// Call when window size changes so viewport of GPUStream can be updated.
		// Implicitly calls UpdateContentScale()
		virtual void WindowSizeChanged();

		virtual DeviceOrientation::Type GetRelativeOrientation() const;
		virtual DeviceOrientation::Type GetLaunchOrientation() const;
		virtual DeviceOrientation::Type GetContentOrientation() const;
		virtual DeviceOrientation::Type GetSurfaceOrientation() const;
	public:
		Renderer& GetRenderer() { return *fRenderer; }
		const Renderer& GetRenderer() const { return *fRenderer; }

		virtual void SetWireframe( bool newValue );

#if defined( Rtt_ANDROID_ENV ) && TEMPORARY_HACK
		// TODO: Remove this once TEMPORARY_HACK is removed in JavaToNativeBridge.cpp
		RenderingStream& GetStream() { return * fStream; }
#endif
	private:
		Renderer *fRenderer;

		// TODO: Refactor data structure portions out
		// We temporarily use RenderingStream b/c it contains key data
		// about window size/orientation
		RenderingStream *fStream;

		PlatformSurface *fTarget;
		int fImageSuffix;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
#endif

#endif // _Rtt_DisplayV2_H__
