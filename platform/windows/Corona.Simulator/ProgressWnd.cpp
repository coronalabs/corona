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
#include "ProgressWnd.h"


//////////////////////////////////////////////////////////////////////////////////////////////////
// CProgressWnd modeless dialog
//////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CProgressWnd, CDialog)

// Constructor - initialize members
CProgressWnd::CProgressWnd(CWnd* pParent /*=NULL*/)
	: CDialog(CProgressWnd::IDD, pParent)
{
	m_sText = "";
}

// Destructor - nothing to clean up
CProgressWnd::~CProgressWnd()
{
}

void CProgressWnd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

// OnInitDialog - initialize progress bar
BOOL CProgressWnd::OnInitDialog()
{
	CDialog::OnInitDialog();

    GetDlgItem( IDC_PROGRESSWND_TEXT )->SetWindowText( m_sText );

	CProgressCtrl *pProgress = (CProgressCtrl *)GetDlgItem( IDC_PROGRESS );
	pProgress->SetRange(0, 100);
	pProgress->SetStep(1);
	pProgress->SetPos(50);
	//pProgress->ModifyStyle( 0, PBS_MARQUEE );
	//pProgress->SetMarquee(TRUE, 0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CProgressWnd, CDialog)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CProgressWnd message handlers

// OnDestroy
void CProgressWnd::OnDestroy()
{
	CDialog::OnDestroy();
}
 
// SetText - load text from resource id
void CProgressWnd::SetText( int nID )
{
   if( nID )
   {
       CString sText;
       sText.LoadString( nID );
       SetText( sText );
   }
}


///////////////////////////////////////////////////////////////////////
// CAutoProgressWnd class -- automatically show & hide progress windows
// Declare local variable in the same scope over which work is occurring,
// similar to CWaitCursor
///////////////////////////////////////////////////////////////////////
// Constructor - show windows

CAutoProgressWnd::CAutoProgressWnd()
{  
	((CSimulatorApp *)AfxGetApp())->ShowProgressWnd( true ); 
}

CAutoProgressWnd::CAutoProgressWnd( CWnd *pParent )
{  
	((CSimulatorApp *)AfxGetApp())->ShowProgressWnd( true, pParent ); 
}

// Destructor - hide window
CAutoProgressWnd::~CAutoProgressWnd()
{ 
	Close();
}

void CAutoProgressWnd::Close()
{
	((CSimulatorApp *)AfxGetApp())->ShowProgressWnd( false ); 
}
