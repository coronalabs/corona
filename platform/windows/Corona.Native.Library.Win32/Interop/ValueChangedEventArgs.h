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
