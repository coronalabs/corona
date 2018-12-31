//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "resource.h"
#include "SimpleBrowser.h"


/// <summary>Dialog used to display an HTML formatted message to the user.</summary>
class CHtmlMessageDlg : public CDialog
{
	DECLARE_DYNAMIC(CHtmlMessageDlg)

	public:
		/// <summary>Creates a new HTML message dialog.</summary>
		/// <param name="pParent">
		///  <para>Pointer to a window to be used as this dialog's parent.</para>
		///  <para>Set to null (the default) to make this dialog parent-less.</para>
		/// </param>
		CHtmlMessageDlg(CWnd* pParent = NULL);

		/// <summary>Destroys this dialog and its resources.</summary>
		virtual ~CHtmlMessageDlg();

		/// <summary>Called just before this dialog is displayed. Initializes controls.</summary>
		virtual BOOL OnInitDialog() override;

		/// <summary>
		///  <para>Sets the text to be shown in the dialog's title bar.</para>
		///  <para>This must be set before showing the window or else the call will be ignored.</para>
		/// </summary>
		/// <param name="text">The text to be shown in the title bar. Can be empty.</param>
		void SetTitle(const CString& text);

		/// <summary>Gets the dialog's title bar text.</summary>
		/// <returns>Returns the title bar text.</returns>
		CString GetTitle() const;

		/// <summary>
		///  <para>Sets the HTML text to be shown in the browser control. Must include the HTML head and body tags.</para>
		///  <para>This must be set before showing the window or else the call will be ignored.</para>
		/// </summary>
		/// <param name="text">The full HTML text including the HEAD and BODY tags.</param>
		void SetHtml(const CString& text);

		/// <summary>
		///  <para>Sets the HTML content to be shown between the web browser control's defautl BODY tags.</para>
		///  <para>
		///   This method is preferred over the SetHtml() method since the dialog will configure the HTML page
		///   to use the Windows system's default font.
		///  </para>
		///  <para>This must be set before showing the window or else the call will be ignored.</para>
		/// </summary>
		/// <param name="text">The HTML text to be injected in-between the dialog's default HTML body tags.</param>
		void SetHtmlBodyContents(const CString& text);

		/// <summary>
		///  <para>Generates the HTML text to be shown in the dialog using the given markdown text.</para>
		///  <para>This must be set before showing the window or else the call will be ignored.</para>
		/// </summary>
		/// <param name="text">The markdown formatted text to be converted to HTML and shown in the dialog.</param>
		void SetHtmlFromMarkdown(const CString& text);

		/// <summary>
		///  <para>Sets the HTML content for this dialog using the given plain text string.</para>
		///  <para>
		///   This method will automatically escape HTML reserved characters such as &lt;, &gt;, etc.
		///   and convert newlines into HTML line breaks.
		///  </para>
		///  <para>This must be set before showing the window or else the call will be ignored.</para>
		/// </summary>
		/// <param name="text">The plain text to be escaped into proper HTML and display in the dialog.</param>
		void SetHtmlFromPlainText(const CString& text);

		/// <summary>Gets the full HTML text, inclue the head and body tags, shown in the dialog.</summary>
		/// <returns>Returns the full HTML text shown in the dialog.</returns>
		CString GetHtml() const;

		// Dialog Data
		enum { IDD = IDD_HTML_MESSAGE };

	protected:
		/// <summary>Associates member variables with window's controls.</summary>
		virtual void DoDataExchange(CDataExchange* pDX);

		/// <summray>
		///  <para>Called when the web browser control is about to load a URL.</para>
		/// </summary>
		/// <param name="pNMHDR">Pointer to a "SimpleBrowser::Notification" object providing the URL.</param>
		/// <param name="pResult">
		///  <para>Provides feedback to the caller on whether or not navigation should be canceled.</para>
		///  <para>If this method upon return sets this to non-zero, then navigation should be canceled.</para>
		///  <para>If zero, then the caller should proceed to load the URL.</para>
		/// </param>
		afx_msg void OnBeforeNavigate2(NMHDR *pNMHDR, LRESULT *pResult);
		
		DECLARE_MESSAGE_MAP()

	private:
		/// <summary>Web browser control used to show the HTML text.</summary>
		SimpleBrowser fWebBrowser;

		/// <summary>The text to be copied to the dialog's title bar when shown.</summary>
		CString fTitleText;

		/// <summary>The full HTML text to be copied to the web browser control when the dialog is shown.</summary>
		CString fHtmlText;
};
