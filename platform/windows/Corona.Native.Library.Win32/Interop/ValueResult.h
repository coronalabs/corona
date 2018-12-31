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

#include "OperationResult.h"


namespace Interop {

template<class T>
/// <summary>Indicates if an operation has succeeded in producing a value.</summary>
class ValueResult : public OperationResult
{
	protected:
		/// <summary>Creates a new result object with the given information.</summary>
		/// <remarks>
		///  This is a protected constructor that is only expected to be called by this class' static
		///  SucceededWith() and FailedWith() functions or by derived versions of this class.
		/// </remarks>
		/// <param name="hasSucceeded">Set to true if the operation succeeded. Set to false if failed.</param>
		/// <param name="message">
		///  Message providing details about the final results of the operation. If the operation failed,
		///  then this message is expected to explain why. Can be set to null or empty string.
		/// </param>
		/// <param name="value">The value to be returned by the GetValue() method.</param>
		ValueResult(bool hasSucceeded, const char *message, T value)
		:	OperationResult(hasSucceeded, message),
			fValue(value)
		{
		}

		/// <summary>Creates a new result object with the given information.</summary>
		/// <remarks>
		///  This is a protected constructor that is only expected to be called by this class' static
		///  SucceededWith() and FailedWith() functions or by derived versions of this class.
		/// </remarks>
		/// <param name="hasSucceeded">Set to true if the operation succeeded. Set to false if failed.</param>
		/// <param name="message">
		///  Message providing details about the final results of the operation. If the operation failed,
		///  then this message is expected to explain why. Can be set to null or empty string.
		/// </param>
		/// <param name="value">The value to be returned by the GetValue() method.</param>
		ValueResult(bool hasSucceeded, const wchar_t *message, T value)
		:	OperationResult(hasSucceeded, message),
			fValue(value)
		{
		}

		/// <summary>Creates a new result object with the given information.</summary>
		/// <remarks>
		///  This is a protected constructor that is only expected to be called by this class' static
		///  SucceededWith() and FailedWith() functions or by derived versions of this class.
		/// </remarks>
		/// <param name="hasSucceeded">Set to true if the operation succeeded. Set to false if failed.</param>
		/// <param name="message">
		///  Message providing details about the final results of the operation. If the operation failed,
		///  then this message is expected to explain why. Can be set to null or empty string.
		/// </param>
		/// <param name="value">The value to be returned by the GetValue() method.</param>
		ValueResult(bool hasSucceeded, const std::shared_ptr<const std::wstring> &message, T value)
		:	OperationResult(hasSucceeded, message),
			fValue(value)
		{
		}

	public:
		/// <summary>Destroys this object.</summary>
		virtual ~ValueResult() {}

		/// <summary>Gets the value that was produced by the operation, if successful.</summary>
		/// <returns>Returns the value produced by the operation.</returns>
		T GetValue() const
		{
			return fValue;
		}

		/// <summary>Determines if this result matches the given result.</summary>
		/// <param name="result">Reference to the result object to compare against.</param>
		/// <returns>Returns true if the objects match. Returns false if not.</returns>
		bool operator==(const ValueResult<T> &result) const
		{
			if (OperationResult::operator!=(result))
			{
				return false;
			}
			return (fValue == result.fValue);
		}

		/// <summary>Determines if this result does not match the given result.</summary>
		/// <param name="result">Reference to the result object to compare against.</param>
		/// <returns>Returns true if the objects do not match. Returns false if they do match.</returns>
		bool operator!=(const ValueResult<T> &result) const
		{
			return !(*this == result);
		}

		/// <summary>Creates a new success result object providing the given value.</summary>
		/// <param name="value">Value to be returned by the result object's GetValue() method.</param>
		/// <returns>Returns a new success result object with the given info.</returns>
		static ValueResult<T> SucceededWith(T value)
		{
			return ValueResult<T>(true, (const wchar_t*)nullptr, value);
		}

		/// <summary>Creates a new result object set to failed and with the given message.</summary>
		/// <param name="message">Message explaining why the operation failed. Can be null or empty.</param>
		/// <returns>Returns a new result object set to failed and with the given message.</returns>
		static ValueResult<T> FailedWith(const char *message)
		{
			return ValueResult<T>(false, message, T());
		}

		/// <summary>Creates a new result object set to failed and with the given message.</summary>
		/// <param name="message">Message explaining why the operation failed. Can be null or empty.</param>
		/// <returns>Returns a new result object set to failed and with the given message.</returns>
		static ValueResult<T> FailedWith(const wchar_t *message)
		{
			return ValueResult<T>(false, message, T());
		}

		/// <summary>Creates a new result object set to failed and with the given message.</summary>
		/// <param name="message">Message explaining why the operation failed. Can be null or empty.</param>
		/// <returns>Returns a new result object set to failed and with the given message.</returns>
		static ValueResult<T> FailedWith(const std::shared_ptr<const std::wstring> &message)
		{
			return ValueResult<T>(false, message, T());
		}

	private:
		T fValue;
};

}	// namespace Interop
