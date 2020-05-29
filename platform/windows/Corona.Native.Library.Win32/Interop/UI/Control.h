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
#include "Interop\EventArgs.h"
#include "UIComponent.h"
#include <Windows.h>


namespace Interop { namespace UI {

/// <summary>Represents a Windows control such as a TextBox, Button, WebBrowser control, etc.</summary>
class Control : public UIComponent
{
	Rtt_CLASS_NO_COPIES(Control)

	public:
		/// <summary>
		///  Defines the "Resized" event type which is raised when the control's width or height has changed.
		/// </summary>
		typedef Event<Control&, const EventArgs&> ResizedEvent;

		/// <summary>
		///  Defines the "GainedFocus" event type which is raised when keyboard focus has just been gained.
		/// </summary>
		typedef Event<Control&, const EventArgs&> GainedFocusEvent;

		/// <summary>
		///  Defines the "LostFocus" event type which is raised when keyboard focus has just been lost.
		/// </summary>
		typedef Event<Control&, const EventArgs&> LostFocusEvent;

		/// <summary>
		///  <para>Creates a control object that does not reference a Win32 UI object.</para>
		///  <para>This object is effectively useless.</para>
		/// </summary>
		Control();

		/// <summary>Creates a new control which wraps the given window handle.</summary>
		/// <param name="windowHandle">
		///  <para>Handle to a Windows control to wrap.</para>
		///  <para>Can be null, but then the control object will do nothing.</para>
		/// </param>
		Control(HWND windowHandle);

		/// <summary>Destroys this object.</summary>
		virtual ~Control();

		/// <summary>Gets an object used to add or remove an event handler for the "Resized" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		ResizedEvent::HandlerManager& GetResizedEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "GainedFocus" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		GainedFocusEvent::HandlerManager& GetGainedFocusEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "LostFocus" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		LostFocusEvent::HandlerManager& GetLostFocusEventHandlers();

		/// <summary>Determines if this control has the input/keyboard focus.</summary>
		/// <returns>
		///  <para>Returns true if this control has the input/keyboard focus.</para>
		///  <para>Returns false if it does not have the focus or this object was not assigned a windows handle.</para>
		/// </returns>
		bool HasFocus() const;

		/// <summary>Assigns this control the input/keyboard focus.</summary>
		void SetFocus();

		/// <summary>Determines if this control is visible.</summary>
		/// <returns>
		///  <para>Returns true if the control is visible.</para>
		///  <para>Returns false if invisible or this object was not assigned a windows handle.</para>
		/// </returns>
		bool IsVisible() const;

		/// <summary>Shows or hides this control.</summary>
		/// <param name="value">Set true to show this control. Set false to hide it.</param>
		void SetVisible(bool value);

		/// <summary>Gets x/y position and width/height of the control in client coordinates.</summary>
		/// <returns>
		///  <para>Returns this control's position, width, and height in client coordinates.</para>
		///  <para>Returns a rectangle with all zeros if this object was not assigned a windows handle.</para>
		/// </returns>
		RECT GetBounds() const;

		/// <summary>Gets the width of the control in client coordinates.</summary>
		/// <returns>
		///  <para>Returns this control's width in client coordinates.</para>
		///  <para>Returns zero if this object was not assigned a windows handle.</para>
		/// </returns>
		int GetWidth() const;

		/// <summary>Gets the height of the control in client coordinates.</summary>
		/// <returns>
		///  <para>Returns this control's height in client coordinates.</para>
		///  <para>Returns zero if this object was not assigned a windows handle.</para>
		/// </returns>
		int GetHeight() const;

		/// <summary>Changes the bounds of the control, such as its x/y position and width/height.</summary>
		/// <param name="bounds">The controls new bounds in client coordinates.</param>
		void SetBounds(const RECT &bounds);

		/// <summary>
		///  <para>Gets the x/y position and width/height of the client area within the control.</para>
		///  <para>
		///  The client area is the region within the control or window's borders and is where content is usually rendered.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns this control's position, width, and height within this control's bounds.</para>
		///  <para>Returns a rectangle with all zeros if this object was not assigned a windows handle.</para>
		/// </returns>
		RECT GetClientBounds() const;

		/// <summary>
		///  <para>Gets the width of the control's client region.</para>
		///  <para>
		///  The client area is the region within the control or window's borders and is where content is usually rendered.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns the client region width within the bounds of the control.</para>
		///  <para>Returns zero if this object was not assigned a windows handle.</para>
		/// </returns>
		int GetClientWidth() const;
		
		/// <summary>
		///  <para>Gets the height of the control's client region.</para>
		///  <para>
		///  The client area is the region within the control or window's borders and is where content is usually rendered.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns the client region height within the bounds of the control.</para>
		///  <para>Returns zero if this object was not assigned a windows handle.</para>
		/// </returns>
		int GetClientHeight() const;

		/// <summary>
		///  <para>Changes the width and height of the client area of the control.</para>
		///  <para>This will also change the control's full width and height, which includes the border.</para>
		/// </summary>
		/// <param name="size">The width and height to change the client area to.</param>
		void SetClientSize(const SIZE size);

	private:
		/// <summary>Called when a Windows message has been dispatched to this control.</summary>
		/// <param name="sender">Reference to this control.</param>
		/// <param name="arguments">
		///  <para>Provides the Windows message information.</para>
		///  <para>Call its SetHandled() and SetReturnValue() methods if this handler will be handling the message.</para>
		/// </param>
		void OnReceivedMessage(UIComponent& sender, HandleMessageEventArgs& arguments);


		/// <summary>Manages the "Resized" event.</summary>
		ResizedEvent fResizedEvent;

		/// <summary>Manages the "GainedFocus" event.</summary>
		GainedFocusEvent fGainedFocusEvent;

		/// <summary>Manages the "LostFocus" event.</summary>
		LostFocusEvent fLostFocusEvent;

		/// <summary>Handler to be invoked when the "ReceivedMessage" event has been raised.</summary>
		UIComponent::ReceivedMessageEvent::MethodHandler<Control> fReceivedMessageEventHandler;
};

} }	// namespace Interop::UI
