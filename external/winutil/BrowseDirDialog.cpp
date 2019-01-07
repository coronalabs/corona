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

#include "BrowseDirDialog.h"
#include "shlobj.h"

// Wrapper around SHBrowseForFolder
// https://msdn.microsoft.com/en-us/library/ms907655.aspx
bool CBrowseDirDialog::Browse(CString &dir, UINT titleId)
{
	LPMALLOC pMalloc;
	if (SHGetMalloc(&pMalloc) != NOERROR)
	{
		return false;
	}

	dir.ReleaseBuffer();

	BROWSEINFO browseInfo = { 0 };

	CWnd *pWnd = CWnd::GetActiveWindow();
	if (pWnd == NULL)
	{
		pWnd = AfxGetMainWnd()->GetLastActivePopup();
	}
	if (pWnd == NULL)
	{
		pWnd = AfxGetMainWnd();
	}
	if (pWnd != NULL)
	{
		browseInfo.hwndOwner = pWnd->GetSafeHwnd();
	}

	CString title;
	if (titleId != 0)
	{
		title.LoadString(titleId);
		if (!title.IsEmpty())
		{
			browseInfo.lpszTitle = title;
		}
	}

	browseInfo.ulFlags = BIF_NEWDIALOGSTYLE | BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;

	if (!dir.IsEmpty())
	{
		browseInfo.lpfn = &CBrowseDirDialog::DefaultDirCallback;
		browseInfo.lParam = (LPARAM)&dir;
	}

	LPITEMIDLIST results = SHBrowseForFolder(&browseInfo);
	if (results == NULL)
	{
		return 0;
	}
	
	CString result;
	bool ret = (bool)SHGetPathFromIDList(results, result.GetBuffer(MAX_PATH));
	result.ReleaseBuffer();

	if (ret)
	{
		dir = result;
	}

	pMalloc->Free(results);
	pMalloc->Release();

	return ret;
}


int CBrowseDirDialog::DefaultDirCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	CString* defaultDir = (CString*)lpData;
	if (uMsg == BFFM_INITIALIZED && defaultDir && !defaultDir->IsEmpty())
	{
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)(LPCTSTR)(*defaultDir));
	}

	return 0;
}

