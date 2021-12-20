#include "Core/Rtt_Build.h"
#include "Core/Rtt_Time.h"
#include "Rtt_Runtime.h"
#include "Rtt_LuaContext.h"
#include "Core/Rtt_Types.h"
#include "Rtt_LinuxContext.h"
#include "Rtt_LinuxPlatform.h"
#include "Rtt_LinuxRuntimeDelegate.h"
#include "Rtt_LuaFile.h"
#include "Core/Rtt_FileSystem.h"
#include "Rtt_Archive.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayDefaults.h"
#include "Rtt_KeyName.h"
#include "Rtt_Freetype.h"
#include "Rtt_LuaLibSimulator.h"
#include "Rtt_LinuxSimulatorView.h"

#include <pwd.h>
#include <libgen.h>
#include <string.h>

#include "Rtt_LinuxContextNewProject.h"

Rtt_EXPORT int luaopen_lfs (lua_State *L);

using namespace Rtt;

namespace Rtt
{
	NewProjectDialog::NewProjectDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style):
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
		SetSize(wxSize(511, 425));
		txtApplicationName = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
		txtProjectFolder = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
		btnBrowse = new wxButton(this, wxID_OPEN, wxT("&Browse..."));
		rProjectOption1 = new wxRadioButton(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
		rProjectOption2 = new wxRadioButton(this, wxID_ANY, wxEmptyString);
		rProjectOption3 = new wxRadioButton(this, wxID_ANY, wxEmptyString);
		rProjectOption4 = new wxRadioButton(this, wxID_ANY, wxEmptyString);
		const wxString cboScreenSizePreset_choices[] = {
			wxT("Phone Preset"),
			wxT("Tablet Preset"),
			wxT("Custom"),
		};
		cboScreenSizePreset = new wxComboBox(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 3, cboScreenSizePreset_choices, wxCB_DROPDOWN);
		txtWidth = new wxTextCtrl(this, wxID_ANY, std::to_string(fScreenWidth));
		txtHeight = new wxTextCtrl(this, wxID_ANY, std::to_string(fScreenHeight));
		rUpright = new wxRadioButton(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
		rSideways = new wxRadioButton(this, wxID_ANY, wxEmptyString);
		btnOK = new wxButton(this, wxID_OK, wxT("OK"));
		btnCancel = new wxButton(this, wxID_CANCEL, wxT("Cancel"));

		set_properties();
		do_layout();
	}


	void NewProjectDialog::set_properties()
	{
		SetTitle(wxT("New Project"));
		SetSize(wxSize(511, 425));
		SetFont(wxFont(8, wxDEFAULT, wxNORMAL, wxNORMAL, 0, wxT("")));
		txtApplicationName->SetValue("Untitled");
		//txtProjectFolder->Enable(true);
		txtProjectFolder->Enable(false);
		cboScreenSizePreset->SetSelection(0);
		txtWidth->Enable(false);
		txtHeight->Enable(false);
		btnOK->SetDefault();
		this->SetProjectPath();
		this->SetResourcePath();
	}


	void NewProjectDialog::do_layout()
	{
		wxBoxSizer* dialog_layout = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer* dialog_bottom = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer* dialog_buttons = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* dialog_middle = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* right_groupboxes = new wxBoxSizer(wxVERTICAL);
		wxStaticBoxSizer* right_groupbox_bottom = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Default Orientation")), wxHORIZONTAL);
		wxBoxSizer* middlerightbottomcolumn2 = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* middlerightbottomcolumn1 = new wxBoxSizer(wxHORIZONTAL);
		wxStaticBoxSizer* right_groupbox_top = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Upright Screen Size")), wxVERTICAL);
		wxBoxSizer* middlerighttopcolumn2 = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* middlerighttopcolumn1 = new wxBoxSizer(wxHORIZONTAL);
		wxStaticBoxSizer* left_groupbox = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Project Template")), wxVERTICAL);
		wxBoxSizer* ptrow4 = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* ptrow3 = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* ptrow2 = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* ptrow1 = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* dialog_top = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* topcolumn3 = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer* topcolumn2 = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer* topcolumn1 = new wxBoxSizer(wxVERTICAL);
		wxStaticText* lblApplicationName = new wxStaticText(this, wxID_ANY, wxT("Application Name : "));
		lblApplicationName->SetFont(wxFont(8, wxDEFAULT, wxNORMAL, wxNORMAL, 0, wxT("")));
		topcolumn1->Add(lblApplicationName, 0, wxALIGN_RIGHT|wxBOTTOM|wxTOP, 6);
		wxStaticText* lblProjectFolder = new wxStaticText(this, wxID_ANY, wxT("Project Folder : "));
		lblProjectFolder->SetFont(wxFont(8, wxDEFAULT, wxNORMAL, wxNORMAL, 0, wxT("")));
		topcolumn1->Add(lblProjectFolder, 0, wxALIGN_RIGHT|wxBOTTOM|wxTOP, 6);
		dialog_top->Add(topcolumn1, 0, wxLEFT|wxRIGHT, 7);
		topcolumn2->Add(txtApplicationName, 0, wxEXPAND, 0);
		topcolumn2->Add(txtProjectFolder, 0, wxEXPAND, 0);
		dialog_top->Add(topcolumn2, 1, wxEXPAND, 0);
		topcolumn3->Add(20, 16, 0, wxBOTTOM|wxTOP, 6);
		topcolumn3->Add(btnBrowse, 0, 0, 0);
		dialog_top->Add(topcolumn3, 0, wxEXPAND|wxLEFT|wxRIGHT, 7);
		dialog_layout->Add(dialog_top, 0, wxBOTTOM|wxEXPAND|wxTOP, 8);
		ptrow1->Add(rProjectOption1, 0, 0, 0);
		wxStaticText* label_1 = new wxStaticText(this, wxID_ANY, wxT("Blank\nCreates a project folder with an empty \"main.lua\""));
		ptrow1->Add(label_1, 0, 0, 0);
		left_groupbox->Add(ptrow1, 1, wxEXPAND|wxRIGHT|wxTOP, 7);
		ptrow2->Add(rProjectOption2, 0, 0, 0);
		wxStaticText* label_2 = new wxStaticText(this, wxID_ANY, wxT("Tab Bar Application\nMultiscreen application using a Tab Bar for"));
		ptrow2->Add(label_2, 0, 0, 0);
		left_groupbox->Add(ptrow2, 1, wxEXPAND, 0);
		ptrow3->Add(rProjectOption3, 0, 0, 0);
		wxStaticText* label_3 = new wxStaticText(this, wxID_ANY, wxT("Physics Based Game\nApplication using the physics and composer"));
		ptrow3->Add(label_3, 0, 0, 0);
		left_groupbox->Add(ptrow3, 1, wxEXPAND, 0);
		ptrow4->Add(rProjectOption4, 0, 0, 0);
		wxStaticText* label_4 = new wxStaticText(this, wxID_ANY, wxT("eBook\nMulti-page interface using the composer"));
		ptrow4->Add(label_4, 0, 0, 0);
		left_groupbox->Add(ptrow4, 1, wxEXPAND, 0);
		dialog_middle->Add(left_groupbox, 1, wxEXPAND|wxLEFT, 7);
		right_groupbox_top->Add(cboScreenSizePreset, 0, wxALL|wxEXPAND, 7);
		wxStaticText* lblWidth = new wxStaticText(this, wxID_ANY, wxT("Width : "), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
		middlerighttopcolumn1->Add(lblWidth, 1, wxBOTTOM|wxTOP, 7);
		middlerighttopcolumn1->Add(txtWidth, 0, 0, 0);
		right_groupbox_top->Add(middlerighttopcolumn1, 0, wxEXPAND|wxRIGHT, 7);
		wxStaticText* lblHeight = new wxStaticText(this, wxID_ANY, wxT("Height : "), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
		middlerighttopcolumn2->Add(lblHeight, 1, wxBOTTOM|wxTOP, 7);
		middlerighttopcolumn2->Add(txtHeight, 0, 0, 0);
		right_groupbox_top->Add(middlerighttopcolumn2, 0, wxBOTTOM|wxEXPAND|wxRIGHT, 7);
		right_groupboxes->Add(right_groupbox_top, 1, wxBOTTOM|wxEXPAND, 7);
		middlerightbottomcolumn1->Add(rUpright, 0, 0, 0);
		wxStaticText* label_5 = new wxStaticText(this, wxID_ANY, wxT("Upright"));
		middlerightbottomcolumn1->Add(label_5, 0, 0, 0);
		right_groupbox_bottom->Add(middlerightbottomcolumn1, 1, wxBOTTOM|wxEXPAND|wxTOP, 7);
		middlerightbottomcolumn2->Add(rSideways, 0, 0, 0);
		wxStaticText* label_6 = new wxStaticText(this, wxID_ANY, wxT("Sideways"));
		middlerightbottomcolumn2->Add(label_6, 0, 0, 0);
		right_groupbox_bottom->Add(middlerightbottomcolumn2, 1, wxBOTTOM|wxEXPAND|wxTOP, 7);
		right_groupboxes->Add(right_groupbox_bottom, 0, wxEXPAND, 0);
		right_groupboxes->Add(20, 20, 1, wxEXPAND, 0);
		dialog_middle->Add(right_groupboxes, 1, wxEXPAND|wxLEFT|wxRIGHT, 7);
		dialog_layout->Add(dialog_middle, 1, wxBOTTOM|wxEXPAND|wxTOP, 7);
		wxStaticLine* static_line_1 = new wxStaticLine(this, wxID_ANY);
		dialog_bottom->Add(static_line_1, 0, wxEXPAND|wxLEFT|wxRIGHT, 7);
		dialog_buttons->Add(btnOK, 0, wxRIGHT, 5);
		dialog_buttons->Add(btnCancel, 0, 0, 0);
		dialog_bottom->Add(dialog_buttons, 1, wxALIGN_RIGHT|wxALL, 7);
		dialog_layout->Add(dialog_bottom, 0, wxEXPAND, 0);
		SetSizer(dialog_layout);
		Layout();
	}
	
	BEGIN_EVENT_TABLE(NewProjectDialog, wxDialog)
		EVT_COMBOBOX(wxID_ANY, NewProjectDialog::onChange)
		EVT_BUTTON(wxID_OPEN, NewProjectDialog::OnProjectFolderBrowse)
		EVT_BUTTON(wxID_OK, NewProjectDialog::onbtnOKClicked)
		EVT_BUTTON(wxID_CANCEL, NewProjectDialog::onbtnCancelClicked)
	END_EVENT_TABLE();

	void NewProjectDialog::OnProjectFolderBrowse(wxCommandEvent &event)
	{
		
		event.Skip();
		
		wxDirDialog openDirDialog( this, _("Choose Project Directory"), fProjectPath, 0, wxDefaultPosition);

		if ( openDirDialog.ShowModal() ==  wxID_OK )
		{ 
			
			fProjectPath = std::string(openDirDialog.GetPath());
			txtProjectFolder->SetValue(fProjectPath);
			
		} else if (openDirDialog.ShowModal() == wxID_CANCEL){
			
			return;
			
		}
		
	}



	void NewProjectDialog::onChange(wxCommandEvent &event)
	{
		
		event.Skip();
		
		wxString strPreset = cboScreenSizePreset->GetValue();
		
		if ( strPreset == "Phone Preset"){
			
			fScreenWidth = 320;
			fScreenHeight = 480;
			txtWidth->Enable(false);
			txtHeight->Enable(false);
			wxLogDebug("Preset is Phone Preset" );
			
		}
		
		if ( strPreset == "Tablet Preset"){
			
			fScreenWidth = 768;
			fScreenHeight = 1024;
			txtWidth->Enable(false);
			txtHeight->Enable(false);
			wxLogDebug("Preset is Tablet Preset" );
			
		}
		
		if ( strPreset == "Custom"){
			
			txtWidth->Enable(true);
			txtHeight->Enable(true);
			wxLogDebug("Preset is Custom Preset" );
			
		}
		
		txtWidth->SetValue( std::to_string(fScreenWidth));
		txtHeight->SetValue( std::to_string(fScreenHeight));
		
	}

	void NewProjectDialog::onbtnOKClicked(wxCommandEvent &event)
	{
		
		wxLogDebug("fProjectName = %s " , txtApplicationName->GetValue().ToStdString() );
		
		bool bDialogClean = true;
		
		// Determine the application template we are using
		
		if ( rProjectOption1->GetValue() == true ){
			
			fTemplateName = "blank";
			//wxLogDebug(wxT("Project = blank\n" ));
			
		}
		
		if ( rProjectOption2->GetValue()  == true ){
			
			fTemplateName = "app";
			//wxLogDebug(wxT("Project = app\n" ));
			
		}
		
		if ( rProjectOption3->GetValue()  == true ){
			
			fTemplateName = "game";
			//wxLogDebug(wxT("Project = game\n" ));
			
		}
		
		if ( rProjectOption4->GetValue()  == true ){
			
			fTemplateName = "ebook";
			//wxLogDebug(wxT("Project = ebook\n" ));
			
		}
		
		fScreenWidth = wxAtoi(txtWidth->GetValue());
		fScreenHeight = wxAtoi(txtHeight->GetValue());
		
		if ( fScreenWidth == 0 || fScreenHeight == 0) {
			
			wxMessageBox( wxT("Height and Width values must be numeric and larger that 0"), wxT("Screen Dimension Errors"), wxICON_INFORMATION);
			bDialogClean = false;
			
		}
		
		wxLogDebug("dimension width x height = %s x %s" , std::to_string(fScreenWidth), std::to_string(fScreenHeight) );

		if ( rUpright->GetValue()  == true ){
			
			fOrientationIndex = "portait";
			//wxLogDebug(wxT("Orientation = portrait\n" ));
			
		}		
		
		if ( rSideways->GetValue()  == true ){
			
			fOrientationIndex = "landscapeRight";
			//wxLogDebug(wxT("Orientation = landscapeRight\n" ));
			
		}		
		
		//TODO: Make sure all variables are sane values before running project creation process
		
		std::string fProjectSavePath = txtProjectFolder->GetValue().ToStdString();
	    fProjectName = txtApplicationName->GetValue().ToStdString();
		fProjectSavePath += LUA_DIRSEP;
		fProjectSavePath += fProjectName;

		wxLogDebug("fProjectSavePath = %s " , fProjectSavePath );
		
		//check if project folder already exists and that the height and width are numbers
		if (Rtt_IsDirectory(fProjectSavePath.c_str()) == true)
		{
			
			wxMessageBox( wxT("Project of that name already exists."), wxT("Duplicate Project Name"), wxICON_INFORMATION);
			bDialogClean = false;
		
		} 
		
		if ( bDialogClean == true ){
			
			this->CreateProject(fProjectSavePath);
			EndModal(wxID_OK);
			
		}
		
	}

	void NewProjectDialog::onbtnCancelClicked(wxCommandEvent &event)
	{

		EndModal(wxID_CLOSE);

	}
	
	void NewProjectDialog::SetProjectPath()
	{
		
		struct passwd* pw = getpwuid(getuid());
		const char* homedir = pw->pw_dir;
		fProjectPath = std::string(homedir);
		fProjectPath += LUA_DIRSEP;
		fProjectPath += "Documents";
		fProjectPath += LUA_DIRSEP;
		fProjectPath += "Corona Projects";
		
		txtProjectFolder->SetValue(fProjectPath);
		
	}
	void NewProjectDialog::SetResourcePath()
	{
		
		static char buf[ PATH_MAX + 1];
		ssize_t count = readlink("/proc/self/exe", buf, PATH_MAX);
		const char *appPath;
		
		if (count != -1) {
		
			appPath = dirname(buf);
		
		}

		fResourcePath = std::string(appPath);
		fResourcePath += LUA_DIRSEP;
		fResourcePath += "Resources";
		wxLogDebug("appPath = %s\n", std::string(appPath) );
		
	}
	
	void NewProjectDialog::CreateProject(std::string projectFolder)
	{
		
		std::string fNewProjectLuaScript = fResourcePath;
		fNewProjectLuaScript += LUA_DIRSEP;
		fNewProjectLuaScript += "homescreen";
		fNewProjectLuaScript += LUA_DIRSEP;
		fNewProjectLuaScript += "newproject.lua";
		wxLogDebug("fNewProjectLuaScript = %s\n", std::string(fNewProjectLuaScript) );
		
		std::string fTemplatesDir = fResourcePath;
		fTemplatesDir += LUA_DIRSEP;
		fTemplatesDir += "homescreen";
		fTemplatesDir += LUA_DIRSEP;
		fTemplatesDir += "templates";
		wxLogDebug("fTemplatesDir = %s\n", std::string(fTemplatesDir) );
		
		if (Rtt_IsDirectory(projectFolder.c_str()) == false)
		{
			Rtt_MakeDirectory(projectFolder.c_str());
			wxLogDebug("Creating directory = %s\n", projectFolder );
		}
		 
		
		lua_State *L = luaL_newstate();
		luaL_openlibs( L );

		Rtt::LuaContext::RegisterModuleLoader( L, "lfs", luaopen_lfs );
//#if 0
    // Uncomment if Simulator services are required in newproject.lua
	//lua_pushlightuserdata( L, (void *)fServices );
    //Rtt::LuaContext::RegisterModuleLoader( L, "simulator", Rtt::LuaLibSimulator::Open, 1 );
//#endif // 0

		const char *script = fNewProjectLuaScript.c_str();
		int status = luaL_loadfile( L ,script ); Rtt_ASSERT( 0 == status );
		lua_createtable( L, 0, 6 );
		{
			lua_pushboolean( L, true );
			lua_setfield( L, -2, "isSimulator" );

			lua_pushstring( L, fTemplateName.c_str() );
			lua_setfield( L, -2, "template" );

			lua_pushinteger( L, fScreenWidth );
			lua_setfield( L, -2, "width" );

			lua_pushinteger( L, fScreenHeight );
			lua_setfield( L, -2, "height" );

			lua_pushstring( L, 0 == fOrientationIndex ? "portrait" : "landscapeRight" );
			lua_setfield( L, -2, "orientation" );

			lua_pushstring( L, projectFolder.c_str() );
			lua_setfield( L, -2, "savePath" );

			lua_pushstring( L, fTemplatesDir.c_str() );
			lua_setfield( L, -2, "templateBaseDir" );
		}

		status = Rtt::LuaContext::DoCall( L, 1, 0 ); Rtt_ASSERT( 0 == status );
		lua_close( L );
			
		
	}

}
