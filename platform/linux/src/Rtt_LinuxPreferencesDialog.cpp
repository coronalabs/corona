#include "Rtt_LinuxPreferencesDialog.h"
#include "Rtt_LuaContext.h"
#include <string.h>

#define ID_RADIO_BUTTON_RELAUNCH_ALWAYS wxID_HIGHEST + 1
#define ID_RADIO_BUTTON_RELAUNCH_NEVER wxID_HIGHEST + 2
#define ID_RADIO_BUTTON_RELAUNCH_ASK wxID_HIGHEST + 3

using namespace std;

namespace Rtt
{
	LinuxPreferencesDialog::LinuxPreferencesDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style) : wxDialog(parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE)
	{
		SetSize(wxSize(520, 320));
		showRuntimeErrors = new wxCheckBox(this, -1, "Show Runtime Errors", wxPoint(0, 0));
		automaticallyLaunchLastProject = new wxCheckBox(this, -1, "Automatically open last project", wxPoint(0, 0));
		relaunchSimulatorOptionText = new wxStaticText(this, wxID_ANY, wxT("Relaunch Simulator when project is modified?"));
		relaunchOnModifyAlways = new wxRadioButton(this, ID_RADIO_BUTTON_RELAUNCH_ALWAYS, "Always", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
		relaunchOnModifyNever = new wxRadioButton(this, ID_RADIO_BUTTON_RELAUNCH_NEVER, "Never", wxDefaultPosition, wxDefaultSize);
		relaunchOnModifyAskEveryTime = new wxRadioButton(this, ID_RADIO_BUTTON_RELAUNCH_ASK, "Ask Every Time", wxDefaultPosition, wxDefaultSize);
		btnOK = new wxButton(this, wxID_OK, wxT("OK"));
		btnCancel = new wxButton(this, wxID_CANCEL, wxT("Cancel"));

		SetTitle(wxT("Preferences"));
		SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
		btnOK->SetDefault();
		SetProperties(true, false, RelaunchType::Always);
		SetLayout();
	}

	void LinuxPreferencesDialog::SetProperties(bool shouldShowRuntimeErrors, bool shouldOpenLastProject, RelaunchType relaunchType)
	{
		showRuntimeErrors->SetValue(shouldShowRuntimeErrors);
		automaticallyLaunchLastProject->SetValue(shouldOpenLastProject);
		relaunchOnModifyAlways->SetValue(false);
		relaunchOnModifyNever->SetValue(false);
		relaunchOnModifyAskEveryTime->SetValue(false);

		switch (relaunchType)
		{
			case RelaunchType::Always:
				relaunchOnModifyAlways->SetValue(true);
				break;

			case RelaunchType::Never:
				relaunchOnModifyNever->SetValue(true);
				break;

			case RelaunchType::Ask:
				relaunchOnModifyAskEveryTime->SetValue(true);
				break;
		}
	}

	void LinuxPreferencesDialog::SetLayout()
	{
		wxBoxSizer *dialogLayout = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *dialogTop = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *dialogMiddle = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *dialogBottom = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *dialogButtons = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *boxSizerTopColumn1 = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *boxSizerTopColumn2 = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *boxSizerTopColumn3 = new wxBoxSizer(wxVERTICAL);
		wxStaticLine *staticLineSeparator = new wxStaticLine(this, wxID_ANY);

		// add to box sizers
		boxSizerTopColumn1->Add(showRuntimeErrors, 0, wxALIGN_LEFT | wxBOTTOM | wxTOP, 6);
		boxSizerTopColumn1->Add(automaticallyLaunchLastProject, 0, wxALIGN_LEFT | wxBOTTOM | wxTOP, 6);
		boxSizerTopColumn1->Add(relaunchSimulatorOptionText, 0, wxALIGN_LEFT | wxBOTTOM | wxTOP, 6);
		boxSizerTopColumn1->Add(relaunchOnModifyAlways, 0, wxALIGN_LEFT | wxBOTTOM | wxTOP, 6);
		boxSizerTopColumn1->Add(relaunchOnModifyNever, 0, wxALIGN_LEFT | wxBOTTOM | wxTOP, 6);
		boxSizerTopColumn1->Add(relaunchOnModifyAskEveryTime, 0, wxALIGN_LEFT | wxBOTTOM | wxTOP, 6);

		// add to dialog buttons
		dialogButtons->Add(btnOK, 0, wxRIGHT, 5);
		dialogButtons->Add(btnCancel, 0, 0, 0);

		// add to dialog layouts
		dialogTop->Add(boxSizerTopColumn1, 0, wxLEFT | wxRIGHT, 7);
		dialogTop->Add(boxSizerTopColumn2, 1, wxLEFT | wxRIGHT, 0);
		dialogTop->Add(boxSizerTopColumn3, 0, wxEXPAND | wxLEFT | wxRIGHT, 7);
		dialogBottom->Add(staticLineSeparator, 0, wxEXPAND | wxLEFT | wxRIGHT, 7);
		dialogBottom->Add(dialogButtons, 1, wxALIGN_CENTER_HORIZONTAL | wxALL, 7);
		dialogLayout->Add(dialogTop, 0, wxBOTTOM | wxEXPAND | wxTOP, 8);
		dialogLayout->Add(dialogBottom, 0, wxEXPAND, 0);

		SetSizer(dialogLayout);
		Layout();
	}

	bool LinuxPreferencesDialog::ShouldShowRuntimeErrors()
	{
		return showRuntimeErrors->GetValue();
	}

	bool LinuxPreferencesDialog::ShouldOpenLastProject()
	{
		return automaticallyLaunchLastProject->GetValue();
	}

	LinuxPreferencesDialog::RelaunchType LinuxPreferencesDialog::ShouldRelaunchOnFileChange()
	{
		return fRelaunchType;
	}

	BEGIN_EVENT_TABLE(LinuxPreferencesDialog, wxDialog)
		EVT_BUTTON(wxID_OK, LinuxPreferencesDialog::OnOKClicked)
		EVT_BUTTON(wxID_CANCEL, LinuxPreferencesDialog::OnCancelClicked)
		EVT_RADIOBUTTON(ID_RADIO_BUTTON_RELAUNCH_ALWAYS, LinuxPreferencesDialog::OnRelaunchAlwaysClicked)
		EVT_RADIOBUTTON(ID_RADIO_BUTTON_RELAUNCH_NEVER, LinuxPreferencesDialog::OnRelaunchNeverClicked)
		EVT_RADIOBUTTON(ID_RADIO_BUTTON_RELAUNCH_ASK, LinuxPreferencesDialog::OnRelaunchAskClicked)
	END_EVENT_TABLE();

	void LinuxPreferencesDialog::OnOKClicked(wxCommandEvent &event)
	{
		EndModal(wxID_OK);
	}

	void LinuxPreferencesDialog::OnCancelClicked(wxCommandEvent &event)
	{
		EndModal(wxID_CLOSE);
	}

	void LinuxPreferencesDialog::OnRelaunchAlwaysClicked(wxCommandEvent &event)
	{
		fRelaunchType = RelaunchType::Always;
	}

	void LinuxPreferencesDialog::OnRelaunchNeverClicked(wxCommandEvent &event)
	{
		fRelaunchType = RelaunchType::Never;
	}

	void LinuxPreferencesDialog::OnRelaunchAskClicked(wxCommandEvent &event)
	{
		fRelaunchType = RelaunchType::Ask;
	}
} // namespace Rtt
