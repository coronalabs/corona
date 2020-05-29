//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif


Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core\Rtt_Build.h"
#	include "Rtt_LuaResource.h"
#	include "Rtt_MPlatform.h"
#	include <queue>
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


#pragma region Forward Declarations
namespace CoronaLabs { namespace Corona { namespace WinRT {
	ref class CoronaRuntimeEnvironment;
} } }
namespace CoronaLabs { namespace WinRT {
	ref class EmptyEventArgs;
	interface class IOperationResult;
} }
#pragma endregion


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace UI {

interface class IMessageBox;
ref class MessageBoxSettings;

/// <summary>
///  <para>Shows and handles a native message box for Corona's native.showAlert() Lua function.</para>
///  <para>Will also automatically invoke the alert's Lua listener when the message box is closed.</para>
/// </summary>
ref class CoronaMessageBoxHandler sealed
{
	internal:
		#pragma region Constructors/Destructors
		/// <summary>Creates a new message box handler for the given Corona runtime.</summary>
		/// <param name="environment">
		///  <para>Provides the IUserInterfaceServices needed to create the native message box.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		CoronaMessageBoxHandler(CoronaRuntimeEnvironment^ environment);

		#pragma endregion


		#pragma region Internal Methods/Properties
		/// <summary>
		///  <para>Gets a Corona native alert reference used to uniquely identify this message box handler.</para>
		///  <para>This is expected to be returned by the Lua native.showAlert() function.</para>
		///  <para>
		///   This value can be used by the Lua native.cancelAlert() function to programmatically close the message box.
		///  </para>
		/// </summary>
		/// <value>
		///  <para>Returns a "void*" pointer type uniquely identifying this message box handler.</para>
		///  <para>Returns null if native message boxes is not supported by the Corona runtime.</para>
		/// </value>
		property Rtt::NativeAlertRef NativeAlertReference { Rtt::NativeAlertRef get(); }

		/// <summary>
		///  Determines if the Corona runtime this message box handler belongs to supports displaying native message boxes.
		/// </summary>
		/// <value>
		///  <para>Set to true if this object supports creating and displaying a native message box.</para>
		///  <para>
		///   Set to false if native message boxes are not supported.
		///   This means that the ShowUsing() method will always fail.
		///  </para>
		/// </value>
		property bool IsSupported { bool get(); }

		/// <summary>Determines if this handler is currently showing its message box.</summary>
		/// <remarks>
		///  Note that this property only reflects the show state for this message box handler.
		///  Another message box handler instance might being showing a message box.
		/// </remarks>
		/// <value>
		///  <para>Set to true if this handler is currently showing its message box.</para>
		///  <para>
		///   Set to false if this handler has not shown a message box yet or the message box it did show is now closed.
		///  </para>
		/// </value>
		property bool IsShowing { bool get(); }

		/// <summary>Displays a message box using the given settings.</summary>
		/// <param name="settings">
		///  <para>Provides the message box title, text, and button labels.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		/// <param name="luaResourcePointer">
		///  <para>
		///   Reference to a Lua listener function to be dispatch a "Completion" event to when the message box has been closed.
		///  </para>
		///  <para>Can be null, which tells this handler to not invoke a Lua listener function.</para>
		///  <para>Note that this handler will *delete* this LuaResource object after invoking its Lua listener.</para>
		/// </param>
		/// <returns>
		///  <para>Returns a success result if the message box was successfully displayed.</para>
		///  <para>
		///   Returns a failure result if a message box is already open, if the Corona runtime does not support displaying
		///   native message boxes, or for other reasons. The returned result object's "Message" property will provide
		///   details as to why it failed.
		///  </para>
		/// </returns>
		CoronaLabs::WinRT::IOperationResult^ ShowUsing(MessageBoxSettings ^settings, Rtt::LuaResource *luaResourcePointer);

		/// <summary>Closes the message box, if currently shown.</summary>
		void Close();

		/// <summary>Closes the message box, if currently shown, by simulating a button press.</summary>
		/// <param name="index">Zero based index of the button label originally given to the ShowUsing() method.</param>
		/// <returns>
		///  <para>Returns true if successfully closed the message box with the given button index.</para>
		///  <para>Returns false if the message box is not currently shown or if given an invalid index.</para>
		/// </returns>
		bool CloseWithButtonIndex(int index);

		/// <summary>Fetches a currently showing/visible message box's handler by its unique reference.</summary>
		/// <param name="value">
		///  The message box handler's unique reference returned by its <see cref="NativeAlertReference"/> property.
		/// </param>
		/// <returns>
		///  <para>Returns the referenced message box handler.</para>
		///  <para>
		///   Returns null if the given native reference was not found or the reference message box
		///   is no longer shown on screen.
		///  </para>
		/// </returns>
		static CoronaMessageBoxHandler^ FetchShownMessageBoxBy(Rtt::NativeAlertRef value);

		#pragma endregion

	private:
		#pragma region EventData Structure
		/// <summary>Stores message box event data to be dispatched to Lua as a "completion" event.</summary>
		struct EventData
		{
			/// <summary>
			///  <para>Reference to the message box handler that is handling this event.</para>
			///  <para>This reference must be set in order to keep the C++/CX object alive.</para>
			/// </summary>
			CoronaMessageBoxHandler^ MessageBoxHandler;

			/// <summary>Pointer to a Lua listener to be invoked when dispatching the event.</summary>
			Rtt::LuaResource* LuaResourcePointer;

			/// <summary>Zero based index of the button that was tapped. Set to -1 if message box was canceled out.</summary>
			int ButtonIndex;

			/// <summary>Set true if the message box was canceled out. Set false if a button was tapped.</summary>
			bool WasCanceled;
		};

		#pragma endregion


		#pragma region Private Methods
		/// <summary>
		///  <para>Called when the native message box has been closed.</para>
		///  <para>Invokes the Lua listener, notifying the owner what button was pressed, if any.</para>
		/// </summary>
		/// <param name="sender">The message box that raised this event.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnMessageBoxClosed(Platform::Object^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args);

		/// <summary>
		///  <para>Called when the Corona runtime has just been resumed after a suspension.</para>
		///  <para>If a message box has been closed during a suspension, its event will be dispatched by this method.</para>
		/// </summary>
		/// <param name="sender">The CoronaRuntimeEnvironment instance that raised this event.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnRuntimeResumed(Platform::Object^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args);

		/// <summary>
		///  <para>Called when the Corona runtime is about to be terminated.</para>
		///  <para>Closes all message boxes that were displayed by that runtime.</para>
		/// </summary>
		/// <param name="sender">The CoronaRuntimeEnvironment instance that raised this event.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnRuntimeTerminating(Platform::Object^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args);

		/// <summary>Dispatches all events in the "fEventDataQueue" member variable to Lua as "completion" events.</summary>
		void DispatchQueuedEvents();

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Reference to the Corona runtime environment needed to create the native message box.</summary>
		CoronaRuntimeEnvironment^ fEnvironment;

		/// <summary>
		///  <para>Optional pointer to a Lua listener to be invoked when the message box has been closed.</para>
		///  <para>Note that this pointer is expected to be deleted by this class.</para>
		/// </summary>
		Rtt::LuaResource* fLuaResourcePointer;

		/// <summary>
		///  <para>Reference to the native message box to be shown and handled by this class.</para>
		///  <para>Will be null if the given Corona runtime does not support message boxes.</para>
		/// </summary>
		IMessageBox^ fMessageBox;

		/// <summary>
		///  <para>A queue of message box events to be dispatched to Lua.</para>
		///  <para>This queue is needed when we can't dispatch an event to Lua immediately, such as during a suspend.</para>
		///  <para>Should only contain one element, but is made a queue in case this handler is re-used in the future.</para>
		/// </summary>
		std::queue<EventData> fEventDataQueue;

		/// <summary>Token received when subscribing to the message box's "Closed" event.</summary>
		Windows::Foundation::EventRegistrationToken fMessageBoxClosedEventToken;

		/// <summary>Token received when subscribing to the Corona runtime's "Resumed" event.</summary>
		Windows::Foundation::EventRegistrationToken fRuntimeResumedEventToken;

		/// <summary>Token received when subscribing to the Corona runtime's "Terminating" event.</summary>
		Windows::Foundation::EventRegistrationToken fRuntimeTerminatingEventToken;

		#pragma endregion
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::UI
