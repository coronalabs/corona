//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidStore_H__
#define _Rtt_AndroidStore_H__

#include "Rtt_PlatformInAppStore.h"

class NativeToJavaBridge;

namespace Rtt
{

class AndroidStore : public PlatformStore
{
	public:
		AndroidStore( const ResourceHandle<lua_State>& handle, NativeToJavaBridge *ntjb );
		virtual ~AndroidStore();
		
	public:
		void UseStore( const char *storeName );
		void StartObservingTransactions();
		void StopObservingTransactions();
		void LoadProducts( lua_State *L, int index );
		void Purchase( lua_State *L, int index );
		bool CanMakePurchases() const;
		bool CanLoadProducts() const;
		void FinishTransaction( PlatformStoreTransaction *transaction );
		void RestoreCompletedTransactions();

	private:
		NativeToJavaBridge *fNativeToJavaBridge;
};

} // namespace Rtt

#endif // _Rtt_AndroidStore_H__
