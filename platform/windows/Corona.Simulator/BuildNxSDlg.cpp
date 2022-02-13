//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Simulator.h"
#include "SimulatorView.h"
#include "Resource.h"
#include "CoronaProject.h"
#include "BuildNxSDlg.h"
#include "BuildProgressDlg.h"
#include "BuildResult.h"
#include "WinGlobalProperties.h"
#include "WinString.h"
#include "BrowseDirDialog.h"
#include "HtmlMessageDlg.h"
#include "MessageDlg.h"
#include "CoronaInterface.h"
#include "Core/Rtt_Build.h"
#include "Rtt_SimulatorAnalytics.h"
#include "Rtt_TargetDevice.h"

// CBuildNxSDlg dialog

IMPLEMENT_DYNAMIC(CBuildNxSDlg, CDialog)

CBuildNxSDlg::CBuildNxSDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBuildNxSDlg::IDD, pParent)
	, m_pProject(NULL)
	, m_nValidFields(0)
{
}

CBuildNxSDlg::~CBuildNxSDlg()
{
}

void CBuildNxSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CBuildNxSDlg, CDialog)
	ON_BN_CLICKED(IDC_BUILD_BROWSE_SAVETO, &CBuildNxSDlg::OnBrowseSaveto)
	ON_BN_CLICKED(IDC_INCLUDE_STANDART_RESOURCES, &CBuildNxSDlg::OnClickedIncludeStandartResources)
END_MESSAGE_MAP()

// OnInitDialog - restore settings from m_pProject if available.
// Choose likely defaults for non-saved settings.
BOOL CBuildNxSDlg::OnInitDialog()
{
	WinString stringConverter;

	// Initialize base class first.
	CDialog::OnInitDialog();

	// Do not display this window if not given a project settings object.
	if (!m_pProject)
	{
		CDialog::OnCancel();
		return TRUE;
	}

	//Set inital project type now that we have Android/NxS (used by the BuildProgressDlg - does not indicate target app store)
	m_pProject->SetTargetPlatform(Rtt::TargetDevice::kNxSPlatform);

	// Limit the text length of CEdit boxes.
	((CEdit*)GetDlgItem(IDC_BUILD_APPNAME))->SetLimitText(128);
	((CEdit*)GetDlgItem(IDC_BUILD_NMETA_PATH))->SetLimitText(MAX_PATH - 1);
	((CEdit*)GetDlgItem(IDC_BUILD_SAVETO))->SetLimitText(MAX_PATH - 1);

	m_nValidFields = 0;

	// If there isn't a package name, create one by reversing the user's email address and adding the app name
	if (m_pProject->GetPackage().IsEmpty())
	{
		CString package("com.solar2d.app.");
		package.Append(m_pProject->GetName());

		for (int c = 0; c < package.GetLength(); c++)
		{
			if (package[c] > 255 || (!isalnum(package[c]) && package[c] != '.' && package[c] != '_'))
			{
				package.SetAt(c, _T('_'));
			}
		}

		package.Trim(_T("._"));

		m_pProject->SetPackage(package);
	}

	fNmetaPath.SetString(L"");
	fNmetaPath.Append(m_pProject->GetDir());
	fNmetaPath.Append(L"\\build.nmeta");

	SetDlgItemText(IDC_BUILD_APPNAME, m_pProject->GetName());
	SetDlgItemText(IDC_BUILD_NMETA_PATH, fNmetaPath);
	SetDlgItemText(IDC_BUILD_PACKAGE, m_pProject->GetPackage());
	SetDlgItemText(IDC_BUILD_PROJECTPATH, m_pProject->GetDir());

	// Initialize build destination directory.
	SetDlgItemText(IDC_BUILD_SAVETO, m_pProject->GetSaveDir());

	// Set up checkboxs
	CheckDlgButton(IDC_INCLUDE_STANDART_RESOURCES, m_pProject->GetUseStandartResources() ? BST_CHECKED : BST_UNCHECKED);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// SetProject - called before DoModal to set the associated project.
void CBuildNxSDlg::SetProject(const std::shared_ptr<CCoronaProject>& pProject)
{
	m_pProject = pProject;  // leave unchanged until OnOK()
}

// OnBrowseSaveto - show directory selection dialog for build destination dir
void CBuildNxSDlg::OnBrowseSaveto()
{
	CString sDir = _T("");
	GetDlgItemText(IDC_BUILD_SAVETO, sDir);

	// Default to the system's Documents if the above field is empty.
	if (sDir.IsEmpty())
	{
		::SHGetFolderPath(nullptr, CSIDL_MYDOCUMENTS, nullptr, 0, sDir.GetBuffer(MAX_PATH));
		sDir.ReleaseBuffer();
	}

	if (CBrowseDirDialog::Browse(sDir, IDS_SELECT_BUILD_OUTPUT_FOLDER_DESCRIPTION))
	{
		SetDlgItemText(IDC_BUILD_SAVETO, sDir);
	}
}

void CBuildNxSDlg::OnClickedIncludeStandartResources()
{
	m_pProject->SetUseStandartResources(IsDlgButtonChecked(IDC_INCLUDE_STANDART_RESOURCES) == BST_CHECKED);
}

// OnOK - build project
// Retrieve values from dialog, save to registry, 
// get key alias password if necessary
// test key alias password
// call core code to package project and send to server for building
void CBuildNxSDlg::OnOK()  // OnBuild()
{
	WinString stringBuffer;
	CString sValue;
	CString sBuildDir;
	CString sNmetaPath;
	CString sPackageName;
	CString sAppName;
	int iVersionCode;

	// Fetch and validate field values.

	GetDlgItemText(IDC_BUILD_APPNAME, sAppName);
	sAppName.Trim();
	if (sAppName.IsEmpty())
	{
		DisplayWarningMessage(IDS_BUILD_APP_NAME_NOT_PROVIDED);
		GetDlgItem(IDC_BUILD_APPNAME)->SetFocus();
		LogAnalytics("build-bungled", "reason", "BUILD_APP_NAME_NOT_PROVIDED");
		return;
	}

	GetDlgItemText(IDC_BUILD_NMETA_PATH, sNmetaPath);
	sNmetaPath.Trim();
	if (sNmetaPath.IsEmpty())
	{
		DisplayWarningMessage(IDS_BUILD_NMETA_NAME_NOT_PROVIDED);
		GetDlgItem(IDC_BUILD_NMETA_PATH)->SetFocus();
		LogAnalytics("build-bungled", "reason", "BUILD_NMETA_NAME_NOT_PROVIDED");
		return;
	}

	GetDlgItemText(IDC_BUILD_SAVETO, sBuildDir);
	sBuildDir.Trim();
	if (sBuildDir.IsEmpty())
	{
		DisplayWarningMessage(IDS_BUILD_PATH_NOT_PROVIDED);
		GetDlgItem(IDC_BUILD_SAVETO)->SetFocus();
		LogAnalytics("build-bungled", "reason", "BUILD_PATH_NOT_PROVIDED");
		return;
	}

	if (sBuildDir.CompareNoCase(m_pProject->GetDir()) == 0)
	{
		DisplayWarningMessage(IDS_BUILD_PATH_INVALID);
		GetDlgItem(IDC_BUILD_SAVETO)->SetFocus();
		LogAnalytics("build-bungled", "reason", "BUILD_PATH_INVALID");
		return;
	}

	if (::PathIsDirectory(sBuildDir) == FALSE)
	{
		// The given path does not exist. Attempt to create it. If we are unable to create the directory,
		// then that may mean the path is invalid or we do not have permission to create it at that location.
		int result = ::SHCreateDirectoryEx(NULL, sBuildDir, NULL);
		if (result != ERROR_SUCCESS)
		{
			DisplayWarningMessage(IDS_BUILD_PATH_CREATION_FAILED);
			GetDlgItem(IDC_BUILD_SAVETO)->SetFocus();
			LogAnalytics("build-bungled", "reason", "BUILD_PATH_CREATION_FAILED");
			return;
		}
	}

	// Store field settings to project.
	m_pProject->SetName(sAppName);
	m_pProject->SetSaveDir(sBuildDir);

	// Save all project settings and then build the project.

	LogAnalytics("build");

	m_pProject->Save();
	CBuildProgressDlg buildDialog;
	buildDialog.SetProjectSettings(m_pProject);
	buildDialog.SetNoServer();
	buildDialog.DoModal();
	CBuildResult result = buildDialog.GetBuildResult();

	// Inform the user if the build has failed. Do not close this window on failure.
	if (result.HasFailed())
	{
		CString title;
		title.LoadString(IDS_WARNING);
		CHtmlMessageDlg htmlMessageDialog;
		htmlMessageDialog.SetTitle(title);
		htmlMessageDialog.SetHtmlFromMarkdown(result.GetStatusMessage());
		htmlMessageDialog.DoModal();
		return;
	}

	// The build has succeeded. Inform the user.
	CMessageDlg messageDlg(this);
	messageDlg.SetText(IDS_BUILD_SUCCEEDED);
	messageDlg.SetDefaultText(IDS_DONE);
	messageDlg.SetAltText(IDS_VIEW_EXPLORER);
	messageDlg.SetFolder(m_pProject->GetSaveDir());
	messageDlg.SetIconStyle(MB_ICONINFORMATION);
//	messageDlg.SetButton3Text(IDS_OPEN_URL_HTML);
	if (messageDlg.DoModal() == ID_MSG_BUTTON3)
	{
		// start live server with URL
		CString sCoronaLiveServerExe = ((CSimulatorApp*)AfxGetApp())->GetApplicationDir() + _T("\\Corona.LiveServer.exe");
		try
		{
			CString sAppPath = CString(_T("html5:")) + m_pProject->GetSaveDir() + _T("\\") + m_pProject->GetName();
			::ShellExecute(nullptr, nullptr, sCoronaLiveServerExe, sAppPath, nullptr, SW_SHOWNORMAL);
		}
		catch (...) {}
	}


	// Close this window.
	CDialog::OnOK();
}

// OnCancel - clean up, exit
void CBuildNxSDlg::OnCancel()
{
	CDialog::OnCancel();
}

// Display a warning message box with the given message.
// Argument "nMessageID" is a unique ID to a string in the resource file's string table.
void CBuildNxSDlg::DisplayWarningMessage(UINT nMessageID)
{
	CString title;
	CString message;

	title.LoadString(IDS_WARNING);
	message.LoadString(nMessageID);
	MessageBox(message, title, MB_OK | MB_ICONWARNING);
}

void CBuildNxSDlg::LogAnalytics(const char* eventName, const char* key, const char* value)
{
	Rtt_ASSERT(GetWinProperties()->GetAnalytics() != NULL);
	Rtt_ASSERT(eventName != NULL && strlen(eventName) > 0);

	// NEEDSWORK: this is horrible
	size_t numItems = 2;
	char** dataKeys = (char**)calloc(sizeof(char*), numItems);
	char** dataValues = (char**)calloc(sizeof(char*), numItems);

	dataKeys[0] = _strdup("target");
	dataValues[0] = _strdup("nxs");
	if (key != NULL && value != NULL)
	{
		dataKeys[1] = _strdup(key);
		dataValues[1] = _strdup(value);
	}
	else
	{
		numItems = 1;
	}

	if (GetWinProperties()->GetAnalytics() != NULL)
	{
		GetWinProperties()->GetAnalytics()->Log(eventName, numItems, dataKeys, dataValues);
	}

	free(dataKeys[0]);
	free(dataValues[0]);
	if (numItems > 1)
	{
		free(dataKeys[1]);
		free(dataValues[1]);
	}
	free(dataKeys);
	free(dataValues);
}
