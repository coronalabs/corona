//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "resource.h"
#include "CoronaProject.h"
#include <memory>


/// <summary>Dialog used to build one Corona project as a Win32 desktop app.</summary>
class CBuildWin32AppDlg : public CDialog
{
	DECLARE_DYNAMIC(CBuildWin32AppDlg)

	public:
		/// <summary>Creates a new dialog used to bulid a Win32 desktop app.</summary>
		/// <param name="pParent">
		///  <para>Pointer to a window to be used as this dialog's parent.</para>
		///  <para>Set to null (the default) to make this dialog parent-less.</para>
		/// </param>
		CBuildWin32AppDlg(CWnd* pParent = NULL);

		/// <summary>Destroys this dialog and its resources.</summary>
		virtual ~CBuildWin32AppDlg();

		/// <summary>Called just before this dialog is displayed. Initializes controls.</summary>
		virtual BOOL OnInitDialog();

		/// <summary>Called when the "Browse" button has been clicked for the "Save to Folder" field.</summary>
		afx_msg void OnBrowseSaveto();

		/// <summary>Called when the F1 key was pressed, requesting help information.</summary>
		/// <param name="helpInfoPointer">Identifies the control the user has clicked on for help information.</param>
		/// <returns>Returns true if a control has the keyboard focus. Returns false if no control has the focus.</returns>
		afx_msg BOOL OnHelpInfo(HELPINFO* helpInfoPointer);

		/// <summary>
		///  <para>Sets the Corona project to be compiled and built as a Win32 app.</para>
		///  <para>Must be called before displaying this dialog.</para>
		/// </summary>
		/// <param name="projectPointer">Pointer to the Corona project to be built.</param>
		void SetProject(const std::shared_ptr<CCoronaProject>& projectPointer);

		/// <summary>
		///  Gets a pointer to the Corona project settings assigned to this dialog via the SetProject() method.
		/// </summary>
		/// <returns>
		///  <para>Returns a pointer to the Corona project assigned to this dialog via the SetProject() method.</para>
		///  <para>Returns null if a project has not been assigned.</para>
		/// </returns>
		std::shared_ptr<CCoronaProject> GetProject() const;

		/// <summary>Determines if the user has chosen to run the app after it was built.</summary>
		/// <returns>
		///  <para>Returns true if the user has chosen to run the app after it was built.</para>
		///  <para>Returns false if the built app was not ran or if the user canceled out of the window.</para>
		/// </returns>
		bool HasRanBuiltApp() const;

		/// <summary>Logs analytics including target platform and optional key/value.</summary>
		void LogAnalytics(const char *eventName, const char *key = NULL, const char *value = NULL);

		// Dialog Data
		enum { IDD = IDD_BUILD_WIN32_APP };

	protected:
		/// <summary>Associates member variables with window's controls.</summary>
		virtual void DoDataExchange(CDataExchange* pDX);

		/// <summary>Called when a button on the dialog's title bar or menu has been clicked/invoked.</summary>
		/// <param name="nID">Identifies the command such as SC_CLOSE, SC_CONTEXTHELP, etc.</param>
		/// <param name="lParam">Provides context sensitive information related to the command.</param>
		afx_msg void OnSysCommand(UINT nID, LPARAM lParam);

		/// <summary>Called when the "Build" button (the default submit button) has been clicked on.</summary>
		virtual void OnOK();

		/// <summary>Called when the "Cancel" button was clicked on.</summary>
		virtual void OnCancel();

		/// <summary>Display a warning message box with the given message and string.</summary>
		/// <param name="messageID">Unique ID to a string in the resource file.</param>
		/// <param name="extraText">
		///  Pointer to additional text to be appended to the end of the message. Can be null.
		/// </param>
		void DisplayWarningMessage(UINT messageID, const TCHAR* extraText = nullptr);

		DECLARE_MESSAGE_MAP()

	private:
		/// <summary>Pointer to the Corona project settings needed to compile/build the app.</summary>
		std::shared_ptr<CCoronaProject> fProjectPointer;

		/// <summary>
		///  <para>Set true if the user has chosen to run the app after it was built.</summary>
		///  <para>Set false if the app was not ran or if the canceled out of the window.</para>
		/// </summary>
		bool fHasRanBuiltApp;
};
