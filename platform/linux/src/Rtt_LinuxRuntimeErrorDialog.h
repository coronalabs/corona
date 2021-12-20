#ifndef Rtt_LINUX_RUNTIME_ERROR_DIALOG_H
#define Rtt_LINUX_RUNTIME_ERROR_DIALOG_H

#include "wx/wx.h"
#include "wx/image.h"
#include "wx/statline.h"
#include "wx/activityindicator.h"
#include <string>

namespace Rtt
{
	class LinuxRuntimeErrorDialog : public wxDialog
	{
	public:
		LinuxRuntimeErrorDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
		void SetProperties(const char *title, const char *errorMsg, const char *stackTraceback);

	private:
		void SetLayout();

	protected:
		wxStaticText *errorLabel;
		wxStaticText *stackTracebackLabel;
		wxStaticText *relaunchProjectLabel;
		wxTextCtrl *errorText;
		wxTextCtrl *stackTraceText;
		wxButton *btnOK;
		wxButton *btnCancel;

		DECLARE_EVENT_TABLE();

	public:
		void OnOKClicked(wxCommandEvent &event);
		void OnCancelClicked(wxCommandEvent &event);
	};
} // namespace Rtt

#endif // Rtt_LINUX_RUNTIME_ERROR_DIALOG_H
