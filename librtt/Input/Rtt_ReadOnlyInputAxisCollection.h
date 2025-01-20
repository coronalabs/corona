//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_ReadOnlyInputAxisCollection_H__
#define _Rtt_ReadOnlyInputAxisCollection_H__

#include "Core/Rtt_Types.h"


// Forward declarations.
namespace Rtt
{
	class InputAxisDescriptor;
	class InputAxisCollection;
	class PlatformInputAxis;
}


namespace Rtt
{

/// Collection which wraps and provides read-only access to an existing InputAxisCollection instance.
/// <br>
/// Note that the collection this read-only container wraps is mutable and can change.
/// <br>
/// This read-only collection will not delete the InputAxisCollection instance that it has been given.
class ReadOnlyInputAxisCollection
{
	public:
		ReadOnlyInputAxisCollection(InputAxisCollection *collectionPointer);
		virtual ~ReadOnlyInputAxisCollection();

		Rtt_Allocator* GetAllocator() const;
		S32 GetCount() const;
		PlatformInputAxis* GetBy(const InputAxisDescriptor &descriptor) const;
		PlatformInputAxis* GetByIndex(S32 index) const;
		PlatformInputAxis* GetByAxisNumber(S32 number) const;
		PlatformInputAxis* GetByDescriptorName(const char *name) const;
		bool Contains(PlatformInputAxis *axisPointer) const;

	private:
		/// Pointer to the axis collection that this collection wraps.
		InputAxisCollection *fCollectionPointer;
};

} // namespace Rtt

#endif // _Rtt_ReadOnlyInputAxisCollection_H__
