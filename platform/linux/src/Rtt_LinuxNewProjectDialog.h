#ifndef Rtt_LINUX_NEW_PROJECT_DIALOG_H
#define Rtt_LINUX_NEW_PROJECT_DIALOG_H

#include "wx/wx.h"
#include "wx/image.h"
#include "wx/statline.h"
#include <string>

namespace Rtt
{
	class LinuxNewProjectDialog: public wxDialog
	{
	public:
		LinuxNewProjectDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);

	public:
		std::string GetProjectName() { return fProjectName; }
		std::string GetProjectFolder() { return fProjectFolder; }

	private:
		void SetProperties();
		void DoLayout();
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
		std::string fProjectFolder;
		std::string fProjectName;

	protected:
		wxTextCtrl *txtApplicationName;
		wxTextCtrl *txtProjectFolder;
		wxButton *btnBrowse;
		wxRadioButton *rProjectOption1;
		wxRadioButton *rProjectOption2;
		wxRadioButton *rProjectOption3;
		wxRadioButton *rProjectOption4;
		wxComboBox *cboScreenSizePreset;
		wxTextCtrl *txtWidth;
		wxTextCtrl *txtHeight;
		wxRadioButton *rUpright;
		wxRadioButton *rSideways;
		wxButton *btnOK;
		wxButton *btnCancel;

		DECLARE_EVENT_TABLE();

	public:
		void OnChange(wxCommandEvent &event);
		void OnProjectFolderBrowse(wxCommandEvent &event);
		void OnOKClicked(wxCommandEvent &event);
		void OnCancelClicked(wxCommandEvent &event);
	};
} // namespace Rtt

#endif // Rtt_LINUX_NEW_PROJECT_DIALOG_H
