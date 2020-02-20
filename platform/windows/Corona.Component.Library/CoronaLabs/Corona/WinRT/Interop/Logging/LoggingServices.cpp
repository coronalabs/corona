//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "LoggingServices.h"
#include "CoronaLabs\WinRT\NativeStringServices.h"
#include <mutex>
#include <queue>
#include <thread>
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core\Rtt_Assert.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Logging {

#pragma region Static Member Variables
/// <summary>Mutex used to synchronize access to this class' member variables.</summary>
static std::recursive_mutex sMutex;

/// <summary>Queue of log messages to be outputed asynchronously on another thread.</summary>
static std::queue<Platform::String^> sLogMessageQueue;

/// <summary>
///  <para>The logging implementation to be used by this class.</para>
///  <para>Typically an instance to a NativeDebuggerLogger or DotNetDebuggerLogger class.</para>
///  <para>Can be null if a logging implementation was not provided, in which case, nothing should be logged.</para>
/// </summary>
static ILogger^ sLogger = nullptr;

/// <summary>Set true if this class should log all messages asynchronously on another thread.</summary>
static bool sIsAsyncLoggingEnabled = true;

/// <summary>
///  <para>
///   Set true if a thread is currently executing in function OnExecuteThread(), meaning it is available to
///   log more messages in the queue.
///  </para>
///  <para>
///   Set false if a thread has not been created yet or the last one has been terminated. In this case,
///   it's this class' responsibility to spawn a new thread in order to log more queued messages.
///  </para>
/// </summary>
static bool sIsThreadAvailable = false;

#pragma endregion


#pragma region Private Static Functions
/// <summary>
///  <para>Called when Corona's Rtt_Log() and Rtt_LogException() functions have been called.</para>
///  <para>Will also be called when Lua's print() function has been called, which is routed to Rtt_LogException().</para>
///  <para>Logs the received message to this class' given "Logger" implementation.</para>
/// </summary>
/// <param name="message">The message to be logged. Will be ignored if null or empty.</param>
void OnCoronaLogging(const char *message)
{
	// Do not continue if the given message is null or empty.
	if (!message || ('\0' == message[0]))
	{
		return;
	}

	// Convert the given message to UTF-16.
	auto utf16Message = CoronaLabs::WinRT::NativeStringServices::Utf16FromUtf8(message);

	// Log the given message.
	LoggingServices::Log(utf16Message);
}

/// <summary>Called by this class' worker thread to log queued messages asynchronously for best performance.</summary>
void OnExecuteThread()
{
	// Temporary queue that "sLogMessageQueue" pops its items to on every async logging pass.
	std::queue<Platform::String^> temporaryQueue;

	// Max time to wait for new messages until we should terminate this thread.
	static const int64 kTimeoutInMilliseconds = 500;

	// Have this thread poll for queued messages until the scheduled termination time occurs.
	auto terminationTimeInTicks = (int64)::GetTickCount64() + kTimeoutInMilliseconds;
	while (true)
	{
		// Wait for more queued messages to write.
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		// Fetch the currently assigned logger.
		auto logger = sLogger;
		bool canWrite = (logger && logger->CanWrite);

		// Pop all of the main queue's messages to this function's temporary queue.
		{
			std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
			while (sLogMessageQueue.empty() == false)
			{
				if (canWrite)
				{
					temporaryQueue.push(sLogMessageQueue.front());
				}
				sLogMessageQueue.pop();
			}
		}

		// Write all queued log messages.
		bool didWrite = false;
		if (temporaryQueue.size() > 0)
		{
			static const int kMaxConcatenatedMessageLength = 1024; //10240;
			auto concatenatedMessage = ref new Platform::Array<wchar_t>(kMaxConcatenatedMessageLength);
			int concatenatedMessageLength = 0;
			while (temporaryQueue.empty() == false)
			{
				// Pop off the next log message.
				auto nextString = temporaryQueue.front();
				temporaryQueue.pop();

				// Log all messages in the concatenated string buffer if full.
				if ((nextString->Length() + concatenatedMessageLength) >= kMaxConcatenatedMessageLength)
				{
					// Log the concatenated messages.
					try { logger->Write(concatenatedMessage); }
					catch (...) {}

					// Reset the string buffer for the next set of queued messages.
					concatenatedMessageLength = 0;
				}

				// Concatenate the next message to the string buffer, if there is room.
				if ((nextString->Length() + concatenatedMessageLength) >= kMaxConcatenatedMessageLength)
				{
					// The next message is too big to fit in the buffer. Log it now.
					try { logger->Write(nextString); }
					catch (...) {}
				}
				else
				{
					// Concatenate the next message to the string buffer.
					wcscpy_s(
						concatenatedMessage->Data + concatenatedMessageLength,
						kMaxConcatenatedMessageLength - concatenatedMessageLength,
						nextString->Data());
					concatenatedMessageLength += nextString->Length();
				}
			}
			if (concatenatedMessageLength > 0)
			{
				try { logger->Write(concatenatedMessage); }
				catch (...) {}
			}
			didWrite = true;
		}

		// Determine if we should wait for more messages or give up and terminate this thread.
		{
			std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
			if (didWrite || (sLogMessageQueue.size() > 0))
			{
				// Bump up this thread's scheduled termination if:
				// 1) We just wrote at least 1 message. (We're anticipating more messages in the near future.)
				// 2) We have more messages queued.
				terminationTimeInTicks = (int64)::GetTickCount64() + kTimeoutInMilliseconds;
			}
			else if ((terminationTimeInTicks - (int64)::GetTickCount64()) > 0)
			{
				// We haven't logged any messages in a while. Time to terminate this thread.
				sIsThreadAvailable = false;
				break;
			}
		}
	}
}

#pragma endregion


#pragma region Constructors/Destructors
LoggingServices::LoggingServices()
{
}

#pragma endregion


#pragma region Public Static Functions/Properties
bool LoggingServices::AsyncLoggingEnabled::get()
{
	return sIsAsyncLoggingEnabled;
}

void LoggingServices::AsyncLoggingEnabled::set(bool value)
{
	sIsAsyncLoggingEnabled = value;
}

ILogger^ LoggingServices::Logger::get()
{
	auto logHandler = Rtt_LogGetHandler();
	if (!logHandler || (&OnCoronaLogging != logHandler))
	{
		Rtt_LogSetHandler(&OnCoronaLogging);
	}

	return sLogger;
}

void LoggingServices::Logger::set(ILogger^ logger)
{
	sLogger = logger;
}

void LoggingServices::Log(Platform::String^ message)
{
	// Do not continue if there is no message to log.
	if (message->IsEmpty())
	{
		return;
	}

	// Do not continue if a logger is not available.
	auto logger = sLogger;
	if (!logger || (logger->CanWrite == false))
	{
		return;
	}

	// Determine if the given message should be logged now or asynchronously.
	if (sIsAsyncLoggingEnabled)
	{
		// Queue the given message to be logged on another thread.
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		{
			// Do not queue the message if the thread can't keep up.
			// This prevents run-away memory usage.
			if (sLogMessageQueue.size() > 1000)
			{
				return;
			}

			// Queue the message.
			sLogMessageQueue.push(message);

			// Create a thread to output the queued message(s), if not already available.
			if (false == sIsThreadAvailable)
			{
				sIsThreadAvailable = true;
				std::thread newThread(&OnExecuteThread);
				newThread.detach();
			}
		}
	}
	else
	{
		// Async logging is disabled. Log it now.
		// Note: Microsoft's Native and Managed debugger logging functions block the thread for about 4 milliseconds.
		logger->Write(message);
	}

}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Logging
