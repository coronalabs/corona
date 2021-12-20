#ifndef Rtt_Android_Build_Dialog
#define Rtt_Android_Build_Dialog

#include "Rtt_LuaContext.h"
#include "Rtt_LinuxSimulatorView.h"
#include "Rtt_LinuxContext.h"
#include "wx/wx.h"
#include "wx/image.h"
#include "wx/statline.h"
#include <string>

namespace Rtt
{
	class AndroidAppPackager;
	class AndroidAppPackagerParams;
	
	class AndroidBuildDialog: public wxDialog
	{
	public:
		AndroidBuildDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);

	public:
		void SetAppContext(SolarAppContext *appContext);
		bool ReadKeystore(std::string keystorePath, std::string password, bool showErrors);
		void OnSelectOutputPathClicked(wxCommandEvent &event);
		void OnSelectKeyStorePathClicked(wxCommandEvent &event);
		void OnBuildClicked(wxCommandEvent &event);
		void OnCancelClicked(wxCommandEvent &event);

	private:
		static int fetchBuildResult(AndroidAppPackager* packager, AndroidAppPackagerParams* androidBuilderParams, const std::string& tmp);
		void SetProperties();
		void DoLayout();

	protected:
		SolarAppContext *fAppContext;
		wxTextCtrl *appNameTextCtrl;
		wxTextCtrl *appVersionTextCtrl;
		wxTextCtrl *appVersionCodeTextCtrl;
		wxTextCtrl *appPackageNameTextCtrl;
		wxTextCtrl *appPathTextCtrl;
		wxTextCtrl *appBuildPathTextCtrl;
		wxButton *appBuildPathButton;
		wxComboBox *targetAppStoreComboBox;
		wxTextCtrl *keystorePathTextCtrl;
		wxButton *keystorePathButton;
		wxComboBox *keyAliasComboBox;
		wxCheckBox *installToDeviceCheckbox;
		wxButton *buildButton;
		wxButton *cancelButton;
		wxString keystorePassword;
		bool keystorePasswordValid;

		DECLARE_EVENT_TABLE();
	};
};

#endif // Rtt_Android_Build_Dialog
