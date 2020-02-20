//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "CoronaLuaIOS.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"
#include "Rtt_ApplePlatform.h"
#include "Rtt_Lua.h"
#include "Rtt_AppleBitmap.h"
#include "Rtt_Runtime.h"
#include "Display/Rtt_LuaLibDisplay.h"
#include "Display/Rtt_BitmapPaint.h"
#include "Display/Rtt_Display.h"
// ----------------------------------------------------------------------------


CORONA_API int
CoronaLuaPushImage( lua_State *L, UIImage *image )
{
	using namespace Rtt;

	int result = 0;

	if ( image )
	{
		AppleFileBitmap* bitmap = NULL;
		Runtime* runtime = LuaContext::GetRuntime( L );
		
		Rtt_Allocator* allocator = &runtime->Platform().GetAllocator();
		Rtt_UNUSED(allocator);
		bitmap = Rtt_NEW( allocator, IPhoneFileBitmap( image ) );
		BitmapPaint* paint = BitmapPaint::NewBitmap( runtime->GetDisplay().GetTextureFactory(), bitmap, false );
		
		if ( bitmap && paint )
		{
			ShapeObject *object = LuaLibDisplay::PushImage( L, NULL, paint, runtime->GetDisplay(), NULL );
			result = ( NULL != object );
		}
	}

	return result;
}
