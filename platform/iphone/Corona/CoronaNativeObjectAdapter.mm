//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

