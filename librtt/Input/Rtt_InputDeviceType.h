//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_InputDeviceType_H__
#define _Rtt_InputDeviceType_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_Types.h"
#include "Core/Rtt_AutoPtr.h"


namespace Rtt
{

/// Indicates the type of device that input events come from such as a keyboard, touchscreen, gamepad, etc.
/// Provides unique IDs that are used by Corona events and by input descriptors.
class InputDeviceType
{
	private:
		InputDeviceType(S32 integerId, const char *stringId, const char *invariantName);

	public:
		InputDeviceType();
		virtual ~InputDeviceType();

		static const InputDeviceType kUnknown;
		static const InputDeviceType kKeyboard;
		static const InputDeviceType kMouse;
		static const InputDeviceType kStylus;
		static const InputDeviceType kTrackball;
		static const InputDeviceType kTouchpad;
		static const InputDeviceType kTouchscreen;
		static const InputDeviceType kJoystick;
		static const InputDeviceType kGamepad;
		static const InputDeviceType kDirectionalPad;
		static const InputDeviceType kSteeringWheel;
		static const InputDeviceType kFlightStick;
		static const InputDeviceType kGuitar;
		static const InputDeviceType kDrumSet;
		static const InputDeviceType kDancePad;

		S32 GetIntegerId() const;
		const char* GetStringId() const;
		const char* GetInvariantName() const;
		bool Equals(const InputDeviceType &value) const;
		bool NotEquals(const InputDeviceType &value) const;
		bool operator==(const InputDeviceType &value) const;
		bool operator!=(const InputDeviceType &value) const;

		/// Object type that is returned by the InputDeviceType::From() static functions.
		/// Indicates if conversion was successful, and if so, provides the device type.
		class ConversionResult
		{
			public:
				ConversionResult();
				ConversionResult(Rtt_Allocator *allocatorPointer, const InputDeviceType &type);
				bool HasSucceeded() const;
				bool HasFailed() const;
				const InputDeviceType* GetDeviceType() const;
			private:
				AutoPtr<InputDeviceType> fDeviceTypeReference;
		};

		static ConversionResult FromIntegerId(Rtt_Allocator *allocatorPointer, S32 id);
		static InputDeviceType::ConversionResult FromStringId(Rtt_Allocator *allocatorPointer, const char *stringId);
		static InputDeviceType::ConversionResult FromInvariantName(Rtt_Allocator *allocatorPointer, const char *name);

	private:
		/// Stores the device type's unique integer ID.
		S32 fIntegerId;

		/// The unique string ID assigned to this device type such as "keyboard", "mouse", "joystick", etc.
		/// <br>
		/// The string ID is typically used by a Lua event table.
		const char *fStringId;

		/// The unique human readable name for this device type such as "Keyboard", "Unknown Device", etc.
		/// <br>
		/// This name is not localized and is intended to be prefixed to a device's descriptor name.
		const char *fInvariantName;
};

} // namespace Rtt

#endif // _Rtt_InputDeviceType_H__
