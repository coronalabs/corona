//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_Display.h"

#include "Core/Rtt_Geometry.h"
#include "Display/Rtt_CPUResourcePool.h"
#include "Display/Rtt_DisplayDefaults.h"
#include "Display/Rtt_MDisplayDelegate.h"
#include "Display/Rtt_BitmapPaint.h"
#include "Display/Rtt_CameraPaint.h"
#include "Display/Rtt_Paint.h"
#include "Display/Rtt_Scene.h"
#include "Display/Rtt_ShaderFactory.h"
#include "Display/Rtt_SpritePlayer.h"
#include "Display/Rtt_TextureFactory.h"
#include "Display/Rtt_TextureResource.h"

#include "Rtt_BufferBitmap.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_PlatformSurface.h"
#include "Rtt_Profiling.h"
#include "CoronaLua.h"

#include "Renderer/Rtt_GLRenderer.h"
#include "Renderer/Rtt_FrameBufferObject.h"
#include "Renderer/Rtt_Matrix_Renderer.h"
#include "Renderer/Rtt_Program.h"
#include "Renderer/Rtt_Texture.h"

// TODO: Remove when we replace TemporaryHackStream
#include "Rtt_GPUStream.h"

// TODO: Remove dependency on Runtime's MCachedResourceLibrary interface
#include "Rtt_Runtime.h"

#define ENABLE_DEBUG_PRINT    0


// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static const char kDisplayNewSnapshotString[] = "display.newSnapshot()";
static const char kGraphicsDefineEffectGraphString[] = "graphics.defineEffect( { graph } )";
static const char kGraphicsDefineEffectFragmentString[] = "graphics.defineEffect( { fragment } )";
static const char kGraphicsDefineEffectVertexString[] = "graphics.defineEffect( { vertex } )";
// NOT USED: static const char kPaintEffectString[] = "paint.effect";
// NOT USED: static const char kObjectBlendEquationString[] = "paint.blendEquation";
static const char kObjectPathString[] = "object.path";
static const char kObjectFillString[] = "object.fill";
static const char kObjectStrokeString[] = "object.stroke";
static const char kObjectBlendModeString[] = "object.blendMode";
static const char kLineStrokeString[] = "line.stroke";
static const char kLineBlendModeString[] = "line.blendMode";
static const char kDisplayColorSampleString[] = "display.colorSample()";
static const char kGraphicsNewOutlineString[] = "graphics.newOutline()";
static const char kDisplayNewEmitterString[] = "display.newEmitter()";
static const char kPhysicsNewParticleSystemString[] = "physics.newParticleSystem()";

const char *
Display::StringForFeature( Feature value )
{
    const char *result = NULL;

    switch ( value )
    {
        case kDisplayNewSnapshot:
            result = kDisplayNewSnapshotString;
            break;
        case kGraphicsDefineEffectGraph:
            result = kGraphicsDefineEffectGraphString;
            break;
        case kGraphicsDefineEffectFragment:
            result = kGraphicsDefineEffectFragmentString;
            break;
        case kGraphicsDefineEffectVertex:
            result = kGraphicsDefineEffectVertexString;
            break;
//        case kPaintEffect:
//            result = kPaintEffectString;
//            break;
//        case kPaintBlendEquation:
//            result = kObjectBlendEquationString;
//            break;
        case kObjectPath:
            result = kObjectPathString;
            break;
        case kObjectFill:
            result = kObjectFillString;
            break;
        case kObjectStroke:
            result = kObjectStrokeString;
            break;
        case kObjectBlendMode:
            result = kObjectBlendModeString;
            break;
        case kLineStroke:
            result = kLineStrokeString;
            break;
        case kLineBlendMode:
            result = kLineBlendModeString;
            break;
        case kDisplayColorSample:
            result = kDisplayColorSampleString;
            break;
        case kGraphicsNewOutline:
            result = kGraphicsNewOutlineString;
            break;
        case kDisplayNewEmitter:
            result = kDisplayNewEmitterString;
            break;
        case kPhysicsNewParticleSystem:
            result = kPhysicsNewParticleSystemString;
            break;
        default:
            Rtt_ASSERT_NOT_IMPLEMENTED();
            break;
    }

    return result;
}

bool
Display::IsProFeature( Feature value )
{
    return ! IsEnterpriseFeature( value );
}

bool
Display::IsEnterpriseFeature( Feature value )
{
    bool result = false;
    
    switch ( value )
    {
        case kGraphicsDefineEffectFragment:
        case kGraphicsDefineEffectVertex:
            result = true;
            break;
        default:
            break;
    }

    return result;
}

const char *
Display::GetTierString( Feature value )
{
    static const char kPro[] = "Pro (or higher)";
    static const char kEnterprise[] = "Enterprise";

    return ( IsEnterpriseFeature( value ) ? kEnterprise : kPro );
}

Display::ScaleMode
Display::ScaleModeFromString( const char *scaleName )
{
    Display::ScaleMode scaleMode = Display::kNone;
    if ( Rtt_StringIsEmpty( scaleName ) == false )
    {
        if ( 0 == Rtt_StringCompareNoCase( scaleName, "zoomEven" ) )
        {
            scaleMode = Display::kZoomEven;
        }
        else if ( 0 == Rtt_StringCompareNoCase( scaleName, "zoomStretch" ) )
        {
            scaleMode = Display::kZoomStretch;
        }
        else if ( 0 == Rtt_StringCompareNoCase( scaleName, "letterbox" ) )
        {
            scaleMode = Display::kLetterbox;
        }
        else if ( 0 == Rtt_StringCompareNoCase( scaleName, "adaptive" ) )
        {
            scaleMode = Display::kAdaptive;
        }
    }
    return scaleMode;
}

// ----------------------------------------------------------------------------

Display::Display( Runtime& owner )
:    fOwner( owner ),
    fDelegate( NULL ),
    fDefaults( Rtt_NEW( owner.Allocator(), DisplayDefaults ) ),
    fDeltaTimeInSeconds( 0.0f ),
    fPreviousTime( owner.GetElapsedTime() ),
    fRenderer( NULL ),
    fShaderFactory( NULL ),
    fSpritePlayer( Rtt_NEW( owner.Allocator(), SpritePlayer( owner.Allocator() ) ) ),
    fTextureFactory( Rtt_NEW( owner.Allocator(), TextureFactory( * this ) ) ),
    fScene( Rtt_NEW( owner.GetAllocator(), Scene( owner.Allocator(), * this ) ) ),
    fProfilingState( Rtt_NEW( owner.GetAllocator(), ProfilingState( owner.GetAllocator() ) ) ),
    fStream( Rtt_NEW( owner.GetAllocator(), GPUStream( owner.GetAllocator() ) ) ),
    fTarget( owner.Platform().CreateScreenSurface() ),
    fImageSuffix( LUA_REFNIL ),
    fDrawMode( kDefaultDrawMode ),
    fIsAntialiased( false ),
    fIsCollecting( false ),
    fIsRestricted( false ),
    fAllowFeatureResult( false ), // When IsRestricted(), default to *not* allowing.
    fShouldRestrictFeature( 0 )
{
}

Display::~Display()
{
    CameraPaint::Finalize();
    Paint::Finalize();

    lua_State *L = GetL();
    if ( L )
    {
        luaL_unref( L, LUA_REGISTRYINDEX, fImageSuffix );
    }

    //Needs to be done before deletes, because it uses scene etc
    fTextureFactory->ReleaseByType( TextureResource::kTextureResource_Any );
    
    Rtt_DELETE( fTarget );
    Rtt_DELETE( fStream );
    Rtt_DELETE( fScene );
    Rtt_DELETE( fProfilingState );
    Rtt_DELETE( fTextureFactory );
    Rtt_DELETE( fSpritePlayer );
    Rtt_DELETE( fShaderFactory );
    Rtt_DELETE( fRenderer );
    Rtt_DELETE( fDefaults );
}

bool
Display::Initialize( lua_State *L, int configIndex, DeviceOrientation::Type orientation )
{
    bool result = false;

    // Only initialize once
    if ( Rtt_VERIFY( ! fRenderer ) )
    {
        Rtt_Allocator *allocator = GetRuntime().GetAllocator();
        fRenderer = Rtt_NEW( allocator, GLRenderer( allocator ) );
        
        fRenderer->Initialize();
        
        CPUResourcePool *resourcePoolObserver = Rtt_NEW(allocator,CPUResourcePool());
        
        fRenderer->SetCPUResourceObserver(resourcePoolObserver);

        ProgramHeader programHeader;

        if ( configIndex > 0 )
        {
            ReadRenderingConfig( L, configIndex, programHeader ); // assumes fStream is not NULL
            bool isV1Compatibility = GetDefaults().IsV1Compatibility();
            GetStage()->SetV1Compatibility( isV1Compatibility );
        }

        // When dynamic content scaling is active, the code thinks the screen has
        // dimensions (fContentWidth, fContentHeight). We tell the renderer the scale
        // factor needed to obtain the rendered content bounds, which are simply the
        // dimensions of the window in scaled content units.  Even though they are
        // in the same units as the content bounds, they may not be the same values.
        // The viewable content bounds are the smaller of the content bounds and the
        // rendered content bounds.
        fStream->Initialize( * fTarget, orientation );

        result = true;

        fShaderFactory = Rtt_NEW( allocator, ShaderFactory( *this, programHeader ) );
    }

    return result;
}
    
void
Display::Teardown()
{
    GetTextureFactory().Teardown();
}

// runtime.lua is pre-compiled into bytecodes and then placed in a byte array
// constant in shell.cpp. The following function defined in shell.cpp loads
// the bytecodes via luaL_loadbuffer. The .cpp file is dynamically generated.
int luaload_runtime(lua_State* L);

static Display::ScaleMode
ToScaleMode( lua_State *L, int index )
{
    Display::ScaleMode scaleMode = Display::kNone;

    lua_getfield( L, index, "scale" );
    const char *scaleName = lua_tostring( L, -1 );
    if ( scaleName )
    {
        scaleMode = Display::ScaleModeFromString( scaleName );
    }
    lua_pop( L, 1 );
    
    return scaleMode;
}

void
Display::ReadRenderingConfig( lua_State *L, int index, ProgramHeader& programHeader )
{
    Rtt_ASSERT( fStream );

    Rtt_ASSERT( 1 == index );
    Rtt_ASSERT( lua_istable( L, index ) );

    lua_getfield( L, index, "shaderPrecision" );
    const char *shaderPrecision = lua_tostring( L, -1 );
    if ( shaderPrecision )
    {
        // This is safer to do than strcpy( outShaderPrecision, shaderPrecision, );
        ProgramHeader::Precision p = ProgramHeader::PrecisionForString( shaderPrecision );
        programHeader.SetPrecision( p );
    }
    else if ( lua_type( L, -1 ) == LUA_TTABLE )
    {
        int index = lua_gettop( L );

        lua_pushnil( L );
        while ( lua_next( L, index ) )
        {
            const char *key = lua_tostring( L, -2 );
            ProgramHeader::Type t = ProgramHeader::TypeForString( key );

            const char *value = lua_tostring( L, -1 );
            ProgramHeader::Precision p = ProgramHeader::PrecisionForString( value );

            programHeader.SetPrecision( t, p );

            lua_pop( L, 1 );
        }
    }
    lua_pop( L, 1 );

    lua_getfield( L, index, "width" );
    int w = (int) lua_tointeger( L, -1 );
    lua_pop( L, 1 );

    lua_getfield( L, index, "height" );
    int h = (int) lua_tointeger( L, -1 );
    lua_pop( L, 1 );

    lua_getfield( L, index, "graphicsCompatibility" );
    bool isV1Compatibility = lua_tointeger( L, -1 );
    if ( isV1Compatibility )
    {
#if defined(Rtt_AUTHORING_SIMULATOR)
        CoronaLuaWarning(L, "V1 graphics compatibility is deprecated. Some features may not work as expected");
#endif

        GetDefaults().SetV1Compatibility( isV1Compatibility );
    }
    lua_pop( L, 1 );

    ScaleMode scaleMode = ToScaleMode( L, index );

    if ( kAdaptive == scaleMode )
    {
        w = fTarget->AdaptiveWidth();
        h = fTarget->AdaptiveHeight();
    }

    // Certain fields only matter if content w,h were provided:
    if ( w > 0 && h > 0 )
    {
        Rtt_ASSERT( fStream );

        // The old Corona behavior was to zoom-stretch if a content width and height was provided without a scale mode.
        if ( kNone == scaleMode )
        {
            scaleMode = kZoomStretch;
        }

        fStream->Preinitialize( w, h );

        fStream->SetScaleMode( scaleMode, Rtt_IntToReal( fTarget->DeviceWidth() ), Rtt_IntToReal( fTarget->DeviceHeight() ) );

        // If we are scaling, then check for imageSuffix table
        if ( scaleMode > kNone )
        {
            #ifdef Rtt_DEBUG
                int top = lua_gettop( L );
            #endif
            if ( Rtt_VERIFY( 0 == GetRuntime().VMContext().DoBuffer( luaload_runtime, false ) ) )
            {
                const char kFunctionName[] = "_createImageSuffixTable";
                lua_getglobal( L, kFunctionName );
                lua_getfield( L, -2, "imageSuffix" );
                if ( lua_istable( L, -1 ) )
                {
                    LuaContext::DoCall( L, 1, 1 ); // _createImageSuffixTable( imageSuffix )
                    if ( lua_istable( L, -1 ) )
                    {
                        fImageSuffix = luaL_ref( L, LUA_REGISTRYINDEX ); // pops result
                    }
                    else
                    {
                        lua_pop( L, 1 ); // pop result
                    }
                }
                else
                {
                    lua_pop( L, 2 ); // pop function and imageSuffix
                }

                lua_pushnil( L );
                lua_setglobal( L, kFunctionName );
            }
            #ifdef Rtt_DEBUG
                Rtt_ASSERT( lua_gettop( L ) == top );
            #endif
        }

        lua_getfield( L, index, "xAlign" );
        Alignment xAlign = kAlignmentCenter;
        const char *alignValue = lua_tostring( L, -1 );
        {
            if ( alignValue )
            {
                if ( 0 == Rtt_StringCompareNoCase( alignValue, "left" ) )
                {
                    xAlign = kAlignmentLeft;
                }
                else if ( 0 == Rtt_StringCompareNoCase( alignValue, "center" ) )
                {
                    xAlign = kAlignmentCenter;
                }
                else if ( 0 == Rtt_StringCompareNoCase( alignValue, "right" ) )
                {
                    xAlign = kAlignmentRight;
                }
            }
        }
        lua_pop( L, 1 );

        lua_getfield( L, index, "yAlign" );
        Alignment yAlign = kAlignmentCenter;
        alignValue = lua_tostring( L, -1 );
        {
            if ( alignValue )
            {
                if ( 0 == Rtt_StringCompareNoCase( alignValue, "top" ) )
                {
                    yAlign = kAlignmentTop;
                }
                else if ( 0 == Rtt_StringCompareNoCase( alignValue, "center" ) )
                {
                    yAlign = kAlignmentCenter;
                }
                else if ( 0 == Rtt_StringCompareNoCase( alignValue, "bottom" ) )
                {
                    yAlign = kAlignmentBottom;
                }
            }
        }
        lua_pop( L, 1 );

        fStream->SetContentAlignment( xAlign, yAlign );
    }

    Rtt_ASSERT( 1 == lua_gettop( L ) );
}

lua_State *
Display::GetL() const
{
    const Runtime& runtime = GetRuntime();
    lua_State *L = ( runtime.IsVMContextValid() ? runtime.VMContext().L() : NULL );
    return L;
}

void
Display::Start()
{
    // The call to PrepareToRenderer requires a current OpenGL context which we can't assume yet.
    // RuntimeGuard will ensure that an OpenGL context is current and that locks are in place for multithreaded conditions.
    // We must do this inside a scope because the RuntimeGuard destructor is relied on to do unlocking and restoring of OpenGL state as necessary.
    // The overloaded operator()() below uses this same RuntimeGuard technique.
    // TODO: We may not need this scope guard.
    // The runtime guard is re-entrant safe so it should be safe to leave guarded when we hit the operator()().
    RuntimeGuard guard( GetRuntime() );

    RenderingStream& stream = * fStream;
    stream.PrepareToRender();
}

void
Display::Restart()
{
    // RenderingStream& stream = * fStream;
    // stream.Reinitialize( * fTarget, stream.GetContentOrientation() );
    // stream.PrepareToRender();
    Restart( fStream->GetContentOrientation() );
}

void
Display::Restart( DeviceOrientation::Type orientation )
{
    RenderingStream& stream = * fStream;
    stream.Reinitialize( * fTarget, orientation );
    stream.PrepareToRender();
}

void
Display::Restart( int newWidth, int newHeight )
{
    RenderingStream& stream = * fStream;
    stream.Preinitialize( newWidth, newHeight );
    Restart( fStream->GetContentOrientation() );
}
    
void
Display::Update()
{
    PROFILING_BEGIN( *GetProfilingState(), up, Update );

    up.Add( "Display::Update Begin" );
    
	Runtime& runtime = fOwner;
	lua_State *L = fOwner.VMContext().L();
	fSpritePlayer->Run( L, Rtt_AbsoluteToMilliseconds(runtime.GetElapsedTime()) );

	up.Add( "Run sprite player" );

	GetScene().QueueUpdateOfUpdatables();

	up.Add( "Queue updatables" );

	if ( fDelegate )
	{
		fDelegate->WillDispatchFrameEvent( * this );
	}

	up.Add( "Prepare for frame event" );

	const FrameEvent& fe = FrameEvent::Constant();
	fe.Dispatch( L, runtime );
	
    up.Add( "FrameEvent" );
    
	const RenderEvent& re = RenderEvent::Constant();
	re.Dispatch( L, runtime );
    
    up.Add( "LateUpdate" );

	Profiling::ResetSums();

    up.Add( "Display::Update End" );
}

void
Display::Render()
{
    PROFILING_BEGIN( *GetProfilingState(), rp, Render );

    rp.Add( "Display::Render Begin" );

	{
		Rtt_AbsoluteTime elapsedTime = GetRuntime().GetElapsedTime();

        const Rtt::Real kMillisecondsPerSecond = 1000.0f;
        // NOT _USED: Rtt::Real totalTime = Rtt_AbsoluteToMilliseconds( elapsedTime ) / kMillisecondsPerSecond;
        fDeltaTimeInSeconds = Rtt_AbsoluteToMilliseconds( elapsedTime - fPreviousTime ) / kMillisecondsPerSecond;

        fPreviousTime = elapsedTime;

        // Use this when debugging:
        //fDeltaTimeInSeconds = ( 1.0f / 30.0f );
    }

	GetScene().Render( * fRenderer, * fTarget, &rp );

    rp.Add( "Display::Render End" );
}

void
Display::Blit()
{
    // We don't use the RuntimeGuard here b/c no Lua is invoked so we don't need
    // to collect unreachables since no new objects would be added to the orphanage
    const Runtime& runtime = GetRuntime();
    runtime.Begin();
    {
        GetScene().Invalidate();
        GetScene().Render( * fRenderer, * fTarget );
    }
    runtime.End();
}

void
Display::Invalidate()
{
    GetScene().Invalidate();
}

StageObject *
Display::GetStage()
{
    return & GetScene().CurrentStage();
}

StageObject *
Display::GetStageOffscreen()
{
    return & GetScene().OffscreenStage();
}

BitmapPaint *
Display::CaptureScreen( bool save_to_file,
                        bool output_file_will_be_png_format )
{
    return Capture( NULL,
                    NULL,
                    save_to_file,
                    output_file_will_be_png_format,
                    false,
                    NULL,
                    NULL );
}

BitmapPaint *
Display::CaptureBounds( Rect *screenBounds,
                        bool will_be_saved_to_file,
                        bool output_file_will_be_png_format )
{
    return Capture( NULL,
                    screenBounds,
                    will_be_saved_to_file,
                    output_file_will_be_png_format,
                    false,
                    NULL,
                    NULL );
}

BitmapPaint *
Display::CaptureDisplayObject( DisplayObject *object,
                                bool will_be_saved_to_file,
                                bool output_file_will_be_png_format,
                                bool crop_object_to_screen_bounds )
{
    return Capture( object,
                    NULL,
                    will_be_saved_to_file,
                    output_file_will_be_png_format,
                    crop_object_to_screen_bounds,
                    NULL,
                    NULL );
}

BitmapPaint *
Display::CaptureSave( DisplayObject *object,
                        bool crop_object_to_screen_bounds,
                        bool output_file_will_be_png_format,
                        const ColorUnion *optionalBackgroundColor )
{
    return Capture( object,
                    NULL,
                    true,
                    output_file_will_be_png_format,
                    crop_object_to_screen_bounds,
                    optionalBackgroundColor,
                    NULL );
}

void
Display::ColorSample( float pos_x,
                        float pos_y,
                        RGBA &output_color )
{
    Rect screenBounds;
    screenBounds.Initialize( pos_x, pos_y, 1.0f, 1.0f );

    BitmapPaint *paint = Capture( NULL,
                                    &screenBounds,
                                    true,
                                    false,
                                    false,
                                    NULL,
                                    &output_color );
    if( ! paint )
    {
        Rtt_TRACE( ( "Unable to capture screen. The platform or device might not be supported." ) );
        return;
    }

    Rtt_DELETE( paint );
}

BitmapPaint *
Display::Capture( DisplayObject *object,
                    Rect *screenBounds,
                    bool will_be_saved_to_file,
                    bool output_file_will_be_png_format,
                    bool crop_object_to_screen_bounds,
                    const ColorUnion *optionalBackgroundColor,
                    RGBA *optional_output_color )
{
    // Do not continue if given invalid screen bounds.
    if( screenBounds )
    {
        if ((screenBounds->xMin >= screenBounds->xMax) ||
            (screenBounds->yMin >= screenBounds->yMax))
        {
            return NULL;
        }
    }

    fRenderer->BeginFrame( 0.1f, 0.1f, GetSx(), GetSy() );

    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////

    Runtime& runtime = GetRuntime();
    Rtt_Allocator *allocator = runtime.GetAllocator();

    // Save current state so we can restore it later
    FrameBufferObject *previous_fbo = fRenderer->GetFrameBufferObject();
    FrameBufferObject *fbo = NULL;

    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////
    //FBO SETUP

    // Calculate pixel dimensions for texture
    // The source position to capture from.
    Real x_in_content_units = 0.0f;
    Real y_in_content_units = 0.0f;
    Real w_in_content_units = 0.0f;
    Real h_in_content_units = 0.0f;

    Rtt::Real offscreenViewMatrix[16];
    Rtt::CreateViewMatrix( 0.0f, 0.0f, 0.5f,
                            0.0f, 0.0f, 0.0f,
                            0.0f, 1.0f, 0.0f,
                            offscreenViewMatrix );
    Rtt::Real offscreenProjMatrix[16];

    if ( object )
    {
        // Calculate the bounds of the given display object in pixels.
        // Clip the object to the screen's bounds.
        Rect objectBounds = object->StageBounds();
        if (screenBounds)
        {
            objectBounds.Intersect(*screenBounds);
        }
        else
        {
            if( crop_object_to_screen_bounds )
            {
                objectBounds.Intersect(GetScreenContentBounds());
            }
        }

        x_in_content_units = objectBounds.xMin;
        y_in_content_units = objectBounds.yMin;
        w_in_content_units = ( objectBounds.xMax - x_in_content_units );
        h_in_content_units = ( objectBounds.yMax - y_in_content_units );

        Rtt::CreateOrthoMatrix( x_in_content_units,
                                objectBounds.xMax,
                                y_in_content_units,
                                objectBounds.yMax,
                                0.0f,
                                1.0f,
                                offscreenProjMatrix );
    }
    else // Full screen or screenBounds.
    {
        Rect *full_screen = const_cast< Rect * >( &GetScreenContentBounds() );

        Rect *bounds_to_use = NULL;

        if( screenBounds )
        {
            // Subregion of the screen.
            screenBounds->Intersect( *full_screen );
            bounds_to_use = screenBounds;
        }
        else
        {
            // Full screen.
            bounds_to_use = full_screen;
        }

        x_in_content_units = bounds_to_use->xMin;
        y_in_content_units = bounds_to_use->yMin;
        w_in_content_units = ( bounds_to_use->xMax - x_in_content_units );
        h_in_content_units = ( bounds_to_use->yMax - y_in_content_units );

        Rtt::CreateOrthoMatrix( x_in_content_units,
                                bounds_to_use->xMax,
                                y_in_content_units,
                                bounds_to_use->yMax,
                                0.0f,
                                1.0f,
                                offscreenProjMatrix );
    }

    // The source position to capture from.
    Rtt_Real x_in_pixels = 0;
    Rtt_Real y_in_pixels = 0;
    Rtt_Real w_in_pixels = w_in_content_units;
    Rtt_Real h_in_pixels = h_in_content_units;
    if(!optional_output_color){

        ContentToScreen( x_in_pixels,
                        y_in_pixels,
                        w_in_pixels,
                        h_in_pixels);

        // We ONLY want the w_in_pixels and h_in_pixels.
        // Using anything but 0 here breaks Android.
        x_in_pixels = 0;
        y_in_pixels = 0;
    }
#    if defined( Rtt_OPENGLES )
        const Texture::Format kFormat = Texture::kRGBA;
#    else
        const Texture::Format kFormat = Texture::kBGRA;
#    endif

    // Using Texture::kNearest and Texture::kClampToEdge here is absolutely
    // mandatory. See:
    //
    //    http://www.khronos.org/registry/gles/extensions/APPLE/APPLE_texture_2D_limited_npot.txt
    //
    //        "A NPOT 2D texture with a wrap mode that is not CLAMP_TO_EDGE or
    //        a minfilter that is not NEAREST or LINEAR is considered incomplete."
    TextureFactory &factory = GetTextureFactory();
    SharedPtr< TextureResource > tex = factory.Create( w_in_pixels,
                                                        h_in_pixels,
                                                        kFormat,
                                                        Texture::kNearest,
                                                        Texture::kClampToEdge,
                                                        will_be_saved_to_file );

    BitmapPaint *paint = Rtt_NEW( allocator,
                                    BitmapPaint( tex ) );

    fbo = Rtt_NEW( allocator,
                    FrameBufferObject( allocator,
                                        paint->GetTexture() ) );

    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////
    //FROM SnapshotObject::Draw()
    //PREPARE TO DRAW!!!!!

    //// Save these so we can restore them later.
    //
    Rtt::Real previous_viewMatrix[16];
    Rtt::Real previous_projMatrix[16];
    fRenderer->GetFrustum( previous_viewMatrix, previous_projMatrix );

    S32 previous_viewport_x, previous_viewport_y, previous_viewport_width, previous_viewport_height;
    fRenderer->GetViewport( previous_viewport_x, previous_viewport_y, previous_viewport_width, previous_viewport_height );
    //
    ////

    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////
    //SCENE RENDER

    Scene& scene = GetScene();

    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////
    //FROM SnapshotObject::Draw()
    //PREPARE TO DRAW!!!!!
    Real texW = Rtt_IntToReal( fbo->GetTexture()->GetWidth() );
    Real texH = Rtt_IntToReal( fbo->GetTexture()->GetHeight() );

    fRenderer->SetFrameBufferObject( fbo );
    fRenderer->PushMaskCount();
    fRenderer->SetViewport( 0, 0, texW, texH );
    fRenderer->Clear( 0.0f, 0.0f, 0.0f, 0.0f );

    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////
    //FROM THIS ORIGINAL FUNCTION MADE BUILDABLE!!!!!!!
    //DRAW!!!!!

    // Fetch the bounds of the portion of the screen that we are
    // going to capture and re-render that part of the screen.
    if ( object )
    {
        // Set the background color to transparent.
        ColorUnion previousClearColor;
        ColorUnion transparentClearColor;

        if( optionalBackgroundColor )
        {
            transparentClearColor.rgba = optionalBackgroundColor->rgba;
        }
        else
        {
            if( output_file_will_be_png_format ||
                ( ! will_be_saved_to_file ) )
            {
                transparentClearColor.rgba.r = 0;
                transparentClearColor.rgba.g = 0;
                transparentClearColor.rgba.b = 0;
            }
            else
            {
                // JPG and others.
                transparentClearColor.rgba.r = 0xff;
                transparentClearColor.rgba.g = 0xff;
                transparentClearColor.rgba.b = 0xff;
            }
            transparentClearColor.rgba.a = 0;
        }

        previousClearColor.pixel = GetDefaults().GetClearColor();
        GetDefaults().SetClearColor( transparentClearColor.pixel );

        fRenderer->SetFrustum( offscreenViewMatrix, offscreenProjMatrix );

        const Rect* snb = GetStage()->GetSnapshotBounds();
        Rect empty;
        if (!screenBounds && object && !crop_object_to_screen_bounds)
        {
            // this will disable culling of ofscreen children
            GetStage()->SetSnapshotBounds(&empty);
        }
        // Render only the given object and its children, if any, to be captured down below.
        scene.Render( *fRenderer, *fTarget, *object );
        if (!screenBounds && object && !crop_object_to_screen_bounds)
        {
            GetStage()->SetSnapshotBounds(snb);
        }

        // Restore background back to its previous color.
        GetDefaults().SetClearColor( previousClearColor.pixel );
    }
    else // Full screen or screenBounds.
    {
        fRenderer->SetFrustum( offscreenViewMatrix, offscreenProjMatrix );

        scene.Render( *fRenderer, *fTarget, scene.CurrentStage() );
    }

    fRenderer->PopMaskCount();

    ////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////
    //FBO CLEANUP

    fRenderer->EndFrame();
    fRenderer->Swap();
    fRenderer->Render();

    if( will_be_saved_to_file ||
        optional_output_color )
    {
        // Create screen capture.
        BufferBitmap *bitmap = static_cast< BufferBitmap * >( tex->GetBitmap() );

        // This function requires coordinates in pixels.
        fStream->CaptureFrameBuffer( *bitmap,
                                        x_in_pixels,
                                        y_in_pixels,
                                        w_in_pixels,
                                        h_in_pixels );

        if( output_file_will_be_png_format )
        {
            // This should ONLY be done for PNGs.
#if !defined(Rtt_NXS_ENV)
                bitmap->UndoPremultipliedAlpha();
#endif
        }

        if( optional_output_color )
        {
            // We want the RGBA value of the first pixel.

            const unsigned char *bytes = static_cast< const unsigned char * >( bitmap->ReadAccess() );

#            ifdef Rtt_OPENGLES

                // IMPORTANT: We're assuming the format is GL_RGBA and GL_UNSIGNED_BYTE.
                optional_output_color->r = bytes[ 0 ];
                optional_output_color->g = bytes[ 1 ];
                optional_output_color->b = bytes[ 2 ];
                optional_output_color->a = bytes[ 3 ];

#            else // Not Rtt_OPENGLES

                // IMPORTANT: We're assuming the format is GL_ARGB and GL_UNSIGNED_BYTE.
                optional_output_color->r = bytes[ 1 ];
                optional_output_color->g = bytes[ 2 ];
                optional_output_color->b = bytes[ 3 ];
                optional_output_color->a = bytes[ 0 ];

#            endif // Rtt_OPENGLES
        }
    }

    // Restore state so further rendering is unaffected
    fRenderer->SetViewport( previous_viewport_x, previous_viewport_y, previous_viewport_width, previous_viewport_height );
    fRenderer->SetFrustum( previous_viewMatrix, previous_projMatrix );
    fRenderer->SetFrameBufferObject( previous_fbo );

#    if ENABLE_DEBUG_PRINT

        printf( "capture bounds in content units: x: %d y: %d w: %d h: %d\n",
                x_in_content_units,
                y_in_content_units,
                w_in_content_units,
                h_in_content_units );
        printf( "capture bounds in pixels: x: %d y: %d w: %d h: %d\n",
                x_in_pixels,
                y_in_pixels,
                w_in_pixels,
                h_in_pixels );

#    endif // ENABLE_DEBUG_PRINT

    Rtt_DELETE( fbo );

    // If object was just created this will draw it to main scene as well, not only to FBO
    scene.Invalidate();

    return paint;
}

void
Display::UnloadResources()
{
    GetRenderer().ReleaseGPUResources();

    // Special case: Text objects use textures that are not backed by a file
///    TextObject::Unload( GetScene().CurrentStage() );
}

void
Display::ReloadResources()
{
    GetRenderer().ReleaseGPUResources();
    GetRenderer().Initialize();

    // Special case: Text objects use textures that are not backed by a file
///    TextObject::Reload( GetScene().CurrentStage() );
}

void
Display::GetImageSuffix( String& outSuffix ) const
{
    Rtt_Real sx = fStream->GetSx();
    Rtt_Real sy = fStream->GetSy();

    if ( Rtt_RealIsOne( sx ) && Rtt_RealIsOne( sy ) )
    {
        // Use original file since content scale is 1:1
    }
    else if ( LUA_REFNIL != fImageSuffix )
    {
        lua_State *L = GetL();
        if ( L )
        {
            #ifdef Rtt_DEBUG
                int top = lua_gettop( L );
            #endif

            Real contentScale = Rtt_RealDivNonZeroAB( Rtt_REAL_1, Rtt_RealDiv2( sx + sx ) );

            lua_rawgeti( L, LUA_REGISTRYINDEX, fImageSuffix ); // t

            Rtt_ASSERT( NULL == outSuffix.GetString() );

            for ( size_t i = lua_objlen( L, -1 );
                  i > 0 && NULL == outSuffix.GetString();
                  i-- )
            {
                lua_rawgeti( L, -1, (int) i ); // item = t[i]
                lua_pushstring( L, "scale" );
                lua_rawget( L, -2 ); // scale = item.scale (pops "scale")
                Real scale = luaL_toreal( L, -1 );
                lua_pop( L, 1 ); // pop scale
                if ( contentScale >= scale )
                {
                    lua_pushstring( L, "suffix" );
                    lua_rawget( L, -2 ); // suffix = item.suffix (pops "suffix")
                    outSuffix.Set( lua_tostring( L, -1 ) );

                    lua_pop( L, 1 ); // pop value
                }

                lua_pop( L, 1 ); // pop item
            }

            lua_pop( L, 1 ); // pop t

            #ifdef Rtt_DEBUG
                Rtt_ASSERT( lua_gettop( L ) == top );
            #endif
        }
    }
}

bool
Display::GetImageFilename( const char *filename, MPlatform::Directory baseDir, String& outFilename ) const
{
    bool result = false;

    Rtt_Real sx = fStream->GetSx();
    Rtt_Real sy = fStream->GetSy();

    if ( Rtt_RealIsOne( sx ) && Rtt_RealIsOne( sy ) )
    {
        // Use original file since content scale is 1:1
    }
    else if ( LUA_REFNIL != fImageSuffix && Rtt_VERIFY( filename ) )
    {
        // last occurence of '.'
        const char *extension = strrchr( filename , '.' );
        if ( extension )
        {
            lua_State *L = GetL();
            if ( L )
            {
                #ifdef Rtt_DEBUG
                    int top = lua_gettop( L );
                #endif

                Real contentScale = Rtt_RealDivNonZeroAB( Rtt_REAL_1, Rtt_RealDiv2( sx + sx ) );

                lua_rawgeti( L, LUA_REGISTRYINDEX, fImageSuffix ); // t

                Rtt_ASSERT( NULL == outFilename.GetString() );

                for ( size_t i = lua_objlen( L, -1 ); i > 0 && !result; i-- )
                {
                    lua_rawgeti( L, -1, (int) i ); // item = t[i]
                    lua_pushstring( L, "scale" );
                    lua_rawget( L, -2 ); // scale = item.scale (pops "scale")
                    Real scale = luaL_toreal( L, -1 );
                    lua_pop( L, 1 ); // pop scale
                    if ( contentScale >= scale )
                    {
                        // Found a candidate suffix
                        lua_pushlstring( L, filename, extension - filename );

                        lua_pushstring( L, "suffix" );
                        lua_rawget( L, -3 ); // suffix = item.suffix (pops "suffix")

                        lua_pushstring( L, extension ); // extension

                        lua_concat( L, 3 ); // filenameSuffixed
                        const char *filenameSuffixed = lua_tostring( L, -1 );

                        // Verify file exists
                        const Runtime& runtime = GetRuntime();
                        String path( runtime.Allocator() );
                        runtime.Platform().PathForFile( filenameSuffixed, baseDir, MPlatform::kTestFileExists, path );
                        if ( path.GetString() )
                        {
                            outFilename.Set( filenameSuffixed );
                            result = true;
                        }

                        lua_pop( L, 1 ); // pop result
                    }

                    lua_pop( L, 1 ); // pop item
                }

                lua_pop( L, 1 ); // pop t

                #ifdef Rtt_DEBUG
                    Rtt_ASSERT( lua_gettop( L ) == top );
                #endif
            }
        }
    }

    return result;
}

bool
Display::PushImageSuffixTable() const
{
    bool wasPushed = false;
    if ( LUA_REFNIL != fImageSuffix )
    {
        lua_State *L = GetL();
        if ( L )
        {
            lua_rawgeti( L, LUA_REGISTRYINDEX, fImageSuffix );
            wasPushed = true;
        }
    }
    return wasPushed;
}

GroupObject *
Display::Overlay()
{
    return & GetScene().Overlay();
}

GroupObject *
Display::Orphanage()
{
    return & GetScene().Orphanage();
}

GroupObject *
Display::HitTestOrphanage()
{
    return & GetScene().SnapshotOrphanage();
}

S32
Display::RenderedContentWidth() const
{
    return fStream->GetRenderedContentWidth();
}

S32
Display::RenderedContentHeight() const
{
    return fStream->GetRenderedContentHeight();
}


S32
Display::ViewableContentWidth() const
{
    // The viewable content width is the smaller of the rendered content width
    // or the content width itself. The rendered width is the window width
    // in *scaled* units as determined by UpdateContentScale().
    //
    // Depending on the relationship of aspect ratios between the window and
    // the content, the rendered width might be larger (kLetterbox)
    // the same (kFillStretch), or smaller (kFillEven).
    S32 renderedContentWidth = RenderedContentWidth();
    S32 contentWidth = ContentWidth();
    return Min( contentWidth, renderedContentWidth );
}

S32
Display::ViewableContentHeight() const
{
    // See comment in Runtime::ViewableContentWidth()
    S32 renderedContentHeight = RenderedContentHeight();
    S32 contentHeight = ContentHeight();
    return Min( contentHeight, renderedContentHeight );
}

Real
Display::ActualContentWidth() const
{
    return fStream->ActualContentWidth();
}

Real
Display::ActualContentHeight() const
{
    return fStream->ActualContentHeight();
}

S32
Display::WindowWidth() const
{
    return fTarget->Width();
}
S32
Display::WindowHeight() const
{
    return fTarget->Height();
}

S32
Display::DeviceWidth() const
{
    return fTarget->DeviceWidth();
}

S32
Display::DeviceHeight() const
{
    return fTarget->DeviceHeight();
}

S32
Display::ScaledWidth() const
{
    return fTarget->ScaledWidth();
}

S32
Display::ScaledHeight() const
{
    return fTarget->ScaledHeight();
}

// ----------------------------------------------------------------------------
/// Determines if the rendering system is displaying its content upright or upside down.
/// <br>
/// This function is used to work-around a bug where the rendering stream for the Corona Simulator
/// incorrectly swaps the content width/height and other properties when rotating the simulator
/// to an orientation that the Corona app does not support. So, if this function returns false,
/// then this indicates that you need to read the opposite content property from the rendering
/// system. For example, if you want to fetch content width and this function returns false,
/// then you should call RenderingStream.ContentHeight(), which will provide content width.
/// <br>
/// Note: This function should be removed when the above Corona Simulator bug has been fixed.
/// <br>
/// Additonal Note: A function like this also exists in the LuaLibDisplay class.
/// @param stream Pointer to Corona's rendering stream.
/// @return Returns true if the rendering system is displaying content upright or upside down.
///         <br>
///         Returns false if the content is being displayed sideways, which can only happen in
///         the Corona Simulator when the display is rotated to an orientation that the app
///         does not support.
bool
Display::IsUpright() const
{
#if defined(Rtt_AUTHORING_SIMULATOR)
    if (DeviceOrientation::IsSideways(GetRelativeOrientation()))
    {
        bool isPortrait = DeviceOrientation::IsUpright( GetContentOrientation() );
        if ((isPortrait && (ContentWidth() < ContentHeight())) ||
            (!isPortrait && (ContentWidth() > ContentHeight())))
        {
            return false;
        }
    }
#endif
    return true;
}

S32
Display::ContentWidthUpright() const
{
    return ( IsUpright() ? ContentWidth() : ContentHeight() );
}

S32
Display::ContentHeightUpright() const
{
    return ( IsUpright() ? ContentHeight() : ContentWidth() );
}

S32
Display::ViewableContentWidthUpright() const
{
    return ( IsUpright() ? ViewableContentWidth() : ViewableContentHeight() );
}

S32
Display::ViewableContentHeightUpright() const
{
    return ( IsUpright() ? ViewableContentHeight() : ViewableContentWidth() );
}

Real
Display::GetSxUpright() const
{
    return ( IsUpright() ? GetSx() : GetSy() );
}

Real
Display::GetSyUpright() const
{
    return ( IsUpright() ? GetSy() : GetSx() );
}

// ----------------------------------------------------------------------------

S32
Display::ContentWidth() const
{
    return fStream->ContentWidth();
}

S32
Display::ContentHeight() const
{
    return fStream->ContentHeight();
}

S32
Display::ScreenWidth() const
{
    return fStream->ScreenWidth();
}

S32
Display::ScreenHeight() const
{
    return fStream->ScreenHeight();
}

Real
Display::GetSx() const
{
    return fStream->GetSx();
}

Real
Display::GetSy() const
{
    return fStream->GetSy();
}

Real
Display::GetXOriginOffset() const
{
    return fStream->GetXOriginOffset();
}

Real
Display::GetYOriginOffset() const
{
    return fStream->GetYOriginOffset();
}

Real
Display::PointsWidth() const
{
    return fTarget->ScaledWidth();
}

Real
Display::PointsHeight() const
{
    return fTarget->ScaledHeight();
}

void
Display::SetScaleMode( ScaleMode mode, Rtt_Real screenWidth, Rtt_Real screenHeight )
{
    fStream->SetScaleMode( mode, screenWidth, screenHeight );
}

Display::ScaleMode
Display::GetScaleMode() const
{
    return fStream->GetScaleMode();
}

void
Display::ContentToScreen( S32& x, S32& y ) const
{
    S32 w = 0;
    S32 h = 0;
    fStream->ContentToScreen( x, y, w, h );
}

void
Display::ContentToScreen( Rtt_Real& x, Rtt_Real& y, Rtt_Real& w, Rtt_Real& h ) const
{
    fStream->ContentToScreen( x, y, w, h );
}

void
Display::ContentToScreen(S32& x, S32& y, S32& w, S32& h ) const
{
    fStream->ContentToScreen( x, y, w, h );
}

void
Display::ContentToPixels( S32& x, S32& y, S32& w, S32& h ) const
{
    fStream->ContentToPixels( x, y, w, h );
}

// Generalized function for calculating proper content scaling factors
void
Display::UpdateContentScale(
                    Rtt_Real screenWidth, Rtt_Real screenHeight,
                    S32 contentW, S32 contentH,
                    ScaleMode scaleMode,
                    Rtt_Real& outSx, Rtt_Real& outSy )
{
    Rtt_ASSERT( contentW > 0 && contentH > 0 );

    // We used to skip this if the scaleMode was kNone. However, on some platforms,
    // the screen dimensions are virtual (i.e. 'points' on iOS/Mac retina displays)
    // whereas the content pixels are the physical pixels when the mode is kNone.
    const Rtt_Real contentWidth = Rtt_IntToReal( contentW );
    const Rtt_Real contentHeight = Rtt_IntToReal( contentH );

    const Rtt_Real screenAspect = Rtt_RealDiv( screenWidth, screenHeight );
    const Rtt_Real contentAspect = Rtt_RealDiv( contentWidth, contentHeight );

    Rtt_Real scaleX = Rtt_REAL_1;
    Rtt_Real scaleY = Rtt_REAL_1;
    switch( scaleMode )
    {
        case kZoomEven:
            scaleX = scaleY = ( screenAspect > contentAspect
                                ? Rtt_RealDiv( contentWidth, screenWidth )
                                : Rtt_RealDiv( contentHeight, screenHeight ) );
            break;
        case kAdaptive:
        case kZoomStretch:
        case kNone:
            scaleX = Rtt_RealDiv( contentWidth, screenWidth );
            scaleY = Rtt_RealDiv( contentHeight, screenHeight );
            break;
        case kLetterbox:
            scaleX = scaleY = ( screenAspect > contentAspect
                                ? Rtt_RealDiv( contentHeight, screenHeight )
                                : Rtt_RealDiv( contentWidth, screenWidth ) );
            break;
        default:
            break;
    }

    outSx = scaleX;
    outSy = scaleY;
}

void
Display::CalculateContentToScreenScale( Real& outSx, Real& outSy ) const
{
    S32 contentW = ContentWidth();
    S32 contentH = ContentHeight();

    // We need to grab the OS width and height in points
    S32 screenW = PointsWidth();
    S32 screenH = PointsHeight();

    // When content is sideways and surface is not, then the surface w,h
    // need to be swapped to match the content w,h.
    DeviceOrientation::Type surfaceOrientation = fTarget->GetOrientation();
    DeviceOrientation::Type contentOrientation = fStream->GetContentOrientation();
    bool isSurfaceSideways = DeviceOrientation::IsSideways( surfaceOrientation );
    bool isContentSideways = DeviceOrientation::IsSideways( contentOrientation );
    if ( isContentSideways && ! isSurfaceSideways )
    {
        Swap( screenW, screenH );
    }

    outSx = Rtt_REAL_1;
    outSy = Rtt_REAL_1;
    Display::UpdateContentScale(
        screenW, screenH,
        contentW, contentH,
        fStream->GetScaleMode(),
        outSx, outSy );
}

void
Display::GetContentRect( Rect& outRect ) const
{
    const Vertex2 minCorner = { Rtt_REAL_0, Rtt_REAL_0 };
    const Vertex2 maxCorner = { Rtt_IntToReal( ContentWidth() ), Rtt_IntToReal( ContentHeight() ) };
    const Rect windowRect( minCorner, maxCorner );
    outRect = windowRect;
}

const Rect&
Display::GetScreenContentBounds() const
{
    return ((const RenderingStream *)fStream)->GetScreenContentBounds();
}

// Used to support auto-rotation of Corona views on devices.
// For simulator, use WindowDidRotate() instead.
//
// On some devices (e.g. iPhone), the surface orientation is fixed relative to the
// device. In this situation, we need to set the orientation of the content in Corona
// independently of the surface. This function, updates the content dimension data
// (which is split across Runtime and GPUStream) and tells GPUStream to update the
// content orientation, updating the projection/frustum.
void
Display::SetContentOrientation( DeviceOrientation::Type newOrientation )
{
    Rtt_ASSERT( fStream );

    if ( ! GetRuntime().IsProperty( Runtime::kIsOrientationLocked ) )
    {
        RenderingStream& stream = * fStream;

//        DeviceOrientation::Type oldOrientation = stream.GetContentOrientation();

//Rtt_ASSERT_NOT_IMPLEMENTED();
        stream.SetContentOrientation( newOrientation );

        GetScene().Invalidate();
        //GetScene().CurrentStage().Invalidate();
    }
}

void
Display::WindowDidRotate( DeviceOrientation::Type newOrientation, bool isNewOrientationSupported )
{
    Rtt_ASSERT( fStream );

    // Auto-rotation follows the following truth table. This is why we only need
    // to know if newOrientation is supported.
    //
    //    isSrcSuppoted    isDstSupported        shouldAutoRotate
    //    ------------------------------------------------------------------------
    //    0                1                    1
    //    1                1                    1
    //    0                0                    0
    //    1                0                    0
    const Runtime& runtime = GetRuntime();
    runtime.Begin();
    {
        bool autoRotate = isNewOrientationSupported && ( ! runtime.IsProperty( Runtime::kIsOrientationLocked ) );

        RenderingStream& stream = * fStream;


        //TODO - check if the new orientation is supported
        stream.SetOrientation(newOrientation, autoRotate);
        
        
        GetScene().Invalidate();

///        GetScene().CurrentStage().Invalidate();
    }
    runtime.End();
}

void
Display::WindowSizeChanged()
{
    const Runtime& runtime = GetRuntime();
    runtime.Begin();
    {
        RenderingStream *stream = fStream;
        if ( stream && stream->IsProperty( RenderingStream::kInitialized ) )
        {
            stream->UpdateViewport( fTarget->Width(), fTarget->Height() );

            // When surface is sideways (simulator), then the surface w,h
            // need to be swapped to match the content w,h.
            S32 screenW = fTarget->DeviceWidth();
            S32 screenH = fTarget->DeviceHeight();
#if defined(Rtt_WIN_ENV) || defined(Rtt_NXS_ENV)
            Rtt::DeviceOrientation::Type orientation = GetContentOrientation();
#else
            //TODO: Determine if we can do this the above Windows way on all platforms.
            Rtt::DeviceOrientation::Type orientation = fTarget->GetOrientation();
#endif
            if ( DeviceOrientation::IsSideways( orientation ) )
            {
                Swap( screenW, screenH );
            }

            // For these scale modes, the content width and height change when the window size changes.
            Rtt::Display::ScaleMode scaleMode = stream->GetScaleMode();
            if ( ( Display::kNone == scaleMode ) || ( Display::kAdaptive == scaleMode ) )
            {
                S32 contentWidth;
                S32 contentHeight;
                if ( Display::kAdaptive == scaleMode )
                {
                    contentWidth = fTarget->AdaptiveWidth();
                    contentHeight = fTarget->AdaptiveHeight();
                }
                else
                {
                    contentWidth = fTarget->DeviceWidth();
                    contentHeight = fTarget->DeviceHeight();
                }
                if ( DeviceOrientation::IsSideways( orientation ) )
                {
                    Swap( contentWidth, contentHeight );
                }
                stream->Preinitialize( contentWidth, contentHeight );
            }

            // Update the display's content scales using the new widths and heights up above.
            stream->UpdateContentScale( screenW, screenH );
        }
    }
    runtime.End();
}

bool
Display::HasWindowSizeChanged() const
{
    if (!fStream || !fTarget)
    {
        return false;
    }
    return ((fStream->DeviceWidth() != fTarget->DeviceWidth()) || (fStream->DeviceHeight() != fTarget->DeviceHeight()));
}

DeviceOrientation::Type
Display::GetRelativeOrientation() const
{
    return fStream->GetRelativeOrientation();
}

DeviceOrientation::Type
Display::GetLaunchOrientation() const
{
    return fStream->GetLaunchOrientation();
}

DeviceOrientation::Type
Display::GetContentOrientation() const
{
    return fStream->GetContentOrientation();
}

DeviceOrientation::Type
Display::GetSurfaceOrientation() const
{
    return fStream->GetSurfaceOrientation();
}

Rtt_Allocator *
Display::GetAllocator() const
{
    return fOwner.GetAllocator();
}

Rtt_AbsoluteTime
Display::GetElapsedTime() const
{
    return fOwner.GetElapsedTime();
}

void
Display::GetViewProjectionMatrix(glm::mat4 &viewMatrix, glm::mat4 &projMatrix)
{
#if defined(Rtt_WIN_ENV) || defined(EMSCRIPTEN) || defined( Rtt_NXS_ENV )
    viewMatrix = glm::lookAt( glm::vec3( 0.0, 0.0, 0.5 ),
                                glm::vec3( 0.0, 0.0, 0.0 ),
                                glm::vec3( 0.0, 1.0, 0.0 ) );

    // Determine if the projection matrix needs to be rotated.
    // Note: Relative rotation is used by the simulator to rotate content to orientations the project does not support.
    //       We also might have to rotate content if the surface orientation is not upright relative to the app's orientation.
    S32 contentRotationInDegrees = fStream->GetRelativeRotation();
    S32 surfaceRotationInDegrees = DeviceOrientation::CalculateRotation(GetSurfaceOrientation(), GetContentOrientation());
    S32 totalRotationInDegrees = contentRotationInDegrees + surfaceRotationInDegrees;
    DeviceOrientation::Type relAngle = DeviceOrientation::OrientationForAngle(totalRotationInDegrees);
    
    // Fetch the scaled width and height of the surface, relative to the surface orientation.
    S32 w = RenderedContentWidth();
    S32 h = RenderedContentHeight();
    if (DeviceOrientation::IsSideways(DeviceOrientation::OrientationForAngle(surfaceRotationInDegrees)))
    {
        Swap(w, h);
    }

    // Invert top/bottom to make (0, 0) be the upper left corner of the window
    projMatrix = glm::ortho( 0.0f, static_cast<Rtt::Real>(w),
                                static_cast<Rtt::Real>(h), 0.0f,
                                0.0f, 1.0f);

    // Rotate the projection matrix, if necessary.
    if (DeviceOrientation::kUpright != relAngle)
    {
        Real halfW = Rtt_RealDiv2(Rtt_IntToReal(w));
        Real halfH = Rtt_RealDiv2(Rtt_IntToReal(h));
        projMatrix = glm::translate(projMatrix, glm::vec3(halfW, halfH, 0.0f));
        projMatrix = glm::rotate(projMatrix, glm::radians( (float)totalRotationInDegrees ), glm::vec3(0, 0.0, 1.0));
        if ( DeviceOrientation::IsSideways( relAngle ) )
        {
            Swap(halfW, halfH);
        }
        projMatrix = glm::translate(projMatrix,glm::vec3(-halfW, -halfH, 0.0f));
    }

    // Apply the letterbox or zoom event offsets.
    Real xOffset = GetXOriginOffset();
    Real yOffset = GetYOriginOffset();
    projMatrix = glm::translate(projMatrix, glm::vec3(xOffset, yOffset, 0.0f));

#else
    //TODO: Verify that the above code works on Mac, iOS, and Android. It should.

    viewMatrix = glm::lookAt( glm::vec3( 0.0, 0.0, 0.5 ),
                                glm::vec3( 0.0, 0.0, 0.0 ),
                                glm::vec3( 0.0, 1.0, 0.0 ) );

    S32 w = RenderedContentWidth();
    S32 h = RenderedContentHeight();

    // Invert top/bottom to make (0, 0) be the upper left corner of the window
    projMatrix = glm::ortho( 0.0f, static_cast<Rtt::Real>( w ),
                                static_cast<Rtt::Real>( h ), 0.0f,
                                0.0f, 1.0f );
                
    Real xOffset = GetXOriginOffset();
    Real yOffset = GetYOriginOffset();
    
    float angle = fStream->GetRelativeRotation();
    DeviceOrientation::Type relAngle = DeviceOrientation::OrientationForAngle( angle );
    
    if (DeviceOrientation::kUpright != relAngle)
    {
        if ( DeviceOrientation::IsSideways( relAngle ) )
        {
            Swap( xOffset, yOffset );
        }
        
        Real halfW = Rtt_RealDiv2( Rtt_IntToReal( w ) );
        Real halfH = Rtt_RealDiv2( Rtt_IntToReal( h ) );


        //Collapse this to minimum function calls
        projMatrix = glm::translate(projMatrix,glm::vec3(halfW, halfH, 0.0f));
        projMatrix = glm::rotate(projMatrix, glm::radians( angle ), glm::vec3(0, 0.0, 1.0));
        
        // Addd in offset BEFORE swap
        halfW -= xOffset;
        halfH -= yOffset;
        
        // For landscape, swap w,h
        if ( DeviceOrientation::IsSideways( relAngle ) )
        {
            Swap(halfW, halfH);
        }

        projMatrix = glm::translate(projMatrix,glm::vec3(-halfW, -halfH, 0.0f));
    }
    else
    {
        // Upright case
        projMatrix = glm::translate(projMatrix,glm::vec3(xOffset, yOffset, 0.0f));
    }
#endif
}

U32
Display::GetMaxTextureSize()
{
    U32 result = 1024;
    result = Renderer::GetMaxTextureSize();
    Rtt_ASSERT( result > 0 );
    return result;
}

const char *
Display::GetGlString( const char *s )
{
    return Renderer::GetGlString( s );
}

bool
Display::GetGpuSupportsHighPrecisionFragmentShaders()
{
    return Renderer::GetGpuSupportsHighPrecisionFragmentShaders();
}

size_t
Display::GetMaxVertexTextureUnits()
{
    return Renderer::GetMaxVertexTextureUnits();
}

void
Display::SetWireframe( bool newValue )
{
    fRenderer->SetWireframeEnabled( newValue );
}

void
Display::Collect( lua_State *L )
{
    // Protect against re-entrancy (not thread safe)
    if ( ! fIsCollecting )
    {
        fIsCollecting = true;
        GroupObject::CollectUnreachables( L, GetScene(), * Orphanage() );
        fIsCollecting = false;
    }
}

// NOTE: Only call this when IsRestricted() is true. Do NOT call otherwise.
// When IsRestricted() is true, then we call this function to see if a feature
// can still be used, e.g. demo mode for Simulator
bool
Display::AllowFeature( Feature value ) const
{
    const char kGraphicsType[] = "graphics";
    const char *feature = StringForFeature( value );

    // Always show message unless this flag is explicitly set to true
    if ( ! fAllowFeatureResult )
    {
        if ( feature )
        {
            lua_State *L = fOwner.VMContext().L();
            
            Rtt_LUA_STACK_GUARD( L );

            Lua::NewEvent( L, "featureRestriction" );
            lua_pushstring( L, feature );
            lua_setfield( L, -2, "feature" );
            lua_pushstring( L, kGraphicsType );
            lua_setfield( L, -2, "type" );
            Lua::DispatchRuntimeEvent( L, 0 );
            
#ifdef Rtt_AUTHORING_SIMULATOR
            // Enable demo mode *only* in simulator
            // NOTE: We toggle the value here so that we can show the message at least once.
            fAllowFeatureResult = true;
#endif
        }
    }

    // Always show this message for subscriptions below Pro.
    Rtt_TRACE_SIM( ( "WARNING: %s is a premium %s feature that requires a Pro (or higher) subscription. To view your project on a device, you must upgrade your subscription.\n", feature, kGraphicsType ) );

    return fAllowFeatureResult;
}

void
Display::SetRestrictedFeature( Feature feature, bool shouldRestrict )
{
    U32 mask = GetRestrictedFeatureMask( feature );
    const U32 p = fShouldRestrictFeature;
    fShouldRestrictFeature = ( shouldRestrict ? p | mask : p & ~mask );
}

bool
Display::IsRestrictedFeature( Feature feature ) const
{
    U32 mask = GetRestrictedFeatureMask( feature );
    return ( fShouldRestrictFeature & mask );
}

bool
Display::ShouldRestrictFeature( Feature feature ) const
{
    bool result = IsRestrictedFeature( feature ) && ! AllowRestrictedFeature( feature );

    // TODO: Remove this check once we migrate from ShouldRestrict => ShouldRestrictFeature
    if ( ! Display::IsEnterpriseFeature( feature ) )
    {
        // 'result' should match old behavior of ShouldRestrict
        Rtt_ASSERT( ShouldRestrict( feature ) == result );
    }

    return result;
}

bool
Display::AllowRestrictedFeature( Feature value ) const
{
    const char kGraphicsType[] = "graphics";
    const char *feature = StringForFeature( value );

    // Always show message unless this flag is explicitly set to true
    // Further, only Pro features will be allowed in "demo" mode
    if ( ! fAllowFeatureResult && IsProFeature( value ) )
    {
        if ( feature )
        {
            lua_State *L = fOwner.VMContext().L();
            
            Rtt_LUA_STACK_GUARD( L );

            Lua::NewEvent( L, "featureRestriction" );
            lua_pushstring( L, feature );
            lua_setfield( L, -2, "feature" );
            lua_pushstring( L, kGraphicsType );
            lua_setfield( L, -2, "type" );
            lua_pushstring( L, GetTierString( value ) );
            lua_setfield( L, -2, "tier" );
            Lua::DispatchRuntimeEvent( L, 0 );
            
#ifdef Rtt_AUTHORING_SIMULATOR
            // Enable demo mode *only* in simulator
            // NOTE: We toggle the value here so that we can show the message at least once.
            fAllowFeatureResult = true;
#endif
        }
    }

    // Always show this message for subscriptions below the required subscription tier.
    Rtt_TRACE_SIM( ( "WARNING: %s is a premium %s feature that requires a %s subscription. To view your project on a device, you must upgrade your subscription.\n", feature, kGraphicsType, GetTierString( value ) ) );

    return fAllowFeatureResult;
}

U32
Display::GetRestrictedFeatureMask( Feature feature )
{
    // Mask can only accomodate 32 features
    Rtt_STATIC_ASSERT( kNumFeatures < (sizeof(U32)*8) );

    U32 mask = 1 << (feature);
    return mask;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

