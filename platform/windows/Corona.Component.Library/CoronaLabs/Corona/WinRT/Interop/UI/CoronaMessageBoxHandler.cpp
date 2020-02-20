//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "CoronaMessageBoxHandler.h"
#include "CoronaLabs\Corona\WinRT\CoronaRuntimeEnvironment.h"
#include "CoronaLabs\Corona\WinRT\CoronaRuntimeState.h"
#include "CoronaLabs\Corona\WinRT\Interop\InteropServices.h"
#include "CoronaLabs\WinRT\EmptyEventArgs.h"
#include "CoronaLabs\WinRT\OperationResult.h"
#include "IMessageBox.h"
#include "IUserInterfaceServices.h"
#include "MessageBoxSettings.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core\Rtt_Build.h"
#	include "Rtt_LuaLibNative.h"
#	include "Rtt_LuaResource.h"
#	include "Rtt_Runtime.h"
#	include <unordered_map>
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace UI {

#pragma region Static Private Member Variables
/// <summary>Defines a hash table of CoronaMessageBoxHandler objects using their NativeAlertRef as the key.</summary>
typedef std::unordered_map<Rtt::NativeAlertRef, CoronaMessageBoxHandler^> HandlerMap;

/// <summary>Defines a pair for the HandlerMap type. Used to insert and foreach elements in the HandlerMap collection.</summary>
typedef std::pair<Rtt::NativeAlertRef, CoronaMessageBoxHandler^> HandlerMapPair;

/// <summary>Static hash table of all handlers referencing message boxes that are currently shown onscreen.</summary>
static HandlerMap sOpenMessagBoxHandlerCollection;

#pragma endregion


#pragma region Consructors/Destructors
CoronaMessageBoxHandler::CoronaMessageBoxHandler(CoronaRuntimeEnvironment ^environment)
{
	// Validate.
	if (nullptr == environment)
	{
		throw ref new Platform::NullReferenceException();
	}

	// Initialize core member variables.
	fEnvironment = environment;
	fMessageBox = nullptr;
	fMessageBoxClosedEventToken.Value = 0;
	fRuntimeResumedEventToken.Value = 0;
	fRuntimeTerminatingEventToken.Value = 0;

	// Attempt to create the native message box.
	auto userInterfaceServices = fEnvironment->InteropServices->UserInterfaceServices;
	if (userInterfaceServices)
	{
		fMessageBox = userInterfaceServices->CreateMessageBox();
	}
}

#pragma endregion


#pragma region Internal Methods/Properties
Rtt::NativeAlertRef CoronaMessageBoxHandler::NativeAlertReference::get()
{
	return (void*)this;
}

bool CoronaMessageBoxHandler::IsSupported::get()
{
	return (fMessageBox != nullptr);
}

bool CoronaMessageBoxHandler::IsShowing::get()
{
	return (fMessageBox != nullptr) ? fMessageBox->IsShowing : false;
}

CoronaLabs::WinRT::IOperationResult^ CoronaMessageBoxHandler::ShowUsing(
	MessageBoxSettings ^settings, Rtt::LuaResource *luaResourcePointer)
{
	// Throw an exception if given a null settings reference.
	if (nullptr == settings)
	{
		throw ref new Platform::NullReferenceException();
	}

	// Do not continue if message boxes are not supported by the runtime.
	if (nullptr == fMessageBox)
	{
		return CoronaLabs::WinRT::OperationResult::FailedWith(L"Native alerts are not supported.");
	}

	// Do not continue if the message box is already being shown.
	if (fMessageBox->IsShowing)
	{
		return CoronaLabs::WinRT::OperationResult::FailedWith(L"A native alert is already being shown.");
	}

	// Do not continue if the Corona runtime is being terminated.
	if ((CoronaRuntimeState::Terminating == fEnvironment->RuntimeState) ||
	    (CoronaRuntimeState::Terminated == fEnvironment->RuntimeState))
	{
		return CoronaLabs::WinRT::OperationResult::FailedWith(
					L"Cannot display a native alert while the Corona runtime is terminating.");
	}

	// Copy the given settings to the message box.
	fMessageBox->Title = settings->Title;
	fMessageBox->Message = settings->Message;
	fMessageBox->ButtonLabels->Clear();
	for (auto&& nextLabel : settings->ButtonLabels)
	{
		fMessageBox->ButtonLabels->Append(nextLabel);
	}

	// Store the given Lua resource, which reference a Lua listener to be invoked when the message box has been closed.
	// Note: This is optional. Will be null if a Lua listener was not provided by the caller.
	fLuaResourcePointer = luaResourcePointer;

	// Add message box event handlers.
	if (!fMessageBoxClosedEventToken.Value)
	{
		fMessageBoxClosedEventToken = fMessageBox->Closed += ref new Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaMessageBoxHandler::OnMessageBoxClosed);
	}

	// Add runtime event handlers, if not done already.
	// Note: They will already have been added if there is a queued message box event that
	//       is waiting for the runtime to be resumed in order to dispatch it to Lua.
	if (!fRuntimeResumedEventToken.Value)
	{
		fRuntimeResumedEventToken = fEnvironment->Resumed += ref new Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaMessageBoxHandler::OnRuntimeResumed);
	}
	if (!fRuntimeTerminatingEventToken.Value)
	{
		fRuntimeTerminatingEventToken = fEnvironment->Terminating += ref new Windows::Foundation::EventHandler<CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaMessageBoxHandler::OnRuntimeTerminating);
	}

	// Add this message box handler to the global collection. This serves the following purposes:
	// 1) Tracks all currently displayed message boxes.
	// 2) Keeps the C++/CX reference for this handler alive until the message box is closed.
	auto iterator = sOpenMessagBoxHandlerCollection.find(this->NativeAlertReference);
	if (sOpenMessagBoxHandlerCollection.end() == iterator)
	{
		sOpenMessagBoxHandlerCollection.insert(HandlerMapPair(this->NativeAlertReference, this));
	}

	// Display the message box.
	fMessageBox->Show();
	return CoronaLabs::WinRT::OperationResult::Succeeded;
}

void CoronaMessageBoxHandler::Close()
{
	if (fMessageBox)
	{
		fMessageBox->Close();
	}
}

bool CoronaMessageBoxHandler::CloseWithButtonIndex(int index)
{
	if (nullptr == fMessageBox)
	{
		return false;
	}
	return fMessageBox->CloseWithButtonIndex(index);
}

#pragma endregion


#pragma region Static Internal Methods
CoronaMessageBoxHandler^ CoronaMessageBoxHandler::FetchShownMessageBoxBy(Rtt::NativeAlertRef value)
{
	// Validate.
	if (nullptr == value)
	{
		return nullptr;
	}

	// Fetch a currently open message box by the given reference.
	auto iterator = sOpenMessagBoxHandlerCollection.find(value);
	if (sOpenMessagBoxHandlerCollection.end() == iterator)
	{
		return nullptr;
	}

	// Found it! Return the handler's reference.
	return iterator->second;
}

#pragma endregion


#pragma region Private Methods
void CoronaMessageBoxHandler::OnMessageBoxClosed(Platform::Object^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args)
{
	// Remove event handlers.
	if (fMessageBoxClosedEventToken.Value)
	{
		fMessageBox->Closed -= fMessageBoxClosedEventToken;
		fMessageBoxClosedEventToken.Value = 0;
	}

	// Remove this object reference from the global "Open Message Box" collection.
	// This dereferences this C++/CX object and indicates that the message box is no longer shown.
	auto iterator = sOpenMessagBoxHandlerCollection.find(this->NativeAlertReference);
	if (iterator != sOpenMessagBoxHandlerCollection.end())
	{
		sOpenMessagBoxHandlerCollection.erase(iterator);
	}

	// Queue the event data to be dispatched to the given Lua listener, if provided.
	if (fLuaResourcePointer)
	{
		// Queue the event data.
		EventData eventData;
		eventData.MessageBoxHandler = this;
		eventData.LuaResourcePointer = fLuaResourcePointer;
		eventData.WasCanceled = (fMessageBox->WasButtonPressed == false);
		eventData.ButtonIndex = fMessageBox->WasButtonPressed ? fMessageBox->PressedButtonIndex : -1;
		fEventDataQueue.push(eventData);

		// Null out the Lua listener pointer since the DispatchQueuedEvents() method below will delete it.
		fLuaResourcePointer = nullptr;
	}

	// Dispatch the above event and any other previous events to Lua.
	// If the runtime is currently suspended, then this function will do nothing.
	DispatchQueuedEvents();
}

void CoronaMessageBoxHandler::OnRuntimeResumed(Platform::Object^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args)
{
	DispatchQueuedEvents();
}

void CoronaMessageBoxHandler::OnRuntimeTerminating(Platform::Object^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args)
{
	// Do not continue if this handler is not showing a message box.
	if (this->IsShowing == false)
	{
		return;
	}

	// Close the message box since the Corona runtime that displayed it is about to be terminated.
	auto lastLuaResourcePointer = fLuaResourcePointer;
	this->Close();

	// If the above Close() method did not trigger a "cancel" event immediately, then do so now.
	// We must do so now because this is our last chance to interact with Lua.
	if ((lastLuaResourcePointer != nullptr) && (lastLuaResourcePointer == fLuaResourcePointer))
	{
		EventData eventData;
		eventData.MessageBoxHandler = this;
		eventData.LuaResourcePointer = fLuaResourcePointer;
		eventData.WasCanceled = true;
		eventData.ButtonIndex = -1;
		fEventDataQueue.push(eventData);
		fLuaResourcePointer = nullptr;
	}

	// Simulate a "Closed" event now in case it hasn't bee raised yet.
	// This will dispatch the above "cancel" event (if queued) and clean up resources.
	OnMessageBoxClosed(fMessageBox, CoronaLabs::WinRT::EmptyEventArgs::Instance);
}

void CoronaMessageBoxHandler::DispatchQueuedEvents()
{
	// Attempt to dispatch all queued events.
	while (fEventDataQueue.size() > 0)
	{
		// Do not continue if the Corona runtime is not currently active. This can happen when:
		// 1) This method was called while the runtim was inactive.
		// 2) The last invoked Lua listener below triggered a suspension.
		auto runtimeState = fEnvironment->RuntimeState;
		if ((runtimeState != CoronaRuntimeState::Starting) &&
		    (runtimeState != CoronaRuntimeState::Running) &&
		    (runtimeState != CoronaRuntimeState::Terminating))
		{
			break;
		}
		if ((CoronaRuntimeState::Terminating == runtimeState) && fEnvironment->NativeRuntimePointer->IsSuspended())
		{
			break;
		}

		// Pop off the next event in the queue.
		auto eventData = fEventDataQueue.front();
		fEventDataQueue.pop();

		// Invoke the Lua listener.
		if (eventData.LuaResourcePointer)
		{
			// Dispatch a Lua "completion" event.
			Rtt::LuaLibNative::AlertComplete(
					*(eventData.LuaResourcePointer), eventData.ButtonIndex, eventData.WasCanceled);

			// Delete the Lua listener reference.
			Rtt_DELETE(eventData.LuaResourcePointer);
		}
	}

	// Determine if Corona is terminating.
	bool isTerminating = false;
	if ((fEnvironment->RuntimeState == CoronaRuntimeState::Terminating) ||
	    (fEnvironment->RuntimeState == CoronaRuntimeState::Terminated))
	{
		isTerminating = true;
	}

	// If we're terminating and we still have queued events, then delete them.
	if (isTerminating)
	{
		while (fEventDataQueue.size() > 0)
		{
			auto eventData = fEventDataQueue.front();
			fEventDataQueue.pop();
			if (eventData.LuaResourcePointer)
			{
				Rtt_DELETE(eventData.LuaResourcePointer);
			}
		}
	}

	// Unsubscribe from the runtime's events if no longer needed.
	if (isTerminating || ((fEventDataQueue.size() <= 0) && (this->IsShowing == false)))
	{
		if (fRuntimeResumedEventToken.Value)
		{
			fEnvironment->Resumed -= fRuntimeResumedEventToken;
			fRuntimeResumedEventToken.Value = 0;
		}
		if (fRuntimeTerminatingEventToken.Value)
		{
			fEnvironment->Terminating -= fRuntimeTerminatingEventToken;
			fRuntimeTerminatingEventToken.Value = 0;
		}
	}
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::UI
