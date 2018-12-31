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

#pragma once

#include "Interop\EventArgs.h"
#include <stdint.h>


namespace Interop { namespace Input {

/// <summary>Event arguments providing a single data value received from one input device's axis.</summary>
class InputAxisDataEventArgs : public EventArgs
{
	public:
		/// <summary>
		///  <para>Stores a single data value received from one input device's axis.</para>
		///  <para>This data structure was designed to be passed around by value cheaply/efficiently.</para>
		///  <para>This data structure is used to create an InputAxisDataEventArgs instance.</para>
		/// </summary>
		struct Data
		{
			/// <summary>Zero based index of the axis that data was received from.</summary>
			unsigned int AxisIndex;

			/// <summary>The raw axis data value that was received.</summary>
			int32_t RawValue;
		};

		/// <summary>Creates a new event arguments object with the given axis data.</summary>
		/// <param name="data">The received axis data.</param>
		InputAxisDataEventArgs(const InputAxisDataEventArgs::Data& data);

		/// <summary>Destroys this object.</summary>
		virtual ~InputAxisDataEventArgs();

		/// <summary>Gets a zero based index of the axis data was received from.</summary>
		/// <returns>Returns a zero based index of the axis data was received from.</returns>
		unsigned int GetAxisIndex() const;

		/// <summary>
		///  <para>Gets the raw axis data value that was received.</para>
		///  <para>This value is expected to be between the axis' defined min and max range.</para>
		/// </summary>
		/// <returns>Returns the raw axis data value received.</returns>
		int32_t GetRawValue() const;

		/// <summary>Gets this axis event's full data as a structure.</summary>
		/// <returns>Returns a copy of this axis event's full data as a structure.</returns>
		InputAxisDataEventArgs::Data ToData() const;

		/// <summary>Creates a new event arguments object with the given axis data.</summary>
		/// <param name="data">The received axis data.</param>
		/// <returns>Returns a new event arguments object providing the given axis data.</returns>
		static InputAxisDataEventArgs From(const InputAxisDataEventArgs::Data& data);

	private:
		/// <summary>Assignment operator made private to make this object immutable.</summary>
		void operator=(const InputAxisDataEventArgs& value) {}

		/// <summary>Stores the axis event's data.</summary>
		InputAxisDataEventArgs::Data fData;
};

} }	// namespace Interop::Input
