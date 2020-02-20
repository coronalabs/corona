//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_PlatformWebPopup.h"

#include "Display/Rtt_Display.h"
#include "Rtt_Event.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_LuaResource.h"
#include "Rtt_Runtime.h"
#include "Rtt_PlatformDisplayObject.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

PlatformWebPopup::PlatformWebPopup()
:	fCallback( NULL ),
	fContentToScreenSx( Rtt_REAL_1 ),
	fContentToScreenSy( Rtt_REAL_1 ),
	fInCallback( false )
{
}

PlatformWebPopup::~PlatformWebPopup()
{
	Rtt_DELETE( fCallback );
}

// TODO: Duplicate of PlatformDisplayObject.  This sucks.  We should convert 
// this entire thing as a subclass of PlatformDisplayObject
void
PlatformWebPopup::Preinitialize( const Display& display )
{
	Real sx, sy;
	display.CalculateContentToScreenScale( sx, sy );

	SetContentToScreenSx( Rtt_RealDiv( Rtt_REAL_1, sx ) );
	SetContentToScreenSy( Rtt_RealDiv( Rtt_REAL_1, sy ) );
}


// TODO: Duplicate of PlatformDisplayObject.  This sucks.  We should convert 
// this entire thing as a subclass of PlatformDisplayObject
void
PlatformWebPopup::GetScreenBounds( const Display& display, Rect& outBounds ) const
{
	outBounds = GetStageBounds();

	PlatformDisplayObject::CalculateScreenBounds(
		display, fContentToScreenSx, fContentToScreenSy, outBounds );
}

bool
PlatformWebPopup::ShouldLoadUrl( const LuaResource& resource, const char *url, bool& inCallback )
{
	bool result = true; // Default to opening link

	// This is used by Rtt_LuaLibNative.cpp to detect if the webPopup callback is making reentrant calls
	inCallback = true;

	UrlRequestEvent e( url );
	if ( 0 == resource.DispatchEvent( e, 1 ) )
	{
		lua_State *L = resource.L(); Rtt_ASSERT( L );

		result = lua_toboolean( L, -1 ) != 0;

		lua_pop( L, 1 );
	}

	inCallback = false;

	return result;
}

bool
PlatformWebPopup::DidFailLoadUrl( const LuaResource& resource, const char *url, const char *errorMsg, S32 errorCode )
{
	bool result = false; // On failure, default to closing popup

	UrlRequestEvent e( url, errorMsg, errorCode );
	if ( 0 == resource.DispatchEvent( e, 1 ) )
	{
		lua_State *L = resource.L(); Rtt_ASSERT( L );

		result = lua_toboolean( L, -1 ) != 0;

		lua_pop( L, 1 );
	}

	return result;
}

bool
PlatformWebPopup::ShouldLoadUrl( const char *url ) const
{
	bool result = true; // Default to opening link

	if ( fCallback )
	{
		const LuaResource& resource = *fCallback;
		result = Self::ShouldLoadUrl( resource, url, fInCallback );
	}

	return result;
}

bool
PlatformWebPopup::DidFailLoadUrl( const char *url, const char *errorMsg, S32 errorCode ) const
{
	bool result = false; // On failure, default to closing popup

	if ( fCallback )
	{
		const LuaResource& resource = *fCallback;
		result = Self::DidFailLoadUrl( resource, url, errorMsg, errorCode );
	}

	return result;
}

void
PlatformWebPopup::SetStageBounds( const Rect& bounds, const Runtime *runtime )
{
	fRect = bounds;
}
		
const Rect & 
PlatformWebPopup::GetStageBounds() const
{
	return fRect;
}
	
void
PlatformWebPopup::SetCallback( LuaResource *callback )
{
	if ( fCallback != callback )
	{
		Rtt_DELETE( fCallback );
		fCallback = callback;
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

