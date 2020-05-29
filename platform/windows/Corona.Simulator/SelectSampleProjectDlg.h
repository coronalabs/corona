//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Resource.h"
#include <map>
#include <list>


/// Dialog used to select a sample project from the Samples directory.
class CSelectSampleProjectDlg : public CDialog
{
	DECLARE_DYNAMIC(CSelectSampleProjectDlg)

public:
	CSelectSampleProjectDlg(CWnd* pParent = NULL);
	virtual ~CSelectSampleProjectDlg();
	CString GetSelectedProjectPath();

	// Dialog Data
	enum { IDD = IDD_SELECT_SAMPLE_PROJECT };

private:
	struct SampleProjectInfo
	{
		CString ProjectName;
		CString ProjectPath;
		int ImageListIndex;
	};
	typedef std::map<CString, int> ImageListMapping;
	typedef std::list<SampleProjectInfo> SampleProjectInfoList;
	typedef std::map<CString, SampleProjectInfoList> CategorizedSampleProjectMap;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();
	void OnOK();
	void LoadProjects(CString &samplesDirectoryPath);
	void BuildList();
	int GetImageIndexForProjectName(CString &projectName);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnFilterComboBoxSelectionChanged();
	afx_msg void OnSampleListDoubleClicked(NMHDR *pNMHDR, LRESULT *pResult);

	CComboBox fFilterComboBox;
	CListCtrl fSampleListCtrl;
	CImageList fImageList;
	CString fSelectedProjectPath;
	ImageListMapping fImageListIndexMapping;
	CategorizedSampleProjectMap fCategorizedProjects;
};
