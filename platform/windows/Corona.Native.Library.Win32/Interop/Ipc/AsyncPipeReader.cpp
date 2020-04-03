//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AsyncPipeReader.h"
#include "Interop\UI\HandleMessageEventArgs.h"
#include "Interop\UI\MessageOnlyWindow.h"
#include "Interop\Ticks.h"
#include <algorithm>
#include <exception>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>


namespace Interop { namespace Ipc {

#pragma region Static Members
/// <summary>Mutex used to synchronize access to this class' static member variables.</summary>
static std::recursive_mutex sMutex;

/// <summary>
///  <para>Hash table used by all worker threads to access the AsyncPipeReader object that spawned them.</para>
///  <para>The key is a unique integer ID assigned to the thread via the OnAsyncExecute() argument.</para>
///  <para>The worker thread is expected to remove the pair when it exits the thread.</para>
///  <para>
///   The reader can null out its pointer in the pair if the thread does not exit in-time. This allows the reader
///   to cut loose a thread before spawning a new one so that the old thread can no longer access it.
///  </para>
/// </summary>
static std::unordered_map<uint32_t, AsyncPipeReader*> sThreadIdToAsyncPipeReaderMap;

/// <summary>Stores the next unique integer ID to be assigned to a worker thread.</summary>
static uint32_t sNextThreadIntegerId;

#pragma endregion


#pragma region Constructors/Destructors
AsyncPipeReader::AsyncPipeReader(HANDLE pipeHandle)
:	fPipeHandle(pipeHandle),
	fIsRunning(false),
	fWasClosed(false),
	fIsRaisingReceivedDataEvents(false),
	fMessageOnlyWindowPointer(Interop::UI::MessageOnlyWindow::GetSharedInstanceForCurrentThread()),
	fReceivedMessageEventHandler(this, &AsyncPipeReader::OnReceivedMessage),
	fPipeClosedMessageId(0),
	fReceivedDataMessageId(0),
	fThreadIntegerId(0)
{
	// Add event handlers.
	if (fMessageOnlyWindowPointer)
	{
		fMessageOnlyWindowPointer->GetReceivedMessageEventHandlers().Add(&fReceivedMessageEventHandler);
		fPipeClosedMessageId = fMessageOnlyWindowPointer->ReserveMessageId();
		fReceivedDataMessageId = fMessageOnlyWindowPointer->ReserveMessageId();
	}
}

AsyncPipeReader::~AsyncPipeReader()
{
	// Stop the worker thread in case it is still running.
	Stop();

	// Remove event handlers.
	if (fMessageOnlyWindowPointer)
	{
		fMessageOnlyWindowPointer->GetReceivedMessageEventHandlers().Remove(&fReceivedMessageEventHandler);
		fMessageOnlyWindowPointer->UnreserveMessageId(fPipeClosedMessageId);
		fMessageOnlyWindowPointer->UnreserveMessageId(fReceivedDataMessageId);
		fPipeClosedMessageId = 0;
		fReceivedDataMessageId = 0;
	}
}

#pragma endregion


#pragma region Public Methods
AsyncPipeReader::ClosedEvent::HandlerManager& AsyncPipeReader::GetClosedEventHandlers()
{
	return fClosedEvent.GetHandlerManager();
}

AsyncPipeReader::ReceivedDataEvent::HandlerManager& AsyncPipeReader::GetReceivedDataEventHandlers()
{
	return fReceivedDataEvent.GetHandlerManager();
}

AsyncPipeReader::ReceivingDataBeganEvent::HandlerManager& AsyncPipeReader::GetReceivingDataBeganEventHandlers()
{
	return fReceivingDataBeganEvent.GetHandlerManager();
}

AsyncPipeReader::ReceivingDataEndedEvent::HandlerManager& AsyncPipeReader::GetReceivingDataEndedEventHandlers()
{
	return fReceivingDataEndedEvent.GetHandlerManager();
}

HANDLE AsyncPipeReader::GetHandle() const
{
	return fPipeHandle;
}

bool AsyncPipeReader::IsRunning() const
{
	return fIsRunning;
}

bool AsyncPipeReader::WasClosed() const
{
	return fWasClosed;
}

OperationResult AsyncPipeReader::Start()
{
	// Validate.
	if (!fPipeHandle)
	{
		return OperationResult::FailedWith(L"Pipe handle is null.");
	}
	if (!fMessageOnlyWindowPointer)
	{
		return OperationResult::FailedWith(L"Failed to obtain a Win32 message-only window.");
	}
	if (fWasClosed)
	{
		return OperationResult::FailedWith(L"Pipe has been closed.");
	}

	// Do not continue if this reader is already running.
	if (IsRunning())
	{
		return OperationResult::FailedWith(L"Already started.");
	}

	// Create a static mapping between the worker thread and this AsyncPipeReader instance.
	// This is used by the worker thread to fetch this object instance safely.
	// It also allows us to cut the thread loose and remove the mapping in case the worker thread hangs.
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);

		// Generate a unique thread ID out of all AsyncPipeReader instances.
		for (fThreadIntegerId = sNextThreadIntegerId;; fThreadIntegerId++)
		{
			// Do not allow an ID of zero.
			// This value is used to indicate that a reader is not currently executing a thread.
			if (0 == fThreadIntegerId)
			{
				continue;
			}

			// Check if this ID is already being used by another pipe reader.
			if (sThreadIdToAsyncPipeReaderMap.find(fThreadIntegerId) != sThreadIdToAsyncPipeReaderMap.end())
			{
				continue;
			}

			// The generated thread ID is unique. Break out of the loop.
			break;
		}
		sNextThreadIntegerId = fThreadIntegerId + 1;

		// Add a mapping between the generated thread ID and this reader instance.
		sThreadIdToAsyncPipeReaderMap.insert(std::pair<uint32_t, AsyncPipeReader*>(fThreadIntegerId, this));
	}
	
	// Clear any data remaining in the queue.
	fReceivedDataCollection.clear();
	fReceivedDataCollection.shrink_to_fit();

	// Start the worker thread used to read data from the pipe.
	fIsRunning = true;
	try
	{
		std::thread newThread(&AsyncPipeReader::OnAsyncExecute, fThreadIntegerId);
		newThread.detach();
	}
	catch (std::exception ex)
	{
		// Failed to start the thread.
		// Reset this reader's variable to indicate that it has not been started.
		{
			std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
			sThreadIdToAsyncPipeReaderMap.erase(fThreadIntegerId);
		}
		fThreadIntegerId = 0;
		fIsRunning = false;

		// Return an error explaining why it failed.
		const char* errorMessage = ex.what();
		if (!errorMessage || ('\0' == errorMessage[0]))
		{
			errorMessage = "Failed to create thread.";
		}
		return OperationResult::FailedWith(errorMessage);
	}

	// This reader has been successfully started.
	return OperationResult::kSucceeded;
}

void AsyncPipeReader::Stop()
{
	// Do not continue if not running.
	if (!fIsRunning)
	{
		return;
	}

	// Flag that this reader is no longer running.
	fIsRunning = false;

	// Wait for the worker thread to exit gracefully.
	const Ticks kTimeoutInTicks = Ticks::FromCurrentTime().AddSeconds(1);
	do
	{
		std::this_thread::yield();
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		if (sThreadIdToAsyncPipeReaderMap.find(fThreadIntegerId) != sThreadIdToAsyncPipeReaderMap.end())
		{
			break;
		}
	} while (Ticks::FromCurrentTime() < kTimeoutInTicks);

	// Remove this reader from the static mapping.
	// This cuts the worker thread loose in case it is blocked/timed-out above, denying it access to this reader.
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		if (sThreadIdToAsyncPipeReaderMap.find(fThreadIntegerId) != sThreadIdToAsyncPipeReaderMap.end())
		{
			sThreadIdToAsyncPipeReaderMap[fThreadIntegerId] = nullptr;
		}
	}
	fThreadIntegerId = 0;

	// Clear any data remaining in the queue.
	fReceivedDataCollection.clear();
	fReceivedDataCollection.shrink_to_fit();
}

void AsyncPipeReader::Poll()
{
	// Remove all "ReceivedData" messages from this pipe's message-only window queue.
	if (fMessageOnlyWindowPointer && fReceivedDataMessageId)
	{
		fMessageOnlyWindowPointer->RemoveMessagesById(fReceivedDataMessageId);
	}

	// Process all queued data from the worker thread right now.
	ProcessReceivedData();

	// If a "PipeClosed" message was posted on the window queue, then handle it now.
	if (fMessageOnlyWindowPointer && fPipeClosedMessageId)
	{
		bool wasRemoved = fMessageOnlyWindowPointer->RemoveMessagesById(fPipeClosedMessageId);
		if (wasRemoved)
		{
			Interop::UI::MessageSettings messageSettings{};
			messageSettings.WindowHandle = fMessageOnlyWindowPointer->GetWindowHandle();
			messageSettings.MessageId = fPipeClosedMessageId;
			OnReceivedMessage(*fMessageOnlyWindowPointer, Interop::UI::HandleMessageEventArgs(messageSettings));
		}
	}
}

#pragma endregion


#pragma region Private Methods
void AsyncPipeReader::OnReceivedMessage(
	Interop::UI::UIComponent& sender, Interop::UI::HandleMessageEventArgs& arguments)
{
	// Do not continue if the received message was already handled.
	if (arguments.WasHandled())
	{
		return;
	}

	// Handle the received message.
	if (arguments.GetMessageId() == fPipeClosedMessageId)
	{
		// *** The pipe has been externally closed. ***
		// Note: The worker thread will automatically self terminate in this case.

		// Handle the close event.
		if (!fWasClosed)
		{
			// First, raise events for all received data in the queue, if any.
			ProcessReceivedData();

			// Update member variables.
			fIsRunning = false;
			fThreadIntegerId = 0;
			fWasClosed = true;

			// Notify the system that the pipe has been closed.
			fClosedEvent.Raise(*this, EventArgs::kEmpty);
		}

		// Flag this message as handled.
		arguments.SetHandled();
		arguments.SetReturnResult(0);
	}
	else if (arguments.GetMessageId() == fReceivedDataMessageId)
	{
		// *** New data has been received from the pipe and appended to this reader's queue. ***

		// Handle all queued data received from the pipe by the worker thread.
		ProcessReceivedData();

		// Flag this message as handled.
		arguments.SetHandled();
		arguments.SetReturnResult(0);
	}
}

void AsyncPipeReader::ProcessReceivedData()
{
	// Do not continue if we're already in the middle of raising "ReceivedData" events below.
	// This prevents data from from being received out of order.
	// Note: This issue can happen if a "ReceivedData" event handler pumps the system's message queue.
	if (fIsRaisingReceivedDataEvents)
	{
		return;
	}

	// Do not continue if this reader is no longer running.
	if (!fIsRunning)
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);
		fReceivedDataCollection.clear();
		return;
	}

	// Do not continue if we haven't received any data.
	if (fReceivedDataCollection.empty())
	{
		return;
	}

	// Notify the system that we're about to process all received data.
	fIsRaisingReceivedDataEvents = true;
	fReceivingDataBeganEvent.Raise(*this, EventArgs::kEmpty);

	// Handle all queued data received from the pipe by the worker thread.
	// Note: Make sure this reader wasn't stopped by an event handler for the above "ReceiveDataBegan" event.
	if (fIsRunning)
	{
		const size_t kDataArraySize = 32;
		IODataEventArgs::Data dataArray[kDataArraySize];
		while (fReceivedDataCollection.size() > 0)
		{
			// Pop off a batch of data from the "fReceivedDataCollection" and copy them to our temporary array.
			int dataItemsCopied = 0;
			{
				std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);
				const int kDataItemsAvailable = (int)fReceivedDataCollection.size();
				int index;
				for (index = 0; (index < kDataItemsAvailable) && (index < kDataArraySize); index++)
				{
					dataArray[index] = fReceivedDataCollection.at(index);
				}
				dataItemsCopied = index;
				if (dataItemsCopied > 0)
				{
					auto iterator = fReceivedDataCollection.begin();
					if (dataItemsCopied >= 2)
					{
						fReceivedDataCollection.erase(iterator, iterator + dataItemsCopied);
					}
					else
					{
						fReceivedDataCollection.erase(iterator);
					}
				}
			}

			// Notify the system about the newly received I/O data.
			for (int index = 0; index < dataItemsCopied; index++)
			{
				// Raise a "ReceivedData" event.
				try
				{
					fReceivedDataEvent.Raise(*this, IODataEventArgs(dataArray[index]));
				}
				catch (...) {}

				// Do not continue if an event handler has stopped this reader or closed the pipe.
				if (!fIsRunning)
				{
					break;
				}
			}

			// Do not continue if an event handler has stopped this reader.
			if (!fIsRunning)
			{
				break;
			}
		}
	}

	// Notify the system that we're done processing all received data.
	fIsRaisingReceivedDataEvents = false;
	fReceivingDataEndedEvent.Raise(*this, EventArgs::kEmpty);
}

#pragma endregion


#pragma region Private Static Functions
void AsyncPipeReader::OnAsyncExecute(const uint32_t threadIntegerId)
{
	// Validate.
	if (!threadIntegerId)
	{
		throw std::exception();
	}

	// Fetch information from the reader.
	HANDLE pipeHandle = nullptr;
	HWND windowHandle = nullptr;
	UINT pipeClosedMessageId = 0;
	UINT receivedDataMessageId = 0;
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		auto iterator = sThreadIdToAsyncPipeReaderMap.find(threadIntegerId);
		if (iterator != sThreadIdToAsyncPipeReaderMap.end())
		{
			auto readerPointer = (*iterator).second;
			if (readerPointer)
			{
				pipeHandle = readerPointer->fPipeHandle;
				windowHandle = readerPointer->fMessageOnlyWindowPointer->GetWindowHandle();
				pipeClosedMessageId = readerPointer->fPipeClosedMessageId;
				receivedDataMessageId = readerPointer->fReceivedDataMessageId;
			}
		}
	}

	// Start our infinite loop which reads the given reader's pipe.
	const DWORD kBufferSizeInBytes = 32767;
	char byteBuffer[kBufferSizeInBytes];
	DWORD bytesCopiedToBuffer = 0;
	bool wasStopRequested = false;
	bool wasPipeClosed = false;
	while (true)
	{
		// Poll the pipe's buffer until:
		// - All of the bytes in the buffer have been read.
		// - The pipe has been closed or errored out.
		// - The reader has requested this thread to exit.
		while (true)
		{
			// Determine if it's time to exit this thread.
			{
				// Exit if the reader was destroyed.
				std::lock_guard<std::recursive_mutex> scopedStaticMutexLock(sMutex);
				AsyncPipeReader* readerPointer = nullptr;
				auto iterator = sThreadIdToAsyncPipeReaderMap.find(threadIntegerId);
				if (iterator != sThreadIdToAsyncPipeReaderMap.end())
				{
					readerPointer = (*iterator).second;
				}
				if (!readerPointer)
				{
					wasStopRequested = true;
					break;
				}

				// Exit if the reader was stopped.
				std::lock_guard<std::recursive_mutex> scopedInstanceMutexLock(readerPointer->fMutex);
				if (!readerPointer->fIsRunning)
				{
					wasStopRequested = true;
					break;
				}
			}

			// Do a non-blocking check for bytes in the pipe's buffer.
			// Note: This *will* block if a ReadFile() was called on another thread. (You should never do that.)
			BOOL result;
			DWORD bytesAvailable = 0;
			result = ::PeekNamedPipe(pipeHandle, nullptr, 0, nullptr, &bytesAvailable, nullptr);
			if (!result)
			{
				if (::GetLastError() != ERROR_IO_PENDING)
				{
					wasPipeClosed = true;
				}
				break;
			}

			// Do not continue if the pipe does not contain any data.
			if (bytesAvailable <= 0)
			{
				break;
			}

			// Fetch data from the pipe.
			auto timestamp = DateTime::FromCurrentLocal();
			DWORD bytesReceived = 0;
			DWORD bytesRequested = (std::min)(bytesAvailable, kBufferSizeInBytes - bytesCopiedToBuffer);
			result = ::ReadFile(pipeHandle, byteBuffer + bytesCopiedToBuffer, bytesRequested, &bytesReceived, nullptr);
			if (!result)
			{
				if (::GetLastError() != ERROR_IO_PENDING)
				{
					wasPipeClosed = true;
				}
				break;
			}
			if (bytesReceived <= 0)
			{
				break;
			}
			if (bytesReceived > bytesRequested)
			{
				bytesReceived = bytesRequested;
			}
			bytesCopiedToBuffer += bytesReceived;

			// Send all received data to the reader on its main thread.
			{
				std::lock_guard<std::recursive_mutex> scopedStaticMutexLock(sMutex);
				auto iterator = sThreadIdToAsyncPipeReaderMap.find(threadIntegerId);
				if ((iterator != sThreadIdToAsyncPipeReaderMap.end()) && (*iterator).second)
				{
					// Split the received data into newline separated strings and push them into the reader's queue.
					auto readerPointer = (*iterator).second;
					std::lock_guard<std::recursive_mutex> scopedInstanceMutexLock(readerPointer->fMutex);
					IODataEventArgs::Data ioData;
					ioData.Timestamp = timestamp;
					while (bytesCopiedToBuffer > 0)
					{
						// Find the next newline character in the buffer.
						DWORD endIndex;
						DWORD skip = 0;
						for (endIndex = 0; endIndex < bytesCopiedToBuffer; endIndex++)
						{
							if (skip == endIndex && byteBuffer[skip] == 0)
							{
								skip++;
							}
							if ('\n' == byteBuffer[endIndex])
							{
								break;
							}
						}
						if (endIndex < bytesCopiedToBuffer)
						{
							// Newline was found. Push that substring into the reader's queue.
							DWORD bytesToCopy = endIndex + 1;
							try
							{
								ioData.Text = std::make_shared<const std::string>(byteBuffer+skip, bytesToCopy-skip);
								readerPointer->fReceivedDataCollection.push_back(ioData);
							}
							catch (...) {}

							// Remove the above substring from this thread's buffer by shifting its bytes.
							bytesCopiedToBuffer -= bytesToCopy;
							if (bytesCopiedToBuffer > 0)
							{
								memmove_s(byteBuffer, kBufferSizeInBytes, byteBuffer + bytesToCopy, bytesCopiedToBuffer);
							}
						}
						else
						{
							// A newline character was not found.
							// If the buffer is full, then push the entire buffer as 1 string into the reader's queue.
							if (bytesCopiedToBuffer >= kBufferSizeInBytes)
							{
								try
								{
									ioData.Text = std::make_shared<const std::string>(byteBuffer+skip, kBufferSizeInBytes-skip);
									readerPointer->fReceivedDataCollection.push_back(ioData);
								}
								catch (...) {}
								bytesCopiedToBuffer = 0;
							}

							// Wait for more data from the pipe. We're hoping to get a newline character later.
							break;
						}
					}

					// Notify the reader on its main thread if at least 1 string was pushed into the reader's queue.
					if (ioData.Text.get() && windowHandle && receivedDataMessageId)
					{
						::PostMessageW(windowHandle, receivedDataMessageId, 0, 0);
					}
				}
			}
		}

		// Exit out of this thead if:
		// - The pipe was closed or errored out.
		// - The reader requested this thread to exit via its Stop() method.
		if (wasPipeClosed || wasStopRequested)
		{
			break;
		}

		// Give this thread a very short break.
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	// Final cleanup before exiting this thread.
	{
		std::lock_guard<std::recursive_mutex> scopedStaticMutexLock(sMutex);
		auto iterator = sThreadIdToAsyncPipeReaderMap.find(threadIntegerId);
		if (iterator != sThreadIdToAsyncPipeReaderMap.end())
		{
			// Notify the reader if the pipe was externally closed/disconnected.
			auto readerPointer = (*iterator).second;
			if (readerPointer && wasPipeClosed && windowHandle && pipeClosedMessageId)
			{
				::PostMessageW(windowHandle, pipeClosedMessageId, 0, 0);
			}

			// Remove this thread's reference in the static map.
			// This signals the reader in its Stop() method that this thread has exited gracefully.
			sThreadIdToAsyncPipeReaderMap.erase(iterator);
		}
	}
}

#pragma endregion

} }	// namespace Interop::Ipc
