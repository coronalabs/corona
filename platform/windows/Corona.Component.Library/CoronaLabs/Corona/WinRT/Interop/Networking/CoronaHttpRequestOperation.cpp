// ----------------------------------------------------------------------------
// 
// CoronaHttpRequestOperation.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "CoronaHttpRequestOperation.h"
#include "CoronaLabs\Corona\WinRT\CoronaRuntimeEnvironment.h"
#include "CoronaLabs\Corona\WinRT\CoronaRuntimeState.h"
#include "CoronaLabs\Corona\WinRT\Interop\InteropServices.h"
#include "CoronaLabs\Corona\WinRT\Interop\ITimer.h"
#include "CoronaLabs\Corona\WinRT\Interop\ITimerServices.h"
#include "CoronaLabs\Corona\WinRT\Interop\Logging\LoggingServices.h"
#include "CoronaLabs\WinRT\EmptyEventArgs.h"
#include "CoronaLabs\WinRT\ImmutableByteBuffer.h"
#include "CoronaLabs\WinRT\IOperationResult.h"
#include "CoronaLabs\WinRT\MessageEventArgs.h"
#include "CoronaLabs\WinRT\OperationResult.h"
#include "CoronaLabs\WinRT\Utf8String.h"
#include "HttpProgressEventArgs.h"
#include "HttpRequestSettings.h"
#include "HttpResponseEventArgs.h"
#include "IHttpRequestOperation.h"
#include "INetworkServices.h"
#include "Rtt_WinRTPlatform.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Rtt_Event.h"
#	include "Rtt_Lua.h"
#	include "Rtt_LuaContext.h"
#	include "Rtt_LuaLibSystem.h"
#	include "Rtt_Runtime.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Networking {

#pragma region Consructors/Destructors
CoronaHttpRequestOperation::CoronaHttpRequestOperation(int integerId, CoronaRuntimeEnvironment^ environment)
{
	// Validate.
	if (nullptr == environment)
	{
		throw ref new Platform::NullReferenceException();
	}

	// Initialize member variables.
	fIntegerId = integerId;
	fEnvironment = environment;
	fWasAborted = false;
	fLuaListenerReference = nullptr;
	fProgressEventsEnabled = false;
	fProgressEventsDirection = TransmitDirection::kDownload;
	fTimerElapsedEventToken.Value = 0;
	fAbortedEventToken.Value = 0;
	fSendingRequestEventToken.Value = 0;
	fAsyncSendProgressChangedEventToken.Value = 0;
	fAsyncSentRequestEventToken.Value = 0;
	fAsyncReceivingResponseEventToken.Value = 0;
	fAsyncReceiveProgressChangedEventToken.Value = 0;
	fAsyncReceivedResponseEventToken.Value = 0;
	fAsyncErrorOccurredEventToken.Value = 0;

	// Create the HTTP request implementation object.
	fOperation = nullptr;
	if (environment->InteropServices->NetworkServices)
	{
		fOperation = environment->InteropServices->NetworkServices->CreateHttpRequest();
	}

	// Create a timer used to dispatch "networkRequest" events queued by the network thread.
	fTimer = environment->InteropServices->TimerServices->CreateTimer();
	{
		// Set the timer's interval to 100 milliseconds.
		const int64 kIntervalInMilliseconds = 100LL;
		Windows::Foundation::TimeSpan timeSpan;
		timeSpan.Duration = kIntervalInMilliseconds * 10000LL;	// Convert to timespan's units per 100 nanoseconds.
		fTimer->Interval = timeSpan;
	}
}

#pragma endregion


#pragma region Internal Methods/Properties
int CoronaHttpRequestOperation::IntegerId::get()
{
	return fIntegerId;
}

CoronaRuntimeEnvironment^ CoronaHttpRequestOperation::RuntimeEnvironment::get()
{
	return fEnvironment;
}

bool CoronaHttpRequestOperation::ProgressEventsEnabled::get()
{
	return fProgressEventsEnabled;
}

void CoronaHttpRequestOperation::ProgressEventsEnabled::set(bool value)
{
	fProgressEventsEnabled = value;
}

CoronaHttpRequestOperation::TransmitDirection CoronaHttpRequestOperation::ProgressEventsDirection::get()
{
	return fProgressEventsDirection;
}

void CoronaHttpRequestOperation::ProgressEventsDirection::set(CoronaHttpRequestOperation::TransmitDirection value)
{
	fProgressEventsDirection = value;
}

Rtt::Lua::Ref CoronaHttpRequestOperation::LuaListener::get()
{
	return fLuaListenerReference;
}

CoronaHttpRequestOperation::NativeEventHandler& CoronaHttpRequestOperation::EndedEventHandler::get()
{
	return fEndedEventHandler;
}

void CoronaHttpRequestOperation::LuaListener::set(Rtt::Lua::Ref value)
{
	fLuaListenerReference = value;
}

HttpRequestSettings^ CoronaHttpRequestOperation::Settings::get()
{
	if (nullptr == fOperation)
	{
		return nullptr;
	}
	return fOperation->Settings;
}

bool CoronaHttpRequestOperation::IsExecuting::get()
{
	if (nullptr == fOperation)
	{
		return false;
	}
	return fOperation->IsExecuting;
}

CoronaLabs::WinRT::IOperationResult^ CoronaHttpRequestOperation::Execute()
{
	// Do not continue if this networking feature was not provided to the Corona runtime.
	if (nullptr == fOperation)
	{
		return CoronaLabs::WinRT::OperationResult::FailedWith(
					L"The HTTP communications feature is not available to the Corona runtime.");
	}

	// Do not continue if currently executing this operation.
	if (this->IsExecuting)
	{
		return CoronaLabs::WinRT::OperationResult::FailedWith(
					L"Corona's HttpRequestOperation class can only send 1 request at a time.");
	}

	// Initialize last received event data which is used to dispatch "networkRequest" events to Lua.
	{
		// Clear the last received event information, if any.
		for (; fEventDataQueue.empty() == false; fEventDataQueue.pop());
		fLastReceivedEventData.Clear();

		// Convert the URI/URL to UTF-8 so that we don't have to convert it every time we dispatch an event to Lua.
		if (fOperation->Settings->Uri)
		{
			fLastReceivedEventData.Url = CoronaLabs::WinRT::Utf8String::From(fOperation->Settings->Uri->RawUri);
		}
		if (nullptr == fLastReceivedEventData.Url)
		{
			fLastReceivedEventData.Url = CoronaLabs::WinRT::Utf8String::Empty;
		}

		// Flag whether or not the response will be downloaded to file.
		fLastReceivedEventData.DownloadFilePath = fOperation->Settings->DownloadFilePath;
		fLastReceivedEventData.WasResponseDownloadedToFile = (fLastReceivedEventData.DownloadFilePath->IsEmpty() == false);
	}

	// Add all event handlers.
	AddEventHandlers();

	// Start a timer which runs on the main thread.
	// This is used to dispatch "networkRequest" events to Lua that were queued by the network thread.
	fTimer->Start();

	// Attempt to start the HTTP request operation.
	fWasAborted = false;
	auto result = fOperation->Execute();
	if (result->HasFailed)
	{
		OnEnd();
	}

	// Return the result.
	return result;
}

void CoronaHttpRequestOperation::Abort()
{
	// Validate.
	if (nullptr == fOperation)
	{
		return;
	}

	// Abort the operation.
	fWasAborted = true;
	fOperation->Abort();
}

#pragma endregion


#pragma region Private Methods
void CoronaHttpRequestOperation::AddEventHandlers()
{
	// Do not continue if this networking feature is not available.
	if (nullptr == fOperation)
	{
		return;
	}

	// Add all event handlers, if not done already.
	if (0 == fTimerElapsedEventToken.Value)
	{
		fTimerElapsedEventToken = fTimer->Elapsed += ref new Windows::Foundation::TypedEventHandler<ITimer^, CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaHttpRequestOperation::OnTimerElapsed);
	}
	if (0 == fAbortedEventToken.Value)
	{
		fAbortedEventToken = fOperation->Aborted += ref new Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaHttpRequestOperation::OnAborted);
	}
	if (0 == fSendingRequestEventToken.Value)
	{
		fSendingRequestEventToken = fOperation->SendingRequest += ref new Windows::Foundation::EventHandler<HttpProgressEventArgs^>(this, &CoronaHttpRequestOperation::OnSendingRequest);
	}
	if (0 == fAsyncSendProgressChangedEventToken.Value)
	{
		fAsyncSendProgressChangedEventToken = fOperation->AsyncSendProgressChanged += ref new Windows::Foundation::EventHandler<HttpProgressEventArgs^>(this, &CoronaHttpRequestOperation::OnAsyncSendProgressChanged);
	}
	if (0 == fAsyncSentRequestEventToken.Value)
	{
		fAsyncSentRequestEventToken = fOperation->AsyncSentRequest += ref new Windows::Foundation::EventHandler<HttpProgressEventArgs^>(this, &CoronaHttpRequestOperation::OnAsyncSentRequest);
	}
	if (0 == fAsyncReceivingResponseEventToken.Value)
	{
		fAsyncReceivingResponseEventToken = fOperation->AsyncReceivingResponse += ref new Windows::Foundation::EventHandler<HttpResponseEventArgs^>(this, &CoronaHttpRequestOperation::OnAsyncReceivingResponse);
	}
	if (0 == fAsyncReceiveProgressChangedEventToken.Value)
	{
		fAsyncReceiveProgressChangedEventToken = fOperation->AsyncReceiveProgressChanged += ref new Windows::Foundation::EventHandler<HttpProgressEventArgs^>(this, &CoronaHttpRequestOperation::OnAsyncReceiveProgressChanged);
	}
	if (0 == fAsyncReceivedResponseEventToken.Value)
	{
		fAsyncReceivedResponseEventToken = fOperation->AsyncReceivedResponse += ref new Windows::Foundation::EventHandler<HttpResponseEventArgs^>(this, &CoronaHttpRequestOperation::OnAsyncReceivedResponse);
	}
	if (0 == fAsyncErrorOccurredEventToken.Value)
	{
		fAsyncErrorOccurredEventToken = fOperation->AsyncErrorOccurred += ref new Windows::Foundation::EventHandler<CoronaLabs::WinRT::MessageEventArgs^>(this, &CoronaHttpRequestOperation::OnAsyncErrorOccurred);
	}
}

void CoronaHttpRequestOperation::RemoveEventHandlers()
{
	// Do not continue if this networking feature is not available.
	if (nullptr == fOperation)
	{
		return;
	}

	// Remove all event handlers, if not done already.
	if (fTimerElapsedEventToken.Value)
	{
		fTimer->Elapsed -= fTimerElapsedEventToken;
		fTimerElapsedEventToken.Value = 0;
	}
	if (fAbortedEventToken.Value)
	{
		fOperation->Aborted -= fAbortedEventToken;
		fAbortedEventToken.Value = 0;
	}
	if (fSendingRequestEventToken.Value)
	{
		fOperation->SendingRequest -= fSendingRequestEventToken;
		fSendingRequestEventToken.Value = 0;
	}
	if (fAsyncSendProgressChangedEventToken.Value)
	{
		fOperation->AsyncSendProgressChanged -= fAsyncSendProgressChangedEventToken;
		fAsyncSendProgressChangedEventToken.Value = 0;
	}
	if (fAsyncSentRequestEventToken.Value)
	{
		fOperation->AsyncSentRequest -= fAsyncSentRequestEventToken;
		fAsyncSentRequestEventToken.Value = 0;
	}
	if (fAsyncReceivingResponseEventToken.Value)
	{
		fOperation->AsyncReceivingResponse -= fAsyncReceivingResponseEventToken;
		fAsyncReceivingResponseEventToken.Value = 0;
	}
	if (fAsyncReceiveProgressChangedEventToken.Value)
	{
		fOperation->AsyncReceiveProgressChanged -= fAsyncReceiveProgressChangedEventToken;
		fAsyncReceiveProgressChangedEventToken.Value = 0;
	}
	if (fAsyncReceivedResponseEventToken.Value)
	{
		fOperation->AsyncReceivedResponse -= fAsyncReceivedResponseEventToken;
		fAsyncReceivedResponseEventToken.Value = 0;
	}
	if (fAsyncErrorOccurredEventToken.Value)
	{
		fOperation->AsyncErrorOccurred -= fAsyncErrorOccurredEventToken;
		fAsyncErrorOccurredEventToken.Value = 0;
	}
}

void CoronaHttpRequestOperation::DispatchEvent(const EventData &eventData)
{
	// Validate.
	if (!fLuaListenerReference)
	{
		return;
	}

	// Fetch the main Lua state.
	// Note: In order to be backward compatible with older version of Corona's network library,
	//       we must never call a Lua listener from a Lua coroutine.
	lua_State *luaStatePointer = nullptr;
	if (fEnvironment->NativeRuntimePointer)
	{
		luaStatePointer = fEnvironment->NativeRuntimePointer->VMContext().L();
	}
	if (nullptr == fEnvironment->NativeRuntimePointer)
	{
		return;
	}

	// Convert phase to its equivalent Corona string constant.
	const char *phaseName;
	switch (eventData.Phase)
	{
		case TransmitPhase::kStarted:
			phaseName = "began";
			break;
		case TransmitPhase::kTransmitting:
			phaseName = "progress";
			break;
		case TransmitPhase::kEnded:
		default:
			phaseName = "ended";
			break;
	}

	// Push the given data as a "networkRequest" event to Lua.
	Rtt::Lua::NewEvent(luaStatePointer, Rtt::NetworkRequestEvent::kName);
	lua_pushboolean(luaStatePointer, eventData.HasErrorOccurred ? 1 : 0);
	lua_setfield(luaStatePointer, -2, "isError");
	lua_pushstring(luaStatePointer, phaseName);
	lua_setfield(luaStatePointer, -2, "phase");
	lua_pushlightuserdata(luaStatePointer, (void*)fIntegerId);
	lua_setfield(luaStatePointer, -2, "requestId");
	lua_pushstring(luaStatePointer, fLastReceivedEventData.Url->Data ? fLastReceivedEventData.Url->Data : "");
	lua_setfield(luaStatePointer, -2, "url");
	lua_pushinteger(luaStatePointer, (int)eventData.BytesTransferred);
	lua_setfield(luaStatePointer, -2, "bytesTransferred");
	lua_pushinteger(luaStatePointer, (int)(eventData.BytesEstimated));
	lua_setfield(luaStatePointer, -2, "bytesEstimated");
	if (eventData.HasResponse)
	{
		lua_pushinteger(luaStatePointer, eventData.ResponseStatusCode);
		lua_setfield(luaStatePointer, -2, "status");

		if (eventData.ResponseHeaders != nullptr)
		{
			lua_createtable(luaStatePointer, 0, (int)eventData.ResponseHeaders->Size);
			for (auto headerPair : eventData.ResponseHeaders)
			{
				if (headerPair->Key && headerPair->Value)
				{
					lua_pushstring(luaStatePointer, headerPair->Value->Data);
					lua_setfield(luaStatePointer, -2, headerPair->Key->Data);
				}
			}
		}
		else
		{
			lua_newtable(luaStatePointer);
		}
		lua_setfield(luaStatePointer, -2, "responseHeaders");

		if (TransmitPhase::kEnded == eventData.Phase)
		{
			bool isResponseTypeBinary = true;
			if (eventData.WasResponseDownloadedToFile)
			{
				// Convert the file path from UTF-16 to UTF-8.
				auto utf8DownloadFilePath = CoronaLabs::WinRT::Utf8String::From(eventData.DownloadFilePath);
				if (nullptr == utf8DownloadFilePath)
				{
					utf8DownloadFilePath = CoronaLabs::WinRT::Utf8String::Empty;
				}

				// Fetch the base directory for the download file path.
				auto baseDirectoryType =
						fEnvironment->NativePlatformPointer->GetBaseDirectoryFromPath(utf8DownloadFilePath->Data);
				if ((baseDirectoryType < 0) || (baseDirectoryType >= Rtt::MPlatform::kNumDirs))
				{
					// Base directory is unknown. Default to the Documents directory since Lua needs a known type.
					baseDirectoryType = Rtt::MPlatform::kDocumentsDir;
				}
				Rtt::String utf8BaseDirectoryPath(&fEnvironment->NativePlatformPointer->GetAllocator());
				fEnvironment->NativePlatformPointer->PathForFile(nullptr, baseDirectoryType, 0, utf8BaseDirectoryPath);
				size_t utf8BaseDirectoryPathLength = 0;
				if (utf8BaseDirectoryPath.IsEmpty() == false)
				{
					utf8BaseDirectoryPathLength = strlen(utf8BaseDirectoryPath.GetString());
				}

				// Remove the base directory path from the file path.
				const char *utf8RelativeDownloadFilePath = utf8DownloadFilePath->Data;
				if ((utf8DownloadFilePath->ByteCount > (int)utf8BaseDirectoryPathLength) && (utf8BaseDirectoryPathLength > 0))
				{
					if (!_strnicmp(utf8RelativeDownloadFilePath, utf8BaseDirectoryPath.GetString(), utf8BaseDirectoryPathLength))
					{
						utf8RelativeDownloadFilePath += utf8BaseDirectoryPathLength;
						if (('\\' == utf8RelativeDownloadFilePath[0]) || ('/' == utf8RelativeDownloadFilePath[0]))
						{
							utf8RelativeDownloadFilePath++;
						}
					}
				}

				// Push the relative path and base directory to a "response" Lua table.
				lua_createtable(luaStatePointer, 0, 2);
				lua_pushstring(luaStatePointer, utf8RelativeDownloadFilePath);
				lua_setfield(luaStatePointer, -2, "filename");
				Rtt::LuaLibSystem::PushDirectory(luaStatePointer, baseDirectoryType);
				lua_setfield(luaStatePointer, -2, "baseDirectory");
			}
			else
			{
				// Fetch the response bytes.
				auto responseByteBuffer = eventData.ResponseBody;
				if (nullptr == responseByteBuffer)
				{
					responseByteBuffer = CoronaLabs::WinRT::ImmutableByteBuffer::Empty;
				}

//TODO: Parse for the "charset" encoding.
#if 0
				// Fetch the character set encoding from the response headers.
				const char kCharSetName[] = "charset";
				auto characterEncoding = CoronaLabs::WinRT::Utf8String::Empty;
				for (auto&& headerPair : eventData.ResponseHeaders)
				{
					if ((nullptr == headerPair->Key) || (nullptr == headerPair->Value))
					{
						continue;
					}
					if (_stricmp(headerPair->Key->Data, "Content-Type") != 0)
					{
						continue;
					}
					
//					auto lowerCaseHeaderValue = headerPair->Value->ToLower();
				}

				// Attempt to convert the received bytes to UTF-8, if applicable.
#else
				isResponseTypeBinary = false;
#endif

				luaL_Buffer luaBuffer;
				luaL_buffinit(luaStatePointer, &luaBuffer);
				int byteCount = eventData.ResponseBody->Count;
				for (int index = 0; index < byteCount; index++)
				{
					luaL_putchar(&luaBuffer, eventData.ResponseBody->GetByIndex(index));
				}
				luaL_pushresult(&luaBuffer);
			}
			lua_setfield(luaStatePointer, -2, "response");

			lua_pushstring(luaStatePointer, isResponseTypeBinary ? "binary" : "text");
			lua_setfield(luaStatePointer, -2, "responseType");
		}
	}

	// If a network error occurred, then log it now.
	if (eventData.HasErrorOccurred)
	{
		Logging::LoggingServices::Log(eventData.ErrorMessage + L"\r\n");
	}

	// Dispatch the above event to the assigned Lua listener.
	Rtt::Lua::DispatchEvent(luaStatePointer, fLuaListenerReference, 0);
}

void CoronaHttpRequestOperation::AsyncQueueEvent(const EventData &eventData)
{
	// Ignore all events if this operation was aborted.
	if (fWasAborted)
	{
		return;
	}

	// Ignore all events if the Corona runtime is terminating.
	auto runtimeState = fEnvironment->RuntimeState;
	if ((CoronaRuntimeState::Terminating == runtimeState) || (CoronaRuntimeState::Terminated == runtimeState))
	{
		return;
	}

	// Ignore upload/download progress events if the Corona runtime is not currently running.
	// No point in queueing these events since progress cannot be displayed onscreen while suspended.
	if (TransmitPhase::kTransmitting == eventData.Phase)
	{
		if ((runtimeState != CoronaRuntimeState::Starting) &&
		    (runtimeState != CoronaRuntimeState::Running) &&
		    (runtimeState != CoronaRuntimeState::Resuming))
		{
			return;
		}
	}

	// Queue the given event.
	std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);
	fEventDataQueue.push(eventData);
}

void CoronaHttpRequestOperation::OnEnd()
{
	// Remove all event handlers.
	RemoveEventHandlers();

	// Stop the timer used to dispatch queued events to Lua.
	fTimer->Stop();

	// Notify the owner of this operation that execution has ended.
	fEndedEventHandler.Invoke(this);
}

void CoronaHttpRequestOperation::OnTimerElapsed(ITimer ^sender, CoronaLabs::WinRT::EmptyEventArgs ^args)
{
	// Validate.
	if (nullptr == fOperation)
	{
		return;
	}

	// Do not continue if the Corona runtime is not currently running.
	auto runtimeState = fEnvironment->RuntimeState;
	if ((runtimeState != CoronaRuntimeState::Starting) &&
	    (runtimeState != CoronaRuntimeState::Running) &&
	    (runtimeState != CoronaRuntimeState::Resuming))
	{
		return;
	}

	// Pop off all of the events from the main queue and push them into a temporary queue.
	// Note: We do this because we don't want to dispatch events while we have the main queue mutex-locked,
	//       because long blocking Lua event listeners would block the network thread's event handlers too.
	std::queue<EventData> temporaryEventDataQueue;
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(fMutex);
		temporaryEventDataQueue.swap(fEventDataQueue);
	}

	// Dispatch all of the queued events to Lua.
	bool hasOperationEnded = false;
	while (temporaryEventDataQueue.size() > 0)
	{
		// Do not continue if this operation was aborted.
		if (fWasAborted)
		{
			break;
		}

		// Pop off the next event in the queue.
		auto eventData = temporaryEventDataQueue.front();
		temporaryEventDataQueue.pop();

		// Optimization: Do not dispatch this event if it and the next event in the queue are progress events.
		//               There is no point in dispatching 2 or more progress events back-to-back.
		if (TransmitPhase::kTransmitting == eventData.Phase)
		{
			if ((temporaryEventDataQueue.size() > 0) &&
			    (TransmitPhase::kTransmitting == temporaryEventDataQueue.front().Phase))
			{
				continue;
			}
		}

		// Dispatch this event to this operation's Lua listener.
		DispatchEvent(eventData);

		// Stop now if an "ended" event was dispatched.
		// This indicates that this operation completed or errored out.
		if (TransmitPhase::kEnded == eventData.Phase)
		{
			hasOperationEnded = true;
			break;
		}
	}

	// If this operation has ended, then clean up and raise an "Ended" event.
	if (hasOperationEnded || fWasAborted)
	{
		OnEnd();
	}
}

void CoronaHttpRequestOperation::OnAborted(Platform::Object ^sender, CoronaLabs::WinRT::EmptyEventArgs ^args)
{
	fWasAborted = true;
	OnEnd();
}

void CoronaHttpRequestOperation::OnSendingRequest(Platform::Object ^sender, HttpProgressEventArgs ^args)
{
	// Do not continue if not set up to dispatch upload progress events.
	if (!fProgressEventsEnabled || (fProgressEventsDirection != TransmitDirection::kUpload))
	{
		return;
	}

	// Update event data.
	fLastReceivedEventData.Phase = TransmitPhase::kStarted;
	fLastReceivedEventData.BytesTransferred = 0;
	if (args->IsTotalBytesToTransferKnown)
	{
		fLastReceivedEventData.BytesEstimated = args->TotalBytesToTransfer;
	}
	else
	{
		fLastReceivedEventData.BytesEstimated = -1;
	}

	// Queue the event to be dispatched to Lua later.
	AsyncQueueEvent(fLastReceivedEventData);
}

void CoronaHttpRequestOperation::OnAsyncSendProgressChanged(Platform::Object ^sender, HttpProgressEventArgs ^args)
{
	// Do not continue if not set up to dispatch upload progress events.
	if (!fProgressEventsEnabled || (fProgressEventsDirection != TransmitDirection::kUpload))
	{
		return;
	}

	// Update event data.
	fLastReceivedEventData.Phase = TransmitPhase::kTransmitting;
	fLastReceivedEventData.BytesTransferred = args->BytesTransferred;

	// Queue the event to be dispatched to Lua later.
	AsyncQueueEvent(fLastReceivedEventData);
}

void CoronaHttpRequestOperation::OnAsyncSentRequest(Platform::Object ^sender, HttpProgressEventArgs ^args)
{
	// Do not continue if not set up to dispatch upload progress events.
	if (!fProgressEventsEnabled || (fProgressEventsDirection != TransmitDirection::kUpload))
	{
		return;
	}

	// Update event data.
	// Note: Do not set the phase to Ended until a response has been received,
	//       which confirms that the server has received the sent request.
	bool hasProgressChanged = (fLastReceivedEventData.BytesTransferred != args->BytesTransferred);
	fLastReceivedEventData.BytesTransferred = args->BytesTransferred;
	fLastReceivedEventData.Phase = TransmitPhase::kTransmitting;

	// Queue the event to be dispatched to Lua later, but only if the byte count has changed/progressed.
	if (hasProgressChanged)
	{
		AsyncQueueEvent(fLastReceivedEventData);
	}
}

void CoronaHttpRequestOperation::OnAsyncReceivingResponse(Platform::Object ^sender, HttpResponseEventArgs ^args)
{
	// Convert the response headers to UTF-8.
	// This is an optimization so that we don't hvae to convert the text for every progress event dispatched to Lua.
	auto utf8Headers = ref new Platform::Collections::Map<
			CoronaLabs::WinRT::Utf8String^, CoronaLabs::WinRT::Utf8String^, CoronaLabs::WinRT::Utf8String::LessThanComparer>();
	for (auto&& utf16HeaderPair : args->Headers)
	{
		// Convert the UTF-16 header pair to UTF-8.
		auto utf8HeaderName = CoronaLabs::WinRT::Utf8String::From(utf16HeaderPair->Key);
		auto utf8HeaderValue = CoronaLabs::WinRT::Utf8String::From(utf16HeaderPair->Value);

		// Header name must be a non-empty string.
		if (!utf8HeaderName || utf8HeaderName->IsEmpty)
		{
			continue;
		}

		// Header value cannot be null, but empty string is okay.
		if (!utf8HeaderValue)
		{
			continue;
		}

		// Add the UTF-8 header pair to the collection.
		utf8Headers->Insert(utf8HeaderName, utf8HeaderValue);
	}

	// Update event data's core response information.
	fLastReceivedEventData.HasResponse = true;
	fLastReceivedEventData.ResponseStatusCode = args->StatusCode;
	fLastReceivedEventData.ResponseHeaders = utf8Headers->GetView();

	// Update event data with estimated bytes to be received if:
	// - Progress events have not been enabled. (So, that we can provide this estimate for the "ended" phase.)
	// - Download progress events have been enabled.
	if (!fProgressEventsEnabled || (TransmitDirection::kDownload == fProgressEventsDirection))
	{
		// Fetch the "Content-Length" header value, if available.
		// This will be the estimated number of bytes to download.
		int64 contentLength = -1;
		for (auto&& utf8HeaderPair : utf8Headers)
		{
			if (_stricmp(utf8HeaderPair->Key->Data, "Content-Length") == 0)
			{
				if (utf8HeaderPair->Value->IsNotEmpty)
				{
					contentLength = _atoi64(utf8HeaderPair->Value->Data);
				}
				break;
			}
		}

		// Set the estimated bytes to download.
		fLastReceivedEventData.BytesTransferred = 0;
		fLastReceivedEventData.BytesEstimated = (contentLength > 0) ? contentLength : -1;
	}
	
	// Queue the event if download progress events were enabled.
	if (fProgressEventsEnabled && (TransmitDirection::kDownload == fProgressEventsDirection))
	{
		AsyncQueueEvent(fLastReceivedEventData);
	}
}

void CoronaHttpRequestOperation::OnAsyncReceiveProgressChanged(Platform::Object ^sender, HttpProgressEventArgs ^args)
{
	// Update event data with total bytes transmitted if:
	// - Progress events have not been enabled. (So, that we can provide the byte total for the "ended" phase.)
	// - Download progress events have been enabled.
	if (!fProgressEventsEnabled || (TransmitDirection::kDownload == fProgressEventsDirection))
	{
		fLastReceivedEventData.Phase = TransmitPhase::kTransmitting;
		fLastReceivedEventData.BytesTransferred = args->BytesTransferred;
	}

	// Queue the event if download progress events were enabled.
	if (fProgressEventsEnabled && (TransmitDirection::kDownload == fProgressEventsDirection))
	{
		AsyncQueueEvent(fLastReceivedEventData);
	}
}

void CoronaHttpRequestOperation::OnAsyncReceivedResponse(Platform::Object ^sender, HttpResponseEventArgs ^args)
{
	// Update event data.
	fLastReceivedEventData.Phase = TransmitPhase::kEnded;
	if (args->Body)
	{
		fLastReceivedEventData.ResponseBody = args->Body;
	}
	else
	{
		fLastReceivedEventData.ResponseBody = CoronaLabs::WinRT::ImmutableByteBuffer::Empty;
	}

	// Queue the event to be dispatched to Lua later.
	AsyncQueueEvent(fLastReceivedEventData);
}

void CoronaHttpRequestOperation::OnAsyncErrorOccurred(Platform::Object ^sender, CoronaLabs::WinRT::MessageEventArgs ^args)
{
	// Update event data with error information.
	fLastReceivedEventData.Phase = TransmitPhase::kEnded;
	fLastReceivedEventData.HasErrorOccurred = true;
	fLastReceivedEventData.ErrorMessage = args->Message;

	// Queue the event to be dispatched to Lua later.
	AsyncQueueEvent(fLastReceivedEventData);
}

#pragma endregion


#pragma region Private EventData Functions
CoronaHttpRequestOperation::EventData::EventData()
{
	Clear();
}

void CoronaHttpRequestOperation::EventData::Clear()
{
	this->Phase = TransmitPhase::kStarted;
	this->BytesTransferred = 0;
	this->BytesEstimated = 0;
	this->HasResponse = false;
	this->ResponseStatusCode = -1;
	this->ResponseHeaders = nullptr;
	this->ResponseBody = CoronaLabs::WinRT::ImmutableByteBuffer::Empty;
	this->WasResponseDownloadedToFile = false;
	this->DownloadFilePath = nullptr;
	this->HasErrorOccurred = false;
	this->ErrorMessage = nullptr;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Networking
