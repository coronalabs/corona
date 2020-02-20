//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformVideoProvider_H__
#define _Rtt_PlatformVideoProvider_H__

#include "Core/Rtt_Types.h"

#include "Rtt_PlatformMediaProviderBase.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class PlatformVideoProvider : public PlatformMediaProviderBase
{
	public:
		typedef PlatformMediaProviderBase Super;

		PlatformVideoProvider( const ResourceHandle<lua_State> & handle ) : PlatformMediaProviderBase( handle )
		{
		}

	public:

	public:
		virtual bool Supports( int source ) const = 0;
		
		virtual bool Show( int source, lua_State* L, int maxTime, int quality ) = 0;

	public:

	protected:

		struct VideoProviderResult
		{
			VideoProviderResult() : SelectedVideoFileName(NULL)
			{ }
			const char* SelectedVideoFileName;
			int Duration;
			long Size;
		};

		static void AddProperties( lua_State *L, void* userdata );

// ----------------------------------------------------------------------------

};

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformVideoProvider_H__
