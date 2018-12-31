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

#ifndef _Rtt_InputAxisDescriptor_H__
#define _Rtt_InputAxisDescriptor_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Types.h"
#include "Rtt_InputAxisType.h"
#include "Rtt_InputDeviceDescriptor.h"


// Forward declarations.
struct Rtt_Allocator;
namespace Rtt
{
	class String;
}


namespace Rtt
{

/// Class used to uniquely identify a specific axis input belonging to one input device.
/// Generates a unique human readable descriptor name such as "Joystick 1: Axis 1", "Joystick 1: Axis 2", etc.
/// <br>
/// Instances of this class are immutable.
class InputAxisDescriptor
{
	public:
		InputAxisDescriptor(const InputDeviceDescriptor &deviceDescriptor, S32 axisNumber);
		InputAxisDescriptor(const InputAxisDescriptor &descriptor);
		virtual ~InputAxisDescriptor();

		Rtt_Allocator* GetAllocator() const;
		const InputDeviceDescriptor& GetDeviceDescriptor() const;
		S32 GetAxisNumber() const;
		const char* GetInvariantName() const;
		bool Equals(const InputAxisDescriptor &descriptor) const;
		bool NotEquals(const InputAxisDescriptor &descriptor) const;
		bool operator==(const InputAxisDescriptor &descriptor) const;
		bool operator!=(const InputAxisDescriptor &descriptor) const;

	private:
		// Assignment operator made private to make this object immutable.
		void operator=(const InputAxisDescriptor &descriptor) { };


		/// The descriptor's unique human readable name. Created on demand.
		mutable String *fInvariantNamePointer;

		/// Descriptor used to uniquely identify the device the input axis belongs to.
		InputDeviceDescriptor fDeviceDescriptor;

		/// The axis input's assigned number.
		S32 fAxisNumber;
};

} // namespace Rtt

#endif // _Rtt_InputAxisDescriptor_H__
