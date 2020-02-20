//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformImageProvider_H__
#define _Rtt_PlatformImageProvider_H__

#include "Core/Rtt_Types.h"

#include "Rtt_PlatformMediaProviderBase.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class PlatformBitmap;
class PlatformData;

// ----------------------------------------------------------------------------

class PlatformImageProvider : public PlatformMediaProviderBase
{
	public:
		typedef PlatformMediaProviderBase Super;

		PlatformImageProvider( const ResourceHandle<lua_State> & handle ) : PlatformMediaProviderBase( handle )
		{
		}

	public:
//		static void* UserdataForSource( Source );
//		static Source SourceForUserdata( void* );

	public:
		virtual bool Supports( int source ) const = 0;
		virtual bool HasAccessTo( int source ) const { return Supports(source); }
	
		// If 'filePath' is NULL, then image is delivered as a display object in the 'target' 
		// property of the event.
		// Otherwise, no display object is created. Instead, save image to 'filePath'. 
		virtual bool Show( int source, const char* filePath, lua_State* L ) = 0;

	public:

	protected:
		struct Parameters
		{
			Parameters( PlatformBitmap* bitmap, PlatformData* data );
			~Parameters();

			PlatformBitmap* bitmap;
			PlatformData* data;
			bool wasCompleted;
		};

		static void AddProperties( lua_State *L, void* userdata );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformImageProvider_H__
