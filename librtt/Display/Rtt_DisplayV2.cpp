//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#if 0

#include "Core/Rtt_Build.h"

#include "Display/Rtt_DisplayV2.h"
#include "Display/Rtt_DisplayDefaults.h"

#include "Renderer/Rtt_GLRenderer.h"

#include "Display/Rtt_Paint.h"
#include "Display/Rtt_Scene.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"
#include "CoronaLua.h"

#include "Rtt_BufferBitmap.h"

#include "Display/Rtt_BitmapPaint.h"
#include "Renderer/Rtt_FrameBufferObject.h"
#include "Renderer/Rtt_Matrix_Renderer.h"
#include "Renderer/Rtt_Texture.h"

#include "Renderer/Rtt_Texture.h"
#include "Display/Rtt_TextureResource.h"
#include "Display/Rtt_TextureFactory.h"

// TODO: Remove when we replace TemporaryHackStream
#include "Rtt_GPUStream.h"

#include "Rtt_CPUResourcePool.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

DisplayV2::DisplayV2( Runtime& owner )
:	Super( owner ),
	fRenderer( NULL ),
	fStream( Rtt_NEW( owner.GetAllocator(), GPUStream( owner.GetAllocator() ) ) ),
	fTarget( owner.Platform().CreateScreenSurface() ),
	fImageSuffix( LUA_REFNIL )
{
}

DisplayV2::~DisplayV2()
{
	lua_State *L = GetL();
	if ( L )
	{
		luaL_unref( L, LUA_REGISTRYINDEX, fImageSuffix );
	}

	Rtt_DELETE( fTarget );
	Rtt_DELETE( fStream );
}

bool
DisplayV2::Initialize( lua_State *L, int configIndex, DeviceOrientation::Type orientation )
{
	bool result = false;

	// Only initialize once
	if ( Rtt_VERIFY( ! fRenderer ) )
	{
		Super::Initialize(L, configIndex, orientation);
	
		Rtt_Allocator *allocator = GetRuntime().GetAllocator();
		fRenderer = Rtt_NEW( allocator, GLRenderer( allocator ) );
		
		fRenderer->Initialize();
		
		CPUResourcePool *resourcePoolObserver = Rtt_NEW(allocator,CPUResourcePool());
		
		fRenderer->SetCPUResourceObserver(resourcePoolObserver);

		if ( configIndex > 0 )
		{
			ReadRenderingConfig( L, configIndex ); // assumes fStream is not NULL
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
	}

	return result;
}

// runtime.lua is pre-compiled into bytecodes and then placed in a byte array
// constant in shell.cpp. The following function defined in shell.cpp loads
// the bytecodes via luaL_loadbuffer. The .cpp file is dynamically generated.
int luaload_runtime(lua_State* L);

void
DisplayV2::ReadRenderingConfig( lua_State *L, int index )
{
	Rtt_ASSERT( fStream );

	Rtt_ASSERT( 1 == index );	
	Rtt_ASSERT( lua_istable( L, index ) );

	lua_getfield( L, index, "width" );
	int w = lua_tointeger( L, -1 );
	lua_pop( L, 1 );

	lua_getfield( L, index, "height" );
	int h = lua_tointeger( L, -1 );
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

	// Certain fields only matter if content w,h were provided:
	if ( w > 0 && h > 0 )
	{
		Rtt_ASSERT( fStream );

		fStream->Preinitialize( w, h );

		lua_getfield( L, index, "scale" );
		const char *scaleValue = lua_tostring( L, -1 );
		{
			ScaleMode scaleMode = kNone;

			if ( scaleValue )
			{
				if ( 0 == Rtt_StringCompareNoCase( scaleValue, "zoomEven" ) )
				{
					scaleMode = kZoomEven;
				}
				else if ( 0 == Rtt_StringCompareNoCase( scaleValue, "zoomStretch" ) )
				{
					scaleMode = kZoomStretch;
				}
				else if ( 0 == Rtt_StringCompareNoCase( scaleValue, "letterbox" ) )
				{
					scaleMode = kLetterbox;
				}
			}

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
					lua_getfield( L, -3, "imageSuffix" );
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
		}
		lua_pop( L, 1 );

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
DisplayV2::GetL() const
{
	const Runtime& runtime = GetRuntime();
	lua_State *L = ( runtime.IsVMContextValid() ? runtime.VMContext().L() : NULL );
	return L;
}

void
DisplayV2::Start()
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
DisplayV2::Restart()
{
	RenderingStream& stream = * fStream;
	stream.Reinitialize( * fTarget, stream.GetContentOrientation() );
	stream.PrepareToRender();
}

void
DisplayV2::Restart( DeviceOrientation::Type orientation )
{
	RenderingStream& stream = * fStream;
	stream.Reinitialize( * fTarget, orientation );
	stream.PrepareToRender();
}

void
DisplayV2::Render()
{
	Super::Render();
	GetScene().Render( * fRenderer, * fTarget );
}

void
DisplayV2::Blit()
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
DisplayV2::Invalidate()
{
	GetScene().Invalidate();
}

StageObject *
DisplayV2::GetStage()
{
	return & GetScene().CurrentStage();
}

StageObject *
DisplayV2::GetStageOffscreen()
{
	return & GetScene().OffscreenStage();
}

BitmapPaint *
DisplayV2::Capture( DisplayObject* object,
					Rect* screenBounds,
					bool will_be_saved_to_file )
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

	fRenderer->BeginFrame( 0.1f, 0.1f, GetDefaults().GetTimeTransform(), GetSx(), GetSy() );

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
	S32 x_in_content_units = 0;
	S32 y_in_content_units = 0;
	S32 w_in_content_units = 0;
	S32 h_in_content_units = 0;

	Rtt::Real offscreenViewMatrix[16];
	Rtt::CreateViewMatrix( 0.0f, 0.0f, 0.5f,
							0.0f, 0.0f, 0.0f,
							0.0f, 1.0f, 0.0f,
							offscreenViewMatrix );
	Rtt::Real offscreenProjMatrix[16];

	if ( object )
	{
		// Calculate the bounds of the given display object in pixels. Clip the object to the screen's bounds.
		Rect objectBounds = object->StageBounds();
		if (screenBounds)
		{
			objectBounds.Intersect(*screenBounds);
		}
		else
		{
			objectBounds.Intersect(GetScreenContentBounds());
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

	Rect contentScreen;
	GetContentRect(contentScreen);

	S32 x_in_pixels = x_in_content_units;

	// y_in_pixels is used with CaptureFrameBuffer(). CaptureFrameBuffer() is a wrapper around
	// glReadPixels().
	//
	// We're starting with coordinates in content-space, where:
	//
	//		The origin is in the upper left corner of the screen.
	//		X+ is towards the right of the screen.
	//		Y+ is towards the bottom of the screen.
	//
	// The parameters to glReadPixels() are expected to be in OpenGL viewport-space, where:
	//
	//		The origin is in the lower left corner of the screen.
	//		X+ is towards the right of the screen.
	//		Y+ is towards the top of the screen.
	//
	// Therefore, we have to transform the y_in_pixels from one space to the other.
	// We start with the full height of the screen in content-space units (contentScreen.yMax).
	// We'll remove both our input position (y_in_content_units) and the height of the capture
	// area (h_in_content_units) to get the starting position.
	//
	// It's necessary to remove h_in_content_units because the capture region grows towards
	// the top of the screen (that's driven by the fact that Y+ is up).
	S32 y_in_pixels = ( contentScreen.yMax - y_in_content_units - h_in_content_units );

	S32 w_in_pixels = w_in_content_units;

	S32 h_in_pixels = h_in_content_units;
	{
		ContentToScreen( x_in_pixels,
							y_in_pixels,
							w_in_pixels,
							h_in_pixels );
	}

	#if defined( Rtt_OPENGLES )
		const Texture::Format kFormat = Texture::kRGBA;
	#else
		const Texture::Format kFormat = Texture::kBGRA;
	#endif

	TextureFactory &factory = GetTextureFactory();
	SharedPtr< TextureResource > tex = factory.Create( w_in_pixels,
														h_in_pixels,
														kFormat,
														Texture::kNearest,
														Texture::kRepeat,
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
	//SET RENDER OPTIONS
	//KEEP THESE COMMENTED-OUT FOR NOW!!!!!!
	/*
	// Unflip projection
	bool isFlippedHorizontalAxis = fStream->IsProperty( RenderingStream::kFlipHorizontalAxis );
	bool isFlippedVerticalAxis = fStream->IsProperty( RenderingStream::kFlipVerticalAxis );

	if ( isFlippedHorizontalAxis ) { fStream->SetProperty( RenderingStream::kFlipHorizontalAxis, false ); }
	if ( isFlippedVerticalAxis ) { fStream->ToggleProperty( RenderingStream::kFlipVerticalAxis ); }
	fStream->UpdateProjection( false );
	*/
	// Flag the display list to draw on the next render pass, down below.

	//THIS ISN'T NECESSARY AT ALL!!!!!!!!
	//scene.Invalidate();

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
		transparentClearColor.rgba.r = 0;
		transparentClearColor.rgba.g = 0;
		transparentClearColor.rgba.b = 0;
		transparentClearColor.rgba.a = 0;
		previousClearColor.pixel = GetDefaults().GetClearColor();
		GetDefaults().SetClearColor( transparentClearColor.pixel );

		fRenderer->SetFrustum( offscreenViewMatrix, offscreenProjMatrix );

		// Render only the given object and its children, if any, to be captured down below.
		scene.Render( *fRenderer, *fTarget, *object );

		// Restore background back to its previous color.
		GetDefaults().SetClearColor( previousClearColor.pixel );
	}
	else // Full screen or screenBounds.
	{
		fRenderer->SetFrustum( offscreenViewMatrix, offscreenProjMatrix );

		scene.Render( *fRenderer, *fTarget, scene.CurrentStage() );
	}

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	//FROM THIS ORIGINAL FUNCTION MADE BUILDABLE!!!!!!!
	//KEEP THESE COMMENTED-OUT FOR NOW!!!!!!
	/*
	// Restore projection settings
	if ( isFlippedHorizontalAxis ) { fStream->ToggleProperty( RenderingStream::kFlipHorizontalAxis ); }
	if ( isFlippedVerticalAxis ) { fStream->ToggleProperty( RenderingStream::kFlipVerticalAxis ); }
	fStream->UpdateProjection( false );
	*/
	scene.Invalidate();

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	//FBO CLEANUP

	fRenderer->EndFrame();
	fRenderer->Swap();
	fRenderer->Render();

	if( will_be_saved_to_file )
	{
		// Create screen capture.
		BufferBitmap *bitmap = static_cast< BufferBitmap * >( tex->GetBitmap() );

		// This function requires coordinates in pixels.
		fStream->CaptureFrameBuffer( *bitmap,
										x_in_pixels,
										y_in_pixels,
										w_in_pixels,
										h_in_pixels );

		// We have to flip vertically because the origin in an OpenGL viewport
		// is the lower left corner, while our origin is in the upper left corner.
		// Therefore, the X axis matches, but not the Y axis.
		bitmap->Flip( false, true );
	}

	// Restore state so further rendering is unaffected
	fRenderer->SetViewport( previous_viewport_x, previous_viewport_y, previous_viewport_width, previous_viewport_height );
	fRenderer->SetFrustum( previous_viewMatrix, previous_projMatrix );
	fRenderer->SetFrameBufferObject( previous_fbo );
	fRenderer->PopMaskCount();

	Rtt_DELETE( fbo );

	return paint;
}

void
DisplayV2::UnloadResources()
{
Rtt_ASSERT_NOT_IMPLEMENTED();
	// Special case: Text objects use textures that are not backed by a file
///	TextObject::Unload( GetScene().CurrentStage() );
}

void
DisplayV2::ReloadResources()
{
Rtt_ASSERT_NOT_IMPLEMENTED();
	// Special case: Text objects use textures that are not backed by a file
///	TextObject::Reload( GetScene().CurrentStage() );
}

void
DisplayV2::GetImageSuffix( String& outSuffix ) const
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

			for ( int i = lua_objlen( L, -1 );
				  i > 0 && NULL == outSuffix.GetString();
				  i-- )
			{
				lua_rawgeti( L, -1, i ); // item = t[i]
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
DisplayV2::GetImageFilename( const char *filename, MPlatform::Directory baseDir, String& outFilename ) const
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

				for ( int i = lua_objlen( L, -1 ); i > 0 && !result; i-- )
				{
					lua_rawgeti( L, -1, i ); // item = t[i]
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

GroupObject *
DisplayV2::Overlay()
{
#ifdef Rtt_AUTHORING_SIMULATOR
	return & GetScene().Overlay();
#else
	return Super::Overlay();
#endif
}

GroupObject *
DisplayV2::Orphanage()
{
	return & GetScene().Orphanage();
}

GroupObject *
DisplayV2::HitTestOrphanage()
{
	return & GetScene().SnapshotOrphanage();
}

S32
DisplayV2::RenderedContentWidth() const
{
	return fStream->GetRenderedContentWidth();
}

S32
DisplayV2::RenderedContentHeight() const
{
	return fStream->GetRenderedContentHeight();
}

S32
DisplayV2::ViewableContentWidth() const
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
DisplayV2::ViewableContentHeight() const
{
	// See comment in Runtime::ViewableContentWidth()
	S32 renderedContentHeight = RenderedContentHeight();
	S32 contentHeight = ContentHeight();
	return Min( contentHeight, renderedContentHeight );
}

Real
DisplayV2::ActualContentWidth() const
{
	return fStream->ActualContentWidth();
}

Real
DisplayV2::ActualContentHeight() const
{
	return fStream->ActualContentHeight();
}

S32
DisplayV2::WindowWidth() const
{ 
	return fTarget->Width(); 
}
S32
DisplayV2::WindowHeight() const
{ 
	return fTarget->Height(); 
}

S32
DisplayV2::DeviceWidth() const
{
	return fStream->DeviceWidth();
}

S32
DisplayV2::DeviceHeight() const
{
	return fStream->DeviceHeight();
}

S32
DisplayV2::ScaledWidth() const
{
	return fTarget->ScaledWidth();
}

S32
DisplayV2::ScaledHeight() const
{
	return fTarget->ScaledHeight();
}

S32
DisplayV2::ContentWidth() const
{
	return fStream->ContentWidth();
}

S32
DisplayV2::ContentHeight() const
{
	return fStream->ContentHeight();
}

S32
DisplayV2::ScreenWidth() const
{
	return fStream->ScreenWidth();
}

S32
DisplayV2::ScreenHeight() const
{
	return fStream->ScreenHeight();
}

Real
DisplayV2::GetSx() const
{
	return fStream->GetSx();
}

Real
DisplayV2::GetSy() const
{
	return fStream->GetSy();
}

Real
DisplayV2::GetXOriginOffset() const
{
	return fStream->GetXOriginOffset();
}

Real
DisplayV2::GetYOriginOffset() const
{
	return fStream->GetYOriginOffset();
}

Real
DisplayV2::PointsWidth() const
{
	return fTarget->ScaledWidth();
}

Real
DisplayV2::PointsHeight() const
{
	return fTarget->ScaledHeight();
}

void
DisplayV2::SetScaleMode( ScaleMode mode, Rtt_Real screenWidth, Rtt_Real screenHeight )
{
	fStream->SetScaleMode( mode, screenWidth, screenHeight );
}

Display::ScaleMode
DisplayV2::GetScaleMode() const
{
	return fStream->GetScaleMode();
}

void
DisplayV2::ContentToScreen( S32& x, S32& y, S32& w, S32& h ) const
{
	fStream->ContentToScreen( x, y, w, h );
}

void
DisplayV2::ContentToPixels( S32& x, S32& y, S32& w, S32& h ) const
{
	fStream->ContentToPixels( x, y, w, h );
}

void
DisplayV2::CalculateContentToScreenScale( Real& outSx, Real& outSy ) const
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
DisplayV2::GetContentRect( Rect& outRect ) const
{
	const Vertex2 minCorner = { Rtt_REAL_0, Rtt_REAL_0 };
	const Vertex2 maxCorner = { Rtt_IntToReal( ContentWidth() ), Rtt_IntToReal( ContentHeight() ) };
	const Rect windowRect( minCorner, maxCorner );
	outRect = windowRect;
}

const Rect&
DisplayV2::GetScreenContentBounds() const
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
DisplayV2::SetContentOrientation( DeviceOrientation::Type newOrientation )
{
	Rtt_ASSERT( fStream );

	if ( ! GetRuntime().IsProperty( Runtime::kIsOrientationLocked ) )
	{
		RenderingStream& stream = * fStream;

		DeviceOrientation::Type oldOrientation = stream.GetContentOrientation();

//Rtt_ASSERT_NOT_IMPLEMENTED();
		stream.SetContentOrientation( newOrientation );

		GetScene().Invalidate();
		//GetScene().CurrentStage().Invalidate();
	}
}

void
DisplayV2::WindowDidRotate( DeviceOrientation::Type newOrientation, bool isNewOrientationSupported )
{
	Rtt_ASSERT( fStream );

	// Auto-rotation follows the following truth table. This is why we only need
	// to know if newOrientation is supported.
	// 
	//	isSrcSuppoted	isDstSupported		shouldAutoRotate
	//	------------------------------------------------------------------------
	//	0				1					1
	//	1				1					1
	//	0				0					0
	//	1				0					0
	const Runtime& runtime = GetRuntime();
	runtime.Begin();
	{
		bool autoRotate = isNewOrientationSupported && ( ! runtime.IsProperty( Runtime::kIsOrientationLocked ) );

		RenderingStream& stream = * fStream;


		//TODO - check if the new orientation is supported
		stream.SetOrientation(newOrientation, autoRotate);
		
		
		GetScene().Invalidate();

///		GetScene().CurrentStage().Invalidate();
	}
	runtime.End();
}

void
DisplayV2::WindowSizeChanged()
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
			if ( DeviceOrientation::IsSideways( fTarget->GetOrientation() ) )
			{
				Swap( screenW, screenH );
			}

			stream->UpdateContentScale( screenW, screenH );
		}
	}
	runtime.End();
}

DeviceOrientation::Type
DisplayV2::GetRelativeOrientation() const
{
	return fStream->GetRelativeOrientation();
}

DeviceOrientation::Type
DisplayV2::GetLaunchOrientation() const
{
	return fStream->GetLaunchOrientation();
}

DeviceOrientation::Type
DisplayV2::GetContentOrientation() const
{
	return fStream->GetContentOrientation();
}

DeviceOrientation::Type
DisplayV2::GetSurfaceOrientation() const
{
	return fStream->GetSurfaceOrientation();
}

void DisplayV2::GetViewProjectionMatrix(glm::mat4 &viewMatrix, glm::mat4 &projMatrix)
{
	viewMatrix = glm::lookAt( glm::vec3( 0.0, 0.0, 0.5 ),
										glm::vec3( 0.0, 0.0, 0.0 ),
										glm::vec3( 0.0, 1.0, 0.0 ) );
				
				
				
	
	S32 w = RenderedContentWidth();
	S32 h = RenderedContentHeight();


	// Invert top/bottom to make (0, 0) be the upper left corner of the window
	projMatrix = glm::ortho(	0.0f, static_cast<Rtt::Real>( w ),
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
		projMatrix = glm::rotate(projMatrix, angle, glm::vec3(0, 0.0, 1.0));
		
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
}


void
DisplayV2::SetWireframe( bool newValue )
{
	fRenderer->SetWireframeEnabled( newValue );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif

