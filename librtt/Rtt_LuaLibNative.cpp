//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_LuaLibNative.h"

#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayDefaults.h"
#include "Display/Rtt_LuaLibDisplay.h"
#include "Rtt_Event.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaResource.h"
#include "Rtt_MCallback.h"
#include "Rtt_MPlatform.h"
#include "Rtt_PlatformDisplayObject.h"
#include "Rtt_PlatformWebPopup.h"
#include "Rtt_Runtime.h"

//#include <string.h>
#ifdef Rtt_WIN_ENV
#undef CreateFont
#endif

#include "CoronaLua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static const char kNativeAlert[] = "native.Alert";

// ----------------------------------------------------------------------------

static const char kNativeFont[] = "native.Font";

static int
gcFontWrapper( lua_State *L )
{
	PlatformFont *p = LuaLibNative::ToFont( L, 1 );
	Rtt_DELETE( p );

	return 0;
}

// ----------------------------------------------------------------------------

// native.showAlert( title, message [, { buttonLabels } [, listener] ] )
// returns an alertId
static int
showAlert( lua_State *L )
{
	const char *title = lua_tostring( L, 1 );
	const char *msg = lua_tostring( L, 2 );
	if ( title && msg )
	{
		const size_t kMaxNumButtons = 5;
		const char *buttons[kMaxNumButtons];
		int numButtons = 0;

		int callbackIndex = 0;

		if ( lua_istable( L, 3 ) )
		{
			const int t = 3; // table index
			int i = 0;
			for ( bool isValidArg = true; isValidArg && i < (int)kMaxNumButtons; i++ )
			{
				lua_rawgeti( L, t, i+1 );
				isValidArg = lua_isstring( L, -1 );
				if ( isValidArg )
				{
					buttons[i] = lua_tostring( L, -1 );
					++numButtons;
				}
				lua_pop( L, 1 );
			}

//			Rtt_WARN_SIM(
//				( "WARNING: native.showAlert() is limited to %d or fewer buttons. The other buttons are ignored.\n", kMaxNumButtons ) );
			
			if ( Lua::IsListener( L, 4, CompletionEvent::kName ) )
			{
				callbackIndex = 4;
			}
            else if (lua_type(L, 4) != LUA_TNONE)
            {
                CoronaLuaError(L, "native.showAlert() expects a listener as argument #4 (got %s)", lua_typename( L, lua_type(L, 4)));
            }
		}

		const MPlatform& platform = LuaContext::GetPlatform( L );
		LuaResource *resource = NULL;
		if ( callbackIndex > 0 )
		{
			resource = Rtt_NEW( & platform.GetAllocator(), LuaResource( LuaContext::GetContext( L )->LuaState(), callbackIndex ) );
		}

		NativeAlertRef *p = (NativeAlertRef*)lua_newuserdata( L, sizeof( NativeAlertRef ) );
		*p = platform.ShowNativeAlert( title, msg, buttons, numButtons, resource );

		luaL_getmetatable( L, kNativeAlert );
		lua_setmetatable( L, -2 );
	}
	else
	{
        CoronaLuaError(L, "native.showAlert() called with unexpected parameters");

		lua_pushnil( L );
	}

	return 1;
}

// native.cancelAlert( alertId )
static int
cancelAlert( lua_State *L )
{
	NativeAlertRef *p = (NativeAlertRef*)luaL_checkudata( L, 1, kNativeAlert );
	if ( p )
	{
		NativeAlertRef alert = *p;
		if ( alert )
		{
			* p = NULL; // NULL-ify Lua reference to callback

			const MPlatform& platform = LuaContext::GetPlatform( L );

			int index = (int) (lua_isnumber( L, 2 ) ? lua_tointeger( L, 2 ) : 0);
			platform.CancelNativeAlert( alert, index );
		}
	}

	return 0;
}

// native.setActivityIndicator( visible )
static int
setActivityIndicator( lua_State *L )
{
	const MPlatform& platform = LuaContext::GetPlatform( L );

	platform.SetActivityIndicator( lua_toboolean( L, 1 ) );

	return 0;
}

// native.showWebPopup( url [, options] )
// options is a table: { listener=,  }
// TODO: add following: { title=, postParams=, width=, height= }
static int
showWebPopup( lua_State *L )
{
	int curArg = 1;

	Rect rect;
	if ( lua_isnumber( L, curArg )
		 && lua_isnumber( L, curArg+1 )
		 && lua_isnumber( L, curArg+2 )
		 && lua_isnumber( L, curArg+3 ) )
	{
		rect.xMin = luaL_toreal( L, curArg++ );
		rect.yMin = luaL_toreal( L, curArg++ );
		Rtt_Real w = luaL_toreal( L, curArg++ );
		Rtt_Real h = luaL_toreal( L, curArg++ );
		if ( w > 0 && h > 0 )
		{
			rect.xMax = rect.xMin + w;
			rect.yMax = rect.yMin + h;
		}
		else
		{
			CoronaLuaError(L, "Bad parameters to native.showWebPopup() width (%g) and height (%g) must be positive", w, h);
		}
	}
	else
	{
		// rect is invalid. Set it to the contentWidth and contentHeight.
		Runtime *runtime = LuaContext::GetRuntime( L );
		rect.xMin = Rtt_REAL_0;
		rect.yMin = Rtt_REAL_0;
		rect.xMax = runtime->GetDisplay().ContentWidth();
		rect.yMax = runtime->GetDisplay().ContentHeight();
	}

	const char *url = lua_isstring( L, curArg ) ? lua_tostring( L, curArg++ ) : NULL;

	if ( url )
	{
		Runtime *runtime = LuaContext::GetRuntime( L );
		const MPlatform& platform = runtime->Platform();
		PlatformWebPopup *popup = platform.GetWebPopup();

		if (popup != NULL && popup->GetInCallback())
		{
			CORONA_LUA_LOG_ERROR(L, "Cannot call native.showWebPopup() from another native.showWebPopup()");

			return 0;
		}

//		popup->Reset();

		if ( popup == NULL )
		{
			return 0;
		}

		if ( lua_istable( L, curArg ) )
		{
			lua_getfield( L, curArg, UrlRequestEvent::kName );
			if ( Lua::IsListener( L, -1, UrlRequestEvent::kName ) )
			{
				LuaResource *resource = Rtt_NEW(
					& platform.GetAllocator(), 
					LuaResource( runtime->VMContext().LuaState(), -1 ) );
				popup->SetCallback( resource );
			}
			else
			{
				popup->SetCallback( NULL );
			}
			lua_pop( L, 1 );

			const char kHasBackgroundKey[] = "hasBackground";
			lua_getfield( L, curArg, kHasBackgroundKey );
			if ( lua_isboolean( L, -1 ) )
			{
				popup->SetValueForKey( L, kHasBackgroundKey, lua_gettop( L ) );
			}
			lua_pop( L, 1 );

			const char kBaseUrlKey[] = "baseUrl";
			lua_getfield( L, curArg, kBaseUrlKey );
//			if ( lua_isstring( L, -1 ) ) // apparently we need to call SetValueForKey anyway.
			{
				popup->SetValueForKey( L, kBaseUrlKey, lua_gettop( L ) );
			}
			lua_pop( L, 1 );
			
			const char kAutoCancelKey[] = "autoCancel";
			lua_getfield( L, curArg, kAutoCancelKey );
			if ( lua_isboolean( L, -1 ) )
			{
				popup->SetValueForKey( L, kAutoCancelKey, lua_gettop( L ) );
			}
			lua_pop( L, 1 );
		}

		popup->Preinitialize( runtime->GetDisplay() );
		popup->SetStageBounds( rect, runtime );
		popup->Show( platform, url );
	}
	else
	{
		CoronaLuaError(L, "Bad parameter to native.showWebPopup() string expected for url parameter");
	}

	return 0;
}

// native.cancelWebPopup()
static int
cancelWebPopup( lua_State *L )
{
	const MPlatform& platform = LuaContext::GetRuntime( L )->Platform();
	PlatformWebPopup *popup = platform.GetWebPopup();

	if (popup != NULL && popup->GetInCallback())
	{
		CORONA_LUA_LOG_ERROR(L, "Cannot call native.cancelWebPopup() from native.showWebPopup() listener");

		return 0;
	}

	lua_pushboolean( L, popup->Close() );

	return 1;
}

// local t = native.newTextField( x, y, w, h )
// 
// t.text
// t.font
// function listener( event )
//   event.phase = "began" (always called), "ended" (always called), "submitted" (only if return key pressed)
//   event.name = "userInput"
// end
// 
// native.setKeyboardFocus( tf )
// if tf is nil, then dismiss keyboard
static int
newTextField( lua_State *L )
{
	int result = 0;

	Runtime& runtime = * LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime.Platform();

	Real x = luaL_toreal( L, 1 );
	Real y = luaL_toreal( L, 2 );
	Real w = luaL_toreal( L, 3 );
	Real h = luaL_toreal( L, 4 );

	if ( w > 0 && h > 0 )
	{
		Rect bounds;
		Display& display = runtime.GetDisplay();
		if ( display.GetDefaults().IsV1Compatibility() )
		{
			bounds.xMin = x;
			bounds.yMin = y;
			bounds.xMax = x + w;
			bounds.yMax = y + h;
		}
		else
		{
			bounds.Initialize( x, y, w, h );
		}

		PlatformDisplayObject *t = platform.CreateNativeTextField( bounds );

		if ( t )
		{
			t->Preinitialize( display );
			t->SetHandle( & platform.GetAllocator(), runtime.VMContext().LuaState() );

			result = LuaLibDisplay::AssignParentAndPushResult( L, display, t, NULL );

			if ( Lua::IsListener( L, 5, PlatformDisplayObject::kUserInputEvent ) )
			{
				CoronaLuaWarning( L, "The 'listener' argument to native.newTextField( left, top, width, height [, listener] ) is deprecated. Call the object method o:addEventListener( '%s', listener ) instead",
					PlatformDisplayObject::kUserInputEvent );
				t->AddEventListener( L, 5, PlatformDisplayObject::kUserInputEvent );
			}

			t->Initialize();
		}
	}

	return result;
}

// local t = native.newTextBox( x, y, w, h )
// 
// t.text
// t.font
// function listener( event )
//   event.phase = "began" (always called), "ended" (always called)
//   event.name = "userInput"
// end
// 
// native.setKeyboardFocus( tf )
// if tf is nil, then dismiss keyboard
static int
newTextBox( lua_State *L )
{
	int result = 0;

	Runtime& runtime = * LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime.Platform();

	Real x = luaL_toreal( L, 1 );
	Real y = luaL_toreal( L, 2 );
	Real w = luaL_toreal( L, 3 );
	Real h = luaL_toreal( L, 4 );

	if ( w > 0 && h > 0 )
	{
		Rect bounds;
		Display& display = runtime.GetDisplay();
		if ( display.GetDefaults().IsV1Compatibility() )
		{
			bounds.xMin = x;
			bounds.yMin = y;
			bounds.xMax = x + w;
			bounds.yMax = y + h;
		}
		else
		{
			bounds.Initialize( x, y, w, h );
		}

		PlatformDisplayObject *t = platform.CreateNativeTextBox( bounds );

		if ( t )
		{
			t->Preinitialize( display );
			t->SetHandle( & platform.GetAllocator(), runtime.VMContext().LuaState() );

			result = LuaLibDisplay::AssignParentAndPushResult( L, display, t, NULL );

			if ( Lua::IsListener( L, 5, PlatformDisplayObject::kUserInputEvent ) )
			{
				CoronaLuaWarning( L, "The 'listener' argument to native.newTextBox( left, top, width, height [, listener] ) is deprecated. Call the object method o:addEventListener( '%s', listener ) instead",
					PlatformDisplayObject::kUserInputEvent );
				t->AddEventListener( L, 5, PlatformDisplayObject::kUserInputEvent );
			}

			t->Initialize();
		}
	}
	else
	{
		// The inverted comparisons avoid testing a float for equality with zero
		luaL_error( L, "native.newTextBox() requires %s greater than zero",
				   (w > 0.f ? "height" : (h > 0.f ? "width" : "width and height")) );
	}

	return result;
}

// native.newMapView( left, top, width, height )
static int
newMapView( lua_State *L )
{
	int result = 0;

	Runtime& runtime = * LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime.Platform();

	Real x = luaL_toreal( L, 1 );
	Real y = luaL_toreal( L, 2 );
	Real w = luaL_toreal( L, 3 );
	Real h = luaL_toreal( L, 4 );

	if ( w > 0 && h > 0 )
	{
		Rect bounds;
		Display& display = runtime.GetDisplay();
		if ( display.GetDefaults().IsV1Compatibility() )
		{
			bounds.xMin = x;
			bounds.yMin = y;
			bounds.xMax = x + w;
			bounds.yMax = y + h;
		}
		else
		{
			bounds.Initialize( x, y, w, h );
		}

		PlatformDisplayObject *t = platform.CreateNativeMapView( bounds );

		if ( t )
		{
			t->Preinitialize( display );
			t->SetHandle( & platform.GetAllocator(), runtime.VMContext().LuaState() );

			result = LuaLibDisplay::AssignParentAndPushResult( L, display, t, NULL );

			if ( Lua::IsListener( L, 5, PlatformDisplayObject::kUserInputEvent ) )
			{
				CoronaLuaWarning( L, "The 'listener' argument to native.newMapView( left, top, width, height [, listener] ) is deprecated. Call the object method o:addEventListener( '%s', listener ) instead",
					PlatformDisplayObject::kUserInputEvent );
				t->AddEventListener( L, 5, PlatformDisplayObject::kUserInputEvent );
			}

			t->Initialize();
		}
	}

	return result;
}

// native.newWebView( left, top, width, height [,listener] )
static int
newWebView( lua_State *L )
{
	int luaArgumentCount = lua_gettop( L );
	int result = 0;

	Runtime& runtime = * LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime.Platform();

	Real x = luaL_toreal( L, 1 );
	Real y = luaL_toreal( L, 2 );
	Real w = luaL_toreal( L, 3 );
	Real h = luaL_toreal( L, 4 );

	if ( w > 0 && h > 0 )
	{
		Rect bounds;
		Display& display = runtime.GetDisplay();
		if ( display.GetDefaults().IsV1Compatibility() )
		{
			bounds.xMin = x;
			bounds.yMin = y;
			bounds.xMax = x + w;
			bounds.yMax = y + h;
		}
		else
		{
			bounds.Initialize( x, y, w, h );
		}

		PlatformDisplayObject *t = platform.CreateNativeWebView( bounds );

		if ( t )
		{
			t->Preinitialize( display );
			t->SetHandle( & platform.GetAllocator(), runtime.VMContext().LuaState() );

			result = LuaLibDisplay::AssignParentAndPushResult( L, display, t, NULL );

			if ( ( luaArgumentCount >= 5 ) && Lua::IsListener( L, 5, UrlRequestEvent::kName ) )
			{
				CoronaLuaWarning( L, "The 'listener' argument to native.newWebView( left, top, width, height [, listener] ) is deprecated. Call the object method o:addEventListener( '%s', listener ) instead",
					UrlRequestEvent::kName );
				t->AddEventListener( L, 5, UrlRequestEvent::kName );
			}

			t->Initialize();
		}
	}

	return result;
}

// native.newVideo( left, top, width, height )
// IMPORTANT NOTE: native.newVideo is overridden in init.lua to do extra things to handle application suspend/resume correctly.
// The overridden init.lua version calls into this version and then does additional things like saving the video handle in a weak list.
static int
newVideo( lua_State *L )
{
	int result = 0;

	Runtime& runtime = * LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime.Platform();

	Real x = luaL_toreal( L, 1 );
	Real y = luaL_toreal( L, 2 );
	Real w = luaL_toreal( L, 3 );
	Real h = luaL_toreal( L, 4 );

	if ( w > 0 && h > 0 )
	{
		Rect bounds;
		Display& display = runtime.GetDisplay();
		if ( display.GetDefaults().IsV1Compatibility() )
		{
			bounds.xMin = x;
			bounds.yMin = y;
			bounds.xMax = x + w;
			bounds.yMax = y + h;
		}
		else
		{
			bounds.Initialize( x, y, w, h );
		}

		PlatformDisplayObject *t = platform.CreateNativeVideo( bounds );

		if ( t )
		{
			t->Preinitialize( display );
			t->SetHandle( & platform.GetAllocator(), runtime.VMContext().LuaState() );

			result = LuaLibDisplay::AssignParentAndPushResult( L, display, t, NULL );
			t->Initialize();
		}
	}

	return result;
}

// native.requestExit()
static int
requestExitApplication( lua_State *L )
{
	const MPlatform& platform = LuaContext::GetPlatform( L );
	platform.RequestSystem( L, "exitApplication", 0 );
	return 0;
}

// native.requestExitActivity()
static int
requestExitActivity( lua_State *L )
{
	const MPlatform& platform = LuaContext::GetPlatform( L );
	platform.RequestSystem( L, "exitActivity", 0 );
	return 0;
}

static const char kFonts[PlatformFont::kNumSystemFonts + 1] = "01";

PlatformFont*
LuaLibNative::CreateFont( lua_State *L, const MPlatform& platform, int index, Real fontSize )
{
	PlatformFont *font = NULL;
	
	// If indexing a native font object, then clone it.
	if ( LuaLibNative::IsFont( L, index ) )
	{
		font = LuaLibNative::ToFont( L, index );
		if ( font )
		{
			// Clone the given font.
			font = font->CloneUsing( &platform.GetAllocator() );
			
			// Only change the cloned font's size if a size was provided.
			// Note: A font size of zero indicates that a size was not provided in Lua.
			if ( fontSize >= Rtt_REAL_1 )
			{
				font->SetSize( fontSize );
			}
			
			// Return the cloned font.
			return font;
		}
	}
	
	// An existing font object was not provided. So, we must create a new one.
	if ( !font )
	{
		// If the given font size is invalid, then use the system's default font size.
		// Note: A font size of zero indicates that a size was not provided in Lua.
		if ( fontSize < Rtt_REAL_1 )
		{
			fontSize = platform.GetStandardFontSize() * LuaContext::GetRuntime( L )->GetDisplay().GetSxUpright();
		}
		
		// Create a new font object.
		if ( lua_isstring( L, index ) )
		{
			// We were given a font name. Attempt to load/create it.
			const char *fontName = lua_tostring( L, index );
			font = platform.CreateFont( fontName, fontSize );
		}
		if ( !font )
		{
			// Assume we were given a font enum type and create a system default font object.
			PlatformFont::SystemFont fontType = PlatformFont::kSystemFont;
			if ( lua_islightuserdata( L, index ) )
			{
				fontType = (PlatformFont::SystemFont)EnumForUserdata(
									kFonts,
									lua_touserdata( L, index ),
									PlatformFont::kNumSystemFonts,
									PlatformFont::kSystemFont );
			}
			font = platform.CreateFont( fontType, fontSize );
		}
	}
	
	// Return the newly created font.
	return font;
}

// native.newFont( name, size )
static int
newFont( lua_State *L )
{
	const MPlatform& platform = LuaContext::GetPlatform( L );

	Rtt_Real fontSize = Rtt_REAL_0;
	if ( lua_isnumber( L, 2 ) )
	{
		fontSize = lua_tonumber( L, 2 );
	}

	PlatformFont *font = LuaLibNative::CreateFont( L, platform, 1, fontSize );
	return LuaLibNative::PushFont( L, font );
}

static int
getFontNames( lua_State *L )
{
	int result = 1;

	const MPlatform& platform = LuaContext::GetPlatform( L );

	// Return an array of strings
	lua_newtable( L );
	int index = lua_gettop( L );

	S32 numFonts = platform.GetFontNames( L, index );

	Rtt_ASSERT( lua_gettop( L ) == index );
	Rtt_ASSERT( numFonts >= 0 && lua_objlen( L, index ) == (size_t)numFonts );

	if ( numFonts <= 0 )
	{
		CoronaLuaWarning(L, "No system fonts found" );
	}

	return result;
}

static int
setKeyboardFocus( lua_State *L )
{
	PlatformDisplayObject *o = NULL;

	if ( ! lua_isnil( L, 1 ) )
	{
		o = (PlatformDisplayObject*)LuaProxy::GetProxyableObject( L, 1 );

		Rtt_WARN_SIM_PROXY_TYPE( L, 1, PlatformDisplayObject );
	}

	const MPlatform& platform = LuaContext::GetPlatform( L );

	platform.SetKeyboardFocus( o );

	return 0;
}

static int
setProperty( lua_State *L )
{
	const MPlatform& platform = LuaContext::GetPlatform( L );

	const char *key = lua_tostring( L, 1 );
	if ( key )
	{
		luaL_argcheck( L, ! lua_isnone( L, 2 ), 2, "no value provided" );
		platform.SetNativeProperty( L, key, 2 );
	}
	else
	{
		luaL_argerror( L, 1, "no string key provided" );
	}

	return 0;
}

static int
getProperty( lua_State *L )
{
	int result = 0;

	const MPlatform& platform = LuaContext::GetPlatform( L );

	const char *key = lua_tostring( L, 1 );
	if ( key )
	{
		result = platform.PushNativeProperty( L, key );
	}
	else
	{
		luaL_argerror( L, 1, "no string key provided" );
	}

	return result;
}

// native.canShowPopup( name )
static int
canShowPopup( lua_State *L )
{
	bool result = false;

	const char *name = lua_tostring( L, 1 );
	if ( name )
	{
		const MPlatform& platform = LuaContext::GetPlatform( L );
		result = platform.CanShowPopup( name );
	}

	lua_pushboolean( L, result );

	return 1;
}

// native.showPopup( name [, options] )
// 
// iOS examples:
// -------------
// 	local mailOptions = {
//		to={ "a@z.com", "b@z.com" }, -- optional (string or, for multiple, array of strings)
//		cc={}, -- optional (string or, for multiple, array of strings)
//		bcc={}, -- optional (string or, for multiple, array of strings)
//		attachment = { baseDir=, filename= [, type=] }, -- optional (single element or, for multiple, array of them)
//		body="some text in the body", -- optional (string)
//		isBodyHtml= false, -- optional (boolean)
//		subject="", -- optional (string)
//		listener=function(event) end, -- optional (function) [TODO] We punt for now. And we may never offer this...
// 	}
// 	native.showPopup( "mail", mailOptions )
// 
static int
showPopup( lua_State *L )
{
	bool result = false;

	const MPlatform& platform = LuaContext::GetPlatform( L );

	const char *name = NULL;
	if ( LUA_TSTRING == lua_type( L, 1 ) )
	{
		name = lua_tostring( L, 1 );
	}

	int optionsIndex = 2;
	if ( lua_isnoneornil( L, optionsIndex ) )
	{
		optionsIndex = 0;
	}
	else if ( LUA_TTABLE != lua_type( L, optionsIndex ) )
	{
		optionsIndex = -1;
	}

	if ( ! name )
	{
		CoronaLuaError( L, "native.showPopup() bad argument #1 (expected string name of the popup but got %s)", lua_typename( L, lua_type( L, 1 ) ) );
	}
	else if ( optionsIndex < 0 )
	{
		CoronaLuaError( L, "native.showPopup() bad argument #2 (expected table but got %s)", lua_typename( L, lua_type( L, 2 ) ) );
	}
	else
	{
		result = platform.ShowPopup( L, name, optionsIndex );
		if ( ! result )
		{
			CoronaLuaError( L, "native.showPopup() does not support %s popups on this device", name );
		}
	}

	lua_pushboolean( L, result );
	return 1;
}

// native.hidePopup( name )
// 
// iOS examples:
// -------------
// native.hidePopup( "mail" )
static int
hidePopup( lua_State *L )
{
	const MPlatform& platform = LuaContext::GetPlatform( L );

	const char *name = NULL;
	if ( LUA_TSTRING == lua_type( L, 1 ) )
	{
		name = lua_tostring( L, 1 );
	}

	if ( ! name )
	{
		CoronaLuaError( L, "native.hidePopup() bad argument #1 (expected string name of the popup but got %s)", lua_typename( L, lua_type( L, 1 ) ) );
	}
	else if ( ! platform.HidePopup( name ) )
	{
		CoronaLuaError( L, "native.hidePopup() does not support %s popups on this device", name );
	}

	return 0;
}

static int
setSync( lua_State* L )
{
	const MPlatform& platform = LuaContext::GetPlatform( L );
	return platform.SetSync( L );
}

static int
getSync( lua_State* L )
{
	const MPlatform& platform = LuaContext::GetPlatform( L );
	return platform.GetSync( L );
}

void
LuaLibNative::Initialize( lua_State *L )
{
	const luaL_Reg kVTable[] =
	{
		{ "showAlert", showAlert },
		{ "cancelAlert", cancelAlert },
		{ "setActivityIndicator", setActivityIndicator },
		{ "showWebPopup", showWebPopup },
		{ "cancelWebPopup", cancelWebPopup },
		{ "newTextBox", newTextBox },
		{ "newTextField", newTextField },
		{ "newMapView", newMapView },
		{ "newWebView", newWebView },
		{ "newVideo", newVideo },
		{ "requestExit", requestExitApplication },
		{ "requestExitActivity", requestExitActivity },
		{ "newFont", newFont },
		{ "getFontNames", getFontNames },
		{ "setKeyboardFocus", setKeyboardFocus },
		{ "setProperty", setProperty },
		{ "getProperty", getProperty },
		{ "canShowPopup", canShowPopup },
		{ "showPopup", showPopup },
		{ "hidePopup", hidePopup },
		{ "setSync", setSync },
		{ "getSync", getSync },

		{ NULL, NULL }
	};

	luaL_register( L, "native", kVTable );
	{
		lua_pushlightuserdata( L, UserdataForEnum( kFonts, PlatformFont::kSystemFont ) );
		lua_setfield( L, -2, "systemFont" );
		lua_pushlightuserdata( L, UserdataForEnum( kFonts, PlatformFont::kSystemFontBold ) );
		lua_setfield( L, -2, "systemFontBold" );
	}
	lua_pop( L, 1 ); // pop "native" table

	// Create kNativeFont metatable
	Lua::InitializeGCMetatable( L, kNativeFont, gcFontWrapper );

	// Create kNativeAlert metatable
	luaL_newmetatable( L, kNativeAlert );
	lua_pop( L, 1 ); // remove mt from stack
}

int
LuaLibNative::PushFont( lua_State *L, PlatformFont *font )
{
	int result = 0;

	if ( font )
	{
		PlatformFont **userdata = (PlatformFont**)lua_newuserdata( L, sizeof( PlatformFont* ) );
		*userdata = font;

		luaL_getmetatable( L, kNativeFont );
		lua_setmetatable( L, -2 );

		result = 1;
	}

	return result;
}

PlatformFont*
LuaLibNative::ToFont( lua_State *L, int index )
{
	return *(PlatformFont**)luaL_checkudata( L, index, kNativeFont );
}

bool
LuaLibNative::IsFont( lua_State *L, int index )
{
	if ( lua_isuserdata( L, index ) )
	{
		if ( EnumForUserdata( kFonts, lua_touserdata(L, index), PlatformFont::kNumSystemFonts, -1 ) == -1 )
		{
			return true;
		}
	}
	return false;
}

void
LuaLibNative::AlertComplete( LuaResource& resource, S32 buttonIndex, bool cancelled )
{
	CompletionEvent e;
	int nargs = resource.PushListenerAndEvent( e );
	if ( nargs > 0 )
	{
		lua_State *L = resource.L(); Rtt_ASSERT( L );

		RuntimeGuard guard( * LuaContext::GetRuntime( L ) );

		lua_pushinteger( L, buttonIndex + 1 ); // Lua indices are 1-based
		lua_setfield( L, -2, "index" );
		lua_pushstring( L, cancelled ? "cancelled" : "clicked" );
		lua_setfield( L, -2, "action" );
		LuaContext::DoCall( L, nargs, 0 );
	}
/*
	int nargs = resource.PushListener( CompletionEvent::kName );
	if ( nargs > 0 )
	{
		// PushListener returns number of arguments it pushed on the stack
		// including the function itself. We should subtract one, but since 
		// we're pushing one more arg (below), the net effect is zero.
		lua_State *L = resource.LuaState().Dereference();
		if ( L )
		{
			CompletionEvent e;
			e.Push( L );
			lua_pushinteger( L, buttonIndex + 1 ); // Lua indices are 1-based
			lua_setfield( L, -2, "index" );
			lua_pushstring( L, cancelled ? "cancelled" : "clicked" );
			lua_setfield( L, -2, "action" );
			LuaContext::DoCall( L, nargs, 0 );
		}
	}
*/
/*
	resource.Push();
	lua_State *L = resource.L();

	if ( ! lua_isnil( L, -1 ) )
	{
		int nargs = 1;
		if ( lua_istable( L, -1 ) )
		{
			++nargs; // t is first arg to t.onComplete method
			lua_pushvalue( L, -1 );
			lua_getfield( L, -1, "onComplete" ); // t.onComplete
			lua_insert( L, -2 ); // move t.onComplete before t
		}

		lua_newtable( L );
		lua_pushinteger( L, buttonIndex + 1 ); // Lua indices are 1-based
		lua_setfield( L, -2, "index" );
		lua_pushstring( L, cancelled ? "cancelled" : "clicked" );
		lua_setfield( L, -2, "action" );
		LuaContext::DoCall( L, nargs, 0 );
	}
*/
}

void
LuaLibNative::PopupClosed( LuaResource& resource, const char *popupName, bool wasCanceled )
{
	PopupClosedEvent e(popupName, wasCanceled);
	int nargs = resource.PushListenerAndEvent( e );
	if ( nargs > 0 )
	{
		lua_State *L = resource.L(); Rtt_ASSERT( L );
		RuntimeGuard guard( * LuaContext::GetRuntime( L ) );
		LuaContext::DoCall( L, nargs, 0 );
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

