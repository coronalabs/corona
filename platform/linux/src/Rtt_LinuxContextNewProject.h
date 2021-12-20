#ifndef NEWPROJECT_H
#define NEWPROJECT_H

#include <wx/wx.h>
#include <wx/image.h>
#include <wx/statline.h>

#include<string>

#pragma once

namespace Rtt
{
	class NewProjectDialog: public wxDialog {
	public:

		NewProjectDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxDEFAULT_DIALOG_STYLE);
		std::string ProjectFolder;
		std::string fProjectName;
	private:
		void set_properties();
		void do_layout();
		void SetProjectPath();
		void SetResourcePath();
		void CreateProject(std::string projectFolder);
		std::string fTemplateName;
		int fScreenWidth;
		int fScreenHeight;
		std::string fOrientationIndex;
		std::string fP;
		std::string fProjectPath;
		std::string fProjectSavePath;
		std::string fResourcePath;
		
	protected:
		wxTextCtrl* txtApplicationName;
		wxTextCtrl* txtProjectFolder;
		wxButton* btnBrowse;
		wxRadioButton* rProjectOption1;
		wxRadioButton* rProjectOption2;
		wxRadioButton* rProjectOption3;
		wxRadioButton* rProjectOption4;
		wxComboBox* cboScreenSizePreset;
		wxTextCtrl* txtWidth;
		wxTextCtrl* txtHeight;
		wxRadioButton* rUpright;
		wxRadioButton* rSideways;
		wxButton* btnOK;
		wxButton* btnCancel;

		DECLARE_EVENT_TABLE();

	public:
		void onChange(wxCommandEvent &event);
		void OnProjectFolderBrowse(wxCommandEvent &event);
		void onbtnOKClicked(wxCommandEvent &event);
		void onbtnCancelClicked(wxCommandEvent &event);
	};
	
} // namespace Rtt

#endif // NEWPROJECTDIALOG_H
