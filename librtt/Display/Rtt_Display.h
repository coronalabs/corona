//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Display_H__
#define _Rtt_Display_H__

////
//
#define GLM_FORCE_RADIANS
#define GLM_FORCE_CXX98
#include "glm/glm.hpp"
#include "glm/ext.hpp"
//
////

#include "Core/Rtt_Types.h"

#include "Core/Rtt_Array.h"
#include "Core/Rtt_Time.h"

#include "Rtt_DeviceOrientation.h"
#include "Rtt_MPlatform.h"

// ----------------------------------------------------------------------------

struct lua_State;
struct luaL_Reg;

namespace Rtt
{

struct Rect;
struct VertexAttributeSupport;

class BitmapPaint;
class DisplayDefaults;
class DisplayObject;
class GroupObject;
class MDisplayDelegate;
class ProgramHeader;
class Renderer;
class Runtime;
class Scene;
class ShaderFactory;
class SpritePlayer;
class StageObject;
class String;
class TextureFactory;
class PlatformSurface;
class RenderingStream;
class ProfilingState;

// ----------------------------------------------------------------------------

class Display
{
    public:
        typedef enum _DrawMode
        {
            kForceRenderDrawMode = 0,    // Blit every frame
            kInvalidateDrawMode,        // Only blits when invalidated
            kWireframeDrawMode,            // Wireframe
            kPhysicsHybridDrawMode,        // Overlay with Physics bodies
            kPhysicsDebugDrawMode,        // Only draw Physics bodies
            
            kNumDrawModes,
            
            // Default
            kDefaultDrawMode = kInvalidateDrawMode,
        }
        DrawMode;

        typedef enum _ScaleMode
        {
            kNone = 0,
            kZoomEven = 1, // scale both width and height evenly until screen is filled; this can cause portions of image to be clipped
            kZoomStretch = 2, // scale width and height unevenly to fill screen
            kLetterbox = 3, // scale larger dimension to fit, creating a letterbox (blank space on the screen)
            kAdaptive = 4, // use heuristics to determine content width/height

            kNumScaleMode
        }
        ScaleMode;

        // List of all premium features (Pro and Enterprise)
        typedef enum _Feature
        {
            kDisplayNewSnapshot,
            kGraphicsDefineEffectGraph,
            kGraphicsDefineEffectFragment,
            kGraphicsDefineEffectVertex,
//            kPaintEffect,
//            kPaintBlendEquation,
            kObjectPath,
            kObjectFill,
            kObjectStroke,
            kObjectBlendMode,
            kLineStroke,
            kLineBlendMode,
            kDisplayColorSample,
            kGraphicsNewOutline,
            kDisplayNewEmitter,
            kPhysicsNewParticleSystem,

            kNumFeatures
        }
        Feature;
        
        static const char *StringForFeature( Feature value );
        static bool IsProFeature( Feature value );
        static bool IsEnterpriseFeature( Feature value );
        static const char *GetTierString( Feature value );
        static ScaleMode ScaleModeFromString( const char *scaleName );

    public:
        Display( Runtime& owner );
        virtual ~Display();

	public:
		//! \Return true for success. False for failure.
		virtual bool Initialize( lua_State *L, int configIndex, DeviceOrientation::Type orientation, const char * backend, void * backendContext );
		virtual void Teardown();

    protected:
        void ReadRenderingConfig( lua_State *L, int index, ProgramHeader& programHeader );

    public:
        virtual void Start();
        virtual void Restart();
        virtual void Restart( DeviceOrientation::Type orientation );
        virtual void Restart( int newWidth, int newHeight );

    public:
        // Call on a timer tick
        void Update();
        lua_State *GetL() const;

    public:
        // Note: NOTHING in this class requires being virtual.
        // We need to consider removing "virtual" everywhere
        // from this class.
        virtual void Render();
        virtual void Blit();
        virtual void Invalidate();
        virtual StageObject *GetStage();
        virtual StageObject *GetStageOffscreen();

        BitmapPaint *CaptureScreen( bool save_to_file,
                                    bool output_file_will_be_png_format );

        BitmapPaint *CaptureBounds( Rect *screenBounds,
                                    bool will_be_saved_to_file,
                                    bool output_file_will_be_png_format );

        BitmapPaint *CaptureDisplayObject( DisplayObject *object,
                                            bool will_be_saved_to_file,
                                            bool output_file_will_be_png_format,
                                            bool crop_object_to_screen_bounds );

        BitmapPaint *CaptureSave( DisplayObject *object,
                                    bool crop_object_to_screen_bounds,
                                    bool output_file_will_be_png_format,
                                    const ColorUnion *optionalBackgroundColor );

        void ColorSample( float pos_x,
                            float pos_y,
                            RGBA &output_color );

    private:
        virtual BitmapPaint *Capture( DisplayObject *object,
                                        Rect *screenBounds,
                                        bool will_be_saved_to_file,
                                        bool output_file_will_be_png_format,
                                        bool crop_object_to_screen_bounds,
                                        const ColorUnion *optionalBackgroundColor,
                                        RGBA *optional_output_color );

    public:
        virtual void UnloadResources();
        virtual void ReloadResources();

	public:
		virtual void GetImageSuffix( String& outSuffix ) const;
		virtual bool GetImageFilename( const char *filename, MPlatform::Directory baseDir, String& outFilename ) const;
		virtual bool PushImageSuffixTable() const;
    
        void GatherObjectFactories( const luaL_Reg funcs[], void * library );
        bool PushObjectFactories() const;

	public:
		virtual GroupObject *Overlay();
		virtual GroupObject *Orphanage();
		virtual GroupObject *HitTestOrphanage();

    public:
        virtual S32 RenderedContentWidth() const;
        virtual S32 RenderedContentHeight() const;
        virtual S32 ViewableContentWidth() const;
        virtual S32 ViewableContentHeight() const;
        virtual Real ActualContentWidth() const;
        virtual Real ActualContentHeight() const;
        virtual S32 WindowWidth() const;
        virtual S32 WindowHeight() const;

    public:
        // Size in physical pixels
        virtual S32 DeviceWidth() const;
        virtual S32 DeviceHeight() const;

        // Size in native (platform-specific) units. By default, this is simply
        // the result of Width() and Height(). However, on some platforms, the
        // size of the screen is in scaled pixels, e.g. on iPhone, size is defined
        // in terms of "points" not actual pixels.
        virtual S32 ScaledWidth() const;
        virtual S32 ScaledHeight() const;
        
    public:
        // TODO: Remove these once we fix Authoring Simulator bugs
        bool IsUpright() const;
        S32 ContentWidthUpright() const;
        S32 ContentHeightUpright() const;
        S32 ViewableContentWidthUpright() const;
        S32 ViewableContentHeightUpright() const;
        Real GetSxUpright() const;
        Real GetSyUpright() const;
        
        
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
/*
        void SetXOriginOffset( Real newValue ) { fXOriginOffset = newValue; }
        void SetYOriginOffset( Real newValue ) { fYOriginOffset = newValue; }

        void SwapContentSize();
        void SwapContentScale();
        void SwapContentAlign();

        // Sets how the content is aligned
        void SetContentAlignment( Alignment xAlign, Alignment yAlign );
        Alignment GetXAlign() const { return (Alignment)fXAlign; }
        Alignment GetYAlign() const { return (Alignment)fYAlign; }
*/
		// Sets the scaling mode and updates content scale factors based on window size
		virtual void SetScaleMode( ScaleMode mode, Rtt_Real screenWidth, Rtt_Real screenHeight );
		virtual ScaleMode GetScaleMode() const;

		void ContentToScreenUnrounded( float& x, float& y ) const;
		void ContentToScreenUnrounded( float& x, float& y, float& w, float& h ) const;

		virtual void ContentToScreen( S32& x, S32& y ) const;
        virtual void ContentToScreen( Rtt_Real& x, Rtt_Real& y, Rtt_Real& w, Rtt_Real& h ) const;
		virtual void ContentToScreen( S32& x, S32& y, S32& w, S32& h ) const;
		virtual void ContentToPixels( S32& x, S32& y, S32& w, S32& h ) const;

    public:
        // Generalized function for calculating proper content scaling factors
        static void UpdateContentScale(
                        Rtt_Real screenWidth, Rtt_Real screenHeight,
                        S32 contentWidth, S32 contentHeight,
                        Display::ScaleMode scaleMode,
                        Rtt_Real& outSx, Rtt_Real& outSy );

        virtual void CalculateContentToScreenScale( Real& outSx, Real& outSy ) const;
        virtual void GetContentRect( Rect& outRect ) const;
        virtual const Rect& GetScreenContentBounds() const;

        virtual void SetContentOrientation( DeviceOrientation::Type newOrientation );

        virtual void WindowDidRotate( DeviceOrientation::Type newOrientation, bool isNewOrientationSupported );

        // Call when window size changes so viewport of GPUStream can be updated.
        // Implicitly calls UpdateContentScale()
        virtual void WindowSizeChanged();

        // Detects if the device width/height of the surface has changed compared to the stream's device width/height.
        // Returns true if they defer, meaning that the caller should then call WindowSizeChanged() to update content scales.
        virtual bool HasWindowSizeChanged() const;

    public:
        virtual DeviceOrientation::Type GetRelativeOrientation() const;
        virtual DeviceOrientation::Type GetLaunchOrientation() const;
        virtual DeviceOrientation::Type GetContentOrientation() const;
        virtual DeviceOrientation::Type GetSurfaceOrientation() const;

    public:
        Rtt_Allocator *GetAllocator() const;

        Runtime& GetRuntime() { return fOwner; }
        const Runtime& GetRuntime() const { return fOwner; }

        MDisplayDelegate *GetDelegate() const { return fDelegate; }
        void SetDelegate( MDisplayDelegate *delegate ) { fDelegate = delegate; }

        DisplayDefaults& GetDefaults() { return * fDefaults; }
        const DisplayDefaults& GetDefaults() const { return * fDefaults; }

        Rtt_AbsoluteTime GetElapsedTime() const;
        float GetDeltaTimeInSeconds() const { return fDeltaTimeInSeconds; }
        Rtt_AbsoluteTime GetPreviousTime() const { return fPreviousTime; }

        Renderer& GetRenderer() { return *fRenderer; }
        const Renderer& GetRenderer() const { return *fRenderer; }

        ShaderFactory& GetShaderFactory() const { return * fShaderFactory; }

        SpritePlayer& GetSpritePlayer() const { return * fSpritePlayer; }

        TextureFactory& GetTextureFactory() const { return * fTextureFactory; }

        void GetViewProjectionMatrix( glm::mat4 &viewMatrix, glm::mat4 &projMatrix );
                
        static U32 GetMaxTextureSize();
        static const char *GetGlString( const char *s );
        static bool GetGpuSupportsHighPrecisionFragmentShaders();
        static U32 GetMaxUniformVectorsCount();
        static U32 GetMaxVertexTextureUnits();

        bool HasFramebufferBlit( bool * canScale ) const;
        void GetVertexAttributes( VertexAttributeSupport & support ) const;

	  public:
		    ProfilingState* GetProfilingState() const { return fProfilingState; }

	  public:
		    Scene& GetScene() { return *fScene; }
		    const Scene& GetScene() const { return *fScene; }

        U8 GetDrawMode() const { return fDrawMode; }
        void SetDrawMode( U8 newValue ) { fDrawMode = newValue; }

        bool IsAntialiased() const { return fIsAntialiased; }
        void SetAntialiased( bool newValue ) { fIsAntialiased = newValue; }

        void SetWireframe( bool newValue );

#if defined( Rtt_ANDROID_ENV ) && TEMPORARY_HACK
    // TODO: Remove this once TEMPORARY_HACK is removed in JavaToNativeBridge.cpp
        RenderingStream& GetStream() { return * fStream; }
#endif

        void Collect( lua_State *L );
 
    public:
        void* GetFactoryFunc() const { return fFactoryFunc; }
        void SetFactoryFunc( void* func ) { fFactoryFunc = func; }

    public:
        // TODO: Remove in favor of SetRestrictedFeature/IsRestrictedFeature/ShouldRestrictFeature
        void SetRestricted( bool newValue ) { fIsRestricted = newValue; }
        bool IsRestricted() const { return fIsRestricted; }
        bool ShouldRestrict( Feature feature ) const
        {
            return IsRestricted() && ! AllowFeature( feature );
        }

    protected:
        // TODO: Remove when we remove ShouldRestrict() above.
        bool AllowFeature( Feature value ) const;

    public:
        // New features should go through these
        void SetRestrictedFeature( Feature feature, bool shouldRestrict );
        bool IsRestrictedFeature( Feature feature ) const;
        bool ShouldRestrictFeature( Feature feature ) const;

    protected:
        bool AllowRestrictedFeature( Feature value ) const;

    protected:
        static U32 GetRestrictedFeatureMask( Feature feature );

    private:
        Runtime& fOwner;
        MDisplayDelegate *fDelegate;
        DisplayDefaults *fDefaults;
        float fDeltaTimeInSeconds;
        Rtt_AbsoluteTime fPreviousTime;
        Renderer *fRenderer;
        ShaderFactory *fShaderFactory;
        SpritePlayer *fSpritePlayer;
        TextureFactory *fTextureFactory;
        Scene *fScene;
		    ProfilingState *fProfilingState;

		// TODO: Refactor data structure portions out
		// We temporarily use RenderingStream b/c it contains key data
		// about window size/orientation
		RenderingStream *fStream;
		PlatformSurface *fTarget;
		int fImageSuffix;
    
        int fObjectFactories;
        void * fFactoryFunc;

		U8 fDrawMode; // stores current physics drawing mode; ranges from 0-2
		bool fIsAntialiased;
		bool fIsCollecting; // guards against nested calls to Collect()
		bool fIsRestricted;
		mutable bool fAllowFeatureResult;
//		U8 fScaleMode;
		U32 fShouldRestrictFeature;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Display_H__
