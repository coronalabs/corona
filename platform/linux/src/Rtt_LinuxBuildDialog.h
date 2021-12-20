#ifndef Rtt_LINUX_BUILD_DIALOG_H
#define Rtt_LINUX_BUILD_DIALOG_H

#include "Rtt_LuaContext.h"
#include "Rtt_LinuxSimulatorView.h"
#include "Rtt_LinuxContext.h"
#include "wx/wx.h"
#include "wx/image.h"
#include "wx/statline.h"
#include <string>

namespace Rtt
{
	class LinuxBuildDialog: public wxDialog
	{
	public:
		LinuxBuildDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);

	public:
		void SetAppContext(SolarAppContext *appContext);
		void OnSelectOutputPathClicked(wxCommandEvent &event);
		void OnBuildClicked(wxCommandEvent &event);
		void OnCancelClicked(wxCommandEvent &event);

	private:
		void SetProperties();
		void DoLayout();

	protected:
		SolarAppContext *fAppContext;
		wxTextCtrl *appNameTextCtrl;
		wxTextCtrl *appVersionTextCtrl;
		wxTextCtrl *appPathTextCtrl;
		wxTextCtrl *appBuildPathTextCtrl;
		wxButton *appBuildPathButton;
		wxCheckBox *includeWidgetResourcesCheckbox;
		wxCheckBox *runAfterBuildCheckbox;
		wxButton *buildButton;
		wxButton *cancelButton;

		DECLARE_EVENT_TABLE();
	};
}

#endif // Rtt_LINUX_BUILD_DIALOG_H
