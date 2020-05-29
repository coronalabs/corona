//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SimulatorDocTemplate.h"
#include "MainFrm.h"
#include "SimulatorDoc.h"
#include "SimulatorView.h"
#include "Resource.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CSimulatorDocTemplate, CSingleDocTemplate)


#pragma region Constructor/Destructor
CSimulatorDocTemplate::CSimulatorDocTemplate()
:	CSimulatorDocTemplate(
		IDR_MAINFRAME, RUNTIME_CLASS(CSimulatorDoc), RUNTIME_CLASS(CMainFrame), RUNTIME_CLASS(CSimulatorView))
{
}

CSimulatorDocTemplate::CSimulatorDocTemplate(
	UINT nIDResource, CRuntimeClass* pDocClass, CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass)
:	CSingleDocTemplate(nIDResource, pDocClass, pFrameClass, pViewClass)
{
}

CSimulatorDocTemplate::~CSimulatorDocTemplate()
{
}

#pragma endregion


#pragma region Public Methods
CDocument* CSimulatorDocTemplate::OpenDocumentFile(LPCTSTR lpszPathName, BOOL bMakeVisible /*= TRUE*/)
{
	CString updatedPathName;
	if (UpdateFilePath(lpszPathName, updatedPathName))
	{
		lpszPathName = updatedPathName;
	}
	return CSingleDocTemplate::OpenDocumentFile(lpszPathName, bMakeVisible);
}

CDocument* CSimulatorDocTemplate::OpenDocumentFile(LPCTSTR lpszPathName, BOOL bAddToMRU, BOOL bMakeVisible)
{
	CString updatedPathName;
	if (UpdateFilePath(lpszPathName, updatedPathName))
	{
		lpszPathName = updatedPathName;
	}
	return CSingleDocTemplate::OpenDocumentFile(lpszPathName, bAddToMRU, bMakeVisible);
}

#pragma endregion

#pragma region Private Methods
bool CSimulatorDocTemplate::UpdateFilePath(LPCTSTR filePath, CString& updatedFilePath) const
{
	// If the given file path references a Corona project directory,
	// then update the path to reference it's "main.lua" file instead.
	bool wasUpdated = false;
	if (filePath && ::PathIsDirectory(filePath))
	{
		CString mainLuaFilePath(filePath);
		mainLuaFilePath.Append(_T("\\main.lua"));
		if (::PathFileExists(mainLuaFilePath))
		{
			updatedFilePath = mainLuaFilePath;
			wasUpdated = true;
		}
	}
	return wasUpdated;
}

#pragma endregion
