//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Core\Rtt_Build.h"
#include "Core\Rtt_Macros.h"
#include "Interop\Event.h"
#include "Interop\EventArgs.h"
#include "Interop\RuntimeEnvironment.h"
#include "Interop\ValueResult.h"
#include "Rtt_LuaResource.h"
#include "Rtt_MPlatform.h"
#include <memory>
#include <queue>


namespace Interop { namespace UI {

class TaskDialog;

/// <summary>
///  <para>Shows and handles a native task dialog for Corona's native.showAlert() Lua function.</para>
///  <para>Will also automatically invoke the alert's Lua listener when the task dialog is closed.</para>
/// </summary>
class CoronaTaskDialogHandler
{
	Rtt_CLASS_NO_COPIES(CoronaTaskDialogHandler)

	private:
		/// <summary>Creates a new task dialog handler for the given Corona runtime environment.</summary>
		/// <param name="environment">The Corona runtime environment the task dialog will be associated with.</param>
		CoronaTaskDialogHandler(RuntimeEnvironment& environment);

	public:
		/// <summary>Destroys this handler and its allocated resources.</summary>
		virtual ~CoronaTaskDialogHandler();

		#pragma region ShowSettings Structure
		/// <summary>Settings to be provided to the CoronaTaskDialogHandler class' ShowUsing() function.</summary>
		struct ShowSettings
		{
			/// <summary>
			///  <para>Pointer to the Corona runtime environment that will own the shown task dialog.</para>
			///  <para>This field is required and cannot be null when passed into the ShowUsing() function.</para>
			/// </summary>
			RuntimeEnvironment* RuntimeEnvironmentPointer;

			/// <summary>UTF-8 text to be displayed in the dialog's title bar. Can be null or empty string.</summary>
			const char* Title;

			/// <summary>UTF-8 text to be displayed in the dialog's content area. Can be null or empty string.</summary>
			const char* Message;

			/// <summary>
			///  <para>Array of UTF-8 encoded strings to be used as button labels.</para>
			///  <para>You must set field "ButtonLabelCount" to the number of elements in this array.</para>
			///  <para>Set to null to not show any custom buttons. Will display a [x] button in this case.</para>
			/// </summary>
			const char** ButtonLabels;

			/// <summary>
			///  <para>The number of custom button labels to be displayed in the dialog.</para>
			///  <para>This value must match the number of array elements in field "ButtonLabels".</para>
			///  <para>Set to zero to not show any custom buttons. Will display a [x] button in this case.</para>
			/// </summary>
			size_t ButtonLabelCount;

			/// <summary>
			///  <para>Set true to show the dialog's [x] close button, which will raise a canceled event.</para>
			///  <para>Set false to hide the dialog's [x] button, forcing the user to click a button label.</para>
			///  <para>Note: The [x] button will always be shown if ButtonLabelCount is zero.</para>
			/// </summary>
			bool IsCancelButtonEnabled;

			/// <summary>
			///  <para>Reference to a Lua listener to dispatch a "completion" event to when the dialog closes.</para>
			///  <para>Can be null, which tells the handler to not invoke a Lua listener function.</para>
			///  <para>Note that the handler will *delete* this LuaResource object after invoking its Lua listener.</para>
			/// </summary>
			Rtt::LuaResource* LuaResourcePointer;
		};

		#pragma endregion


		#pragma region ShowResult Class
		/// <summary>
		///  <para>Type returned by the CoronaTaskDialogHandler class' ShowUsing() static function.</para>
		///  <para>Determines if a dialog was successfully shown with the given ShowSettings information.</para>
		///  <para>
		///   If successful, the GetValue() method will returned a shared pointer to a new
		///   CoronaTaskDialogHanlder instance that manages the display dialog.
		///  </para>
		/// </summary>
		typedef ValueResult<std::shared_ptr<CoronaTaskDialogHandler>> ShowResult;

		#pragma endregion


		#pragma region Public Methods
		/// <summary>
		///  <para>Gets a Corona native alert reference used to uniquely identify this task dialog handler.</para>
		///  <para>This is expected to be returned by the Lua native.showAlert() function.</para>
		///  <para>
		///   This value can be used by the Lua native.cancelAlert() function to programmatically close the dialog.
		///  </para>
		/// </summary>
		/// <returns>Returns a "void*" pointer type uniquely identifying this task dialog handler.</returns>
		Rtt::NativeAlertRef GetNativeAlertReference() const;

		/// <summary>Determines if this handler is currently showing its task dialog.</summary>
		/// <returns>
		///  <para>Returns true if this handler is currently showing its task dialog.</para>
		///  <para>Returns false if this handler's dialog has been closed.</para>
		/// </returns>
		bool IsShowing() const;

		/// <summary>
		///  <para>Closes the task dialog, if currently shown.</para>
		///  <para>
		///   If a Lua listener/resource was provided to this handler, then a "completion" event flagged
		///   as "cancelled" will be dispatched to it.
		///  </para>
		/// </summary>
		void Close();

		/// <summary>
		///  <para>Closes the task dialog, if currently shown, by simulating a button press.</para>
		///  <para>
		///   If a Lua listener/resource was provided to this handler, then a "completion" event flagged
		///   as "clicked" will be dispatched to it along with the given button index.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns true if the dialog was successfully closed with the given button index.</para>
		///  <para>Returns false if the dialog is not currently shown or if given an invalid index.</para>
		/// </returns>
		bool CloseWithButtonIndex(int index);

		#pragma endregion


		#pragma region Public Static Functions
		/// <summary>Displays a task dialog using the given settings.</summary>
		/// <param name="settings">
		///  <para>
		///   Provides a pointer to the Corona runtime environment that owns the new dialog. This cannot be null.
		///  </para>
		///  <para>Also provides the dialog's title, text, and button labels.</para>
		/// </param>
		/// <returns>
		///  <para>
		///   Returns a success result if the task dialog was successfully displayed. The result object's GetValue()
		///   method will provide a reference to this handler of which the caller is expected to call the handler's
		///   GetNativeAlertReference() method to return back to Lua from a native.showAlert() Lua function.
		///  </para>
		///  <para>
		///   Returns a failure result if a Corona runtime environment was not provided by the given settings or if
		///   the OS failed to show the dialog. The returned result object's GetMessage() method will provide
		///   details as to why it failed.
		///  </para>
		/// </returns>
		static CoronaTaskDialogHandler::ShowResult ShowUsing(const CoronaTaskDialogHandler::ShowSettings& settings);
		
		/// <summary>Fetches a currently showing/visible dialog's handler by its unique reference.</summary>
		/// <param name="value">
		///  The dialog handler's unique reference returned by its GetNativeAlertReference() method.
		/// </param>
		/// <returns>
		///  <para>Returns a shared pointer to the referenced task dialog handler.</para>
		///  <para>
		///   Returns an empty shared pointer if the given native reference was not found
		///   or the referenced dialog is no longer shown on screen.
		///  </para>
		/// </returns>
		static std::shared_ptr<CoronaTaskDialogHandler> FetchShownDialogFor(Rtt::NativeAlertRef value);
		
		/// <summary>
		///  <para>
		///   Determines if there are any handlers currently showing/displaying a dialog for the
		///   given Corona runtime environment.
		///  </para>
		///  <para>
		///   Note that this only checks for dialogs shown via CoronaTaskDialogHandler instance. It does not check
		///   if a dialog/window is displayed by external means.
		///  </para>
		/// </summary>
		/// <param name="environment">Reference to the Corona runtime environment to lookup.</param>
		/// <returns>
		///  <para>
		///   Returns true if at least 1 dialog is currently being shown by a handler owned by the
		///   given Corona runtime environment.
		///  </para>
		///  <para>Returns false if no dialogs are being shown by any handlers owned by the given runtime.</para>
		/// </returns>
		static bool IsShowingDialogFor(const RuntimeEnvironment& environment);

		/// <summary>
		///  <para>
		///   Determines if there is at least 1 pending CoronaTaskDialogHandler instance for the
		///   given Corona runtime environment.
		///  </para>
		///  <para>A pending handler is one that is about to show a dialog or is currently showing a dialog.</para>
		///  <para>A handler is no longer pending once it has been closed.</para>
		/// </summary>
		/// <returns>
		///  <para>Returns true if there is at least 1 pending CoronaTaskDialogHandler instance.</para>
		///  <para>Returns false if there are no pending handlers.</para>
		/// </returns>
		static bool HasPendingHandlersFor(const RuntimeEnvironment& environment);

		#pragma endregion

	private:
		#pragma region EventData Structure
		/// <summary>Stores task dialog event data to be dispatched to Lua as a "completion" event.</summary>
		struct EventData
		{
			/// <summary>Zero based index of the button that was tapped. Set to -1 if task dialog was canceled out.</summary>
			int ButtonIndex;

			/// <summary>Set true if the task dialog was canceled out. Set false if a button was tapped.</summary>
			bool WasCanceled;
		};

		#pragma endregion


		#pragma region Private Methods
		/// <summary>Dispatches a Corona "completion" event to Lua for a closed dialog.</summary>
		void DispatchCompletionEvent();

		#pragma endregion


		#pragma region Private Static Functions
		/// <summary>
		///  <para>Removes the given dialog handler from this class' internal global collection.</para>
		///  <para>
		///   This function is expected to be called after the given handler's dialog has been closed
		///   and its event has been dispatched to Lua.
		///  </para>
		///  <para>
		///   This will decrement the handler's shared pointer reference, causing it to be deleted
		///   if there are no more references to it.
		///  </para>
		/// </summary>
		/// <param name="handlerPointer">The handler to be removed.</param>
		/// <returns>
		///  <para>Returns true if the given handler was successfully removed.</para>
		///  <para>Returns false if the given handler was null or not found in the global collection.</para>
		/// </returns>
		static bool RemoveFromStaticCollection(const CoronaTaskDialogHandler* handlerPointer);

		/// <summary>
		///  <para>Removes the given dialog handler from this class' internal global collection.</para>
		///  <para>
		///   This function is expected to be called after the given handler's dialog has been closed
		///   and its event has been dispatched to Lua.
		///  </para>
		///  <para>
		///   This will decrement the handler's shared pointer reference, causing it to be deleted
		///   if there are no more references to it.
		///  </para>
		/// </summary>
		/// <param name="handlerPointer">The handler to be removed.</param>
		/// <returns>
		///  <para>Returns true if the given handler was successfully removed.</para>
		///  <para>Returns false if the given handler was null or not found in the global collection.</para>
		/// </returns>
		static bool RemoveFromStaticCollection(const std::shared_ptr<CoronaTaskDialogHandler> handlerPointer);

		/// <summary>
		///  <para>Called when a Corona runtime has just been resumed after a suspension.</para>
		///  <para>
		///   If any task dialogs have been closed during a suspension, its event will be dispatched by this method.
		///  </para>
		/// </summary>
		/// <remarks>
		///  This must be a static function so that it can delete all handlers that have been closed
		///  and had all of their events dispatched.
		/// </remarks>
		/// <param name="sender">The RuntimeEnvironment instance that raised this event.</param>
		/// <param name="arguments">Empty event arguments.</param>
		static void OnRuntimeResumed(RuntimeEnvironment& sender, const EventArgs& arguments);

		/// <summary>
		///  <para>Called when the Corona runtime is about to be terminated.</para>
		///  <para>Closes all task dialogs that were displayed by that runtime.</para>
		/// </summary>
		/// <remarks>
		///  This must be a static function so that it can delete all handlers associated with the terminating runtime.
		/// </remarks>
		/// <param name="sender">The RuntimeEnvironment instance that raised this event.</param>
		/// <param name="arguments">Empty event arguments.</param>
		static void OnRuntimeTerminating(RuntimeEnvironment& sender, const EventArgs& arguments);

		/// <summary>
		///  <para>Called when a Windows message has been received by the message-only window.</para>
		///  <para>Used to display the task dialog since they can only be displayed modally/blocking.</para>
		/// </summary>
		/// <remarks>
		///  This must be a static function in case the runtime gets destroyed while displaying the dialog.
		/// </remarks>
		/// <param name="sender">Reference to the message-only window.</param>
		/// <param name="arguments">
		///  <para>Provides the Windows message information.</para>
		///  <para>Call its SetHandled() and SetReturnValue() methods if this handler will be handling the message.</para>
		/// </param>
		static void OnReceivedMessage(UIComponent& sender, HandleMessageEventArgs& arguments);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>Reference to the Corona runtime environment that this task dialog handler belongs to.</summary>
		RuntimeEnvironment& fEnvironment;

		/// <summary>
		///  <para>Optional pointer to a Lua listener to be invoked when the message box has been closed.</para>
		///  <para>Note that this pointer is expected to be deleted by this class.</para>
		/// </summary>
		Rtt::LuaResource* fLuaResourcePointer;

		/// <summary>
		///  <para>Unique windows message ID reserved for this handler via the runtime's message-only window.</para>
		///  <para>Used by the OnReceivedMessage() function to fetch the handler that has posted a message to itself.</para>
		/// </summary>
		UINT fReservedMessageId;

		/// <summary>Pointer to a task dialog to be shown and handled by this class.</summary>
		TaskDialog* fTaskDialogPointer;

		/// <summary>Set true if the task dialog was closed and member variable "fEventData" has the dialog's results.</summary>
		bool fHasEventData;

		/// <summary>
		///  <para>Dialog "completion" data to be dispatched as an event to Lua.</para>
		///  <para>Should be ignored until member variable "fHasEventData" has been set true.</para>
		/// </summary>
		CoronaTaskDialogHandler::EventData fEventData;

		/// <summary>Handler to be invoked when the "Resumed" event has been raised.</summary>
		RuntimeEnvironment::ResumedEvent::FunctionHandler fRuntimeResumedEventHandler;

		/// <summary>Handler to be invoked when the "Terminating" event has been raised.</summary>
		RuntimeEnvironment::TerminatingEvent::FunctionHandler fRuntimeTerminatingEventHandler;

		/// <summary>
		///  Handler to be invoked when the "ReceivedMessage" event has been raised by the message-only window.
		/// </summary>
		UIComponent::ReceivedMessageEvent::FunctionHandler fReceivedMessageEventHandler;

		#pragma endregion
};

} }	// namespace Interop::UI
