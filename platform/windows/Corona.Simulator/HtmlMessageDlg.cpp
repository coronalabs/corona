//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HtmlMessageDlg.h"
#include "Interop\Graphics\\FontSizeConverter.h"
#include "sundown\buffer.h"
#include "sundown\html.h"
#include "sundown\markdown.h"
#include "WinString.h"


IMPLEMENT_DYNAMIC(CHtmlMessageDlg, CDialog)


#pragma region Constructor/Destructor
CHtmlMessageDlg::CHtmlMessageDlg(CWnd* pParent /*=NULL*/)
:	CDialog(CHtmlMessageDlg::IDD, pParent)
{
}

CHtmlMessageDlg::~CHtmlMessageDlg()
{
}

#pragma endregion


#pragma region Message Mappings
void CHtmlMessageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHtmlMessageDlg, CDialog)
	ON_NOTIFY(SimpleBrowser::BeforeNavigate2, IDC_WEB_BROWSER, OnBeforeNavigate2)
END_MESSAGE_MAP()

#pragma endregion


#pragma region Public Methods
BOOL CHtmlMessageDlg::OnInitDialog()
{
	// Initialize base class first.
	CDialog::OnInitDialog();

	// Update the window's title bar text.
	SetWindowText(fTitleText);

	// Set up the web browser control.
	fWebBrowser.CreateFromControl(this, IDC_WEB_BROWSER);
	fWebBrowser.PutRegisterAsDropTarget(false);
	fWebBrowser.SetMessageTranslationEnabled(false);
	fWebBrowser.Write(fHtmlText);

	return TRUE;
}

void CHtmlMessageDlg::SetTitle(const CString& text)
{
	fTitleText = text;
}

CString CHtmlMessageDlg::GetTitle() const
{
	return fTitleText;
}

void CHtmlMessageDlg::SetHtml(const CString& text)
{
	fHtmlText = text;
}

void CHtmlMessageDlg::SetHtmlBodyContents(const CString& text)
{
	// Fetch the Windows system's default font name and size.
	CString defaultFontName;
	int defaultFontSizeInPixels = 0;
	{
		NONCLIENTMETRICS nonClientMetrics{};
		nonClientMetrics.cbSize = sizeof(nonClientMetrics);
		auto wasSuccessful = ::SystemParametersInfo(
				SPI_GETNONCLIENTMETRICS, sizeof(nonClientMetrics), &nonClientMetrics, 0);
		if (wasSuccessful)
		{
			// Fetch the default font name.
			defaultFontName = nonClientMetrics.lfMessageFont.lfFaceName;

			// Fetch the default font size in pixels, excluding line spacing.
			auto windowPointer = AfxGetMainWnd();
			if (windowPointer && windowPointer->GetDC())
			{
				Interop::Graphics::FontSizeConverter fontSizeConverter;
				fontSizeConverter.SetSizeUsing(windowPointer->GetDC()->GetSafeHdc(), nonClientMetrics.lfMessageFont);
				fontSizeConverter.ConvertTo(Gdiplus::UnitPixel);
				defaultFontSizeInPixels = (int)(fontSizeConverter.GetSize() + 0.5f);
			}
		}
	}

	// Update our HTML text string using the given "text" argument as the body's content.
	fHtmlText = _T("<html><body");
	if (!defaultFontName.IsEmpty())
	{
		// Apply the system's default font name and size to the HTML body.
		fHtmlText.Append(_T(" style=\"font-family='"));
		fHtmlText.Append(defaultFontName);
		fHtmlText.Append(_T("';"));
		if (defaultFontSizeInPixels > 0)
		{
			CString fontSizeStyleText;
			fontSizeStyleText.Format(_T("font-size=%dpx;"), defaultFontSizeInPixels);
			fHtmlText.Append(fontSizeStyleText);
		}
		fHtmlText.Append(_T("\""));
	}
	fHtmlText.Append(_T(">"));
	fHtmlText.Append(text);
	fHtmlText.Append(_T("</body></html>"));
}

void CHtmlMessageDlg::SetHtmlFromMarkdown(const CString& text)
{
	// First, initialize the HTML text to an empty body in case something goes wrong down below.
	SetHtmlFromPlainText(_T(""));

	// Do not continue if given a null/empty string.
	WinString textTranscoder(text);
	if (textTranscoder.IsEmpty())
	{
		return;
	}

	// Inject a markdown style line break in all places that have a single '\n' line ending.
	// Notes:
	// - Markdown will ignore single \n line ending and merge the 2 lines together instead. (We don't want this.)
	// - Adding 2 spaces or more to the end of the line causes markdown to generate an HTML <br> line break.
	// - Markdown will generate an HTML paragraph <p></p> block when a line ends with 2 \n\n line endings.
	textTranscoder.Replace("\r\n", "\n");
	textTranscoder.Replace("\n", "  \n");
	textTranscoder.Replace("  \n  \n", "\n\n");

	// Fetch the byte length (excluding the null character) of the markdown message in UTF-8 form.
	auto utf8MarkdownLength = strlen(textTranscoder.GetUTF8());
	if (utf8MarkdownLength <= 0)
	{
		return;
	}

	// Convert the markdown string to HTML using the "sundown" library.
	// Note: Sundown does not generate HTML <head> or <body> tags.
	//       It only generates HTML content to be copied between <body></body> tags.
	auto inputBufferPointer = bufnew(1024);
	auto outputBufferPointer = bufnew(64);
	if (inputBufferPointer && outputBufferPointer)
	{
		sd_callbacks sundownCallbacks{};
		html_renderopt htmlRenderOptions{};
		sdhtml_renderer(&sundownCallbacks, &htmlRenderOptions, 0);
		auto markdownPointer = sd_markdown_new(0, 16, &sundownCallbacks, &htmlRenderOptions);
		bufgrow(inputBufferPointer, utf8MarkdownLength + 1);
		if (markdownPointer && inputBufferPointer->data && (inputBufferPointer->asize > utf8MarkdownLength))
		{
			memcpy_s(inputBufferPointer->data, inputBufferPointer->asize, textTranscoder.GetUTF8(), utf8MarkdownLength + 1);
			inputBufferPointer->size = utf8MarkdownLength + 1;
			sd_markdown_render(outputBufferPointer, inputBufferPointer->data, inputBufferPointer->size, markdownPointer);
			sd_markdown_free(markdownPointer);
			if (outputBufferPointer->data)
			{
				// Markdown conversion was successful.
				// Transcode the HTML text from UTF-8 to UTF-16 and store it.
				textTranscoder.SetUTF8((const char*)outputBufferPointer->data);
				if (!textTranscoder.IsEmpty())
				{
					SetHtmlBodyContents(textTranscoder.GetTCHAR());
				}
			}
		}
	}
	if (inputBufferPointer)
	{
		bufrelease(inputBufferPointer);
	}
	if (outputBufferPointer)
	{
		bufrelease(outputBufferPointer);
	}
}

void CHtmlMessageDlg::SetHtmlFromPlainText(const CString& text)
{
	CString escapedText(text);
	escapedText.Replace(_T("<"), _T("&lt;"));
	escapedText.Replace(_T(">"), _T("&gt;"));
	escapedText.Replace(_T("&"), _T("&amp;"));
	escapedText.Replace(_T("\r\n"), _T("</br>"));
	escapedText.Replace(_T("\n"), _T("</br>"));
	SetHtmlBodyContents(escapedText);
}

CString CHtmlMessageDlg::GetHtml() const
{
	return fHtmlText;
}

#pragma endregion


#pragma region Protected Methods
void CHtmlMessageDlg::OnBeforeNavigate2(NMHDR *pNMHDR, LRESULT *pResult)
{
	// Validate.
	if (!pResult)
	{
		return;
	}

	// Do not let the web control display the clicked on URL.
	// Override it and display the URL via the system's default web browser application.
	try
	{
		if (pNMHDR)
		{
			auto notificationPointer = (SimpleBrowser::Notification*)pNMHDR;
			if (!notificationPointer->URL.IsEmpty() && (notificationPointer->URL != _T("about:blank")))
			{
				::ShellExecute(nullptr, _T("open"), notificationPointer->URL, nullptr, nullptr, SW_SHOWNORMAL);
				*pResult = 1;
			}
		}
	}
	catch (...) {}
}

#pragma endregion
