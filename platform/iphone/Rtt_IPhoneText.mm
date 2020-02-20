//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_IPhoneText.h"

#include "Rtt_Lua.h"
#include "Display/Rtt_LuaLibDisplay.h"

#include <CoreGraphics/CGBase.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

UIColor*
IPhoneText::GetTextColor( lua_State *L, int index, bool isByteColorRange )
{
	Color c = LuaLibDisplay::toColor(L,index,isByteColorRange);
	RGBA rgba = ( (ColorUnion*)(& c) )->rgba;
	
	CGFloat r = (CGFloat)rgba.r / 255.0f;
	CGFloat g = (CGFloat)rgba.g / 255.0f;
	CGFloat b = (CGFloat)rgba.b / 255.0f;
	CGFloat a = (CGFloat)rgba.a / 255.0f;

	return [UIColor colorWithRed:r green:g blue:b alpha:a];
}

UIReturnKeyType
IPhoneText::GetUIReturnKeyTypeFromIMEType( const char* imeType)
{
	UIReturnKeyType type = UIReturnKeyDefault;
    if ( strcmp( "go", imeType ) == 0)
    {
        type = UIReturnKeyGo;
    }
    else if ( strcmp( "join", imeType) == 0)
    {
        type = UIReturnKeyJoin;
    }
    else if ( strcmp( "next", imeType) == 0)
    {
        type = UIReturnKeyNext;
    }
    else if ( strcmp( "route", imeType) == 0)
    {
        type = UIReturnKeyRoute;
    }
    else if ( strcmp( "search", imeType) == 0)
    {
        type = UIReturnKeySearch;
    }
    else if ( strcmp( "send", imeType) == 0)
    {
        type = UIReturnKeySend;
    }
    else if ( strcmp( "done", imeType) == 0)
    {
        type = UIReturnKeyDone;
    }
    else if ( strcmp( "emergencycall", imeType) == 0)
    {
        type = UIReturnKeyEmergencyCall;
    }
	return type;        
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

