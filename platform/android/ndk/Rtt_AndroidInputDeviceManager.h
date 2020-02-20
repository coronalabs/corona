//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidInputDeviceManager_H__
#define _Rtt_AndroidInputDeviceManager_H__

#include "Input/Rtt_PlatformInputDeviceManager.h"


// Forward declarations.
class NativeToJavaBridge;
namespace Rtt
{
	class AndroidInputDevice;
}


namespace Rtt
{

/// Stores a collection of all input devices that Corona supports and assigns them
/// unique identifiers and descriptors for easy lookup within native code and in Lua.
class AndroidInputDeviceManager : public PlatformInputDeviceManager
{
	public:
		AndroidInputDeviceManager(Rtt_Allocator *allocatorPointer, NativeToJavaBridge *ntjb);
		virtual ~AndroidInputDeviceManager();

		AndroidInputDevice* GetByCoronaDeviceId(int id) const;

	protected:
		virtual PlatformInputDevice* CreateUsing(const InputDeviceDescriptor &descriptor);
		virtual void Destroy(PlatformInputDevice* devicePointer);

	private:
		NativeToJavaBridge *fNativeToJavaBridge;
};

} // namespace Rtt

#endif // _Rtt_AndroidInputDeviceManager_H__
