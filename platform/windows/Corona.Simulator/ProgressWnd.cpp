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
