//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Windows.h>


namespace Interop {

/// <summary>
///  <para>Stores date and time with millisecond precision.</para>
///  <para>Provides static functions to easily fetch the current local or UTC system date/time.</para>
///  <para>Note: Functions to fetch current time are only accurate to 10-16ms on Windows 7 and older OS versions.</para>
/// </summary>
class DateTime
{
	private:
		/// <summary>Creates a new date time object initialized to the given values.</summary>
		/// <remarks>Constructor made private to force callers to use the static From*() functions.</remarks>
		/// <param name="systemTime">Win32 struct providing the date and time values.</param>
		/// <param name="isLocal">
		///  <para>Set true to indicate that the "systemTime" argument is in local time.</para>
		///  <para>Set false to indicate that the "systemTime" argument is in UTC time.</para>
		/// </param>
		DateTime(const SYSTEMTIME& systemTime, bool isLocal);

	public:
		/// <summary>Creates a new date time object initialized to current local time.</summary>
		DateTime();

		/// <summary>Destroys this object.</summary>
		virtual ~DateTime();

		/// <summary>Determines if the stored date time values are in local time, not UTC.</summary>
		/// <returns>Returns true if date time is stored in local time. Returns false if UTC.</returns>
		bool IsLocal() const;

		/// <summary>Determines if the stored date time values are in UTC time, not local time.</summary>
		/// <returns>Returns true if date time is stored in UTC time. Returns false if local.</returns>
		bool IsUtc() const;

		/// <summary>Gets this object's date time value as a Win32 "SYSTEMTIME" struct.</summary>
		/// <returns>Returns this object's date time value as a Win32 "SYSTEMTIME" struct.</returns>
		SYSTEMTIME ToSystemTime() const;

		/// <summary>Gets this object's date time value as a Win32 "FILETIME" struct.</summary>
		/// <returns>Returns this object's date time value as a Win32 "FILETIME" struct.</returns>
		FILETIME ToFileTime() const;

		/// <summary>Creates and returns a new DateTime object converted to local time.</summary>
		/// <returns>Returns a new DateTime object providing this object's timestamp in local time form.</returns>
		DateTime ToLocal() const;

		/// <summary>Creates and returns a new DateTime object converted to UTC time.</summary>
		/// <returns>Returns a new DateTime object providing this object's timestamp in UTC time form.</returns>
		DateTime ToUtc() const;

		/// <summary>Creates a new DateTime object set to the system clock's current local time.</summary>
		/// <returns>Returns a new DateTime object set to current local time.</returns>
		static DateTime FromCurrentLocal();

		/// <summary>Creates a new DateTime object set to the system clock's current UTC time.</summary>
		/// <returns>Returns a new DateTime object set to current UTC time.</returns>
		static DateTime FromCurrentUtc();

	private:
		/// <summary>Stores the date time values.</summary>
		SYSTEMTIME fSystemTime;

		/// <summary>Set true if "fSystemTime" is in local time. Set false if in UTC time.</summary>
		bool fIsLocal;
};

}	// namespace Interop
