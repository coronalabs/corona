//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MessageOnlyWindow.h"
#include "Interop\StaticFinalizer.h"
#include "Rtt_Assert.h"
#include <mutex>
#include <thread>
#include <unordered_map>


namespace Interop { namespace UI {

#pragma region Private Static Variables
/// <summary>Mutex used to synchronize access to this module's static variables between threads.</summary>
static std::recursive_mutex sMutex;

/// <summary>
///  <para>Stores a collection of message-only class names that have been registered into the Windows OS.</para>
///  <para>The STL pair's key is set to the class name and the value is the registered class' atom.</para>
/// </summary>
static std::unordered_map<std::wstring, ATOM> sRegisteredClassNameCollection;

/// <summary>
///  <para>Stores a collection of shared/global message-only window instances to be used per thread.</para>
///  <para>These instances are created on demand via the static GetSharedInstanceForCurrentThread() function.</para>
/// </summary>
static std::unordered_map<std::thread::id, MessageOnlyWindow*> sMessageOnlyWindowsCollection;

/// <summary>Finalizer used to automatically cleanup static variable resources upon application exit.</summary>
/// <remarks>
///  This static variable must be declared *last* to ensure that its static destructor gets called first.
/// </remarks>
static StaticFinalizer sStaticFinalizer([]()
{
	// Notes:
	// - We do not need to use a mutex here since all statics are destroyed on the same thread.
	// - Also, using a mutex during static variable destruction can cause crashes/deadlocks and should be avoided.

	// Destroy all shared message only windows.
	for (auto&& pair : sMessageOnlyWindowsCollection)
	{
		if (pair.second)
		{
			delete pair.second;
		}
	}
	sMessageOnlyWindowsCollection.clear();

	// Unregister all class names from the system.
	HMODULE moduleHandle = nullptr;
	DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
	::GetModuleHandleExW(flags, (LPCWSTR)&sMutex, &moduleHandle);
	for (auto&& pair : sRegisteredClassNameCollection)
	{
		auto atom = pair.second;
		if (atom)
		{
			::UnregisterClass(MAKEINTATOM(atom), moduleHandle);
		}
	}
	sRegisteredClassNameCollection.clear();
});

#pragma endregion


#pragma region Constructors/Destructors
MessageOnlyWindow::MessageOnlyWindow()
:	MessageOnlyWindow(nullptr)
{
}

MessageOnlyWindow::MessageOnlyWindow(const wchar_t* className)
:	UIComponent()
{
	std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);

	// If a class name was not provided, then use a default name.
	if (!className || (L'\0' == className[0]))
	{
		className = L"CoronaLabs.Corona.MessageOnlyWindow";
	}
	fRegisteredClassName = className;

	// If the given class name hasn't been registered yet, then do so now.
	// Note: A class name only needs to be registered once per module.
	ATOM atom = 0;
	const auto iterator = sRegisteredClassNameCollection.find(std::wstring(className));
	if (iterator == sRegisteredClassNameCollection.end())
	{
		HMODULE moduleHandle = nullptr;
		DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
		::GetModuleHandleExW(flags, (LPCWSTR)&sMutex, &moduleHandle);

		WNDCLASSEXW settings{};
		settings.cbSize = sizeof(settings);
		settings.lpszClassName = className;
		settings.hInstance = moduleHandle;
		settings.lpfnWndProc = MessageOnlyWindow::OnProcessMessage;
		atom = ::RegisterClassExW(&settings);
		if (atom)
		{
			sRegisteredClassNameCollection.insert(std::pair<std::wstring, ATOM>(fRegisteredClassName, atom));
		}
	}
	else
	{
		atom = (*iterator).second;
	}
	if (!atom)
	{
		throw std::runtime_error("Failed to register Corona's 'MessageOnlyWindow' class.");
	}

	// Create the message-only window and attach Corona's message receiver to it.
	auto windowHandle = ::CreateWindowEx(0, MAKEINTATOM(atom), TEXT(""), 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, 0);

	// Store the window handle and start listening to its Windows message events.
	OnSetWindowHandle(windowHandle);
}

MessageOnlyWindow::~MessageOnlyWindow()
{
	// Fetch the message-only window's handle.
	auto windowHandle = GetWindowHandle();
	if (!windowHandle)
	{
		return;
	}

	// Detach the WndProc callback from the window.
	// Note: Must be done before destroying it.
	OnSetWindowHandle(nullptr);

	// Destroy the message-only window object.
	::DestroyWindow(windowHandle);
}

#pragma endregion


#pragma region Public Methods
const wchar_t* MessageOnlyWindow::GetRegisteredClassName() const
{
	return fRegisteredClassName.empty() ? L"" : fRegisteredClassName.c_str();
}

UINT MessageOnlyWindow::ReserveMessageId()
{
	// Fetch an unreserved Windows message ID within the WM_USER and WM_APP range.
	// This ID only has to be unique within this MessageOnlyWindows instance.
	static const UINT kMaxMessageId = 0xBFFF;
	for (int messageId = WM_USER; messageId <= kMaxMessageId; messageId++)
	{
		auto iter = fReservedMessageIdSet.insert(messageId);
		if (iter.second)
		{
			return messageId;
		}
	}

	// If all message IDs have been reserved, then we're probably leaking message IDs somewhere.
	// Log this as a warning and then attempt to live dangerously and return a default ID.
	Rtt_LogException("MessageOnlyWindow failed to reserve a unique Windows message ID.");
	return WM_USER;
}

void MessageOnlyWindow::UnreserveMessageId(UINT id)
{
	fReservedMessageIdSet.erase(id);
}

bool MessageOnlyWindow::RemoveMessagesById(UINT messageId)
{
	// Fetch the message-only window's handle.
	auto windowHandle = GetWindowHandle();
	if (!windowHandle)
	{
		return false;
	}

	// Remove all messages from the Windows message queue having the given ID.
	bool wasRemoved = false;
	MSG message{};
	while (::PeekMessageW(&message, windowHandle, messageId, messageId, PM_NOREMOVE | PM_NOYIELD))
	{
		// Check if the "peeked" message in the queue is a Windows quit message.
		// Note: Should never be found in a "message-only window", but check just in case.
		if (WM_QUIT == message.message)
		{
			break;
		}

		// Remove the message from the queue.
		::GetMessage(&message, windowHandle, messageId, messageId);
		wasRemoved = true;
	}
	return wasRemoved;
}

#pragma endregion


#pragma region Public Static Functions
MessageOnlyWindow* MessageOnlyWindow::GetSharedInstanceForCurrentThread()
{
	std::lock_guard<std::recursive_mutex> scopedMutexLock(sMutex);

	// Set up this class' static finalizer to delete its shared instances upon app exit *before*
	// the base class' static variables get destroyed. We do this by setting up a dependency.
	// Note: This prevents a crash in case UIComponent's statics get destroyed before this class' statics.
	if (sStaticFinalizer.HasDependencies() == false)
	{
		sStaticFinalizer.AddDependency(UIComponent::GetStaticFinalizer());
	}

	// Determine if a message-only window has already been created for this thread yet.
	MessageOnlyWindow* messageOnlyWindowPointer = nullptr;
	auto iter = sMessageOnlyWindowsCollection.find(std::this_thread::get_id());
	if (iter == sMessageOnlyWindowsCollection.end())
	{
		// A message-only window hasn't been created yet.
		// Create it now and add it to the static collection for fast retrieval later.
		messageOnlyWindowPointer = new MessageOnlyWindow();
		auto pair = std::pair<std::thread::id, MessageOnlyWindow*>(std::this_thread::get_id(), messageOnlyWindowPointer);
		sMessageOnlyWindowsCollection.insert(pair);
	}
	else
	{
		// Message-only window already exists. Fetch it.
		messageOnlyWindowPointer = (*iter).second;
	}
	return messageOnlyWindowPointer;
}

#pragma endregion


#pragma region Private Methods
LRESULT MessageOnlyWindow::OnProcessMessage(HWND windowHandle, UINT messageId, WPARAM wParam, LPARAM lParam)
{
	// Unhandled messages will be delivered here. Let Windows do its default handling on them.
	return ::DefWindowProc(windowHandle, messageId, wParam, lParam);
}

#pragma endregion

} }	// namespace Interop::UI
