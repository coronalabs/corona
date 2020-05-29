//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidStoreProvider_H__
#define _Rtt_AndroidStoreProvider_H__

#include "Rtt_PlatformInAppStore.h"
#include "Rtt_AndroidStore.h"

class NativeToJavaBridge;

namespace Rtt
{
class String;

class AndroidStoreProvider : public PlatformStoreProvider
{
	public:
		AndroidStoreProvider( const ResourceHandle<lua_State>& handle, NativeToJavaBridge *ntjb );
		virtual ~AndroidStoreProvider();
	
		void SetActiveStore( const char *storeName );
		PlatformStore* GetActiveStore();
		const char* GetTargetedStoreName();
	
	private:
		AndroidStore *fActiveStore;
		String *fTargetedStoreName;
		NativeToJavaBridge *fNativeToJavaBridge;
};

} // namespace Rtt

#endif // _Rtt_AndroidStoreProvider_H__
