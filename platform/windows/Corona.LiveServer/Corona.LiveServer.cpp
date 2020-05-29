//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Corona.LiveServer.h"
#include "Corona.LiveServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCoronaLiveServerApp

BEGIN_MESSAGE_MAP(CCoronaLiveServerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCoronaLiveServerApp construction

CCoronaLiveServerApp::CCoronaLiveServerApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	m_Server = nullptr;
	m_HttpServer = nullptr;
}

CCoronaLiveServerApp::~CCoronaLiveServerApp()
{
	delete m_Server;
	delete m_HttpServer;
}

CoronaLiveServerWin* CCoronaLiveServerApp::GetServer()
{
	if (m_Server == nullptr)
	{
		m_Server = new CoronaLiveServerWin();
	}
	return m_Server;
}

Rtt_HTTPServer* CCoronaLiveServerApp::GetHttpServer()
{
	if (m_HttpServer == nullptr)
	{
		m_HttpServer = new Rtt_HTTPServer();
	}
	return m_HttpServer;
}

// The one and only CCoronaLiveServerApp object

CCoronaLiveServerApp theApp;


// CCoronaLiveServerApp initialization

LPCTSTR CCoronaLiveServerApp::sNamedMutexId = _T("Global\\CoronaLabs.Corona.LiveServer.SingleInstance");
LPCTSTR CCoronaLiveServerApp::sLiveDataPrefix = _T("CORONA LIVE 30fc2776 ");


BOOL CCoronaLiveServerApp::InitInstance()
{
	WNDCLASS wc;
	::GetClassInfo(AfxGetInstanceHandle(), _T("#32770"), &wc);
	wc.lpszClassName = _T("CoronaLiveDlgClass");
	AfxRegisterClass(&wc);

	HANDLE hMutex = CreateMutex(0, TRUE, sNamedMutexId);
	std::unique_ptr<void, decltype(&ReleaseMutex)> releaseMutex(hMutex, ReleaseMutex);
	std::unique_ptr<void, decltype(&CloseHandle)> closeMutexHandle(hMutex, CloseHandle);
	if (hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		HWND hWnd = FindWindow(_T("CoronaLiveDlgClass"), _T("Corona Live Server"));
		if (hWnd != NULL)
		{
			CString cmd(m_lpCmdLine);
			if (cmd.GetLength() > 0)
			{
				cmd.Insert(0, sLiveDataPrefix);
				COPYDATASTRUCT cds;
				cds.cbData = (cmd.GetLength()+1)*sizeof(TCHAR);
				cds.lpData = cmd.GetBuffer();
				SendMessage(hWnd, WM_COPYDATA, 0, (LPARAM)&cds);
			}
			else
			{
				ShowWindow(hWnd, SW_SHOW);
				SwitchToThisWindow(hWnd, TRUE);
			}
		}
		return 0;
	}

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}
	
	// Standard initialization

	SetRegistryKey(_T("Corona Labs"));

	CCoronaLiveServerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
	}

	delete m_Server;
	m_Server = nullptr;

	delete m_HttpServer;
	m_HttpServer = nullptr;
	
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}



int CCoronaLiveServerApp::ExitInstance()
{
	delete m_Server;
	m_Server = nullptr;

	delete m_HttpServer;
	m_HttpServer = nullptr;

	return CWinApp::ExitInstance();
}
