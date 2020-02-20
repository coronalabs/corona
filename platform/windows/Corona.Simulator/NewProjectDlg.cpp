//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Shlobj.h>
#include <Shlwapi.h>
#include "NewProjectDlg.h"
#include "Simulator.h"
#include "Core/Rtt_Build.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "WinString.h"
#include "BrowseDirDialog.h"

Rtt_EXPORT int luaopen_lfs (lua_State *L);

IMPLEMENT_DYNAMIC(CNewProjectDlg, CDialog)


#pragma region Constructor/Destructor
/// Creates a "New Project" window.
CNewProjectDlg::CNewProjectDlg(CWnd* pParent /*=NULL*/)
:	CDialog(CNewProjectDlg::IDD, pParent),
	fPhoneScreenSize(320, 480),
	fTabletScreenSize(768, 1024)
{
}

/// Destructor.
CNewProjectDlg::~CNewProjectDlg()
{
}

#pragma endregion


#pragma region Message Mappings
/// Associates member variables with window's controls.
void CNewProjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_APP_NAME_EDIT_BOX, fAppNameEditBox);
	DDX_Control(pDX, IDC_APP_LOCATION_EDIT_BOX, fProjectLocationEditBox);
	DDX_Control(pDX, IDC_SCREEN_SIZE_COMBO_BOX, fScreenSizeComboBox);
	DDX_Control(pDX, IDC_SCREEN_WIDTH_EDIT_BOX, fScreenWidthEditBox);
	DDX_Control(pDX, IDC_SCREEN_HEIGHT_EDIT_BOX, fScreenHeightEditBox);
}

BEGIN_MESSAGE_MAP(CNewProjectDlg, CDialog)
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, &CNewProjectDlg::OnClickedBrowseButton)
	ON_EN_CHANGE(IDC_APP_NAME_EDIT_BOX, &CNewProjectDlg::OnChangeAppNameEditBox)
	ON_CBN_SELCHANGE(IDC_SCREEN_SIZE_COMBO_BOX, &CNewProjectDlg::OnScreenSizeComboBoxSelectionChanged)
	ON_BN_CLICKED(IDC_TEMPLATE_EBOOK, &CNewProjectDlg::OnClickedTemplateEbook)
END_MESSAGE_MAP()

#pragma endregion


#pragma region Window Event Handlers
/// Initializes this dialog's controls.
BOOL CNewProjectDlg::OnInitDialog()
{
	LOGFONT fontSettings;
	CFont *fontPointer;
	CButton *buttonPointer;
	CString stringBuffer;
	HRESULT result;
	char pathName[MAX_PATH];
	int index;

	// Initialize base class first.
	CDialog::OnInitDialog();

	// Clear the "new project path". To be set when the user presses the OK button.
	fNewProjectPath.Empty();

	// Set a maximum character limit for the application name edit box.
	fAppNameEditBox.SetLimitText(32);

	// Create a directory under "My Documents" where new projects will go to by default.
	pathName[0] = '\0';
	result = ::SHGetFolderPathA(NULL, CSIDL_MYDOCUMENTS, NULL, 0, pathName);
	if (S_OK == result)
	{
		strcat_s(pathName, MAX_PATH, "\\Corona Projects");
		result = ::SHCreateDirectoryExA(NULL, pathName, NULL);
	}

	// Set up the project folder selection dialog and field. Have it default to the above path.
	fNewProjectPath = pathName;
	UpdateProjectLocationField();

	// Set a maximum character limit for the screen width/height edit boxes.
	fScreenWidthEditBox.SetLimitText(5);
	fScreenHeightEditBox.SetLimitText(5);

	// Make all of the project template text bold.
	buttonPointer = (CButton*)GetDlgItem(IDC_TEMPLATE_BLANK);
	fontPointer = buttonPointer->GetFont();
	fontPointer->GetLogFont(&fontSettings);
	fontSettings.lfWeight = FW_BOLD;
	fBoldRadioButtonFont.CreateFontIndirect(&fontSettings);
	buttonPointer->SetFont(&fBoldRadioButtonFont);
	buttonPointer = (CButton*)GetDlgItem(IDC_TEMPLATE_TAB_BAR);
	buttonPointer->SetFont(&fBoldRadioButtonFont);
	buttonPointer = (CButton*)GetDlgItem(IDC_TEMPLATE_GAME);
	buttonPointer->SetFont(&fBoldRadioButtonFont);
	buttonPointer = (CButton*)GetDlgItem(IDC_TEMPLATE_EBOOK);
	buttonPointer->SetFont(&fBoldRadioButtonFont);

	// Check the "Blank" project template button by default.
	buttonPointer = (CButton*)GetDlgItem(IDC_TEMPLATE_BLANK);
	buttonPointer->SetCheck(BST_CHECKED);

	// Check the "Upright" orientation button by default.
	buttonPointer = (CButton*)GetDlgItem(IDC_ORIENTATION_UPRIGHT);
	buttonPointer->SetCheck(BST_CHECKED);

	// Set up screen size ComboBox.
	stringBuffer.LoadString(IDS_SCREEN_SIZE_PHONE);
	index = fScreenSizeComboBox.AddString(stringBuffer);
	fScreenSizeComboBox.SetItemData(index, (DWORD_PTR)&fPhoneScreenSize);
	fScreenSizeComboBox.SetCurSel(index);
	stringBuffer.LoadString(IDS_SCREEN_SIZE_TABLET);
	index = fScreenSizeComboBox.AddString(stringBuffer);
	fScreenSizeComboBox.SetItemData(index, (DWORD_PTR)&fTabletScreenSize);
	stringBuffer.LoadString(IDS_SCREEN_SIZE_CUSTOM);
	index = fScreenSizeComboBox.AddString(stringBuffer);
	fScreenSizeComboBox.SetItemData(index, NULL);
	OnScreenSizeComboBoxSelectionChanged();

	// Return TRUE unless you set the focus to a control.
	return TRUE;
}

/// Called when the text in the application name field has changed.
/// Updates the project folder field with a new path, appending the new application name to the path.
void CNewProjectDlg::OnChangeAppNameEditBox()
{
	UpdateProjectLocationField();
}

/// Called when the user selects a new screen size from the ComboBox.
/// Updates the width and height edit fields.
void CNewProjectDlg::OnScreenSizeComboBoxSelectionChanged()
{
	CSize *sizePointer;
	int index;

	// Fetch the selected item.
	index = fScreenSizeComboBox.GetCurSel();
	if (index < 0)
	{
		return;
	}

	// Update the width/height edit fields according to the current selection.
	sizePointer = (CSize*)fScreenSizeComboBox.GetItemData(index);
	if (sizePointer)
	{
		CString text;
		text.Format(_T("%d"), sizePointer->cx);
		fScreenWidthEditBox.SetWindowText(text);
		text.Format(_T("%d"), sizePointer->cy);
		fScreenHeightEditBox.SetWindowText(text);
	}
	fScreenWidthEditBox.EnableWindow(sizePointer ? FALSE : TRUE);
	fScreenHeightEditBox.EnableWindow(sizePointer ? FALSE : TRUE);
}

/// Displays a directory selection dialog for the user to select a root project directory.
void CNewProjectDlg::OnClickedBrowseButton()
{
	// Display a folder selection dialog. Update the project folder field if a selection was made.
	if (CBrowseDirDialog::Browse(fNewProjectPath, IDS_SELECT_ROOT_PROJECT_FOLDER_DESCRIPTION))
	{
		UpdateProjectLocationField();
	}
}

/// Called when the "eBook" template radion button has been clicked on.
void CNewProjectDlg::OnClickedTemplateEbook()
{
	CSize *sizePointer;
	int index;

	// The "eBook" template was designed to use a tablet screen size.
	// Find the tablet preset in the combo box and select it.
	for (index = 0; index < fScreenSizeComboBox.GetCount(); index++)
	{
		sizePointer = (CSize*)fScreenSizeComboBox.GetItemData(index);
		if (sizePointer == &fTabletScreenSize)
		{
			fScreenSizeComboBox.SetCurSel(index);
			OnScreenSizeComboBoxSelectionChanged();
			return;
		}
	}
}

/// Validates the dialog's settings, creates a new project, and then closes this dialog.
void CNewProjectDlg::OnOK()
{
	CString applicationName;
	CString projectPath;
	CString templatePath;
	CStringA templateName;
	CStringA defaultOrientationName;
	CString stringBuffer;
	WinString stringConverter;
	int screenWidth;
	int screenHeight;

	// Fetch and validate field values.
	fAppNameEditBox.GetWindowText(applicationName);
	applicationName.Trim();
	if (applicationName.IsEmpty())
	{
		DisplayWarningMessage(IDS_BUILD_APP_NAME_NOT_PROVIDED);
		fAppNameEditBox.SetFocus();
		return;
	}
	CString invalidPathCharacters(_T("<>:\"/\\|?*"));
	if (applicationName.FindOneOf(invalidPathCharacters) >= 0)
	{
		CString message;
		CString substring;
		for (int index = 0; index < invalidPathCharacters.GetLength(); index++)
		{
			if (substring.GetLength() > 0)
			{
				substring += _T(" ");
			}
			substring += invalidPathCharacters.GetAt(index);
		}
		message.Format(IDS_PROJECT_APPLICATION_NAME_INVALID, substring);
		DisplayWarningMessage(message);
		fAppNameEditBox.SetFocus();
		return;
	}

	// Fetch and validate the project path.
	UpdateProjectLocationField();
	fProjectLocationEditBox.GetWindowText(projectPath);
	projectPath.Trim();
	if (projectPath.IsEmpty())
	{
		DisplayWarningMessage(IDS_PROJECT_PATH_NOT_PROVIDED);
		fProjectLocationEditBox.SetFocus();
		return;
	}

	// Fetch selected template.
	if (((CButton*)GetDlgItem(IDC_TEMPLATE_TAB_BAR))->GetCheck() == BST_CHECKED)
	{
		templateName = "app";
	}
	else if (((CButton*)GetDlgItem(IDC_TEMPLATE_GAME))->GetCheck() == BST_CHECKED)
	{
		templateName = "game";
	}
	else if (((CButton*)GetDlgItem(IDC_TEMPLATE_EBOOK))->GetCheck() == BST_CHECKED)
	{
		templateName = "ebook";
	}
	else
	{
		templateName = "blank";
	}
	templatePath = ((CSimulatorApp*)AfxGetApp())->GetResourceDir() + _T("\\homescreen\\templates");

	// Fetch and validate screen width.
	screenWidth = 0;
	fScreenWidthEditBox.GetWindowText(stringBuffer);
	stringBuffer.Trim();
	if (stringBuffer.GetLength() > 0)
	{
		screenWidth = _ttoi(stringBuffer);
	}
	if (screenWidth < 1)
	{
		DisplayWarningMessage(IDS_PROJECT_SCREEN_WIDTH_IS_INVALID);
		fScreenWidthEditBox.SetFocus();
		return;
	}

	// Fetch and validate screen height.
	screenHeight = 0;
	fScreenHeightEditBox.GetWindowText(stringBuffer);
	stringBuffer.Trim();
	if (stringBuffer.GetLength() > 0)
	{
		screenHeight = _ttoi(stringBuffer);
	}
	if (screenHeight < 1)
	{
		DisplayWarningMessage(IDS_PROJECT_SCREEN_HEIGHT_IS_INVALID);
		fScreenHeightEditBox.SetFocus();
		return;
	}

	// Fetch default orientation selection.
	if (((CButton*)GetDlgItem(IDC_ORIENTATION_SIDEWAYS))->GetCheck() == BST_CHECKED)
	{
		defaultOrientationName = "landscapeRight";
	}
	else
	{
		defaultOrientationName = "portrait";
	}

	// If the given project path does not exist, then create the project directory.
	// If the directory does exist, then make sure an existing project is not already there.
	if (::PathIsDirectory(projectPath) == FALSE)
	{
		// The given path does not exist. Attempt to create it. If we are unable to create the
		// directory, then that may mean the path is invalid or we do not have permission to
		// create it at that location.
		int result = ::SHCreateDirectoryEx(NULL, projectPath, NULL);
		if (result != ERROR_SUCCESS)
		{
			DisplayWarningMessage(IDS_PROJECT_PATH_CREATION_FAILED);
			fProjectLocationEditBox.SetFocus();
			return;
		}
	}
	else
	{
		// The path already exists.
		// Do not continue if a Corona project already exists at that location.
		stringBuffer = projectPath + _T("\\main.lua");
		if (((CSimulatorApp*)AfxGetApp())->CheckPathExists(stringBuffer))
		{
			DisplayWarningMessage(IDS_PROJECT_PATH_ALREADY_TAKEN);
			fProjectLocationEditBox.SetFocus();
			return;
		}
	}

	// This dialog's field values are valid.
	// Have a Lua script create the Corona Project.
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	Rtt::LuaContext::RegisterModuleLoader( L, "lfs", luaopen_lfs );
	CString newProjectLuaFilePath = ((CSimulatorApp*)AfxGetApp())->GetResourceDir();
	newProjectLuaFilePath += _T("\\homescreen\\newproject.lua");
	stringConverter.SetTCHAR(newProjectLuaFilePath);
	int luaStatus = luaL_loadfile(L, stringConverter.GetUTF8());
	lua_createtable(L, 0, 6);
	{
		lua_pushboolean( L, 1 );
		lua_setfield( L, -2, "isSimulator" );

		lua_pushstring( L, templateName );
		lua_setfield( L, -2, "template" );

		lua_pushinteger( L, screenWidth );
		lua_setfield( L, -2, "width" );

		lua_pushinteger( L, screenHeight );
		lua_setfield( L, -2, "height" );

		lua_pushstring( L, defaultOrientationName );
		lua_setfield( L, -2, "orientation" );

		stringConverter.SetTCHAR(projectPath);
		lua_pushstring( L, stringConverter.GetUTF8() );
		lua_setfield( L, -2, "savePath" );

		stringConverter.SetTCHAR(templatePath);
		lua_pushstring( L, stringConverter.GetUTF8() );
		lua_setfield( L, -2, "templateBaseDir" );
	}
	luaStatus = Rtt::LuaContext::DoCall(L, 1, 0);
	lua_close(L);
	if (luaStatus != 0)
	{
		DisplayWarningMessage(IDS_PROJECT_FILES_CREATION_FAILED);
		return;
	}

	// New project was successfully created. Store the project path and then close this dialog.
	fNewProjectPath = projectPath;
	CDialog::OnOK();
}

#pragma endregion


#pragma region Public Functions
/// Gets the absolute path to the new Corona project folder that the user created.
/// This function should only be called after the user presses the OK button on this dialog
/// @return Returns an absolute path to the new Corona project folder.
///         Returns an empty string if the user canceled out of this dialog.
CString CNewProjectDlg::GetNewProjectPath()
{
	return fNewProjectPath;
}

#pragma endregion


#pragma region Private Functions
/// Updates the read-only project location text box with an absolute path that includes
/// the selected root project folder with the application name appended to it.
void CNewProjectDlg::UpdateProjectLocationField()
{
	CString applicationName;
	CString path;

	// Build a project path using the current application name.
	fAppNameEditBox.GetWindowText(applicationName);
	applicationName.Trim();
	path = fNewProjectPath;
	if (path.GetLength() <= 0)
	{
		path = _T("C:");
	}
	if (applicationName.GetLength() > 0)
	{
		path += _T("\\") + applicationName;
	}

	// Update the project location field.
	fProjectLocationEditBox.SetWindowText(path);
}

/// Display a warning message box with the given string resource ID.
/// @param messageId A unique integer ID to a string in the string table to be used for the message.
void CNewProjectDlg::DisplayWarningMessage(UINT messageId)
{
	CString message;

	message.LoadString(messageId);
	DisplayWarningMessage(message);
}

/// Display a warning message box with the given string.
/// @param message The string to be displayed in the message box.
void CNewProjectDlg::DisplayWarningMessage(CString &message)
{
	CString title;

	title.LoadString(IDS_WARNING);
	MessageBox(message, title, MB_OK | MB_ICONWARNING);
}

#pragma endregion
