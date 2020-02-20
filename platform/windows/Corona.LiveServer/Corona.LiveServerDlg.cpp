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
#include "afxdialogex.h"
#include "../../../external/winutil/BrowseDirDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CCoronaLiveServerDlg dialog



CCoronaLiveServerDlg::CCoronaLiveServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCoronaLiveServerDlg::IDD, pParent)
{
	HICON largeAppIconHandle = nullptr;
	HICON smallAppIconHandle = nullptr;
	HINSTANCE instanceHandle = AfxGetInstanceHandle();
	// First, attempt to load the appropriate icon sizes matching the system's current DPI scale factor.
	// Note: The below API is only supported on Windows Vista and newer OS versions.
	typedef HRESULT(WINAPI *LoadIconMetricCallback)(HINSTANCE, PCWSTR, int, HICON*);
	HMODULE moduleHandle = ::LoadLibraryW(L"ComCtl32");
	LoadIconMetricCallback loadIconMetricCallback = nullptr;
	if (moduleHandle)
	{
		loadIconMetricCallback = (LoadIconMetricCallback)::GetProcAddress(moduleHandle, "LoadIconMetric");
	}
	if (loadIconMetricCallback)
	{
		loadIconMetricCallback(instanceHandle, MAKEINTRESOURCE(IDR_MAINFRAME), LIM_LARGE, &largeAppIconHandle);
		loadIconMetricCallback(instanceHandle, MAKEINTRESOURCE(IDR_MAINFRAME), LIM_SMALL, &smallAppIconHandle);
	}

	// If we've failed to load icons above, then fallback to loading them via older Win32 APIs.
	// Note: This is always the case for Windows XP.
	if (!largeAppIconHandle)
	{
		largeAppIconHandle = ::LoadIconW(instanceHandle, MAKEINTRESOURCE(IDR_MAINFRAME));
	}
	if (!smallAppIconHandle)
	{
		int iconWidth = ::GetSystemMetrics(SM_CXSMICON);
		int iconHeight = ::GetSystemMetrics(SM_CYSMICON);
		if (iconWidth <= 0)
		{
			iconWidth = 16;
		}
		if (iconHeight <= 0)
		{
			iconHeight = 16;
		}
		smallAppIconHandle = (HICON)::LoadImageW(
			instanceHandle, MAKEINTRESOURCEW(IDR_MAINFRAME), IMAGE_ICON, iconWidth, iconHeight, LR_DEFAULTCOLOR);
		if (!smallAppIconHandle)
		{
			smallAppIconHandle = largeAppIconHandle;
		}
	}

	m_hIconBig = largeAppIconHandle;
	m_hIconSmall = smallAppIconHandle;

}

void CCoronaLiveServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROJECT_LIST, m_ProjectList);
	DDX_Control(pDX, IDC_ADD, m_AddButton);
	DDX_Control(pDX, IDC_REMOVE, m_RemoveButton);
}

BEGIN_MESSAGE_MAP(CCoronaLiveServerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_REMOVE, &CCoronaLiveServerDlg::OnBnClickedRemove)
	ON_BN_CLICKED(IDC_ADD, &CCoronaLiveServerDlg::OnBnClickedAdd)
	ON_LBN_SELCHANGE(IDC_PROJECT_LIST, &CCoronaLiveServerDlg::OnLbnSelchangeProjectList)
	ON_WM_DROPFILES()
	ON_MESSAGE(WM_ICON_NOTIFY, OnTrayNotification)
	ON_COMMAND(ID_POPUP_EXIT, &CCoronaLiveServerDlg::OnPopupExit)
	ON_COMMAND(ID_POPUP_SHOW, &CCoronaLiveServerDlg::OnPopupShow)
	ON_WM_COPYDATA()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_LBN_DBLCLK(IDC_PROJECT_LIST, &CCoronaLiveServerDlg::OnLbnDblclkProjectList)
END_MESSAGE_MAP()


// CCoronaLiveServerDlg message handlers

LRESULT CCoronaLiveServerDlg::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
	return m_trayIcon.OnTrayNotification(wParam, lParam);
}


BOOL CCoronaLiveServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString itemStr;
		itemStr.LoadString(IDS_EXIT);
		pSysMenu->AppendMenu(MF_STRING, ID_POPUP_EXIT, itemStr);

		itemStr.LoadString(IDS_ABOUTBOX);
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, itemStr);

		itemStr.LoadString(IDS_ONLINE_HELP);
		pSysMenu->AppendMenu(MF_STRING, IDM_ONLINEHLELP, itemStr);

		pSysMenu->AppendMenu(MF_SEPARATOR);

		itemStr.LoadStringW(IDS_SET_HTML_DIR);
		pSysMenu->AppendMenu(MF_STRING, IDM_SET_HTML_DIR, itemStr);

		m_openURLButtonAdded = false;
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIconBig, TRUE);			// Set big icon
	SetIcon(m_hIconSmall, FALSE);		// Set small icon

	CString tooltip;
	tooltip.LoadString(IDS_LIVE_SERVER_TRAY_TOOLTIP);
	m_trayIcon.Create(this, WM_ICON_NOTIFY, tooltip, m_hIconSmall, IDR_POPUP_MENU);

	ParseCommandLine(theApp.m_lpCmdLine);

	GetClientRect(&m_rcDlg);

	m_ProjectList.GetWindowRect(&m_rcProj);
	ScreenToClient(&m_rcProj);

	m_AddButton.GetWindowRect(&m_rcAdd);
	ScreenToClient(&m_rcAdd);

	m_RemoveButton.GetWindowRect(&m_rcRemove);
	ScreenToClient(&m_rcRemove);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCoronaLiveServerDlg::ParseCommandLine(CString str)
{
	if (str.GetLength() == 0)
		return;

	if (str.Find(_T("html5:"), 0) == 0)
	{
		SetHTML5Directory(str.Mid(6));
	}
	else
	{
		AddProject(str);
	}
}

void CCoronaLiveServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == ID_POPUP_EXIT)
	{
		OnPopupExit();
	}
	else if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if (nID == IDM_ONLINEHLELP)
	{
		ShowHelp();
	}
	else if (nID == IDM_SET_HTML_DIR)
	{
		BrowseForHTML5Dir();
	}
	else if (nID == IDM_OPEN_HTML_URL)
	{
		OpenHTML5URL();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCoronaLiveServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIconBig);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCoronaLiveServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIconBig);
}



BOOL CCoronaLiveServerDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void CCoronaLiveServerDlg::ReEnableControls()
{
	m_RemoveButton.EnableWindow(m_ProjectList.GetCurSel() != LB_ERR);
}


void CCoronaLiveServerDlg::OnBnClickedRemove()
{
	int sel = m_ProjectList.GetCurSel();
	if (sel != LB_ERR)
	{
		CString str;
		m_ProjectList.GetText(sel, str);
		theApp.GetServer()->Stop(str.GetBuffer());
		m_ProjectList.DeleteString(sel);
	}
}


void CCoronaLiveServerDlg::OnBnClickedAdd()
{
	CString projectPath;
	if (CBrowseDirDialog::Browse(projectPath))
	{
		AddProject(projectPath);
	}	
}

void CCoronaLiveServerDlg::SetHTML5Directory(CString str)
{
	str.Trim(_T('"'));
	str.TrimRight(_T("\\/"));

	CString tmp = str;
	GetFullPathName(tmp, MAX_PATH, str.GetBuffer(MAX_PATH), NULL);
	str.ReleaseBuffer();

	if (!PathFileExists(str))
	{
		return;
	}

	theApp.GetHttpServer()->Set(str.GetBuffer());

	if (theApp.GetHttpServer()->GetServerAddress().empty())
	{
		return;
	}

	if (!m_openURLButtonAdded)
	{
		CMenu* pSysMenu = GetSystemMenu(FALSE);
		if (pSysMenu != NULL)
		{
			CString itemStr;
			itemStr.LoadStringW(IDS_OPEN_HTML_URL);
			pSysMenu->AppendMenu(MF_STRING, IDM_OPEN_HTML_URL, itemStr);
		}
		m_openURLButtonAdded = true;
	}
	
	OpenHTML5URL();
}


bool CCoronaLiveServerDlg::AddProject(CString str)
{
	str.Trim(_T('"'));
	str.TrimRight(_T("\\/"));

	CString tmp = str;
	GetFullPathName(tmp, MAX_PATH, str.GetBuffer(MAX_PATH), NULL);
	str.ReleaseBuffer();

	CString mainLua(_T("\\main.lua"));
	if (str.Right(mainLua.GetLength()) == mainLua)
	{
		str = str.Left(str.GetLength() - mainLua.GetLength());
	}

	bool addToUI = true;
	if (m_ProjectList.FindStringExact(-1, str) != LB_ERR)
	{
		addToUI = false;
	}

	CString cl = str + _T("\\.CoronaLiveBuild");
	if (!PathFileExists(cl)) 
	{
		AfxMessageBox(IDS_PROJECT_NOT_CONFIGURED);
		return false;
	}

	int err = theApp.GetServer()->Add(str.GetBuffer());
	if (err != 0)
	{
		
		if (10 == err) // if no bounjour, exit, so people don't get confused
		{
			if (AfxMessageBox(IDS_ERR_UNKNOWN + err, MB_YESNO) == IDYES)
			{
				ShellExecute(0, 0, L"https://support.apple.com/kb/DL999", 0, 0, SW_SHOW);
			}
			OnPopupExit();
		}
		else
		{
			AfxMessageBox(IDS_ERR_UNKNOWN + err);
		}
		return false;
	}

	if (addToUI)
	{
		m_ProjectList.AddString(str);
	}

	OnPopupShow();

	return true;
}


void CCoronaLiveServerDlg::OnLbnSelchangeProjectList()
{
	ReEnableControls();
}


void CCoronaLiveServerDlg::OnDropFiles(HDROP hDropInfo)
{
	CString sFile;
	DWORD   nBuffer = 0;

	int nFilesDropped = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	for (int i = 0; i<nFilesDropped; i++)
	{
		nBuffer = DragQueryFile(hDropInfo, i, NULL, 0);
		DragQueryFile(hDropInfo, i, sFile.GetBuffer(nBuffer + 1), nBuffer + 1);
		sFile.ReleaseBuffer();

		CString cl = sFile + _T("\\.CoronaLiveBuild");
		if (PathFileExists(cl))
		{
			AddProject(sFile);
		}
	}

	// Free the memory block containing the dropped-file information 
	DragFinish(hDropInfo);
}


void CCoronaLiveServerDlg::OnPopupExit()
{
	OnPopupShow();
	PostMessage(WM_COMMAND, IDOK, 0);
}


void CCoronaLiveServerDlg::OnPopupShow()
{
	if (!IsWindowVisible())
	{
		ShowWindow(SW_SHOW);
	}
	SetForegroundWindow();
}


void CCoronaLiveServerDlg::OnCancel()
{
	if (!theApp.GetProfileInt(_T("UI"), _T("Tray warning shown"), 0))
	{ // show warning message only once
		AfxMessageBox(IDS_INFO_CLOSE_TO_TRAY, MB_ICONINFORMATION | MB_OK);
		theApp.WriteProfileInt(_T("UI"), _T("Tray warning shown"), 1);
	}
	ShowWindow(SW_HIDE);
}


BOOL CCoronaLiveServerDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	LPCTSTR prefix = CCoronaLiveServerApp::sLiveDataPrefix;
	LPCTSTR data = (LPTSTR)pCopyDataStruct->lpData;
	auto prefixLen = _tcslen(prefix);
	if (pCopyDataStruct->cbData > (prefixLen*sizeof(TCHAR)) )
	{
		if (_tcsncmp(data, prefix, prefixLen) == 0)
		{
			CString str(data + prefixLen);
			ParseCommandLine(str);
		}
	}
	return CDialog::OnCopyData(pWnd, pCopyDataStruct);
}


BOOL CCoronaLiveServerDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if(ID_HELP == wParam)
	{
		return ShowHelp();
	}
	else
	{
		return CDialog::OnCommand(wParam, lParam);
	}
}

void CCoronaLiveServerDlg::BrowseForHTML5Dir()
{
	CString projectPath;
	if (CBrowseDirDialog::Browse(projectPath))
	{
		SetHTML5Directory(projectPath);
	}
}

void CCoronaLiveServerDlg::OpenHTML5URL()
{
	std::string url = theApp.GetHttpServer()->GetServerAddress();
	
	if (!url.empty())
	{
		try {
			CString curl(url.c_str());
			if ((unsigned long)ShellExecute(0, _T("open"), curl, 0, 0, SW_SHOW) <= 32ul)
			{
				CString error = _T("Unable to open default browser at ") + curl;
				AfxMessageBox(error);
			}
		}
		catch (...) {
		}
	}
}

BOOL CCoronaLiveServerDlg::ShowHelp()
{
	try {
		ShellExecute(0, _T("open"), _T("https://docs.coronalabs.com/guide/distribution/liveBuild/"), 0, 0, SW_SHOW);
	}
	catch(...) {
	}
	return TRUE;
}



#pragma comment(lib, "version.lib")

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString productName, copyright;

	// This code is borrowed from ApplicationServices.cpp
	// Fetch the version info resource from the executable file.
	LPVOID versionInfoResourcePointer = nullptr;
	{
		auto resourceHandle = ::FindResourceW(nullptr, MAKEINTRESOURCEW(VS_VERSION_INFO), RT_VERSION);
		if (resourceHandle)
		{
			auto resourceDataHandle = ::LoadResource(nullptr, resourceHandle);
			if (resourceDataHandle)
			{
				versionInfoResourcePointer = ::LockResource(resourceDataHandle);
			}
		}
	}
	if (versionInfoResourcePointer)
	{
		// Fetch an array of [LanguageId,CodePageId] objects.
		struct TranslationInfo
		{
			WORD wLanguageId;
			WORD wCodePageId;
		} *translationInfoArray = nullptr;
		UINT translationInfoArrayByteLength = 0;
		::VerQueryValueW(
			versionInfoResourcePointer, L"\\VarFileInfo\\Translation",
			(LPVOID*)&translationInfoArray, &translationInfoArrayByteLength);

		// Traverse each localized version info structure.
		int translationInfoCount = translationInfoArrayByteLength / sizeof(TranslationInfo);
		for (int index = 0; index < translationInfoCount; index++)
		{
			// Generate the root sub-block entry name for this localized entry.
			const size_t kMaxSubBlockRootNameLength = 32;
			wchar_t subBlockRootName[kMaxSubBlockRootNameLength];
			subBlockRootName[0] = L'\0';
			_snwprintf_s(
				subBlockRootName, kMaxSubBlockRootNameLength, L"\\StringFileInfo\\%04x%04x\\",
				translationInfoArray[index].wLanguageId, translationInfoArray[index].wCodePageId);

			// Fetch product name.
			{
				BYTE* bufferPointer = nullptr;
				UINT bufferByteLength = 0;
				std::wstring subBlockName(subBlockRootName);
				subBlockName.append(L"ProductName");
				::VerQueryValueW(versionInfoResourcePointer, subBlockName.c_str(), (LPVOID*)&bufferPointer, &bufferByteLength);
				if (bufferPointer && (bufferByteLength > 0))
				{
					productName = (wchar_t*)bufferPointer;
				}
			}

			// Fetch copyright string.
			{
				BYTE* bufferPointer = nullptr;
				UINT bufferByteLength = 0;
				std::wstring subBlockName(subBlockRootName);
				subBlockName.append(L"LegalCopyright");
				::VerQueryValueW(versionInfoResourcePointer, subBlockName.c_str(), (LPVOID*)&bufferPointer, &bufferByteLength);
				if (bufferPointer && (bufferByteLength > 0))
				{
					copyright = (wchar_t*)bufferPointer;
				}
			}
		}
	}

	CWnd *productLine = GetDlgItem(IDC_ABOUT_PRODUCT);
	if (productLine && !productName.IsEmpty())
	{
		productLine->SetWindowText(productName);
	}

	CWnd *copyrightLine = GetDlgItem(IDC_ABOUT_COPYRIGHT);
	if (copyrightLine && !copyright.IsEmpty())
	{
		copyrightLine->SetWindowText(copyright);
	}

	return TRUE;  
}


void CCoronaLiveServerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	RECT newDlg;
	GetClientRect(&newDlg);
	
	if (m_ProjectList.m_hWnd) {
		RECT &rcOrig = m_rcProj;
		RECT rc = rcOrig;
		rc.right = newDlg.right - (m_rcDlg.right - rcOrig.right);
		rc.bottom = newDlg.bottom - (m_rcDlg.bottom - rcOrig.bottom);
		m_ProjectList.MoveWindow(&rc);
	}

	if (m_AddButton.m_hWnd) {
		RECT &rcOrig = m_rcAdd;
		RECT rc = rcOrig;
		rc.bottom = newDlg.bottom - (m_rcDlg.bottom - rcOrig.bottom);
		rc.top = newDlg.bottom - (m_rcDlg.bottom - rcOrig.top);
		m_AddButton.MoveWindow(&rc);
	}

	if (m_RemoveButton.m_hWnd) {
		RECT &rcOrig = m_rcRemove;
		RECT rc = rcOrig;
		rc.bottom = newDlg.bottom - (m_rcDlg.bottom - rcOrig.bottom);
		rc.top = newDlg.bottom - (m_rcDlg.bottom - rcOrig.top);
		m_RemoveButton.MoveWindow(&rc);
	}
}


void CCoronaLiveServerDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = m_rcProj.right;
	lpMMI->ptMinTrackSize.y = m_rcProj.bottom;

	CDialog::OnGetMinMaxInfo(lpMMI);
}


void CCoronaLiveServerDlg::OnLbnDblclkProjectList()
{
	int sel = m_ProjectList.GetCurSel();
	if (sel != LB_ERR)
	{
		CString str;
		m_ProjectList.GetText(sel, str);
		try {
			ShellExecute(m_hWnd, _T("open"), str, 0, 0, SW_SHOW);
		}
		catch (...) {
		}
	}
}
