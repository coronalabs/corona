//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

