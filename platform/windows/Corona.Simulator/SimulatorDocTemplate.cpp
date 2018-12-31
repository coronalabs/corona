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
