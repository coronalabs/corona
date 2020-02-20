//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Interop\Ipc\AsyncPipeReader.h"
#include "Interop\Ipc\Process.h"
#include "Interop\DateTime.h"
#include "AlertListPane.h"
#include "LogEntryCollection.h"
#include "MainApp.h"
#include <memory>
#include <string>
#include <vector>


class MainFrame : public CFrameWndEx
{
	protected: 
		DECLARE_DYNAMIC(MainFrame)

	public:
		enum class IpcCommandId
		{
			kIsFileSelected = 1,
			kIsRunning = 2,
			kRun = 3,
			kStop = 4
		};

		MainFrame();
		virtual ~MainFrame();

		virtual BOOL PreCreateWindow(CREATESTRUCT& createSettings);
		virtual BOOL PreTranslateMessage(MSG* messagePointer);
		virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
		void StartMonitoringApp(const wchar_t* filePath, const wchar_t* commandLineArguments);
		bool IsMonitoring();
		const wchar_t* GetMonitoredAppFilePath() const;
		const wchar_t* GetCustomWindowName() const;
		void SetCustomWindowName(const wchar_t* name);

	protected:
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnClose();
		afx_msg void OnGetMinMaxInfo(MINMAXINFO* minMaxInfoPointer);
		afx_msg void OnSetFocus(CWnd* lastFocusedWindowPointer);
		afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
		afx_msg void OnTimer(UINT_PTR eventId);
		afx_msg BOOL OnCopyData(CWnd* windowPointer, COPYDATASTRUCT* dataPointer);
		afx_msg void OnLinkNotification(LPNMHDR notifyHeaderPointer, LRESULT* resultPointer);
		afx_msg void OnFileOpen();
		afx_msg void OnUpdateFileOpen(CCmdUI* commandPointer);
		afx_msg void OnFileSaveAs();
		afx_msg void OnUpdateFileSaveAs(CCmdUI* commandPointer);
		afx_msg void OnProcessStart();
		afx_msg void OnUpdateProcessStart(CCmdUI* commandPointer);
		afx_msg void OnProcessStop();
		afx_msg void OnUpdateProcessStop(CCmdUI* commandPointer);
		afx_msg void OnCopy();
		afx_msg void OnUpdateCopy(CCmdUI* commandPointer);
		afx_msg void OnClearAll();
		afx_msg void OnFind();
		afx_msg void OnFindNext();
		afx_msg void OnFindPrevious();
		afx_msg void OnSetupSubmenu();
		afx_msg void OnAppAbout();
		afx_msg void OnHelp();
		afx_msg void OnVisualThemeSelectedById(UINT id);
		afx_msg void OnUpdateVisualThemeMenuItem(CCmdUI* commandPointer);
		afx_msg void OnUpdateAlwaysEnabledCommand(CCmdUI* commandPointer);
		DECLARE_MESSAGE_MAP()

	private:
		struct PendingLogEntry
		{
			Interop::DateTime Timestamp;
			std::shared_ptr<const std::string> Text;
		};

		void OnVisualThemeChanged(MainApp& sender, const Interop::EventArgs& arguments);
		void OnProcessExited(Interop::Ipc::Process& sender, const Interop::EventArgs& arguments);
		void OnPipeClosed(Interop::Ipc::AsyncPipeReader& sender, const Interop::EventArgs& arguments);
		void OnPipeReceivedData(Interop::Ipc::AsyncPipeReader& sender, const Interop::Ipc::IODataEventArgs& arguments);
		void OnAlertListRequestingShowSelectedEntry(AlertListPane& sender, const Interop::EventArgs& arguments);
		void OnAlertListEntryCountChanged(AlertListPane& sender, const Interop::EventArgs& arguments);
		void UpdateStatusBarLogEntryCount();
		void DoFind(bool isForwardFindRequested);
		void QueueLogEntry(const Interop::DateTime& timestamp, const char* text);
		void QueueLogEntry(const Interop::DateTime& timestamp, const wchar_t* text);
		void QueueLogEntry(const Interop::DateTime& timestamp, std::shared_ptr<const std::string> text);
		void ProcessQueuedLogEntries();


		CMFCToolBar fToolBar;
		CMFCStatusBar fStatusBar;
		AlertListPane fAlertListPane;
		CRichEditCtrl fRichEditControl;
		LogEntryCollection fLogEntryCollection;
		std::vector<PendingLogEntry> fPendingLogEntryQueue;
		bool fIsProcessingQueuedLogEntries;
		CStringW fAppFilePath;
		CStringW fCommandLineArguments;
		CStringW fCustomWindowName;
		HWND fFindEditControlHandle;
		MainApp::VisualThemeChangedEvent::MethodHandler<MainFrame> fVisualThemeChangedEventHandler;
		Interop::Ipc::Process::ExitedEvent::MethodHandler<MainFrame> fProcessExitedEventHandler;
		Interop::Ipc::AsyncPipeReader::ClosedEvent::MethodHandler<MainFrame> fPipeClosedEventHandler;
		Interop::Ipc::AsyncPipeReader::ReceivedDataEvent::MethodHandler<MainFrame> fPipeReceivedDataEventHandler;
		AlertListPane::RequestingShowSelectedEntryEvent::MethodHandler<MainFrame> fAlertListRequestingShowSelectedEntryEventHandler;
		AlertListPane::EntryCountChangedEvent::MethodHandler<MainFrame> fAlertListEntryCountChangedEventHandler;
		std::shared_ptr<Interop::Ipc::Process> fProcessPointer;
		Interop::Ipc::AsyncPipeReader* fAsyncStdInReaderPointer;
};
