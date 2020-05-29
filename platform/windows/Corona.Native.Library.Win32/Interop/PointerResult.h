//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "OperationResult.h"


namespace Interop {

template<class TClass>
/// <summary>Indicates if an operation has succeeded in producing a pointer.</summary>
class PointerResult : public OperationResult
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
		/// <param name="pointer">Pointer to be provided by the result object's GetPointer() method. Can be null.</param>
		PointerResult(bool hasSucceeded, const char *message, TClass *pointer)
		:	OperationResult(hasSucceeded, message),
			fPointer(pointer)
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
		/// <param name="pointer">Pointer to be provided by the result object's GetPointer() method. Can be null.</param>
		PointerResult(bool hasSucceeded, const wchar_t *message, TClass *pointer)
		:	OperationResult(hasSucceeded, message),
			fPointer(pointer)
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
		/// <param name="pointer">Pointer to be provided by the result object's GetPointer() method. Can be null.</param>
		PointerResult(bool hasSucceeded, const std::shared_ptr<const std::wstring> &message, TClass *pointer)
		:	OperationResult(hasSucceeded, message),
			fPointer(pointer)
		{
		}

	public:
		/// <summary>Destroys this object.</summary>
		virtual ~PointerResult() {}

		/// <summary>Gets the pointer that was produced by the operation.</summary>
		/// <returns>
		///  <para>Returns a pointer if the operation succeeded.</para>
		///  <para>Returns null if the operation failed to produce a pointer.</para>
		/// </returns>
		TClass* GetPointer() const
		{
			return fPointer;
		}

		/// <summary>Determines if this result matches the given result.</summary>
		/// <param name="result">Reference to the result object to compare against.</param>
		/// <returns>Returns true if the objects match. Returns false if not.</returns>
		bool operator==(const PointerResult<TClass> &result) const
		{
			if (OperationResult::operator!=(result))
			{
				return false;
			}
			return (fPointer == result.fPointer);
		}

		/// <summary>Determines if this result does not match the given result.</summary>
		/// <param name="result">Reference to the result object to compare against.</param>
		/// <returns>Returns true if the objects do not match. Returns false if they do match.</returns>
		bool operator!=(const PointerResult<TClass> &result) const
		{
			return !(*this == result);
		}

		/// <summary>Creates a new success result object providing the given pointer.</summary>
		/// <param name="pointer">Pointer to be returned by the result object's GetPointer() method.</param>
		/// <returns>Returns a new success result object with the given info.</returns>
		static PointerResult<TClass> SucceededWith(TClass *pointer)
		{
			return PointerResult<TClass>(true, (const wchar_t*)nullptr, pointer);
		}

		/// <summary>Creates a new result object set to failed and with the given message.</summary>
		/// <param name="message">Message explaining why the operation failed. Can be null or empty.</param>
		/// <returns>Returns a new result object set to failed and with the given message.</returns>
		static PointerResult<TClass> FailedWith(const char *message)
		{
			return PointerResult<TClass>(false, message, nullptr);
		}

		/// <summary>Creates a new result object set to failed and with the given message.</summary>
		/// <param name="message">Message explaining why the operation failed. Can be null or empty.</param>
		/// <returns>Returns a new result object set to failed and with the given message.</returns>
		static PointerResult<TClass> FailedWith(const wchar_t *message)
		{
			return PointerResult<TClass>(false, message, nullptr);
		}

		/// <summary>Creates a new result object set to failed and with the given message.</summary>
		/// <param name="message">Message explaining why the operation failed. Can be null or empty.</param>
		/// <returns>Returns a new result object set to failed and with the given message.</returns>
		static PointerResult<TClass> FailedWith(const std::shared_ptr<const std::wstring> &message)
		{
			return PointerResult<TClass>(false, message, nullptr);
		}

	private:
		/// <summary>The pointer produced by the operation or null if the operation failed.</summary>
		TClass* fPointer;
};

}	// namespace Interop
