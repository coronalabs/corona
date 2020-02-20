//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_PlatformImageProvider.h"

#include "Display/Rtt_BitmapPaint.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_LuaLibDisplay.h"
#include "Display/Rtt_PlatformBitmap.h"
#include "Display/Rtt_ShapeObject.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_PlatformData.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

/*
void*
PlatformImageProvider::UserdataForSource( Source s )
{
	return const_cast< char* >( & kSources[s] );
}

PlatformImageProvider::Source
PlatformImageProvider::SourceForUserdata( void* p )
{
	U32 offset = (const char*)p - kSources;
	return offset < kNumSources ? (Source)offset : kPhotoLibrary;
}
*/

PlatformImageProvider::Parameters::Parameters( PlatformBitmap* aBitmap, PlatformData* aData )
:	bitmap( aBitmap ),
	data( aData )
{
	wasCompleted = (bitmap || data);
}

PlatformImageProvider::Parameters::~Parameters()
{
	// If callback sets this->bitmap to NULL, then they assume ownership.
	// Otherwise, we free it here.  Ditto for this->data.
	Rtt_DELETE( data );
	Rtt_DELETE( bitmap );
}

void
PlatformImageProvider::AddProperties( lua_State *L, void* userdata )
{
	Parameters* params = (Parameters*)userdata;
	PlatformBitmap* bitmap = params->bitmap;
	if ( bitmap )
	{
		// Add more info like image and editing info
		Runtime& runtime = * LuaContext::GetRuntime( L );

		BitmapPaint* paint = BitmapPaint::NewBitmap( runtime.GetDisplay().GetTextureFactory(), bitmap, false );
		LuaLibDisplay::PushImage( L, NULL, paint, runtime.GetDisplay(), NULL );
		lua_setfield( L, -2, "target" );

		/*
		PlatformData* data = params->data;
		if ( data && data->PushIterator( L ) )
		{
			lua_setfield( L, -2, "source" );
		}
		 */

		// Prevent caller from freeing bitmap, i.e. assume ownership
		params->bitmap = NULL;
		params->data = NULL;
	}
	lua_pushboolean( L, params->wasCompleted ? 1 : 0 );
	lua_setfield( L, -2, "completed" );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

