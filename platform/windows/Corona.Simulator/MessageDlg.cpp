//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "simulator.h"
#include "MessageDlg.h"
#include "afxinet.h"  // AfxParseUrl()
#include "winstring.h"
#include "CoronaInterface.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
// CMessageDlg dialog
////////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CMessageDlg, CDialog)

// Constructor - initialize all members
CMessageDlg::CMessageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMessageDlg::IDD, pParent),
      m_sTitle(_T("")),
      m_sText(_T("")),
      m_sButtonDefault(_T("")),
      m_sButtonAlt(_T("")),
      m_sButton3(_T("")),
      m_sUrl(_T("")),
      m_sFolder(_T("")),
      m_nIconStyle( 0 ),
	  m_pLuaResource( NULL )

{
}

// Destructor - nothing to clean up
CMessageDlg::~CMessageDlg()
{
}

void CMessageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMessageDlg, CDialog)
	ON_BN_CLICKED(ID_MSG_BUTTON2, &CMessageDlg::OnButton2)
	ON_BN_CLICKED(ID_MSG_BUTTON3, &CMessageDlg::OnButton3)
END_MESSAGE_MAP()


// CMessageDlg message handlers

// OnInitDialog - initialize title, message, button text, icon
BOOL CMessageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    if( ! m_sTitle.IsEmpty() )
        SetWindowText( m_sTitle );
    SetDlgItemText( IDC_MSG_TEXT, m_sText );
    if( ! m_sButtonDefault.IsEmpty() )
		SetDlgItemText( IDOK, m_sButtonDefault );
    if( m_sButtonAlt.IsEmpty() )
        GetDlgItem( ID_MSG_BUTTON2 )->ShowWindow( SW_HIDE );
	else SetDlgItemText( ID_MSG_BUTTON2, m_sButtonAlt );
    if( m_sButton3.IsEmpty() )
        GetDlgItem( ID_MSG_BUTTON3 )->ShowWindow( SW_HIDE );
	else SetDlgItemText( ID_MSG_BUTTON3, m_sButton3 );

    CStatic *pIcon = (CStatic *)GetDlgItem( IDC_MSG_ICON );
    if( m_nIconStyle == MB_ICONEXCLAMATION )
		pIcon->SetIcon( ::LoadIcon(NULL, IDI_WARNING) );
	else if( m_nIconStyle == MB_ICONINFORMATION )
		pIcon->SetIcon( ::LoadIcon(NULL, IDI_INFORMATION) );

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// OnCancel - Cancel msg or Esc.  No Cancel button.
void CMessageDlg::OnCancel()
{
// EndDialog is called in EndNativeAlert
//	CDialog::OnCancel();  

    if( IsNativeAlert() )
        EndNativeAlert( IDCANCEL );
}

// OnOK - First button (from right), default button
// If there's a url set, execute it (e.g. Purchase) and don't exit message dlg
void CMessageDlg::OnOK()
{
    if( IsNativeAlert() )
        EndNativeAlert( IDOK );
    else if( ! m_sUrl.IsEmpty() )
	{
        // Dialog stays open in this case
        // Url is validated in SetUrl()
		ShellExecute( NULL, _T("open"), m_sUrl, NULL, NULL, SW_SHOWNORMAL);
	}
	else CDialog::OnOK();
}

// OnButton2 - Second button (from right)
// If there's a folder set, open it (e.g. successful build)
void CMessageDlg::OnButton2()
{
    if( IsNativeAlert() )
        EndNativeAlert( ID_MSG_BUTTON2 );
	else if( ! m_sFolder.IsEmpty() )
	{
        // Folder is validated in SetFolder()
        ShellExecute( NULL, _T("open"), m_sFolder, NULL, NULL, SW_SHOWNORMAL );
		CDialog::EndDialog( ID_MSG_BUTTON2 );
	}
	else CDialog::EndDialog( ID_MSG_BUTTON2 );
}

// OnButton3 - Third button (from right)
void CMessageDlg::OnButton3()
{
    if( IsNativeAlert() )
        EndNativeAlert( ID_MSG_BUTTON3 );
	else CDialog::EndDialog( ID_MSG_BUTTON3 );
}

// SetTitle - title of message dialog
void CMessageDlg::SetTitle( int nID )
{
   if( nID )
   {
       CString sTitle;
       sTitle.LoadStringW( nID );
       SetTitle( sTitle );
   }
}

// SetText - message text, loaded from resource id
void CMessageDlg::SetText( int nID )
{
   if( nID )
   {
       CString sText;
       sText.LoadStringW( nID );
       SetText( sText );
   }
}

// SetDefaultText - text for OK button (Button1), loaded from resource id
void CMessageDlg::SetDefaultText( int nID )
{
   if( nID )
   {
       CString sButton;
       sButton.LoadStringW( nID );
       SetDefaultText( sButton );
   }
}

// SetAltText - text for Alt button (Button2), loaded from resource id
void CMessageDlg::SetAltText( int nID )
{
   if( nID )
   {
       CString sButton;
       sButton.LoadStringW( nID );
       SetAltText( sButton );
   }
}

// SetButton3Text - text for Button3, loaded from resource id
void CMessageDlg::SetButton3Text( int nID )
{
   if( nID )
   {
       CString sButton;
       sButton.LoadStringW( nID );
       SetButton3Text( sButton );
   }
}

// SetUrl - char * version, calls TCHAR version, which validates it
// Url is loaded when Button1/OK is pressed.
// Returns true if url is valid (or empty)
bool CMessageDlg::SetUrl( const char *url )
{
     if( NULL == url ) 
	 {
         m_sUrl = _T("");
         return true;
	 }

     WinString strUrl;
     strUrl.SetUTF8( url );

     return SetUrl( strUrl.GetTCHAR() );
}

// SetUrl - TCHAR version, validates URL
// Url is loaded when Button1/OK is pressed.
// Returns true if url is valid (or empty)
bool CMessageDlg::SetUrl( const TCHAR *sUrl )
{
     if( NULL == sUrl ) 
	 {
         m_sUrl = _T("");
         return true;
	 }

     DWORD dwServiceType;
     CString strServer, strObject;
     INTERNET_PORT nPort;

	 if( AfxParseURL( sUrl, dwServiceType, strServer, strObject, nPort )
         && ( (dwServiceType == AFX_INET_SERVICE_HTTP)
               || (dwServiceType == AFX_INET_SERVICE_HTTPS) ) )
	 {
		 m_sUrl = sUrl; 
		 return true;
	 }

	 return false;
}

// SetFolder - validate folder
// Folder is opened in Explorer when Button2/Alt is pressed
// Returns true if folder is valid (or empty)
bool CMessageDlg::SetFolder( CString sFolder )
{
     if( sFolder.IsEmpty() )
	 {
         m_sFolder = sFolder;
         return true;
	 }

     // No trailing backslashes
     if( sFolder[sFolder.GetLength() - 1] == _T('\\') )
		 sFolder.Left(sFolder.GetLength() - 1);

	 if( CSimulatorApp::CheckDirExists( sFolder ) )
	 {
		 m_sFolder = sFolder; 
		 return true;
	 }

	 return false;
}

// SetNativeAlertInfo - parameter to pass back to lua
void CMessageDlg::SetNativeAlertInfo( void *pLuaResource )
{
    m_pLuaResource = pLuaResource;
}

// IsNativeAlert - was this called from lua code
bool CMessageDlg::IsNativeAlert()
{
    return (m_pLuaResource != NULL);
}

// EndNativeAlert - close dialog, return which button clicked
// This can be called from lua code as well
void CMessageDlg::EndNativeAlert( int idButton )
{
	// Close this dialog.
	CDialog::EndDialog( idButton );

	// Fetch a zero based index of the button that was clicked on.
    bool canceled = (idButton == IDCANCEL);
    int index = 0;
    if( idButton == IDOK )
        index = 0;
	else if( idButton == ID_MSG_BUTTON2 )
        index = 1;
	else if( idButton == ID_MSG_BUTTON3 )
        index = 2;

	// Fetch the Lua listener resource.
	// Note: We must null out the member variable before calling appEndNativeAlert() in case the Lua listener
	//       shows another native alert, which may re-use this dialog and overwrite this member variable.
	//       We also must null out this member variable in case this function is recursively called below as well.
	auto luaResourcePointer = m_pLuaResource;
	m_pLuaResource = nullptr;

	// Invoke the Lua listener.
	appEndNativeAlert( luaResourcePointer, index, canceled );
}
