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

#ifndef _Rtt_InputDeviceDescriptor_H__
#define _Rtt_InputDeviceDescriptor_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Types.h"
#include "Core/Rtt_AutoPtr.h"
#include "Rtt_InputDeviceType.h"


// Forward declarations.
struct Rtt_Allocator;
namespace Rtt
{
	class String;
}


namespace Rtt
{

/// Class used to uniquely identify a specific input device and to generate a unique human readable
/// descriptor name for that device such as "Keyboard 1", "Joystick 1", "Joystick 2", etc.
/// <br>
/// Instances of this class are immutable.
class InputDeviceDescriptor
{
	public:
		InputDeviceDescriptor(Rtt_Allocator *allocatorPointer, InputDeviceType deviceType, S32 deviceNumber);
		InputDeviceDescriptor(const InputDeviceDescriptor &descriptor);
		virtual ~InputDeviceDescriptor();

		Rtt_Allocator* GetAllocator() const;
		InputDeviceType GetDeviceType() const;
		S32 GetDeviceNumber() const;
		S64 GetIntegerId() const;
		const char* GetInvariantName() const;
		bool Equals(const InputDeviceDescriptor &descriptor) const;
		bool NotEquals(const InputDeviceDescriptor &descriptor) const;
		bool operator==(const InputDeviceDescriptor &descriptor) const;
		bool operator!=(const InputDeviceDescriptor &descriptor) const;

		/// Object type that is returned by the InputDeviceDescriptor::FromIntegerId() static function.
		/// Indicates if conversion was successful, and if so, provides the descriptor.
		class ConversionResult
		{
			public:
				ConversionResult();
				ConversionResult(const InputDeviceDescriptor &descriptor);
				bool HasSucceeded() const;
				bool HasFailed() const;
				const InputDeviceDescriptor* GetDescriptor() const;
			private:
				AutoPtr<InputDeviceDescriptor> fDescriptorReference;
		};

		static ConversionResult FromIntegerId(Rtt_Allocator *allocatorPointer, S64 id);

	private:
		// Assignment operator made private to make this object immutable.
		void operator=(const InputDeviceDescriptor &descriptor) { };


		/// Allocator used to create the descriptor's invariant name.
		Rtt_Allocator *fAllocatorPointer;

		/// The descriptor's unique human readable name. Created on demand.
		mutable String *fInvariantNamePointer;

		/// The type of input device such as "keyboard", "mouse", "joystick", etc.
		InputDeviceType fDeviceType;

		/// The device's assigned number.
		S32 fDeviceNumber;
};

} // namespace Rtt

#endif // _Rtt_InputDeviceDescriptor_H__
