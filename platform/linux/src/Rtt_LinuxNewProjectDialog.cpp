#include "Rtt_LinuxNewProjectDialog.h"
#include <string.h>
#include "Rtt_LuaContext.h"
#include "Rtt_LinuxContext.h"
#include "Rtt_LinuxUtils.h"
#include "Core/Rtt_FileSystem.h"

// export for LUA
Rtt_EXPORT int luaopen_lfs(lua_State *L);

using namespace std;

namespace Rtt
{
	LinuxNewProjectDialog::LinuxNewProjectDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style):
		wxDialog(parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE),
		fProjectName(""),
		fTemplateName(""),
		fScreenWidth(320),
		fScreenHeight(480),
		fOrientationIndex(""),
		fProjectPath(""),
		fProjectSavePath(""),
		fResourcePath("")

	{
		const wxString cboScreenSizePresetChoices[] =
		{
			wxT("Phone Preset"),
			wxT("Tablet Preset"),
			wxT("Custom"),
		};

		SetSize(wxSize(600, 425));
		txtApplicationName = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
		txtProjectFolder = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
		btnBrowse = new wxButton(this, wxID_OPEN, wxT("&Browse..."));
		rProjectOption1 = new wxRadioButton(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
		rProjectOption2 = new wxRadioButton(this, wxID_ANY, wxEmptyString);
		rProjectOption3 = new wxRadioButton(this, wxID_ANY, wxEmptyString);
		rProjectOption4 = new wxRadioButton(this, wxID_ANY, wxEmptyString);
		cboScreenSizePreset = new wxComboBox(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 3, cboScreenSizePresetChoices, wxCB_DROPDOWN);
		txtWidth = new wxTextCtrl(this, wxID_ANY, to_string(fScreenWidth));
		txtHeight = new wxTextCtrl(this, wxID_ANY, to_string(fScreenHeight));
		rUpright = new wxRadioButton(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
		rSideways = new wxRadioButton(this, wxID_ANY, wxEmptyString);
		btnOK = new wxButton(this, wxID_OK, wxT("OK"));
		btnCancel = new wxButton(this, wxID_CANCEL, wxT("Cancel"));

		SetProperties();
		DoLayout();
	}

	void LinuxNewProjectDialog::SetProperties()
	{
		SetTitle(wxT("New Project"));
		SetSize(wxSize(600, 425));
		SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
		txtApplicationName->SetValue("Untitled");
		txtProjectFolder->Enable(false);
		cboScreenSizePreset->SetSelection(0);
		txtWidth->Enable(false);
		txtHeight->Enable(false);
		btnOK->SetDefault();
		SetProjectPath();
		SetResourcePath();
	}

	void LinuxNewProjectDialog::DoLayout()
	{
		wxBoxSizer *dialogLayout = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *dialogTop = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *dialogMiddle = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *dialogBottom = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *dialogButtons = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *groupBoxRight = new wxBoxSizer(wxVERTICAL);
		wxStaticBoxSizer *groupBoxLeft = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Project Template")), wxVERTICAL);
		wxStaticBoxSizer *groupBoxRightBottom = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Default Orientation")), wxHORIZONTAL);
		wxStaticBoxSizer *groupBoxRightTop = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Upright Screen Size")), wxVERTICAL);
		wxBoxSizer *topColumn1 = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *topColumn2 = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *topColumn3 = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *topRow1 = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *topRow2 = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *topRow3 = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *topRow4 = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *middleRightTopColumn1 = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *middleRightTopColumn2 = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *middleRightBottomColumn1 = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *middleRightBottomColumn2 = new wxBoxSizer(wxHORIZONTAL);
		wxStaticText *appNameText = new wxStaticText(this, wxID_ANY, wxT("Application Name : "));
		wxStaticText *projectFolderText = new wxStaticText(this, wxID_ANY, wxT("Project Folder : "));
		wxStaticText *blankProjectText = new wxStaticText(this, wxID_ANY, wxT("Blank\nCreates a blank project."));
		wxStaticText *tabBarProjecText = new wxStaticText(this, wxID_ANY, wxT("Tab Bar Application\nUsing a tab bar for navigation."));
		wxStaticText *physicsProjectText = new wxStaticText(this, wxID_ANY, wxT("Physics Based Game\nUsing the physics and composer libraries."));
		wxStaticText *eBookText = new wxStaticText(this, wxID_ANY, wxT("eBook\nUsing the composer library."));
		wxStaticText *widthText = new wxStaticText(this, wxID_ANY, wxT("Width : "), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
		wxStaticText *heightText = new wxStaticText(this, wxID_ANY, wxT("Height : "), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
		wxStaticText *orientationText1 = new wxStaticText(this, wxID_ANY, wxT("Upright"));
		wxStaticText *orientationText2 = new wxStaticText(this, wxID_ANY, wxT("Sideways"));
		wxStaticLine *staticLineSeparator = new wxStaticLine(this, wxID_ANY);

		// set fonts
		appNameText->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
		projectFolderText->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

		// add to top columns
		topColumn1->Add(appNameText, 0, wxALIGN_RIGHT | wxBOTTOM | wxTOP, 6);
		topColumn1->Add(projectFolderText, 0, wxALIGN_RIGHT | wxBOTTOM | wxTOP, 6);
		topColumn2->Add(txtApplicationName, 0, wxEXPAND, 0);
		topColumn2->Add(txtProjectFolder, 0, wxEXPAND, 0);
		topColumn3->Add(20, 16, 0, wxBOTTOM | wxTOP, 6);
		topColumn3->Add(btnBrowse, 0, 0, 0);

		// add to top rows
		topRow1->Add(rProjectOption1, 0, 0, 0);
		topRow1->Add(blankProjectText, 0, 0, 0);
		topRow2->Add(rProjectOption2, 0, 0, 0);
		topRow2->Add(tabBarProjecText, 0, 0, 0);
		topRow3->Add(rProjectOption3, 0, 0, 0);
		topRow3->Add(physicsProjectText, 0, 0, 0);
		topRow4->Add(rProjectOption4, 0, 0, 0);
		topRow4->Add(eBookText, 0, 0, 0);
		middleRightTopColumn1->Add(widthText, 1, wxBOTTOM | wxTOP, 7);
		middleRightTopColumn1->Add(txtWidth, 0, 0, 0);
		middleRightTopColumn2->Add(heightText, 1, wxBOTTOM | wxTOP, 7);
		middleRightTopColumn2->Add(txtHeight, 0, 0, 0);
		middleRightBottomColumn1->Add(rUpright, 0, 0, 0);
		middleRightBottomColumn1->Add(orientationText1, 0, 0, 0);
		middleRightBottomColumn2->Add(rSideways, 0, 0, 0);
		middleRightBottomColumn2->Add(orientationText2, 0, 0, 0);

		// add to groupboxes
		groupBoxLeft->Add(topRow1, 1, wxEXPAND | wxRIGHT | wxTOP, 7);
		groupBoxLeft->Add(topRow2, 1, wxEXPAND, 0);
		groupBoxLeft->Add(topRow3, 1, wxEXPAND, 0);
		groupBoxLeft->Add(topRow4, 1, wxEXPAND, 0);
		groupBoxRight->Add(groupBoxRightTop, 1, wxBOTTOM | wxEXPAND, 7);
		groupBoxRight->Add(groupBoxRightBottom, 0, wxEXPAND, 0);
		groupBoxRight->Add(20, 20, 1, wxEXPAND, 0);
		groupBoxRightTop->Add(cboScreenSizePreset, 0, wxALL | wxEXPAND, 7);
		groupBoxRightTop->Add(middleRightTopColumn1, 0, wxEXPAND | wxRIGHT, 7);
		groupBoxRightTop->Add(middleRightTopColumn2, 0, wxBOTTOM | wxEXPAND | wxRIGHT, 7);
		groupBoxRightBottom->Add(middleRightBottomColumn1, 1, wxBOTTOM | wxEXPAND | wxTOP, 7);
		groupBoxRightBottom->Add(middleRightBottomColumn2, 1, wxBOTTOM | wxEXPAND | wxTOP, 7);

		// add to dialog buttons
		dialogButtons->Add(btnOK, 0, wxRIGHT, 5);
		dialogButtons->Add(btnCancel, 0, 0, 0);

		// add to dialog layouts
		dialogTop->Add(topColumn1, 0, wxLEFT | wxRIGHT, 7);
		dialogTop->Add(topColumn2, 1, wxEXPAND, 0);
		dialogTop->Add(topColumn3, 0, wxEXPAND | wxLEFT | wxRIGHT, 7);
		dialogMiddle->Add(groupBoxLeft, 1, wxEXPAND | wxLEFT, 7);
		dialogMiddle->Add(groupBoxRight, 1, wxEXPAND | wxLEFT | wxRIGHT, 7);
		dialogBottom->Add(staticLineSeparator, 0, wxEXPAND | wxLEFT | wxRIGHT, 7);
		dialogBottom->Add(dialogButtons, 1, wxALIGN_RIGHT | wxALL, 7);
		dialogLayout->Add(dialogTop, 0, wxBOTTOM | wxEXPAND | wxTOP, 8);
		dialogLayout->Add(dialogMiddle, 1, wxBOTTOM | wxEXPAND | wxTOP, 7);
		dialogLayout->Add(dialogBottom, 0, wxEXPAND, 0);

		SetSizer(dialogLayout);
		Layout();
	}

	BEGIN_EVENT_TABLE(LinuxNewProjectDialog, wxDialog)
		EVT_COMBOBOX(wxID_ANY, LinuxNewProjectDialog::OnChange)
		EVT_BUTTON(wxID_OPEN, LinuxNewProjectDialog::OnProjectFolderBrowse)
		EVT_BUTTON(wxID_OK, LinuxNewProjectDialog::OnOKClicked)
		EVT_BUTTON(wxID_CANCEL, LinuxNewProjectDialog::OnCancelClicked)
	END_EVENT_TABLE();

	void LinuxNewProjectDialog::OnProjectFolderBrowse(wxCommandEvent &event)
	{
		event.Skip();
		wxDirDialog openDirDialog(this, _("Choose Project Directory"), fProjectPath, 0, wxDefaultPosition);

		if (openDirDialog.ShowModal() == wxID_OK)
		{
			fProjectPath = string(openDirDialog.GetPath());
			txtProjectFolder->SetValue(fProjectPath);
		}
	}

	void LinuxNewProjectDialog::OnChange(wxCommandEvent &event)
	{
		event.Skip();
		wxString strPreset = cboScreenSizePreset->GetValue();

		if (strPreset.IsSameAs("Phone Preset"))
		{
			fScreenWidth = 320;
			fScreenHeight = 480;
			txtWidth->Enable(false);
			txtHeight->Enable(false);
		}
		else if (strPreset.IsSameAs("Tablet Preset"))
		{
			fScreenWidth = 768;
			fScreenHeight = 1024;
			txtWidth->Enable(false);
			txtHeight->Enable(false);
		}
		else if (strPreset.IsSameAs("Custom"))
		{
			txtWidth->Enable(true);
			txtHeight->Enable(true);
		}

		txtWidth->SetValue(to_string(fScreenWidth));
		txtHeight->SetValue(to_string(fScreenHeight));
	}

	void LinuxNewProjectDialog::OnOKClicked(wxCommandEvent &event)
	{
		bool bDialogClean = true;

		// Determine the application template we are using
		if (rProjectOption1->GetValue() == true)
		{
			fTemplateName = "blank";
		}

		if (rProjectOption2->GetValue() == true)
		{
			fTemplateName = "app";
		}

		if (rProjectOption3->GetValue() == true)
		{
			fTemplateName = "game";
		}

		if (rProjectOption4->GetValue() == true)
		{
			fTemplateName = "ebook";
		}

		fScreenWidth = wxAtoi(txtWidth->GetValue());
		fScreenHeight = wxAtoi(txtHeight->GetValue());
		fOrientationIndex = rUpright->GetValue() ? "portait" : "landscapeRight";

		if (fScreenWidth == 0 || fScreenHeight == 0)
		{
			wxMessageBox(wxT("Height and Width values must be numeric and larger than 0"), wxT("Screen Dimension Error"), wxICON_INFORMATION);
			bDialogClean = false;
		}

		// TODO: Make sure all variables are sane values before running project creation process
		string fProjectSavePath(txtProjectFolder->GetValue().ToStdString());
		fProjectName = txtApplicationName->GetValue().ToStdString();
		fProjectFolder = txtProjectFolder->GetValue().ToStdString();
		fProjectSavePath.append("/").append(fProjectName);

		// check if project folder already exists and that the height and width are numbers
		if (Rtt_IsDirectory(fProjectSavePath.c_str()))
		{
			wxMessageBox(wxT("Project of that name already exists."), wxT("Duplicate Project Name"), wxICON_INFORMATION);
			bDialogClean = false;
		}

		if (bDialogClean)
		{
			CreateProject(fProjectSavePath);
			EndModal(wxID_OK);
		}
	}

	void LinuxNewProjectDialog::OnCancelClicked(wxCommandEvent &event)
	{
		EndModal(wxID_CLOSE);
	}

	void LinuxNewProjectDialog::SetProjectPath()
	{
		const char* homedir = GetHomePath();
		fProjectPath = string(homedir);
		fProjectPath.append("/Documents/Solar2D Projects");
		txtProjectFolder->SetValue(fProjectPath);
	}

	void LinuxNewProjectDialog::SetResourcePath()
	{
		fResourcePath = string(GetStartupPath(NULL));
		fResourcePath.append("/Resources");
	}

	void LinuxNewProjectDialog::CreateProject(string projectFolder)
	{
		string fNewProjectLuaScript(fResourcePath);
		fNewProjectLuaScript.append("/homescreen/newproject.lua");

		string fTemplatesDir(fResourcePath);
		fTemplatesDir.append("/homescreen/templates");

		if (!Rtt_IsDirectory(projectFolder.c_str()))
		{
			Rtt_MakeDirectory(projectFolder.c_str());
		}

		lua_State *L = luaL_newstate();
		luaL_openlibs(L);
		Rtt::LuaContext::RegisterModuleLoader(L, "lfs", luaopen_lfs);

		const char *script = fNewProjectLuaScript.c_str();
		int status = luaL_loadfile(L, script);
		Rtt_ASSERT(0 == status);

		lua_createtable(L, 0, 6);
		{
			lua_pushboolean(L, true);
			lua_setfield(L, -2, "isSimulator");

			lua_pushstring(L, fTemplateName.c_str());
			lua_setfield(L, -2, "template");

			lua_pushinteger(L, fScreenWidth);
			lua_setfield(L, -2, "width");

			lua_pushinteger(L, fScreenHeight);
			lua_setfield(L, -2, "height");

			lua_pushstring(L, 0 == fOrientationIndex ? "portrait" : "landscapeRight");
			lua_setfield(L, -2, "orientation");

			lua_pushstring(L, projectFolder.c_str());
			lua_setfield(L, -2, "savePath");

			lua_pushstring(L, fTemplatesDir.c_str());
			lua_setfield(L, -2, "templateBaseDir");
		}

		status = Rtt::LuaContext::DoCall(L, 1, 0);
		Rtt_ASSERT(0 == status);
		lua_close(L);
	}
}
