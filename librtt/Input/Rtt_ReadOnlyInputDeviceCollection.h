//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ReadOnlyInputDeviceCollection_H__
#define _Rtt_ReadOnlyInputDeviceCollection_H__

#include "Core/Rtt_Types.h"


// Forward declarations.
namespace Rtt
{
	class InputDeviceDescriptor;
	class InputDeviceCollection;
	class PlatformInputDevice;
}


namespace Rtt
{

/// Collection which wraps and provides read-only access to an existing InputDeviceCollection instance.
/// <br>
/// Note that the collection this read-only container wraps is mutable and can change.
/// <br>
/// This read-only collection will not delete the InputDeviceCollection instance that it has been given.
class ReadOnlyInputDeviceCollection
{
	public:
		ReadOnlyInputDeviceCollection(InputDeviceCollection *collectionPointer);
		virtual ~ReadOnlyInputDeviceCollection();

		Rtt_Allocator* GetAllocator() const;
		S32 GetCount() const;
		PlatformInputDevice* GetBy(const InputDeviceDescriptor &descriptor) const;
		PlatformInputDevice* GetByIndex(S32 index) const;
		PlatformInputDevice* GetByDescriptorName(const char *name) const;
		PlatformInputDevice* GetByDescriptorId(S64 id) const;
		bool CopyConnectedDevicesTo(InputDeviceCollection &collection) const;
		bool Contains(PlatformInputDevice *devicePointer) const;

	private:
		/// Pointer to the device collection that this collection wraps.
		InputDeviceCollection *fCollectionPointer;
};

} // namespace Rtt

#endif // _Rtt_ReadOnlyInputDeviceCollection_H__
