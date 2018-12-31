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
#include "SelectSampleProjectDlg.h"
#include "Simulator.h"
#include <Uxtheme.h>


IMPLEMENT_DYNAMIC(CSelectSampleProjectDlg, CDialog)

#pragma region Constructor/Destructor
/// Creates a new "Select Samle Project" dialog.
CSelectSampleProjectDlg::CSelectSampleProjectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectSampleProjectDlg::IDD, pParent)
{
	int index;

	// Create image list for the list control.
	HINSTANCE instanceHandle = AfxGetInstanceHandle();
	fImageList.Create(32, 32, ILC_COLOR32, 0, 1);
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDR_MAINFRAME)));
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_ACTIVITY_INDICATOR)));
	fImageListIndexMapping[CString(_T("ActivityIndicator"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_ALERT)));
	fImageListIndexMapping[CString(_T("Alert"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_ANIMATION)));
	fImageListIndexMapping[CString(_T("TimeAnimation"))] = index;
	fImageListIndexMapping[CString(_T("FrameAnimation2"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_AUDIO)));
	fImageListIndexMapping[CString(_T("AudioPlayer"))] = index;
	fImageListIndexMapping[CString(_T("EventSound"))] = index;
	fImageListIndexMapping[CString(_T("SimpleMixer"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_BALLOON)));
	fImageListIndexMapping[CString(_T("CollisionFilter"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_BRIDGE)));
	fImageListIndexMapping[CString(_T("Bridge"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_BUTTONS)));
	fImageListIndexMapping[CString(_T("ButtonEvents"))] = index;
	fImageListIndexMapping[CString(_T("MultitouchButton"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_CAMERA)));
	fImageListIndexMapping[CString(_T("Camera"))] = index;
	fImageListIndexMapping[CString(_T("ScreenCaptureToFile"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_CAN)));
	fImageListIndexMapping[CString(_T("Bullet"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_CHAINS)));
	fImageListIndexMapping[CString(_T("Chains"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_COMPASS)));
	fImageListIndexMapping[CString(_T("Compass"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_CRATE)));
	fImageListIndexMapping[CString(_T("CollisionDetection"))] = index;
	fImageListIndexMapping[CString(_T("HelloPhysics"))] = index;
	fImageListIndexMapping[CString(_T("ManyCrates"))] = index;
	fImageListIndexMapping[CString(_T("ShapeTumbler"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_EGGBREAKER)));
	fImageListIndexMapping[CString(_T("EggBreaker"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_FACEBOOK)));
	fImageListIndexMapping[CString(_T("Facebook"))] = index;
	fImageListIndexMapping[CString(_T("Scrumptious"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_FISHIES)));
	fImageListIndexMapping[CString(_T("Fishies"))] = index;
	fImageListIndexMapping[CString(_T("Fishies-iPad"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_FRUIT)));
	fImageListIndexMapping[CString(_T("FrameAnimation1"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_GLOBE)));
	fImageListIndexMapping[CString(_T("HelloWorld"))] = index;
	fImageListIndexMapping[CString(_T("HelloWorldLocalized"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_GPS)));
	fImageListIndexMapping[CString(_T("GPS"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_INNERACTIVE)));
	fImageListIndexMapping[CString(_T("inneractive"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_INTERNET)));
	fImageListIndexMapping[CString(_T("AsynchHTTP"))] = index;
	fImageListIndexMapping[CString(_T("AsynchImageDownload"))] = index;
	fImageListIndexMapping[CString(_T("Reachability"))] = index;
	fImageListIndexMapping[CString(_T("SimpleImageDownload"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_JUNGLESCENE)));
	fImageListIndexMapping[CString(_T("JungleScene"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_MAIL)));
	fImageListIndexMapping[CString(_T("ComposeEmailSMS"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_MICROPHONE)));
	fImageListIndexMapping[CString(_T("SimpleAudioRecorder"))] = index;
	fImageListIndexMapping[CString(_T("SimpleTuner"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_MOVIECLIP)));
	fImageListIndexMapping[CString(_T("Movieclip"))] = index;
	fImageListIndexMapping[CString(_T("StreamingVideo"))] = index;
	fImageListIndexMapping[CString(_T("StreamingVideoPort"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_MULTIPUCK)));
	fImageListIndexMapping[CString(_T("MultiPuck"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_MULTITOUCH)));
	fImageListIndexMapping[CString(_T("DragMe"))] = index;
	fImageListIndexMapping[CString(_T("DragMeMultitouch"))] = index;
	fImageListIndexMapping[CString(_T("MultitouchFingers"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_OPEN_FEINT)));
	fImageListIndexMapping[CString(_T("OFlandscape"))] = index;
	fImageListIndexMapping[CString(_T("OFportrait"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_PLATFORM)));
	fImageListIndexMapping[CString(_T("DragPlatforms"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_POLYLINES)));
	fImageListIndexMapping[CString(_T("PolyLines"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_SIMPLEPOOL)));
	fImageListIndexMapping[CString(_T("SimplePool"))] = index;
	fImageListIndexMapping[CString(_T("SimplePoolPlus"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_SPOTLIGHT)));
	fImageListIndexMapping[CString(_T("Flashlight"))] = index;
	fImageListIndexMapping[CString(_T("X-ray"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_TEXT_INPUT)));
	fImageListIndexMapping[CString(_T("NativeDisplayObjects"))] = index;
	fImageListIndexMapping[CString(_T("NativeKeyboard2"))] = index;
	index = fImageList.Add(LoadIcon(instanceHandle, MAKEINTRESOURCE(IDI_TWITTER)));
	fImageListIndexMapping[CString(_T("Twitter"))] = index;

	// Load all projects in the samples directory.
	// This populates the fCategorizedProjects map used to set up the list control later.
	LoadProjects(((CSimulatorApp*)AfxGetApp())->GetSampleDir());
}

/// Destructor.
CSelectSampleProjectDlg::~CSelectSampleProjectDlg()
{
}

#pragma endregion


#pragma region Message Mappings
/// Associates member variables with window's controls.
void CSelectSampleProjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SAMPLE_LIST, fSampleListCtrl);
	DDX_Control(pDX, IDC_FILTER_COMBO_BOX, fFilterComboBox);
}

BEGIN_MESSAGE_MAP(CSelectSampleProjectDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_FILTER_COMBO_BOX, &CSelectSampleProjectDlg::OnFilterComboBoxSelectionChanged)
	ON_NOTIFY(NM_DBLCLK, IDC_SAMPLE_LIST, &CSelectSampleProjectDlg::OnSampleListDoubleClicked)
END_MESSAGE_MAP()

#pragma endregion


#pragma region Window Event Handlers
/// Initializes this dialog's controls.
BOOL CSelectSampleProjectDlg::OnInitDialog()
{
	CString text;
	int index;

	// Initialize base class first.
	CDialog::OnInitDialog();

	// Clear the last selected project path. To be set when the user presses the OK button.
	fSelectedProjectPath.Empty();

	// Set up the combo box with category names, used to filter the list control.
	fFilterComboBox.Clear();
	text.LoadString(IDS_FILTER_ALL_PROJECTS);
	index = fFilterComboBox.AddString(text);
	fFilterComboBox.SetCurSel(index);
	for (CategorizedSampleProjectMap::iterator iterator = fCategorizedProjects.begin();
	     iterator != fCategorizedProjects.end(); iterator++)
	{
		fFilterComboBox.AddString((*iterator).first);
	}

	// Set up list control to show a grouped tile view.
	// Setting the theme to "explorer" makes it show the Windows 7/Vista theme on those OSes.
	SetWindowTheme(fSampleListCtrl.GetSafeHwnd(), _T("explorer"), NULL);
	fSampleListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	LVTILEVIEWINFO tileInfo;
	tileInfo.cbSize = sizeof(tileInfo);
	tileInfo.dwMask = LVTVIM_COLUMNS | LVTVIM_TILESIZE;
	tileInfo.cLines = 0;
	tileInfo.dwFlags = LVTVIF_AUTOSIZE;
	fSampleListCtrl.SetTileViewInfo(&tileInfo);
	fSampleListCtrl.SetView(LV_VIEW_TILE);
	fSampleListCtrl.EnableGroupView(TRUE);
	fSampleListCtrl.SetImageList(&fImageList, LVSIL_NORMAL);

	// Add projects to the list control.
	// This must be done after setting up the combo box.
	BuildList();

	// Return TRUE unless you set the focus to a control.
	return TRUE;
}

/// Called when the combo box selection has changed.
/// Filters the list control's contents by only showing projects belonging to the current selection.
void CSelectSampleProjectDlg::OnFilterComboBoxSelectionChanged()
{
	BuildList();
}

/// Called when the user double clicks inside the list control.
/// Selects the double clicked project and closes this dialog.
void CSelectSampleProjectDlg::OnSampleListDoubleClicked(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW *listViewPointer;

	// The result must be set to zero.
	*pResult = 0;

	// Do not continue if the user double clicked in an empty spot within the control.
	listViewPointer = (NM_LISTVIEW*)pNMHDR;
	if (!listViewPointer)
	{
		return;
	}
	if ((listViewPointer->iItem < 0) || (listViewPointer->iItem >= fSampleListCtrl.GetItemCount()))
	{
		return;
	}

	// A selection was made. Store the selection and close this dialog.
	OnOK();
}

/// Stores the sample project selection and then closes this dialog.
void CSelectSampleProjectDlg::OnOK()
{
	SampleProjectInfo *projectInfoPointer = NULL;
	POSITION itemPosition;

	// Fetch the selected project from the list control.
	itemPosition = fSampleListCtrl.GetFirstSelectedItemPosition();
	if (itemPosition)
	{
		int index = fSampleListCtrl.GetNextSelectedItem(itemPosition);
		if (index >= 0)
		{
			projectInfoPointer = (SampleProjectInfo*)fSampleListCtrl.GetItemData(index);
		}
	}

	// Do not continue if a selection was not made.
	if (!projectInfoPointer)
	{
		CString title;
		CString message;
		title.LoadString(IDS_WARNING);
		message.LoadString(IDS_MUST_SELECT_SAMPLE_PROJECT);
		MessageBox(message, title, MB_OK | MB_ICONWARNING);
		return;
	}

	// Store the selection and then close this dialog.
	fSelectedProjectPath = projectInfoPointer->ProjectPath;
	CDialog::OnOK();
}

#pragma endregion


#pragma region Public Functions
/// Gets the absolute path to the sample project folder that the user selected.
/// This function should only be called after the user presses the OK button on this dialog.
/// @return Returns an absolute path to the sample project folder.
///         Returns an empty string if the user canceled out of this dialog.
CString CSelectSampleProjectDlg::GetSelectedProjectPath()
{
	return fSelectedProjectPath;
}

#pragma endregion


#pragma region Private Functions
/// Scans the given path for sample projects and adds them to the "fCategorizedProjects" member variable.
/// @param samplesDirectoryPath Path to the root samples directory.
void CSelectSampleProjectDlg::LoadProjects(CString &samplesDirectoryPath)
{
	WIN32_FIND_DATA rootFindFileData;
	WIN32_FIND_DATA subdirectoryFindFileData;
	HANDLE rootFindFileHandle;
	HANDLE subdirectoryFindFileHandle;
	CString subdirectoryPath;
	CString projectPath;

	// Remove any existing items in the list control.
	fCategorizedProjects.clear();

	// Validate argument.
	if (samplesDirectoryPath.IsEmpty())
	{
		return;
	}

	// Start enumerating the root samples directory. This also checks if the directory exists.
	rootFindFileHandle = ::FindFirstFile(CString(samplesDirectoryPath + _T("\\*")), &rootFindFileData);
	if (INVALID_HANDLE_VALUE == rootFindFileHandle)
	{
		// Directory not found. Do not continue.
		return;
	}

	// Traverse all directories in the root samples directory.
	// These directories split sample projects into categories.
	do
	{
		if ((rootFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			(_tcscmp(rootFindFileData.cFileName, _T(".")) != 0) &&
		    (_tcscmp(rootFindFileData.cFileName, _T("..")) != 0))
		{
			// Traverse the subdirectory's directories.
			// These directories also represent the sample project names.
			subdirectoryPath = samplesDirectoryPath;
			subdirectoryPath += _T("\\");
			subdirectoryPath += rootFindFileData.cFileName;
			subdirectoryFindFileHandle =
					::FindFirstFile(CString(subdirectoryPath + _T("\\*")), &subdirectoryFindFileData);
			if (subdirectoryFindFileHandle != INVALID_HANDLE_VALUE)
			{
				do
				{
					// Add the project to the mapping if the directory contains a "main.lua" file.
					if ((subdirectoryFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
						(_tcscmp(subdirectoryFindFileData.cFileName, _T(".")) != 0) &&
						(_tcscmp(subdirectoryFindFileData.cFileName, _T("..")) != 0))
					{
						projectPath = subdirectoryPath;
						projectPath += _T("\\");
						projectPath += subdirectoryFindFileData.cFileName;
						CString projectFilePath = projectPath + _T("\\main.lua");
						if (((CSimulatorApp*)AfxGetApp())->CheckPathExists(projectFilePath))
						{
							SampleProjectInfo info;
							info.ProjectName = subdirectoryFindFileData.cFileName;
							info.ProjectPath = projectPath;
							info.ImageListIndex = GetImageIndexForProjectName(CString(subdirectoryFindFileData.cFileName));
							fCategorizedProjects[CString(rootFindFileData.cFileName)].push_back(info);
						}
					}
				} while (::FindNextFile(subdirectoryFindFileHandle, &subdirectoryFindFileData) != FALSE);
				::FindClose(subdirectoryFindFileHandle);
			}
		}
	} while (::FindNextFile(rootFindFileHandle, &rootFindFileData) != FALSE);

	// Close the handle to the root samples directory.
	::FindClose(rootFindFileHandle);
}

/// Updates the list control with sample projects.
/// Only shows projects that belong to the current selection in the filter combo box.
void CSelectSampleProjectDlg::BuildList()
{
	CategorizedSampleProjectMap::iterator mapIterator;
	SampleProjectInfoList::iterator listIterator;
	CString filterName;
	LVGROUP group;
	LVITEM item;
	int index;

	// Remove any existing items in the list control.
	fSampleListCtrl.DeleteAllItems();

	// Fetch the currently selected category name from the combo box.
	// Ignore the first selection, which is the "- All Projects" selection.
	// Leave the "filterName" variable set to an empty string if we do NOT want to filter.
	index = fFilterComboBox.GetCurSel();
	if (index > 0)
	{
		fFilterComboBox.GetLBText(index, filterName);
	}

	// Initialize group object to be used by all group headers in the list control.
	ZeroMemory(&group, sizeof(group));
	group.cbSize = sizeof(group);
	group.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN | LVGF_STATE;
	group.state = LVGS_NORMAL;
	group.uAlign = LVGA_HEADER_LEFT;
	group.iGroupId = 0;

	// Initialize list control item.
	ZeroMemory(&item, sizeof(item));
	item.mask = LVIF_GROUPID | LVIF_COLUMNS | LVIF_IMAGE;
	item.cColumns = 0;

	// Add sample projects to list controls by groups.
	for (mapIterator = fCategorizedProjects.begin();
	     mapIterator != fCategorizedProjects.end(); mapIterator++)
	{
		// Skip adding this group's projects if it is filtered out.
		CString categoryName = (*mapIterator).first;
		if (!filterName.IsEmpty() && (filterName != categoryName))
		{
			continue;
		}

		// Add group to list control. This must be done before adding the group's items.
		group.pszHeader = categoryName.GetBuffer();
		group.cchHeader = categoryName.GetLength();
		fSampleListCtrl.InsertGroup(group.iGroupId, &group);
		categoryName.ReleaseBuffer();

		// Add groups sample projects to list control.
		for (listIterator = (*mapIterator).second.begin();
		     listIterator != (*mapIterator).second.end(); listIterator++)
		{
			index = fSampleListCtrl.GetItemCount();
			fSampleListCtrl.InsertItem(index, (*listIterator).ProjectName);
			item.iItem = index;
			item.iGroupId = group.iGroupId;
			item.iImage = (*listIterator).ImageListIndex;
			fSampleListCtrl.SetItem(&item);
			fSampleListCtrl.SetItemData(index, (DWORD_PTR)&(*listIterator));
		}

		// Generate a unique ID for the next group.
		group.iGroupId++;
	}
}

/// Gets an index to an image within the CImageList for the given sample project.
/// @param projectName The name of the sample project folder. Must not include the path.
/// @return Returns an index to an image within the CImageList that the list control uses.
int CSelectSampleProjectDlg::GetImageIndexForProjectName(CString &projectName)
{
	ImageListMapping::iterator iterator;
	int index = 0;

	iterator = fImageListIndexMapping.find(projectName);
	if (fImageListIndexMapping.end() != iterator)
	{
		index = (*iterator).second;
	}
	return index;
}

#pragma endregion
