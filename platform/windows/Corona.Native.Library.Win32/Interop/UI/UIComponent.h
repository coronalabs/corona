//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Macros.h"
#include "Interop\Event.h"
#include "HandleMessageEventArgs.h"
#include "ReadOnlyUIComponentCollection.h"
#include <Windows.h>


#pragma region Forward Declarations
namespace Interop
{
	class StaticFinalizer;
}

#pragma endregion


#ifndef WM_REFLECT_OFFSET
/// <summary>
///  <para>Offset to be applied to a windows message ID before "reflecting" it back to the child control.</para>
///  <para>Note that this is the same offset value used by Microsoft's MFC and .NET WinForms UI frameworks.</para>
/// </summary>
/// <remarks>
///  Message reflection is a concept created by Microsoft and is used by ActiveX, MFC, and .NET WinForms. It's not
///  a feature built into the operation system's standard Win32 APIs. How it works is that notification messages sent
///  from the child control to the parent control/window can be reflected back to the child control. This is useful
///  for derived versions of a child control so that they can custom render themselves and provide their own special
///  event handling, allowing for more self-contained code.
/// </remarks>
#	define WM_REFLECT_OFFSET WM_USER + 0x1C00
#endif

/// <summary>
///  <para>Macro used to convert the given windows message ID to a reflected message ID.</para>
///  <para>To be used by child controls to handle messages that the parent reflected back to them.</para>
///  <para>This is needed because the reflected message ID is offsetted by WM_REFLECT_OFFSET to void ID collision.</para>
///  <para>Example usage:  WM_REFLECTED(WM_COMMAND)</para>
/// </summary>
#define WM_REFLECTED(messageId) WM_REFLECT_OFFSET + messageId


namespace Interop { namespace UI {

/// <summary>
///  <para>Represents a Win32 user interface component such as a window, control, etc.</para>
///  <para>Provides access to the component's Windows messages via its internal Win32 WndProc callback.</para>
/// </summary>
class UIComponent
{
	Rtt_CLASS_NO_COPIES(UIComponent)

	public:
		/// <summary>
		///  Defines the "ReceivedMessage" event type which is raised when a Windows message has been received.
		/// </summary>
		typedef Event<UIComponent&, HandleMessageEventArgs&> ReceivedMessageEvent;

		/// <summary>
		///  <para>Defines the "Destroying" event type which is raised when the UI component is about to be destroyed.</para>
		///  <para>The UI component's window handle will be set to null after this event is raised.</para>
		/// </summary>
		typedef Event<UIComponent&, const EventArgs&> DestroyingEvent;

		/// <summary>
		///  <para>Creates a new UI component object that does not reference a Win32 object.</para>
		///  <para>This object is effectively useless.</para>
		/// </summary>
		UIComponent();

		/// <summary>Creates a new UI component that wraps the given Win32 object.</summary>
		/// <param name="windowHandle">
		///  <para>Handle to the window or control to receive Windows messages from.</para>
		///  <para>Can be null, but then this UI component will do nothing.</para>
		/// </param>
		UIComponent(HWND windowHandle);

		/// <summary>Destroys this UI component, any resources it owns, and detaches its WndProc callback.</summary>
		virtual ~UIComponent();

		/// <summary>Gets an object used to add or remove an event handler for the "ReceivedMessage" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		ReceivedMessageEvent::HandlerManager& GetReceivedMessageEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "Destroying" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		DestroyingEvent::HandlerManager& GetDestroyingEventHandlers();

		/// <summary>Gets a handle to the window or control that this object is receiving Windows messages from.</summary>
		/// <returns>
		///  <para>Returns a handle to the window or control that this object is receiving Windows messages from.</para>
		///  <para>Returns null if this component is not wrapping a window or control.</para>
		/// </returns>
		HWND GetWindowHandle() const;

		/// <summary>
		///  <para>
		///   Fetches a collection of all currently existing/available UIComponent objects assigned the given window handle.
		///  </para>
		///  <para>Note: Cannot be used to fetch a UIComponent assigned a null handle.</para>
		/// </summary>
		/// <param name="windowHandle">
		///  <para>The window handle currently assigned to an existing UIComponent object.</para>
		///  <para>If given null, then this function will return an empty collection.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a collection of existing UIComponent objects assigned the given window handle.</para>
		///  <para>Returns an empty collection if there are no UI components assigned the given window handle.</para>
		/// </returns>
		static ReadOnlyUIComponentCollection FetchExistingBy(HWND windowHandle);

		/// <summary>Fetches this class' finalizer which gets invoked upon app exit.</summary>
		/// <remarks>
		///  The returned finalizer is intended to be used as a dependency target by the caller's finalizer.
		///  This allows the caller's finalizer callback to cleanup its static variables before this class' static
		///  variables gets destroyed, avoiding static destruction order issues.
		/// </remarks>
		/// <returns>Returns a reference to this class' finalizer.</returns>
		static const StaticFinalizer& GetStaticFinalizer();

	protected:
		/// <summary>
		///  <para>Called just after the "Destroying" event was raised.</para>
		///  <para>
		///   Intended to be overriden by a derived class to perform final cleanup after the component's
		///   event handlers have performed their final operations on it.
		///  </para>
		/// </summary>
		virtual void OnRaisedDestroyingEvent();

		/// <summary>
		///  <para>
		///   Protected method expected to be called by a derived class to assign a window handle to this component.
		///  </para>
		///  <para>
		///   Assigning a window handle allows this object to receive Windows message from it by overriding
		///   its Win32 WndProc callback.
		///  </para>
		///  <para>
		///   If a derived class destroys the assigned window handle, then the derived class must set this
		///   to null before the UIComponent's destructor gets invoked.
		///  </para>
		/// </summary>
		/// <param name="windowHandle">
		///  <para>Handle to the window or control to receive Windows messages from.</para>
		///  <para>Set this to null to stop receiving Windows messages if previously assigned.</para>
		/// </param>
		void OnSetWindowHandle(HWND windowHandle);

	private:
		/// <summary>
		///  A WndProc C callback that gets invoked when a Windows message has been received from the assigned window.
		/// </summary>
		/// <param name="windowHandle">Handle to the window or control that is receiving the message.</param>
		/// <param name="messageId">Unique integer ID of the message such as WM_CLOSE, WM_SiZE, etc.</param>
		/// <param name="wParam">Additional information assigned to the message.</param>
		/// <param name="lParam">Additional information assigned to the message.</param>
		/// <returns>Returns a value to the source of the Windows message.</returns>
		static LRESULT CALLBACK OnProcessMessage(HWND windowHandle, UINT messageId, WPARAM wParam, LPARAM lParam);

		/// <summary>
		///  <para>Processes a Windows message received by WndProc callback OnProcessMessage().</para>
		///  <para>Dispatches the recieved message to all UIComponent objects associated with it.</para>
		/// </summary>
		/// <param name="messageEventArgs">
		///  <para>The WndProc callback's Windows message wrapped in an event arguments object.</para>
		///  <para>This object will be dispatched to UIComponents by raising a "ReceivedMessage" event.</para>
		///  <para>The arguments WasHandled() method will returned true if a component handled the message.</para>
		/// </param>
		static void OnProcess(HandleMessageEventArgs& messageEventArgs);


		/// <summary>Manages the "ReceivedMessage" event.</summary>
		ReceivedMessageEvent fReceivedMessageEvent;

		/// <summary>Manages the "Destroying" event.</summary>
		DestroyingEvent fDestroyingEvent;

		/// <summary>Handle to the window or control this object is receiving Windows messages from.</summary>
		HWND fWindowHandle;
};

} }	// namespace Interop::UI
