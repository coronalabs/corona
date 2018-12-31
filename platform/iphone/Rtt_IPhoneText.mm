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

