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

#include "Core\Rtt_Macros.h"
#include "Interop\UI\UIComponent.h"
#include "Interop\Event.h"
#include "Interop\EventArgs.h"
#include "Interop\OperationResult.h"
#include "IODataEventArgs.h"
#include <mutex>
#include <vector>
#include <Windows.h>


#pragma region Forward Declarations
namespace Interop {
	namespace UI {
		class HandleMessageEventArgs;
		class MessageOnlyWindow;
	}
}

#pragma endregion


namespace Interop { namespace Ipc {

/// <summary>
///  <para>Reader used to easily/safely monitor and read data from a pipe on a separate thread.</para>
///  <para>Provides "ReceivedData" and "Closed" events on the thread the reader was created on.</para>
/// </summary>
class AsyncPipeReader
{
	Rtt_CLASS_NO_COPIES(AsyncPipeReader)

	public:
		#pragma region Public Event Types
		/// <summary>Defines the "Closed" event type which is raised when the pipe has been closed externally.</summary>
		typedef Event<AsyncPipeReader&, const EventArgs&> ClosedEvent;

		/// <summary>
		///  Defines the "ReceivedData" event type which is raised when a newline '\n' terminated string
		///  has been received from the pipe.
		/// </summary>
		typedef Event<AsyncPipeReader&, const IODataEventArgs&> ReceivedDataEvent;

		/// <summary>
		///  <para>
		///   Defines the "ReceivingDataBegan" event type which is raised when this pipe is about to raise
		///   1 or more "ReceivedData" events followed by a "ReceivingDataEnded" event.
		///  </para>
		///  <para>
		///   An event handler can use this to optimize for receiving several "ReceivedData" events in large batches.
		///  </para>
		/// </summary>
		typedef Event<AsyncPipeReader&, const EventArgs&> ReceivingDataBeganEvent;

		/// <summary>
		///  <para>
		///   Defines the "ReceivingDataEnded" event type which is raised when this pipe has finished raising
		///   1 or more "ReceivedData" events as a single batch.
		///  </para>
		///  <para>This event always follows a "ReceivingDataBegan" event.</para>
		///  <para>
		///   An event handler can use this to optimize for receiving several "ReceivedData" events in large batches.
		///  </para>
		/// </summary>
		typedef Event<AsyncPipeReader&, const EventArgs&> ReceivingDataEndedEvent;

		#pragma endregion


		#pragma region Constructors/Destructors
		/// <summary>
		///  <para>Creates a new reader which fetches data from the given pipe on another thread.</para>
		///  <para>Note: The caller is not expected to read from the given pipe while this reader is running.</para>
		/// </summary>
		/// <param name="pipeHandle">
		///  <para>Handle to a pipe to read data from.</para>
		///  <para>Can be null, but then this reader will always fail to start.</para>
		/// </param>
		AsyncPipeReader(HANDLE pipeHandle);

		/// <summary>Stops reading from the assigned pipe and destroys this object.</summary>
		virtual ~AsyncPipeReader();

		#pragma endregion


		#pragma region Public Methods
		/// <summary>Gets an object used to add or remove an event handler for the "Closed" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		ClosedEvent::HandlerManager& GetClosedEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "ReceivedData" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		ReceivedDataEvent::HandlerManager& GetReceivedDataEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "ReceivingDataBegan" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		ReceivingDataBeganEvent::HandlerManager& GetReceivingDataBeganEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "ReceivingDataEnded" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		ReceivingDataEndedEvent::HandlerManager& GetReceivingDataEndedEventHandlers();

		/// <summary>Gets a Win32 handle to the pipe that this reader will fetch data from.</summary>
		/// <returns>Returns a Win32 handle to the pipe that this reader was assigned via its constructor.</returns>
		HANDLE GetHandle() const;

		/// <summary>Determines if this reader has been started and is currently reading from the pipe.</summary>
		/// <returns>
		///  <para>Returns true if this reader has been started and is currently reading from the pipe.</para>
		///  <para>
		///   Returns false if not started, stopped, or if the pipe has been closed while the reader was running.
		///  </para>
		/// </returns>
		bool IsRunning() const;

		/// <summary>
		///  <para>Determines if the pipe was closed while this reader was running.</para>
		///  <para>
		///   Note: The reader cannot detect if the pipe given to it via the constructor is closed until the
		///   reader has been started.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>
		///   Returns true if the pipe was closed or errored out while this reader was running.
		///   You can no longer read from the pipe in this case.
		///  </para>
		///  <para>Returns false if the pipe is open or not detected as closed yet.</para>
		/// </returns>
		bool WasClosed() const;

		/// <summary>Starts reading from the pipe on another thread.</summary>
		/// <returns>
		///  <para>Returns a success result if the reader was successfully started with the given pipe.</para>
		///  <para>
		///   Returns a failure result if the reader was given an invalid pipe handle, the pipe is closed,
		///   if already started, or if unable to spawn a thread for reading. The result object's GetMessage()
		///   method will detail why it failed.
		///  </para>
		/// </returns>
		OperationResult Start();

		/// <summary>Stops reading from the pipe.</summary>
		void Stop();

		/// <summary>
		///  <para>
		///   Polls for all data received from the pipe immediately instead of waiting for the windows message pump
		///   to poll for pending events.
		///  </para>
		///  <para>Raises a "ReceivedData" event for each newline ended string found in the pipe.</para>
		///  <para>Raises a "Closed" event if the pipe has been closed externally.</para>
		///  <para>Note: Will only poll if pipe reading has been started.</para>
		/// </summary>
		void Poll();

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>
		///  <para>Called when a Windows message has been received by this reader's message-only window.</para>
		///  <para>
		///   Used by the worker thread to notify the reader on the thread it was created on that new data
		///   has been received/queued or if the pipe was closed.
		///  </para>
		/// </summary>
		/// <param name="sender">Reference to the message-only window.</param>
		/// <param name="arguments">
		///  <para>Provides the Windows message information.</para>
		///  <para>Call its SetHandled() and SetReturnValue() methods if this handler will be handling the message.</para>
		/// </param>
		void OnReceivedMessage(Interop::UI::UIComponent& sender, Interop::UI::HandleMessageEventArgs& arguments);

		/// <summary>
		///  <para>Handles all received data queued in "fReceivedDataCollection" and raises "ReceivedData" events.</para>
		///  <para>
		///   This method is expected to be called by the OnReceivedMessage() method when the worker thread notifies
		///   the reader on the main thread that new data has been queued.
		///  </para>
		/// </summary>
		void ProcessReceivedData();

		#pragma endregion


		#pragma region Private Static Functions
		/// <summary>Called by this class' worker threads to read data from a pipe asynchronously.</summary>
		/// <param name="threadIntegerId">
		///  <para>Unique integer ID assigned to the thread by the reader.</para>
		///  <para>
		///   Used by this function to access the reader object via the static "sThreadIdToAsyncPipeReaderMap" collection.
		///  </para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		static void OnAsyncExecute(const uint32_t threadIntegerId);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Mutex used to synchronize access to this class instance's member variables.</summary>
		std::recursive_mutex fMutex;

		/// <summary>Manages the "Closed" event.</summary>
		ClosedEvent fClosedEvent;

		/// <summary>Manages the "ReceivedData" event.</summary>
		ReceivedDataEvent fReceivedDataEvent;

		/// <summary>Manages the "ReceivingDataBegan" event.</summary>
		ReceivingDataBeganEvent fReceivingDataBeganEvent;

		/// <summary>Manages the "ReceivingDataEnded" event.</summary>
		ReceivingDataEndedEvent fReceivingDataEndedEvent;

		/// <summary>Handler to be invoked when the "ReceivedMessage" event has been raised.</summary>
		Interop::UI::UIComponent::ReceivedMessageEvent::MethodHandler<AsyncPipeReader> fReceivedMessageEventHandler;

		/// <summary>The Win32 handle to the pipe.</summary>
		HANDLE fPipeHandle;

		/// <summary>Set true if Start() was called and the reader is actively reading/polling the pipe.</summary>
		bool fIsRunning;

		/// <summary>
		///  <para>Set true if the pipe was closed externally and cannot be read anymore.</para>
		///  <para>Set false if the pipe connection is currently open.</para>
		/// </summary>
		bool fWasClosed;

		/// <summary>
		///  Set true if this reader is in the middle of popping data from its "fReceivedDataCollection" queue
		///  and raising "ReceivedData" events.
		/// </summary>
		bool fIsRaisingReceivedDataEvents;

		/// <summary>
		///  <para>Stores a queue of data received from the pipe.</para>
		///  <para>Data is pushed in by the worker thread and popped on the thread that created this reader.</para>
		/// </summary>
		std::vector<IODataEventArgs::Data> fReceivedDataCollection;

		/// <summary>
		///  Pointer to a Win32 message-only window used by this reader to send notifications to itself
		///  from its worker thread to the thread that created this reader.
		/// </summary>
		Interop::UI::MessageOnlyWindow* fMessageOnlyWindowPointer;

		/// <summary>
		///  <para>Unique Windows message ID reserved by the Win32 message-only window used by this reader.</para>
		///  <para>Used to notify the reader on the thread it was created on that the pipe was closed/disconnected.</para>
		/// </summary>
		UINT fPipeClosedMessageId;

		/// <summary>
		///  <para>Unique Windows message ID reserved by the Win32 message-only window used by this reader.</para>
		///  <para>
		///   Used to notify the reader on the thread it was created on that the "fReceivedDataCollection"
		///   has new data queued into it by the worker thread.
		///  </para>
		/// </summary>
		UINT fReceivedDataMessageId;

		/// <summary>
		///  <para>Unique integer ID assigned to thread that is reading data from the pipe.</para>
		///  <para>Set to zero if the reader is no longer running and monitoring its thread.</para>
		/// </summary>
		uint32_t fThreadIntegerId;

		#pragma endregion
};

} }	// namespace Interop::Ipc
