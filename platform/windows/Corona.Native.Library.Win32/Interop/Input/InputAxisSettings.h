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

#include "Input\Rtt_InputAxisType.h"


namespace Interop { namespace Input {

/// <summary>Stores the configuration for one axis input belonging to an input device.</summary>
class InputAxisSettings
{
	public:
		#pragma region Constructors/Destructors
		/// <summary>Creates a new input axis configuration.</summary>
		InputAxisSettings();

		/// <summary>Destroys this object.</summary>
		virtual ~InputAxisSettings();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Gets the type of axis such as kX, kY, kLeftTrigger, kRightTrigger, etc.</summary>
		/// <returns>Returns the type of axis input this is.</returns>
		Rtt::InputAxisType GetType() const;

		/// <summary>Sets the type of axis such as kX or kY for a joystick.</summary>
		/// <param name="type">The type of axis to set this to. Set to kUnknown if not known.</param>
		void SetType(const Rtt::InputAxisType& value);

		/// <summary>Gets the minimum value that this axis input can provide.</summary>
		/// <returns>Returns the minimum value that can be provided.</returns>
		int32_t GetMinValue() const;

		/// <summary>Sets the minimum value that this axis input can provide.</summary>
		/// <param name="value">The minimum value that can be provided. Typically 0 or -32768.</param>
		void SetMinValue(int32_t value);

		/// <summary>Gets the maximum value that this axis input can provide.</summary>
		/// <returns>Returns the maximum value that can be provided.</returns>
		int32_t GetMaxValue() const;

		/// <summary>Sets the maximum value that this axis input can provide.</summary>
		/// <param name="value">The maximum value that can be provided. Typically 255 or 32767.</param>
		void SetMaxValue(int32_t value);

		/// <summary>Gets the +/- accuracy of the values this axis input provides.</summary>
		/// <returns>
		///  <para>Returns the accuracy of this axis input's values. Will be greater than or equal to zero.</para>
		///  <para>
		///   A value of zero indicates perfect accuracy, but more likely indicates that the accuracy
		///   of the input device is unknown.
		///  </para>
		/// </returns>
		float GetAccuracy() const;

		/// <summary>Sets the +/- accuracy of the values this axis input provides.</summary>
		/// <param name="value">
		///  <para>The accuracy of the input. Must be greater than or equal to zero.</para>
		///  <para>Zero indicates perfect accuracy.</para>
		/// </param>
		void SetAccuracy(float value);

		/// <summary>Determines if this axis input provides absolute values or relative values.</summary>
		/// <returns>
		///  <para>
		///   Returns true if the axis provides absolute values, meaning that it is positional data.
		///   This is typically used by joysticks, indicating their currently held position.
		///  </para>
		///  <para>
		///   Returns false if the axis provides relative values, which is typically the distance travelled
		///   since the last axis input event. This is typically used by mice and trackpads, indicating
		///   how far the input device has moved since it's last input event.
		///  </para>
		/// </returns>
		bool IsAbsolute() const;

		/// <summary>Sets whether or not this axis input provides absolute values or relative values.</summary>
		/// <param name="value">
		///  <para>
		///   Set true if the axis provides absolute values, which is positional data.
		///   This is typically used by joysticks, indicating their currently held position.
		///  </para>
		///  <para>
		///   Set false if the axis provides relative values, which is typically the distance travelled
		///   since the last received axis input event. This is typically used by mice and trackpads,
		///   indicating how far the input device has move since its last input event.
		///  </para>
		/// </param>
		void SetIsAbsolute(bool value);

		/// <summary>Determines if this axis configuration matches the given axis configuration.</summary>
		/// <param name="value">Reference to the axis configuration to compare against.</param>
		/// <returns>Returns true if the axis configurations match. Returns false if not.</returns>
		bool Equals(const InputAxisSettings& value) const;

		/// <summary>Determines if this axis configuration does not match the given axis configuration.</summary>
		/// <param name="value">Reference to the axis configuration to compare against.</param>
		/// <returns>Returns true if the axis configurations do not match. Returns false if they do match.</returns>
		bool NotEquals(const InputAxisSettings& value) const;

		/// <summary>Determines if this axis configuration matches the given axis configuration.</summary>
		/// <param name="value">Reference to the axis configuration to compare against.</param>
		/// <returns>Returns true if the axis configurations match. Returns false if not.</returns>
		bool operator==(const InputAxisSettings& value) const;

		/// <summary>Determines if this axis configuration does not match the given axis configuration.</summary>
		/// <param name="value">Reference to the axis configuration to compare against.</param>
		/// <returns>Returns true if the axis configurations do not match. Returns false if they do match.</returns>
		bool operator!=(const InputAxisSettings& value) const;

		#pragma endregion

	private:
		#pragma region Private Member Variables
		/// <summary>The type of axis such as kX, kY, kLeftTrigger, kRightTrigger, etc.</summary>
		Rtt::InputAxisType fType;

		/// <summary>The minimum value that the axis input can provide.</summary>
		int32_t fMinValue;

		/// <summary>The maximum value that the axis input can provide.</summary>
		int32_t fMaxValue;

		/// <summary>The +/- accuracy of an axis value relative to its min/max range.</summary>
		float fAccuracy;

		/// <summary>
		///  <para>Set true if the axis provides absolute values, like a joystick.</para>
		///  <para>Set false if the axis provides relative values, like a mouse.</para>
		/// </summary>
		bool fIsAbsolute;

		#pragma endregion
};

} }	// namespace Interop::Input
