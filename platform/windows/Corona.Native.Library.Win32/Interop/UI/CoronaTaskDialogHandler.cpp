//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CoronaTaskDialogHandler.h"
#include "Interop\MDeviceSimulatorServices.h"
#include "Interop\RuntimeState.h"
#include "RenderSurfaceControl.h"
#include "Rtt_LuaLibNative.h"
#include "TaskDialog.h"
#include "Window.h"
#include <algorithm>
#include <list>
#include <mutex>


namespace Interop { namespace UI {

#pragma region Private Static Member Variables
/// <summary>
///  <para>Stores a collection of all active task dialog handlers used by all active Corona runtimes.</para>
///  <para>
///   A handler is expected to be removed from this collection after it has been closed and its events dispatched.
///  </para>
/// </summary>
static std::list<std::shared_ptr<CoronaTaskDialogHandler>> sHandlerCollection;

/// <summary>Mutex used to synchronize access to the "sHandlerCollection" static variable between threads.</summary>
static std::recursive_mutex sMutex;

#pragma endregion


#pragma region Constructors/Destructors
CoronaTaskDialogHandler::CoronaTaskDialogHandler(RuntimeEnvironment& environment)
:	fEnvironment(environment),
	fLuaResourcePointer(nullptr),
	fReservedMessageId(0),
	fTaskDialogPointer(nullptr),
	fHasEventData(false),
	fRuntimeResumedEventHandler(&CoronaTaskDialogHandler::OnRuntimeResumed),
	fRuntimeTerminatingEventHandler(&CoronaTaskDialogHandler::OnRuntimeTerminating),
	fReceivedMessageEventHandler(&CoronaTaskDialogHandler::OnReceivedMessage)
{
	// Add runtime event handlers.
	fEnvironment.GetResumedEventHandlers().Add(&fRuntimeResumedEventHandler);
	fEnvironment.GetTerminatingEventHandlers().Add(&fRuntimeTerminatingEventHandler);

	// Reserve a unique message ID from the runtime's message-only window and subscribe to its events.
	MessageOnlyWindow& messageOnlyWindow = fEnvironment.GetMessageOnlyWindow();
	fReservedMessageId = messageOnlyWindow.ReserveMessageId();
	messageOnlyWindow.GetReceivedMessageEventHandlers().Add(&fReceivedMessageEventHandler);

	// Initialize event data.
	fEventData.WasCanceled = true;
	fEventData.ButtonIndex = -1;
}

CoronaTaskDialogHandler::~CoronaTaskDialogHandler()
{
	// Remove event handlers.
	fEnvironment.GetResumedEventHandlers().Remove(&fRuntimeResumedEventHandler);
	fEnvironment.GetTerminatingEventHandlers().Remove(&fRuntimeTerminatingEventHandler);
	fEnvironment.GetMessageOnlyWindow().GetReceivedMessageEventHandlers().Remove(&fReceivedMessageEventHandler);

	// Release the reserved Win32 message ID.
	fEnvironment.GetMessageOnlyWindow().UnreserveMessageId(fReservedMessageId);
}

#pragma endregion


#pragma region Public Methods
Rtt::NativeAlertRef CoronaTaskDialogHandler::GetNativeAlertReference() const
{
	return (Rtt::NativeAlertRef)this;
}

bool CoronaTaskDialogHandler::IsShowing() const
{
	if (fTaskDialogPointer && fTaskDialogPointer->IsShowing())
	{
		return true;
	}
	return false;
}

void CoronaTaskDialogHandler::Close()
{
	if (fTaskDialogPointer)
	{
		fTaskDialogPointer->Close();
	}
}

bool CoronaTaskDialogHandler::CloseWithButtonIndex(int index)
{
	if (fTaskDialogPointer)
	{
		return fTaskDialogPointer->CloseWithButtonIndex(index);
	}
	return false;
}

#pragma endregion


#pragma region Public Static Functions
CoronaTaskDialogHandler::ShowResult CoronaTaskDialogHandler::ShowUsing(
	const CoronaTaskDialogHandler::ShowSettings& settings)
{
	// Do not continue if not provide a Corona runtime.
	if (!settings.RuntimeEnvironmentPointer)
	{
		Rtt_ASSERT(0);
		return ShowResult::FailedWith(L"Cannot display a native alert without a Corona runtime reference.");
	}

	// Do not continue if the Corona runtime is being terminated.
	auto runtimeState = settings.RuntimeEnvironmentPointer->GetRuntimeState();
	if ((RuntimeState::kTerminating == runtimeState) || (RuntimeState::kTerminated == runtimeState))
	{
		return ShowResult::FailedWith(L"Cannot display a native alert while the Corona runtime is terminating.");
	}

	// Create a new handler to manage the dialog to be shown down below.
	auto handlerPointer = std::shared_ptr<CoronaTaskDialogHandler>(
			new CoronaTaskDialogHandler(*settings.RuntimeEnvironmentPointer));

	// Create a new task dialog and copy the given settings to it.
	handlerPointer->fTaskDialogPointer = new TaskDialog();
	TaskDialogSettings& dialogSettings = handlerPointer->fTaskDialogPointer->GetSettings();
	WinString stringTranscoder;
	stringTranscoder.SetUTF8(settings.Title);
	dialogSettings.SetTitleText(stringTranscoder.GetUTF16());
	stringTranscoder.SetUTF8(settings.Message);
	dialogSettings.SetMessageText(stringTranscoder.GetUTF16());
	dialogSettings.SetCancelButtonEnabled(settings.IsCancelButtonEnabled);
	dialogSettings.GetButtonLabels().clear();
	if (settings.ButtonLabels && (settings.ButtonLabelCount > 0))
	{
		for (int index = 0; index < (int)settings.ButtonLabelCount; index++)
		{
			stringTranscoder.SetUTF8(settings.ButtonLabels[index]);
			dialogSettings.GetButtonLabels().push_back(stringTranscoder.GetUTF16());
		}
	}
	auto deviceSimulatorServicesPointer = settings.RuntimeEnvironmentPointer->GetDeviceSimulatorServices();
	if (deviceSimulatorServicesPointer && deviceSimulatorServicesPointer->IsAlertButtonOrderRightToLeft())
	{
		dialogSettings.SetReverseHorizontalButtonsEnabled(true);
	}

	// Store the given Lua resource, which reference a Lua listener to be invoked when the message box has been closed.
	// Note: This is optional. Will be null if a Lua listener was not provided by the caller.
	handlerPointer->fLuaResourcePointer = settings.LuaResourcePointer;

	// Post a message via the message-only window to have the dialog displayed in a non-blocking way.
	auto wasPosted = ::PostMessageW(
			handlerPointer->fEnvironment.GetMessageOnlyWindow().GetWindowHandle(),
			handlerPointer->fReservedMessageId, 0, 0);
	if (!wasPosted)
	{
		// This should never happen unless something is seriously wrong with the Windows message pump.
		return ShowResult::FailedWith(L"Failed to display a native alert via a Win32 PostMessage() call.");
	}

	// Add this handler to the global collection. This serves the following purposes:
	// 1) Tracks all Corona owned task dialogs that are currently shown.
	// 2) Keeps the C++ shared pointer reference for this handler alive until the dialog has been closed.
	if (!FetchShownDialogFor(handlerPointer->GetNativeAlertReference()))
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		sHandlerCollection.push_back(std::shared_ptr<CoronaTaskDialogHandler>(handlerPointer));
	}

	// Return a success result.
	return ShowResult::SucceededWith(handlerPointer);
}

std::shared_ptr<CoronaTaskDialogHandler> CoronaTaskDialogHandler::FetchShownDialogFor(Rtt::NativeAlertRef value)
{
	// Attempt to fetch a dialog handler by its Corona alert reference.
	if (value)
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		for (auto&& handlerPointer : sHandlerCollection)
		{
			if (handlerPointer && (handlerPointer->GetNativeAlertReference() == value) && handlerPointer->IsShowing())
			{
				return handlerPointer;
			}
		}
	}

	// Failed to find dialog handler. Return an empty shared pointer.
	return std::shared_ptr<CoronaTaskDialogHandler>();
}

bool CoronaTaskDialogHandler::IsShowingDialogFor(const RuntimeEnvironment& environment)
{
	std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
	for (auto&& handlerPointer : sHandlerCollection)
	{
		if (handlerPointer && (&environment == &handlerPointer->fEnvironment) && handlerPointer->IsShowing())
		{
			return true;
		}
	}
	return false;
}

bool CoronaTaskDialogHandler::HasPendingHandlersFor(const RuntimeEnvironment& environment)
{
	std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
	for (auto&& handlerPointer : sHandlerCollection)
	{
		if (handlerPointer && (&environment == &handlerPointer->fEnvironment) && handlerPointer->fTaskDialogPointer)
		{
			return true;
		}
	}
	return false;
}

#pragma endregion


#pragma region Private Methods
void CoronaTaskDialogHandler::DispatchCompletionEvent()
{
	// Do not continue if we have not received a dialog result yet.
	if (!fHasEventData)
	{
		return;
	}

	// Do not continue if the Corona runtime is not currently active.
	RuntimeState runtimeState;
	runtimeState = fEnvironment.GetRuntimeState();
	if ((runtimeState != RuntimeState::kStarting) &&
	    (runtimeState != RuntimeState::kRunning) &&
	    (runtimeState != RuntimeState::kTerminating))
	{
		return;
	}
	if ((RuntimeState::kTerminating == runtimeState) && fEnvironment.GetRuntime()->IsSuspended())
	{
		return;
	}

	// Invoke the Lua listener.
	if (fLuaResourcePointer)
	{
		// Dispatch a Lua "completion" event.
		Rtt::LuaLibNative::AlertComplete(*fLuaResourcePointer, fEventData.ButtonIndex, fEventData.WasCanceled);

		// Delete the Lua listener reference.
		Rtt_DELETE(fLuaResourcePointer);
	}

	// Remove this handler from the global collection.
	RemoveFromStaticCollection(this);
}

#pragma endregion


#pragma region Private Static Functions
bool CoronaTaskDialogHandler::RemoveFromStaticCollection(const CoronaTaskDialogHandler* handlerPointer)
{
	if (handlerPointer)
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		for (auto iter = sHandlerCollection.begin(); iter != sHandlerCollection.end(); iter++)
		{
			if ((*iter).get() == handlerPointer)
			{
				sHandlerCollection.erase(iter);
				return true;
			}
		}
	}
	return false;
}

bool CoronaTaskDialogHandler::RemoveFromStaticCollection(const std::shared_ptr<CoronaTaskDialogHandler> handlerPointer)
{
	std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
	auto iter = std::find(sHandlerCollection.begin(), sHandlerCollection.end(), handlerPointer);
	if (iter != sHandlerCollection.end())
	{
		sHandlerCollection.erase(iter);
		return true;
	}
	return false;
}

void CoronaTaskDialogHandler::OnRuntimeResumed(RuntimeEnvironment& sender, const EventArgs& arguments)
{
	// Fetch and queue all dialog handlers associated with the Corona runtime that is about to be resumed.
	std::queue<std::shared_ptr<CoronaTaskDialogHandler>> handlerQueue;
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		for (auto handlerPointer : sHandlerCollection)
		{
			if (&handlerPointer->fEnvironment == &sender)
			{
				handlerQueue.push(handlerPointer);
			}
		}
	}

	// Traverse all dialog handlers and dispatch any pending "completion" events.
	// This handles the case where a dialog was closed while the Corona runtime was suspend.
	while (handlerQueue.size() > 0)
	{
		auto handlerPointer = handlerQueue.front();
		handlerQueue.pop();
		if (handlerPointer->fHasEventData)
		{
			handlerPointer->DispatchCompletionEvent();
		}
	}
}

void CoronaTaskDialogHandler::OnRuntimeTerminating(RuntimeEnvironment& sender, const EventArgs& arguments)
{
	// Fetch and queue all dialog handlers associated with the Corona runtime that is about to be terminated.
	std::queue<std::shared_ptr<CoronaTaskDialogHandler>> handlerQueue;
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		for (auto handlerPointer : sHandlerCollection)
		{
			if (&handlerPointer->fEnvironment == &sender)
			{
				handlerQueue.push(handlerPointer);
			}
		}
	}

	// Destroy the queued handlers.
	while (handlerQueue.size() > 0)
	{
		// Pop off the next handler to be deleted.
		auto handlerPointer = handlerQueue.front();
		handlerQueue.pop();

		// If this handler is displaying a dialog, then close it now.
		if (handlerPointer->IsShowing())
		{
			// Close the dialog.
			handlerPointer->Close();

			// Dispatch a canceled "completion" event.
			// We must do so now since this is our last chance to interact with Lua.
			if (handlerPointer->fLuaResourcePointer)
			{
				handlerPointer->fEventData.WasCanceled = true;
				handlerPointer->fEventData.ButtonIndex = -1;
				handlerPointer->fHasEventData = true;
				handlerPointer->DispatchCompletionEvent();
				handlerPointer->fLuaResourcePointer = nullptr;
			}
		}

		// Remove the handler from the global collection.
		RemoveFromStaticCollection(handlerPointer);
	}
}

void CoronaTaskDialogHandler::OnReceivedMessage(UIComponent& sender, HandleMessageEventArgs& arguments)
{
	// Fetch the dialog handler that posted the received message by its unique message ID.
	// Note: Do not fetch a shared pointer here because the dialog shown below can outlive the Corona runtime.
	//       We must allows the runtime's "Terminating" event to delete the handler to prevent a crash in its destructor.
	auto messageId = arguments.GetMessageId();
	CoronaTaskDialogHandler* weakHandlerPointer = nullptr;
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		for (auto&& nextSharedHandlerPointer : sHandlerCollection)
		{
			if (nextSharedHandlerPointer && (nextSharedHandlerPointer->fReservedMessageId == messageId))
			{
				weakHandlerPointer = nextSharedHandlerPointer.get();
				break;
			}
		}
	}
	if (!weakHandlerPointer)
	{
		return;
	}

	// Fetch the handler's dialog pointer.
	auto dialogPointer = weakHandlerPointer->fTaskDialogPointer;
	if (!dialogPointer)
	{
		RemoveFromStaticCollection(weakHandlerPointer);
		return;
	}

	// Assign the dialog a parent window under the runtime's window hierarchy.
	HWND windowHandle = nullptr;
	{
		// If we're currently showing dialogs now, then fetch the top-most dialog's window handle.
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		for (auto&& nextSharedHandlerPointer : sHandlerCollection)
		{
			if (&nextSharedHandlerPointer->fEnvironment == &weakHandlerPointer->fEnvironment)
			{
				if (nextSharedHandlerPointer->fTaskDialogPointer && nextSharedHandlerPointer->IsShowing())
				{
					windowHandle = nextSharedHandlerPointer->fTaskDialogPointer->GetWindowHandle();
					if (windowHandle && !::GetLastActivePopup(windowHandle))
					{
						break;
					}
				}
			}
		}
		if (!windowHandle)
		{
			// No dialogs are currently shown.
			// Use the Corona runtime's main window or surface control as the dialog's parent.
			auto windowPointer = weakHandlerPointer->fEnvironment.GetMainWindow();
			if (windowPointer)
			{
				windowHandle = windowPointer->GetWindowHandle();
			}
			if (!windowHandle)
			{
				auto renderSurfacePointer = weakHandlerPointer->fEnvironment.GetRenderSurface();
				if (renderSurfacePointer)
				{
					windowHandle = renderSurfacePointer->GetWindowHandle();
				}
			}
		}
	}
	dialogPointer->GetSettings().SetParentWindowHandle(windowHandle);

	// Show the dialog. (Blocks until the dialog is closed.)
	auto showResult = dialogPointer->Show();

	// Fetch the dialog's results.
	EventData eventData{};
	eventData.WasCanceled = (dialogPointer->WasButtonPressed() == false);
	eventData.ButtonIndex = -1;
	if (dialogPointer->WasButtonPressed())
	{
		eventData.ButtonIndex = dialogPointer->GetLastPressedButtonIndex();
	}

	// Delete the displayed dialog.
	delete dialogPointer;
	dialogPointer = nullptr;

	// Flag that this message was handled.
	arguments.SetHandled();

	// Fetch a shared pointer to the dialog handler, if still available.
	// We need it to keep the object alive since the DispatchCompletionEvent() method below can dereference it.
	std::shared_ptr<CoronaTaskDialogHandler> sharedHandlerPointer;
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		for (auto&& nextSharedHandlerPointer : sHandlerCollection)
		{
			if (nextSharedHandlerPointer && (nextSharedHandlerPointer.get() == weakHandlerPointer))
			{
				sharedHandlerPointer = nextSharedHandlerPointer;
				break;
			}
		}
	}
	if (!sharedHandlerPointer)
	{
		// Handler is no longer available.
		// This can happen if the Corona runtime was terminated while displaying the above dialog.
		return;
	}

	// Null out the handler's dialog pointer that was deleted up above.
	sharedHandlerPointer->fTaskDialogPointer = nullptr;

	// Dispatch the above event to Lua.
	// If the runtime is currently suspended, then this method will do nothing.
	sharedHandlerPointer->fEventData = eventData;
	sharedHandlerPointer->fHasEventData = true;
	sharedHandlerPointer->DispatchCompletionEvent();
}

#pragma endregion

} }	// namespace Interop::UI
