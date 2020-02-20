//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif


#include "ICoronaBoxedData.h"


namespace CoronaLabs { namespace Corona { namespace WinRT {

/// <summary>
///  <para>Object which stores a single value that can be transferred to/from Lua.</para>
///  <para>Can perform Equals() and CompareTo() checks with other comparable objects and can be used as a key in a table.</para>
///  <para>Instances of this interface are immutable and can be used as a key in a hash table or dictionary.</para>
/// </summary>
public interface class ICoronaBoxedComparableData
:	public ICoronaBoxedData
{
	/// <summary>Determines if this object matches the given object.</summary>
	/// <param name="value">The object to compare with.</param>
	/// <returns>
	///  <para>Returns true if the given object's value matches this object's value.</para>
	///  <para>Returns false if they do not equal, if given null, or if the given object is of a different type.</para>
	/// </returns>
	bool Equals(Platform::Object^ value);

	/// <summary>Determines if this object is less than, greater than, or equal to the given object.</summary>
	/// <param name="value">The object to be compared with.</param>
	/// <returns>
	///  <para>Returns a positive value if this object is greater than the given object.</para>
	///  <para>Returns a negative value if this object is less than the given object.</para>
	///  <para>Returns zero if the both objects are equal.</para>
	/// </returns>
	int CompareTo(Platform::Object^ value);

	/// <summary>Gets an integer hash code for this object.</summary>
	/// <returns>Returns this object's hash code.</returns>
	int GetHashCode();
};

} } }	// namespace CoronaLabs::Corona::WinRT
