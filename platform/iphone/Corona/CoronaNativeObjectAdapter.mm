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

#import "CoronaNativeObjectAdapter.h"

#include "Rtt_IPhoneNativeObject.h"
#include "Rtt_LuaContext.h"
#include "Display/Rtt_LuaLibDisplay.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

CORONA_EXPORT int CoronaPushNativeObject( lua_State *L, CGRect bounds, id<CoronaNativeObjectAdapter>adapter )
{
	using namespace Rtt;

	int result = 0;

	Runtime& runtime = * LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime.Platform();
	
	float x = bounds.origin.x;
	float y = bounds.origin.y;
	float w = bounds.size.width;
	float h = bounds.size.height;

	if ( w > 0 && h > 0 )
	{
		Rtt::Rect r;
		r.xMin = x;
		r.yMin = y;
		r.xMax = x + w;
		r.yMax = y + h;
		IPhoneNativeObject *o = Rtt_NEW( platform.GetAllocator(), IPhoneNativeObject( r, adapter ) );

		if ( o )
		{
			Display& display = runtime.GetDisplay();
			o->Preinitialize( display );
			o->SetHandle( & platform.GetAllocator(), runtime.VMContext().LuaState() );

			result = LuaLibDisplay::AssignParentAndPushResult( L, display, o, NULL );
			
			if ( result > 0 )
			{
				o->Initialize();
			}
		}
	}

	return result;
}

// ----------------------------------------------------------------------------

