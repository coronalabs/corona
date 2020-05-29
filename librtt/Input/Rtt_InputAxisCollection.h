//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_InputAxisCollection_H__
#define _Rtt_InputAxisCollection_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Array.h"
#include "Rtt_PlatformInputAxis.h"


// Forward declarations.
struct Rtt_Allocator;
namespace Rtt
{
	class InputAxisDescriptor;
}


namespace Rtt
{

/// Stores a collection of PlatformInputAxis object pointers for easy retrieval.
/// <br>
/// Note that this collection will not delete the objects that it stores.
/// The class that creates those objects is responsible for deleting them.
class InputAxisCollection
{
	public:
		InputAxisCollection(Rtt_Allocator *allocatorPointer);
		InputAxisCollection(const InputAxisCollection &collection);
		virtual ~InputAxisCollection();

		void Add(PlatformInputAxis *axisPointer);
		void Add(const InputAxisCollection &collection);
		Rtt_Allocator* GetAllocator() const;
		S32 GetCount() const;
		PlatformInputAxis* GetBy(const InputAxisDescriptor &descriptor) const;
		PlatformInputAxis* GetByIndex(S32 index) const;
		PlatformInputAxis* GetByAxisNumber(S32 number) const;
		PlatformInputAxis* GetByDescriptorName(const char *name) const;
		bool Contains(PlatformInputAxis *axisPointer) const;
		bool Remove(PlatformInputAxis *axisPointer);
		bool RemoveBy(const InputAxisDescriptor &descriptor);
		bool RemoveByIndex(S32 index);
		bool RemoveByAxisNumber(S32 number);
		bool RemoveByDescriptorName(const char *name);
		void Clear();
		void operator=(const InputAxisCollection &collection);

	private:
		/// Allocator used to create this collection's internal array.
		Rtt_Allocator *fAllocatorPointer;

		/// Stores a collection of input axis object pointers.
		LightPtrArray<PlatformInputAxis> fCollection;
};

} // namespace Rtt

#endif // _Rtt_InputAxisCollection_H__
