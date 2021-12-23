#ifndef Rtt_LINUX_PREFERENCES_DIALOG_H
#define Rtt_LINUX_PREFERENCES_DIALOG_H

#include "wx/wx.h"
#include "wx/image.h"
#include "wx/statline.h"
#include "wx/activityindicator.h"
#include <string>

namespace Rtt
{
	class LinuxPreferencesDialog : public wxDialog
	{
	public:
		LinuxPreferencesDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);

		enum RelaunchType
		{
			Always, Never, Ask
		};

	public:
		void SetProperties(bool shouldShowRuntimeErrors, bool shouldOpenLastProject, RelaunchType relaunchType);
		bool ShouldShowRuntimeErrors();
		bool ShouldOpenLastProject();
		RelaunchType ShouldRelaunchOnFileChange();

	private:
		void SetLayout();

	protected:
		RelaunchType fRelaunchType;
		wxCheckBox *showRuntimeErrors;
		wxCheckBox *automaticallyLaunchLastProject;
		wxStaticText *relaunchSimulatorOptionText;
		wxRadioButton *relaunchOnModifyAlways;
		wxRadioButton *relaunchOnModifyNever;
		wxRadioButton *relaunchOnModifyAskEveryTime;
		wxButton *btnOK;
		wxButton *btnCancel;

		DECLARE_EVENT_TABLE();

	public:
		void OnOKClicked(wxCommandEvent &event);
		void OnCancelClicked(wxCommandEvent &event);
		void OnRelaunchAlwaysClicked(wxCommandEvent &event);
		void OnRelaunchNeverClicked(wxCommandEvent &event);
		void OnRelaunchAskClicked(wxCommandEvent &event);
	};
} // namespace Rtt

#endif // Rtt_LINUX_PREFERENCES_DIALOG_H
