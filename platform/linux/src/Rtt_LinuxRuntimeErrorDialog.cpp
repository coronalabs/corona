#include "Rtt_LinuxRuntimeErrorDialog.h"
#include "Rtt_LinuxContext.h"
#include <string.h>

using namespace std;

namespace Rtt
{
	LinuxRuntimeErrorDialog::LinuxRuntimeErrorDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style) : wxDialog(parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE)
	{
		SetSize(wxSize(520, 480));
		errorText = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(500, 180), wxTE_READONLY | wxTE_MULTILINE);
		stackTraceText = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(500, 120), wxTE_READONLY | wxTE_MULTILINE);
		errorLabel = new wxStaticText(this, wxID_ANY, wxT("Error Message:"));
		stackTracebackLabel = new wxStaticText(this, wxID_ANY, wxT("Stack Traceback:"));
		relaunchProjectLabel = new wxStaticText(this, wxID_ANY, wxT("Do you want to relaunch the project?"));
		btnOK = new wxButton(this, wxID_OK, wxT("Yes"));
		btnCancel = new wxButton(this, wxID_CANCEL, wxT("No"));

		SetLayout();
	}

	void LinuxRuntimeErrorDialog::SetProperties(const char *title, const char *errorMsg, const char *stackTraceback)
	{
		SetTitle(title);
		errorText->SetValue(errorMsg);
		stackTraceText->SetValue(stackTraceback);
		SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
		btnOK->SetDefault();
	}

	void LinuxRuntimeErrorDialog::SetLayout()
	{
		wxBoxSizer *dialogLayout = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *dialogTop = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *dialogMiddle = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *dialogBottom = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *dialogButtons = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *boxSizerTopColumn = new wxBoxSizer(wxVERTICAL);
		wxStaticLine *staticLineSeparator = new wxStaticLine(this, wxID_ANY);

		// set fonts

		// add to box sizers
		boxSizerTopColumn->Add(errorLabel, 0, wxALIGN_CENTER_HORIZONTAL, 3);
		boxSizerTopColumn->Add(errorText, 0, 0, 6);
		boxSizerTopColumn->Add(stackTracebackLabel, 0, wxALIGN_CENTER_HORIZONTAL, 3);
		boxSizerTopColumn->Add(stackTraceText, 0, 0, 6);
		boxSizerTopColumn->Add(relaunchProjectLabel, 0, wxALIGN_CENTER_HORIZONTAL, 3);

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

	BEGIN_EVENT_TABLE(LinuxRuntimeErrorDialog, wxDialog)
		EVT_BUTTON(wxID_OK, LinuxRuntimeErrorDialog::OnOKClicked)
		EVT_BUTTON(wxID_CANCEL, LinuxRuntimeErrorDialog::OnCancelClicked)
	END_EVENT_TABLE();

	void LinuxRuntimeErrorDialog::OnOKClicked(wxCommandEvent &event)
	{
		EndModal(wxID_OK);
		wxYield();
		wxCommandEvent ev(eventRelaunchProject);
		wxPostEvent(wxGetApp().GetFrame(), ev);
	}

	void LinuxRuntimeErrorDialog::OnCancelClicked(wxCommandEvent &event)
	{
		EndModal(wxID_CLOSE);
	}
} // namespace Rtt
