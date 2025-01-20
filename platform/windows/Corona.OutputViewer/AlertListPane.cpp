//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AlertListPane.h"
#include "resource.h"
#include "VisualTheme.h"
#include "WinString.h"


#ifdef _DEBUG
#	define new DEBUG_NEW
#endif


#pragma region Message Mappings
BEGIN_MESSAGE_MAP(AlertListPane, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_ALERT_LIST, OnListDoubleClicked)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_ALERT_LIST, OnListCustomDraw)
END_MESSAGE_MAP()

#pragma endregion


#pragma region Constructors/Destructors
AlertListPane::AlertListPane()
:	fVisualThemeChangedEventHandler(this, &AlertListPane::OnVisualThemeChanged),
	fErrorCount(0),
	fWarningCount(0)
{
	// Listen for visual theme changes.
	auto applicationPointer = (MainApp*)AfxGetApp();
	applicationPointer->GetVisualThemeChangedEventHandlers().Add(&fVisualThemeChangedEventHandler);
}

AlertListPane::~AlertListPane()
{
	// Delete our cloned log entries
	for (auto&& entryPointer : fLogEntryPointerSet)
	{
		delete entryPointer;
	}
	fLogEntryPointerSet.clear();

	// Remove event handlers.
	auto applicationPointer = (MainApp*)AfxGetApp();
	applicationPointer->GetVisualThemeChangedEventHandlers().Remove(&fVisualThemeChangedEventHandler);
}

#pragma endregion


#pragma region Event Handlers
int AlertListPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// Call the base class' OnCreate() method first.
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	// Set up the ListView control.
	DWORD windowStyles =
			AFX_WS_DEFAULT_VIEW | LVS_REPORT | LVS_NOSORTHEADER |
			LVS_NOCOLUMNHEADER | LVS_SHOWSELALWAYS | LVS_SINGLESEL;
	fListControl.Create(windowStyles, CRect(0, 0, 0, 0), this, IDC_ALERT_LIST);
	::SetWindowTheme(fListControl.GetSafeHwnd(), _T("explorer"), nullptr);
	fListControl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	fListControl.SetView(LV_VIEW_DETAILS);
	LVCOLUMNW column{};
	column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	column.fmt = LVCFMT_LEFT;
	column.cx = 500;
	column.pszText = L"Message";
	column.cchTextMax = (sizeof(column.pszText) / sizeof(wchar_t)) + 1;
	fListControl.InsertColumn(0, &column);

	// Apply the currently selected visual theme to this pane's UI.
	OnVisualThemeChanged(*(MainApp*)AfxGetApp(), Interop::EventArgs::kEmpty);

	// Return zero to indicate that we've successfully set up the UI.
	return 0;
}

void AlertListPane::OnSize(UINT nType, int cx, int cy)
{
	// Let the base class handle the resize first.
	CDockablePane::OnSize(nType, cx, cy);

	// Resize the list control to fill this pane's new size.
	fListControl.SetWindowPos(nullptr, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

	// Resize the width of the list control's last column to completely fill the width of the control.
	int scrollbarWidth = ::GetSystemMetrics(SM_CXVSCROLL);
	if (scrollbarWidth < 0)
	{
		scrollbarWidth = 0;
	}
	fListControl.SetColumnWidth(0, (cx - scrollbarWidth) - 2);
}

void AlertListPane::OnListDoubleClicked(NMHDR *notificationInfoPointer, LRESULT *lResultPointer)
{
	NM_LISTVIEW* listViewPointer;

	// The result must be set to zero.
	*lResultPointer = 0;

	// Do not continue if the user double clicked in an empty spot within the control.
	listViewPointer = (NM_LISTVIEW*)notificationInfoPointer;
	if (!listViewPointer)
	{
		return;
	}
	if ((listViewPointer->iItem < 0) || (listViewPointer->iItem >= fListControl.GetItemCount()))
	{
		return;
	}

	// Raise an event requesting the owner of this control to display the double clicked log entry.
	fRequestingShowSelectedEntryEvent.Raise(*this, Interop::EventArgs::kEmpty);
}

void AlertListPane::OnListCustomDraw(NMHDR *notificationInfoPointer, LRESULT *lResultPointer)
{
	// Initialize the result to let the list control do its default drawing.
	*lResultPointer = CDRF_DODEFAULT;

	// Fetch the custom draw settings.
	auto customDrawPointer = (LPNMLVCUSTOMDRAW)notificationInfoPointer;
	if (!customDrawPointer)
	{
		return;
	}

	// Handle the custom paint notification.
	switch (customDrawPointer->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
		{
			// Request the list control to let us custom draw each item in the list.
			*lResultPointer = CDRF_NOTIFYITEMDRAW;
			break;
		}
		case CDDS_ITEMPREPAINT:
		{
			// Paint the list's item text with custom colors based on the log entry "type".
			auto entryPointer = (LogEntry*)fListControl.GetItemData(customDrawPointer->nmcd.dwItemSpec);
			if (entryPointer)
			{
				const VisualTheme* themePointer = &((MainApp*)AfxGetApp())->GetVisualTheme();
				if (&VisualTheme::kDark == themePointer)
				{
					customDrawPointer->clrTextBk = RGB(32, 32, 32);
				}
				switch (entryPointer->GetType())
				{
					case LogEntry::Type::kError:
						// Use red text for error messages.
						if (&VisualTheme::kDark == themePointer)
						{
							customDrawPointer->clrText = RGB(255, 0, 0);
						}
						else
						{
							customDrawPointer->clrText = RGB(192, 0, 0);
						}
						break;
					case LogEntry::Type::kWarning:
						// Use yellow text for warning messages.
						if (&VisualTheme::kDark == themePointer)
						{
							customDrawPointer->clrText = RGB(255, 255, 0);
						}
						else
						{
							customDrawPointer->clrText = RGB(96, 96, 0);
						}
						break;
				}
			}
			break;
		}
	}
}

void AlertListPane::OnVisualThemeChanged(MainApp& sender, const Interop::EventArgs& arguments)
{
	// Fetch the currently selected theme.
	const VisualTheme* themePointer = &sender.GetVisualTheme();

	// Apply the theme to the ListView control.
	if (&VisualTheme::kDark == themePointer)
	{
		fListControl.SetBkColor(RGB(32, 32, 32));
	}
	else
	{
		fListControl.SetBkColor(GetSysColor(COLOR_WINDOW));
	}
	fListControl.Invalidate();
}

#pragma endregion


#pragma region Public Methods
AlertListPane::RequestingShowSelectedEntryEvent::HandlerManager&
AlertListPane::GetRequestingShowSelectedEntryEventHandlers()
{
	return fRequestingShowSelectedEntryEvent.GetHandlerManager();
}

AlertListPane::EntryCountChangedEvent::HandlerManager& AlertListPane::GetEntryCountChangedEventHandlers()
{
	return fEntryCountChangedEvent.GetHandlerManager();
}

void AlertListPane::AddLogEntry(const LogEntry& entry)
{
	// Copy the given log entry text and modify it as follows:
	// - Strip off the newline character at the end of the message. Must be shown in single line form.
	// - Replace all embed carriage returns, newlines, tabs, and nulls with "[\r]", "[\n]", "[\t]", and "[\0]".
	WinString updatedMessage(entry.GetText());
	updatedMessage.TrimEnd(L"\r\n");
//TODO: Add support for embedded nulls in the message. They currently truncate the message.
//	updatedMessage.Replace("\0", "[\\0]");
	updatedMessage.Replace("\t", "[\\t]");
	updatedMessage.Replace("\r", "[\\r]");
	updatedMessage.Replace("\n", "[\\n]");

	// Do not continue if the log entry message is empty.
	if (updatedMessage.IsEmpty())
	{
		return;
	}

	// Clone the given log message and add it to our own collection.
	auto clonedLogEntryPointer = new LogEntry(entry);
	fLogEntryPointerSet.insert(clonedLogEntryPointer);
	
	// Add the entry to the list control.
	LVITEM item{};
	item.mask = LVIF_TEXT | LVIF_PARAM;
	item.iItem = fListControl.GetItemCount();
	item.pszText = updatedMessage.GetBuffer();
	item.cchTextMax = updatedMessage.GetLength() + 1;
	item.lParam = (LPARAM)clonedLogEntryPointer;
	fListControl.InsertItem(&item);

	// Update the error/warning count.
	switch (entry.GetType())
	{
		case LogEntry::Type::kError:
			fErrorCount++;
			break;
		case LogEntry::Type::kWarning:
			fWarningCount++;
			break;
	}

	// Notify the system that this list's entry count has changed.
	fEntryCountChangedEvent.Raise(*this, Interop::EventArgs::kEmpty);
}

void AlertListPane::RemoveAllLogEntries()
{
	// Delete all log entries in our in-memory collection.
	bool didContainEntries = false;
	for (auto&& entryPointer : fLogEntryPointerSet)
	{
		delete entryPointer;
		didContainEntries = true;
	}
	fLogEntryPointerSet.clear();

	// Remove all entries from the list control.
	fListControl.DeleteAllItems();

	// Reset the error/warning counts to zero.
	fErrorCount = 0;
	fWarningCount = 0;

	// Notify the system that this list's entry count has changed.
	if (didContainEntries)
	{
		fEntryCountChangedEvent.Raise(*this, Interop::EventArgs::kEmpty);
	}
}

bool AlertListPane::RemoveLogEntry(const LogEntry& entry)
{
	// Attempt to remove the given entry from the list control, if it exists.
	for (int index = 0; index < fListControl.GetItemCount(); index++)
	{
		// Fetch the next entry.
		auto nextEntryPointer = (LogEntry*)fListControl.GetItemData(index);
		if (nextEntryPointer && (nextEntryPointer->GetIntegerId() == entry.GetIntegerId()))
		{
			// Found it! Remove it from the list control.
			fListControl.DeleteItem(index);

			// Delete our copy of the entry from collection.
			auto iterator = fLogEntryPointerSet.find(nextEntryPointer);
			if (iterator != fLogEntryPointerSet.end())
			{
				fLogEntryPointerSet.erase(iterator);
				delete nextEntryPointer;
			}

			// Update the error/warning count.
			switch (entry.GetType())
			{
				case LogEntry::Type::kError:
					if (fErrorCount > 0)
					{
						fErrorCount--;
					}
					break;
				case LogEntry::Type::kWarning:
					if (fWarningCount > 0)
					{
						fWarningCount--;
					}
					break;
			}

			// Notify the system that this list's entry count has changed.
			fEntryCountChangedEvent.Raise(*this, Interop::EventArgs::kEmpty);
			return true;
		}
	}

	// Failed to find the given log entry in the collection.
	return false;
}

DWORD AlertListPane::GetSelectedLogEntryId() const
{
	auto itemPosition = fListControl.GetFirstSelectedItemPosition();
	if (itemPosition)
	{
		int index = fListControl.GetNextSelectedItem(itemPosition);
		if (index >= 0)
		{
			auto entryPointer = (LogEntry*)fListControl.GetItemData(index);
			if (entryPointer)
			{
				return entryPointer->GetIntegerId();
			}
		}
	}
	return 0;
}

int AlertListPane::GetErrorLogEntryCount() const
{
	return fErrorCount;
}

int AlertListPane::GetWarningLogEntryCount() const
{
	return fWarningCount;
}

BOOL AlertListPane::PreTranslateMessage(MSG* messagePointer)
{
	// Validate.
	if (!messagePointer)
	{
		return FALSE;
	}

	// If the "Enter" key was released while a log entry was selected,
	// then request the owner to show the currently selected entry.
	if (messagePointer->hwnd == fListControl.GetSafeHwnd())
	{
		if ((WM_KEYUP == messagePointer->message) && (VK_RETURN == messagePointer->wParam))
		{
			if (GetSelectedLogEntryId() != 0)
			{
				// Notify this window's owner to show the currently selected entry.
				fRequestingShowSelectedEntryEvent.Raise(*this, Interop::EventArgs::kEmpty);

				// Return true to handle to signal that this message was handled.
				return TRUE;
			}
		}
	}

	// Let this window do its default handling of this message.
	return CDockablePane::PreTranslateMessage(messagePointer);
}

#pragma endregion
