//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformInputAxis_H__
#define _Rtt_PlatformInputAxis_H__

#include "Core/Rtt_Types.h"
#include "Rtt_InputAxisDescriptor.h"
#include "Rtt_InputAxisType.h"


extern "C"
{
	struct lua_State;
}


namespace Rtt
{

/// Stores information about one axis input belonging to one input device.
/// Provides a PushTo() function that makes it easy to push this information as a table to Lua.
class PlatformInputAxis
{
	public:
		PlatformInputAxis(const InputAxisDescriptor &descriptor);
		virtual ~PlatformInputAxis();

		InputAxisDescriptor& GetDescriptor();
		InputAxisType GetType() const;
		void SetType(InputAxisType type);
		Rtt_Real GetMinValue() const;
		void SetMinValue(Rtt_Real value);
		Rtt_Real GetMaxValue() const;
		void SetMaxValue(Rtt_Real value);
		Rtt_Real GetAccuracy() const;
		void SetAccuracy(Rtt_Real value);
		bool IsAbsolute() const;
		void SetIsAbsolute(bool value);
		virtual void PushTo(lua_State *L);

		virtual Rtt_Real GetNormalizedValue(Rtt_Real rawValue);
	private:
		// Assignment operator made private to prevent instances from being overwritten.
		void operator=(const PlatformInputAxis &device) { };


		/// Descriptor used to uniquely identify this axis input.
		InputAxisDescriptor fDescriptor;

		/// The type of axis such as "x", "y", "leftTrigger", etc.
		InputAxisType fAxisType;

		/// The minimum value that the axis input can provide.
		Rtt_Real fMinValue;

		/// The maximum value that the axis input can provide.
		Rtt_Real fMaxValue;

		/// The +/- accuracy of an axis value.
		Rtt_Real fAccuracy;

		/// Set true if the axis provides absolute values, like a joystick.
		/// Set false if the axis provides relative values, like a mouse.
		bool fIsAbsolute;
};

} // namespace Rtt

#endif // _Rtt_PlatformInputAxis_H__
