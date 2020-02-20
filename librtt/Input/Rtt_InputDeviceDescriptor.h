//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
