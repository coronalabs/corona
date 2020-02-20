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


#include "CoronaLabs\WinRT\EmptyEventArgs.h"
#include "CoronaLabs\WinRT\MessageEventArgs.h"
#include "HttpProgressEventArgs.h"
#include "HttpResponseEventArgs.h"


#pragma region Forward Declarations
namespace CoronaLabs { namespace WinRT {
	interface class IOperationResult;
} }
#pragma endregion


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Networking {

ref class HttpRequestSettings;

/// <summary>Sends an HTTP request to a server and receives an HTTP response.</summary>
public interface class IHttpRequestOperation
{
	#pragma region Events
	/// <summary>
	///  <para>Raised just before connecting to the specified server to send the request to.</para>
	///  <para>Provides the number of bytes to be sent to the server.</para>
	///  <para>This event is raised on the same thread that called this object's Execute() method.</para>
	/// </summary>
	event Windows::Foundation::EventHandler<HttpProgressEventArgs^>^ SendingRequest;

	/// <summary>
	///  <para>Raised periodically as this operation object sends HTTP request bytes to the server in batches.</para>
	///  <para>
	///   Will not be raised if the HTTP request packet size is small and is sent in one shot, in which case,
	///   the AsyncSentRequest event will be immediately raised after the SendingRequest event.
	///  </para>
	///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
	/// </summary>
	event Windows::Foundation::EventHandler<HttpProgressEventArgs^>^ AsyncSendProgressChanged;
	
	/// <summary>
	///  <para>Raised when this object has finished sending the full HTTP request to the server.</para>
	///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
	/// </summary>
	event Windows::Foundation::EventHandler<HttpProgressEventArgs^>^ AsyncSentRequest;

	/// <summary>
	///  <para>Raised when the beginning of the HTTP response (ie: its headers) has been received from the server.</para>
	///  <para>Note that this may provide the number of bytes expected to be received from the server.</para>
	///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
	/// </summary>
	event Windows::Foundation::EventHandler<HttpResponseEventArgs^>^ AsyncReceivingResponse;

	/// <summary>
	///  <para>Raised periodically as HTTP response bytes are being received from the server.</para>
	///  <para>
	///   Will not be raised if the full HTTP response has been received in one shot, in which case,
	///   the AsyncReceivedResponse event will be raised immediately after the AsyncReceivingResponse event.
	///  </para>
	///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
	/// </summary>
	event Windows::Foundation::EventHandler<HttpProgressEventArgs^>^ AsyncReceiveProgressChanged;

	/// <summary>
	///  <para>Raised when the full HTTP response has been received from the server.</para>
	///  <para>This operation will be flagged as completed at this point and is free to execute another HTTP request.</para>
	///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
	/// </summary>
	event Windows::Foundation::EventHandler<HttpResponseEventArgs^>^ AsyncReceivedResponse;

	/// <summary>
	///  <para>Raised when a connection error or response timeout occurs.</para>
	///  <para>Note that this event does not get raised when an HTTP error response has been received.</para>
	///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
	/// </summary>
	event Windows::Foundation::EventHandler<CoronaLabs::WinRT::MessageEventArgs^>^ AsyncErrorOccurred;

	/// <summary>
	///  <para>Raised when the Abort() method gets called.</para>
	///  <para>This event is raised on the same thread that called this object's Abort() method.</para>
	/// </summary>
	event Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>^ Aborted;

	#pragma endregion


	#pragma region Methods/Properties
	/// <summary>
	///  Gets modifiable settings used to provide the HTTP request data to be sent and how to handle the received response.
	/// </summary>
	/// <value>Settings providing the HTTP request information to be sent and configuration on how to handle the response.</value>
	property HttpRequestSettings^ Settings { HttpRequestSettings^ get(); }

	/// <summary>Determines if this operation is in the middle of handling an HTTP request/response.</summary>
	/// <value>
	///  <para>
	///   Returns true if this operation object's <see cref="Execute()"/> method has been called and is in the middle
	///   of sending an HTTP request or handling an HTTP response.
	///  </para>
	///  <para>
	///   Returns false if the <see cref="Execute()"/> method has not been called yet or the last executed HTTP request has
	///   been completed, aborted, or erroed out.
	///  </para>
	/// </value>
	property bool IsExecuting { bool get(); }
	
	/// <summary>
	///  <para>Sends an HTTP request and receives an HTTP response based on the <see cref="Settings"/> property.</para>
	///  <para>
	///   A call to this method will be ignored if this operation object is already in the middle of executing
	///   an HTTP request operation.
	///  </para>
	/// </summary>
	/// <returns>
	///  <para>Returns a success result if the HTTP request operation has been started.</para>
	///  <para>
	///   Returns a failure result if the <see cref="Settings"/> property contains invalid information or if
	///   this operation object is already in the middle of executing an HTTP request. In this case, the returned
	///   result object will typically provide an error message detailing why it failed.
	///  </para>
	/// </returns>
	CoronaLabs::WinRT::IOperationResult^ Execute();
	
	/// <summary>
	///  <para>Aborts the currently executing HTTP request operation, if running.</para>
	///  <para>Will raise an Aborted event if this operation object is currently executing.</para>
	///  <para>
	///   Once aborted, the <see cref="IsExecuting"/> property will be set false and no more events will be raised
	///   until the next call to the <see cref="Execute()"/> method.
	///  </para>
	/// </summary>
	void Abort();

	#pragma endregion
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Networking
