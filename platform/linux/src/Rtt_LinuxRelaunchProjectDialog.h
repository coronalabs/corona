#ifndef Rtt_LINUX_RELAUNCH_PROJECT_DIALOG_H
#define Rtt_LINUX_RELAUNCH_PROJECT_DIALOG_H

#include "wx/wx.h"
#include "wx/image.h"
#include "wx/statline.h"
#include <string>

namespace Rtt
{
	class LinuxRelaunchProjectDialog : public wxDialog
	{
	public:
		LinuxRelaunchProjectDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);

	private:
		void SetProperties();
		void SetLayout();

	protected:
		wxStaticText *messageLabel;
		wxButton *btnOK;
		wxButton *btnCancel;

		DECLARE_EVENT_TABLE();

	public:
		void OnOKClicked(wxCommandEvent &event);
		void OnCancelClicked(wxCommandEvent &event);
	};
} // namespace Rtt

#endif // Rtt_LINUX_RELAUNCH_PROJECT_DIALOG_H
