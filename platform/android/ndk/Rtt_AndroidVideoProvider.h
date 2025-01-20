//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidVideoProvider_H__
#define _Rtt_AndroidVideoProvider_H__

#include "librtt/Rtt_PlatformVideoProvider.h"

class NativeToJavaBridge;

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

/// Provides an video picker window. Selected video can be sent to a Lua listener function.
class AndroidVideoProvider : public PlatformVideoProvider
{
	public:
		AndroidVideoProvider( const ResourceHandle<lua_State> & handle, NativeToJavaBridge *ntjb );
		virtual bool Supports( int source ) const;
		virtual bool Show( int source, lua_State* L, int maxTime, int quality );
		virtual bool IsShown() { return fIsVideoProviderShown; }
		virtual void CloseWithResult( const char *selectedVideoFileName, const int duration, const long size );
		void Abort() { fIsVideoProviderShown = false; }
	
	private:
		
		/// Set TRUE if video picker window is currently shown. Set FALSE if not.
		bool fIsVideoProviderShown;
		NativeToJavaBridge *fNativeToJavaBridge;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidVideoProvider_H__
