#include "Rtt_LinuxClearSandboxDialog.h"

namespace Rtt
{
	LinuxClearProjectSandboxDialog::LinuxClearProjectSandboxDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style):
		wxDialog(parent, id, title, pos, size, wxCAPTION)
	{
		okButton = new wxButton(this, wxID_OK, wxT("Confirm"));
		cancelButton = new wxButton(this, wxID_CANCEL, wxT("Cancel"));

		SetProperties();
		DoLayout();
	}

	void LinuxClearProjectSandboxDialog::SetProperties()
	{
		SetTitle(wxT("Clear Project Sandbox?"));
	}

	void LinuxClearProjectSandboxDialog::DoLayout()
	{
		wxBoxSizer *parentGridSizer = new wxBoxSizer(wxVERTICAL);
		wxFlexGridSizer *buttonGridSizer = new wxFlexGridSizer(1, 2, 0, 0);
		wxGridSizer *optionsGridSizer = new wxGridSizer(1, 1, 0, 0);
		wxStaticText *informationLabel = new wxStaticText(this, wxID_ANY, wxT("You're about to clear the sandbox for this project. This action cannot be undone.\n\nDo you wish to proceed?"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER | wxALIGN_LEFT | wxALIGN_RIGHT);
		optionsGridSizer->Add(informationLabel, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT, 10);
		parentGridSizer->Add(optionsGridSizer, 0, wxALL | wxEXPAND, 10);
		wxStaticLine *staticLineSeparator = new wxStaticLine(this, wxID_ANY);
		parentGridSizer->Add(staticLineSeparator, 0, wxBOTTOM | wxEXPAND | wxTOP, 5);
		buttonGridSizer->Add(okButton, 1, wxRIGHT | wxTOP, 10);
		buttonGridSizer->Add(cancelButton, 1, wxRIGHT | wxTOP, 10);
		buttonGridSizer->AddGrowableCol(0);
		buttonGridSizer->AddGrowableCol(1);
		parentGridSizer->Add(buttonGridSizer, 1, wxALIGN_CENTER_HORIZONTAL, 0);
		SetSizer(parentGridSizer);
		parentGridSizer->Fit(this);
		Layout();
		Centre();
	}

	BEGIN_EVENT_TABLE(LinuxClearProjectSandboxDialog, wxDialog)
		EVT_BUTTON(wxID_OK, LinuxClearProjectSandboxDialog::OnConfirmClicked)
		EVT_BUTTON(wxID_CANCEL, LinuxClearProjectSandboxDialog::OnCancelClicked)
	END_EVENT_TABLE();

	void LinuxClearProjectSandboxDialog::OnConfirmClicked(wxCommandEvent &event)
	{
		EndModal(wxID_OK);
	}

	void LinuxClearProjectSandboxDialog::OnCancelClicked(wxCommandEvent &event)
	{
		EndModal(wxID_CLOSE);
	}
};
