//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_WinVideoProvider_H__
#define _Rtt_WinVideoProvider_H__

#include "Rtt_PlatformVideoProvider.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class WinVideoProvider : public PlatformVideoProvider
{
	public:
		WinVideoProvider( const ResourceHandle<lua_State> & handle ) : PlatformVideoProvider( handle )
		{
		}
		virtual bool Supports( int source ) const;
		virtual bool Show( int source, lua_State* L, int maxTime, int quality );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_WinVideoProvider_H__
