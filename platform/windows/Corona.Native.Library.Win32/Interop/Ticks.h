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

#include <cstdint>
#include <functional>
#include <Windows.h>


namespace Interop {

/// <summary>
///  <para>Stores system ticks retrieved via the Win32 GetTickCount() API.</para>
///  <para>This is a constantly increasing number, measured in milliseconds, since OS system startup.</para>
///  <para>Ticks are unaffected by system clock changes, makin it ideal for measuring time.</para>
///  <para>
///   The raw ticks value will eventually overflow after 49.7 days, but this class will correctly handling
///   Ticks comparisons when overflow occurs.
///  </para>
///  <para>Instances of this class can be passed by value efficiently.</para>
/// </summary>
class Ticks
{
	private:
		/// <summary>Creates a new ticks object initialized with a raw tick count integer value.</summary>
		/// <param name="rawValue">Raw integer value expected to come from the Win32 GetTickCount() function.</param>
		Ticks(int32_t rawValue);

	public:
		/// <summary>Creates a new ticks object initialized to a raw value of zero.</summary>
		Ticks();

		/// <summary>Destroys this object.</summary>
		virtual ~Ticks();

		/// <summary>Adds the given milliseconds to this object's tick count.</summary>
		/// <param name="value">The number of milliseconds to be added. Can be negative.</param>
		/// <returns>Returns a reference to itself with the updated time.</returns>
		Ticks& AddMilliseconds(int value);

		/// <summary>Adds the given seconds to this object's tick count.</summary>
		/// <param name="value">The number of seconds to be added. Can be negative.</param>
		/// <returns>Returns a reference to itself with the updated time.</returns>
		Ticks& AddSeconds(int value);

		/// <summary>
		///  <para>Gets the raw integer count as used by the Win32 APIs, such as the Win32 GetTickCount() function.</para>
		///  <para>
		///   This is an unsigned integer, which can overflow, that is the number of milliseconds since the OS started up.
		///  </para>
		/// </summary>
		/// <returns>Returns the raw integer tick count as used by the Win32 APIs.</returns>
		DWORD ToRawValue() const;

		/// <summary>Determines if this ticks value matches the given ticks.</summary>
		/// <param name="value">Reference to the ticks value to compare against.</param>
		/// <returns>Returns true if the objects match. Returns false if not.</returns>
		bool Equals(const Ticks& value) const;

		/// <summary>Determines if this ticks value does not match the given ticks.</summary>
		/// <param name="value">Reference to the ticks value to compare against.</param>
		/// <returns>Returns true if the objects do not match. Returns false if they do match.</returns>
		bool NotEquals(const Ticks& value) const;

		/// <summary>Determines if this ticks value matches the given ticks.</summary>
		/// <param name="value">Reference to the ticks value to compare against.</param>
		/// <returns>Returns true if the objects match. Returns false if not.</returns>
		bool operator==(const Ticks& value) const;

		/// <summary>Determines if this ticks value does not match the given ticks.</summary>
		/// <param name="value">Reference to the ticks value to compare against.</param>
		/// <returns>Returns true if the objects do not match. Returns false if they do match.</returns>
		bool operator!=(const Ticks& value) const;

		/// <summary>Compares this ticks object with the given ticks object.</summary>
		/// <param name="value">The object to be compared with this object.</param>
		/// <returns>
		///  <para>Returns a positive number if this object is greater than the given object.</para>
		///  <para>Returns zero if this object equals the given object.</para>
		///  <para>Returns a negative number if this object is less than the given object.</para>
		/// </returns>
		int CompareTo(const Ticks& value) const;

		/// <summary>Determines if this ticks object is greater than the given ticks object.</summary>
		/// <param name="value">The object to be compared with this object.</param>
		/// <returns>Returns true if this object is greater than the given object. Returns false if not.</returns>
		bool operator>(const Ticks& value) const;

		/// <summary>Determines if this ticks object is greater than or equal to the given ticks object.</summary>
		/// <param name="value">The object to be compared with this object.</param>
		/// <returns>
		///  Returns true if this object is greater than or equal to the given object. Returns false if not.
		/// </returns>
		bool operator>=(const Ticks& value) const;

		/// <summary>Determines if this ticks object is less than the given ticks object.</summary>
		/// <param name="value">The object to be compared with this object.</param>
		/// <returns>Returns true if this object is less than the given object. Returns false if not.</returns>
		bool operator<(const Ticks& value) const;

		/// <summary>Determines if this ticks object is less than or equal to the given ticks object.</summary>
		/// <param name="value">The object to be compared with this object.</param>
		/// <returns>
		///  Returns true if this object is less than or equal to the given object. Returns false if not.
		/// </returns>
		bool operator<=(const Ticks& value) const;

		/// <summary>
		///  Creates a new object set to the current ticks time of the system via the Win32 GetTickCount() function.
		/// </summary>
		/// <returns>Returns a new ticks object set to the system current tick time.</returns>
		static Ticks FromCurrentTime();

		/// <summary>
		///  Creates a new ticks object using the raw DWORD integer value that normally comes from the
		///  Win32 GetTickCount() function.
		/// </summary>
		/// <param name="value">The raw integer tick count to initialize the Ticks object to.</param>
		/// <returns>Returns a new ticks object set to the given raw tick count integer value.</returns>
		static Ticks FromRawValue(DWORD value);

	private:
		/// <summary>The raw Win32 tick value converted from an unsigned "DWORD" to a signed 32-bit integer.</summary>
		int32_t fValue;
};

}	// namespace Interop


namespace std {

#pragma region std::hash<Ticks> Template Specialization
template<>
/// <summary>
///  <para>Template specialization of "std::hash" for the Ticks class.</para>
///  <para>Provides a reasonably unique hash code for the a Ticks object.</para>
///  <para>
///   Intended to be used by STL collections that store Ticks objects in a hash table such as
///   the std::unordered_map, std::unordered_set, etc.
///  </para>
/// </summary>
struct hash<Interop::Ticks>
{
	/// <summary>Gets a reasonably unique hash code for the given object.</summary>
	/// <param name="value">The object to calculate a hash code for.</param>
	/// <returns>Returns a reasonably unique hash code for the given object.</returns>
	size_t operator()(const Interop::Ticks& value) const
	{
		return (size_t)value.ToRawValue();
	}
};

#pragma endregion

}	// namespace std
