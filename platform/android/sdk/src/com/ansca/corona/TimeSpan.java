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

package com.ansca.corona;


/**
 * Stores a short range of time providing easy access to its milliseconds, seconds, minutes, hours, and days.
 * Supports ranges between milliseconds and days. Supports negative time ranges as well.
 * <p>
 * Instances of this class are immutable.
 */
public class TimeSpan implements Comparable {
	/** The time span reprsented in total milliseconds. */
	private long fMilliseconds;
	
	
	/**
	 * Creates a new TimeSpan object set to the given time values.
	 * @param days The number of days.
	 * @param hours The number of hours. Can be greater than 23. Can be positive or negative.
	 * @param minutes The number of minutes. Can be greater than 59. Can be positive or negative.
	 * @param seconds The number of seconds. Can be greater than 59. Can be positive or negative.
	 * @param milliseconds The number of milliseconds. Can be greater than 999. Can be positive or negative.
	 */
	public TimeSpan(long days, long hours, long minutes, long seconds, long milliseconds) {
		fMilliseconds = milliseconds;
		fMilliseconds += seconds * 1000L;
		fMilliseconds += minutes * 60000L;	// = 1000 * 60
		fMilliseconds += hours * 3600000L;	// = 1000 * 60 * 60
		fMilliseconds += days * 86400000L;	// = 1000 * 60 * 60 * 24
	}
	
	/**
	 * Gets the days component of the time span.
	 * @return Returns the days component of the time span.
	 */
	public int getDays() {
		return (int)(fMilliseconds / 86400000L);
	}
	
	/**
	 * Gets the hours component of the time span.
	 * @return Returns the hours component of the time span, ranging between 0 and 23.
	 */
	public int getHours() {
		return (int)(fMilliseconds / 3600000L) % 24;
	}
	
	/**
	 * Gets the minutes component of the time span.
	 * @return Returns the minutes component of the time span, ranging between 0 and 59.
	 */
	public int getMinutes() {
		return (int)(fMilliseconds / 60000L) % 60;
	}
	
	/**
	 * Gets the seconds component of the time span.
	 * @return Returns the seconds component of the time span, ranging between 0 and 59.
	 */
	public int getSeconds() {
		return (int)(fMilliseconds / 1000L) % 60;
	}
	
	/**
	 * Gets the milliseconds component of the time span.
	 * @return Returns the milliseconds component of the time span, ranging between 0 and 999.
	 */
	public int getMilliseconds() {
		return (int)(fMilliseconds % 1000);
	}
	
	/**
	 * Gets the total number of days in the entire time span, including fractional days.
	 * @return Returns the total number of days.
	 */
	public double getTotalDays() {
		return (double)fMilliseconds / 86400000.0;
	}
	
	/**
	 * Gets the total number of hours in the time span, including fractional hours.
	 * @return Returns the total number of hours. Can be greater than 23. 
	 */
	public double getTotalHours() {
		return (double)fMilliseconds / 3600000.0;
	}
	
	/**
	 * Gets the total number of minutes in the time span, including fractional minutes.
	 * @return Returns the total number of minutes. Can be greater than 59. 
	 */
	public double getTotalMinutes() {
		return (double)fMilliseconds / 60000.0;
	}
	
	/**
	 * Gets the total number of seconds in the time span, including fractional seconds.
	 * @return Returns the total number of seconds. Can be greater than 59. 
	 */
	public double getTotalSeconds() {
		return (double)fMilliseconds / 1000.0;
	}
	
	/**
	 * Gets the total number of milliseconds in the time span.
	 * @return Returns the total number of milliseconds. Can be greater than 999. 
	 */
	public long getTotalMilliseconds() {
		return fMilliseconds;
	}
	
	/**
	 * Determines if the given time span values match.
	 * @param value The time span object to be compare with this instance.
	 * @return Returns true if time spans equal. Returns false if not or if given a null object.
	 */
	public boolean equals(TimeSpan value) {
		if (value == null) {
			return false;
		}
		return (fMilliseconds == value.fMilliseconds);
	}
	
	/**
	 * Determines if the given object matches this object.
	 * @param value The time span object to be compare with this instance.
	 * @return Returns true if time spans equal.
	 *         Returns false if they do not equal, if given null, or if given object is not of type TimeSpan.
	 */
	@Override
	public boolean equals(Object value) {
		if ((value instanceof TimeSpan) == false) {
			return false;
		}
		return equals((TimeSpan)value);
	}
	
	/**
	 * Compares this time span with the given time span.
	 * @param value The time span to be compared with this object.
	 * @return Returns a positive number if this object is greater than the given object.
	 *         Returns zero if this object equals the given object.
	 *         Returns a negative number if this object is less than the given object.
	 */
	public int compareTo(TimeSpan value) {
		// Return "greater" if given a null object.
		if (value == null) {
			return 1;
		}
		
		// Compare time spans.
		long result = fMilliseconds - value.fMilliseconds;
		if (result < 0) {
			return -1;				// This object is lesser.
		}
		else if (result == 0) {
			return 0;				// Objects are equal.
		}
		return 1;					// This object is greater.
	}
	
	/**
	 * Compares this time span with the given object.
	 * @param value The object to be compared with this object.
	 * @return Returns a positive number if this object is greater than the given object.
	 *         Returns zero if this object equals the given object.
	 *         Returns a negative number if this object is less than the given object.
	 */
	@Override
	public int compareTo(Object value) {
		// Return "greater" if given a null or invalid object.
		if ((value instanceof TimeSpan) == false) {
			return 1;
		}
		
		// Compare objects.
		return compareTo((TimeSpan)value);
	}
	
	@Override
	/**
	 * Gets an integer hash code for this object.
	 * @return Returns this object's hash code.
	 */
	public int hashCode() {
		return (int)(fMilliseconds ^ (fMilliseconds >>> 32));
	}
	
	/**
	 * Creates a new time span object from the given milliseconds.
	 * @param value The number of milliseconds to set the time span to. Can be greater than 999. Can be negative.
	 * @return Returns a new TimeSpan object set to the given number of milliseconds.
	 */
	public static TimeSpan fromMilliseconds(long value) {
		return new TimeSpan(0, 0, 0, 0, value);
	}
	
	/**
	 * Creates a new time span object from the given seconds.
	 * @param value The number of seconds to set the time span to. Can be greater than 59. Can be negative.
	 * @return Returns a new TimeSpan object set to the given number of seconds.
	 */
	public static TimeSpan fromSeconds(long value) {
		return new TimeSpan(0, 0, 0, value, 0);
	}
	
	/**
	 * Creates a new time span object from the given minutes.
	 * @param value The number of minutes to set the time span to. Can be greater than 59. Can be negative.
	 * @return Returns a new TimeSpan object set to the given number of minutes.
	 */
	public static TimeSpan fromMinutes(long value) {
		return new TimeSpan(0, 0, value, 0, 0);
	}
}
