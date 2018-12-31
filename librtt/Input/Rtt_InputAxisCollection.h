//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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
