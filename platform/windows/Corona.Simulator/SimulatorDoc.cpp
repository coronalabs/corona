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

#include "SimulatorDoc.h"
#include "SimulatorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////
// CSimulatorDoc
//////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CSimulatorDoc, CDocument)

BEGIN_MESSAGE_MAP(CSimulatorDoc, CDocument)
END_MESSAGE_MAP()


// CSimulatorDoc construction/destruction

// Constructor - not sure why explicit call to mProject constructor is needed, but otherwise
// wasn't initialized properly.
CSimulatorDoc::CSimulatorDoc()
:	mProjectPointer(std::make_shared<CCoronaProject>())
{
}

// Destructor - nothing to clean up
CSimulatorDoc::~CSimulatorDoc()
{
}

/// Called when no Corona project file has been selected.
/// This happens on application startup or when the users closes the currently simulated project.
/// @return Returns true if this document was loaded successfully.
BOOL CSimulatorDoc::OnNewDocument()
{
	// Let the base class handle this first.
	// This will clear the last selected file path.
	if (!CDocument::OnNewDocument())
	{
		return FALSE;
	}
	
	// Clear the project settings.
	mProjectPointer = std::make_shared<CCoronaProject>();
    mProjectPointer->Init(_T(""));
	SetTitle(_T(""));
	
	// Update the view. This will show a home screen if enabled or a blank simulator screen.
	UpdateAllViews(NULL);
	return TRUE;
}

/// Called when a file is to be opened or saved.
/// Overriden to start simulation on an opened file.
/// @param ar Archive object indicating if we are to be reading or writing to file.
void CSimulatorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// The application does not support writing to the "main.lua" file.
	}
	else
	{
		// Do not continue if a "main.lua" file was not selected.
		CString name = ar.GetFile()->GetFileName();
		if (name != "main.lua")
		{
			AfxMessageBox( IDS_ONLYMAINLUA, MB_OK );
			return;
		}
		
		// Get the full absolute path to the file.
		CString path = ar.GetFile()->GetFilePath();
		DWORD attr = ::GetFileAttributes( path );
		if ( attr == INVALID_FILE_ATTRIBUTES )
		{
			AfxMessageBox( IDS_OPENMAINLUA, MB_OK );
			return;
		}
		
		// Store the file path and have the view start simulating the project.
		mProjectPointer = std::make_shared<CCoronaProject>();
		mProjectPointer->Init(path);
		UpdateAllViews(NULL);
	}
}

#ifdef _DEBUG
void CSimulatorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSimulatorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// SetTitle - Set window title to project's directory instead of "main.lua"
void CSimulatorDoc::SetTitle(LPCTSTR lpszTitle)
{
	CString sTitle = GetPath();

	if ( ! sTitle.IsEmpty() )
	{
		sTitle = CCoronaProject::RemoveMainLua( sTitle );
		sTitle = sTitle.Right( sTitle.GetLength() - sTitle.ReverseFind(_T('\\')) - 1 );
	}
	else sTitle = lpszTitle;

	CDocument::SetTitle(sTitle);
}
