//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_TargetAndroidAppStore_H__
#define _Rtt_TargetAndroidAppStore_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include "Rtt_TargetDevice.h"


namespace Rtt
{

/// Provides Android app store build targets to be used by the Corona Simulator build window.
class TargetAndroidAppStore
{
	private:
		TargetAndroidAppStore(
			const char *storeStringId, const char *storeName, TargetDevice::Platform platform);

	public:
		virtual ~TargetAndroidAppStore();

		static const TargetAndroidAppStore kNone;
		static const TargetAndroidAppStore kGoogle;
		static const TargetAndroidAppStore kAmazon;
		static const TargetAndroidAppStore kOuya;
		static const TargetAndroidAppStore kGameStick;

		const char* GetStringId() const;
		const char* GetName() const;
		TargetDevice::Platform GetPlatform() const;

		static int GetCount();
		static TargetAndroidAppStore* GetByIndex(int index);
		static TargetAndroidAppStore* GetByStringId(const char *stringId);

	private:
		String fStoreStringId;
		String fStoreName;
		TargetDevice::Platform fPlatform;
};

} // namespace Rtt

#endif // _Rtt_TargetAndroidAppStore_H__
