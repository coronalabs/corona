//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformInputDeviceManager_H__
#define _Rtt_PlatformInputDeviceManager_H__

#include "Rtt_InputDeviceCollection.h"
#include "Rtt_ReadOnlyInputDeviceCollection.h"


// Forward declarations.
struct Rtt_Allocator;
namespace Rtt
{
	class InputDeviceType;
	class PlatformInputDevice;
}


namespace Rtt
{

/// Stores a collection of all input devices that Corona supports and assigns them
/// unique identifiers and descriptors for easy lookup within native code and in Lua.
/// <br>
/// A derived version of this class is intended to be made if a platform has its own
/// derived version of a PlatformInputDevice object. In this case, the derived version
/// of this manager class should override the CreateUsing() and Destroy() functions
/// to create and destroy these platform specific input device objects.
class PlatformInputDeviceManager
{
	public:
		PlatformInputDeviceManager(Rtt_Allocator *allocatorPointer);
		virtual ~PlatformInputDeviceManager();

		PlatformInputDevice* Add(InputDeviceType type);
		Rtt_Allocator* GetAllocator() const;
		const ReadOnlyInputDeviceCollection& GetDevices() const;

	protected:
		virtual PlatformInputDevice* CreateUsing(const InputDeviceDescriptor &descriptor);
		virtual void Destroy(PlatformInputDevice* devicePointer);

	private:
		// Copy constructor and assignment operator made private to prevent object from being overwritten.
		void operator=(const PlatformInputDeviceManager &manager) { };

		/// Stores all input devices that have been discovered.
		/// Will contain disconnected devices that were once connected.
		InputDeviceCollection fDeviceCollection;

		/// Read-only collection which wraps the mutable "fDeviceCollection" member variable.
		/// This is the collection that get exposed outside of this class.
		ReadOnlyInputDeviceCollection fReadOnlyDeviceCollection;
};

} // namespace Rtt

#endif // _Rtt_PlatformInputDeviceManager_H__
