//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Interop\Event.h"
#include "Interop\EventArgs.h"
#include "LogEntry.h"
#include "MainApp.h"
#include <unordered_set>


/// <summary>A dockable "Alert List" pane used to list all error and warning messages received by the app.</summary>
class AlertListPane : public CDockablePane
{
	public:
		/// <summary>
		///  <para>
		///   Defines the "RequestingShowSelectedEntry" event type which is raised when a log entry has been
		///   double clicked on in the alert pane.
		///  </para>
		///  <para>
		///   Event handler is expected to call the AlertListPane's GetSelectedLogEntryId() to determin
		///   which entry is to be shown.
		///  </para>
		/// </summary>
		typedef Interop::Event<AlertListPane&, const Interop::EventArgs&> RequestingShowSelectedEntryEvent;

		/// <summary>
		///  Defines the "EntryCountChanged" event type which is raised when a log entry has been added
		///  or removed from this pane's alert list.
		/// </summary>
		typedef Interop::Event<AlertListPane&, const Interop::EventArgs&> EntryCountChangedEvent;

		/// <summary>Creates a new dockable "Alert List" pane.</summary>
		AlertListPane();

		/// <summary>Destroys this object.</summary>
		virtual ~AlertListPane();

		/// <summary>
		///  Gets an object used to add or remove an event handler for the "RequestingShowSelectedEntry" event.
		/// </summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		RequestingShowSelectedEntryEvent::HandlerManager& GetRequestingShowSelectedEntryEventHandlers();

		/// <summary>Gets an object used to add or remove an event handler for the "EntryCountChanged" event.</summary>
		/// <returns>Returns the event's handler manager used to add/remove event handlers.</returns>
		EntryCountChangedEvent::HandlerManager& GetEntryCountChangedEventHandlers();

		/// <summary>
		///  <para>Adds the given log entry to the alert list's collection and raises an "EntryCountChanged" event.</para>
		///  <para>The given entry is cloned/copied by this pane.</para>
		/// </summary>
		/// <param name="entry">The log entry to be copied to the alert list.</param>
		void AddLogEntry(const LogEntry& entry);

		/// <summary>
		///  <para>Removes all log entries from the alert list.</para>
		///  <para>Raises an "EntryCountChanged" event if the alert list contained at least 1 log entry.</para>
		/// </summary>
		void RemoveAllLogEntries();

		/// <summary>
		///  <para>Removes the referenced log entry from the alert list by its unique integer ID.</para>
		///  <para>Raises an "EntryCountChanged" event is removed.</para>
		/// </summary>
		/// <param name="entry">Log entry to be removed from the alert list.</param>
		/// <return>Returns true if successfully removed. Returns false if log entry not found.</returns>
		bool RemoveLogEntry(const LogEntry& entry);

		/// <summary>Fetches the unique integer ID of the currently selected/highlighted log entry.</summary>
		/// <returns>
		///  <para>Returns the unique integer ID of the currently selected log entry in the alert list.</para>
		///  <para>Returns zero if a log entry is not currently selected.</para>
		/// </returns>
		DWORD GetSelectedLogEntryId() const;

		/// <summary>Gets the number of error entries in the alert list.</summary>
		/// <returns>Returns the number of error log entries in the alert list.</returns>
		int GetErrorLogEntryCount() const;

		/// <summary>Gets the number of warning entries in the alert list.</summary>
		/// <returns>Returns the number of warning log entries in the alert list.</returns>
		int GetWarningLogEntryCount() const;

		/// <summary>
		///  Called when a windows message directed to this window is about to be "translated" by the
		///  application's main message pump.
		/// </summary>
		/// <param name="messagePointer">Pointer to the windows message that has been received.</param>
		/// <returns>Returns TRUE if the message was custom handled by this window. Returns false if not.</returns>
		virtual BOOL PreTranslateMessage(MSG* messagePointer);

	protected:
		/// <summary>Called when this pane is being created. Initializes its UI controls.</summary>
		/// <param name="lpCreateStruct">Pointer to the creation settings.</param>
		/// <returns>Returns zero if successfully created. Returns non-zero if failed.</returns>
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

		/// <summary>Called when the window has been resized.</summary>
		/// <param name="nType">Type of resize such as SIZE_MAXIMIZED, SIZE_MINIMIZED, SIZE_RESTORED, etc.</param>
		/// <param name="cx">The new client width of this window.</param>
		/// <param name="cy">The new client height of this window.</param>
		afx_msg void OnSize(UINT nType, int cx, int cy);

		/// <summary>Called when the user double clicks in the list control.</summary>
		/// <param name="notificationInfoPointer">Pointer to information about what was double clicked on.</param>
		/// <param name="lResultPointer">Pointer to the windows message's LRESULT used to respond to the message.</param>
		afx_msg void OnListDoubleClicked(NMHDR *notificationInfoPointer, LRESULT *lResultPointer);

		/// <summary>Called when the list control requests this window to custom draw its content.</summary>
		/// <param name="notificationInfoPointer">Pointer to the list control's custom draw settings.</param>
		/// <param name="lResultPointer">Pointer to the windows message's LRESULT used to respond to the message.</param>
		afx_msg void OnListCustomDraw(NMHDR *notificationInfoPointer, LRESULT *lResultPointer);

		DECLARE_MESSAGE_MAP()

	private:
		/// <summary>Called when the application's visual theme has been changed.</summary>
		/// <param name="sender">The application object providing the currently selected visual theme.</param>
		/// <param name="arguments">Empty event arguments.</param>
		void OnVisualThemeChanged(MainApp& sender, const Interop::EventArgs& arguments);


		/// <summary>Manages the "RequestingShowSelectedEntry" event.</summary>
		RequestingShowSelectedEntryEvent fRequestingShowSelectedEntryEvent;

		/// <summary>Manages the "EntryCountChanged" event.</summary>
		EntryCountChangedEvent fEntryCountChangedEvent;

		/// <summary>Handler to be invoked when the "VisualThemeChanged" event has been raised.</summary>
		MainApp::VisualThemeChangedEvent::MethodHandler<AlertListPane> fVisualThemeChangedEventHandler;

		/// <summary>ListView control used to display a list of all logged errors and warnings.</summary>
		CListCtrl fListControl;

		/// <summary>Number of "error" log entries listed in this pane.</summary>
		int fErrorCount;

		/// <summary>Number of "warning" log entries listed in this pane.</summary>
		int fWarningCount;

		/// <summary>
		///  <para>Stores a set log entry pointers that the list control references.</para>
		///  <para>These are allocated copies of the log entries given to the alert list via the AddLogEntry() method.</para>
		///  <para>These must be deleted when entries are removed or when the alert list pane is being destroyed.</para>
		/// </summary>
		std::unordered_set<LogEntry*> fLogEntryPointerSet;
};

