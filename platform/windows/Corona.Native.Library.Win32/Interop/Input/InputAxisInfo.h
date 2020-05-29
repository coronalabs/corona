//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "InputAxisSettings.h"


namespace Interop { namespace Input {

/// <summary>
///  <para>Provides read-only information about one axis input belonging to an input device.</para>
///  <para>Instances of this class are immutable.</para>
/// </summary>
class InputAxisInfo
{
	public:
		#pragma region Constructors/Destructors
		/// <summary>Creates an immutable object providing a copy of the given axis information.</summary>
		/// <param name="settings">The axis settings to be copied.</param>
		InputAxisInfo(const InputAxisSettings& settings);

		/// <summary>Destroys this object.</summary>
		virtual ~InputAxisInfo();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Gets the type of axis such as kX, kY, kLeftTrigger, kRightTrigger, etc.</summary>
		/// <returns>Returns the type of axis input this is.</returns>
		Rtt::InputAxisType GetType() const;

		/// <summary>Gets the minimum value that this axis input can provide.</summary>
		/// <returns>Returns the minimum value that can be provided.</returns>
		int32_t GetMinValue() const;

		/// <summary>Gets the maximum value that this axis input can provide.</summary>
		/// <returns>Returns the maximum value that can be provided.</returns>
		int32_t GetMaxValue() const;

		/// <summary>Gets the +/- accuracy of the values this axis input provides.</summary>
		/// <returns>
		///  <para>Returns the accuracy of this axis input's values. Will be greater than or equal to zero.</para>
		///  <para>
		///   A value of zero indicates perfect accuracy, but more likely indicates that the accuracy
		///   of the input device is unknown.
		///  </para>
		/// </returns>
		float GetAccuracy() const;

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

		/// <summary>Determines if this axis configuration matches the given axis configuration.</summary>
		/// <param name="value">Reference to the axis configuration to compare against.</param>
		/// <returns>Returns true if the axis configurations match. Returns false if not.</returns>
		bool Equals(const InputAxisInfo& value) const;

		/// <summary>Determines if this axis configuration matches the given axis configuration.</summary>
		/// <param name="value">Reference to the axis configuration to compare against.</param>
		/// <returns>Returns true if the axis configurations match. Returns false if not.</returns>
		bool Equals(const InputAxisSettings& value) const;

		/// <summary>Determines if this axis configuration does not match the given axis configuration.</summary>
		/// <param name="value">Reference to the axis configuration to compare against.</param>
		/// <returns>Returns true if the axis configurations do not match. Returns false if they do match.</returns>
		bool NotEquals(const InputAxisInfo& value) const;

		/// <summary>Determines if this axis configuration does not match the given axis configuration.</summary>
		/// <param name="value">Reference to the axis configuration to compare against.</param>
		/// <returns>Returns true if the axis configurations do not match. Returns false if they do match.</returns>
		bool NotEquals(const InputAxisSettings& value) const;

		/// <summary>Determines if this axis configuration matches the given axis configuration.</summary>
		/// <param name="value">Reference to the axis configuration to compare against.</param>
		/// <returns>Returns true if the axis configurations match. Returns false if not.</returns>
		bool operator==(const InputAxisInfo& value) const;

		/// <summary>Determines if this axis configuration matches the given axis configuration.</summary>
		/// <param name="value">Reference to the axis configuration to compare against.</param>
		/// <returns>Returns true if the axis configurations match. Returns false if not.</returns>
		bool operator==(const InputAxisSettings& value) const;

		/// <summary>Determines if this axis configuration does not match the given axis configuration.</summary>
		/// <param name="value">Reference to the axis configuration to compare against.</param>
		/// <returns>Returns true if the axis configurations do not match. Returns false if they do match.</returns>
		bool operator!=(const InputAxisInfo& value) const;

		/// <summary>Determines if this axis configuration does not match the given axis configuration.</summary>
		/// <param name="value">Reference to the axis configuration to compare against.</param>
		/// <returns>Returns true if the axis configurations do not match. Returns false if they do match.</returns>
		bool operator!=(const InputAxisSettings& value) const;

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>Assignment operator made private to make this object immutable.</summary>
		void operator=(const InputAxisInfo& info) {}

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>A copy of the axis settings wrapped by this immutable object.</summary>
		InputAxisSettings fSettings;

		#pragma endregion
};

} }	// namespace Interop::Input
