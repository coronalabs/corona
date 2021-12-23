#ifndef Rtt_LINUX_CLONE_PROJECT_DIALOG_H
#define Rtt_LINUX_CLONE_PROJECT_DIALOG_H

#include "wx/wx.h"
#include "wx/image.h"
#include "wx/statline.h"
#include "wx/activityindicator.h"
#include <string>

namespace Rtt
{
	class LinuxCloneProjectDialog : public wxDialog
	{
	public:
		LinuxCloneProjectDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);

	private:
		void SetProperties();
		void SetLayout();
		void CloneProject();

	protected:
		wxTextCtrl *txtCloneUrl;
		wxTextCtrl *txtProjectFolder;
		wxButton *btnBrowse;
		wxActivityIndicator *activityIndicator;
		wxButton *btnOK;
		wxButton *btnCancel;

		DECLARE_EVENT_TABLE();

	public:
		void OnProjectFolderBrowse(wxCommandEvent &event);
		void OnOKClicked(wxCommandEvent &event);
		void OnCancelClicked(wxCommandEvent &event);
	};
} // namespace Rtt

#endif // Rtt_LINUX_CLONE_PROJECT_DIALOG_H
