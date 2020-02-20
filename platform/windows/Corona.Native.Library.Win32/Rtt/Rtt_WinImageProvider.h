//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_WinImageProvider_H__
#define _Rtt_WinImageProvider_H__

#include "Rtt_PlatformImageProvider.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class WinImageProvider : public PlatformImageProvider
{
	public:
		WinImageProvider( const ResourceHandle<lua_State> & handle ) : PlatformImageProvider( handle )
		{
		}
		virtual bool Supports( int source ) const;
		virtual bool Show( int source, const char* filePath, lua_State* L );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_WinImageProvider_H__
