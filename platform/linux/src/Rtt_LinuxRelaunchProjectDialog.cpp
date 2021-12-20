#include "Rtt_LinuxRelaunchProjectDialog.h"
#include "Rtt_LinuxContext.h"
#include <string.h>

using namespace std;

namespace Rtt
{
	LinuxRelaunchProjectDialog::LinuxRelaunchProjectDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style) : wxDialog(parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE)
	{
		SetSize(wxSize(520, 220));
		messageLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE);
		btnOK = new wxButton(this, wxID_OK, wxT("Yes"));
		btnCancel = new wxButton(this, wxID_CANCEL, wxT("No"));

		SetProperties();
		SetLayout();
	}

	void LinuxRelaunchProjectDialog::SetProperties()
	{
		SetTitle("Project Modified On Disk");
		SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
		messageLabel->SetLabel("A file in your project has been modified on disk.\n\nWould you like to relaunch the project?");
		btnOK->SetDefault();
	}

	void LinuxRelaunchProjectDialog::SetLayout()
	{
		wxBoxSizer *dialogLayout = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *dialogTop = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *dialogMiddle = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *dialogBottom = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *dialogButtons = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *boxSizerTopColumn = new wxBoxSizer(wxVERTICAL);
		wxStaticLine *staticLineSeparator = new wxStaticLine(this, wxID_ANY);

		// add to box sizers
		boxSizerTopColumn->Add(messageLabel, 0, wxALIGN_CENTER_HORIZONTAL, 7);

		// add to dialog buttons
		dialogButtons->Add(btnOK, 0, wxRIGHT, 5);
		dialogButtons->Add(btnCancel, 0, 0, 0);

		// add to dialog layouts
		dialogTop->Add(boxSizerTopColumn, 0, wxEXPAND, 0);
		dialogBottom->Add(staticLineSeparator, 0, wxEXPAND | wxLEFT | wxRIGHT, 7);
		dialogBottom->Add(dialogButtons, 1, wxALIGN_CENTER_HORIZONTAL | wxALL, 7);
		dialogLayout->Add(dialogTop, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 7);
		dialogLayout->Add(dialogBottom, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0);

		SetSizer(dialogLayout);
		Layout();
	}

	BEGIN_EVENT_TABLE(LinuxRelaunchProjectDialog, wxDialog)
		EVT_BUTTON(wxID_OK, LinuxRelaunchProjectDialog::OnOKClicked)
		EVT_BUTTON(wxID_CANCEL, LinuxRelaunchProjectDialog::OnCancelClicked)
	END_EVENT_TABLE();

	void LinuxRelaunchProjectDialog::OnOKClicked(wxCommandEvent & event)
	{
		EndModal(wxID_OK);
		wxYield();
		wxCommandEvent ev(eventRelaunchProject);
		wxPostEvent(wxGetApp().GetFrame(), ev);
	}

	void LinuxRelaunchProjectDialog::OnCancelClicked(wxCommandEvent & event)
	{
		EndModal(wxID_CLOSE);
	}
} // namespace Rtt
