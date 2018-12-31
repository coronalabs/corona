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
