//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "EventArgs.h"


namespace Interop {

template<class T>
/// <summary>
///  <para>Event arguments providing data for a value changing event.</para>
///  <para>Provides the old value and the new value that it was changed to.</para>
/// </summary>
class ValueChangedEventArgs : public EventArgs
{
	public:
		/// <summary>Creates a new event arguments object with the given values.</summary>
		/// <param name="oldValue">The old value before the change.</param>
		/// <param name="newValue">The new value that is replacing the old value.</param>
		ValueChangedEventArgs(T oldValue, T newValue)
		:	EventArgs(),
			fOldValue(oldValue),
			fNewValue(newValue)
		{
		}

		/// <summary>Destroys this object.</summary>
		virtual ~ValueChangedEventArgs() {}

		/// <summary>Gets the old/previous value.</summary>
		/// <returns>Returns the old value.</returns>
		T GetOldValue() const
		{
			return fOldValue;
		}

		/// <summary>Gets the new value that has replaced the old value.</summary>
		/// <returns>Returns the new value.</returns>
		T GetNewValue() const
		{
			return fNewValue;
		}

	private:
		/// <summary>The old/previous value before the change.</summary>
		T fOldValue;

		/// <summary>The new value that has replaced the old value.</summary>
		T fNewValue;
};

}	// namespace Interop
