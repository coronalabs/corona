//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_OperationResult_H__
#define __Rtt_OperationResult_H__

#include "Rtt_SharedStringPtr.h"


namespace Rtt
{

/**
  Indicates if an operation has succeeded or failed.

  Instances of this class are expected to be passed by value.
 */
class OperationResult
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
		 */
		OperationResult(bool hasSucceeded, const char* utf8Message);

		/**
		  Creates a new result object with the given information.

		  This is a protected constructor that is only expected to be called by this class' static
		  SucceededWith() and FailedWith() functions or by derived versions of this class.
		  @param hasSucceeded Set to true if the operation succeeded. Set to false if failed.
		  @param utf8Message Message providing details about the final results of the operation.
		                     If the operation failed, then this message is expected to explain why.
		                     Can be set to null or empty string.
		 */
		OperationResult(bool hasSucceeded, const Rtt::SharedConstStdStringPtr& utf8Message);

	public:
		/** Destroys this object. */
		virtual ~OperationResult();

		/**
		  Determines if the operation succeeded.
		  @return Returns true if the operation was executed successfully.

		          Returns false if the operation failed, in which case the GetUtf8Message() method will likely
		          provide details as to what went wrong.
		 */
		bool HasSucceeded() const;

		/**
		  Determines if the operation failed.
		  @return Returns true if the operation failed, in which case the GetUtf8Message() method will likely
		          provide details as to what went wrong.

		          Returns false if the operation was executed successfully.
		 */
		bool HasFailed() const;

		/**
		  Gets a UTF-8 encoded message providing details about the final result of the operation.
		  @return Returns a message providing details about the final result of the operation.
		          If the operation failed, then this message typically provides details indicating what went wrong.
		          Operations that are successful typically provide an empty message string.
		 */
		const char* GetUtf8Message() const;

		/**
		  Gets a shared pointer to this object's UTF-8 encoded message providing details about the final result
		  of the operation.
		  @return Returns a message providing details about the final result of the operation.
		          If the operation failed, then this message typically provides details indicating what went wrong.
		          Operations that are successful typically provide an empty message string.
		 */
		Rtt::SharedConstStdStringPtr GetUtf8MessageAsSharedPointer() const;

		/** Gets a reusable OperationResult object set to succeeded and with an empty message. */
		static const OperationResult kSucceeded;

		/**
		  Creates a new OperationResult object set to succeeded and with the given message.
		  @param utf8Message A status message to be provided by the returned object. Can be null or empty.
		                     This message is copied by the returned object.
		  @return Returns a new OperationResult object set to succeeded and with the given message.
		 */
		static OperationResult SucceededWith(const char* utf8Message);

		/**
		  Creates a new OperationResult object set to succeeded and with the given message.
		  @param utf8Message A status message to be provided by the returned object. Can be null or empty.
		  @return Returns a new OperationResult object set to succeeded and with the given message.
		 */
		static OperationResult SucceededWith(const Rtt::SharedConstStdStringPtr& utf8Message);

		/**
		  Creates a new OperationResult object set to failed and with the given message.
		  @param utf8Message Message explaining why the operation failed. Can be null or empty.
		                     This message is copied by the returned object.
		  @return Returns a new OperationResult object set to failed and with the given message.
		 */
		static OperationResult FailedWith(const char* utf8Message);

		/**
		  Creates a new OperationResult object set to failed and with the given message.
		  @param utf8Message Message explaining why the operation failed. Can be null or empty.
		  @return Returns a new OperationResult object set to failed and with the given message.
		 */
		static OperationResult FailedWith(const Rtt::SharedConstStdStringPtr& utf8Message);

	private:
		/** Set to true if the operation succeeded. Set to false if failed. */
		bool fHasSucceeded;

		/** Shared pointer to a UTF-8 encoded message. Will be null if no message was provided. */
		Rtt::SharedConstStdStringPtr fUtf8MessagePointer;
};

}	// namespace Rtt

#endif	// __Rtt_OperationResult_H__
