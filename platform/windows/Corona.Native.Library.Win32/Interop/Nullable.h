//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Assert.h"


namespace Interop {

template<class T>
/// <summary>
///  <para>
///   Nullable type used to box/store a value type's (ie: bool, int, float, struct, etc.) data
///   and in addition can also be set to nullptr.
///  </para>
///  <para>Works very similarly to .NET's "System.Nullable&lt;T&gt;" class.</para>
///  <para>Also works like Java's primitive boxing classes such as Boolean, Integer, Float, etc.</para>
///  <para>Value Assignment Example:  Nullable&lt;int&gt; nullableInt = 123;</para>
///  <para>Value Retrieval Example:  int value = (int)Nullable&lt;int&gt;(123);</para>
///  <para>Null Assignment Example:  Nullable&lt;int&gt; nullableInt = nullptr;</para>
/// </summary>
class Nullable
{
	public:
		/// <summary>Creates a new object initialized to null.</summary>
		Nullable()
		:	fHasValue(false),
			fValue()
		{
		}
		
		/// <summary>Creates a new object initialized with the given value.</summary>
		Nullable(const T& value)
		:	fHasValue(true),
			fValue(value)
		{
		}

		/// <summary>Destroys this object.</summary>
		virtual ~Nullable() {}

		/// <summary>Determines if this object does not reference a value.<summary>
		/// <returns>
		///  <para>Returns true if a value has not been assigned to this object.</para>
		///  <para>Returns false if this object is storing a value.</para>
		/// </returns>
		bool IsNull() const
		{
			return !fHasValue;
		}

		/// <summary>Determines if this object does reference a valid value.<summary>
		/// <returns>
		///  <para>Returns true if this object is storing a value.</para>
		///  <para>Returns false if a value has not been assigned to this object.</para>
		/// </returns>
		bool IsNotNull() const
		{
			return fHasValue;
		}

		/// <summary>
		///  <para>Assigns the given value to this object.</para>
		///  <para>This Nullable object will not be null after this operation.</para>
		/// </summary>
		/// <returns>Returns a reference to this nullable object, allowing assignments to be chained.</returns>
		Nullable<T>& operator=(const T& value)
		{
			fHasValue = true;
			fValue = value;
			return *this;
		}

		/// <summary>
		///  <para>Assigns the given pointer's value to this object.</para>
		///  <para>This object will be set to null if given a null pointer.</para>
		/// </summary>
		/// <param name="valuePointer">
		///  <para>Pointer to the value to be copied to this object.</para>
		///  <para>Set to nullptr to make this Nullable object null.</para>
		/// </param>
		/// <returns>Returns a reference to this nullable object, allowing assignments to be chained.</returns>
		Nullable<T>& operator=(const T* valuePointer)
		{
			if (valuePointer)
			{
				fHasValue = true;
				fValue = *valuePointer;
			}
			else
			{
				fHasValue = false;
			}
			return *this;
		}

		/// <summary>
		///  <para>Gets a copy of this Nullable object's stored value.</para>
		///  <para>Will assert if this object is null.</para>
		///  <para>You should call IsNull() or IsNotNull() before invoking this method.</para></para>
		/// </summary>
		/// <returns>Returns a copy of this object's value.</returns>
		operator T() const
		{
			Rtt_ASSERT(fHasValue);
			return fValue;
		}

		/// <summary>
		///  Gets a pointer to this Nullable object's stored value or returns a null pointer if this object is null.
		/// </summary>
		/// <returns>
		///  <para>Returns a pointer to this object's stored value if not not null.</para>
		///  <para>Returns a null pointer if this Nullable object is null (ie: not assigned a value).</para>
		/// </returns>
		explicit operator T*() const
		{
			return fHasValue ? &fValue : nullptr;
		}

		/// <summary>Determines if this object's value matches the given value or if they're both null.</summary>
		/// <param name="value">Reference to the object to compare against.</param>
		/// <returns>Returns true if the objects match. Returns false if not.</returns>
		bool operator==(const Nullable<T>& value) const
		{
			return (value.fHasValue == fHasValue) && (value.fValue == fValue);
		}

		/// <summary>Determines if this object's value matches the given value.</summary>
		/// <param name="value">Reference to the object to compare against.</param>
		/// <returns>Returns true if the objects match. Returns false if not.</returns>
		bool operator==(const T& value) const
		{
			return fHasValue && (value == fValue);
		}

		/// <summary>Determines if this object's value matches the given pointer's value.</summary>
		/// <param name="value">Pointer to the object to compare against. Can be null.</param>
		/// <returns>
		///  <para>Returns true if the values match or if they're both null.</para>
		///  <para>Returns false if values do not match or if one object is null and the other is not.</para>
		/// </returns>
		bool operator==(const T* valuePointer) const
		{
			if (!valuePointer)
			{
				return !fHasValue;
			}
			return fHasValue && (valuePointer == &fValue);
		}

		/// <summary>
		///  Determines if this object's value does not match the given value
		///  or if one object is null and the other is not.
		/// </summary>
		/// <param name="value">Reference to the object to compare against.</param>
		/// <returns>Returns true if the objects do not match. Returns false if they do.</returns>
		bool operator!=(const Nullable<T>& value) const
		{
			return !(*this == value);
		}

		/// <summary>Determines if this object's value does not match the given value.</summary>
		/// <param name="value">Reference to the object to compare against.</param>
		/// <returns>Returns true if the objects do not match. Returns false if they do.</returns>
		bool operator!=(const T& value) const
		{
			return !(*this == value);
		}

		/// <summary>Determines if this object's value does not match the given pointer's value.</summary>
		/// <param name="value">Pointer to the object to compare against. Can be null.</param>
		/// <returns>
		///  <para>Returns true if values do not match or if one object is null and the other is not.</para>
		///  <para>Returns false if the values match or if they're both null.</para>
		/// </returns>
		bool operator!=(const T* valuePointer) const
		{
			return !(*this == valuePointer);
		}

	private:
		/// <summary>Set to true if this object is not in the null state and "fValue" is storing a valid value.</summary>
		bool fHasValue;

		/// <summary>
		///  <para>The value this nullable object is storing/boxing.</para>
		///  <para>This member variable is invalid if "fHasValue" is set to false (in the null state).</para>
		/// </summary>
		T fValue;
};

}	// namespace Interop
