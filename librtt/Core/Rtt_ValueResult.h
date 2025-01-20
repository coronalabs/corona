//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_ValueResult_H__
#define __Rtt_ValueResult_H__

#include "Rtt_OperationResult.h"


namespace Rtt
{

template<class T>
/**
  Indicates if an operation has succeeded in producing a value.

  Instances of this class are expected to be passed by value.
 */
class ValueResult : public OperationResult
{
	protected:
		/**
		  Creates a new result object with the given information.

		  This is a protected constructor that is only expected to be called by this class' static
		  SucceededWith() and FailedWith() functions or by derived versions of this class.
		  @param hasSucceeded Set to true if the operation succeeded. Set to false if failed.
		  @param utf8Message Message to be copied by this object. Provides details about the operation's final result.
		                     If the operation failed, then this message is expected to explain why.
		                     Can be set to null or empty string.
		  @param value The value to be returned by the GetValue() method.
		 */
		ValueResult(bool hasSucceeded, const char* utf8Message, T value)
		:	OperationResult(hasSucceeded, utf8Message),
			fValue(value)
		{
		}

		/**
		  Creates a new result object with the given information.

		  This is a protected constructor that is only expected to be called by this class' static
		  SucceededWith() and FailedWith() functions or by derived versions of this class.
		  @param hasSucceeded Set to true if the operation succeeded. Set to false if failed.
		  @param utf8Message Message providing details about the final results of the operation.
		                     If the operation failed, then this message is expected to explain why.
		                     Can be set to null or empty string.
		  @param value The value to be returned by the GetValue() method.
		 */
		ValueResult(bool hasSucceeded, const Rtt::SharedConstStdStringPtr& utf8Message, T value)
		:	OperationResult(hasSucceeded, utf8Message),
			fValue(value)
		{
		}

	public:
		/** Destroys this object. */
		virtual ~ValueResult() {}

		/**
		  Gets the value that was produced by the operation, if successful.
		  @return Returns the value produced by the operation.
		 */
		T GetValue() const
		{
			return fValue;
		}

		/**
		  Creates a new success result object providing the given value.
		  @param value Value to be returned by the result object's GetValue() method.
		  @return Returns a new success result object with the given info.
		 */
		static ValueResult<T> SucceededWith(T value)
		{
			return ValueResult<T>(true, (const char*)NULL, value);
		}

		/**
		  Creates a new result object set to failed and with the given message.
		  @param utf8Message Message explaining why the operation failed. Can be null or empty.
		                     This message is copied by the returned object.
		  @return Returns a new result object set to failed and with the given message.
		 */
		static ValueResult<T> FailedWith(const char* utf8Message)
		{
			return ValueResult<T>(false, utf8Message, T());
		}

		/**
		  Creates a new result object set to failed and with the given message.
		  @param utf8Message Message explaining why the operation failed. Can be null or empty.
		  @return Returns a new result object set to failed and with the given message.
		 */
		static ValueResult<T> FailedWith(const Rtt::SharedConstStdStringPtr& utf8Message)
		{
			return ValueResult<T>(false, utf8Message, T());
		}

	private:
		T fValue;
};

}	// namespace Rtt

#endif	// __Rtt_ValueResult_H__
