//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_InputDeviceCollection_H__
#define _Rtt_InputDeviceCollection_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Array.h"
#include "Rtt_PlatformInputDevice.h"


// Forward declarations.
struct Rtt_Allocator;
namespace Rtt
{
	class InputDeviceDescriptor;
}


namespace Rtt
{

/// Stores a collection of PlatformInputDevice object pointers for easy retrieval.
/// <br>
/// Note that this collection will not delete the objects that it stores.
/// The class that creates those objects is responsible for deleting them.
class InputDeviceCollection
{
	public:
		InputDeviceCollection(Rtt_Allocator *allocatorPointer);
		InputDeviceCollection(const InputDeviceCollection &collection);
		virtual ~InputDeviceCollection();

		void Add(PlatformInputDevice *devicePointer);
		void Add(const InputDeviceCollection &collection);
		Rtt_Allocator* GetAllocator() const;
		S32 GetCount() const;
		PlatformInputDevice* GetBy(const InputDeviceDescriptor &descriptor) const;
		PlatformInputDevice* GetByIndex(S32 index) const;
		PlatformInputDevice* GetByDescriptorName(const char *name) const;
		PlatformInputDevice* GetByDescriptorId(S64 id) const;
		bool CopyConnectedDevicesTo(InputDeviceCollection &collection) const;
		bool Contains(PlatformInputDevice *devicePointer) const;
		bool Remove(PlatformInputDevice *devicePointer);
		bool RemoveBy(const InputDeviceDescriptor &descriptor);
		bool RemoveByIndex(S32 index);
		bool RemoveByDescriptorName(const char *name);
		bool RemoveByDescriptorId(S64 id);
		void Clear();
		void operator=(const InputDeviceCollection &collection);

	private:
		/// Allocator used to create this collection's internal array.
		Rtt_Allocator *fAllocatorPointer;

		/// Stores a collection of input device object pointers.
		LightPtrArray<PlatformInputDevice> fCollection;
};

} // namespace Rtt

#endif // _Rtt_InputDeviceCollection_H__
