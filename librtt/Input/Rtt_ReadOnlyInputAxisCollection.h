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
