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
#include <collection.h>
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core\Rtt_Build.h"
#	include "Rtt_Lua.h"
#	include <mutex>
#	include <queue>
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


#pragma region Forward Declarations
namespace CoronaLabs { namespace Corona { namespace WinRT {
	namespace Interop {
		interface class ITimer;
	}
	ref class CoronaRuntimeEnvironment;
} } }
namespace CoronaLabs { namespace WinRT {
	ref class ImmutableByteBuffer;
	interface class IOperationResult;
	ref class MessageEventArgs;
	ref class Utf8String;
} }
#pragma endregion


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Networking {

ref class HttpProgressEventArgs;
ref class HttpRequestSettings;
ref class HttpResponseEventArgs;
interface class IHttpRequestOperation;

/// <summary>
///  <para>Implements sending an HTTP request to a server and receives an HTTP response.</para>
///  <para>This binds with Corona's Lua environment, pushing "networkRequest" events to Lua if a listener was provided.</para>
/// </summary>
ref class CoronaHttpRequestOperation sealed
{
	internal:
		#pragma region TransmitDirection Enum
		/// <summary>Indicates the transmission direction such as upload or download.</summary>
		enum class TransmitDirection
		{
			/// <summary>Indicates that data is being sent/uploaded to a server.</summary>
			kUpload,

			/// <summary>Indicates that data is being received/downloaded from a server.</summary>
			kDownload
		};

		#pragma endregion


		#pragma region TransmitPhase Enum
		/// <summary>Indicates if the upload/download has started, transmitting, or ended.</summary>
		enum class TransmitPhase
		{
			/// <summary>Indicates that the upload/download has started.</summary>
			kStarted,

			/// <summary>Indicates that the upload/download is currently in progress.</summary>
			kTransmitting,

			/// <summary>Indicates that the upload/download has finished.</summary>
			kEnded
		};

		#pragma endregion


		#pragma region NativeEventHandler Class
		/// <summary>
		///  <para>Stores a native C/C++ function pointer that will be called when an event has been raised.</para>
		///  <para>Instances of this class are expected to be passed to the SetEndedEventHandler() method.</para>
		/// </summary>
		class NativeEventHandler
		{
			public:
				/// <summary>Defines the function signature to be passed into the handler's constructor.</summary>
				typedef void(*FunctionType)(CoronaHttpRequestOperation^, void*);

				/// <summary>Creates a handler without a callback. Will do nothing when invoked.</summary>
				NativeEventHandler()
				{
					Clear();
				}

				/// <summary>
				///  Creates a new event handler which will call the given C/C++ function when the event gets raised.
				/// </summary>
				/// <param name="functionPointer">
				///  <para>Pointer to a C/C++ function or a C++ static function in a class.</para>
				///  <para>Can be null, but then this handler will do nothing when invoked.</para>
				/// </param>
				/// <param name="contextPointer">
				///  <para>Pointer to custom data that will be passed to the callback.</para>
				///  <para>Expected to be set to a C++ object's "this" pointer, if applicable.</para>
				/// </param>
				NativeEventHandler(FunctionType functionPointer, void *contextPointer)
				{
					Set(functionPointer, contextPointer);
				}

				/// <summary>Assigns a C/C++ function callback to this handler.</summary>
				/// <param name="functionPointer">
				///  <para>Pointer to a C/C++ function or a C++ static function in a class.</para>
				///  <para>Can be null, but then this handler will do nothing when invoked.</para>
				/// </param>
				/// <param name="contextPointer">
				///  <para>Pointer to custom data that will be passed to the callback.</para>
				///  <para>Expected to be set to a C++ object's "this" pointer, if applicable.</para>
				/// </param>
				void Set(FunctionType functionPointer, void *contextPointer)
				{
					fFunctionPointer = functionPointer;
					fContextPointer = contextPointer;
				}

				/// <summary>Clears the last assigned callback, causing this handler to do nothing when it is invoked.</summary>
				void Clear()
				{
					fFunctionPointer = nullptr;
					fContextPointer = nullptr;
				}

				/// <summary>Invokes this handler's function/callback with the given arguments.</summary>
				/// <param name="operation">The HTTP request operation that is raising this event.</param>
				void Invoke(CoronaHttpRequestOperation^ operation)
				{
					if (fFunctionPointer)
					{
						(*fFunctionPointer)(operation, fContextPointer);
					}
				}

			private:
				FunctionType fFunctionPointer;
				void* fContextPointer;
		};

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>Creates a new HTTP request operation object.</summary>
		/// <param name="integerId">
		///  <para>Unique integer ID assigned to this HTTP request operation out of all concurrent operations.</para>
		///  <para>This ID will be provided to Lua via the "event.requestId" field.</para>
		/// </param>
		/// <param name="environment">
		///  <para>Provides the INetworkServices implementation and a Lua state to push this operation's events to.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		CoronaHttpRequestOperation(int integerId, CoronaRuntimeEnvironment^ environment);

		#pragma endregion


		#pragma region Internal Methods/Properties
		/// <summary>Gets the unique integer ID of this operation out of all concurrent HTTP request operations.</summary>
		/// <value>
		///  <para>Unique integer ID assigned to this HTTP request operation.</para>
		///  <para>This value is provided to Lua via the "event.requestId" field.</para>
		/// </value>
		property int IntegerId { int get(); }

		/// <summary>Gets the Corona runtime environment this operation is associated with.</summary>
		/// <value>The Corona runtime environment this operation is associated with.</value>
		property CoronaRuntimeEnvironment^ RuntimeEnvironment { CoronaRuntimeEnvironment^ get(); }

		/// <summary>Gets or sets whether or not upload/download progress events should be pushed into Lua.</summary>
		/// <value>
		///  <para>Set true if upload/download progress events will be pushed into Lua.</para>
		///  <para>Set false to disable progress events. Only an "ended" event phase will be provided in this case.</para>
		/// </value>
		property bool ProgressEventsEnabled { bool get(); void set(bool value); }

		/// <summary>
		///  <para>Gets or sets the if this operation should provide upload or download progress events to Lua.</para>
		///  <para>This property is only applicable if the <see cref="ProgressEventsEnabled"/> property is set true.</para>
		/// </summary>
		/// <value>The transmit direction this operation will provide progress events for, if enabled.</value>
		property TransmitDirection ProgressEventsDirection { TransmitDirection get(); void set(TransmitDirection value); }

		/// <summary>
		///  Gets or sets a eference to a Lua event listener that this operation should use to push Lua events to.
		/// </summary>
		/// <value>
		///  <para>Reference to a Lua event listener to push events to.</para>
		///  <para>Set to null to not push events to Lua.</para>
		/// </value>
		property Rtt::Lua::Ref LuaListener { Rtt::Lua::Ref get(); void set(Rtt::Lua::Ref value); }

		/// <summary>
		///  <para>Gets the event handler to be invoked when this operation has finished executing.</para>
		///  <para>
		///   The operation is considered "Ended" when the full HTTP response has been received, a network error occurs,
		///   or when the operation has been aborted.
		///  </para>
		///  <para>The caller is expected to assign this handler a callback to a native C/C++ function.</para>
		/// </summary>
		/// <value>Event handler to be invoked when the operation has ended.</value>
		property NativeEventHandler& EndedEventHandler { NativeEventHandler& get(); }

		/// <summary>
		///  Gets modifiable settings used to provide the HTTP request data to be sent and how to handle the received response.
		/// </summary>
		/// <value>
		///  Settings providing the HTTP request information to be sent and configuration on how to handle the response.
		/// </value>
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

	private:
		#pragma region Private EventData Structure
		/// <summary>
		///  Private structure used to store async HTTP request event info to be dispatched to
		///  Lua as a "networkRequest" event.
		/// </summary>
		struct EventData
		{
			/// <summary>The URI/URL of the server the HTTP request was sent to.</summary>
			CoronaLabs::WinRT::Utf8String^ Url;

			/// <summary>Indicates if the upload/download has started, is in progress, or has finished.</summary>
			TransmitPhase Phase;

			/// <summary>Number of bytes uploade/downloaded so far.</summary>
			int64 BytesTransferred;

			/// <summary>Total number bytes expected to be uploaded/downloaded to/from the server.</summary>
			int64 BytesEstimated;

			/// <summary>Set true if this structure provides HTTP response information.</summary>
			bool HasResponse;

			/// <summary>
			///  <para>The HTTP status code received by the HTTP response.</para>
			///  <para>Not applicable if field "HasResponse" is set to false.</para>
			/// </summary>
			int ResponseStatusCode;
			
			/// <summary>
			///  <para>The HTTP response headers received.</para>
			///  <para>Not applicable if field "HasResponse" is set to false.</para>
			/// </summary>
			Windows::Foundation::Collections::IMapView<CoronaLabs::WinRT::Utf8String^, CoronaLabs::WinRT::Utf8String^>^ ResponseHeaders;

			/// <summary>
			///  <para>Set to true if the response body/content was written to file.</para>
			///  <para>Not applicable if field "HasResponse" is set to false.</para>
			/// </summary>
			bool WasResponseDownloadedToFile;
			
			/// <summary>
			///  <para>The path to the file the response's body/content was downloaded to.</para>
			///  <para>Not applicable if field "HasResponse" or "WasResponseDownloadedToFile" is set to false.</para>
			/// </summary>
			Platform::String^ DownloadFilePath;
			
			/// <summary>
			///  <para>Provides the HTTP response's body/content in byte form.</para>
			///  <para>Not applicable if field "HasResponse" is set to false.</para>
			///  <para>Not applicable if field "WasResponseDownloadedToFile" is set to true.</para>
			/// </summary>
			CoronaLabs::WinRT::ImmutableByteBuffer^ ResponseBody;

			/// <summary>Set to true if a network connection error or timeout occurred.</summary>
			bool HasErrorOccurred;

			/// <summary>
			///  If field "HasErrorOccurred" is set to true, then this message provides details about the network error.
			/// </summary>
			Platform::String^ ErrorMessage;


			/// <summary>Creates a new event data object initialized to its defaults.</summary>
			EventData();

			/// <summary>Re-initializes all data fields back to their defaults.</summary>
			void Clear();
		};

		#pragma endregion


		#pragma region Private Methods
		/// <summary>Subscribes to the events belonging to member variables "fTimer" and "fOperation".</summary>
		void AddEventHandlers();

		/// <summary>
		///  <para>Unsubscribes to the events belonging to member variables "fTimer" and "fOperation".</para>
		///  <para>
		///   Expected to be called when this operation finishes executing to release this object's
		///   reference from those member variables.
		///  </para>
		/// </summary>
		void RemoveEventHandlers();

		/// <summary>Dispatches the given event information to this operation's Lua listener.</summary>
		/// <param name="eventData">The event information to be dispatched as a Lua "networkRequest" event.</param>
		void DispatchEvent(const EventData &eventData);

		/// <summary>
		///  <para>Queues a copy of the given event information to be dispatched to Lua later on the main thread.</para>
		///  <para>This method is thread safe and can be called on any thread.</para>
		/// </summary>
		/// <param name="eventData">The event information to be copied and pushed to the end of the event queue.</param>
		void AsyncQueueEvent(const EventData &eventData);

		/// <summary>
		///  <para>To be called by this object when this operation is about to finish executing.</para>
		///  <para>Cleans up resources, removes event handlers, and invokes "fEndedEventHandler".</para>
		/// </summary>
		void OnEnd();

		/// <summary>
		///  <para>Called at regular intervals when member variable "fTimer" has elapsed.</para>
		///  <para>Dispatches all queued events to the Lua listener as "networkRequest" events.</para>
		/// </summary>
		/// <param name="sender">The timer that raised this event.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnTimerElapsed(ITimer ^sender, CoronaLabs::WinRT::EmptyEventArgs ^args);

		/// <summary>Called when the HTTP request has been aborted.</summary>
		/// <param name="sender">Reference to the "IHttpRequestOperation" object that raised this event.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnAborted(Platform::Object ^sender, CoronaLabs::WinRT::EmptyEventArgs ^args);

		/// <summary>Called when connected to the server/host and is about to send the HTTP request.</summary>
		/// <param name="sender">Reference to the "IHttpRequestOperation" object that raised this event.</param>
		/// <param name="args">Provides how many bytes are in the HTTP request body/content that will be sent.</param>
		void OnSendingRequest(Platform::Object ^sender, HttpProgressEventArgs ^args);

		/// <summary>
		///  <para>Called periodically as this operation object sends HTTP request bytes to the server in batches.</para>
		///  <para>
		///   Will not be called if the HTTP request packet size is small and is sent in one shot, in which case,
		///   the OnAsyncSentRequest() method will be invoked right after the OnSendingRequest() method.
		///  </para>
		///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
		/// </summary>
		/// <param name="sender">Reference to the "IHttpRequestOperation" object that raised this event.</param>
		/// <param name="args">Provides how many bytes have been transmitted to the server so far.</param>
		void OnAsyncSendProgressChanged(Platform::Object ^sender, HttpProgressEventArgs ^args);
		
		/// <summary>
		///  <para>Called when this operation has finished sending the full HTTP request to the server.</para>
		///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
		/// </summary>
		/// <param name="sender">Reference to the "IHttpRequestOperation" object that raised this event.</param>
		/// <param name="args">Provides how many bytes have been transmitted to the server.</param>
		void OnAsyncSentRequest(Platform::Object ^sender, HttpProgressEventArgs ^args);

		/// <summary>
		///  <para>Called when the beginning of the HTTP response (ie: its headers) has been received from the server.</para>
		///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
		/// </summary>
		/// <param name="sender">Reference to the "IHttpRequestOperation" object that raised this event.</param>
		/// <param name="args">
		///  Provides the HTTP response headers, status code, and expected number of bytes to be received.
		/// </param>
		void OnAsyncReceivingResponse(Platform::Object ^sender, HttpResponseEventArgs ^args);
		
		/// <summary>
		///  <para>Called periodically as HTTP response bytes are being received from the server.</para>
		///  <para>
		///   Will not be called if the full HTTP response has been received in one shot, in which case,
		///   the OnAsyncReceivedResponse() method will be called right after the OnAsyncReceivingResponse() method.
		///  </para>
		///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
		/// </summary>
		/// <param name="sender">Reference to the "IHttpRequestOperation" object that raised this event.</param>
		/// <param name="args">Provides the number of bytes received from the server so far.</param>
		void OnAsyncReceiveProgressChanged(Platform::Object ^sender, HttpProgressEventArgs ^args);

		/// <summary>
		///  <para>Raised when the full HTTP response has been received from the server.</para>
		///  <para>This operation will be flagged as completed at this point and is free to execute another HTTP request.</para>
		///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
		/// </summary>
		/// <param name="sender">Reference to the "IHttpRequestOperation" object that raised this event.</param>
		/// <param name="args">
		///  Provides the HTTP response headers, status code, and the body/content if not set up to download to file.
		/// </param>
		void OnAsyncReceivedResponse(Platform::Object ^sender, HttpResponseEventArgs ^args);

		/// <summary>
		///  <para>Called when a connection error or response timeout occurs.</para>
		///  <para>Note that this method does not get called when an HTTP error response has been received.</para>
		///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
		/// </summary>
		/// <param name="sender">Reference to the "IHttpRequestOperation" object that raised this event.</param>
		/// <param name="args">Provides an error message explaining exactly what went wrong.</param>
		void OnAsyncErrorOccurred(Platform::Object ^sender, CoronaLabs::WinRT::MessageEventArgs ^args);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>
		///  <para>Unique integer ID assigned to this operation out of all concurrent HTTP request operations.</para>
		///  <para>Provided to Lua via the "event.requestId" field.</para>
		/// </summary>
		int fIntegerId;
		
		/// <summary>Corona environment providing the network implementation and Lua state to push events to.</summary>
		CoronaRuntimeEnvironment^ fEnvironment;

		/// <summary>The HTTP request operation implementation.</summary>
		IHttpRequestOperation^ fOperation;

		/// <summary>Set true if an executing operation was aborted via the Abort() method.</summary>
		bool fWasAborted;

		/// <summary>
		///  Timer running on the main thread that is used to dispatch network events queued on the network thread.
		/// </summary>
		Interop::ITimer^ fTimer;

		/// <summary>Reference pointer to a Lua event listener. Null if not assigned.</summary>
		Rtt::Lua::Ref fLuaListenerReference;

		/// <summary>Set true to enable pushing upload/download progress events to Lua.</summary>
		bool fProgressEventsEnabled;

		/// <summary>
		///  <para>Determines if upload or download progress events should be pushed into Lua.</para>
		///  <para>Only applicable if member variable "fProgressEventsEnabled" is set to true.</para>
		/// </summary>
		TransmitDirection fProgressEventsDirection;

		/// <summary>
		///  <para>Queue of network event data to be dispatched to Lua as "networkRequest" events.</para>
		///  <para>
		///   Event data is expected to be pushed into this queue from the network thread and popped by the main UI thread.
		///  </para>
		/// </summary>
		std::queue<EventData> fEventDataQueue;

		/// <summary>
		///  <para>Stores the last event data received by this HTTP request operation's event handlers.</para>
		///  <para>Used to store previous received values to be delivered to Lua via a "networkRequest" event.</para>
		///  <para>
		///   For example, if a network error occurs, this would provide bytes transferred or response headers received.
		///  </para>
		/// </summary>
		EventData fLastReceivedEventData;

		/// <summary>Mutex used to synchronize access to the "fEventDataQueue" member variable.</summary>
		std::recursive_mutex fMutex;

		/// <summary>Callback to be invoked when the HTTP request operation stops executing.</summary>
		NativeEventHandler fEndedEventHandler;

		/// <summary>Token received when subscribing to a timer's "Elapsed" event.</summary>
		Windows::Foundation::EventRegistrationToken fTimerElapsedEventToken;

		/// <summary>Token received when subscribing to the operation's "Aborted" event.</summary>
		Windows::Foundation::EventRegistrationToken fAbortedEventToken;

		/// <summary>Token received when subscribing to the operation's "SendingRequest" event.</summary>
		Windows::Foundation::EventRegistrationToken fSendingRequestEventToken;

		/// <summary>Token received when subscribing to the operation's "AsyncSendProgressChanged" event.</summary>
		Windows::Foundation::EventRegistrationToken fAsyncSendProgressChangedEventToken;

		/// <summary>Token received when subscribing to the operation's "AsyncSentRequest" event.</summary>
		Windows::Foundation::EventRegistrationToken fAsyncSentRequestEventToken;

		/// <summary>Token received when subscribing to the operation's "AsyncReceivingResponse" event.</summary>
		Windows::Foundation::EventRegistrationToken fAsyncReceivingResponseEventToken;

		/// <summary>Token received when subscribing to the operation's "AsyncReceiveProgressChanged" event.</summary>
		Windows::Foundation::EventRegistrationToken fAsyncReceiveProgressChangedEventToken;

		/// <summary>Token received when subscribing to the operation's "AsyncReceivedResponse" event.</summary>
		Windows::Foundation::EventRegistrationToken fAsyncReceivedResponseEventToken;

		/// <summary>Token received when subscribing to the operation's "AsyncErrorOccurred" event.</summary>
		Windows::Foundation::EventRegistrationToken fAsyncErrorOccurredEventToken;

		#pragma endregion
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Networking
