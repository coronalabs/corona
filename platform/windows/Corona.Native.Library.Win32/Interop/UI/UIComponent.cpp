//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIComponent.h"
#include "Interop\StaticFinalizer.h"
#include "UIComponentCollection.h"
#include <exception>
#include <mutex>
#include <unordered_map>


namespace Interop { namespace UI {

#pragma region Static Member Variables
/// <summary>
///  <para>
///   Stores a collection of all UIComponent objects associated with one HWND, along with the WndProc callback
///   that was originally assigned to the window.
///  </para>
///  <para>This struct is only intended to be used by the static "sHandleComponentMapping" member variable.</para>
/// </summary>
struct HandleComponentData
{
	/// <summary>Collection of UIComponents assigned to the same window handle.</summary>
	UIComponentCollection Collection;

	/// <summary>
	///  <para>Pointer to the assigned window's previous WndProc callback.</para>
	///  <para>Must be invoked if none of the UIComponent message handlers handle a WndProc message.</para>
	///  <para>Must also be re-assigned to the window when all UIComponents have been detached from the window.</para>
	/// </summary>
	WNDPROC PreviousCallback;
};

/// <summary>Defines the collection type for the "sHandleComponentMapping" static variable.</summary>
typedef std::unordered_map<HWND, HandleComponentData*> HandleComponentMap;

/// <summary>Defines the key/value pair used by the "HandleComponentMap" type.</summary>
typedef std::pair<HWND, HandleComponentData*> HandleComponentMapPair;

/// <summary>
///  <para>Stores a hash table of HWND and HandleComponentData pairs.</para>
///  <para>Needed in order to fetch the UI components assigned to a HWND from the OnProcessMessage() C function.</para>
/// </summary>
static HandleComponentMap sHandleComponentMapping;

/// <summary>Mutex used to synchronize access to the "sHandleComponentMapping" static variable between threads.</summary>
static std::recursive_mutex sMutex;

/// <summary>Finalizer used to automatically cleanup static variable resources upon application exit.</summary>
/// <remarks>
///  <para>
///   This finalizer is used as a dependency target for finalizers belonging to derived classes, such as the
///   the "MessageOnlyWindow" class. This ensures that derived classes can cleanup their static variables
///   before the destructors for this class' static variables get called, preventing a crash.
///  </para>
///  <para>This static variable must be declared *last* to ensure that its static destructor gets called first.</para>
/// </remarks>
static StaticFinalizer sStaticFinalizer;

#pragma endregion


#pragma region Constructors/Destructors
UIComponent::UIComponent()
:	fWindowHandle(nullptr)
{
}

UIComponent::UIComponent(HWND windowHandle)
:	UIComponent()
{
	// Wrap the given window handle and attach a WndProc callback to it.
	OnSetWindowHandle(windowHandle);
}

UIComponent::~UIComponent()
{
	// Detach this component's WndProc callback from the Win32 object.
	OnSetWindowHandle(nullptr);
}

#pragma endregion


#pragma region Public Methods
UIComponent::ReceivedMessageEvent::HandlerManager& UIComponent::GetReceivedMessageEventHandlers()
{
	return fReceivedMessageEvent.GetHandlerManager();
}

UIComponent::DestroyingEvent::HandlerManager& UIComponent::GetDestroyingEventHandlers()
{
	return fDestroyingEvent.GetHandlerManager();
}

HWND UIComponent::GetWindowHandle() const
{
	return fWindowHandle;
}

#pragma endregion


#pragma region Public Static Functions
ReadOnlyUIComponentCollection UIComponent::FetchExistingBy(HWND windowHandle)
{
	std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);

	auto iter = sHandleComponentMapping.find(windowHandle);
	if (iter != sHandleComponentMapping.end())
	{
		auto dataPointer = (*iter).second;
		if (dataPointer)
		{
			return ReadOnlyUIComponentCollection(dataPointer->Collection);
		}
	}
	return ReadOnlyUIComponentCollection();
}

const StaticFinalizer& UIComponent::GetStaticFinalizer()
{
	return sStaticFinalizer;
}

#pragma endregion


#pragma region Protected Methods
void UIComponent::OnRaisedDestroyingEvent()
{
}

void UIComponent::OnSetWindowHandle(HWND windowHandle)
{
	// Do not continue if this handle has already been assigned.
	if (windowHandle == fWindowHandle)
	{
		return;
	}

	// Synchronize access to the static "sHandleComponentMapping" collection.
	std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);

	// Detach this component from the previous window.
	if (fWindowHandle)
	{
		// Fetch the UIComponent collection assigned to the window handle.
		auto iter = sHandleComponentMapping.find(fWindowHandle);
		if (iter != sHandleComponentMapping.end())
		{
			auto dataPointer = (*iter).second;
			if (dataPointer)
			{
				// Remove this UIComponent from the window handle's associated collection.
				dataPointer->Collection.Remove(this);

				// If there are no more UIComponent assigned to the window handle, then do a final cleanup.
				if (dataPointer->Collection.GetCount() <= 0)
				{
					// Restore the window's orignal WndProc callback.
					if (dataPointer->PreviousCallback)
					{
						::SetWindowLongPtr(fWindowHandle, GWLP_WNDPROC, (LONG_PTR)dataPointer->PreviousCallback);
					}

					// Delete the window handle from the map.
					sHandleComponentMapping.erase(iter);
					delete dataPointer;
				}
			}
		}
	}

	// Attach this component to the given window.
	fWindowHandle = windowHandle;
	if (fWindowHandle)
	{
		// Check if there are other components already assigned to the given window handle.
		HandleComponentData* dataPointer = nullptr;
		auto iter = sHandleComponentMapping.find(fWindowHandle);
		if (iter != sHandleComponentMapping.end())
		{
			dataPointer = (*iter).second;
		}

		// Attach this component.
		if (dataPointer)
		{
			// There is another UIComponent already assigned the given window handle.
			// So, all we have to do is add this UIComponent to the collection and we're done.
			dataPointer->Collection.Add(this);
		}
		else
		{
			// This is the first UIComponent to be assigned to the given window handle.
			// Attach our WndProc callback to the given window and add an entry to the main collection.
			dataPointer = new HandleComponentData();
			dataPointer->Collection.Add(this);
			dataPointer->PreviousCallback =
					(WNDPROC)::SetWindowLongPtr(fWindowHandle, GWLP_WNDPROC, (LONG_PTR)OnProcessMessage);
			sHandleComponentMapping.insert(HandleComponentMapPair(fWindowHandle, dataPointer));
		}
	}
}

#pragma endregion


#pragma region Private Static Functions
LRESULT CALLBACK UIComponent::OnProcessMessage(HWND windowHandle, UINT messageId, WPARAM wParam, LPARAM lParam)
{
	// Fetch UIComponent information for the given window handle.
	HandleComponentData* dataPointer = nullptr;
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		auto iter = sHandleComponentMapping.find(windowHandle);
		if (iter != sHandleComponentMapping.end())
		{
			dataPointer = (*iter).second;
		}
	}

	// Handle the recevied message.
	if (dataPointer)
	{
		// Fetch the original WndProc callback in case we lose it after raising events below.
		auto previousCallback = dataPointer->PreviousCallback;

		// Create the message event arguments.
		MessageSettings messageSettings;
		messageSettings.WindowHandle = windowHandle;
		messageSettings.MessageId = messageId;
		messageSettings.WParam = wParam;
		messageSettings.LParam = lParam;
		HandleMessageEventArgs messageEventArgs(messageSettings);

		// Handle the Windows message.
		OnProcess(messageEventArgs);
		if (messageEventArgs.WasHandled())
		{
			// The event was handled. Return the handler's result value.
			return messageEventArgs.GetReturnResult();
		}
		else if (previousCallback)
		{
			// The event was not handled. Send the message to the window's previously assigned WndProc callback.
			return ::CallWindowProc(previousCallback, windowHandle, messageId, wParam, lParam);
		}
	}

	// We no longer have a UIComponent attached to the given window.
	// Nor do we have the window's original WndProc callback to dispatch it to.
	// So, we have no choice but to let Windows do its default handling on the received message.
	return ::DefWindowProc(windowHandle, messageId, wParam, lParam);
}

void UIComponent::OnProcess(HandleMessageEventArgs& messageEventArgs)
{
	// Fetch all UIComponents assigned to the given window handle.
	HWND windowHandle = messageEventArgs.GetWindowHandle();
	HandleComponentData* dataPointer = nullptr;
	{
		std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);
		auto iter = sHandleComponentMapping.find(windowHandle);
		if (iter != sHandleComponentMapping.end())
		{
			dataPointer = (*iter).second;
		}
	}
	if (!dataPointer)
	{
		return;
	}

	// Create a copy of the UIComponent collection in case they get added/removed after raising events below.
	// Note: For best performance, try to make a copy on the stack instead of the heap.
	static const int kMaxStackArraySize = 8;
	UIComponent *componentPointerStackArray[kMaxStackArraySize];
	UIComponent **componentPointerHeapArray = nullptr;
	UIComponent **componentPointerArray = nullptr;
	int componentCount = dataPointer->Collection.GetCount();
	if (componentCount > kMaxStackArraySize)
	{
		componentPointerHeapArray = new UIComponent*[componentCount];
		componentPointerArray = componentPointerHeapArray;
	}
	else
	{
		componentPointerArray = componentPointerStackArray;
	}
	for (int index = 0; index < componentCount; index++)
	{
		componentPointerArray[index] = dataPointer->Collection.GetByIndex(index);
	}

	// Send the received message to all assigned components, using the following rules:
	// - The last component assigned to the window gets the message first. (Order matters!)
	// - If a component handles the message, all other components in the chain will not get the message.
	for (int index = componentCount - 1; index >= 0; index--)
	{
		// Fetch the next UIComponent.
		auto componentPointer = componentPointerArray[index];
		if (!componentPointer)
		{
			continue;
		}

		// Skip this UIComponent if it has been removed while iterating through this loop.
		auto componentCollection = UIComponent::FetchExistingBy(windowHandle);
		if (componentCollection.Contains(componentPointer) == false)
		{
			continue;
		}

		// Raise a "ReceivedMessage" event.
		componentPointer->fReceivedMessageEvent.Raise(*componentPointer, messageEventArgs);

		// If this UIComponent handled the event, then do not pass the message to any other UIComponent.
		if (messageEventArgs.WasHandled())
		{
			break;
		}
	}

	// Perform special handling for the following windows messages after raising the "ReceivedMessage" event above.
	switch (messageEventArgs.GetMessageId())
	{
		case WM_DESTROY:
		{
			// Raise a "Destroying" event for all UIComponents associated with this window handle.
			for (int index = componentCount - 1; index >= 0; index--)
			{
				// Fetch the next UIComponent.
				auto componentPointer = componentPointerArray[index];
				if (!componentPointer)
				{
					continue;
				}

				// Raise a "Destroying" event, if the component hasn't removed itself already.
				if (UIComponent::FetchExistingBy(windowHandle).Contains(componentPointer))
				{
					componentPointer->fDestroyingEvent.Raise(*componentPointer, EventArgs::kEmpty);
					componentPointer->OnRaisedDestroyingEvent();
				}

				// Release the component's window handle, if the component hasn't removed itself already.
				if (UIComponent::FetchExistingBy(windowHandle).Contains(componentPointer))
				{
					componentPointer->OnSetWindowHandle(nullptr);
				}
			}
			break;
		}
		case WM_COMMAND:
		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORSCROLLBAR:
		case WM_CTLCOLORSTATIC:
		case WM_DRAWITEM:
		case WM_MEASUREITEM:
		case WM_DELETEITEM:
		case WM_VKEYTOITEM:
		case WM_CHARTOITEM:
		case WM_COMPAREITEM:
		case WM_HSCROLL:
		case WM_VSCROLL:
		case WM_NOTIFY:
		{
			// ------------------------------------------------------------------------------------------
			// This message was sent by a child control.
			// Reflect it back to the child control in case it wants to custom handle/render itself.
			// ------------------------------------------------------------------------------------------

			// Do not reflect the message if it was already handled by the parent.
			if (messageEventArgs.WasHandled())
			{
				break;
			}

			// Fetch the child control's handle from the message.
			HWND childWindowHandle = nullptr;
			switch (messageEventArgs.GetMessageId())
			{
				case WM_DRAWITEM:
				case WM_MEASUREITEM:
				case WM_DELETEITEM:
				case WM_COMPAREITEM:
					childWindowHandle = ::GetDlgItem(windowHandle, messageEventArgs.GetWParam());
					break;
				case WM_NOTIFY:
					if (messageEventArgs.GetLParam())
					{
						childWindowHandle = ((NMHDR*)messageEventArgs.GetLParam())->hwndFrom;
					}
					break;
				default:
					childWindowHandle = (HWND)messageEventArgs.GetLParam();
					break;
			}
			if (!childWindowHandle)
			{
				break;
			}

			// Create a new window message for the child based on the received parent message.
			// Note: The child control will received an offsetted message ID.
			//       This is exactly how Microsoft handles it in MFC, .NET WinForms, and ActiveX controls.
			MessageSettings childMessageSettings;
			childMessageSettings.WindowHandle = childWindowHandle;
			childMessageSettings.MessageId = WM_REFLECT_OFFSET + messageEventArgs.GetMessageId();
			childMessageSettings.WParam = messageEventArgs.GetWParam();
			childMessageSettings.LParam = messageEventArgs.GetLParam();
			HandleMessageEventArgs childMessageEventArgs(childMessageSettings);

			// Send the message to the child control.
			// Note: This only works for UIComponent derived classes.
			//       This will not send a message via the Win32 SendMessage() or PostMessage() functions.
			//       This is by design in case there is message ID collision with a 3rd party child control.
			OnProcess(childMessageEventArgs);

			// If the child control handled the message, then copy the result to the parent's message event arguments.
			if (childMessageEventArgs.WasHandled())
			{
				messageEventArgs.SetReturnResult(childMessageEventArgs.GetReturnResult());
				messageEventArgs.SetHandled();
			}
			break;
		}
	}

	// Delete the temporary array if allocated on the heap.
	if (componentPointerHeapArray)
	{
		delete[] componentPointerHeapArray;
	}
}

#pragma endregion

} }	// namespace Interop::UI
