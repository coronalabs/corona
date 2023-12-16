//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_LuaLibDisplay.h"

#include "Display/Rtt_BitmapPaint.h"
#include "Display/Rtt_CameraPaint.h"
#include "Display/Rtt_ClosedPath.h"
#include "Display/Rtt_CompositePaint.h"
#include "Display/Rtt_ContainerObject.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayDefaults.h"
#include "Display/Rtt_EmitterObject.h"
#include "Display/Rtt_GradientPaint.h"
#include "Display/Rtt_GroupObject.h"
#include "Display/Rtt_ImageSheetPaint.h"
#include "Display/Rtt_Paint.h"
#include "Display/Rtt_RectObject.h"
#include "Display/Rtt_Scene.h"
#include "Display/Rtt_ShaderFactory.h"
#include "Display/Rtt_ShapeAdapterPolygon.h"
#include "Display/Rtt_ShapeAdapterMesh.h"
#include "Display/Rtt_ShapeObject.h"
#include "Display/Rtt_ShapePath.h"
#include "Display/Rtt_SnapshotObject.h"
#include "Display/Rtt_StageObject.h"
#include "Display/Rtt_TextureFactory.h"
#include "Renderer/Rtt_Texture.h"
#include "Renderer/Rtt_VideoSource.h"

#include "Display/Rtt_EmitterObject.h"

#include "Corona/CoronaLibrary.h"
#include "Corona/CoronaLua.h"

#ifdef Rtt_EXPERIMENTAL_FILTER
#include "Rtt_BufferBitmap.h"
#endif // Rtt_EXPERIMENTAL_FILTER

#include "Display/Rtt_PlatformBitmap.h"
#include "Rtt_ContainerObject.h"
#include "Rtt_ClosedPath.h"
#include "Display/Rtt_EmbossedTextObject.h"
#include "Display/Rtt_ImageFrame.h"
#include "Display/Rtt_ImageSheet.h"
#include "Display/Rtt_ImageSheetUserdata.h"
#include "Rtt_LineObject.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_Matrix.h"
#include "Rtt_MPlatform.h"
#include "Display/Rtt_OpenPath.h"
#include "Rtt_RenderingStream.h"
#include "Rtt_Runtime.h"
#include "Display/Rtt_SpriteObject.h"
#include "Display/Rtt_TextObject.h"
#include "Renderer/Rtt_Texture.h"
#include "Renderer/Rtt_Renderer.h"

#include "Rtt_Event.h"
#include "Rtt_LuaResource.h"
#include "Rtt_Profiling.h"

#include "Core/Rtt_StringHash.h"
#include "Core/Rtt_String.h"

#include <string.h>

#include "Rtt_LuaAux.h"

#ifdef Rtt_WIN_ENV
#undef CreateFont
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class DisplayLibrary
{
	public:
		typedef DisplayLibrary Self;

	public:
		static const char kName[];

	protected:
		static const char kStatusBarModes[];

	protected:
		DisplayLibrary( Display& display );
		~DisplayLibrary();

	public:
		Display& GetDisplay() { return fDisplay; }

	public:
		static int Open( lua_State *L );

	protected:
		static int Initialize( lua_State *L );
		static int Finalizer( lua_State *L );

	public:
		static Self *ToLibrary( lua_State *L );

	protected:
		static int ValueForKey( lua_State *L );

	public:
		static ShapeObject* PushImage(
			lua_State *L,
			Vertex2* topLeft,
			BitmapPaint* paint,
			Display& display,
			GroupObject *parent,
			Real w,
			Real h );
		static ShapeObject* PushImage(
			lua_State *L,
			Vertex2* topLeft,
			BitmapPaint* paint,
			Display& display,
			GroupObject *parent );

	public:
		static int newCircle( lua_State *L );
		static int newPolygon( lua_State *L );
		static int newRect( lua_State *L );
		static int newRoundedRect( lua_State *L );
		static int newLine( lua_State *L );
		static int newImage( lua_State *L );
		static int newImageRect( lua_State *L );
		static int newEmitter( lua_State *L );
		static int newText( lua_State *L );
		static int newEmbossedText( lua_State *L );
		static int newGroup( lua_State *L );
		static int newContainer( lua_State *L );
		static int _newContainer( lua_State *L );
		static int newSnapshot( lua_State *L );
		static int newSprite( lua_State *L );
		static int newMesh( lua_State *L );
		static int getDefault( lua_State *L );
		static int setDefault( lua_State *L );
		static int getCurrentStage( lua_State *L );
		static int collectOrphans( lua_State *L );
		static int setStatusBar( lua_State *L );
		static int capture( lua_State *L );
		static int captureBounds( lua_State *L );
		static int captureScreen( lua_State *L );
        static int save( lua_State *L );
		static int colorSample( lua_State *L );
		static int setDrawMode( lua_State *L );
		static int getSafeAreaInsets( lua_State *L );
		static int enableStatistics( lua_State *L );
		static int getStatistics( lua_State *L );
		static int getSums( lua_State *L );
		static int getTimings( lua_State *L );

		static int _initProfiling( lua_State *L );
		static int _allocateProfile( lua_State *L );
	#ifdef Rtt_DEBUG
		static int _getlevelcounts( lua_State *L );
		static int _getlevelinfo( lua_State *L );
		static int _getfirst( lua_State *L );
		static int _getnameandbelow( lua_State *L );
	#endif
		static int _beginProfile( lua_State *L );
		static int _addProfileEntry( lua_State *L );
		static int _endProfile( lua_State *L );

	private:
		static void GetRect( lua_State *L, Rect &bounds );

		Display& fDisplay;
};

// ----------------------------------------------------------------------------

const char DisplayLibrary::kName[] = "display";
const char DisplayLibrary::kStatusBarModes[MPlatform::kNumModes + 1] = "0123";

// ----------------------------------------------------------------------------

DisplayLibrary::DisplayLibrary( Display& display )
:	fDisplay( display )
{
}

DisplayLibrary::~DisplayLibrary()
{
}

int
DisplayLibrary::Open( lua_State *L )
{
	Display *display = (Display *)lua_touserdata( L, lua_upvalueindex( 1 ) );
	Rtt_ASSERT( display );

	// Register __gc callback
	const char kMetatableName[] = __FILE__; // Globally unique string to prevent collision
	CoronaLuaInitializeGCMetatable( L, kMetatableName, Finalizer );

	// Functions in library
	const luaL_Reg kVTable[] =
	{
		{ "newCircle", newCircle },
		{ "newPolygon", newPolygon },
		{ "newRect", newRect },
		{ "newRoundedRect", newRoundedRect },
		{ "newLine", newLine },
		{ "newImage", newImage },
		{ "newImageRect", newImageRect },
		{ "newEmitter", newEmitter },
		{ "newText", newText },
		{ "newEmbossedText", newEmbossedText },
		{ "newGroup", newGroup },
		{ "newContainer", newContainer },
		{ "_newContainer", _newContainer },
		{ "newSnapshot", newSnapshot },
		{ "newSprite", newSprite },
		{ "newMesh", newMesh },
		{ "getDefault", getDefault },
		{ "setDefault", setDefault },
		{ "getCurrentStage", getCurrentStage },
		{ "_collectOrphans", collectOrphans },
		{ "setStatusBar", setStatusBar },
		{ "capture", capture },
		{ "captureBounds", captureBounds },
		{ "captureScreen", captureScreen },
		{ "save", save },
		{ "colorSample", colorSample },
		{ "setDrawMode", setDrawMode },
		{ "getSafeAreaInsets", getSafeAreaInsets },
		{ "enableStatistics", enableStatistics },
		{ "getStatistics", getStatistics },
		{ "getSums", getSums },
		{ "getTimings", getTimings },

		{ "_initProfiling", _initProfiling },
		{ "_allocateProfile", _allocateProfile },
	#ifdef Rtt_DEBUG
		{ "_getlevelcounts", _getlevelcounts },
		{ "_getlevelinfo", _getlevelinfo },
		{ "_getfirst", _getfirst },
		{ "_getnameandbelow", _getnameandbelow },
	#endif
		{ "_beginProfile", _beginProfile },
		{ "_addProfileEntry", _addProfileEntry },
		{ "_endProfile", _endProfile },

		{ NULL, NULL }
	};

	// Set library as upvalue for each library function
	Self *library = Rtt_NEW( & display->GetRuntime().GetAllocator(), Self( * display ) );

	// Store the library singleton in the registry so it persists
	// using kMetatableName as the unique key.
	CoronaLuaPushUserdata( L, library, kMetatableName ); // push ud
	lua_pushstring( L, kMetatableName ); // push key
	lua_settable( L, LUA_REGISTRYINDEX ); // pops ud, key

	// Leave library" on top of stack
	// Set library as upvalue for each library function
	int result = CoronaLibraryNew( L, kName, "com.coronalabs", 1, 1, kVTable, library );
	{
		lua_pushlightuserdata( L, library );
		lua_pushcclosure( L, ValueForKey, 1 ); // pop ud
		CoronaLibrarySetExtension( L, -2 ); // pop closure

		lua_pushlightuserdata( L, UserdataForEnum( kStatusBarModes, MPlatform::kHiddenStatusBar ) );
		lua_setfield( L, -2, "HiddenStatusBar" );
		lua_pushlightuserdata( L, UserdataForEnum( kStatusBarModes, MPlatform::kDefaultStatusBar ) );
		lua_setfield( L, -2, "DefaultStatusBar" );
		lua_pushlightuserdata( L, UserdataForEnum( kStatusBarModes, MPlatform::kTranslucentStatusBar ) );
		lua_setfield( L, -2, "TranslucentStatusBar" );
		lua_pushlightuserdata( L, UserdataForEnum( kStatusBarModes, MPlatform::kDarkStatusBar ) );
		lua_setfield( L, -2, "DarkStatusBar" );
		lua_pushlightuserdata( L, UserdataForEnum( kStatusBarModes, MPlatform::kLightTransparentStatusBar ) );
		lua_setfield( L, -2, "LightTransparentStatusBar" );
		lua_pushlightuserdata( L, UserdataForEnum( kStatusBarModes, MPlatform::kDarkTransparentStatusBar ) );
		lua_setfield( L, -2, "DarkTransparentStatusBar" );



		const char *refPts = LuaLibDisplay::ReferencePoints();
		lua_pushlightuserdata( L, UserdataForEnum( refPts, DisplayObject::kReferenceCenter ) );
		lua_setfield( L, -2, "CenterReferencePoint" );
		lua_pushlightuserdata( L, UserdataForEnum( refPts, DisplayObject::kReferenceTopLeft ) );
		lua_setfield( L, -2, "TopLeftReferencePoint" );
		lua_pushlightuserdata( L, UserdataForEnum( refPts, DisplayObject::kReferenceTopCenter ) );
		lua_setfield( L, -2, "TopCenterReferencePoint" );
		lua_pushlightuserdata( L, UserdataForEnum( refPts, DisplayObject::kReferenceTopRight ) );
		lua_setfield( L, -2, "TopRightReferencePoint" );
		lua_pushlightuserdata( L, UserdataForEnum( refPts, DisplayObject::kReferenceCenterRight ) );
		lua_setfield( L, -2, "CenterRightReferencePoint" );
		lua_pushlightuserdata( L, UserdataForEnum( refPts, DisplayObject::kReferenceBottomRight ) );
		lua_setfield( L, -2, "BottomRightReferencePoint" );
		lua_pushlightuserdata( L, UserdataForEnum( refPts, DisplayObject::kReferenceBottomCenter ) );
		lua_setfield( L, -2, "BottomCenterReferencePoint" );
		lua_pushlightuserdata( L, UserdataForEnum( refPts, DisplayObject::kReferenceBottomLeft ) );
		lua_setfield( L, -2, "BottomLeftReferencePoint" );
		lua_pushlightuserdata( L, UserdataForEnum( refPts, DisplayObject::kReferenceCenterLeft ) );
		lua_setfield( L, -2, "CenterLeftReferencePoint" );
	}

	return result;
}

int
DisplayLibrary::Finalizer( lua_State *L )
{
	Self *library = (Self *)CoronaLuaToUserdata( L, 1 );

	delete library;
	
	return 0;
}

DisplayLibrary *
DisplayLibrary::ToLibrary( lua_State *L )
{
	// library is pushed as part of the closure
	Self *library = (Self *)lua_touserdata( L, lua_upvalueindex( 1 ) );
	return library;
}

int
DisplayLibrary::ValueForKey( lua_State *L )
{
	int result = 1;

	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();

	const char *key = lua_tostring( L, 2 ); Rtt_ASSERT( key );

	static const char * keys[] = 
	{
		"stageWidth",			// 0
		"contentWidth",			// 1
		"stageHeight",			// 2
		"contentHeight",		// 3
		"viewableContentWidth",	// 4
		"viewableContentHeight", // 5
		"statusBarHeight",		// 6
		"fps",					// 7
		"currentStage",			// 8
		"screenOriginX",		// 9
		"screenOriginY",		// 10
		"contentScaleX",		// 11
		"contentScaleY",		// 12		
		"contentCenterX",		// 13
		"contentCenterY",		// 14
		"imageSuffix",			// 15
        "pixelWidth",			// 16
        "pixelHeight",			// 17
		"actualContentWidth", // 18
		"actualContentHeight", // 19
        "topStatusBarContentHeight",//20
        "bottomStatusBarContentHeight",//21
		"safeScreenOriginX", //22
		"safeScreenOriginY", //23
		"safeActualContentWidth", //24
		"safeActualContentHeight", //25
	};
	
	static StringHash sHash( *LuaContext::GetAllocator( L ), keys, sizeof( keys ) / sizeof(const char *), 26, 26, 17, __FILE__, __LINE__ );
	StringHash *hash = &sHash;

	int index = hash->Lookup( key );
	switch ( index )
	{
	case 0:	// "stageWidth"
		{
			CoronaLuaWarning( L, "display.stageWidth has been deprecated. Use display.contentWidth instead" );
			lua_pushinteger( L, display.ContentWidthUpright() );
		}
		break;
	case 1:	//"contentWidth"
		{
			lua_pushinteger( L, display.ContentWidthUpright() );
		}
		break;
	case 2:	// "stageHeight"
		{
			CoronaLuaWarning( L, "display.stageHeight has been deprecated. Use display.contentHeight instead" );
			lua_pushinteger( L, display.ContentHeightUpright() );
		}
		break;
	case 3:	// "contentHeight"
		{
			lua_pushinteger( L, display.ContentHeightUpright() );
		}
		break;
	case 4:	// "viewableContentWidth"
		{
			lua_pushinteger( L, display.ViewableContentWidthUpright() );
		}
		break;
	case 5:	// "viewableContentHeight"
		{
			lua_pushinteger( L, display.ViewableContentHeightUpright() );
		}
		break;
	case 6:	// "statusBarHeight"
		{
			Runtime& runtime = * LuaContext::GetRuntime( L );
			lua_pushinteger( L, runtime.Platform().GetStatusBarHeight() );
		}
		break;
	case 7:	// "fps"
		{
			lua_pushinteger( L, display.GetRuntime().GetFPS() );
		}
		break;
	case 8:	// "currentStage"
		{
			display.GetStage()->GetProxy()->PushTable( L );
		}
		break;
	case 9:	// "screenOriginX"
		{
			lua_pushnumber( L, Rtt_RealToFloat( - display.GetXOriginOffset() ) );
		}
		break;
	case 10:	// "screenOriginY"
		{
			lua_pushnumber( L, Rtt_RealToFloat( - display.GetYOriginOffset() ) );
		}
		break;
	case 11:	// "contentScaleX"
		{
			lua_pushnumber( L, Rtt_RealToFloat( display.GetSxUpright() ) );
		}
		break;
	case 12:	// "contentScaleY"
		{
			lua_pushnumber( L, Rtt_RealToFloat( display.GetSyUpright() ) );
		}
		break;
	case 13:	// "contentCenterX"
		{
			lua_pushnumber( L, 0.5*display.ContentWidthUpright() );
		}
		break;
	case 14:	// "contentCenterY"
		{
			lua_pushnumber( L, 0.5*display.ContentHeightUpright() );
		}
		break;
	case 15:	// "imageSuffix"
		{
			String suffix( LuaContext::GetAllocator( L ) );
			display.GetImageSuffix( suffix );
			const char *str = suffix.GetString();
			if ( str )
			{
				lua_pushstring( L, str );
			}
			else
			{
				lua_pushnil( L );
			}
		}
		break;
	case 16:	// "pixelWidth"
		{
			lua_pushnumber( L, display.DeviceWidth() );
		}
		break;
	case 17:	// "pixelHeight"
		{
			lua_pushnumber( L, display.DeviceHeight() );
		}
		break;
	case 18:	// "actualContentWidth"
		{
			lua_pushnumber( L, display.ActualContentWidth() );
		}
		break;
	case 19:	// "actualContentHeight"
		{
			lua_pushnumber( L, display.ActualContentHeight() );
		}
		break;
    case 20:	// "topStatusBarContentHeight"
		{
            Runtime& runtime = * LuaContext::GetRuntime( L );
            Real result = Rtt_RealMul(Rtt_IntToReal(runtime.Platform().GetTopStatusBarHeightPixels()),display.GetSy());
			lua_pushnumber( L,  Rtt_RealToFloat(result));
		}
        break;
    case 21:	// "bottomStatusBarContentHeight"
		{
            Runtime& runtime = * LuaContext::GetRuntime( L );
            Real result = Rtt_RealMul(Rtt_IntToReal(runtime.Platform().GetBottomStatusBarHeightPixels()),display.GetSy());
			lua_pushnumber( L, Rtt_RealToFloat(result));
		}
        break;
	case 22: // safeScreenOriginX
		{
			Runtime& runtime = * LuaContext::GetRuntime( L );

			Rtt_Real top, left, bottom, right;
			runtime.Platform().GetSafeAreaInsetsPixels(top, left, bottom, right);
			lua_pushnumber( L, (left*display.GetSx()) - display.GetXOriginOffset() );
		}
		break;
	case 23: // safeScreenOriginY
		{
			Runtime& runtime = * LuaContext::GetRuntime( L );

			Rtt_Real top, left, bottom, right;
			runtime.Platform().GetSafeAreaInsetsPixels(top, left, bottom, right);
			lua_pushnumber( L, (top*display.GetSy()) - display.GetYOriginOffset() );
		}
		break;
	case 24: // safeActualContentWidth
		{
			Runtime& runtime = * LuaContext::GetRuntime( L );

			Rtt_Real top, left, bottom, right;
			runtime.Platform().GetSafeAreaInsetsPixels(top, left, bottom, right);
			lua_pushnumber( L, display.ActualContentWidth() - ((left + right)*display.GetSx()) );
		}
		break;
	case 25: // safeActualContentHeight
		{
			Runtime& runtime = * LuaContext::GetRuntime( L );

			Rtt_Real top, left, bottom, right;
			runtime.Platform().GetSafeAreaInsetsPixels(top, left, bottom, right);
			lua_pushnumber( L, display.ActualContentHeight() - ((top + bottom)*display.GetSy()) );
		}
		break;
	default:
		{
			result = 0;
		}
		break;
	}

	return result;
}

// ----------------------------------------------------------------------------

static GroupObject*
GetParent( lua_State *L, int& nextArg )
{
	GroupObject* parent = NULL;

	if ( lua_istable( L, nextArg ) )
	{
		Rtt_WARN_SIM_PROXY_TYPE( L, nextArg, GroupObject );

		DisplayObject *object = (DisplayObject*)LuaProxy::GetProxyableObject( L, nextArg );
		parent = object ? object->AsGroupObject() : NULL;
		if ( parent )
		{
			++nextArg;
		}
	}

	return parent;
}

static void
AssignDefaultFillColor( const Display& display, ShapeObject& o )
{

	SharedPtr< TextureResource > resource = display.GetTextureFactory().GetDefault();

	Rtt_ASSERT( ! o.GetPath().GetFill() );
	Paint *p = Paint::NewColor(
						display.GetAllocator(),
						resource, display.GetDefaults().GetFillColor() );
	o.SetFill( p );
}

#ifdef OLD_GRAPHICS
LinearGradientPaint*
DisplayLibrary::newLinearGradient( lua_State *L, int index )
{
	LinearGradientPaint *p = NULL;

	LinearGradientData **ud = (LinearGradientData **)luaL_checkudata( L, index, LinearGradientData::kMetatableName );
	if ( ud )
	{
		LinearGradientData *data = *ud;
		if ( data )
		{
			p = Paint::NewLinearGradient( LuaContext::GetRuntime( L )->Allocator(), * data );
		}
	}

	return p;
}
#endif

ShapeObject*
DisplayLibrary::PushImage(
	lua_State *L,
	Vertex2* topLeft,
	BitmapPaint* paint,
	Display& display,
	GroupObject *parent,
	Real w,
	Real h )
{
	ShapeObject* v = RectObject::NewRect( display.GetAllocator(), w, h );

	int result = LuaLibDisplay::AssignParentAndPushResult( L, display, v, parent );
	if ( Rtt_VERIFY( result ) )
	{
		if ( topLeft )
		{
			Real x = topLeft->x;
			Real y = topLeft->y;

			if ( display.GetDefaults().IsV1Compatibility() )
			{
				x += Rtt_RealDiv2( w );
				y += Rtt_RealDiv2( h );
			}
			v->Translate( x, y );
		}
		v->SetFill( paint );
	}
	else
	{
		Rtt_DELETE( v );
		v = NULL;
	}
	return v;
}

ShapeObject*
DisplayLibrary::PushImage(
	lua_State *L,
	Vertex2* topLeft,
	BitmapPaint* paint,
	Display& display,
	GroupObject *parent )
{
	// Fetch image's width and height. (Might be auto-downscaled here due to texture size limitations.)
	PlatformBitmap *bitmap = paint->GetBitmap();
	Texture *texture = paint->GetTexture(); Rtt_ASSERT( texture );

	// bitmap may be NULL when FBO's are involved
	Real width = Rtt_IntToReal( bitmap ? bitmap->UprightWidth() : texture->GetWidth() );
	Real height = Rtt_IntToReal( bitmap ? bitmap->UprightHeight() : texture->GetHeight() );

	// Create the image object with the above dimensions.
	ShapeObject* v = PushImage( L, topLeft, paint, display, parent, width, height );

    v->SetObjectDesc("ImageObject");
    
	/*
	// TODO: Figure out what to do with this block once Android is ready...

	// Check if the image was downscaled/downsampled when loaded from file. This happens if it exceeds texture size limits.
	// If the image was downscaled, then scale up the display object to match the size of the original unscaled image.
	if ( v && paint->GetBitmap().WasScaled() && (paint->GetBitmap().GetScale() > 0) )
	{
		Real scale = Rtt_RealDiv( Rtt_REAL_1, paint->GetBitmap().GetScale() );
		Real lastWidth = width;
		Real lastHeight = height;
		width = Rtt_RealMul( width, scale );
		height = Rtt_RealMul( height, scale );
		v->Translate( Rtt_RealDiv2( width - lastWidth ), Rtt_RealDiv2( height - lastHeight ) );
		v->Scale( scale, scale, false );
	}

	// If full resolution property has not been set, then automatically downscale the longest length to fit the stage's longest length.
	// For example, a landscape sized image will be downscaled to fit a portrait stage's height.
	// The intent is to allow loading portrait/landscape orientation images on startup for apps that support orientation changes.
	if ( v && ! paint->GetBitmap().IsProperty(PlatformBitmap::kIsBitsFullResolution) )
	{
		Real w = width;
		Real h = height;

		const PlatformSurface& surface = runtime.Surface();
		S32 wMax = surface.DeviceWidth();
		S32 hMax = surface.DeviceHeight();

		// Align longest image edge to the longest screen edge to calculate proper scale.
		// If image is landscape and screen size is portrait (or vice versa), 
		// then swap screen dimensions. 
		bool isImageLandscape = w > h;
		bool isScreenLandscape = wMax > hMax;
		if ( isImageLandscape ^ isScreenLandscape )
		{
			Swap( wMax, hMax );
		}

		Real scale = Rtt_REAL_1;
		if ( w > wMax || h > hMax )
		{
			Real scaleW = Rtt_RealDivNonZeroB( Rtt_IntToReal( wMax ), w );
			Real scaleH = Rtt_RealDivNonZeroB( Rtt_IntToReal( hMax ), h );
			scale = ( scaleH < scaleW ? scaleH : scaleW );
		}

		if (scale < Rtt_REAL_1)
		{
			Real lastWidth = width;
			Real lastHeight = height;
			width = Rtt_RealMul(width, scale);
			height = Rtt_RealMul(height, scale);
			v->Translate(Rtt_RealDiv2( width - lastWidth ), Rtt_RealDiv2( height - lastHeight ));
			v->Scale(scale, scale, false);
		}
	}
	*/
	
	return v;
}

int
DisplayLibrary::newCircle( lua_State *L )
{
	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();

	int nextArg = 1;
	GroupObject *parent = GetParent( L, nextArg );

	Real x = luaL_checkreal( L, nextArg++ );
	Real y = luaL_checkreal( L, nextArg++ );
	Real r = luaL_checkreal( L, nextArg++ );

	ShapePath *path = ShapePath::NewCircle( display.GetAllocator(), r );
	ShapeObject *v = Rtt_NEW( display.GetAllocator(), ShapeObject( path ) );

	int result = LuaLibDisplay::AssignParentAndPushResult( L, display, v, parent );
	AssignDefaultFillColor( display, * v );
	v->Translate( x, y );

	return result;
}

int
DisplayLibrary::newPolygon( lua_State *L )
{
	int result = 0;

	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();

	int nextArg = 1;
	GroupObject *parent = GetParent( L, nextArg );

	Real x = luaL_checkreal( L, nextArg++ );
	Real y = luaL_checkreal( L, nextArg++ );

	ShapePath *path = ShapePath::NewPolygon( display.GetAllocator() );

	TesselatorPolygon *tesselator = (TesselatorPolygon *)path->GetTesselator();
	if ( ShapeAdapterPolygon::InitializeContour( L, nextArg, * tesselator ) )
	{
		ShapeObject *v = Rtt_NEW( display.GetAllocator(), ShapeObject( path ) );

		result = LuaLibDisplay::AssignParentAndPushResult( L, display, v, parent );
		AssignDefaultFillColor( display, * v );
		v->Translate( x, y );
	}
	else
	{
		luaL_argerror( L, nextArg, "ERROR: display.newPolygon() expected an array of vertices" );
		Rtt_DELETE( path );
	}

	if ( display.GetDefaults().IsV1Compatibility() )
	{
		CoronaLuaWarning( L, "display.newPolygon() is only supported in graphics 2.0. Your mileage may vary in graphicsCompatibility 1.0 mode" );
	}

	return result;
}

	
int
DisplayLibrary::newMesh( lua_State *L )
{
	int result = 0;
	
	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();
	
	int nextArg = 1;

	GroupObject *parent = NULL;
	Real x=0,y=0;
	
	if ( lua_istable( L, nextArg ) && LuaProxy::IsProxy(L, nextArg))
	{
		parent = GetParent( L, nextArg );
	}
	
	if(lua_type(L, nextArg) == LUA_TNUMBER && lua_type(L, nextArg+1) == LUA_TNUMBER)
	{
		x = lua_tonumber( L, nextArg++ );
		y = lua_tonumber( L, nextArg++ );
	}
	
	if(lua_istable( L, nextArg ))
	{
		lua_getfield( L, -1, "parent" );
		if ( lua_istable( L, -1) )
		{
			int parentArg = Lua::Normalize( L, -1 );
			parent = GetParent( L, parentArg );
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, -1, "x" );
		if (lua_type( L, -1 ) == LUA_TNUMBER)
		{
			x = lua_tonumber( L, -1 );
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, -1, "y" );
		if (lua_type( L, -1 ) == LUA_TNUMBER)
		{
			y = lua_tonumber( L, -1 );
		}
		lua_pop( L, 1 );
	}
	else
	{
		CoronaLuaError( L, "display.newMesh() bad argument #%d: table expected but got %s",
					   nextArg, lua_typename( L, lua_type( L, nextArg ) ));
		return result;
	}
	
	ShapePath *path = ShapePath::NewMesh( display.GetAllocator(), ShapeAdapterMesh::GetMeshMode( L, nextArg) );
	
	TesselatorMesh *tesselator = (TesselatorMesh *)path->GetTesselator();
	if ( ShapeAdapterMesh::InitializeMesh( L, nextArg, * tesselator ) )
	{
		ShapeObject *v = Rtt_NEW( display.GetAllocator(), ShapeObject( path ) );
		
		if (tesselator->GetFillPrimitive() == Geometry::kIndexedTriangles)
		{
			path->Invalidate( ShapePath::kFillSourceIndices );
		}
		
		result = LuaLibDisplay::AssignParentAndPushResult( L, display, v, parent );
		AssignDefaultFillColor( display, *v );
		v->Translate( x, y );
	}
	else
	{
		Rtt_DELETE( path );
	}
	
	if ( display.GetDefaults().IsV1Compatibility() )
	{
		CoronaLuaWarning( L, "display.newMesh() is only supported in graphics 2.0" );
	}
	
	return result;
}
	
int
DisplayLibrary::newRect( lua_State *L )
{
	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();

	int nextArg = 1;
	GroupObject *parent = GetParent( L, nextArg );

	Real x = luaL_checkreal( L, nextArg++ );
	Real y = luaL_checkreal( L, nextArg++ );
	Real w = luaL_checkreal( L, nextArg++ );
	Real h = luaL_checkreal( L, nextArg++ );

	ShapeObject* v = RectObject::NewRect( display.GetAllocator(), w, h );
	int result = LuaLibDisplay::AssignParentAndPushResult( L, display, v, parent );

	if ( display.GetDefaults().IsV1Compatibility() )
	{
		x += Rtt_RealDiv2( w );
		y += Rtt_RealDiv2( h );
	}
	v->Translate( x, y );
	AssignDefaultFillColor( display, * v );

	return result;
}

int
DisplayLibrary::newRoundedRect( lua_State *L )
{
	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();

	int nextArg = 1;
	GroupObject *parent = GetParent( L, nextArg );

	Real x = luaL_checkreal( L, nextArg++ );
	Real y = luaL_checkreal( L, nextArg++ );
	Real w = luaL_checkreal( L, nextArg++ );
	Real h = luaL_checkreal( L, nextArg++ );
	Real r = luaL_checkreal( L, nextArg++ );

	ShapePath *path = ShapePath::NewRoundedRect( display.GetAllocator(), w, h, r );
	ShapeObject *v = Rtt_NEW( display.GetAllocator(), ShapeObject( path ) );

	int result = LuaLibDisplay::AssignParentAndPushResult( L, display, v, parent );

	if ( display.GetDefaults().IsV1Compatibility() )
	{
		x += Rtt_RealDiv2( w );
		y += Rtt_RealDiv2( h );
	}
	v->Translate( x, y );
	AssignDefaultFillColor( display, * v );

	return result;
}

int
DisplayLibrary::newLine( lua_State *L )
{
	Self *library = ToLibrary( L );
	Display &display = library->GetDisplay();
	Runtime &runtime = *LuaContext::GetRuntime( L );
	Rtt_Allocator *pAllocator = runtime.Allocator();

	int nextArg = 1;
	GroupObject *parent = GetParent( L, nextArg );

	// number of parameters (excluding self)
	int numArgs = lua_gettop( L );

	Vertex2 translate_to_origin = { 0.0f, 0.0f };

	OpenPath *path = Rtt_NEW( pAllocator, OpenPath( pAllocator ) );
	{
		/*
		printf( "***\n*** %s\n*** numArgs: %d\n***\n",
				Rtt_FUNCTION,
				numArgs );
		*/
		int end = ( numArgs - nextArg );

		if( ! ( end & 1 ) )
		{
			luaL_error( L, "ERROR: display.newLine() requires an even number of vertices (got %d)", ( end + 1 ) );
		}
        else if (end < 3)
        {
			luaL_error( L, "ERROR: display.newLine() requires at least 4 vertices (got %d)", ( end + 1 ) );
        }

		for ( int i = 0; i < end; i += 2 )
		{
			Vertex2 v =
			{
				luaL_checkreal( L, nextArg + ( i + 0 ) ),
				luaL_checkreal( L, nextArg + ( i + 1 ) )
			};

			//printf( "*** %3.1f %3.1f\n", v.x, v.y );

			if( i == 0 )
			{
				// First point.
				// The first point is at the origin, so it's always 0,0.
				// Save the origin so we can translate the rest.
				translate_to_origin = v;
			}

			v.x -= translate_to_origin.x;
			v.y -= translate_to_origin.y;

			path->Append( v );
		}

		//printf( "***\n***\n***\n" );
	}

	int result = 0;
	{
		LineObject* o = Rtt_NEW( pAllocator, LineObject( path ) );
		result = LuaLibDisplay::AssignParentAndPushResult( L, display, o, parent );
		o->Translate( translate_to_origin.x, translate_to_origin.y );

		// Assign default line width
		o->SetStrokeWidth( Rtt_REAL_1 );

		// Assign default color
		SharedPtr< TextureResource > resource = display.GetTextureFactory().GetDefault();
		Paint *p = Paint::NewColor( runtime.Allocator(), resource, display.GetDefaults().GetLineColor() );
		o->SetStroke( p );
	}

	return result;
}

// display.newImage( [parentGroup,] filename [, baseDirectory] [, x, y] [,isFullResolution] )
// display.newImage( [parentGroup,] imageSheet, frameIndex [, x, y] )
int
DisplayLibrary::newImage( lua_State *L )
{
	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();

	int result = 0;
#ifdef Rtt_DEBUG
	int top = lua_gettop( L );
#endif
	// [parentGroup,]
	int nextArg = 1;
	GroupObject *parent = GetParent( L, nextArg );

	// Only required param is "filename"
	// filename [, baseDirectory]
	if ( lua_isstring( L, nextArg ) )
	{
		const char *imageName = lua_tostring( L, nextArg++ );

		MPlatform::Directory baseDir = MPlatform::kResourceDir;
		if ( lua_islightuserdata( L, nextArg ) )
		{
			void* p = lua_touserdata( L, nextArg );
			baseDir = (MPlatform::Directory)EnumForUserdata( LuaLibSystem::Directories(),
															 p,
															 MPlatform::kNumDirs,
															 MPlatform::kResourceDir );
			++nextArg;
		}

		// [, x, y]
		Vertex2* p = NULL;
		Vertex2 topLeft = { Rtt_REAL_0, Rtt_REAL_0 }; // default to (0,0) if not supplied
		if ( lua_isnumber( L, nextArg ) && lua_isnumber( L, nextArg + 1 ) )
		{
			topLeft.x = luaL_toreal( L, nextArg++ );
			topLeft.y = luaL_toreal( L, nextArg++ );
		}
		p = & topLeft;

		U32 flags = 0;

		// [,isFullResolution]
		if ( lua_isboolean( L, nextArg ) && lua_toboolean( L, nextArg ) )
		{
			flags |= PlatformBitmap::kIsBitsFullResolution;
		}

		Runtime& runtime = library->GetDisplay().GetRuntime();
		BitmapPaint *paint = BitmapPaint::NewBitmap( runtime, imageName, baseDir, flags );

		if ( paint && paint->GetBitmap() && paint->GetBitmap()->NumBytes() == 0 )
		{
			CoronaLuaWarning(L, "file '%s' does not contain a valid image", imageName);
		}

		if ( paint )
		{
			result = NULL != PushImage( L, p, paint, display, parent );
		}
	}
	else if ( lua_isuserdata( L, nextArg ) )
	{
		ImageSheetUserdata *ud = ImageSheet::ToUserdata( L, nextArg );
		if ( ud )
		{
			nextArg++;
			const AutoPtr< ImageSheet >& sheet = ud->GetSheet();

			int frameIndex = (int) lua_tointeger( L, nextArg );
			if ( frameIndex <= 0 )
			{
				CoronaLuaWarning( L, "display.newImage( imageGroup, frameIndex ) given an invalid frameIndex (%d). Defaulting to 1", frameIndex );
				frameIndex = 1;
			}

			// Map 1-based Lua indices to 0-based C indices
			--frameIndex;
			++nextArg;

			// [, x, y]
			Vertex2* p = NULL;
			Vertex2 topLeft = { Rtt_REAL_0, Rtt_REAL_0 }; // default to (0,0) if not supplied
			if ( lua_isnumber( L, nextArg ) && lua_isnumber( L, nextArg + 1 ) )
			{
				topLeft.x = luaL_toreal( L, nextArg++ );
				topLeft.y = luaL_toreal( L, nextArg++ );
			}
			p = & topLeft;
            
            if( sheet->GetNumFrames() <= frameIndex )
            {
                CoronaLuaWarning( L, "display.newImage( imageGroup, frameIndex ) given an invalid frameIndex (%d). Defaulting to max frame", frameIndex+1 );
                frameIndex = sheet->GetNumFrames()-1;
            }
			const ImageFrame *frame = sheet->GetFrame( frameIndex );
			Real w = Rtt_IntToReal( frame->GetWidth() );
			Real h = Rtt_IntToReal( frame->GetHeight() );

			ImageSheetPaint *paint = ImageSheetPaint::NewBitmap( display.GetAllocator(), sheet, frameIndex );

			if ( paint )
			{
				result = NULL != PushImage( L, p, paint, display, parent, w, h );
			}
		}
	}
	else
	{
		CoronaLuaError( L, "display.newImage() bad argument #%d: filename or image sheet expected, but got %s",
				nextArg, lua_typename( L, lua_type( L, nextArg ) ) );
	}

	Rtt_ASSERT( lua_gettop( L ) == (top + result) );

	return result;
}


// display.newImageRect( [parentGroup,] filename [, baseDirectory], w, h )
// display.newImageRect( imageGroup, frameIndex, w, h )
int
DisplayLibrary::newImageRect( lua_State *L )
{
	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();

	int result = 0;

	// [parentGroup,]
	int nextArg = 1;

	// NOTE: GetParent() increments nextArg if parent found
	GroupObject *parent = GetParent( L, nextArg );

	// Only required param is "filename"
	// filename [, baseDirectory]
	if ( LUA_TSTRING == lua_type( L, nextArg ) )
	{
		const char *imageName = lua_tostring( L, nextArg++ );
		
		MPlatform::Directory baseDir = MPlatform::kResourceDir;
		if ( lua_islightuserdata( L, nextArg ) )
		{
			void* p = lua_touserdata( L, nextArg );
			baseDir = (MPlatform::Directory)EnumForUserdata( LuaLibSystem::Directories(),
															 p,
															 MPlatform::kNumDirs,
															 MPlatform::kResourceDir );
			++nextArg;
		}

		// w, h
		// These are required arguments, so only create a new image rect if they are present
		if ( lua_isnumber( L, nextArg ) && lua_isnumber( L, nextArg + 1 ) )
		{
			Real w = luaL_toreal( L, nextArg++ );
			Real h = luaL_toreal( L, nextArg++ );

			U32 flags = PlatformBitmap::kIsNearestAvailablePixelDensity | PlatformBitmap::kIsBitsFullResolution;

			Runtime& runtime = library->GetDisplay().GetRuntime();
			BitmapPaint *paint = BitmapPaint::NewBitmap( runtime, imageName, baseDir, flags );

			if ( paint && paint->GetBitmap() && paint->GetBitmap()->NumBytes() == 0 )
			{
				CoronaLuaWarning(L, "file '%s' does not contain a valid image", imageName);
			}
			if ( Rtt_VERIFY( paint ) )
			{
				result = NULL != PushImage( L, NULL, paint, display, parent, w, h );
			}
		}
		else
		{
			bool isWidthValid = lua_isnumber( L, nextArg );
			int narg = ( isWidthValid ? nextArg+1 : nextArg );
			const char *tname = ( isWidthValid ? "height" : "width" );

			CoronaLuaError( L, "display.newImageRect() bad argument #%d: %s expected, but got %s",
					narg, tname, lua_typename( L, lua_type( L, nextArg ) ) );
		}
	}
	else if ( lua_isuserdata( L, nextArg ) )
	{
		ImageSheetUserdata *ud = ImageSheet::ToUserdata( L, nextArg );
		if ( ud )
		{
			nextArg++;
			const AutoPtr< ImageSheet >& sheet = ud->GetSheet();

			int frameIndex = (int) lua_tointeger( L, nextArg );
			if ( frameIndex <= 0 )
			{
				CoronaLuaWarning( L, "display.newImage( imageGroup, frameIndex ) given an invalid frameIndex (%d). Defaulting to 1", frameIndex );
				frameIndex = 1;
			}

			// Map 1-based Lua indices to 0-based C indices
			--frameIndex;
			++nextArg;

			// w, h
			// These are required arguments, so only create a new image rect if they are present
			if ( lua_isnumber( L, nextArg ) && lua_isnumber( L, nextArg + 1 ) )
			{
				Real w = luaL_toreal( L, nextArg++ );
				Real h = luaL_toreal( L, nextArg++ );

				ImageSheetPaint *paint = ImageSheetPaint::NewBitmap( display.GetAllocator(), sheet, frameIndex );

				if ( Rtt_VERIFY( paint ) )
				{
					result = NULL != PushImage( L, NULL, paint, display, parent, w, h );
				}
			}
			else
			{
				bool isWidthValid = lua_isnumber( L, nextArg );
				int narg = ( isWidthValid ? nextArg+1 : nextArg );
				const char *tname = ( isWidthValid ? "height" : "width" );

				CoronaLuaError( L, "display.newImageRect() bad argument #%d: %s expected, but got %s",
						narg, tname, lua_typename( L, lua_type( L, nextArg ) ) );
			}
		}
	}
	else
	{
		CoronaLuaError( L, "display.newImageRect() bad argument #%d: filename or image sheet expected, but got %s",
				nextArg, lua_typename( L, lua_type( L, nextArg ) ) );
	}

	return result;
}

// display.newEmitter( [parentGroup,] params_table, w, h )
int
DisplayLibrary::newEmitter( lua_State *L )
{
	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();

	if ( display.ShouldRestrict( Display::kDisplayNewEmitter ) )
	{
		return 0;
	}

	int result = 0;

	EmitterObject *eo = Rtt_NEW( runtime.Allocator(), EmitterObject );
	if( eo->Initialize( L, display ) )
	{
		result = LuaLibDisplay::AssignParentAndPushResult( L, display, eo, NULL );
	}
	else
	{
		Rtt_DELETE( eo );
		luaL_error( L,
					"ERROR: invalid EmitterObject" );
	}

	return result;
}

static int CreateTextObject( lua_State *L, bool isEmbossed )
{
	int result = 0;
	
	// [parentGroup,]
	int nextArg = 1;
	
	Real x = Rtt_REAL_0;
	Real y = Rtt_REAL_0;
	
	// Default to single line
	Real w = Rtt_REAL_0;
	Real h = Rtt_REAL_0;
	
	DisplayLibrary::Self *library = DisplayLibrary::ToLibrary( L );
	Display& display = library->GetDisplay();
	Runtime& runtime = display.GetRuntime();
	
	Real fontSize = Rtt_REAL_0;		// A font size of zero means use the system default font.
	PlatformFont *font = NULL;
	
	const char *alignment = "left";
	const char* str = NULL;
	
	// NOTE: GetParent() increments nextArg if parent found
	GroupObject *parent = NULL;
	
	if ( lua_istable( L, nextArg ) &&
		LuaProxy::IsProxy(L, nextArg) == false)
	{
		if ( LUA_TTABLE == lua_type( L, -1 ) )
		{
			lua_getfield( L, -1, "parent" );
			if ( lua_istable( L, -1) )
			{
				int parentArg = Lua::Normalize( L, -1 );
				parent = GetParent( L, parentArg );
			}
			else if (lua_type( L, -1 ) != LUA_TNIL)
			{
				CoronaLuaWarning( L, "display.newText() ignoring invalid 'parent' parameter (expected table but got %s)",
								 lua_typename( L, lua_type( L, nextArg + 1 ) ) );
			}
			lua_pop( L, 1 );
			
			lua_getfield( L, -1, "text" );
			if ( (str = lua_tostring( L, -1 )) == NULL )
			{
				luaL_error( L, "ERROR: display.newText() %s 'text' parameter (expected string but got %s)",
						    (lua_type( L, nextArg + 1 ) == LUA_TNIL ? "missing" : "invalid"),
						    lua_typename( L, lua_type( L, nextArg + 1 ) ) );
			}
			lua_pop( L, 1 );

			lua_getfield( L, -1, "x" );
			if (lua_type( L, -1 ) == LUA_TNUMBER)
			{
				x = luaL_checkreal( L, -1 );
			}
			else if (lua_type( L, -1 ) != LUA_TNIL)
			{
				CoronaLuaWarning( L, "display.newText() ignoring invalid 'x' parameter (expected number but got %s)",
								  lua_typename( L, lua_type( L, nextArg + 1 ) ) );
			}
			lua_pop( L, 1 );
			
			lua_getfield( L, -1, "y" );
			if (lua_type( L, -1 ) == LUA_TNUMBER)
			{
				y = luaL_checkreal( L, -1 );
			}
			else if (lua_type( L, -1 ) != LUA_TNIL)
			{
				CoronaLuaWarning( L, "display.newText() ignoring invalid 'y' parameter (expected number but got %s)",
								 lua_typename( L, lua_type( L, nextArg + 1 ) ) );
			}
			lua_pop( L, 1 );
			
			lua_getfield( L, -1, "width" );
			if (lua_type( L, -1 ) == LUA_TNUMBER)
			{
				w = luaL_checkreal( L, -1 );
			}
			else if (lua_type( L, -1 ) != LUA_TNIL)
			{
				CoronaLuaWarning( L, "display.newText() ignoring invalid 'width' parameter (expected number but got %s)",
								 lua_typename( L, lua_type( L, nextArg + 1 ) ) );
			}
			lua_pop( L, 1 );
			
			lua_getfield( L, -1, "height" );
			if (lua_type( L, -1 ) == LUA_TNUMBER)
			{
				h = luaL_checkreal( L, -1 );
			}
			else if (lua_type( L, -1 ) != LUA_TNIL)
			{
				CoronaLuaWarning( L, "display.newText() ignoring invalid 'height' parameter (expected number but got %s)",
								 lua_typename( L, lua_type( L, nextArg + 1 ) ) );
			}
			lua_pop( L, 1 );
			
			lua_getfield( L, -1, "align" );
			if (lua_type( L, -1 ) == LUA_TSTRING)
			{
				alignment = luaL_checkstring( L, -1 );
			}
			else if (lua_type( L, -1 ) != LUA_TNIL)
			{
				CoronaLuaWarning( L, "display.newText() ignoring invalid 'align' parameter (expected string but got %s)",
								 lua_typename( L, lua_type( L, nextArg + 1 ) ) );
			}
			lua_pop( L, 1 );
			
			lua_getfield( L, -1, "fontSize" );
			if ( lua_isnumber( L, -1 ) )
			{
				fontSize = luaL_toreal( L, -1 );
			}
			else if (lua_type( L, -1 ) != LUA_TNIL)
			{
				CoronaLuaWarning( L, "display.newText() ignoring invalid 'fontSize' parameter (expected number but got %s)",
								 lua_typename( L, lua_type( L, nextArg + 1 ) ) );
			}
			lua_pop( L, 1 );
			
			lua_getfield( L, -1, "font" );
			font = LuaLibNative::CreateFont( L, runtime.Platform(), -1, fontSize);
			lua_pop( L, 1 );
		}
	}
	else
	{
		//Legacy support
		
		// NOTE: GetParent() increments nextArg if parent found
		parent = GetParent( L, nextArg );
		
		str = luaL_checkstring( L, nextArg++ );
		if ( Rtt_VERIFY( str ) )
		{
			x = luaL_checkreal( L, nextArg++ );
			y = luaL_checkreal( L, nextArg++ );
			
			if ( lua_type( L, nextArg ) == LUA_TNUMBER )
			{
				// If width is provided, height must be provided
				// Multiline
				if ( lua_type( L, nextArg + 1 ) == LUA_TNUMBER )
				{
					w = luaL_toreal( L, nextArg++ );
					h = luaL_toreal( L, nextArg++ );
				}
				else
				{
					luaL_error( L, "ERROR: display.newText() bad argument #%d (expected height to be number but got %s instead)",
                               nextArg + 1, lua_typename( L, lua_type( L, nextArg + 1 ) ) );
				}
			}
			
			// Fetch the font name/type Lua stack index.
			int fontArg = nextArg++;
			
			// Fetch the font size. Will use the default font size if not provided.
			if ( lua_isnumber( L, nextArg ) )
			{
				fontSize = luaL_toreal( L, nextArg );
			}
			
			// Create a font with the given settings.
			font = LuaLibNative::CreateFont( L, runtime.Platform(), fontArg, fontSize );
		}
	}
	
	TextObject* textObject;
	if (isEmbossed)
	{
		textObject = Rtt_NEW( runtime.Allocator(), EmbossedTextObject( display, str, font, w, h, alignment ) );
	}
	else
	{
		textObject = Rtt_NEW( runtime.Allocator(), TextObject( display, str, font, w, h, alignment ) );
	}
	result = LuaLibDisplay::AssignParentAndPushResult( L, display, textObject, parent );
	
	Real width = textObject->GetGeometricProperty( kWidth );
	Real height = textObject->GetGeometricProperty( kHeight );
	
	if ( display.GetDefaults().IsV1Compatibility() )
	{
		x += Rtt_RealDiv2( width );
		y += Rtt_RealDiv2( height );
	}
	textObject->Translate( x, y );
	
	// Set the default text color.
	if (isEmbossed && display.GetDefaults().IsV1Compatibility())
	{
		// In graphics 1.0, embossed text is always black by default.
		SharedPtr< TextureResource > resource = display.GetTextureFactory().GetDefault();
		Paint *paintPointer = Paint::NewColor(display.GetAllocator(), resource, 0, 0, 0, 255);
		textObject->SetFill( paintPointer );
	}
	else
	{
		// Setup the display object to use the default text color.
		AssignDefaultFillColor( display, * textObject );
	}
	
	return result;
}

// display.newText( [parentGroup,] string, x, y, [w, h,] font, size )
int
DisplayLibrary::newText( lua_State *L )
{
	bool isEmbossed = false;
	return CreateTextObject( L, isEmbossed );
}

// display.newEmbossedText( [parentGroup,] string, x, y, [w, h,] font, size )
int
DisplayLibrary::newEmbossedText( lua_State *L )
{
	bool isEmbossed = true;
	return CreateTextObject( L, isEmbossed );
}

// display.newGroup( [child1 [, child2 [, child3 ... ]]] )
// With no args, create an empty group and set parent to root
// 
// The following is EXPERIMENTAL and undocumented:
// When a child is passed, create a new group whose parent is the child's, 
// set the (x,y) to be the child's, insert the child into the new group, and
// reset the child's transform.  For subsequent child arguments, only
// insert into new group if each of those children has the same parent as the
// first one.
// TODO: What about other transforms (rotations, scale)?  Do these matter?
int
DisplayLibrary::newGroup( lua_State *L )
{
	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();
	Rtt_Allocator* context = display.GetAllocator();

	GroupObject *o = Rtt_NEW( context, GroupObject( context, NULL ) );
	GroupObject *parent = NULL; // Default parent is root

	DisplayObject *child = NULL;
	if ( ! lua_isnone( L, 1 ) )
	{
		child = (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

		// First child determines the parent and origin of group
		parent = child->GetParent();
	}

	// Fetch num arguments *before* pushing result

	int numArgs = lua_gettop( L );
	int result = LuaLibDisplay::AssignParentAndPushResult( L, display, o, parent );

	if ( child )
	{
		// If there are child arguments, then add them to the group "o"
		// Note that these must be done after o's parent is assigned.  
		// This ensures that o's stage is properly set before the children get
		// re-parented; otherwise the children's stage will be NULL.

		Real x = child->GetGeometricProperty( kOriginX );
		Real y = child->GetGeometricProperty( kOriginY );

		for ( int i = 1; i <= numArgs; i++ )
		{
			child = (DisplayObject*)LuaProxy::GetProxyableObject( L, i );

			if ( child && child->GetParent() == parent )
			{
				Rtt_WARN_SIM_PROXY_TYPE( L, i, DisplayObject );
				o->Insert( -1, child, false );
				child->Translate( -x, -y );
			}
			else
			{
				CoronaLuaWarning( L, "display.newGroup() argument #%d not added to group because "
								 "its parent differs from the first argument's original parent", i );
			}
		}

		o->Translate( x, y );
	}

	return result;
}

// display.newContainer( [parent, ] w, h )
int
DisplayLibrary::newContainer( lua_State *L )
{
	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();
	if ( display.GetDefaults().IsV1Compatibility() )
	{
		CoronaLuaWarning( L, "display.newContainer() is only supported in graphics 2.0. Your mileage may vary in graphicsCompatibility 1.0 mode" );
	}

	return _newContainer( L );
}

int
DisplayLibrary::_newContainer( lua_State *L )
{
	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();
	Rtt_Allocator* context = display.GetAllocator();

	// [parentGroup,]
	int nextArg = 1;

	// NOTE: GetParent() increments nextArg if parent found
	GroupObject *parent = GetParent( L, nextArg );

	Real w = luaL_checkreal( L, nextArg++ );
	Real h = luaL_checkreal( L, nextArg++ );

	ContainerObject *o = Rtt_NEW( context, ContainerObject( context, NULL, w, h ) );
	o->Initialize( display );

	int result = LuaLibDisplay::AssignParentAndPushResult( L, display, o, parent );

	return result;
}

// display.newSnapshot( [parent, ] w, h )
int
DisplayLibrary::newSnapshot( lua_State *L )
{
	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();

	if ( display.ShouldRestrict( Display::kDisplayNewSnapshot ) )
	{
		return 0;
	}

	Rtt_Allocator* context = display.GetAllocator();

	// [parentGroup,]
	int nextArg = 1;

	// NOTE: GetParent() increments nextArg if parent found
	GroupObject *parent = GetParent( L, nextArg );

	Real w = luaL_checkreal( L, nextArg++ );
	Real h = luaL_checkreal( L, nextArg++ );

	SnapshotObject *o = Rtt_NEW( context, SnapshotObject( context, display, w, h ) );

	if ( display.GetDefaults().IsV1Compatibility() )
	{
		CoronaLuaWarning( L, "display.newSnapshot() is only supported in graphics 2.0. Your mileage may vary in graphicsCompatibility 1.0 mode" );
	}

	int result = LuaLibDisplay::AssignParentAndPushResult( L, display, o, parent );
	
	o->Initialize( L, display, w, h );

	return result;
}

// display.newSprite( [parent, ] imageSheet, sequenceData )
int
DisplayLibrary::newSprite( lua_State *L )
{
	int result = 0;

	int nextArg = 1;
	GroupObject *parent = GetParent( L, nextArg );
	ImageSheetUserdata *ud = ImageSheet::ToUserdata( L, nextArg );

	if ( ud )
	{
		ImageSheetUserdata *ud = ImageSheet::ToUserdata( L, nextArg );

		nextArg++;

		if ( lua_istable( L, nextArg ) )
		{
			Self *library = ToLibrary( L );
			Display& display = library->GetDisplay();
			Rtt_Allocator *context = display.GetAllocator();

			SpritePlayer& player = display.GetSpritePlayer();
			SpriteObject *o = SpriteObject::Create( context, ud->GetSheet(), player );
			if ( o )
			{
				// Need to assign parent so we can call Initialize()
				result = LuaLibDisplay::AssignParentAndPushResult( L, display, o, parent );

				o->Initialize( context );

				// Add sequences
				int numSequences = (int) lua_objlen( L, nextArg );

				if ( 0 == numSequences )
				{
					// Table is the (single) sequence data itself
					SpriteObjectSequence *sequence = SpriteObjectSequence::Create( context, L, nextArg );
					o->AddSequence( sequence );
				}
				else
				{
					// Table is the an array of sequence data
					for ( int i = 0; i < numSequences; i++ )
					{
						lua_rawgeti( L, nextArg, i+1); // Lua is 1-based
						SpriteObjectSequence *sequence = SpriteObjectSequence::Create( context, L, -1 );
						o->AddSequence( sequence );
						lua_pop( L, 1 );
					}
				}
			}
			else
			{
				CoronaLuaError( L, "display.newSprite() failed. Returning 'nil'" );
			}
		}
		else
		{
			CoronaLuaError( L, "display.newSprite() requires argument #%d to a table containing sequence data", nextArg );
		}
	}
	else
	{
		CoronaLuaError( L, "display.newSprite() requires argument #%d to be an imageSheet", nextArg );
	}

	return result;
}

static int
PushColor( lua_State *L, Color c, bool isBytes )
{
	ColorUnion color; color.pixel = c;
	if ( isBytes )
	{
		lua_pushinteger( L, color.rgba.r );
		lua_pushinteger( L, color.rgba.g );
		lua_pushinteger( L, color.rgba.b );
		lua_pushinteger( L, color.rgba.a );
	}
	else
	{
		float kInv255 = 1.f / 255.f;
		lua_pushnumber( L, kInv255 * color.rgba.r );
		lua_pushnumber( L, kInv255 * color.rgba.g );
		lua_pushnumber( L, kInv255 * color.rgba.b );
		lua_pushnumber( L, kInv255 * color.rgba.a );
	}

	return 4;
}

int
DisplayLibrary::getDefault( lua_State *L )
{
	int result = 1;

	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();

	const char *key = lua_tostring( L, 1 );

	DisplayDefaults& defaults = display.GetDefaults();

	if ( Rtt_StringCompare( key, "anchorX" ) == 0 )
	{
		lua_pushnumber( L, defaults.GetAnchorX() );
	}
	else if ( Rtt_StringCompare( key, "anchorY" ) == 0 )
	{
		lua_pushnumber( L, defaults.GetAnchorY() );
	}
	else if ( Rtt_StringCompare( key, "fillColor" ) == 0 )
	{
		Color c = defaults.GetFillColor();
		result = PushColor( L, c, defaults.IsByteColorRange() );
	}
	else if ( Rtt_StringCompare( key, "strokeColor" ) == 0 )
	{
		Color c = defaults.GetStrokeColor();
		result = PushColor( L, c, defaults.IsByteColorRange() );
	}
	else if ( Rtt_StringCompare( key, "lineColor" ) == 0 )
	{
		Color c = defaults.GetLineColor();
		result = PushColor( L, c, defaults.IsByteColorRange() );
	}
	else if ( Rtt_StringCompare( key, "background" ) == 0 )
	{
		Color c = defaults.GetClearColor();
		result = PushColor( L, c, defaults.IsByteColorRange() );
	}
	else if ( Rtt_StringCompare( key, "magTextureFilter" ) == 0 )
	{
		RenderTypes::TextureFilter filter = defaults.GetMagTextureFilter();
		lua_pushstring( L, RenderTypes::StringForTextureFilter( filter ) );
	}
	else if ( Rtt_StringCompare( key, "minTextureFilter" ) == 0 )
	{
		RenderTypes::TextureFilter filter = defaults.GetMinTextureFilter();
		lua_pushstring( L, RenderTypes::StringForTextureFilter( filter ) );
	}
	else if ( Rtt_StringCompare( key, "textureWrapX" ) == 0 )
	{
		RenderTypes::TextureWrap wrap = defaults.GetTextureWrapX();
		lua_pushstring( L, RenderTypes::StringForTextureWrap( wrap ) );
	}
	else if ( Rtt_StringCompare( key, "textureWrapY" ) == 0 )
	{
		RenderTypes::TextureWrap wrap = defaults.GetTextureWrapY();
		lua_pushstring( L, RenderTypes::StringForTextureWrap( wrap ) );
	}
	else if ( Rtt_StringCompare( key, "emitterScaling" ) == 0 )
	{
		EmitterObject::Mapping mapping = (EmitterObject::Mapping)defaults.GetEmitterMapping();
		lua_pushstring( L, EmitterObject::GetStringForMapping( mapping ) );
	}
	else if ( Rtt_StringCompare( key, "graphicsCompatibility" ) == 0 )
	{
		int version = defaults.IsV1Compatibility() ? 1 : 2;
		lua_pushinteger( L, version );
	}
	else if ( Rtt_StringCompare( key, "isByteColorRange" ) == 0 )
	{
		bool isByteColorRange = defaults.IsByteColorRange();
		lua_pushboolean( L, isByteColorRange );
	}
	else if ( Rtt_StringCompare( key, "isNativeTextFieldFontSizeScaled" ) == 0 )
	{
		bool value = defaults.IsNativeTextFieldFontSizeScaled();
		lua_pushboolean( L, value ? 1 : 0 );
	}
	else if ( Rtt_StringCompare( key, "isNativeTextBoxFontSizeScaled" ) == 0 )
	{
		bool value = defaults.IsNativeTextBoxFontSizeScaled();
		lua_pushboolean( L, value ? 1 : 0 );
	}
	else if ( Rtt_StringCompare( key, "isShaderCompilerVerbose" ) == 0 )
	{
		bool value = defaults.IsShaderCompilerVerbose();
		lua_pushboolean( L, value ? 1 : 0 );
	}
	else if ( ( Rtt_StringCompare( key, "isAnchorClamped" ) == 0 ) )
	{
		bool value = defaults.IsAnchorClamped();
		lua_pushboolean( L, value ? 1 : 0 );
	}
	else if ( ( Rtt_StringCompare( key, "isImageSheetSampledInsideFrame" ) == 0 ) )
	{
		bool value = defaults.IsImageSheetSampledInsideFrame();
		lua_pushboolean( L, value ? 1 : 0 );
	}
	else if ( ( Rtt_StringCompare( key, "isImageSheetFrameTrimCorrected" ) == 0 ) )
	{
		bool value = defaults.IsImageSheetFrameTrimCorrected();
		lua_pushboolean( L, value ? 1 : 0 );
	}
	else if ( ( Rtt_StringCompare( key, "isExternalTextureRetina" ) == 0 ) )
	{
		bool value = defaults.IsExternalTextureRetina();
		lua_pushboolean( L, value ? 1 : 0 );
	}
	else if ( key )
	{
		luaL_error( L, "ERROR: display.getDefault() given invalid key (%s)", key );
		result = 0;
	}

	return result;
}

int
DisplayLibrary::setDefault( lua_State *L )
{
	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();

	int index = 1;
	const char *key = lua_tostring( L, index++ );

	DisplayDefaults& defaults = display.GetDefaults();
	Color c = LuaLibDisplay::toColor( L, index, defaults.IsByteColorRange() );

	if ( Rtt_StringCompare( key, "anchorX" ) == 0 )
	{
		float anchorX = lua_tonumber( L, index );
		if ( defaults.IsAnchorClamped() )
		{
			anchorX = Clamp( anchorX, 0.f, 1.f );
		}
		defaults.SetAnchorX( anchorX );
	}
	else if ( Rtt_StringCompare( key, "anchorY" ) == 0 )
	{
		float anchorY = lua_tonumber( L, index );
		if ( defaults.IsAnchorClamped() )
		{
			anchorY = Clamp( anchorY, 0.f, 1.f );
		}
		defaults.SetAnchorY( anchorY );
	}
	else if ( Rtt_StringCompare( key, "fillColor" ) == 0 )
	{
		defaults.SetFillColor( c );
	}
	else if ( Rtt_StringCompare( key, "strokeColor" ) == 0 )
	{
		defaults.SetStrokeColor( c );
	}
	else if ( Rtt_StringCompare( key, "lineColor" ) == 0 )
	{
		defaults.SetLineColor( c );
	}
//	else if ( Rtt_StringCompare( key, "textColor" ) == 0 )
//	{
//		defaults.SetTextColor( c );
//	}
	else if ( Rtt_StringCompare( key, "background" ) == 0 )
	{
		defaults.SetClearColor( c );
		display.Invalidate(); // Invalidate scene so background is updated
	}
	else if ( Rtt_StringCompare( key, "magTextureFilter" ) == 0 )
	{
		const char *value = lua_tostring( L, index );
		RenderTypes::TextureFilter filter = RenderTypes::TextureFilterForString( value );
		defaults.SetMagTextureFilter( filter );
	}
	else if ( Rtt_StringCompare( key, "minTextureFilter" ) == 0 )
	{
		const char *value = lua_tostring( L, index );
		RenderTypes::TextureFilter filter = RenderTypes::TextureFilterForString( value );
		defaults.SetMinTextureFilter( filter );
	}
	else if ( Rtt_StringCompare( key, "textureWrapX" ) == 0 )
	{
		const char *value = lua_tostring( L, index );
		RenderTypes::TextureWrap wrap = RenderTypes::TextureWrapForString( value );
		defaults.SetTextureWrapX( wrap );
	}
	else if ( Rtt_StringCompare( key, "textureWrapY" ) == 0 )
	{
		const char *value = lua_tostring( L, index );
		RenderTypes::TextureWrap wrap = RenderTypes::TextureWrapForString( value );
		defaults.SetTextureWrapY( wrap );
	}
	else if ( Rtt_StringCompare( key, "emitterMapping" ) == 0 )
	{
		const char *value = lua_tostring( L, index );
		EmitterObject::Mapping mapping = EmitterObject::GetMappingForString( value, EmitterObject::kMapping_Legacy );
		defaults.SetEmitterMapping( (U8)mapping );
	}
	else if ( Rtt_StringCompare( key, "preloadTextures" ) == 0 )
	{
		bool preloadTextures = lua_toboolean( L, index );
	
		defaults.SetPreloadTextures( preloadTextures );
	}
	else if ( Rtt_StringCompare( key, "cameraSource" ) == 0 )
	{
		VideoSource source = kCamera;
		const char *value = lua_tostring( L, index );
		if ( Rtt_StringCompare( value, "front" ) == 0 )
		{
			source = kCameraFront;
		}
		display.GetTextureFactory().SetVideoSource(source);
	}
	else if ( Rtt_StringCompare( key, "isNativeTextFieldFontSizeScaled" ) == 0 )
	{
		bool value = lua_toboolean( L, index ) ? true : false;
		defaults.SetIsNativeTextFieldFontSizeScaled( value );
	}
	else if ( Rtt_StringCompare( key, "isNativeTextBoxFontSizeScaled" ) == 0 )
	{
		bool value = lua_toboolean( L, index ) ? true : false;
		defaults.SetIsNativeTextBoxFontSizeScaled( value );
	}
	else if ( Rtt_StringCompare( key, "isShaderCompilerVerbose" ) == 0 )
	{
		bool value = lua_toboolean( L, index ) ? true : false;
		defaults.SetShaderCompilerVerbose( value );
	}
	else if ( ( Rtt_StringCompare( key, "isAnchorClamped" ) == 0 ) )
	{
		bool value = lua_toboolean( L, index ) ? true : false;
		defaults.SetAnchorClamped( value );
	}
	else if ( ( Rtt_StringCompare( key, "isImageSheetSampledInsideFrame" ) == 0 ) )
	{
		bool value = lua_toboolean( L, index ) ? true : false;
		defaults.SetImageSheetSampledInsideFrame( value );
	}
	else if ( ( Rtt_StringCompare( key, "isImageSheetFrameTrimCorrected" ) == 0 ) )
	{
		bool value = lua_toboolean( L, index ) ? true : false;
		defaults.SetImageSheetFrameTrimCorrected( value );
  }
	else if ( ( Rtt_StringCompare( key, "isExternalTextureRetina" ) == 0 ) )
	{
		bool value = lua_toboolean( L, index ) ? true : false;
		defaults.SetExternalTextureRetina( value );
	}
	else if ( key )
	{
		luaL_error( L, "ERROR: display.setDefault() given invalid key (%s)", key );
	}


	return 0;
}

int
DisplayLibrary::getCurrentStage( lua_State *L )
{
	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();

	return display.GetStage()->GetProxy()->PushTable( L );
}

/*
static void*
UserdataForStatusBarMode( MPlatform::StatusBarMode mode )
{
	return const_cast< char* >( & kStatusBarModes[mode] );
}

static MPlatform::StatusBarMode
StatusBarModeForUserdata( void* p )
{
	U32 offset = (const char*)p - kStatusBarModes;
	return offset < MPlatform::kNumModes ? (MPlatform::StatusBarMode)offset : MPlatform::kDefaultStatusBar;
}
*/

int
DisplayLibrary::collectOrphans( lua_State *L )
{
	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();

	GroupObject *orphanage = display.Orphanage(); Rtt_ASSERT( orphanage );
	GroupObject::CollectUnreachables( L, display.GetScene(), * orphanage );
	return 0;
}

int
DisplayLibrary::setStatusBar( lua_State *L )
{

	MPlatform::StatusBarMode mode = (MPlatform::StatusBarMode)EnumForUserdata(
		kStatusBarModes,
		lua_touserdata( L, 1 ),
		MPlatform::kNumModes,
		MPlatform::kDefaultStatusBar );

	if (lua_type( L, 1 ) != LUA_TLIGHTUSERDATA)
	{
		CoronaLuaWarning( L, "display.setStatusBar() parameter is a %s.  Expected a status bar mode",
						 lua_typename( L, lua_type( L, 1 ) ) );
	}
	else
	{
		const MPlatform& platform = LuaContext::GetRuntime( L )->Platform();
		platform.SetStatusBarMode( mode );
	}

#ifdef Rtt_AUTHORING_SIMULATOR
	GroupObject& overlay = LuaContext::GetRuntime( L )->GetDisplay().GetScene().Overlay();
	overlay.GetProxy()->PushTable( L ); // push self
	lua_getfield( L, -1, "setStatusBarMode" ); // push function
	lua_insert( L, -2 ); // swap self, function
	lua_pushvalue( L, 1 ); // push mode

	LuaContext::DoCall( L, 2, 0 );
#endif

	return 0;
}

int
DisplayLibrary::capture( lua_State *L )
{
	if( lua_isnil( L, 1 ) )
	{
		// Nothing to do.
		CoronaLuaWarning( L, "display.capture() first parameter was nil. Expected a display object" );
		return 0;
	}

	LuaProxy* proxy = LuaProxy::GetProxy( L, 1 );
	if( ! Rtt_VERIFY( proxy ) )
	{
		// Nothing to do.
		return 0;
	}

	DisplayObject* displayObject = (DisplayObject*)(proxy->Object());

	// Default values for options.
	bool saveToFile = false;
	bool cropObjectToScreenBounds = true;

	if( lua_isboolean( L, 2 ) )
	{
		// [,saveToPhotoLibrary]
		saveToFile = lua_toboolean( L, 2 );
	}
	else if( lua_istable( L, 2 ) )
	{
		// It's a table of options.

		lua_getfield( L, -1, "saveToPhotoLibrary" );
		if( lua_isboolean( L, -1 ) )
		{
			saveToFile = lua_toboolean( L, -1 );
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "isFullResolution" );
		if( lua_isboolean( L, -1 ) )
		{
			cropObjectToScreenBounds = ( ! lua_toboolean( L, -1 ) );
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "captureOffscreenArea" );
		if( lua_isboolean( L, -1 ) )
		{
			cropObjectToScreenBounds = ( ! lua_toboolean( L, -1 ) );
		}
		lua_pop( L, 1 );
	}

	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();
	Runtime *runtime = & display.GetRuntime();

	// Do a screenshot of the given display object.
	BitmapPaint *paint = display.CaptureDisplayObject( displayObject,
														saveToFile,
														false,
														cropObjectToScreenBounds );
    
    //Rerender and Invalidate to prevent errors on start up
    display.Invalidate();
    display.Render();
    
	if( ! paint )
	{
		CoronaLuaError(L, "display.capture() unable to capture screen. The platform or device might not be supported" );

		// Nothing to do.
		return 0;
	}

	// Add the screenshot to the photo library if set.
	if( saveToFile )
	{
		bool didAdd = runtime->Platform().AddBitmapToPhotoLibrary( paint->GetBitmap() );
		Rtt_WARN_SIM( didAdd, ( "WARNING: Simulator does not support adding screen shots to photo library\n" ) );
	}
	
	// Create a bitmap display object and have it returned to Lua.
	// Note: This screenshot will be automatically rendered on top of the display. If the user does
	//       not want to see it, then he/she should do a display.remove() the returned image object.
	Vertex2 topLeft = { Rtt_REAL_0, Rtt_REAL_0 };
	ShapeObject *v = PushImage( L, & topLeft, paint, display, NULL );
	if( ! v )
	{
		// Nothing to do.
		return 0;
	}

	// Resize the screenshot to match the size of object that it captured.
	// We have to do this because the screenshot's size is in pixels which
	// will likely be bigger than the display object itself onscreen in
	// content coordinates.
	const Rtt_Real bitmapWidth = Rtt_IntToReal(paint->GetTexture()->GetWidth());
	const Rtt_Real bitmapHeight = Rtt_IntToReal(paint->GetTexture()->GetHeight());

	Rect bounds = displayObject->StageBounds();
	if ( cropObjectToScreenBounds )
	{
		bounds.Intersect(display.GetScreenContentBounds());
	}

	// Scale.
	Rtt_Real xScale = ( (float)( bounds.xMax - bounds.xMin ) / bitmapWidth );
	Rtt_Real yScale = ( (float)( bounds.yMax - bounds.yMin ) / bitmapHeight );
	v->Scale(xScale, yScale, true);

	// Position.
	if ( display.GetDefaults().IsV1Compatibility() )
	{
		Rtt_Real xOffset = Rtt_RealDiv(Rtt_RealMul(bitmapWidth, xScale) - bitmapWidth, Rtt_REAL_2);
		Rtt_Real yOffset = Rtt_RealDiv(Rtt_RealMul(bitmapHeight, yScale) - bitmapHeight, Rtt_REAL_2);
		v->Translate(xOffset, yOffset);
	}

	return 1;
}

int
DisplayLibrary::captureScreen( lua_State *L )
{
	bool saveToFile = ( lua_isboolean( L, 1 ) &&
							lua_toboolean( L, 1 ) );

	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();
	Runtime *runtime = & display.GetRuntime();

	// Do a screenshot.
	BitmapPaint *paint = display.CaptureScreen( saveToFile,
												false );
	if( ! paint )
	{
		CoronaLuaError( L, "display.captureScreen() unable to capture screen. The platform or device might not be supported" );
		return 0;
	}

	// Add the screenshot to the photo library if set.
	if( saveToFile )
	{
		bool didAdd = runtime->Platform().AddBitmapToPhotoLibrary( paint->GetBitmap() );

		if (! didAdd)
		{
			CoronaLuaWarning( L, "display.captureScreen() unable to capture screen" );
		}
	}

	// Note: This screenshot will be automatically rendered on top of the display. If the user does
	//       not want to see it, then he/she should do a display.remove() the returned image object.
	Vertex2 topLeft = { Rtt_REAL_0, Rtt_REAL_0 };
	ShapeObject *v = PushImage( L, & topLeft, paint, display, NULL );
	if( ! v )
	{
		// Nothing to do.
		return 0;
	}

	// Resize the screenshot display object to fit the screen's bounds in content coordinates.
	// We have to do this because the screenshot's size is in pixels matching the resolution of the device's display,
	// and will likely not match the size of the content region.
	const Rtt_Real bitmapWidth = Rtt_IntToReal(paint->GetTexture()->GetWidth());
	const Rtt_Real bitmapHeight = Rtt_IntToReal(paint->GetTexture()->GetHeight());

	Rtt_Real xScale = Rtt_RealDiv(Rtt_RealMul(Rtt_IntToReal(display.ScreenWidth()), display.GetSx()), bitmapWidth);
	Rtt_Real yScale = Rtt_RealDiv(Rtt_RealMul(Rtt_IntToReal(display.ScreenHeight()), display.GetSy()), bitmapHeight);
	v->Scale(xScale, yScale, true);

	if ( display.GetDefaults().IsV1Compatibility() )
	{
		// Re-position the screenshot display object in case it has captured the areas around the content region.
		// This can happen if letterboxing is enabled or if the content size does not match the aspect ratio of the device.
		// In this case, the object should be moved in the top-left direction to be correctly centered.
		S32 xPosition = 0;
		S32 yPosition = 0;
		display.ContentToScreen(xPosition, yPosition);

		Rtt_Real xScreenOffset = Rtt_RealMul(Rtt_IntToReal(-xPosition), display.GetSx());
		Rtt_Real yScreenOffset = Rtt_RealMul(Rtt_IntToReal(-yPosition), display.GetSy());

		Rtt_Real xScaleOffset = Rtt_RealDiv(Rtt_RealMul(bitmapWidth, xScale) - bitmapWidth, Rtt_IntToReal(2));
		Rtt_Real yScaleOffset = Rtt_RealDiv(Rtt_RealMul(bitmapHeight, yScale) - bitmapHeight, Rtt_IntToReal(2));
		v->Translate(xScreenOffset + xScaleOffset, yScreenOffset + yScaleOffset);
	}

	return 1;
}

void DisplayLibrary::GetRect( lua_State *L, Rect &bounds )
{
	// Self *library = ToLibrary( L );

	// Do not continue if not given any arguments.
	if (lua_gettop(L) <= 0)
	{
		luaL_error(L, "display.captureBounds() expects a bounds table");
	}

	// Verify that the first argument is a Lua table.
	if (!lua_istable(L, 1))
	{
		luaL_error(L, "ERROR: display.captureBounds() given an invalid argument. Was expecting a bounds table but got a %s",
                   lua_typename( L, lua_type( L, 1 ) ) );
	}

	// Fetch the bounds table's xMin value.
	lua_getfield(L, 1, "xMin");
	if (lua_type(L, -1) == LUA_TNUMBER)
	{
		bounds.xMin = lua_tonumber(L, -1);
	}
	else
	{
		luaL_error(L, "ERROR: display.captureBounds() given a bounds table with an invalid or missing 'xMin' entry");
	}
	lua_pop(L, 1);
	
	// Fetch the bounds table's yMin value.
	lua_getfield(L, 1, "yMin");
	if (lua_type(L, -1) == LUA_TNUMBER)
	{
		bounds.yMin = lua_tonumber(L, -1);
	}
	else
	{
		luaL_error(L, "ERROR: display.captureBounds() given a bounds table with an invalid or missing 'yMin' entry");
	}
	lua_pop(L, 1);
	
	// Fetch the bounds table's xMax value.
	lua_getfield(L, 1, "xMax");
	if (lua_type(L, -1) == LUA_TNUMBER)
	{
		bounds.xMax = lua_tonumber(L, -1);
	}
	else
	{
		luaL_error(L, "ERROR: display.captureBounds() given a bounds table with an invalid or missing 'xMax' entry");
	}
	lua_pop(L, 1);
	
	// Fetch the bounds table's yMax value.
	lua_getfield(L, 1, "yMax");
	if (lua_type(L, -1) == LUA_TNUMBER)
	{
		bounds.yMax = lua_tonumber(L, -1);
	}
	else
	{
		luaL_error(L, "ERROR: display.captureBounds() given a bounds table with an invalid or missing 'yMax' entry");
	}
	lua_pop(L, 1);

	// If the min bounds is greater than the max bounds, then swap them.
	if (bounds.xMin > bounds.xMax)
	{
		Swap(bounds.xMin, bounds.xMax);
	}
	if (bounds.yMin > bounds.yMax)
	{
		Swap(bounds.yMin, bounds.yMax);
	}
}

int
DisplayLibrary::captureBounds( lua_State *L )
{
	Rect bounds;
	GetRect( L, bounds );

	// Capture the specified bounds of the screen.
	bool saveToFile = ( lua_isboolean( L, 2 ) &&
							lua_toboolean( L, 2 ) );

	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();
	Runtime *runtime = & display.GetRuntime();

	bounds.Intersect(display.GetScreenContentBounds());

	// Do a screenshot.
	BitmapPaint *paint = display.CaptureBounds( &bounds,
												saveToFile,
												false );
	if( ! paint )
	{
		CoronaLuaError( L, "display.CaptureBounds() unable to capture screen bounds. The platform or device might not be supported" );
		return 0;
	}
	
	// Add the screenshot to the photo library if the second argument was set.
	if( saveToFile )
	{
		bool didAdd = runtime->Platform().AddBitmapToPhotoLibrary( paint->GetBitmap() );
		Rtt_WARN_SIM( didAdd, ( "WARNING: Simulator does not support adding screen shots to photo library\n" ) );
	}
	
	// Create a display object for the screenshot and push it into Lua.
	// Note: This screenshot will be automatically rendered on top of the display. If the user does
	//       not want to see it, then he/she should do a "display:remove()" on the returned object.
	Vertex2 topLeft = { Rtt_REAL_0, Rtt_REAL_0 };
	ShapeObject *v = PushImage( L, & topLeft, paint, runtime->GetDisplay(), NULL );
	if( ! v )
	{
		Rtt_DELETE(paint);
		return 0;
	}

	// Resize the screenshot to match the size of the region that it captured in content coordinates.
	// We have to do this because the screenshot's size is in pixels which will likely be larger than
	// the captured region in content coordinates on high resolution displays.
	const Rtt_Real width = Rtt_IntToReal(paint->GetTexture()->GetWidth());
	const Rtt_Real height = Rtt_IntToReal(paint->GetTexture()->GetHeight());
	Rtt_Real xScale = Rtt_RealDiv(Rtt_IntToReal(bounds.xMax - bounds.xMin), width);
	Rtt_Real yScale = Rtt_RealDiv(Rtt_IntToReal(bounds.yMax - bounds.yMin), height);
	v->Scale(xScale, yScale, true);
// TODO: Fix halfW.halfH correction????
	Rtt_Real xOffset = Rtt_RealDiv(Rtt_RealMul(width, xScale) - width, Rtt_REAL_2);
	Rtt_Real yOffset = Rtt_RealDiv(Rtt_RealMul(height, yScale) - height, Rtt_REAL_2);
	v->Translate(xOffset, yOffset);
	
	// Return a display object showing the captured region.
	return 1;
}

int
DisplayLibrary::save( lua_State *L )
{
	if( lua_isnil( L, 1 ) )
	{
		// Nothing to do.
		CoronaLuaWarning( L, "display.save() first parameter was nil. Expected a display object" );
		return 0;
	}

	LuaProxy* proxy = LuaProxy::GetProxy( L, 1 );
	if( ! Rtt_VERIFY( proxy ) )
	{
		// Nothing to do.
		return 0;
	}

	// Default values for options.
	const char* imageName = NULL;
	MPlatform::Directory baseDir = MPlatform::kDocumentsDir;
	bool cropObjectToScreenBounds = true;
	ColorUnion backgroundColor;
	bool backgroundColorHasBeenProvided = false;
	float jpegQuality = 1.0f;

	if( lua_istable( L, 2 ) )
	{
		// It's a table of options.

		// filename is required.
		lua_getfield( L, -1, "filename" );
		imageName = luaL_checkstring( L, -1 );
		if( ! Rtt_VERIFY( imageName ) )
		{
			// Nothing to do.
			lua_pop( L, 1 );
			return 0;
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "baseDir" );
		baseDir = LuaLibSystem::ToDirectory( L, -1, MPlatform::kDocumentsDir );
		if( ! LuaLibSystem::IsWritableDirectory( baseDir ) )
		{
			// If the given directory is not writable,
			// then default to the Documents directory.
			baseDir = MPlatform::kDocumentsDir;
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "isFullResolution" );
		if( lua_isboolean( L, -1 ) )
		{
			cropObjectToScreenBounds = ( ! lua_toboolean( L, -1 ) );
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "captureOffscreenArea" );
		if( lua_isboolean( L, -1 ) )
		{
			cropObjectToScreenBounds = ( ! lua_toboolean( L, -1 ) );
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "backgroundColor" );
		backgroundColorHasBeenProvided = lua_istable( L, -1 );
		if( backgroundColorHasBeenProvided )
		{
			LuaLibDisplay::ArrayToColor( L, -1, backgroundColor.pixel, false );
		}
		lua_pop( L, 1 );
		
		lua_getfield( L, -1, "jpegQuality" );
		if( lua_isnumber( L, -1 ) )
		{
			jpegQuality = Clamp( lua_tonumber( L, -1 ), 0., 1. );
		}
		lua_pop( L, 1 );
	}
	else
	{
		// Get the options, the old way.

		// filename is required.
		imageName = luaL_checkstring( L, 2 );
		if( ! Rtt_VERIFY( imageName ) )
		{
			// Nothing to do.
			return 0;
		}

		// baseDir is optional.
		baseDir = LuaLibSystem::ToDirectory( L, 3, MPlatform::kDocumentsDir );
		if( ! LuaLibSystem::IsWritableDirectory( baseDir ) )
		{
			// If the given directory is not writable,
			// then default to the Documents directory.
			baseDir = MPlatform::kDocumentsDir;
		}
	}

	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();
	Runtime *runtime = & display.GetRuntime();

	DisplayObject* displayObject = (DisplayObject*)(proxy->Object());

	// Do a screenshot of the given display object.
	BitmapPaint *paint = display.CaptureSave( displayObject,
												cropObjectToScreenBounds,
												Rtt_StringEndsWithNoCase( imageName, ".png" ),
												( backgroundColorHasBeenProvided ? &backgroundColor : NULL ) );
	if( ! paint )
	{
		CoronaLuaError(L, "display.save() unable to capture screen. The platform or device might not be supported" );

		// Nothing to do.
		return 0;
	}

	const MPlatform& platform = runtime->Platform();
	String bitmapPath( runtime->GetAllocator() );

	platform.PathForFile( imageName, baseDir, MPlatform::kDefaultPathFlags, bitmapPath );
	platform.SaveBitmap( paint->GetBitmap(), bitmapPath.GetString(), jpegQuality );

	Rtt_DELETE( paint );

	return 0;
}

int
DisplayLibrary::colorSample( lua_State *L )
{
	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();

	if ( display.ShouldRestrict( Display::kDisplayColorSample ) )
	{
		return 0;
	}

	float pos_x = lua_tonumber( L, 1 );
	float pos_y = lua_tonumber( L, 2 );

	if( ! Lua::IsListener( L, 3, ColorSampleEvent::kName ) )
	{
		char msg[ 128 ];
		sprintf( msg,
					"ERROR: display.colorSample() requires a function, or an object able to respond to %s",
					MapLocationEvent::kName );
		luaL_argerror( L, 3, msg );
		return 0;
	}

	// Result callback.
	LuaResource *resource = Rtt_NEW( LuaContext::GetAllocator( L ),
										LuaResource( LuaContext::GetContext( L )->LuaState(),
														3 /*!< Callback index. */ ) );
	RGBA color;
	color.Clear();
    
	display.ColorSample( pos_x,
							pos_y,
							color );
    display.Render(); // rerender if no texture has been created
    
	ColorSampleEvent e( pos_x,
							pos_y,
							color );

	resource->DispatchEvent( e );

	Rtt_DELETE( resource );

	return 0;
}

static const char kForceRenderDrawName[] = "forceRender";
static const char kWireframeDrawName[] = "wireframe";
static const char kHybridDrawName[] = "hybrid";
static const char kDebugDrawName[] = "debug";

int
DisplayLibrary::setDrawMode( lua_State *L )
{
	Self *library = ToLibrary( L );
	Display& display = library->GetDisplay();

	const char *drawType = lua_tostring( L, 1 );
	
	Display::DrawMode drawMode = Display::kDefaultDrawMode;
	
	if ( Rtt_StringCompare( kForceRenderDrawName, drawType ) == 0 )
	{
		CORONA_LOG_WARNING( "[display.setDrawMode()] The 'forceRender' mode is deprecated, as it is no longer necessary to force updates to time-based shader effects." );
		drawMode = Display::kForceRenderDrawMode;
	}
	else if ( Rtt_StringCompare( kWireframeDrawName, drawType ) == 0 )
	{
		drawMode = Display::kWireframeDrawMode;
	}
	else if ( Rtt_StringCompare( kHybridDrawName, drawType ) == 0 ) 
	{
		drawMode = Display::kPhysicsHybridDrawMode;
	}
	else if ( Rtt_StringCompare( kDebugDrawName, drawType ) == 0 ) 
	{
		drawMode = Display::kPhysicsDebugDrawMode;
	}

	display.SetDrawMode( drawMode );

	return 0;
}

int
DisplayLibrary::getSafeAreaInsets( lua_State *L )
{
	Runtime& runtime = * LuaContext::GetRuntime( L );
	Display &display = runtime.GetDisplay();

	Rtt_Real top, left, bottom, right;
	runtime.Platform().GetSafeAreaInsetsPixels(top, left, bottom, right);

	lua_pushnumber( L, top*display.GetSy() );
	lua_pushnumber( L, left*display.GetSx() );
	lua_pushnumber( L, bottom*display.GetSy() );
	lua_pushnumber( L, right*display.GetSx() );
	return 4;
}

int
DisplayLibrary::enableStatistics( lua_State *L )
{
	Self* lib = (Self *)lua_touserdata( L, lua_upvalueindex( 1 ) );
	
	lib->GetDisplay().GetRenderer().SetStatisticsEnabled( lua_toboolean( L, 1 ) );
	
	return 0;
}

int
DisplayLibrary::getStatistics( lua_State *L )
{
	Self* lib = (Self *)lua_touserdata( L, lua_upvalueindex( 1 ) );
	if ( lua_istable( L, 1 ) )
	{
		Renderer::Statistics stats;
		
		if ( lib->GetDisplay().GetRenderer().GetStatisticsEnabled() )
		{
			stats = lib->GetDisplay().GetRenderer().GetFrameStatistics();
		}

		lua_pushnumber( L, stats.fResourceCreateTime );
		lua_setfield( L, 1, "resourceCreateTime" );
		lua_pushnumber( L, stats.fResourceUpdateTime );
		lua_setfield( L, 1, "resourceUpdateTime" );
		lua_pushnumber( L, stats.fResourceDestroyTime );
		lua_setfield( L, 1, "resourceDestroyTime" );
        lua_pushnumber( L, stats.fPreparationTime );
		lua_setfield( L, 1, "preparationTime" );
		lua_pushnumber( L, stats.fRenderTimeCPU );
		lua_setfield( L, 1, "renderTimeCPU" );
		lua_pushnumber( L, stats.fRenderTimeGPU );
		lua_setfield( L, 1, "renderTimeGPU" );
		lua_pushinteger( L, stats.fDrawCallCount );
		lua_setfield( L, 1, "drawCallCount" );
		lua_pushinteger( L, stats.fTriangleCount );
		lua_setfield( L, 1, "triangleCount" );
		lua_pushinteger( L, stats.fLineCount );
		lua_setfield( L, 1, "lineCount" );
		lua_pushinteger( L, stats.fGeometryBindCount );
		lua_setfield( L, 1, "geometryBindCount" );
		lua_pushinteger( L, stats.fProgramBindCount );
		lua_setfield( L, 1, "programBindCount" );
		lua_pushinteger( L, stats.fTextureBindCount );
		lua_setfield( L, 1, "textureBindCount" );
		lua_pushinteger( L, stats.fTextureBindCount );
		lua_setfield( L, 1, "textureBindCount" );
	}

	return 0;
}

int
DisplayLibrary::getSums ( lua_State* L )
{
    return Profiling::VisitSums( L );
}

int
DisplayLibrary::getTimings( lua_State *L )
{
	Self* lib = (Self *)lua_touserdata( L, lua_upvalueindex( 1 ) );
	if ( lua_isstring( L, 2 ) )
	{
		Profiling* profiling = lib->GetDisplay().GetProfilingState()->Get( lua_tostring( L, 2 ) );

		if ( NULL != profiling )
		{
			return profiling->VisitEntries( L );
		}
	}

	lua_pushinteger( L, 0 );

	return 1;
}

int
DisplayLibrary::_initProfiling( lua_State *L )
{
	Self* lib = (Self *)lua_touserdata( L, lua_upvalueindex( 1 ) );
	lua_setbookmarkf( L, ProfilingState::Bookmark, lib->GetDisplay().GetProfilingState() );

	return 0;
}

int
DisplayLibrary::_allocateProfile( lua_State *L )
{
	Self* lib = (Self *)lua_touserdata( L, lua_upvalueindex( 1 ) );
	if ( lua_isstring( L, 1 ) )
	{
		int profileID = lib->GetDisplay().GetProfilingState()->Create( lua_tostring( L, 1 ) );

		lua_pushinteger( L, profileID + 1 );
	}

	else
	{
		lua_pushnil( L );
	}

	return 1;
}

#ifdef Rtt_DEBUG

int DisplayLibrary::_getlevelcounts( lua_State *L )
{
	int ci, nbookmarks;

	lua_getlevelcounts( L, &ci, &nbookmarks );
	lua_pushinteger( L, ci - 1 ); // n.b. want caller's ci
	lua_pushinteger( L, nbookmarks );

	return 2;
}

int DisplayLibrary::_getlevelinfo( lua_State *L )
{
	int arg = 1;

	if ( !lua_isnumber( L, arg ) )
	{
		arg++;
	}

	int index = lua_tointeger( L, arg ) - 1;
	int ci, id = lua_getlevelid( L, index, &ci );

	if ( id > 0 )
	{
		lua_pushinteger( L, id );
		lua_pushinteger( L, ci );

		return 2;
	}

	else
	{
		lua_pushnil( L );

		return 1;
	}
}

int
DisplayLibrary::_getfirst( lua_State *L )
{
	Self* lib = (Self *)lua_touserdata( L, lua_upvalueindex( 1 ) );
	Profiling *first = lib->GetDisplay().GetProfilingState()->GetTopOfList();

	if ( first )
	{
		lua_pushlightuserdata( L, first );
	}

	else
	{
		lua_pushnil( L );
	}

	return 1;
}

int DisplayLibrary::_getnameandbelow( lua_State *L )
{
	Profiling* profiling = (Profiling*)lua_touserdata( L, 1 );

	if ( profiling )
	{
		lua_pushstring( L, profiling->GetName() );
		lua_pushlightuserdata( L, profiling->GetBelow() );

		return 2;
	}

	else
	{
		lua_pushnil( L );

		return 1;
	}
}

#endif

int
DisplayLibrary::_beginProfile( lua_State *L )
{
	Self* lib = (Self *)lua_touserdata( L, lua_upvalueindex( 1 ) );
	if ( lua_isnumber( L, 1 ) )
	{
		int id = lua_tointeger( L, 1 );
		Profiling* profiling = lib->GetDisplay().GetProfilingState()->Open( id );

		if ( profiling )
		{
			lua_pushlightuserdata( L, profiling );
			lua_setlevelid( L, id );

			return 1;
		}
	}

	lua_pushnil( L );

	return 1;
}

int
DisplayLibrary::_addProfileEntry( lua_State *L )
{
	Self* lib = (Self *)lua_touserdata( L, lua_upvalueindex( 1 ) );
	if ( lua_islightuserdata( L, 1 ) )
	{
		int type = lua_type( L, 2 );

		if ( LUA_TTABLE == type || LUA_TFUNCTION == type )
		{
			lib->GetDisplay().GetProfilingState()->AddEntry( lua_touserdata( L, 1 ), Profiling::Payload( lua_topointer( L, 2 ), LUA_TTABLE == type ) );
		}

		else
		{
			CoronaLuaError( L, "Profile entry expected to be function or table listener, got %s", lua_typename( L, type ) );
		}
	}

	return 0;
}

int
DisplayLibrary::_endProfile( lua_State *L )
{
	Self* lib = (Self *)lua_touserdata( L, lua_upvalueindex( 1 ) );
	if ( lua_islightuserdata( L, 1 ) )
	{
		lib->GetDisplay().GetProfilingState()->Close( lua_touserdata( L, 1 ) );

		lua_setlevelid( L, 0 );
	}

	return 0;
}

// ----------------------------------------------------------------------------

static const char kReferencePoints[DisplayObject::kNumReferencePoints + 1] = "012345678";

const char*
LuaLibDisplay::ReferencePoints()
{
	return kReferencePoints; 
}

ShapeObject*
LuaLibDisplay::PushImage(
	lua_State *L,
	Vertex2* topLeft,
	BitmapPaint* paint,
	Display& display,
	GroupObject *parent,
	Real w,
	Real h )
{
	return DisplayLibrary::PushImage( L, topLeft, paint, display, parent, w, h );
}

ShapeObject*
LuaLibDisplay::PushImage(
	lua_State *L,
	Vertex2* topLeft,
	BitmapPaint* paint,
	Display& display,
	GroupObject *parent )
{
	return DisplayLibrary::PushImage( L, topLeft, paint, display, parent );
}

void
LuaLibDisplay::Initialize( lua_State *L, Display& display )
{
	Rtt_LUA_STACK_GUARD( L );

	lua_pushlightuserdata( L, & display );
	CoronaLuaRegisterModuleLoader( L, DisplayLibrary::kName, DisplayLibrary::Open, 1 );

	CoronaLuaPushModule( L, DisplayLibrary::kName );
	lua_setglobal( L, DisplayLibrary::kName ); // display = library
}

// NOTE: All transformations should be applied AFTER this call.
// The act of adding an object to a group (i.e. assigning the parent) implicitly
// nukes the child's transform.
int
LuaLibDisplay::AssignParentAndPushResult( lua_State *L, Display& display, DisplayObject* o, GroupObject *pParent )
{
	if ( ! pParent )
	{
		pParent = display.GetStage();
	}
	pParent->Insert( -1, o, false );

	o->AddedToParent( L, pParent );

	o->InitProxy( L );

	// NOTE: V1 compatibility on an object can only be set on creation.
	const DisplayDefaults& defaults = display.GetDefaults();
	bool isV1Compatibility = defaults.IsV1Compatibility();
	o->SetV1Compatibility( isV1Compatibility );

	if ( ! isV1Compatibility )
	{
		o->SetAnchorX( defaults.GetAnchorX() );
		o->SetAnchorY( defaults.GetAnchorY() );
	}
	
	// NOTE: This restriction should only be set on creation.
	// This is a performance optimization to make the initial restriction
	// check fast. Any subsequent checking can be more expensive.
	o->SetRestricted( display.IsRestricted() );

	LuaProxy* proxy = o->GetProxy(); Rtt_ASSERT( proxy );
	return proxy->PushTable( L );
}

Color
LuaLibDisplay::toColorFloat( lua_State *L, int index )
{
	int numArgs = lua_gettop( L ) - index + 1; // add 1 b/c index is 1-based

	ColorUnion c;

	if ( numArgs >= 3 )
	{
		float r = Clamp( lua_tonumber( L, index++ ), 0., 1. );
		float g = Clamp( lua_tonumber( L, index++ ), 0., 1. );
		float b = Clamp( lua_tonumber( L, index++ ), 0., 1. );
		float a = Clamp( (lua_isnone( L, index ) ? 1.0 : lua_tonumber( L, index )), 0., 1. );

		RGBA rgba = { U8(r*255.0f), U8(g*255.0f), U8(b*255.0f), U8(a*255.0f) };
		c.rgba = rgba;
	}
	else
	{
		// Treat first value as a grayscale param
		float value = Clamp( lua_tonumber( L, index++ ), 0., 1. );
		float a = Clamp( (lua_isnone( L, index ) ? 1.0 : lua_tonumber( L, index )), 0., 1. );
		RGBA rgba = { U8(value*255.0f), U8(value*255.0f), U8(value*255.0f), U8(a*255.0f) };
		c.rgba = rgba;
	}

	return c.pixel;
}

Color
LuaLibDisplay::toColorByte( lua_State *L, int index )
{
	int numArgs = lua_gettop( L ) - index + 1; // add 1 b/c index is 1-based

	ColorUnion c;

	if ( numArgs >= 3 )
	{
		int r = (int) lua_tointeger( L, index++ );
		int g = (int) lua_tointeger( L, index++ );
		int b = (int) lua_tointeger( L, index++ );
		int a = (int) ( lua_isnone( L, index ) ? 255 : lua_tointeger( L, index ) );
		RGBA rgba = { (U8)r, (U8)g, (U8)b, (U8)a };
		c.rgba = rgba;
	}
	else
	{
		// Treat first value as a grayscale param
		int value = (int) lua_tointeger( L, index++ );
		int a = (int) ( lua_isnone( L, index ) ? 255 : lua_tointeger( L, index ) );
		RGBA rgba = { (U8)value, (U8)value, (U8)value, (U8)a };
		c.rgba = rgba;
	}

	return c.pixel;
}

int
LuaLibDisplay::PushColorChannels( lua_State *L, Color c, bool isBytes )
{
	return PushColor( L, c, isBytes );
}

Color
LuaLibDisplay::toColor( lua_State *L, int index, bool isBytes )
{
	return ( isBytes ? toColorByte( L, index ) : toColorFloat( L, index ) );
}

Paint*
LuaLibDisplay::LuaNewColor( lua_State *L, int index, bool isBytes )
{
	Color c = Self::toColor( L, index, isBytes );
	SharedPtr< TextureResource > resource = LuaContext::GetRuntime( L )->GetDisplay().GetTextureFactory().GetDefault();
	Paint* p = Paint::NewColor( LuaContext::GetRuntime( L )->Allocator(), resource, c );
	return p;
}

// { type="image", baseDir=, filename= }
static BitmapPaint *
NewBitmapPaintFromFile( lua_State *L, int paramsIndex )
{
	BitmapPaint *paint = NULL;

	lua_getfield( L, paramsIndex, "filename" );
	const char *imageName = lua_tostring( L, -1 );
	if ( imageName )
	{
		lua_getfield( L, paramsIndex, "baseDir" );
		MPlatform::Directory baseDir =
			LuaLibSystem::ToDirectory( L, -1 );
		lua_pop( L, 1 );

		Runtime *runtime = LuaContext::GetRuntime( L );
		U32 flags = PlatformBitmap::kIsNearestAvailablePixelDensity;
		paint = BitmapPaint::NewBitmap( *runtime, imageName, baseDir, flags );
		if ( paint && paint->GetBitmap() && paint->GetBitmap()->NumBytes() == 0 )
		{
			CoronaLuaWarning(L, "file '%s' does not contain a valid image", imageName);
		}
	}
	lua_pop( L, 1 );

	return paint;
}

// { type="image", sheet=, frame= }
static BitmapPaint *
NewBitmapPaintFromSheet( lua_State *L, int paramsIndex )
{
	BitmapPaint *result = NULL;

	lua_getfield( L, paramsIndex, "sheet" );
	if ( lua_isuserdata( L, -1 ) )
	{
		ImageSheetUserdata *ud = ImageSheet::ToUserdata( L, -1 );
		if ( ud )
		{
			const AutoPtr< ImageSheet >& sheet = ud->GetSheet();

			lua_getfield( L, paramsIndex, "frame" );
			int frameIndex = (int) lua_tointeger( L, -1 );
			lua_pop( L, 1 );

			if ( frameIndex <= 0 )
			{
				CoronaLuaWarning( L, "image paint given an invalid frameIndex (%d). Defaulting to 1", frameIndex );
				frameIndex = 1;
			}

			// Map 1-based Lua indices to 0-based C indices
			--frameIndex;

			ImageSheetPaint *paint = ImageSheetPaint::NewBitmap(
				LuaContext::GetAllocator( L ), sheet, frameIndex );

			result = paint;
		}
	}
	lua_pop( L, 1 );

	return result;
}

BitmapPaint *
LuaLibDisplay::LuaNewBitmapPaint( lua_State *L, int paramsIndex )
{
	BitmapPaint *result = NULL;

	if ( ! result )
	{
		result = NewBitmapPaintFromFile( L, paramsIndex );
	}

	if ( ! result )
	{
		result = NewBitmapPaintFromSheet( L, paramsIndex );
	}

	return result;
}

void
LuaLibDisplay::ArrayToColor( lua_State *L, int index, Color& outColor, bool isBytes )
{
	int top = lua_gettop( L );

	index = Lua::Normalize( L, index );

	int numArgs = Min( (int)lua_objlen( L, index ), 4 );
	for ( int i = 0; i < numArgs; i++ )
	{
		lua_rawgeti( L, index, (i+1) ); // 1-based
	}

	if ( numArgs > 0 )
	{
		outColor = LuaLibDisplay::toColor( L, top+1, isBytes );
	}

	lua_pop( L, numArgs );
}

// { type="gradient", color1={r,g,b,a}, color2={r,g,b,a}, direction= }
GradientPaint *
LuaLibDisplay::LuaNewGradientPaint( lua_State *L, int paramsIndex )
{
	GradientPaint *result = NULL;

	const RGBA kDefault = { 0, 0, 0, 255 };
	ColorUnion color1, color2;
	color1.rgba = kDefault;
	color2.rgba = kDefault;

	// We only support byte channels in old graphics (v1)-related functions
	// TODO: Remove this once we remove graphics.newGradient()
	lua_getfield( L, paramsIndex, "graphicsCompatibility" );
	const bool isBytes = ( 1 == lua_tointeger( L, -1 ) );
	lua_pop( L, 1 );

	lua_getfield( L, paramsIndex, "color1" );
	if ( lua_istable( L, -1 ) )
	{
		ArrayToColor( L, -1, color1.pixel, isBytes );
	}
	lua_pop( L, 1 );

	lua_getfield( L, paramsIndex, "color2" );
	if ( lua_istable( L, -1 ) )
	{
		ArrayToColor( L, -1, color2.pixel, isBytes );
	}
	lua_pop( L, 1 );

	GradientPaint::Direction direction = GradientPaint::kDefaultDirection;
	Rtt_Real angle = Rtt_REAL_0;
	lua_getfield( L, paramsIndex, "direction" );
	if ( lua_type( L, -1 ) == LUA_TSTRING )
	{
		direction = GradientPaint::StringToDirection( lua_tostring( L, -1 ) );
	}
	else if ( lua_type( L, -1 ) == LUA_TNUMBER )
	{
		angle = Rtt_FloatToReal(lua_tonumber(L, -1));
	}
	lua_pop( L, 1 );

	Runtime *runtime = LuaContext::GetRuntime( L );
	Display& display = runtime->GetDisplay();
	result = GradientPaint::New( display.GetTextureFactory(), color1.pixel, color2.pixel, direction, angle );

	return result;
}

CompositePaint *
LuaLibDisplay::LuaNewCompositePaint( lua_State *L, int paramsIndex )
{
	CompositePaint *result = NULL;
	Paint *paint0 = NULL;
	Paint *paint1 = NULL;

	lua_getfield( L, paramsIndex, "paint1" );
	if ( lua_istable( L, -1 ) )
	{
		paint0 = LuaNewPaint( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, paramsIndex, "paint2" );
	if ( lua_istable( L, -1 ) )
	{
		paint1 = LuaNewPaint( L, -1 );
	}
	lua_pop( L, 1 );

	if ( paint0 && paint1 )
	{
		result = Rtt_NEW( LuaContext::GetAllocator( L ), CompositePaint( paint0, paint1 ) );
	}

	return result;
}

CameraPaint *
LuaLibDisplay::LuaNewCameraPaint( lua_State *L, int paramsIndex )
{
	Runtime *runtime = LuaContext::GetRuntime( L );
	Display& display = runtime->GetDisplay();
	
	SharedPtr< TextureResource > resource = display.GetTextureFactory().GetVideo();
	
	CameraPaint *result = Rtt_NEW( LuaContext::GetAllocator( L ), CameraPaint( resource ) );

	return result;
}

// object.fill = { r, g, b, a }
// object.fill = { type="image", baseDir=, filename= }
// object.fill = { type="image", sheet=, frame= }
// object.fill = { type="gradient", color1={r,g,b,a}, color2={r,g,b,a} }
// object.fill = { type="composite", paint1={}, paint2={} }
// object.fill = { type="camera" }
// TODO: object.fill = other.fill
Paint *
LuaLibDisplay::LuaNewPaint( lua_State *L, int index )
{
	Paint *result = NULL;

	index = Lua::Normalize( L, index );

	// We only support byte channels in old graphics (v1)-related functions
	const bool isBytes = false;

	if ( lua_istable( L, index ) )
	{
		lua_getfield( L, index, "type" );
		const char *paintType = lua_tostring( L, -1 );
		if ( paintType )
		{
			if ( 0 == strcmp( "image", paintType ) )
			{
				result = LuaNewBitmapPaint( L, index );
			}
			else if ( 0 == strcmp( "gradient", paintType ) )
			{
				result = LuaNewGradientPaint( L, index );
			}
			else if ( 0 == strcmp( "composite", paintType ) )
			{
				result = LuaNewCompositePaint( L, index );
			}
			else if ( 0 == strcmp( "camera", paintType ) )
			{
				result = LuaNewCameraPaint( L, index );
			}
		}
		else
		{
			// Assume it's a color array
			Color c;
			ArrayToColor( L, index, c, isBytes );
			
			SharedPtr< TextureResource > resource = LuaContext::GetRuntime( L )->GetDisplay().GetTextureFactory().GetDefault();
			result = Paint::NewColor( LuaContext::GetRuntime( L )->Allocator(), resource, c );
		}
		lua_pop( L, 1 );
	}
	else if ( lua_type( L, index ) == LUA_TNUMBER )
	{
		result = LuaLibDisplay::LuaNewColor( L, index, isBytes );
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

