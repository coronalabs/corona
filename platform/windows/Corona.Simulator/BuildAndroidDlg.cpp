//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Simulator.h"
#include "SimulatorView.h"
#include "Resource.h"
#include "CoronaProject.h"
#include "PasswordDlg.h"
#include "BuildAndroidDlg.h"
#include "BuildProgressDlg.h"
#include "BuildResult.h"
#include "WinGlobalProperties.h"
#include "WinString.h"
#include "DirDialog.h"
#include "HtmlMessageDlg.h"
#include "MessageDlg.h"
#include "ListKeyStore.h"
#include "CoronaInterface.h"
#include "Core/Rtt_Build.h"
#include "Rtt_AndroidSupportTools.h"
#include "Rtt_Authorization.h"
#include "Rtt_AuthorizationTicket.h"
#include "Rtt_SimulatorAnalytics.h"
#include "Rtt_TargetDevice.h"
#include "Rtt_TargetAndroidAppStore.h"
#include "Rtt_WebServicesSession.h"
#include <Shlwapi.h>


// CBuildAndroidDlg dialog

CString CBuildAndroidDlg::m_sHelpURL;

IMPLEMENT_DYNAMIC(CBuildAndroidDlg, CDialog)

CBuildAndroidDlg::CBuildAndroidDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBuildAndroidDlg::IDD, pParent),
    m_pProject( NULL ),
	m_sPrevKeystorePath( "" ),
	m_sKeystoreDir( "" ),
	m_nValidFields( 0 ),
	m_androidValidatorLuaState( NULL )
{
}

CBuildAndroidDlg::~CBuildAndroidDlg()
{
	if (m_androidValidatorLuaState)
	{
		Rtt_AndroidSupportTools_CloseLuaState(m_androidValidatorLuaState);
	}
}

void CBuildAndroidDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CBuildAndroidDlg, CDialog)
	ON_BN_CLICKED(IDC_BUILD_BROWSE_KEYSTORE, &CBuildAndroidDlg::OnBrowseKeystore)
	ON_BN_CLICKED(IDC_BUILD_BROWSE_SAVETO, &CBuildAndroidDlg::OnBrowseSaveto)
	ON_EN_KILLFOCUS(IDC_BUILD_KEYSTORE, &CBuildAndroidDlg::OnKillFocusKeystorePath)
	ON_CBN_SELCHANGE(IDC_BUILD_KEYALIAS, &CBuildAndroidDlg::OnChangeAliasList)
	ON_CBN_SETFOCUS(IDC_BUILD_KEYALIAS, &CBuildAndroidDlg::OnSetFocusAliasList)
#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
	ON_BN_CLICKED(IDC_ENABLE_MONETIZATION, &CBuildAndroidDlg::OnBnClickedEnableMonetization)
#endif
	ON_WM_HELPINFO()
	ON_WM_SYSCOMMAND()
	ON_BN_CLICKED(IDC_CREATE_LIVE_BUILD, &CBuildAndroidDlg::OnBnClickedCreateLiveBuild)
END_MESSAGE_MAP()

// OnInitDialog - restore settings from m_pProject if available.
// Choose likely defaults for non-saved settings.
// Check for trial users, who can only use installed debug.keystore
BOOL CBuildAndroidDlg::OnInitDialog()
{
	WinString stringConverter;
	CString stringBuffer;
	Rtt::Authorization *pAuth = GetWinProperties()->GetAuth();
	const Rtt::AuthorizationTicket *pTicket = (pAuth != NULL) ? pAuth->GetTicket() : NULL;

	// Initialize base class first.
	CDialog::OnInitDialog();

	// Do not display this window if not given a project settings object.
	if (!m_pProject)
	{
		CDialog::OnCancel();
		return TRUE;
	}

	//Set inital project type now that we have Android/Web (used by the BuildProgressDlg - does not indicate target app store)
	m_pProject->SetTargetPlatform(Rtt::TargetDevice::kAndroidPlatform);

	// Initialize the Android validator.
	WinString androidValidatorFilePath;
	androidValidatorFilePath.SetUTF8(GetWinProperties()->GetResourcesDir());
	if ((androidValidatorFilePath.GetLength() > 0) && !androidValidatorFilePath.EndsWith("\\"))
	{
		androidValidatorFilePath.Append("\\");
	}
	androidValidatorFilePath.Append("AndroidValidation.lua");
	m_androidValidatorLuaState = Rtt_AndroidSupportTools_NewLuaState(androidValidatorFilePath.GetUTF8());

	// Limit the text length of CEdit boxes.
	((CEdit*)GetDlgItem(IDC_BUILD_APPNAME))->SetLimitText(128);
	((CEdit*)GetDlgItem(IDC_BUILD_VERSION_CODE))->SetLimitText(9);
	((CEdit*)GetDlgItem(IDC_BUILD_VERSION_NAME))->SetLimitText(16);
	((CEdit*)GetDlgItem(IDC_BUILD_PACKAGE))->SetLimitText(128);
	((CEdit*)GetDlgItem(IDC_BUILD_KEYSTORE))->SetLimitText(MAX_PATH - 1);
	((CEdit*)GetDlgItem(IDC_BUILD_SAVETO))->SetLimitText(MAX_PATH - 1);

	// Fetch the last selected app store.
	stringConverter.SetUTF8(Rtt::TargetAndroidAppStore::kGoogle.GetStringId());
    stringBuffer = AfxGetApp()->GetProfileString(
				REGISTRY_BUILD_ANDROID, REGISTRY_TARGET_APP_STORE, stringConverter.GetTCHAR());
	stringConverter.SetTCHAR(stringBuffer);
	Rtt::TargetAndroidAppStore *pLastSelectedStore =
				Rtt::TargetAndroidAppStore::GetByStringId(stringConverter.GetUTF8());

	// Set up the app store combo box.
    CComboBox *pStoreComboBox = (CComboBox*)GetDlgItem(IDC_BUILD_TARGET_APP_STORE);
	int iStoreCount = Rtt::TargetAndroidAppStore::GetCount();
	for (int iStoreIndex = 0; iStoreIndex < iStoreCount; iStoreIndex++)
	{
		Rtt::TargetAndroidAppStore *pNextStore = Rtt::TargetAndroidAppStore::GetByIndex(iStoreIndex);
		if (pNextStore != NULL)
		{
			stringConverter.SetUTF8(pNextStore->GetName());
			int iComboBoxIndex = pStoreComboBox->AddString(stringConverter.GetTCHAR());
			if (iComboBoxIndex >= 0)
			{
				pStoreComboBox->SetItemDataPtr(iComboBoxIndex, (void*)pNextStore);
				if (pNextStore == pLastSelectedStore)
				{
					pStoreComboBox->SetCurSel(iComboBoxIndex);
				}
				if ((pStoreComboBox->GetCurSel() < 0) &&
				    (pNextStore == &Rtt::TargetAndroidAppStore::kGoogle))
				{
					pStoreComboBox->SetCurSel(iComboBoxIndex);
				}
			}
		}
	}

    m_sKeystoreDir = AfxGetApp()->GetProfileString( REGISTRY_BUILD_ANDROID, REGISTRY_KEYSTOREDIR, 
		((CSimulatorApp *)AfxGetApp())->GetResourceDir() );  // default to resource dir (where debug.keystore is)
    CString sLastKeystore = AfxGetApp()->GetProfileString( REGISTRY_BUILD_ANDROID, REGISTRY_LASTKEYSTORE, _T("") );

    m_sPrevKeystorePath = _T("");
    m_nValidFields = 0;

	// If there isn't a package name, create one by reversing the user's email address and adding the app name
	if (m_pProject->GetPackage().IsEmpty() && pTicket)
	{
		CString emailAddr(pTicket->GetUsername());
		CString delim = _T(".@+:?/=");

		int i = 0;
		CStringArray saItems;
		for(CString sItem = emailAddr.Tokenize(delim,i); i >= 0; sItem = emailAddr.Tokenize(delim,i))
		{
			saItems.Add( sItem );
		}

		CString package;
		for (int i = saItems.GetSize() - 1; i >= 0; --i )
		{
			package.Append(saItems.GetAt(i));
			package.Append(_T("."));
		}
		package.Append(m_pProject->GetName());

		for (int c = 0; c < package.GetLength(); c++)
		{
			if (package[c] > 255 || (! isalnum(package[c]) && package[c] != '.' && package[c] != '_'))
			{
				package.SetAt(c, _T('_'));
			}
		}

		package.Trim(_T("._"));

		m_pProject->SetPackage(package);
	}

	stringBuffer.Format(_T("%d"), m_pProject->GetAndroidVersionCode());
	SetDlgItemText( IDC_BUILD_APPNAME, m_pProject->GetName() );
	SetDlgItemText( IDC_BUILD_VERSION_CODE, stringBuffer );
	SetDlgItemText( IDC_BUILD_VERSION_NAME, m_pProject->GetAndroidVersionName() );
	SetDlgItemText( IDC_BUILD_PACKAGE, m_pProject->GetPackage() );
	SetDlgItemText( IDC_BUILD_PROJECTPATH, m_pProject->GetDir() );

	CSimulatorApp *pApp = (CSimulatorApp *)AfxGetApp();

	// Initialize keystore path from, in order available
	// 1. saved keystore path for this project
	// 2. last used keystore path (also get last-used password)
	//    (if neither are available, there is also a saved keystore directory for browsing)
	// or, for trial users, the app's installed debug.keystore
	if ((pTicket != NULL) && pTicket->IsAllowedToBuild(Rtt::TargetDevice::kAndroidPlatform))
	{
		if( m_pProject->GetKeystorePath().IsEmpty() ) // none saved for project, get last-used path
		{
			if (sLastKeystore.IsEmpty())
			{
				SetDlgItemText( IDC_BUILD_KEYSTORE, CCoronaProject::GetTrialKeystorePath() );
				m_pProject->SetKeystorePassword( GetTrialKeystorePassword() );
			}
			else
			{
				if ( ! pApp->CheckPathExists( sLastKeystore ) )
				{
					DisplayWarningMessage(IDS_ANDROID_KEYSTORE_FILE_NOT_FOUND, sLastKeystore);
				}
				else
				{
					SetDlgItemText( IDC_BUILD_KEYSTORE, sLastKeystore );
					// Third argument tells what this password is for
					m_pProject->RegistryGetKeystorePassword( REGISTRY_BUILD_ANDROID, REGISTRY_LASTKEYSTOREPWD, sLastKeystore );
				}
			}
		}
		else // use the one saved with this project
		{
			if ( ! pApp->CheckPathExists( m_pProject->GetKeystorePath() ) )
			{
				DisplayWarningMessage(IDS_ANDROID_KEYSTORE_FILE_NOT_FOUND, m_pProject->GetKeystorePath());
			}
			else
			{
				SetDlgItemText( IDC_BUILD_KEYSTORE, m_pProject->GetKeystorePath() );
			}
		}
	}
	else // trial build only allows debug keystore
	{
		SetDlgItemText( IDC_BUILD_KEYSTORE, CCoronaProject::GetTrialKeystorePath() );
		m_pProject->SetKeystorePassword( GetTrialKeystorePassword() );
		GetDlgItem( IDC_BUILD_KEYSTORE )->EnableWindow( FALSE );  // no editing for trial users
	}

	// After all that, get the values for keystore and password and try to initialize alias list
	CString sKeystore, sKeystorePassword;
	GetDlgItemText( IDC_BUILD_KEYSTORE, sKeystore );

	// Non-trial user with our installed keystore
	if( sKeystore.CompareNoCase( CCoronaProject::GetTrialKeystorePath() ) == 0 )
	{
		m_pProject->SetKeystorePassword( GetTrialKeystorePassword() );
	}
	// For all cases, set variable to pass in to ReadKeystore
	sKeystorePassword = m_pProject->GetKeystorePassword( sKeystore ); // argument has to match what pwd is for

	ReadKeystore( sKeystore, sKeystorePassword, false );  // bShowErrors=false, dialog not visible yet

	// Choose the saved key alias in the key alias dropdown
	CComboBox *pAlias = (CComboBox *)GetDlgItem( IDC_BUILD_KEYALIAS );
    if( ! m_pProject->GetAlias().IsEmpty() )
	{
		pAlias->SelectString( -1, m_pProject->GetAlias() );
		OnChangeAliasList();
	}

	// Initialize build destination directory.
	SetDlgItemText(IDC_BUILD_SAVETO, m_pProject->GetSaveDir());

	// Set up the "Live Build" checkbox.
	CheckDlgButton(IDC_CREATE_LIVE_BUILD, m_pProject->GetCreateLiveBuild() ? BST_CHECKED : BST_UNCHECKED);

#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
	// Initialize the "Enable Monetization" checkbox
	CButton *enableMonetizationBtn = (CButton *)GetDlgItem(IDC_ENABLE_MONETIZATION);
	bool debugMonetizationPlugin = AfxGetApp()->GetProfileInt(REGISTRY_SECTION, REGISTRY_DEBUG_MONETIZATION_PLUGIN, 
		REGISTRY_DEBUG_MONETIZATION_PLUGIN_DEFAULT) ? true : false;

	if (debugMonetizationPlugin)
	{
		// Load setting from project
		enableMonetizationBtn->SetCheck((m_pProject->GetEnableMonetization() ? BST_CHECKED : BST_UNCHECKED));
	}
	else
	{
		// Hide and disable "Enable Monetization" checkbox
		enableMonetizationBtn->ShowWindow(SW_HIDE);
		enableMonetizationBtn->SetCheck(BST_UNCHECKED);
		m_pProject->SetEnableMonetization(false);
	}

#else
	// Disable dialog control
	CButton *enableMonetizationBtn = (CButton *)GetDlgItem(IDC_ENABLE_MONETIZATION);
	enableMonetizationBtn->ShowWindow(SW_HIDE);
	enableMonetizationBtn->SetCheck(BST_UNCHECKED);

#endif // AUTO_INCLUDE_MONETIZATION_PLUGIN

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// SetProject - called before DoModal to set the associated project.
void CBuildAndroidDlg::SetProject( const std::shared_ptr<CCoronaProject>& pProject )
{
    m_pProject = pProject;  // leave unchanged until OnOK()
}

// BuildAndroidDlg message handlers

// OnBrowseKeystore - check authorization, then allow user to select keystore.
// Then call UpdateAliasList which gets password, reads keystore, and fills 
// Key Alias dropdown.
void CBuildAndroidDlg::OnBrowseKeystore()
{
	// Authorization check - trial & ios-only can't change the keystore
	if( appAllowFullBuild(Rtt::TargetDevice::kAndroidPlatform) )
	{
		GetDlgItem( IDC_BUILD_KEYSTORE )->EnableWindow( TRUE );  // may have just purchased

		// Show Open dialog so user can browse to a new keystore
		CString sPath = _T("");
		GetDlgItemText( IDC_BUILD_KEYSTORE, sPath );

		CString sFilter  = _T("Keystore files (*.keystore)|*.keystore|All Files (*.*)|*.*||");
		CFileDialog fileDlg( TRUE, NULL, sPath, OFN_FILEMUSTEXIST, sFilter);

		fileDlg.m_ofn.lStructSize = sizeof( fileDlg.m_ofn );
		fileDlg.m_ofn.lpstrTitle = _T("Select Keystore");
		fileDlg.m_ofn.lpstrInitialDir = m_sKeystoreDir;

		if( fileDlg.DoModal() == IDOK )
		{
			// Display selected filename
			sPath = fileDlg.GetPathName();
			SetDlgItemText( IDC_BUILD_KEYSTORE, sPath );

			// Update Alias dropdown
			UpdateAliasList();
		}
	}
}

// OnKillFocusKeystorePath - ask for password when user leaves Keystore edit field,
// unless focus leaves window or is going to Cancel
void CBuildAndroidDlg::OnKillFocusKeystorePath()
{
    // Unless focus is still within dlg and on Cancel, update aliases
    // TODO: What if someone alt-tabs away from the dialog and then
    // clicks somewhere else?
	CWnd *pFocus =  CWnd::GetFocus(); 
	if( pFocus
        && (pFocus->GetParent() ==  this)
        && (pFocus->GetDlgCtrlID() == IDCANCEL) )
	{
        return;
	}

    UpdateAliasList();
}

// OnSetFocusAliasLIst - when focus goes to alias list dropdown, check if it's 
// empty and ask for keystore password.
void CBuildAndroidDlg::OnSetFocusAliasList()
{
	CComboBox *pAlias = (CComboBox *)GetDlgItem( IDC_BUILD_KEYALIAS );
	
	if( pAlias->GetCount() == 0 )
	{
		UpdateAliasList();
		pAlias->ShowDropDown( FALSE );  // Attempt to close dropdown, but doesn't seem to work
	}
}

BOOL CBuildAndroidDlg::OnHelpInfo(HELPINFO* helpInfoPointer)
{
	// Display help information related to this dialog.
	// Note: If we don't handle this here, then SimulatorView::OnHelp() will get called which displays
	//       the main Corona documentation page on Corona Labs' website.
	OnSysCommand(SC_CONTEXTHELP, 0);
	return TRUE;
}

// UpdateAliasList - Do the complicated work of validating the keystore password 
// and reading the keystore to fill the alias list drop-down
void CBuildAndroidDlg::UpdateAliasList()
{
	CString sKeystorePath;
	GetDlgItemText( IDC_BUILD_KEYSTORE, sKeystorePath );

	CSimulatorApp *pApp = (CSimulatorApp *)AfxGetApp();

	if( sKeystorePath != m_sPrevKeystorePath )  // skip if we already did all this
	{
        m_sPrevKeystorePath = sKeystorePath;  // remember previous path

		// Clear alias dropdown
		CComboBox *pAliasList = (CComboBox *)GetDlgItem( IDC_BUILD_KEYALIAS );
		pAliasList->ResetContent();
		OnChangeAliasList();

		// If keystore path is empty, return now
		if( sKeystorePath.IsEmpty() )
		{
			return;
		}

		// Check for existence of keystore file before trying to parse it
		if( ! pApp->CheckPathExists( sKeystorePath ) )
		{
			DisplayWarningMessage(IDS_ANDROID_KEYSTORE_FILE_NOT_FOUND, sKeystorePath);
			return;
		}

        // Get keystore password        
		CString sPassword = _T("");
		m_pProject->ClearKeystorePassword();  // new keystore, new password needed

		// Known password for the keystore we install, don't prompt user
		if( ! sKeystorePath.CompareNoCase( CCoronaProject::GetTrialKeystorePath() ) )
		{
			sPassword = GetTrialKeystorePassword();
		}

		// Loop until we get valid password, or until user clicks Cancel
        bool bDone = false;
        while (!bDone)
		{
            if( sPassword.IsEmpty() )  // show password dialog to get password
			{
				CPasswordDlg passwordDlg( this );
				passwordDlg.SetKey( _T("Keystore: ") + sKeystorePath );
				if( passwordDlg.DoModal() == IDOK )
				{
					sPassword = passwordDlg.GetPassword();

                    // Need to save the password at least for the current build
					m_pProject->SetKeystorePassword( sPassword );
                    // Remember whether to save it to registry
                    m_pProject->SetSaveKeystorePassword( passwordDlg.GetSavePassword() );

					// Save directory for future use in keystore open dialog
					// Do here because all changes to keystore path end up here
					m_sKeystoreDir = sKeystorePath.Left( sKeystorePath.ReverseFind(_T('\\')));
				}
				else // password dialog was canceled
				{
					bDone = true;
					m_sPrevKeystorePath	 = "";  // clear this so user can try again later
				}
			}
             
            if( ! bDone )  // user clicked OK
			{
				// Have password, try opening the keystore, post errors to user
				bDone = ReadKeystore( sKeystorePath, sPassword, true );
				if( !bDone )
				{
                    sPassword = "";  // clear to try again
				}
			}
		}
	}
}

// ReadKeystore - Use Java to read key alias list from keystore
// Posts errors to user if bShowErrors is true (default except from OnInitDialog)
// Returns true if password worked, false otherwise
bool CBuildAndroidDlg::ReadKeystore( CString sKeystorePath, CString sPassword, bool bShowErrors )
{
	bool bPasswordSucceeded = false;

	// Scroll the keystore edit control so the end with the filename of the keystore is visible
	CWnd *buildKeystoreField = GetDlgItem(IDC_BUILD_KEYSTORE);
	if (buildKeystoreField != NULL)
	{
		DWORD len = buildKeystoreField->SendMessage( WM_GETTEXTLENGTH, 0, 0 );   
		buildKeystoreField->SendMessage( EM_SETSEL, len, len);   
		buildKeystoreField->SendMessage( EM_SCROLLCARET, 0, 0 ); 
	}

	CComboBox *pAliasList = (CComboBox *)GetDlgItem( IDC_BUILD_KEYALIAS );

	// Convert string arguments to UTF8
	WinString strKeystorePath, strPassword;
	strKeystorePath.SetTCHAR( sKeystorePath );
	strPassword.SetTCHAR( sPassword );

	ListKeyStore listKeyStore;  // Uses Java to read keystore

	if( listKeyStore.GetAliasList( strKeystorePath.GetUTF8(), strPassword.GetUTF8() ))
	{
		bPasswordSucceeded = true;

		if( listKeyStore.GetSize() < 1 )
		{
			CString msg, title;
			msg.LoadString( IDS_KEYSTORE_ATLEASTONE );
			title.LoadString( IDS_ANDROIDBUILD );
			if( bShowErrors )
			{
				MessageBox( msg, title, MB_ICONEXCLAMATION );
			}
		}
		else
		{
			// Succeeded and there is at least one alias - add aliases to alias dropdown
			WinString strTitle;
			for ( int i = 0; i < listKeyStore.GetSize(); i++ )
			{	
				strTitle.SetUTF8( listKeyStore.GetAlias(i) );
				int itemIndex = pAliasList->AddString( strTitle.GetTCHAR() );
			}

			// Insert "(Choose from the following)"
			CString sChoose;
			sChoose.LoadString( IDS_CHOOSEFOLLOWING );
			pAliasList->InsertString( 0, sChoose );

			// if only one valid key, select that, otherwise select "(choose from the following)"
			pAliasList->SetCurSel((pAliasList->GetCount() == 2) ? 1 : 0);

			OnChangeAliasList();
		}
	}
	else // didn't get valid password, or keystore bad format
	{
		CString msg, title;
		msg.Format( IDS_INVALID_KEYSTORE_s, sKeystorePath );
		title.LoadString( IDS_ANDROIDBUILD );
		if( bShowErrors )
		{
			MessageBox( msg, title, MB_ICONEXCLAMATION );
		}
	}

	return bPasswordSucceeded;
}

// OnBrowseSaveto - show directory selection dialog for build destination dir
void CBuildAndroidDlg::OnBrowseSaveto()
{
   CString sDir = _T("");
   GetDlgItemText( IDC_BUILD_SAVETO, sDir );
   
   // Default to the system's Documents if the above field is empty.
   if (sDir.IsEmpty())
   {
	   ::SHGetFolderPath(nullptr, CSIDL_MYDOCUMENTS, nullptr, 0, sDir.GetBuffer(MAX_PATH));
	   sDir.ReleaseBuffer();
   }

   CDirDialog dirDialog;
   dirDialog.m_strSelDir = sDir;
   dirDialog.m_strTitle.LoadString(IDS_SELECT_BUILD_OUTPUT_FOLDER_DESCRIPTION);
   
   if( dirDialog.DoBrowse() == IDOK )
   {
      sDir = dirDialog.m_strPath;
      SetDlgItemText( IDC_BUILD_SAVETO, sDir );
   }
}

// OnChangeAliasList - validate Key alias - must have values in
// dropdown and have one selected.  
// Also clear saved password if alias has changed.
// TODO: we always test with debug.keystore which only has one key alias,
// so this code needs more testing
void CBuildAndroidDlg::OnChangeAliasList()
{
    CComboBox *pAliasList = (CComboBox *)GetDlgItem( IDC_BUILD_KEYALIAS );

    CString sValue;
    GetDlgItemText( IDC_BUILD_KEYALIAS, sValue );

    if( sValue != m_sPrevKeystoreAlias )
	{
        m_pProject->ClearAliasPassword();
		m_sPrevKeystoreAlias = sValue;
	}
}

void CBuildAndroidDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// Handle the received dialog system command.
	if (SC_CONTEXTHELP == nID)
	{
		// The help [?] button or F1 key was pressed.
		// Display the Android build documentation on Corona Labs' website.
		try
		{
			const wchar_t kUrl[] = L"https://docs.coronalabs.com/daily/guide/distribution/androidBuild";
			::ShellExecuteW(nullptr, L"open", kUrl, nullptr, nullptr, SW_SHOWNORMAL);
		}
		catch (...) {}
	}
	else
	{
		// Let the dialog perform its default handling on the received system command.
		CDialog::OnSysCommand(nID, lParam);
	}
}

// OnOK - build project
// Retrieve values from dialog, save to registry, 
// get key alias password if necessary
// test key alias password
// call core code to package project and send to server for building
void CBuildAndroidDlg::OnOK()  // OnBuild()
{
	Rtt::TargetAndroidAppStore *pTargetStore = NULL;
	CComboBox *pComboBox;
	WinString stringBuffer;
    CString sValue;
	CString sKeystore;
	CString sBuildDir;
	CString sVersionName;
	CString sPackageName;
	CString sAppName;
	CString sKeyAlias;
	int iVersionCode;
	int iIndex;
	bool isLiveBuild;

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
    GetDlgItemText(IDC_BUILD_VERSION_CODE, sValue);
	sValue.Trim();
	if (sValue.IsEmpty())
	{
		DisplayWarningMessage(IDS_BUILD_VERSION_NUMBER_NOT_PROVIDED);
		GetDlgItem(IDC_BUILD_VERSION_CODE)->SetFocus();
		LogAnalytics("build-bungled", "reason", "BUILD_APP_VERSION_NUMBER_NOT_PROVIDED");
		return;
	}
	iVersionCode = _ttoi(sValue);
	if (iVersionCode < 1)
	{
		DisplayWarningMessage(IDS_INVALID_ANDROID_APP_VERSION_NUMBER);
		GetDlgItem(IDC_BUILD_VERSION_CODE)->SetFocus();
		LogAnalytics("build-bungled", "reason", "INVALID_ANDROID_APP_VERSION_NUMBER");
		return;
	}
	GetDlgItemText(IDC_BUILD_VERSION_NAME, sVersionName);
	sVersionName.Trim();
	if (sVersionName.IsEmpty())
	{
		DisplayWarningMessage(IDS_BUILD_VERSION_NAME_NOT_PROVIDED);
		GetDlgItem(IDC_BUILD_VERSION_NAME)->SetFocus();
		LogAnalytics("build-bungled", "reason", "BUILD_VERSION_NAME_NOT_PROVIDED");
		return;
	}
	GetDlgItemText(IDC_BUILD_PACKAGE, sPackageName);
	sPackageName.Trim();
	if (sPackageName.IsEmpty())
	{
		DisplayWarningMessage(IDS_BUILD_PACKAGE_NAME_NOT_PROVIDED);
		GetDlgItem(IDC_BUILD_PACKAGE)->SetFocus();
		LogAnalytics("build-bungled", "reason", "BUILD_PACKAGE_NAME_NOT_PROVIDED");
		return;
	}
	stringBuffer.SetTCHAR(sPackageName);
	if (!Rtt_AndroidSupportTools_IsAndroidPackageName(m_androidValidatorLuaState, stringBuffer.GetUTF8()))
	{
		DisplayWarningMessage(IDS_VALID_PACKAGE);
		GetDlgItem(IDC_BUILD_PACKAGE)->SetFocus();
		LogAnalytics("build-bungled", "reason", "bad-package-identifier");
		return;
	}
	pComboBox = (CComboBox*)GetDlgItem(IDC_BUILD_TARGET_APP_STORE);
	iIndex = pComboBox->GetCurSel();
	if (iIndex >= 0)
	{
		pTargetStore = (Rtt::TargetAndroidAppStore*)pComboBox->GetItemDataPtr(iIndex);
	}
	if (!pTargetStore)
	{
		DisplayWarningMessage(IDS_BUILD_TARGET_APP_STORE_NOT_SELECTED);
		pComboBox->SetFocus();
		LogAnalytics("build-bungled", "reason", "BUILD_TARGET_APP_STORE_NOT_SELECTED");
		return;
	}
    GetDlgItemText(IDC_BUILD_KEYSTORE, sKeystore);
	sKeystore.Trim();
	if (sKeystore.IsEmpty())
	{
		DisplayWarningMessage(IDS_ANDROID_KEYSTORE_FILE_NOT_PROVIDED);
		GetDlgItem(IDC_BUILD_KEYSTORE)->SetFocus();
		LogAnalytics("build-bungled", "reason", "ANDROID_KEYSTORE_FILE_NOT_PROVIDED");
		return;
	}
	if (::PathFileExists(sKeystore) == FALSE)
	{
		DisplayWarningMessage(IDS_ANDROID_KEYSTORE_FILE_NOT_FOUND, sKeystore);
		GetDlgItem(IDC_BUILD_KEYSTORE)->SetFocus();
		LogAnalytics("build-bungled", "reason", "ANDROID_KEYSTORE_FILE_NOT_FOUND");
		return;
	}
    GetDlgItemText(IDC_BUILD_KEYALIAS, sKeyAlias);
	pComboBox = (CComboBox*)GetDlgItem(IDC_BUILD_KEYALIAS);
	if ((pComboBox->GetCurSel() <= 0) || sKeyAlias.IsEmpty())
	{
		DisplayWarningMessage(IDS_ANDROID_KEYALIAS_NOT_SELECTED);
		GetDlgItem(IDC_BUILD_KEYALIAS)->SetFocus();
		LogAnalytics("build-bungled", "reason", "ANDROID_KEYALIAS_NOT_SELECTED");
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
	if (sBuildDir.Compare(m_pProject->GetDir()) == 0)
	{
		DisplayWarningMessage(IDS_BUILD_PATH_INVALID);
		GetDlgItem(IDC_BUILD_SAVETO)->SetFocus();
		LogAnalytics("build-bungled", "reason", "BUILD_PATH_INVALID" );
		return;
	}
	if (::PathIsDirectory(sBuildDir) == FALSE)
	{
		// The given path does not exist. Attempt to create it. If we are unable to create the directory,
		// then that may mean the path is invalid or we do not have permission to create it at that location.
		int result = ::SHCreateDirectoryEx(NULL, sBuildDir, NULL);
		if (result != ERROR_SUCCESS)
		{
			DisplayWarningMessage(IDS_BUILD_PATH_CREATION_FAILED, sBuildDir);
			GetDlgItem(IDC_BUILD_SAVETO)->SetFocus();
			LogAnalytics("build-bungled", "reason", "BUILD_PATH_CREATION_FAILED");
			return;
		}
	}
	isLiveBuild = (IsDlgButtonChecked(IDC_CREATE_LIVE_BUILD) == BST_CHECKED);
	
	// Display a nag window if the user is not authorized to build for the selected app store.
	// In this case, the build system will create a trial version of the app instead.
	appAllowFullBuild(pTargetStore->GetPlatform());

	// Store field settings to project.
	m_pProject->SetName(sAppName);
	m_pProject->SetAndroidVersionCode(iVersionCode);
	m_pProject->SetAndroidVersionName(sVersionName);
    m_pProject->SetPackage(sPackageName);
	m_pProject->SetKeystorePath(sKeystore);
	m_pProject->SetAlias(sKeyAlias);
	m_pProject->SetSaveDir(sBuildDir);
	m_pProject->SetTargetOS(_T("Android 2.3.3"));		// <- This string is only used by logging/analytics.
	m_pProject->SetCreateLiveBuild(isLiveBuild);

	// Update global build settings in registry.
	stringBuffer.SetUTF8(pTargetStore->GetStringId());
    AfxGetApp()->WriteProfileString( REGISTRY_BUILD_ANDROID, REGISTRY_TARGET_APP_STORE, stringBuffer.GetTCHAR() );
    AfxGetApp()->WriteProfileString( REGISTRY_BUILD_ANDROID, REGISTRY_KEYSTOREDIR, m_sKeystoreDir );
    AfxGetApp()->WriteProfileString( REGISTRY_BUILD_ANDROID, REGISTRY_LASTKEYSTORE, sKeystore );

	// Third argument is what this is a password for, not value of password
	m_pProject->RegistryPutKeystorePassword(REGISTRY_BUILD_ANDROID, REGISTRY_LASTKEYSTOREPWD, sKeystore);

	// We know the Key Alias password for the trial keystore
	if (!sKeystore.CompareNoCase(CCoronaProject::GetTrialKeystorePath()))
	{
		m_pProject->SetKeystorePassword(GetTrialKeystorePassword());
		m_pProject->SetAliasPassword(GetTrialKeyAliasPassword());
	}

	// Loop until a valid password has been received or until the user cancels out.
	// TODO: Password validation should be done in the password dialog. That would eliminate the need for this loop.
    CString sAliasPassword = m_pProject->GetAliasPassword();
    bool bDone = false;
    while (!bDone)
	{
		// Ask for the user's password if the user has not provided one yet.
        if (sAliasPassword.IsEmpty())
		{
			CPasswordDlg passwordDlg( this );
			passwordDlg.SetKey( _T("Key alias: ") + sKeyAlias );
			if (passwordDlg.DoModal() != IDOK)
			{
				LogAnalytics("build-bungled", "reason", "cancel-android-key-password");
				return;
			}
			sAliasPassword = passwordDlg.GetPassword();
			m_pProject->SetAliasPassword( sAliasPassword );
			m_pProject->SetSaveAliasPassword( passwordDlg.GetSavePassword() );
		}

		// Validate user's password.
		if (!bDone)
		{
			if (m_pProject->ValidateKeystoreAliasPassword())
			{
				// Password is valid.
				bDone = true;
			}
			else
			{
				// Password is invalid.
				DisplayWarningMessage(IDS_INVALID_ALIAS, m_pProject->GetAlias());
				sAliasPassword.Empty();
			}
		}
	}

	LogAnalytics("build", "store", pTargetStore->GetStringId());

	m_pProject->Save();
	CBuildProgressDlg buildDialog;
	buildDialog.SetProjectSettings(m_pProject);
	buildDialog.SetTargetedAppStore(pTargetStore);
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
    messageDlg.SetText( IDS_BUILD_SUCCEEDED );
    messageDlg.SetDefaultText( IDS_DONE );
    messageDlg.SetAltText( IDS_VIEW_EXPLORER );
    messageDlg.SetFolder( m_pProject->GetSaveDir() );
    messageDlg.SetIconStyle( MB_ICONINFORMATION );
    messageDlg.DoModal();

	// Add the project to the Corona Live Server list, if enabled.
	if (isLiveBuild)
	{
		CString sCoronaLiveServerExe = ((CSimulatorApp *)AfxGetApp())->GetApplicationDir() + _T("\\Corona.LiveServer.exe");
		CString sAppPath = m_pProject->GetPath();
		try
		{
			::ShellExecute(nullptr, nullptr, sCoronaLiveServerExe, sAppPath, nullptr, SW_SHOWNORMAL);
		}
		catch (...) {}
	}

	// Close this window.
	CDialog::OnOK();
}

// OnCancel - clean up, exit
void CBuildAndroidDlg::OnCancel()
{
	// remove temp passwords
	if (m_pProject)
	{
		m_pProject->ClearKeystorePassword();  
		m_pProject->ClearAliasPassword();
	}

	CDialog::OnCancel();
}

// GetTrialKeystorePassword
CString CBuildAndroidDlg::GetTrialKeystorePassword()
{
	return _T("android");
}

// GetTrialKeyAliasPassword - sole key alias is androiddebugkey
// I noticed that signing seems to succeed with any password for the trial key alias
CString CBuildAndroidDlg::GetTrialKeyAliasPassword()
{
	return _T("android");
}

// Display a warning message box with the given message.
// Argument "nMessageID" is a unique ID to a string in the resource file's string table.
void CBuildAndroidDlg::DisplayWarningMessage(UINT nMessageID)
{
	CString title;
	CString message;

	title.LoadString(IDS_WARNING);
	message.LoadString(nMessageID);
	MessageBox(message, title, MB_OK | MB_ICONWARNING);
}

// Display a warning message box with the given message and string.
// Argument "nMessageID" is a unique ID to a string in the resource file's string table.
void CBuildAndroidDlg::DisplayWarningMessage(UINT nMessageID, CString extraInfo)
{
	CString title;
	CString message;

	title.LoadString(IDS_WARNING);
	message.LoadString(nMessageID);
	message.Append(_T("\r\n\r\n"));
	message.Append(extraInfo);
	MessageBox(message, title, MB_OK | MB_ICONWARNING);
}

void CALLBACK CBuildAndroidDlg::HelpCallback(LPHELPINFO lpHelpInfo)
{
	// AfxGetApp()->WinHelp(lpHelpInfo->dwContextId);

	try
	{
		::ShellExecuteW(nullptr, L"open", m_sHelpURL, nullptr, nullptr, SW_SHOWNORMAL);
	}
	catch (...) {}
}

UINT CBuildAndroidDlg::DisplayWarningMessageWithHelp(UINT nTitleID, UINT nMessageID, CString helpURL)
{
	MSGBOXPARAMS mbp;
	CString title;
	CString message;

	title.LoadString(nTitleID);
	message.LoadString(nMessageID);

	memset(&mbp, 0, sizeof mbp);

	m_sHelpURL = helpURL;

	mbp.cbSize = sizeof MSGBOXPARAMS;
	mbp.hwndOwner = m_hWnd;
	mbp.hInstance = AfxGetInstanceHandle();
	mbp.lpszText = message;

	// if you wanted to specify a different caption, here is where you do it
	mbp.lpszCaption = title;

	mbp.dwStyle = MB_YESNO | MB_ICONWARNING | MB_HELP;

	//  mbp.lpszIcon = ; // note, you could provide your own custom ICON here!

	mbp.dwContextHelpId = 1; // not used
	mbp.lpfnMsgBoxCallback = (MSGBOXCALLBACK) &HelpCallback;
	mbp.dwLanguageId = 0x0409;

	return ::MessageBoxIndirect(&mbp);
}

#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
// Called when the user changes the "Enable Monetization" checkbox
void CBuildAndroidDlg::OnBnClickedEnableMonetization()
{
	CButton *enableMonetizationBtn = (CButton *) GetDlgItem(IDC_ENABLE_MONETIZATION);

	// If they uncheck the box, ask them if they are sure
	if (enableMonetizationBtn->GetCheck() == BST_UNCHECKED)
	{
		CString title;
		CString message;
		DWORD answer = IDNO;

		title.LoadString(IDS_ENABLE_MONETIZATION);
		message.LoadString(IDS_MONETIZATION_WARNING);
		//answer = MessageBox(message, title, MB_YESNO | MB_ICONWARNING | MB_HELP);
		answer = DisplayWarningMessageWithHelp(IDS_ENABLE_MONETIZATION, IDS_MONETIZATION_WARNING, L"https://fusepowered.com/corona");

		// The dialog asks whether they really want to disable monetization so
		// if they answer "No" we need to re-check the checkbox
		if (answer == IDNO)
		{
			enableMonetizationBtn->SetCheck(BST_CHECKED);
		}
	}

	m_pProject->SetEnableMonetization((enableMonetizationBtn->GetCheck() == BST_CHECKED));
}
#endif // AUTO_INCLUDE_MONETIZATION_PLUGIN



void CBuildAndroidDlg::OnBnClickedCreateLiveBuild()
{
	// Check that they have the Debug keystore selected if they want a Live Build
	if (IsDlgButtonChecked(IDC_CREATE_LIVE_BUILD) == BST_CHECKED)
	{
		CString sKeystorePath;
		GetDlgItemText(IDC_BUILD_KEYSTORE, sKeystorePath);

		if( sKeystorePath.CompareNoCase( CCoronaProject::GetTrialKeystorePath() ) != 0 )
		{
			CString title;
			CString message;
			DWORD answer = IDNO;

			title.LoadString(IDS_CREATE_LIVE_BUILD);
			message.LoadString(IDS_CREATE_LIVE_BUILD_WARNING);

			answer = DisplayWarningMessageWithHelp(IDS_CREATE_LIVE_BUILD, IDS_CREATE_LIVE_BUILD_WARNING, _T("https://docs.coronalabs.com/guide/distribution/liveBuild/"));

			if (answer == IDYES)
			{
				SetDlgItemText(IDC_BUILD_KEYSTORE, CCoronaProject::GetTrialKeystorePath());
				UpdateAliasList();
			}
		}
	}
}

void CBuildAndroidDlg::LogAnalytics(const char *eventName, const char *key, const char *value)
{
	Rtt_ASSERT(GetWinProperties()->GetAnalytics() != NULL);
	Rtt_ASSERT(eventName != NULL && strlen(eventName) > 0);

	// NEEDSWORK: this is horrible
	size_t numItems = 2;
	char **dataKeys = (char **)calloc(sizeof(char *), numItems);
	char **dataValues = (char **)calloc(sizeof(char *), numItems);

	dataKeys[0] = _strdup("target");
	dataValues[0] = _strdup("android");
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
