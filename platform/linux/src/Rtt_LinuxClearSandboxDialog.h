#ifndef Rtt_Linux_Clear_Sandbox_Dialog
#define Rtt_Linux_Clear_Sandbox_Dialog

#if 0

namespace Rtt
{
	class LinuxClearProjectSandboxDialog: public wxDialog
	{
	public:
		LinuxClearProjectSandboxDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
		void OnConfirmClicked(wxCommandEvent &event);
		void OnCancelClicked(wxCommandEvent &event);

	private:
		void SetProperties();
		void DoLayout();

	protected:
		wxButton *okButton;
		wxButton *cancelButton;

		DECLARE_EVENT_TABLE();
	};
};
#endif

#endif // Rtt_Linux_Clear_Sandbox_Dialog
