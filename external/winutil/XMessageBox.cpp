// XMessageBox.cpp  Version 1.10
//
// Author: Hans Dietrich
//         hdietrich@gmail.com
//
// Description:
//     XMessageBox.cpp implements XMessageBox(), a drop-in replacement for 
//     MessageBox() that includes custom checkboxes, custom buttons, custom
//     icon, and more.  For more information see
//         http://www.codeproject.com/KB/dialog/xmessagebox.aspx
//
// History
//     Version 1.10 - 2008 November 29
//     - Fixed problem with XMESSAGEBOX_DO_NOT_SAVE_CHECKBOX
//
//     Version 1.9 - 2008 November 22
//     - Fixed keyboard processing broken in 1.8
//
//     Version 1.8 - 2008 November 19
//     - Added new bit flag VistaStyle to XMSGBOXPARAMS::dwOptions.
//       Setting this option bit will cause the message background to be 
//       painted with the current window color (typically white), and the 
//       buttons to be right-justified.
//     - Added new bit flag Narrow to XMSGBOXPARAMS::dwOptions.
//       Setting this option bit will cause the message box to be no wider 
//       than SM_CXSCREEN / 3. 
//     - Added two new members to XMSGBOXPARAMS:  crText and crBackground, 
//       allowing you to specify the message text and background colors.
//     - Made all buttons auto-size, depending on button text.  Buttons will 
//       have the usual width, unless the text is too long to fit.
//     - Eliminated requirement that you define all the user-defined button 
//       strings, even if you just wanted to change a few strings. All the 
//       button strings that you do <u>not</u> specify will have their default 
//       value. 
//     - Fixed centering problem when checkbox is specified
//     - Replaced checkbox width calculation with one based on DLUs
//     - Added function XMessageBoxGetCheckBox() to check if a checkbox value 
//       has been stored in registry or ini file
//     - Added countdown indicator to caption when disabled timer is used
//     - Added an internal message loop.  
//
//     Version 1.7 - 2008 November 9
//     - Converted button sizes to dialog units, suggested by Brian
//     - Made custom buttons auto-size (depending on text), suggested by Albert Weinert
//
//     Version 1.6 - 2008 November 6
//     - Added Ctrl-C feature
//     - Mods for 64-bit support, provided by wilfridc
//     - Added VS2005 project
//     - Fixed problem with Vista and NONCLIENTMETRICS struct
//     - Fixed bug with using resource id for caption
//
//     Version 1.5 - 2006 August 21
//     - Fixed bugs reported by kingworm, TMS_73, Curtis Faith, ladislav Hruska,
//       Tim Hodgson, DrJohnAir
//     - Incorporated Uwe Keim's changes for dynamic button captions
//
//     Version 1.4 - 2003 December 10
//     - Implemented MB_DONOTSHOWAGAIN
//     - Implemented MB_TOPMOST
//     - Implemented MB_SETFOREGROUND
//     - Added MB_SKIPSKIPALLCANCEL and MB_IGNOREIGNOREALLCANCEL, suggested
//       by Shane L
//     - Added HINSTANCE parameter for loading strings from extra-exe resource
//     - Added "report function" parameter for optional report function
//     - Added custom button parameter to allow definition of custom buttons,
//       thanks to Obliterator for comments and review
//     - Added timeout parameter to automatically select default button
//       after timeout expires, thanks to Obliterator for suggestion
//     - Added disabled time parameter, that will disable all buttons on the
//       messagebox for n seconds (for nag dialogs).
//     - Added custom icon parameter
//     - The XMessageBox dialog will now be centered even in non-MFC apps,
//       thanks to Tom Wright for suggestion
//     - The message text and caption text can now be passed as either a string
//       or a resource ID (using MAKEINTRESOURCE)
//     - The initial x,y screen coordinates can now be specified.
//     - The buttons can now be centered (default) or right-justified, as
//       in XP Explorer.
//     - Response to "Do Not Ask/Tell" checkboxes will be saved automatically
//       to ini file, if lpszModule member is non-NULL
//     - Gathered all optional parameters into one optional XMSGBOXPARAMS struct.
//     - Added assert if default button is set to Help or Report button
//     - Removed statics and rearranged code into classes, thanks to code from
//       Anne Jan Beeks
//
//     Version 1.3 - 2001 July 31
//     - Miscellaneous improvements and bug fixes
//
//     Version 1.2 - 2001 July 13
//     - Initial public release
//
// Some parts of this software are from information in the Microsoft SDK.
//
// This software is released into the public domain.  You are free to use it
// in any way you like, except that you may not sell this source code.
//
// This software is provided "as is" with no expressed or implied warranty.
// I accept no liability for any damage or loss of business that this software
// may cause.
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// include the following line if compiling an MFC app
#include "stdafx.h"
///////////////////////////////////////////////////////////////////////////////


#ifndef _MFC_VER
#include <windows.h>
#include <stdio.h>
#include <crtdbg.h>
#include <tchar.h>
#pragma message("    compiling for Win32")
#else
#pragma message("    compiling for MFC")
#endif

#include "XMessageBox.h"

#pragma warning(disable : 4127)		// conditional expression is constant
#pragma warning(disable : 4996)		// disable bogus deprecation warning

#define countof(array) (sizeof(array)/sizeof(array[0]))


#ifndef __noop
#if _MSC_VER < 1300
#define __noop ((void)0)
#endif
#endif

#undef TRACE
#define TRACE __noop

//=============================================================================
// if you want to see the TRACE output, uncomment this line:
//#include "XTrace.h"
//=============================================================================


///////////////////////////////////////////////////////////////////////////////
//
// If you want to save the state of "Do Not Ask/Tell" checkbox to profile (ini)
// file instead of registry, uncomment the following line:
//
//#define XMESSAGEBOX_USE_PROFILE_FILE


///////////////////////////////////////////////////////////////////////////////
//
// If you do not want automatic saving of "Do Not Ask/Tell" checkbox, 
// uncomment the following line:
//
//#define XMESSAGEBOX_DO_NOT_SAVE_CHECKBOX


///////////////////////////////////////////////////////////////////////////////
//
// If you have chosen to automatically save "Do Not Ask/Tell" checkbox to ini:
//
// Normally the lpszModule and nLine data are encoded, since it might not be
// desirable to allow users to be able to see the path and module name of
// your source file.  If you do not want encoding of "Do Not Ask/Tell" module
// info in the registry (or ini file), uncomment the following line:
//
//#define XMESSAGEBOX_DO_NOT_ENCODE


///////////////////////////////////////////////////////////////////////////////
//
// This identifier specifies the format of the text displayed for the timeout 
// key, which by default is "%s = %d".  You may change this to anything you 
// wish, as long as 1) there is both a %s and a %d;  and 2) the %s precedes 
// the %d.
//
#define XMESSAGEBOX_TIMEOUT_TEXT_FORMAT	_T("%s = %d")


///////////////////////////////////////////////////////////////////////////////
//
// This identifier specifies the name of the ini file, which by default 
// is "XMessageBox.ini".
//
#define XMESSAGEBOX_INI_FILE			_T("XMessageBox.ini")


///////////////////////////////////////////////////////////////////////////////
//
// This identifier specifies the registry key used to store checkbox values.  
// By default it is "XMessageBox".
//
#define XMESSAGEBOX_REGISTRY_KEY		_T("XMessageBox")

///////////////////////////////////////////////////////////////////////////////
//
// If you do not want to display the disabled timer countdown in the caption,
// uncomment the following line:
//
//#define XMESSAGEBOX_NO_DISABLED_COUNTDOWN

///////////////////////////////////////////////////////////////////////////////
//+++1.8
// If you do not want to use automatic Vista detection and style, comment out 
// the following line:
//
#define XMESSAGEBOX_AUTO_VISTA_STYLE


#ifndef XMESSAGEBOX_USE_PROFILE_FILE
static void WriteRegistry(LPCTSTR lpszCompanyName, LPCTSTR lpszKey, DWORD dwData);
static DWORD ReadRegistry(LPCTSTR lpszCompanyName, LPCTSTR lpszKey);
#endif // XMESSAGEBOX_USE_PROFILE_FILE


///////////////////////////////////////////////////////////////////////////////
//
// CXRect - replacement for CRect
//
class CXRect : public tagRECT
{
public:
// Constructors
	// uninitialized rectangle
	CXRect() {}

// Attributes (in addition to RECT members)

	// retrieves the width
	int Width() const	{ return right - left; }

	// returns the height
	int Height() const	{ return bottom - top; }

// Operations

	// set rectangle from left, top, right, and bottom
	void SetRect(int x1, int y1, int x2, int y2)
	{
		::SetRect(this, x1, y1, x2, y2);
	}
};


///////////////////////////////////////////////////////////////////////////////
//
// CXDialogItem
//
class CXDialogTemplate;

class CXDialogItem
{
public:
	DLGITEMTEMPLATE m_dlgItemTemplate;
	enum Econtroltype { ICON = 0x7F, BUTTON, EDITCONTROL, STATICTEXT, CHECKBOX };
	Econtroltype m_controltype;
	TCHAR * m_pszCaption;	//+++1.5

public:
	CXDialogItem(Econtroltype cType);	// default constructor will fill in default values
	CXDialogItem() {};					// default constructor, not to be called directly
	virtual ~CXDialogItem();			//+++1.5

	void AddItem(CXDialogTemplate& dialog, 
				 Econtroltype cType,
				 UINT nID,
				 CXRect* prect = NULL,
				 LPCTSTR pszCaption = NULL);
};


///////////////////////////////////////////////////////////////////////////////
//
// CXDialogTemplate
//

enum EButtons
{
	eAbort = 0,
	eCancel,
	eContinue,
	eDoNotAskAgain,
	eDoNotTellAgain,
	eDoNotShowAgain,
	eHelp,
	eIgnore,
	eIgnoreAll,
	eNo,
	eNoToAll,
	eOK,
	eReport,
	eRetry,
	eSkip,
	eSkipAll,
	eTryAgain,
	eYes,
	eYesToAll,
	LAST_BUTTON
};

class CXDialogTemplate
{
// Constructors
public:
	CXDialogTemplate(HWND hWnd,
					 LPCTSTR lpszMessage,
					 LPCTSTR lpszCaption,
					 UINT nStyle,
					 XMSGBOXPARAMS *pXMB);

	virtual ~CXDialogTemplate();

// Attributes
public:
	LPCTSTR	GetMessageText() const				{ return m_lpszMessage; }
	int&	GetButtonCount()					{ return m_nButton; }
	UINT	GetDefaultButtonId() const			{ return m_nDefId; }
	void	SetDefaultButtonId(UINT nDefId)		{ m_nDefId = nDefId; }
	int		GetDefaultButton() const			{ return m_nDefButton; }
	int		GetReturnValue() const				{ return m_nReturnValue; }
	BOOL	IsEnded() const						{ return m_bEnded; }

// Operations
public:
	int		AddButton(EButtons eButton, UINT nID, int x, int y);
	void	AddItem(CXDialogItem::Econtroltype cType,
					UINT nID,
					CXRect* prect = NULL,
					LPCTSTR pszCaption = NULL);
	void	AddCheckBox(int& x, 
						int& y, 
						CXRect& rect, 
						CXRect& mbrect, 
						CXRect& buttonrow, 
						CXRect& checkboxrect, 
						LPCTSTR lpszButtonCaption);
	int		Display();

// Implementation
protected:
	enum { FirstControlId = 1001};
	enum { MessageControlId = 1001};
	enum { MaxButtonStringSize = 100};
	enum
	{
		ButtonWidth             = 38,		// dialog units  +++1.7
		ButtonTimeoutExtraWidth = 13,		// dialog units  +++1.7
		ButtonHeight            = 12,		// dialog units  +++1.7
		ButtonSpacing           = 6,
		BottomMargin            = 12,
		SpacingBetweenMessageAndButtons = 5,
		DoNotAskAgainHeight     = 10,		// dialog units  +++1.8
											// see http://msdn.microsoft.com/en-us/library/ms997619.aspx
		IdDoNotAskAgain         = 5555,
		IdExHelp                = 300,
		IdExReport              = 301,

		// if you change the value for MaxItems, make sure that the code
		// in CXDialogTemplate remains consistent with your changes.
		MaxItems                = 20,	// max no. of items in the dialog
		MaxCustomButtons        = 8,	// Increased by Corona Labs from 4 to 8.
		MinimalHeight           = 70,
		SpacingSize             = 8,
		MessageSize             = 64*1024,
	};

	CXDialogItem* m_pDlgItemArray[MaxItems];
	XMESSAGEBOX_REPORT_FUNCTION m_lpReportFunc;
	XMESSAGEBOX_CREATED_FUNCTION m_lpCreatedFunc;	// Added by Corona Labs.
	DWORD		m_dwReportUserData;
	LONG_PTR	m_lpCreatedFuncData;				// Added by Corona Labs.
	int			m_Options;
	int			m_nButton;			// current button no.
	int			m_nDefButton;		// Default button
	int			m_nTimeoutSeconds;	// timeout in seconds (before default button selected)
	int			m_nDisabledSeconds;	// disabled time in seconds (before buttons are enabled)
	int			m_X, m_Y;			// initial x,y screen coordinates
	int			m_nButtonWidth;		// button width in pixels
	int			m_nButtonTimeoutExtraWidth;	// timeout button extra width in pixels
	int			m_nButtonHeight;	// button height in pixels
	int			m_nDoNotAskAgainHeight;	// checkbox height in pixels
	int			m_nReturnValue;		// dialog return value
	int			m_nLine;			// line number (for saving DoNotAsk state)
	UINT		m_nMaxID;			// max control id (one more)
	UINT		m_nDefId;			// button number of default button
	UINT		m_nHelpId;			// help context id
	UINT		m_nStyle;			// message box style
	UINT		m_nMessageId;		// message static control id
	BOOL		m_bRightJustifyButtons;	// TRUE = right justify buttons
	BOOL		m_bVistaStyle;		// TRUE = Vista style				//+++1.8
	BOOL		m_bNarrow;			// TRUE = use narrow message box	//+++1.8
	BOOL		m_bEnded;			// TRUE = dialog has ended
	LPCTSTR		m_lpszModule;		// module name (for saving DoNotAsk state)

	HWND		m_hWndOwner;		// handle of owner window				+++1.6
	HINSTANCE	m_hInstanceStrings;	// handle to instance used for loading strings
	HINSTANCE	m_hInstanceIcon;	// handle to instance used for loading icon
	CXRect		m_msgrect;			// message rect for painting

	LPTSTR		m_lpszMessage;		// message buffer
	LPTSTR		m_lpszCaption;		// caption buffer
	TCHAR		m_szCustomButtons[MAX_PATH];	// custom buttons - strings 
												// separated by \n
	TCHAR		m_szCompanyName[MAX_PATH];		// used when saving checkbox state in registry
	HICON		m_hIcon;			// Handle of icon
	HANDLE		m_hFont;			// handle to font for the message box
	COLORREF	m_crText;			// message text color
	COLORREF	m_crBackground;		// message background color
	HBRUSH		m_hBackgroundBrush;	// background brush for message

	DLGTEMPLATE m_dlgTempl;			// message box dialog template

	TCHAR		m_szDefaultButton	[MaxButtonStringSize];	// used for original default 
															// button text, in case of
															// countdown timer

	struct ButtonInfo
	{
		UINT resId;
		TCHAR *pszDefaultText;
	};
	static ButtonInfo g_ButtonText[LAST_BUTTON];	// must be in same order as EButtons enum
	int		m_nButtonWidths[LAST_BUTTON];			// must be in same order as EButtons enum
	TCHAR	m_szButtonText[LAST_BUTTON][MaxButtonStringSize];

	enum EOptions
	{
		DoNotAskAgain    = 0x01,		// include Do Not Ask checkbox
		DoNotTellAgain   = 0x02,		// include Do Not Tell checkbox
		DoNotShowAgain   = 0x04,		// include Do Not Show checkbox
		CancelButton     = 0x08,		// include Cancel button
		OkButton         = 0x10,		// MB_OK used
		CancelOrOkButton = CancelButton | OkButton,
		EDefault         = 0x00
	};

	static INT_PTR CALLBACK MsgBoxDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM me);

	void	LoadButtonStrings();
	void	LoadButtonStringsFromResources(HINSTANCE hInstance);
	BOOL	OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
	int		Option(EOptions id) const	{ return m_Options & id; }
	void	Set(EOptions id)			{ m_Options |= id;};
	BOOL	SetClipboardText(LPCTSTR lpszBuffer);
	void	Unset(EOptions id)			{ m_Options &= ~id;};

	//-[UK
	void LoadUserDefinedButtonStrings( 
		const XMSGBOXPARAMS::CUserDefinedButtonCaptions& udcs);
	//-]UK
};


///////////////////////////////////////////////////////////////////////////////
// default button strings - must be in same order as EButtons enum
struct CXDialogTemplate::ButtonInfo CXDialogTemplate::g_ButtonText[] =		
{
	IDS_ABORT,            _T("&Abort"), 
	IDS_CANCEL,           _T("Cancel"),
	IDS_CONTINUE,         _T("&Continue"),
	IDS_DONOTASKAGAIN,    _T("Don't ask me again"),
	IDS_DONOTTELLAGAIN,   _T("Don't tell me again"),
	IDS_DONOTSHOWAGAIN,   _T("Don't show again"),
	IDS_HELP,             _T("&Help"),
	IDS_IGNORE,           _T("&Ignore"),
	IDS_IGNOREALL,        _T("I&gnore All"),
	IDS_NO,               _T("&No"),
	IDS_NOTOALL,          _T("N&o to All"),
	IDS_OK,               _T("OK"),
	IDS_REPORT,           _T("Re&port"),
	IDS_RETRY,            _T("&Retry"),
	IDS_SKIP,             _T("&Skip"),
	IDS_SKIPALL,          _T("S&kip All"),
	IDS_TRYAGAIN,         _T("&Try Again"),
	IDS_YES,              _T("&Yes"),
	IDS_YESTOALL,         _T("Y&es to All")
};

#ifdef XMESSAGEBOX_AUTO_VISTA_STYLE
///////////////////////////////////////////////////////////////////////////////
// IsVistaOrLater
static BOOL IsVistaOrLater()
{
	BOOL rc = FALSE;

	OSVERSIONINFO osvi = { 0 };
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&osvi))
	{
		if (osvi.dwMajorVersion >= 6)
			rc = TRUE;
	}

	return rc;
}
#endif // XMESSAGEBOX_AUTO_VISTA_STYLE

#ifndef XMESSAGEBOX_DO_NOT_SAVE_CHECKBOX
///////////////////////////////////////////////////////////////////////////////
//
// encode()
//
// Purpose:     Disguise string with simple encoding.  Note that there is
//              no requirement to decode string.
//
// Parameters:  str - pointer null-terminated string to encode
//
// Returns:     LPTSTR - pointer to str
//

#ifndef XMESSAGEBOX_DO_NOT_ENCODE

// folowing string MUST be at least 64 TCHARs
static TCHAR *szAlphabet = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ")
						   _T("0123456789")
						   _T("abcdefghijklmnopqrstuvwxyz")
						   _T("98");
#endif // XMESSAGEBOX_DO_NOT_ENCODE

///////////////////////////////////////////////////////////////////////////////
// encode
static LPTSTR encode(LPTSTR str)
{
#ifndef XMESSAGEBOX_DO_NOT_ENCODE
	for (UINT i = 0; i < _tcslen(str); i++)
	{
		UINT n = (UINT) str[i];
		str[i] = szAlphabet[(n + (5*i)) % 64];
	}
#endif // XMESSAGEBOX_DO_NOT_ENCODE

	return str;
}
#endif // XMESSAGEBOX_DO_NOT_SAVE_CHECKBOX

///////////////////////////////////////////////////////////////////////////////
//
// XMessageBox()
//
// The XMessageBox function creates, displays, and operates a message box.
// The message box contains an application-defined message and title, plus
// any combination of predefined icons, push buttons, and checkboxes.
//
// For more information see
//     http://www.codeproject.com/KB/dialog/xmessagebox.aspx
//
// int XMessageBox(HWND hwnd,                 // handle of owner window
//                 LPCTSTR lpszMessage,       // address of text in message box
//                 LPCTSTR lpszCaption,       // address of title of message box
//                 UINT nStyle,               // style of message box
//                 XMSGBOXPARAMS * pXMB)      // optional parameters
//
// PARAMETERS
//
//     hwnd              - Identifies the owner window of the message box to be
//                         created. If this parameter is NULL, the message box
//                         has no owner window.
//
//     lpszMessage       - Pointer to a null-terminated string containing the
//                         message to be displayed.
//
//     lpszCaption       - Pointer to a null-terminated string used for the
//                         dialog box title. If this parameter is NULL, the
//                         default title Error is used.
//
//     nStyle            - Specifies a set of bit flags that determine the
//                         contents and  behavior of the dialog box. This
//                         parameter can be a combination of flags from the
//                         following groups of flags.
//
//     pXMB              - Pointer to optional parameters.  The parameters
//                         struct XMSGBOXPARAMS is defined in XMessageBox.h.
//
///////////////////////////////////////////////////////////////////////////////

int XMessageBox(HWND hwnd,
				LPCTSTR lpszMessage,
				LPCTSTR lpszCaption /*= NULL*/,
				UINT nStyle /*= MB_OK | MB_ICONEXCLAMATION*/,
				XMSGBOXPARAMS * pXMB /*= NULL*/)
{
	TRACE(_T("in XMessageBox\n"));
	_ASSERTE(lpszMessage);

	if (hwnd == NULL)
	{
		hwnd = ::GetActiveWindow() ;
		if (hwnd != NULL)
		{
			hwnd = ::GetLastActivePopup(hwnd) ;
		}
	};

	if ((nStyle & MB_ICONHAND) && (nStyle & MB_SYSTEMMODAL))
	{
		// NOTE: When an application calls MessageBox and specifies the
		// MB_ICONHAND and MB_SYSTEMMODAL flags for the nStyle parameter,
		// the system displays the resulting message box regardless of
		// available memory.

		return ::MessageBox(hwnd, lpszMessage, lpszCaption, nStyle);
	}


	if (lpszCaption == NULL)			// +++1.6
		lpszCaption = _T("Error");

	XMSGBOXPARAMS xmb;
	if (pXMB)
		xmb = *pXMB;


#ifndef XMESSAGEBOX_DO_NOT_SAVE_CHECKBOX

	// are Do Not Ask styles specified?
	if ((nStyle & MB_DONOTASKAGAIN) || 
		(nStyle & MB_DONOTTELLAGAIN) ||
		(nStyle & MB_DONOTSHOWAGAIN))
	{
		// is module name supplied?
		if (xmb.lpszModule && (xmb.lpszModule[0] != _T('\0')))
		{
			// caller specified Do No Ask style and a module name -
			// check if answer previously saved in registry or ini file

			DWORD dwData = XMessageBoxGetCheckBox(xmb);

			// Note:  dwData will be 0 if either ReadRegistry or 
			// GetPrivateProfileString fail to find key

			if ((dwData & MB_DONOTASKAGAIN) || 
				(dwData & MB_DONOTTELLAGAIN) ||
				(dwData & MB_DONOTSHOWAGAIN))
			{
				TRACE(_T("saved DoNotAsk found, returning 0x%08X\n"), dwData);
				return (int)dwData;
			}
		}
	}

#endif	// #ifndef XMESSAGEBOX_DO_NOT_SAVE_CHECKBOX

	// should be seconds, not milliseconds
	_ASSERTE(xmb.nTimeoutSeconds < 1000);
	if (xmb.nTimeoutSeconds >= 1000)
		xmb.nTimeoutSeconds = 10;

	// should be seconds, not milliseconds
	_ASSERTE(xmb.nDisabledSeconds < 1000);
	if (xmb.nDisabledSeconds >= 1000)
		xmb.nDisabledSeconds = 10;

	// cannot have both disabled and timeout seconds
	_ASSERTE(xmb.nDisabledSeconds == 0 || xmb.nTimeoutSeconds == 0);

	//+++1.8
#ifdef XMESSAGEBOX_AUTO_VISTA_STYLE
	if (IsVistaOrLater())
		xmb.dwOptions |= XMSGBOXPARAMS::VistaStyle;
#endif

	CXDialogTemplate dlg(hwnd,
						 lpszMessage,
						 lpszCaption,
						 nStyle,
						 &xmb);

	if ((nStyle & MB_NOSOUND) == 0)
		::MessageBeep(nStyle & MB_ICONMASK);

	int rc = dlg.Display();

	return rc;
}

///////////////////////////////////////////////////////////////////////////////
// IconProc
LONG CALLBACK IconProc(HWND hwnd, UINT message, WPARAM, LPARAM)
{
	if (message == WM_PAINT)
	{
		PAINTSTRUCT ps;
		HDC hdc;
		hdc = BeginPaint(hwnd, &ps);
		::DrawIcon(hdc, 0, 0, (HICON)(::GetWindowLongPtr(hwnd, GWLP_USERDATA)));
		EndPaint(hwnd, &ps);
	}

	return FALSE;
}



///////////////////////////////////////////////////////////////////////////////
// CXDialogTemplate class

///////////////////////////////////////////////////////////////////////////////
// CXDialogTemplate ctor
CXDialogTemplate::CXDialogTemplate(HWND hWnd,
								   LPCTSTR lpszMessage,
								   LPCTSTR lpszCaption,
								   UINT nStyle,
								   XMSGBOXPARAMS *pXMB) :
	m_nReturnValue(-1),
	m_bEnded(FALSE),
	m_hWndOwner(hWnd),
	m_lpszMessage(NULL),
	m_lpszCaption(NULL),
	m_hBackgroundBrush(0),
	m_nStyle(nStyle),
	m_nHelpId(pXMB->nIdHelp),
	m_hInstanceStrings(pXMB->hInstanceStrings),
	m_hInstanceIcon(pXMB->hInstanceIcon),
	m_lpReportFunc(pXMB->lpReportFunc),
	m_lpCreatedFunc(pXMB->lpCreatedFunc),
	m_dwReportUserData(pXMB->dwReportUserData),
	m_lpCreatedFuncData(pXMB->lpCreatedFuncData),
	m_nTimeoutSeconds(pXMB->nTimeoutSeconds),
	m_nDisabledSeconds(pXMB->nDisabledSeconds),
	m_Options(EDefault),
	m_hIcon(NULL),
	m_hFont(NULL),
	m_nButton(0),				// current button no.
	m_nDefId(1),				// button number of default button
	m_nMaxID(FirstControlId),	// control id
	m_X(pXMB->x),
	m_Y(pXMB->y),
	m_lpszModule(pXMB->lpszModule),
	m_nLine(pXMB->nLine),
	m_crText(pXMB->crText),
	m_crBackground(pXMB->crBackground),
	m_bRightJustifyButtons(pXMB->dwOptions & XMSGBOXPARAMS::RightJustifyButtons),
	m_bVistaStyle(pXMB->dwOptions & XMSGBOXPARAMS::VistaStyle),		//+++1.8
	m_bNarrow(pXMB->dwOptions & XMSGBOXPARAMS::Narrow)				//+++1.8
{
	TRACE(_T("in CXDialogTemplate::CXDialogTemplate\n"));

	if (m_bVistaStyle)	//+++1.8
	{
		m_bRightJustifyButtons = TRUE;
		// set background color only if not already specified
		if (m_crBackground == CLR_INVALID)
			m_crBackground = ::GetSysColor(COLOR_WINDOW);
	}

	if (m_crBackground != CLR_INVALID)
		m_hBackgroundBrush = ::CreateSolidBrush(m_crBackground);

	// translate dialog units to pixels    +++1.7
	int nBaseunitX = LOWORD(GetDialogBaseUnits());
	int nBaseunitY = HIWORD(GetDialogBaseUnits());

	m_nButtonWidth  = MulDiv(ButtonWidth, nBaseunitX, 4);
	m_nButtonHeight = MulDiv(ButtonHeight, nBaseunitY, 8);
	m_nButtonTimeoutExtraWidth = MulDiv(ButtonTimeoutExtraWidth, nBaseunitX, 4);

	if (m_bVistaStyle)		//+++1.8
	{
		// Vista buttons are bigger
		m_nButtonWidth  += MulDiv(6, nBaseunitX, 4);
		m_nButtonHeight += MulDiv(1, nBaseunitY, 8);
	}

	m_nDoNotAskAgainHeight = MulDiv(DoNotAskAgainHeight, nBaseunitY, 8);

	int button_width = m_nButtonWidth;

	// set default button
	switch (nStyle & MB_DEFMASK)
	{
		case MB_DEFBUTTON1 : m_nDefButton = 1; break;
		case MB_DEFBUTTON2 : m_nDefButton = 2; break;
		case MB_DEFBUTTON3 : m_nDefButton = 3; break;
		case MB_DEFBUTTON4 : m_nDefButton = 4; break;
		case MB_DEFBUTTON5 : m_nDefButton = 5; break;
		case MB_DEFBUTTON6 : m_nDefButton = 6; break;
		default:             m_nDefButton = 1; break;
	}

	// store company name to use for saving checkbox state in registry
	memset(m_szCompanyName, 0, sizeof(m_szCompanyName));
	_tcscpy(m_szCompanyName, pXMB->szCompanyName);
		
	// m_szDefaultButton is used to save text for timeout option
	memset(m_szDefaultButton, 0, sizeof(m_szDefaultButton));

	///////////////////////////////////////////////////////////////////////////
	// allocate buffers for message and caption - buffers must be allocated
	// (instead of just using pointers) because we may need to load the strings
	// from resources
	m_lpszMessage = new TCHAR [MessageSize];
	if (m_lpszMessage)
		m_lpszMessage[0] = _T('\0');
	memset(m_lpszMessage, 0, MessageSize*sizeof(TCHAR));

	m_lpszCaption = new TCHAR [MessageSize];
	if (m_lpszCaption)
		m_lpszCaption[0] = _T('\0');
	memset(m_lpszCaption, 0, MessageSize*sizeof(TCHAR));

	///////////////////////////////////////////////////////////////////////////
	// set instance handle for strings
	HINSTANCE hInstanceStrings = m_hInstanceStrings;
	if (!hInstanceStrings)
		hInstanceStrings = ::GetModuleHandle(NULL);

	///////////////////////////////////////////////////////////////////////////
	// load message text from resource or string
	if (lpszMessage)
	{
		if (HIWORD(lpszMessage) == NULL)
		{
			// looks like a resource id
			if (::LoadString(hInstanceStrings,
							 LOWORD((DWORD)lpszMessage),
							 m_lpszMessage,
							 MessageSize-1) == 0)
				m_lpszMessage[0] = _T('\0');
		}
		else
		{
			// looks like a string pointer
			_tcsncpy(m_lpszMessage, lpszMessage, MessageSize-1);
		}
		m_lpszMessage[MessageSize-1] = _T('\0');
	}

	///////////////////////////////////////////////////////////////////////////
	// load caption from resource or string
	if (lpszCaption)
	{
		if (HIWORD(lpszCaption) == NULL)
		{
			// looks like a resource id
			if (::LoadString(hInstanceStrings,
							 LOWORD((DWORD)lpszCaption),
							 m_lpszCaption,
							 MessageSize-1) == 0)
				m_lpszCaption[0] = _T('\0');
		}
		else
		{
			// looks like a string pointer
			_tcsncpy(m_lpszCaption, lpszCaption, MessageSize-1);
		}
		m_lpszCaption[MessageSize-1] = _T('\0');
	}

	///////////////////////////////////////////////////////////////////////////
	// load custom buttons from resource or string
	memset(m_szCustomButtons, 0, sizeof(m_szCustomButtons));

	if (pXMB->nIdCustomButtons)
	{
		// load from resource id
		if (::LoadString(hInstanceStrings,
						 pXMB->nIdCustomButtons,
						 m_szCustomButtons,
						 countof(m_szCustomButtons)-1) == 0)
			m_szCustomButtons[0] = _T('\0');
	}

	if ((m_szCustomButtons[0] == _T('\0')) && (pXMB->szCustomButtons[0] != _T('\0')))
	{
		// load from string
		_tcsncpy(m_szCustomButtons, pXMB->szCustomButtons, countof(m_szCustomButtons)-1);
	}
	m_szCustomButtons[countof(m_szCustomButtons)-1] = _T('\0');

	///////////////////////////////////////////////////////////////////////////
	// load report button caption from resource or string
	memset(&m_szButtonText[eReport], 0, MaxButtonStringSize*sizeof(TCHAR));

	if (pXMB->nIdReportButtonCaption)
	{
		// load from resource id
		if (::LoadString(hInstanceStrings,
						 pXMB->nIdReportButtonCaption,
						 &m_szButtonText[eReport][0],
						 MaxButtonStringSize-1) == 0)
			m_szButtonText[eReport][0] = _T('\0');
	}

	if ((m_szButtonText[eReport][0] == _T('\0')) && (pXMB->szReportButtonCaption[0] != _T('\0')))
	{
		_tcsncpy(&m_szButtonText[eReport][0], pXMB->szReportButtonCaption, MaxButtonStringSize-1);
	}
	m_szButtonText[eReport][MaxButtonStringSize-1] = _T('\0');

	///////////////////////////////////////////////////////////////////////////
	// load button captions 

	LoadButtonStrings();				// assume default strings

	//-[UK
	if (pXMB->bUseUserDefinedButtonCaptions)
		LoadUserDefinedButtonStrings(pXMB->UserDefinedButtonCaptions);
	else if (!(nStyle & MB_NORESOURCE))
		LoadButtonStringsFromResources(hInstanceStrings);	// try to load from resource strings
	//-]UK

	///////////////////////////////////////////////////////////////////////////
	// get dc for drawing
	HDC hdc = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	_ASSERTE(hdc);

	///////////////////////////////////////////////////////////////////////////
	// get message font

	// +++1.6
	struct OLD_NONCLIENTMETRICS
	{
		UINT    cbSize;
		int     iBorderWidth;
		int     iScrollWidth;
		int     iScrollHeight;
		int     iCaptionWidth;
		int     iCaptionHeight;
		LOGFONT lfCaptionFont;
		int     iSmCaptionWidth;
		int     iSmCaptionHeight;
		LOGFONT lfSmCaptionFont;
		int     iMenuWidth;
		int     iMenuHeight;
		LOGFONT lfMenuFont;
		LOGFONT lfStatusFont;
		LOGFONT lfMessageFont;
	};

	// Get the system window message font for use in 
	// the buttons and message

	const UINT cbProperSize = sizeof(OLD_NONCLIENTMETRICS);

	NONCLIENTMETRICS ncm;
	ncm.cbSize = cbProperSize;

#ifdef _DEBUG
	BOOL ok = 
#endif
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, cbProperSize, &ncm, 0);
	_ASSERTE(ok);
	m_hFont = ::CreateFontIndirect(&ncm.lfMessageFont);
	_ASSERTE(m_hFont);

	HFONT hOldFont = (HFONT)::SelectObject(hdc, m_hFont);

	///////////////////////////////////////////////////////////////////////////
	// set width of buttons
	for (int button_index = 0; button_index < LAST_BUTTON; button_index++)
	{
		SIZE size;
		::GetTextExtentPoint32(hdc, m_szButtonText[button_index], 
			_tcslen(m_szButtonText[button_index]), &size);

		_ASSERTE(size.cx != 0);

		int w = size.cx + 20;
		w = (w > button_width) ? w : button_width;

		// allow for wider buttons if timeout specified
		if (m_nTimeoutSeconds)
			w += m_nButtonTimeoutExtraWidth;

		m_nButtonWidths[button_index] = w;
	}

	int nMaxWidth = (::GetSystemMetrics(SM_CXSCREEN) / 2) + 80;	//+++1.5
	if (m_bVistaStyle)											//+++1.8
		nMaxWidth = ::GetSystemMetrics(SM_CXSCREEN) / 2 - 10;	//+++1.8
	if (m_bNarrow)												//+++1.8
		nMaxWidth = ::GetSystemMetrics(SM_CXSCREEN) / 3;		//+++1.8
	if (nStyle & MB_ICONMASK)
		nMaxWidth -= GetSystemMetrics(SM_CXICON) + 2*SpacingSize;

	SetRect(&m_msgrect, 0, 0, nMaxWidth, nMaxWidth);

	///////////////////////////////////////////////////////////////////////////
	// get output size of message text
	::DrawText(hdc, GetMessageText(), -1, &m_msgrect, DT_LEFT | DT_NOPREFIX |
		DT_WORDBREAK | DT_CALCRECT | DT_EXPANDTABS);	//+++1.5

	int nMessageHeight = m_msgrect.Height();
	TRACE(_T("nMessageHeight=%d\n"), nMessageHeight);

	// get height of a single line
	SIZE nLineSize;
	::GetTextExtentPoint32(hdc, _T("My"), 2, &nLineSize);	//+++1.8

	m_msgrect.right  += 12;
	m_msgrect.bottom += 5;

	m_msgrect.left    = 2 * SpacingSize;
	m_msgrect.top     = m_bVistaStyle ? 4 * SpacingSize : 2 * SpacingSize;	//+++1.8
	m_msgrect.right  += 2 * SpacingSize;
	m_msgrect.bottom += m_bVistaStyle ? 7 * SpacingSize : 3 * SpacingSize;	//+++1.8

	// client rect
	CXRect mbrect;
	SetRect(&mbrect, 0, 0,
		m_msgrect.Width() + (2 * SpacingSize),
		m_msgrect.Height() + (2 * SpacingSize));
	if (m_bVistaStyle)
		mbrect.right += 2*SpacingSize;
	if (mbrect.Height() < MinimalHeight)
		mbrect.bottom = MinimalHeight;

	///////////////////////////////////////////////////////////////////////////
	// initialize the DLGTEMPLATE structure
	m_dlgTempl.x = 0;
	m_dlgTempl.y = 0;

	m_dlgTempl.cdit = 0;

	m_dlgTempl.style = WS_CAPTION | WS_VISIBLE | WS_SYSMENU |
		WS_POPUP | DS_MODALFRAME | DS_CENTER;

	m_dlgTempl.dwExtendedStyle = 0;

	if (nStyle & MB_SYSTEMMODAL)
		m_dlgTempl.style |= DS_SYSMODAL;

	for (int j = 0; j < MaxItems; j++)
		m_pDlgItemArray[j] = NULL;

	int x, y;

	CXRect iconrect;
	SetRect(&iconrect, 0, 0, 0, 0);
	CXRect rect;

	HINSTANCE hInstanceIcon = m_hInstanceIcon;
	if (!hInstanceIcon)
		hInstanceIcon = ::GetModuleHandle(NULL);

	///////////////////////////////////////////////////////////////////////////
	// load icon by id or by name
	if (pXMB->nIdIcon)
	{
		m_hIcon = ::LoadIcon(hInstanceIcon, MAKEINTRESOURCE(pXMB->nIdIcon));
	}
	else if (pXMB->szIcon[0] != _T('\0'))
	{
		m_hIcon = ::LoadIcon(hInstanceIcon, pXMB->szIcon);
	}
	else if (nStyle & MB_ICONMASK)
	{
		LPTSTR lpIcon = (LPTSTR)IDI_EXCLAMATION;

		switch (nStyle & MB_ICONMASK)
		{
			case MB_ICONEXCLAMATION: lpIcon = (LPTSTR)IDI_EXCLAMATION; break;
			case MB_ICONHAND:        lpIcon = (LPTSTR)IDI_HAND;        break;
			case MB_ICONQUESTION:    lpIcon = (LPTSTR)IDI_QUESTION;    break;
			case MB_ICONASTERISK:    lpIcon = (LPTSTR)IDI_ASTERISK;    break;
		}

		if (lpIcon)
			m_hIcon = ::LoadIcon(NULL, lpIcon);
	}

	if (m_hIcon)
	{
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		TRACE(_T("cyIcon=%d\n"), cyIcon);

		int icon_x = m_bVistaStyle ? 3*SpacingSize : SpacingSize;		//+++1.8
		int icon_y = m_bVistaStyle ? 3*SpacingSize : SpacingSize;		//+++1.8

		m_msgrect.left  += cxIcon + icon_x;
		m_msgrect.right += cxIcon + icon_x;

		mbrect.right = m_msgrect.right + SpacingSize;
		if (m_bVistaStyle)
			mbrect.right += 2*SpacingSize;

		SetRect(&iconrect, icon_x, icon_y, icon_x + cxIcon + 2, icon_y + cyIcon + 2);
		AddItem(CXDialogItem::STATICTEXT, 1000, &iconrect, _T(""));

		///+++1.8
		if (m_bVistaStyle)
		{
			// check if more than one line
			if (nMessageHeight > nLineSize.cy)	
			{
				// more than one line in message, so align 
				// message top with icon top
				int h = m_msgrect.Height();
				m_msgrect.top = iconrect.top;
				m_msgrect.bottom = m_msgrect.top + h;
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// add message text
	m_nMessageId = m_nMaxID;
	AddItem(CXDialogItem::STATICTEXT, m_nMaxID++, &m_msgrect, GetMessageText());

	///////////////////////////////////////////////////////////////////////////
	// process buttons

	int cItems = 0;
	int nWidthCustomButtons = 0;		//+++1.7
	int nWidthStdButtons = 0;			//+++1.7

	if (m_szCustomButtons[0] == _T('\0'))
	{
		// process standard buttons

		switch (nStyle & MB_TYPEMASK)
		{
			case MB_ABORTRETRYIGNORE: 
				cItems = 3;
				nWidthStdButtons = m_nButtonWidths[eAbort] + 
								   m_nButtonWidths[eRetry] + 
								   m_nButtonWidths[eIgnore];
				break;
			case MB_CANCELTRYCONTINUE: 
				cItems = 3; 
				nWidthStdButtons = m_nButtonWidths[eCancel] + 
								   m_nButtonWidths[eTryAgain] + 
								   m_nButtonWidths[eContinue];
				break;
			case MB_CONTINUEABORT: 
				cItems = 2; 
				nWidthStdButtons = m_nButtonWidths[eContinue] + 
								   m_nButtonWidths[eAbort];
				break;
			case MB_IGNOREIGNOREALLCANCEL: 
				cItems = 3; 
				nWidthStdButtons = m_nButtonWidths[eIgnore] + 
								   m_nButtonWidths[eIgnoreAll] + 
								   m_nButtonWidths[eCancel];
				break;
			case MB_OK: 
				cItems = 1; 
				nWidthStdButtons = m_nButtonWidths[eOK];
				break;
			case MB_OKCANCEL: 
				cItems = 2; 
				nWidthStdButtons = m_nButtonWidths[eOK] + 
								   m_nButtonWidths[eCancel];
				break;
			case MB_RETRYCANCEL: 
				cItems = 2; 
				nWidthStdButtons = m_nButtonWidths[eRetry] + 
								   m_nButtonWidths[eCancel];
				break;
			case MB_SKIPSKIPALLCANCEL: 
				cItems = 3; 
				nWidthStdButtons = m_nButtonWidths[eSkip] + 
								   m_nButtonWidths[eSkipAll] + 
								   m_nButtonWidths[eCancel];
				break;
			case MB_YESNO: 
				cItems = 2; 
				nWidthStdButtons = m_nButtonWidths[eYes] + 
								   m_nButtonWidths[eNo];
				break;
			case MB_YESNOCANCEL: 
				cItems = 3; 
				nWidthStdButtons = m_nButtonWidths[eYes] + 
								   m_nButtonWidths[eNo] + 
								   m_nButtonWidths[eCancel];
				break;
		}

		if (nStyle & MB_YESTOALL)
		{
			_ASSERTE((nStyle & MB_YESNO) || (nStyle & MB_YESNOCANCEL));
			if ((nStyle & MB_YESNO) || (nStyle & MB_YESNOCANCEL))
			{
				nWidthStdButtons += m_nButtonWidths[eYesToAll];
				cItems++;
			}
		}

		if (nStyle & MB_NOTOALL)
		{
			_ASSERTE((nStyle & MB_YESNO) || (nStyle & MB_YESNOCANCEL));
			if ((nStyle & MB_YESNO) || (nStyle & MB_YESNOCANCEL))
			{
				nWidthStdButtons += m_nButtonWidths[eNoToAll];
				cItems++;
			}
		}
	}
	else
	{
		// process custom buttons

		TCHAR szCustomButtons[MAX_PATH];
		_tcscpy(szCustomButtons, m_szCustomButtons);

		int i = 0;
		TCHAR * cp = _tcstok(szCustomButtons, _T("\n"));
		while (cp != NULL)
		{
			if (i >= MaxCustomButtons)
				break;

			SIZE size;
			::GetTextExtentPoint32(hdc, cp, _tcslen(cp), &size);	//+++1.7

			int w = size.cx + 20;
			w = (w > button_width) ? w : button_width;

			// allow for wider buttons if timeout specified
			if (m_nTimeoutSeconds)
				w += m_nButtonTimeoutExtraWidth;

			nWidthCustomButtons += w;		//+++1.7

			cp = _tcstok(NULL, _T("\n"));
			i++;
		}

		cItems += i;
	}

	int nHelpButton = 0;
	if (nStyle & MB_HELP)
	{
		cItems++;
		nWidthStdButtons += m_nButtonWidths[eHelp];	//+++1.7
		nHelpButton = cItems;
	}

	int nReportButton = 0;
	if (m_lpReportFunc)
	{
		cItems++;
		nWidthStdButtons += m_nButtonWidths[eReport];				//+++1.7
		nReportButton = cItems;
	}

	if (nStyle & MB_DONOTASKAGAIN)
		Set(DoNotAskAgain);
	else if (nStyle & MB_DONOTTELLAGAIN)
		Set(DoNotTellAgain);
	else if (nStyle & MB_DONOTSHOWAGAIN)
		Set(DoNotShowAgain);

	_ASSERTE(cItems > 0);

	CXRect buttonrow;
	y = (m_msgrect.bottom > iconrect.bottom) ? m_msgrect.bottom : iconrect.bottom;
	y += SpacingBetweenMessageAndButtons;

	if (m_hIcon || m_hBackgroundBrush)
		y += 9;

	//+++1.7
	int nTotalButtonWidth = nWidthStdButtons + 
							nWidthCustomButtons +
							(ButtonSpacing * (cItems-1));

	SetRect(&buttonrow, 0, y, nTotalButtonWidth, y + m_nButtonHeight);

	if (m_nDefButton > cItems)
		m_nDefButton = 1;

	if (nHelpButton == m_nDefButton)
	{
		TRACE(_T("ERROR:  Help button cannot be default button\n"));
		_ASSERTE(FALSE);
	}

	if (nReportButton == m_nDefButton)
	{
		TRACE(_T("ERROR:  Report button cannot be default button\n"));
		_ASSERTE(FALSE);
	}

	///////////////////////////////////////////////////////////////////////////
	// add buttons

	// get checkbox width   +++1.8

	int checkbox_index = -1;
	if (nStyle & MB_DONOTASKAGAIN)
		checkbox_index = eDoNotAskAgain;
	else if (nStyle & MB_DONOTTELLAGAIN)
		checkbox_index = eDoNotTellAgain;
	else if (nStyle & MB_DONOTSHOWAGAIN)
		checkbox_index = eDoNotShowAgain;

	int nCheckBoxWidth = 0;

	if (checkbox_index != -1)
	{
		nCheckBoxWidth = m_nDoNotAskAgainHeight;	// use height as width of box
		nCheckBoxWidth += m_nButtonWidths[checkbox_index];
	}

	mbrect.bottom = buttonrow.bottom + BottomMargin;

	int bw = buttonrow.Width();
	int bleft = 2 * SpacingSize;
	int bright = bleft + bw;

	if (mbrect.right <= (bright + (2 * SpacingSize)))
		mbrect.right = bright + (2 * SpacingSize);

	if (mbrect.Width() < nCheckBoxWidth)
		mbrect.right = bleft + nCheckBoxWidth;

	x = (mbrect.Width() - bw) / 2;
	y = buttonrow.top;

	if (m_bRightJustifyButtons)
	{
		x = mbrect.right - nTotalButtonWidth - 2*SpacingSize;		//+++1.7
	}

	if (m_szCustomButtons[0] == _T('\0'))
	{
		// no custom buttons

		switch (nStyle & MB_TYPEMASK)
		{
			case MB_OK:
				x += AddButton(eOK, IDOK, x, y);
				Set(OkButton);
				break;

			case MB_OKCANCEL:
				x += AddButton(eOK, IDOK, x, y);
				x += AddButton(eCancel, IDCANCEL, x, y);
				Set(CancelButton);
				break;

			case MB_YESNO:
				x += AddButton(eYes, IDYES, x, y);
				if (nStyle & MB_YESTOALL)
					x += AddButton(eYesToAll, IDYESTOALL, x, y);
				x += AddButton(eNo, IDNO, x, y);
				if (nStyle & MB_NOTOALL)
					x += AddButton(eNoToAll, IDNOTOALL, x, y);
				break;

			case MB_YESNOCANCEL:
				x += AddButton(eYes, IDYES, x, y);
				if (nStyle & MB_YESTOALL)
					x += AddButton(eYesToAll, IDYESTOALL, x, y);
				x += AddButton(eNo, IDNO, x, y);
				if (nStyle & MB_NOTOALL)
					x += AddButton(eNoToAll, IDNOTOALL, x, y);
				x += AddButton(eCancel, IDCANCEL, x, y);
				Set(CancelButton);
				break;

			case MB_ABORTRETRYIGNORE:
				x += AddButton(eAbort, IDABORT, x, y);
				x += AddButton(eRetry, IDRETRY, x, y);
				x += AddButton(eIgnore, IDIGNORE, x, y);
				break;

			case MB_RETRYCANCEL:
				x += AddButton(eRetry, IDRETRY, x, y);
				x += AddButton(eCancel, IDCANCEL, x, y);
				Set(CancelButton);
				break;

			case MB_CONTINUEABORT:
				x += AddButton(eContinue, IDCONTINUE, x, y);
				x += AddButton(eAbort, IDABORT, x, y);
				break;

			case MB_CANCELTRYCONTINUE:
				x += AddButton(eCancel, IDCANCEL, x, y);
				x += AddButton(eTryAgain, IDTRYAGAIN, x, y);
				x += AddButton(eContinue, IDCONTINUE, x, y);
				Set(CancelButton);
				break;

			case MB_SKIPSKIPALLCANCEL:
				x += AddButton(eSkip, IDSKIP, x, y);
				x += AddButton(eSkipAll, IDSKIPALL, x, y);
				x += AddButton(eCancel, IDCANCEL, x, y);
				Set(CancelButton);
				break;

			case MB_IGNOREIGNOREALLCANCEL:
				x += AddButton(eIgnore, IDIGNORE, x, y);
				x += AddButton(eIgnoreAll, IDIGNOREALL, x, y);
				x += AddButton(eCancel, IDCANCEL, x, y);
				Set(CancelButton);
				break;

			default:
				x += AddButton(eOK, IDOK, x, y);
				break;
		}
	}
	else
	{
		// process custom buttons

		TCHAR szCustomButtons[MAX_PATH] = { 0 };
		_tcsncpy(szCustomButtons, m_szCustomButtons, countof(szCustomButtons)-1);

		int i = 0;
		TCHAR * cp = _tcstok(szCustomButtons, _T("\n"));
		while (cp != NULL)
		{
			if (i >= MaxCustomButtons)
				break;

			SIZE size;
			::GetTextExtentPoint32(hdc, cp, _tcslen(cp), &size);	//+++1.7

			int w = size.cx + 20;

			w = (w > button_width) ? w : button_width;

			// allow for wider buttons if timeout specified
			if (m_nTimeoutSeconds)
				w += m_nButtonTimeoutExtraWidth;

			rect.SetRect(x, y, x + w, y + m_nButtonHeight);
			AddItem(CXDialogItem::BUTTON, IDCUSTOM1 + i, &rect, cp);
			x += w + ButtonSpacing;
			cp = _tcstok(NULL, _T("\n"));
			i++;
		}
	}

	if (nStyle & MB_HELP)
	{
		x += AddButton(eHelp, IdExHelp, x, y);
	}

	if (m_lpReportFunc)
	{
		x += AddButton(eReport, IdExReport, x, y);
	}

	///////////////////////////////////////////////////////////////////////////
	// add checkbox

	TRACE(_T("m_nDoNotAskAgainHeight=%d\n"), m_nDoNotAskAgainHeight);
	if (checkbox_index != -1)		//+++1.8
	{
		CXRect checkboxrect;
		SetRect(&checkboxrect, 0, 0, nCheckBoxWidth, m_nDoNotAskAgainHeight);
		AddCheckBox(x, y, rect, mbrect, buttonrow, checkboxrect, 
			m_szButtonText[checkbox_index]);
	}

	if (buttonrow.bottom >= mbrect.bottom)
		mbrect.bottom = buttonrow.bottom + (2 * SpacingSize);

	if (mbrect.right < (buttonrow.right + (2 * SpacingSize)))
		mbrect.right = buttonrow.right + (2 * SpacingSize);

	m_dlgTempl.x = 0;
	m_dlgTempl.y = 0;
	m_dlgTempl.cx = (short)((mbrect.Width() * 4) / nBaseunitX);			//+++1.8
	m_dlgTempl.cy = (short)((mbrect.Height() * 8) / nBaseunitY);

	::SelectObject(hdc, hOldFont);
	::DeleteDC(hdc);
}

///////////////////////////////////////////////////////////////////////////////
// CXDialogTemplate dtor
CXDialogTemplate::~CXDialogTemplate()
{
	TRACE(_T("in CXDialogTemplate::~CXDialogTemplate\n"));
	if (m_hIcon)
		DestroyIcon(m_hIcon);
	m_hIcon = 0;

	if (m_hFont)
		::DeleteObject(m_hFont);

	if (m_hBackgroundBrush)
		::DeleteObject(m_hBackgroundBrush);
	m_hBackgroundBrush = 0;

	for (int i = 0; i < MaxItems; i++)
	{
		if (m_pDlgItemArray[i])
		{
			delete m_pDlgItemArray[i];
			m_pDlgItemArray[i] = NULL;
		}
	}

	if (m_lpszMessage)
		delete [] m_lpszMessage;
	m_lpszMessage = NULL;

	if (m_lpszCaption)
		delete [] m_lpszCaption;
	m_lpszCaption = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// AddButton
int CXDialogTemplate::AddButton(EButtons eButton, UINT nID, int x, int y)
{
	CXRect rect;
	SetRect(&rect, x, y, x + m_nButtonWidths[eButton], y + m_nButtonHeight);
	AddItem(CXDialogItem::BUTTON, nID, &rect, m_szButtonText[eButton]);
	return m_nButtonWidths[eButton] + ButtonSpacing;
}

///////////////////////////////////////////////////////////////////////////////
// AddCheckBox
void CXDialogTemplate::AddCheckBox(int& x, 
								   int& y, 
								   CXRect& rect, 
								   CXRect& mbrect, 
								   CXRect& buttonrow, 
								   CXRect& checkboxrect, 
								   LPCTSTR lpszButtonCaption)
{
	x = 2 * ButtonSpacing + 5;
	y += m_nButtonHeight + (2 * ButtonSpacing);

	int w = checkboxrect.Width();

	SetRect(&rect, x, y, x + w, y + m_nDoNotAskAgainHeight);
	AddItem(CXDialogItem::CHECKBOX, IdDoNotAskAgain, &rect, lpszButtonCaption);

	buttonrow.bottom = y + m_nDoNotAskAgainHeight;
	mbrect.bottom = buttonrow.bottom + SpacingSize;
	if (mbrect.Width() < (x + w))
		mbrect.right = mbrect.left + x + w;
}

//-[UK
void CXDialogTemplate::LoadUserDefinedButtonStrings( 
	const XMSGBOXPARAMS::CUserDefinedButtonCaptions& udcs)
{
	TRACE(_T("in CXDialogTemplate::LoadUserDefinedButtonStrings\n"));

	if (udcs.szAbort[0])
		_tcscpy(m_szButtonText[eAbort],          udcs.szAbort);
	if (udcs.szCancel[0])
		_tcscpy(m_szButtonText[eCancel],         udcs.szCancel);
	if (udcs.szContinue[0])
		_tcscpy(m_szButtonText[eContinue],       udcs.szContinue);
	if (udcs.szDoNotAskAgain[0])
		_tcscpy(m_szButtonText[eDoNotAskAgain],  udcs.szDoNotAskAgain);
	if (udcs.szDoNotTellAgain[0])
		_tcscpy(m_szButtonText[eDoNotTellAgain], udcs.szDoNotTellAgain);
	if (udcs.szDoNotShowAgain[0])
		_tcscpy(m_szButtonText[eDoNotShowAgain], udcs.szDoNotShowAgain);
	if (udcs.szHelp[0])
		_tcscpy(m_szButtonText[eHelp],           udcs.szHelp);
	if (udcs.szIgnore[0])
		_tcscpy(m_szButtonText[eIgnore],         udcs.szIgnore);
	if (udcs.szIgnoreAll[0])
		_tcscpy(m_szButtonText[eIgnoreAll],      udcs.szIgnoreAll);
	if (udcs.szNo[0])
		_tcscpy(m_szButtonText[eNo],             udcs.szNo);
	if (udcs.szNoToAll[0])
		_tcscpy(m_szButtonText[eNoToAll],        udcs.szNoToAll);
	if (udcs.szOK[0])
		_tcscpy(m_szButtonText[eOK],             udcs.szOK);
	if (m_szButtonText[eReport][0] == _T('\0'))
		if (udcs.szReport[0])
			_tcscpy(m_szButtonText[eReport],     udcs.szReport);
	if (udcs.szRetry[0])
		_tcscpy(m_szButtonText[eRetry],          udcs.szRetry);
	if (udcs.szSkip[0])
		_tcscpy(m_szButtonText[eSkip],           udcs.szSkip);
	if (udcs.szSkipAll[0])
		_tcscpy(m_szButtonText[eSkipAll],        udcs.szSkipAll);
	if (udcs.szTryAgain[0])
		_tcscpy(m_szButtonText[eTryAgain],       udcs.szTryAgain);
	if (udcs.szYes[0])
		_tcscpy(m_szButtonText[eYes],            udcs.szYes);
	if (udcs.szYesToAll[0])
		_tcscpy(m_szButtonText[eYesToAll],       udcs.szYesToAll);
}
//-]UK

///////////////////////////////////////////////////////////////////////////////
// LoadButtonStrings - load the default strings
void CXDialogTemplate::LoadButtonStrings()
{
	TRACE(_T("in CXDialogTemplate::LoadButtonStrings\n"));

	for (int index = 0; index < LAST_BUTTON; index++)
	{
		if (index == eReport)
		{
			// Report button text may already be loaded
			if (m_szButtonText[index][0] == _T('\0'))
				_tcscpy(m_szButtonText[index], g_ButtonText[index].pszDefaultText);
		}
		else
		{
			_tcscpy(m_szButtonText[index], g_ButtonText[index].pszDefaultText);
		}
		m_szButtonText[index][MaxButtonStringSize-1] = _T('\0');
	}
}

///////////////////////////////////////////////////////////////////////////////
// LoadButtonStringsFromResources
void CXDialogTemplate::LoadButtonStringsFromResources(HINSTANCE hInstance)
{
	TRACE(_T("in CXDialogTemplate::LoadButtonStringsFromResources\n"));

	_ASSERTE(hInstance);

	for (int index = 0; index < LAST_BUTTON; index++)
	{
		int rc = 0;
		if (index == eReport)
		{
			// Report button text may already be loaded
			if (m_szButtonText[index][0] == _T('\0'))
				rc = ::LoadString(hInstance, g_ButtonText[index].resId, 
						m_szButtonText[index], MaxButtonStringSize);
			else
				rc = 1;
		}
		else
		{
			rc = ::LoadString(hInstance, g_ButtonText[index].resId, 
					m_szButtonText[index], MaxButtonStringSize);
		}
		if (rc == 0)
			_tcscpy(m_szButtonText[index], g_ButtonText[index].pszDefaultText);
		m_szButtonText[index][MaxButtonStringSize-1] = _T('\0');
	}
}

///////////////////////////////////////////////////////////////////////////////
// MsgBoxDlgProc
INT_PTR CALLBACK CXDialogTemplate::MsgBoxDlgProc(HWND hwnd,
												 UINT message,
												 WPARAM wParam,
												 LPARAM lParam)
{
	CXDialogTemplate* Me = (CXDialogTemplate*) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);
	HWND hwndChild = 0;

	switch (message)
	{
		case WM_CTLCOLORSTATIC:		//+++1.8
		{
			if (Me->m_hBackgroundBrush)
			{
				HWND hStatic = ::GetDlgItem(hwnd, Me->m_nMessageId);
				if (Me->m_crBackground == CLR_INVALID)
					::SetBkColor((HDC)wParam, ::GetSysColor(COLOR_WINDOW));
				else
					::SetBkColor((HDC)wParam, Me->m_crBackground);
				if (Me->m_crText == CLR_INVALID)
					::SetTextColor((HDC)wParam, ::GetSysColor(COLOR_WINDOWTEXT));
				else
					::SetTextColor((HDC)wParam, Me->m_crText);
				if (hStatic && (hStatic == (HWND)lParam))
					return (LRESULT)Me->m_hBackgroundBrush;
			}
			return FALSE;
		}

		case WM_ERASEBKGND:			//+++1.8
		{
			if (Me->m_hBackgroundBrush)
			{
				HDC hdc = (HDC) wParam;

				CXRect rect;
				::GetClientRect(hwnd, &rect);

				// first fill entire client area 
				HBRUSH hBrush = ::CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
				::FillRect(hdc, &rect, hBrush);
				::DeleteObject(hBrush);

				// next fill message background 
				rect.bottom = Me->m_msgrect.bottom;
				if (Me->m_hBackgroundBrush)
					::FillRect(hdc, &rect, Me->m_hBackgroundBrush);

				return TRUE;
			}
			return FALSE;
		}

		case WM_INITDIALOG:
		{
			TRACE(_T("in CXDialogTemplate::MsgBoxDlgProc - WM_INITDIALOG\n"));
			//m_hWnd = hwnd;

			::SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);	// save it for the others
			Me = (CXDialogTemplate*) lParam;
			_ASSERTE(Me);

			HDC hdc = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
			_ASSERTE(hdc);

			::SelectObject(hdc, Me->m_hFont);
			::DeleteDC(hdc);
			UINT nID;

			for (nID = FirstControlId; nID < Me->m_nMaxID; nID++)
			{
				hwndChild = ::GetDlgItem(hwnd, nID);
				if (::IsWindow(hwndChild))
					::SendMessage(hwndChild, WM_SETFONT, (WPARAM)Me->m_hFont, 0);
				else
					break;
			}
			for (nID = 1; nID <= ID_XMESSAGEBOX_LAST_ID; nID++)
			{
				hwndChild = ::GetDlgItem(hwnd, nID);
				if (hwndChild && ::IsWindow(hwndChild))
				{
					::SendMessage(hwndChild, WM_SETFONT, (WPARAM)Me->m_hFont, 0);

					// disable all buttons if disabled seconds specified
					if (Me->m_nDisabledSeconds)
					{
						TCHAR szClassName[MAX_PATH];
						::GetClassName(hwndChild, szClassName, countof(szClassName)-2);
						if (_tcsicmp(szClassName, _T("Button")) == 0)
						{
							::EnableWindow(hwndChild, FALSE);
						}
						::EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_GRAYED);
					}
				}
			}

			hwndChild = ::GetDlgItem(hwnd, IdExHelp);
			if (hwndChild && ::IsWindow(hwndChild))
				::SendMessage(hwndChild, WM_SETFONT, (WPARAM)Me->m_hFont, 0);

			hwndChild = ::GetDlgItem(hwnd, IdDoNotAskAgain);
			if (hwndChild && ::IsWindow(hwndChild))
			{
				::SendMessage(hwndChild, WM_SETFONT, (WPARAM)Me->m_hFont, 0);
				CheckDlgButton(hwnd, IdDoNotAskAgain, 0);
			}

			hwndChild = ::GetDlgItem(hwnd, IdExReport);
			if (hwndChild && ::IsWindow(hwndChild))
				::SendMessage(hwndChild, WM_SETFONT, (WPARAM)Me->m_hFont, 0);

			hwndChild = ::GetDlgItem(hwnd, Me->m_nDefId);
			if (hwndChild && ::IsWindow(hwndChild))
				::SetFocus(hwndChild);

			// disable close button just like real MessageBox
			if (!Me->Option(CancelOrOkButton))
				::EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_GRAYED);

			if (Me->m_hIcon)
			{
				HWND hwndIcon;

				hwndIcon = ::GetDlgItem(hwnd, 1000);
				::SetWindowLongPtr(hwndIcon, GWLP_WNDPROC, (LONG_PTR) IconProc);
				::SetWindowLongPtr(hwndIcon, GWLP_USERDATA, (LONG_PTR) Me->m_hIcon);
			}

			if (Me->m_nStyle & MB_TOPMOST)
				::SetWindowPos(hwnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);

			if (Me->m_nStyle & MB_SETFOREGROUND)
				::SetForegroundWindow(hwnd);

			if (Me->m_X || Me->m_Y)
			{
				// caller has specified screen coordinates
				CXRect dlgRect;
				::GetWindowRect(hwnd, &dlgRect);
				::MoveWindow(hwnd, Me->m_X, Me->m_Y, dlgRect.Width(), dlgRect.Height(), TRUE);
			}
			else if (GetParent(hwnd) != NULL)
			{
				// code to center dialog (necessary if not called by MFC app)
				// [suggested by Tom Wright]
				CXRect mainRect, dlgRect;
				::GetWindowRect(::GetParent(hwnd), &mainRect);
				::GetWindowRect(hwnd, &dlgRect);
				int x = (mainRect.right  + mainRect.left)/2 - dlgRect.Width()/2;
				int y =	(mainRect.bottom + mainRect.top) /2 - dlgRect.Height()/2;
				::MoveWindow(hwnd, x, y, dlgRect.Width(), dlgRect.Height(), TRUE);
			}

			// display seconds countdown if timeout specified
			if (Me->m_nTimeoutSeconds > 0)
				::SetTimer(hwnd, 1, 1000, NULL);

			if (Me->m_nDisabledSeconds > 0)
				::SetTimer(hwnd, 2, 1000, NULL);

			::SetForegroundWindow(hwnd);

			return FALSE;

		} // case WM_INITDIALOG

		case WM_COMMAND:
		{
			TRACE(_T("WM_COMMAND\n"));
			// user clicked on something - stop the timer
			::KillTimer(hwnd, 1);

			if (Me->GetDefaultButtonId())
			{
				HWND hwndDefButton = ::GetDlgItem(hwnd, Me->GetDefaultButtonId());
				if (hwndDefButton && ::IsWindow(hwndDefButton))
				{
					if (Me->m_szDefaultButton[0] != _T('\0'))
					{
						::SetWindowText(hwndDefButton, Me->m_szDefaultButton);
					}
				}
			}

			switch (wParam)
			{
				case IDCLOSE:
					TRACE(_T("IDCLOSE\n"));
					return TRUE;

				case IDCANCEL:
					TRACE(_T("IDCANCEL\n"));
#if 0
					if (Me->Option(CancelButton))
					{
						Me->m_bEnded = TRUE;
						Me->m_nReturnValue = IDCANCEL;
					}
					else if (Me->Option(OkButton))
					{
						Me->m_bEnded = TRUE;
						Me->m_nReturnValue = IDOK;
					}
					return FALSE;
#else
					// CoronaLabs:
					// Always handle the [x] cancel button.
					if (Me->Option(OkButton))
					{
						Me->m_nReturnValue = IDOK;
					}
					else
					{
						Me->m_nReturnValue = IDCANCEL;
					}
					Me->m_bEnded = TRUE;
					return TRUE;
#endif

				case IdExHelp:
				{
					TCHAR szBuf[_MAX_PATH*2] = { 0 };
					::GetModuleFileName(NULL, szBuf, countof(szBuf) - 1);
					if (_tcslen(szBuf) > 0)
					{
						TCHAR *cp = _tcsrchr(szBuf, _T('.'));
						if (cp)
						{
							_tcscpy(cp, _T(".hlp"));
							::WinHelp(hwnd, szBuf,
								(Me->m_nHelpId == 0) ? HELP_PARTIALKEY : HELP_CONTEXT,
								Me->m_nHelpId);
						}
					}
					return FALSE;
				}

				case IdExReport:
				{
					if (Me->m_lpReportFunc)
						Me->m_lpReportFunc(Me->GetMessageText(), Me->m_dwReportUserData);
					return FALSE;
				}

				case IdDoNotAskAgain: //IdDoNotAskAgain & DoNotTellAgain share the same id!!
					return FALSE;

				default:
				{
					hwndChild = ::GetDlgItem(hwnd, IdDoNotAskAgain);
					BOOL bFlag = FALSE;

					if (hwndChild && ::IsWindow(hwndChild))
						bFlag = (BOOL)::SendMessage(hwndChild, BM_GETCHECK, 0, 0);

					if (Me->Option(DoNotAskAgain))
						wParam |= bFlag ? MB_DONOTASKAGAIN : 0;
					else if (Me->Option(DoNotTellAgain))
						wParam |= bFlag ? MB_DONOTTELLAGAIN : 0;
					else if (Me->Option(DoNotShowAgain))
						wParam |= bFlag ? MB_DONOTSHOWAGAIN : 0;

#ifndef XMESSAGEBOX_DO_NOT_SAVE_CHECKBOX

					// save return code in ini file if Do Not Ask style and
					// module name were specified

					if (bFlag && Me->m_lpszModule &&
						(Me->m_lpszModule[0] != _T('\0')))
					{
						TCHAR szPathName[MAX_PATH*2];

						// get full path to ini file
						szPathName[0] = _T('\0');
						::GetModuleFileName(NULL, szPathName, countof(szPathName)-1);

						TCHAR *cp = _tcsrchr(szPathName, _T('\\'));
						if (cp != NULL)
							*(cp+1) = _T('\0');
						_tcscat(szPathName, XMESSAGEBOX_INI_FILE);

						// key is module name and line
						TCHAR szKey[MAX_PATH*2];
						_tcscpy(szKey, Me->m_lpszModule);
						TRACE(_T("szKey=<%s>\n"), szKey);

						encode(szKey);		// simple encoding to obscure module name

						TCHAR szLine[100];
						szLine[0] = _T('\0');
						_tcscat(szKey, _itot(Me->m_nLine, szLine, 10));

						TRACE(_T("szKey=<%s>\n"), szKey);

#ifndef XMESSAGEBOX_USE_PROFILE_FILE

						WriteRegistry(Me->m_szCompanyName, szKey, wParam);
						TRACE(_T("writing data to registry: wParam=0x%08X\n"), wParam);

#else

						// data string is hex value of XMessageBox return code
						TCHAR szData[100];
						_stprintf(szData, _T("%08X"), wParam);


						::WritePrivateProfileString(_T("DoNotAsk"),		// section name
													szKey,				// key name
													szData,				// string to add
													szPathName);		// initialization file
#endif	// XMESSAGEBOX_USE_PROFILE_FILE



#ifdef _DEBUG
						// verify that we can read data

						DWORD dwData = 0;

#ifndef XMESSAGEBOX_USE_PROFILE_FILE

						// read from registry 

						dwData = ReadRegistry(Me->m_szCompanyName, szKey);
						TRACE(_T("ReadRegistry: dwData=0x%08X\n"), dwData);

#else
						// read from ini file

						TCHAR szBuf[100];
						::GetPrivateProfileString(_T("DoNotAsk"),	// section name
												  szKey,			// key name
												  _T(""),			// default string
												  szBuf,			// destination buffer
												  countof(szBuf)-1,	// size of destination buffer
												  szPathName);		// initialization file name

						dwData = _tcstoul(szBuf, NULL, 16);
						TRACE(_T("szBuf=<%s>  dwData=0x%08X\n"), szBuf, dwData);

#endif	// XMESSAGEBOX_USE_PROFILE_FILE

						_ASSERTE(dwData == (DWORD) wParam);

#endif	// _DEBUG
					}
#endif	// XMESSAGEBOX_DO_NOT_SAVE_CHECKBOX

					Me->m_bEnded = TRUE;
					Me->m_nReturnValue = wParam;

					return FALSE;

				} // default

			} // switch (wParam)

		} // case WM_COMMAND

		case WM_LBUTTONDOWN:
		case WM_NCLBUTTONDOWN:
		{
			// user clicked on dialog or titlebar - stop the timer
			::KillTimer(hwnd, 1);

			if (Me->GetDefaultButtonId())
			{
				HWND hwndDefButton = ::GetDlgItem(hwnd, Me->GetDefaultButtonId());
				if (hwndDefButton && ::IsWindow(hwndDefButton))
				{
					if (Me->m_szDefaultButton[0] != _T('\0'))
					{
						::SetWindowText(hwndDefButton, Me->m_szDefaultButton);
					}
				}
			}

			return FALSE;
		}

		case WM_TIMER:		// used for timeout
		{
			if (wParam == 1)			// timeout timer
			{
				if (Me->m_nTimeoutSeconds <= 0)
				{
					::KillTimer(hwnd, wParam);

					// time's up, select default button
					::SendMessage(hwnd, WM_COMMAND, 
						Me->GetDefaultButtonId() | MB_TIMEOUT, 0);

					return FALSE;
				}

				if (Me->GetDefaultButtonId() == 0)
					return FALSE;

				HWND hwndDefButton = ::GetDlgItem(hwnd, Me->GetDefaultButtonId());
				if (hwndDefButton == NULL || !::IsWindow(hwndDefButton))
					return FALSE;

				if (Me->m_szDefaultButton[0] == _T('\0'))
				{
					// first time - get text of default button
					::GetWindowText(hwndDefButton, Me->m_szDefaultButton, 
						MaxButtonStringSize);
				}

				TCHAR szButtonText[MaxButtonStringSize*2];
				_stprintf(szButtonText, XMESSAGEBOX_TIMEOUT_TEXT_FORMAT, 
					Me->m_szDefaultButton, Me->m_nTimeoutSeconds);

				::SetWindowText(hwndDefButton, szButtonText);

				Me->m_nTimeoutSeconds--;
			}
			else if (wParam == 2)		// disabled timer
			{
				if (Me->m_nDisabledSeconds <= 0)
				{
					// disabled timer has expired, enable all controls

					::KillTimer(hwnd, wParam);

					for (UINT nID = 1; nID <= ID_XMESSAGEBOX_LAST_ID; nID++)
					{
						hwndChild = ::GetDlgItem(hwnd, nID);
						if (hwndChild && ::IsWindow(hwndChild))
						{
							// enable all buttons
							TCHAR szClassName[MAX_PATH];
							::GetClassName(hwndChild, szClassName, countof(szClassName)-2);
							if (_tcsicmp(szClassName, _T("Button")) == 0)
							{
								::EnableWindow(hwndChild, TRUE);
							}
						}
					} // for

					if (Me->Option(CancelOrOkButton))
						::EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_ENABLED);

					// restore original caption
					::SetWindowText(hwnd, Me->m_lpszCaption);

					::RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW);
				}
				else
				{
#ifndef XMESSAGEBOX_NO_DISABLED_COUNTDOWN
					// update caption with disabled timer countdown
					if (Me->m_lpszCaption)
					{
						TCHAR *pszCaption = new TCHAR [_tcslen(Me->m_lpszCaption) + 100];
						_tcscpy(pszCaption, Me->m_lpszCaption);
						_tcscat(pszCaption, _T(" "));
						_itot(Me->m_nDisabledSeconds, &pszCaption[_tcslen(pszCaption)], 10);
						::SetWindowText(hwnd, pszCaption);
						delete [] pszCaption;
					}
#endif
					Me->m_nDisabledSeconds--;
				}
			}
			return FALSE;
		}

	} // switch (message)

	return FALSE;
}

//1.6+++
///////////////////////////////////////////////////////////////////////////////
// SetClipboardText - Places text on the clipboard
///////////////////////////////////////////////////////////////////////////////
//
// Parameters:
//	lpszBuffer - pointer to a string to put on the clipboard
//
// Return Values:
//	TRUE       - Text was successfully copied onto clipboard
//	FALSE      - Text not copied
//
BOOL CXDialogTemplate::SetClipboardText(LPCTSTR lpszBuffer)
{
	BOOL bSuccess = FALSE;

	// First, open the clipboard. OpenClipboard() takes one
	// parameter, the handle of the window that will temporarily
	// be it's owner. If NULL is passed, the current process
	// is assumed. After opening, empty the clipboard so we
	// can put our text on it.
	if (::OpenClipboard(NULL))
	{
		::EmptyClipboard();

		// Get the size of the string in the buffer that was
		// passed into the function, so we know how much global
		// memory to allocate for the string.
		size_t nSize = _tcslen(lpszBuffer);

		// Allocate the memory for the string.
		HGLOBAL hGlobal = ::GlobalAlloc(GMEM_ZEROINIT, (nSize+1)*sizeof(TCHAR));

		// If we got any error during the memory allocation,
		// we have been returned a NULL handle.
		if (hGlobal)
		{
			// Now we have a global memory handle to the text
			// stored on the clipboard. We have to lock this global
			// handle so that we have access to it.
			LPTSTR lpszData = (LPTSTR) ::GlobalLock(hGlobal);

			if (lpszData)
			{
				// Now, copy the text from the buffer into the allocated
				// global memory pointer
				_tcscpy(lpszData, lpszBuffer);

				// Now, simply unlock the global memory pointer,
				// set the clipboard data type and pointer,
				// and close the clipboard.
				::GlobalUnlock(hGlobal);
#ifdef _UNICODE
				::SetClipboardData(CF_UNICODETEXT, hGlobal);
#else
				::SetClipboardData(CF_TEXT, hGlobal);
#endif
				bSuccess = TRUE;
			}
		}
		::CloseClipboard();
	}

	return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////
// CXDialogTemplate::AddItem
void CXDialogTemplate::AddItem(CXDialogItem::Econtroltype cType,
							   UINT nID,
							   CXRect* prect,
							   LPCTSTR pszCaption)
{
	_ASSERTE(m_pDlgItemArray[m_dlgTempl.cdit] == NULL);

	CXDialogItem::Econtroltype ct = cType;

	if (ct == CXDialogItem::CHECKBOX)
		ct = CXDialogItem::BUTTON;

	m_pDlgItemArray[m_dlgTempl.cdit] = new CXDialogItem(ct);
	_ASSERTE(m_pDlgItemArray[m_dlgTempl.cdit]);

	m_pDlgItemArray[m_dlgTempl.cdit]->AddItem(*this, cType, nID, prect, pszCaption);

	m_dlgTempl.cdit++;
	_ASSERTE(m_dlgTempl.cdit < MaxItems);
}

///////////////////////////////////////////////////////////////////////////////
// OnKeyDown - returns TRUE if Ctrl-C processed
BOOL CXDialogTemplate::OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	//TRACE(_T("in CXDialogTemplate::OnKeyDown:  0x%x  0x%x\n"), wParam, lParam);

	BOOL rc = FALSE;

	// convert virtual key code
	TCHAR ch = (TCHAR) wParam;

	// if the most significant bit is set, the key is down
	BOOL bCtrlIsDown = 	GetAsyncKeyState(VK_CONTROL) < 0;

	if (bCtrlIsDown && (ch == _T('C')))
	{
		rc = TRUE;

		// user hit Ctrl-C -- copy contents to clipboard
		if (::IsWindow(hWnd))
		{
			TRACE(_T("_____ CXDialogTemplate::OnKeyDown: Ctrl-C\n"));
			TCHAR *pszDivider = _T("---------------------------\r\n");
			TCHAR *pszText = new TCHAR [8192];
			_tcscpy(pszText, pszDivider);
			size_t n = _tcslen(pszText);
			::GetWindowText(hWnd, &pszText[n], 1000);		// caption
			_tcscat(pszText, _T("\r\n"));
			_tcscat(pszText, pszDivider);
			HWND hMsgHwnd = ::GetDlgItem(hWnd, MessageControlId);
			if (::IsWindow(hMsgHwnd))
			{
				n = _tcslen(pszText);
				::GetWindowText(hMsgHwnd, &pszText[n], 4000);	// message
				_tcscat(pszText, _T("\r\n"));
				_tcscat(pszText, pszDivider);
			}

			TCHAR szClassName[MAX_PATH];
			TCHAR szButton[200];
			HWND hwndChild = ::GetWindow(hWnd, GW_CHILD);
			while (hwndChild)
			{
				if (::IsWindow(hwndChild))
				{
					::GetClassName(hwndChild, szClassName, countof(szClassName)-2);
					if (_tcsicmp(szClassName, _T("Button")) == 0)
					{
						::GetWindowText(hwndChild, szButton, countof(szButton)-1);	// button text
						size_t i = _tcslen(pszText);
						TCHAR *cp = szButton;
						while (*cp)
						{
							if (*cp != _T('&'))
								pszText[i++] = *cp;
							cp++;
						}
						pszText[i] = 0;
						_tcscat(pszText, _T("   "));
					}
				}
				hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
			}
			_tcscat(pszText, _T("\r\n"));
			_tcscat(pszText, pszDivider);
			SetClipboardText(pszText);
			delete [] pszText;
		}
	}
	return rc;
}

///////////////////////////////////////////////////////////////////////////////
// CXDialogTemplate::Display
int CXDialogTemplate::Display()
{
	TRACE(_T("in CXDialogTemplate::Display\n"));
	// The first step is to allocate memory to define the dialog. The information to be
	// stored in the allocated buffer is the following:
	//
	// 1. DLGTEMPLATE structure
	//    typedef struct
	//    {
	//       DWORD style;
	//       DWORD dwExtendedStyle;
	//       WORD cdit;
	//       short x;
	//       short y;
	//       short cx;
	//       short cy;
	//    } DLGTEMPLATE;
	// 2. 0x0000 (Word) indicating the dialog has no menu
	// 3. 0x0000 (Word) Let windows assign default class to the dialog
	// 4. (Caption) Null terminated unicode string
	// 5. 0x000B (size of the font to be used)
	// 6. "MS Sans Serif" (name of the typeface to be used)
	// 7. DLGITEMTEMPLATE structure for the button (HAS TO BE DWORD ALIGNED)
	//    typedef struct
	//    {
	//       DWORD style;
	//       DWORD dwExtendedStyle;
	//       short x;
	//       short y;
	//       short cx;
	//       short cy;
	//       WORD id;
	//    } DLGITEMTEMPLATE;
	// 8. 0x0080 to indicate the control is a button
	// 9. (Title). Unicode null terminated string with the caption
	// 10. 0x0000 0 extra bytes of data for this control
	// 11. DLGITEMTEMPLATE structure for the Static Text (HAS TO BE DWORD ALIGNED)
	// 12. 0x0081 to indicate the control is static text
	// 13. (Title). Unicode null terminated string with the text
	// 14. 0x0000. 0 extra bytes of data for this control


	TCHAR szTitle[1024];
	_tcsncpy(szTitle, m_lpszCaption, countof(szTitle)-1);
	szTitle[countof(szTitle)-1] = _T('\0');
	int nTitleLen = (int)_tcslen(szTitle);

	int i = 0;

	int nBufferSize = sizeof(DLGTEMPLATE) +
		(2 * sizeof(WORD)) + // menu and class
		((nTitleLen + 1) * sizeof(WCHAR));

	// NOTE - font is set in MsgBoxDlgProc

	nBufferSize = (nBufferSize + 3) & ~3; // adjust size to make
	// first control DWORD aligned

	// loop to calculate size of buffer we need -
	// add size of each control:
	// sizeof(DLGITEMTEMPLATE) +
	// sizeof(WORD) + // atom value flag 0xFFFF
	// sizeof(WORD) + // ordinal value of control's class
	// sizeof(WORD) + // no. of bytes in creation data array
	// sizeof title in WCHARs
	for (i = 0; i < m_dlgTempl.cdit; i++)
	{
		int nItemLength = sizeof(DLGITEMTEMPLATE) + 3 * sizeof(WORD);

#ifdef _UNICODE
		int nActualChars = _tcslen(m_pDlgItemArray[i]->m_pszCaption) + 1;	//+++1.5;
#else
		int nActualChars = MultiByteToWideChar(CP_ACP, 0,
								(LPCSTR)m_pDlgItemArray[i]->m_pszCaption, 
								-1, NULL, 0);	//+++1.5
#endif

		_ASSERTE(nActualChars > 0);
		nItemLength += nActualChars * sizeof(WCHAR);

		if (i != m_dlgTempl.cdit - 1) // the last control does not need extra bytes
		{
			nItemLength = (nItemLength + 3) & ~3; // take into account gap
		} // so next control is DWORD aligned

		nBufferSize += nItemLength;
	}

	HLOCAL hLocal = LocalAlloc(LHND, nBufferSize);
	_ASSERTE(hLocal);
	if (hLocal == NULL)
		return IDCANCEL;

	BYTE* pBuffer = (BYTE*)LocalLock(hLocal);
	_ASSERTE(pBuffer);
	if (pBuffer == NULL)
	{
		LocalFree(hLocal);
		return IDCANCEL;
	}

	BYTE* pdest = pBuffer;

	// transfer DLGTEMPLATE structure to the buffer
	memcpy(pdest, &m_dlgTempl, sizeof(DLGTEMPLATE));
	pdest += sizeof(DLGTEMPLATE);

	*(WORD*)pdest = 0;		// no menu
	pdest += sizeof(WORD);	//+++1.5
	*(WORD*)pdest = 0;		// use default window class		//+++1.5
	pdest += sizeof(WORD);	//+++1.5

	// transfer title
	WCHAR *pchCaption = new WCHAR[nTitleLen + 100];
	memset(pchCaption, 0, nTitleLen*sizeof(WCHAR) + 2);

#ifdef _UNICODE
	memcpy(pchCaption, szTitle, nTitleLen * sizeof(TCHAR));
	int nActualChars = nTitleLen + 1;
#else
	int nActualChars = MultiByteToWideChar(CP_ACP, 0,
			(LPCSTR)szTitle, -1, pchCaption, nTitleLen + 1);
#endif

	_ASSERTE(nActualChars > 0);
	memcpy(pdest, pchCaption, nActualChars * sizeof(WCHAR));
	pdest += nActualChars * sizeof(WCHAR);
	delete [] pchCaption;		//+++1.5

	// will now transfer the information for each one of the item templates
	for (i = 0; i < m_dlgTempl.cdit; i++)
	{
		pdest = (BYTE*)(((DWORD_PTR)pdest + 3) & ~3);	// make the pointer DWORD aligned
		memcpy(pdest, (void *)&m_pDlgItemArray[i]->m_dlgItemTemplate,
			sizeof(DLGITEMTEMPLATE));
		pdest += sizeof(DLGITEMTEMPLATE);
		*(WORD*)pdest = 0xFFFF;						// indicating atom value
		pdest += sizeof(WORD);
		*(WORD*)pdest = (WORD)m_pDlgItemArray[i]->m_controltype;	// atom value for the control
		pdest += sizeof(WORD);

		// transfer the caption even when it is an empty string

		int nChars = (int)_tcslen(m_pDlgItemArray[i]->m_pszCaption) + 1;	//+++1.5

		WCHAR * pchCaption = new WCHAR[nChars+100];

#ifdef _UNICODE
		memset(pchCaption, 0, nChars*sizeof(TCHAR) + 2);
		memcpy(pchCaption, m_pDlgItemArray[i]->m_pszCaption, nChars * sizeof(TCHAR));	//+++1.5
		int nActualChars = nChars;
#else
		int nActualChars = MultiByteToWideChar(CP_ACP, 0,
			(LPCSTR)m_pDlgItemArray[i]->m_pszCaption, -1, pchCaption, nChars);	//+++1.5
#endif

		_ASSERTE(nActualChars > 0);
		memcpy(pdest, pchCaption, nActualChars * sizeof(WCHAR));
		pdest += nActualChars * sizeof(WCHAR);
		delete [] pchCaption;	//+++1.5

		*(WORD*)pdest = 0; // How many bytes in data for control
		pdest += sizeof(WORD);
	}

	_ASSERTE(pdest - pBuffer <= nBufferSize); //+++1.5 // just make sure we did not overrun the heap
	HINSTANCE hInstance = GetModuleHandle(NULL);

	TRACE(_T("calling CreateDialogIndirectParam\n"));

	HWND hDlg = ::CreateDialogIndirectParam(hInstance, (LPCDLGTEMPLATE) pBuffer,
		m_hWndOwner, MsgBoxDlgProc, (LPARAM) this);
	_ASSERTE(hDlg);

	if (hDlg)
	{
		// Corona Labs:
		// Provide the message box's HWND to the caller if a callback was provided.
		// This allows the caller to close the message box externally by posting an BN_CLICKED message.
		if (m_lpCreatedFunc)
		{
			(*m_lpCreatedFunc)(hDlg, m_lpCreatedFuncData);
		}

#if 0
		// disable owner - this is a modal dialog
		::EnableWindow(m_hWndOwner, FALSE);
#else
		// Corona Labs:
		// Disable the parent windows if modal.
		HWND parentWindowHandle = m_hWndOwner;
		while (parentWindowHandle && ::IsWindow(parentWindowHandle))
		{
			::EnableWindow(parentWindowHandle, FALSE);
			parentWindowHandle = ::GetParent(parentWindowHandle);
		}
#endif

		MSG msg;
		memset(&msg, 0, sizeof(msg));

		// message loop for dialog

		while (!IsEnded())
		{
			if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					break;
				}
				if (msg.message == WM_KEYDOWN)
				{
					// returns TRUE if Ctrl-C processed
					if (OnKeyDown(hDlg, msg.wParam, msg.lParam))
						continue;
				}
				if (!::IsDialogMessage(hDlg, &msg))
				{
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
			}
			else if (!IsEnded())
			{
				::WaitMessage();	// suspend thread until new message arrives
			}
		}

		TRACE(_T("===== message loop ended\n"));

		if (msg.message == WM_QUIT)
		{
			TRACE(_T("_____ WM_QUIT seen\n"));
			PostQuitMessage((int)msg.wParam);
		}

#if 0
		// re-enable owner
		if (::IsWindow(m_hWndOwner))
			::EnableWindow(m_hWndOwner, TRUE);
#else
		// Corona Labs:
		// Re-enable the parent windows if modal.
		parentWindowHandle = m_hWndOwner;
		while (parentWindowHandle && ::IsWindow(parentWindowHandle))
		{
			::EnableWindow(parentWindowHandle, TRUE);
			parentWindowHandle = ::GetParent(parentWindowHandle);
		}
#endif

		::DestroyWindow(hDlg);
	}

	LocalUnlock(hLocal);
	LocalFree(hLocal);

	return m_nReturnValue;
}

///////////////////////////////////////////////////////////////////////////////
// CXDialogItem class

///////////////////////////////////////////////////////////////////////////////
// CXDialogItem ctor
CXDialogItem::CXDialogItem(CXDialogItem::Econtroltype ctrlType) :
	m_controltype(ctrlType),
	m_pszCaption(NULL)		//+++1.5
{
}

///////////////////////////////////////////////////////////////////////////////
// CXDialogItem dtor
CXDialogItem::~CXDialogItem()
{
	if (m_pszCaption)
		delete [] m_pszCaption;
}

///////////////////////////////////////////////////////////////////////////////
// CXDialogItem::AddItem
void CXDialogItem::AddItem(CXDialogTemplate& dialog,
						   Econtroltype ctrltype,
						   UINT nID,
						   CXRect* prect,
						   LPCTSTR lpszCaption)
{
	short hidbu = HIWORD(GetDialogBaseUnits());
	short lodbu = LOWORD(GetDialogBaseUnits());

	// first fill in the type, location and size of the control
	m_controltype = ctrltype;

	if (m_controltype == CHECKBOX)
		m_controltype = BUTTON;

	if (prect != NULL)
	{
		m_dlgItemTemplate.x = (short)((prect->left * 4) / lodbu);
		m_dlgItemTemplate.y = (short)((prect->top * 8) / hidbu);
		m_dlgItemTemplate.cx = (short)((prect->Width() * 4) / lodbu);
		m_dlgItemTemplate.cy = (short)((prect->Height() * 8) / hidbu);
	}
	else
	{
		m_dlgItemTemplate.x = 0;
		m_dlgItemTemplate.y = 0;
		m_dlgItemTemplate.cx = 10; // some useless default
		m_dlgItemTemplate.cy = 10;
	}

	m_dlgItemTemplate.dwExtendedStyle = 0;
	m_dlgItemTemplate.id = (WORD)nID;

	switch (ctrltype)
	{
		case ICON:
			m_dlgItemTemplate.style = WS_CHILD | SS_ICON | WS_VISIBLE;
			break;

		case BUTTON:
			dialog.GetButtonCount()++;
			m_dlgItemTemplate.style = WS_VISIBLE | WS_CHILD | WS_TABSTOP;
			if (dialog.GetButtonCount() == dialog.GetDefaultButton())
			{
				m_dlgItemTemplate.style |= BS_DEFPUSHBUTTON;
				dialog.SetDefaultButtonId(nID);
			}
			else
			{
				m_dlgItemTemplate.style |= BS_PUSHBUTTON;
			}
			break;

		case CHECKBOX:
			m_dlgItemTemplate.style = WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX;
			break;

		case EDITCONTROL:
			m_dlgItemTemplate.style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_LEFT;
			break;

		case STATICTEXT:
			m_dlgItemTemplate.style = WS_CHILD | WS_VISIBLE | SS_LEFT;
			break;

		default:
			_ASSERTE(FALSE); // should never get here
	}

	int stringLength = (int)(lpszCaption ? _tcslen(lpszCaption) : 0);	//+++1.5
	m_pszCaption = new TCHAR [stringLength + 1];				//+++1.5
	_tcscpy(m_pszCaption, lpszCaption ? lpszCaption : _T(""));	//+++1.5
}

#ifndef XMESSAGEBOX_DO_NOT_SAVE_CHECKBOX
#ifndef XMESSAGEBOX_USE_PROFILE_FILE

static DWORD ReadRegistry(LPCTSTR lpszCompanyName, LPCTSTR lpszKey)
{
	_ASSERTE((lpszKey != NULL) && (lpszKey[0] != _T('\0')));
	if (!lpszKey || lpszKey[0] == _T('\0'))
		return 0;

	TCHAR * szRegPath = _T("Software\\");

	TCHAR szKey[_MAX_PATH*2] = { 0 };
	_tcscpy(szKey, szRegPath);

	if (lpszCompanyName && lpszCompanyName[0] != _T('\0'))
	{
		_tcscat(szKey, lpszCompanyName);
		_tcscat(szKey, _T("\\"));
	}

	TCHAR szPathName[_MAX_PATH*2] = { 0 };
	::GetModuleFileName(NULL, szPathName, MAX_PATH*2-2);

	TCHAR *cp = _tcsrchr(szPathName, _T('\\'));
	if (cp == NULL)
		cp = szPathName;
	else
		cp++;

	_tcscat(szKey, cp);

	_tcscat(szKey, _T("\\"));
	_tcscat(szKey, XMESSAGEBOX_REGISTRY_KEY);

	TRACE(_T("szKey=<%s>\n"), szKey);

	// open the registry event source key
	DWORD dwData = 0;
	HKEY hKey = NULL;
	TRACE(_T("trying to open key\n"));
	LONG lRet = ::RegOpenKeyEx(HKEY_CURRENT_USER, szKey, 0, KEY_READ, &hKey);

	if (lRet == ERROR_SUCCESS)
	{
		// registry key was opened or created -
		TRACE(_T("key opened ok\n"));

		// === write EventMessageFile key ===
		DWORD dwType = 0;
		DWORD dwSize = sizeof(DWORD);
		lRet = ::RegQueryValueEx(hKey, lpszKey, 0, &dwType, (LPBYTE) &dwData, &dwSize);

		::RegCloseKey(hKey);

		if (lRet != ERROR_SUCCESS)
			dwData = 0;
	}
	else
	{
		dwData = 0;
	}

	return dwData;
}

static void WriteRegistry(LPCTSTR lpszCompanyName, LPCTSTR lpszKey, DWORD dwData)
{
	_ASSERTE((lpszKey != NULL) && (lpszKey[0] != _T('\0')));
	if (!lpszKey || lpszKey[0] == _T('\0'))
		return;

	TCHAR * szRegPath = _T("Software\\");

	TCHAR szKey[_MAX_PATH*2] = { 0 };
	_tcscpy(szKey, szRegPath);

	if (lpszCompanyName && lpszCompanyName[0] != _T('\0'))
	{
		_tcscat(szKey, lpszCompanyName);
		_tcscat(szKey, _T("\\"));
	}

	TCHAR szPathName[_MAX_PATH*2] = { 0 };
	::GetModuleFileName(NULL, szPathName, MAX_PATH*2-2);

	TCHAR *cp = _tcsrchr(szPathName, _T('\\'));
	if (cp == NULL)
		cp = szPathName;
	else
		cp++;

	_tcscat(szKey, cp);
	_tcscat(szKey, _T("\\"));
	_tcscat(szKey, XMESSAGEBOX_REGISTRY_KEY);

	TRACE(_T("szKey=<%s>\n"), szKey);

	// open the registry key
	DWORD dwResult = 0;
	HKEY hKey = NULL;
	LONG lRet = ::RegCreateKeyEx(HKEY_CURRENT_USER, szKey, 0, NULL,
					REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwResult);

	if (lRet == ERROR_SUCCESS)
	{
		// registry key was opened or created -

		::RegSetValueEx(hKey, lpszKey, 0, REG_DWORD,
			(const BYTE *) &dwData, sizeof(DWORD));

		::RegCloseKey(hKey);
	}

	return;
}

#endif	// XMESSAGEBOX_USE_PROFILE_FILE


//+++1.8
///////////////////////////////////////////////////////////////////////////////
//
// XMessageBoxGetCheckBox()
//
// The XMessageBoxGetCheckBox function checks if a value has been stored in
// the registry or ini file for one of the Do not ask/tell/show checkboxes.
// A non-zero value is returned if a value has been stored, otherwise 0.
//
// For more information see
//     http://www.codeproject.com/KB/dialog/xmessagebox.aspx
//
// DWORD XMessageBoxGetCheckBox(LPCTSTR lpszCompanyName, 
//                              LPCTSTR lpszModule, 
//                              int nLine)
//
// PARAMETERS
//
//     lpszCompanyName - Specifies the company name for the application.
//                       Should be the same as passed to XMessageBox().
//
//     lpszModule      - Specifies the source module name for the application.
//                       Should be the same as passed to XMessageBox().
//
//     nLine           - Identifies the source module line number.
//                       Should be the same as passed to XMessageBox().
//
///////////////////////////////////////////////////////////////////////////////
DWORD XMessageBoxGetCheckBox(LPCTSTR lpszCompanyName, 
							 LPCTSTR lpszModule, 
							 int nLine)
{
	DWORD dwData = 0;

	_ASSERTE(lpszCompanyName);
	_ASSERTE(lpszModule);

	if (lpszCompanyName && lpszModule)
	{
		// key is module name and line
		TCHAR szKey[MAX_PATH*2];
		_tcsncpy(szKey, lpszModule, countof(szKey)-30);
		szKey[countof(szKey)-30] = 0;
		TRACE(_T("szKey=<%s>\n"), szKey);

		encode(szKey);		// simple encoding to obscure module name

		TCHAR szLine[100];
		szLine[0] = _T('\0');
		_tcscat(szKey, _itot(nLine, szLine, 10));
		TRACE(_T("szKey=<%s>\n"), szKey);


#ifndef XMESSAGEBOX_USE_PROFILE_FILE

		TRACE(_T("reading from registry\n"));
		dwData = ReadRegistry(lpszCompanyName, szKey);

#else

		// get full path to ini file
		TCHAR szPathName[MAX_PATH*2];
		szPathName[0] = _T('\0');
		::GetModuleFileName(NULL, szPathName, countof(szPathName)-1);

		TCHAR *cp = _tcsrchr(szPathName, _T('\\'));
		if (cp != NULL)
			*(cp+1) = _T('\0');
		_tcscat(szPathName, XMESSAGEBOX_INI_FILE);
		TRACE(_T("reading from ini file <%s>\n"), szPathName);

		TCHAR szBuf[100];
		szBuf[0] = _T('\0');

		// data string is hex value of XMessageBox return code
		::GetPrivateProfileString(_T("DoNotAsk"),	// section name
								  szKey,			// key name
								  _T(""),			// default string
								  szBuf,			// destination buffer
								  countof(szBuf)-1,	// size of destination buffer
								  szPathName);		// initialization file name

		dwData = _tcstoul(szBuf, NULL, 16);
		TRACE(_T("szBuf=<%s>\n"), szBuf);

#endif	// XMESSAGEBOX_USE_PROFILE_FILE

	}

	TRACE(_T("dwData=0x%08X\n"), dwData);
	return dwData;
}

///////////////////////////////////////////////////////////////////////////////
//
// XMessageBoxGetCheckBox()
//
// The XMessageBoxGetCheckBox function checks if a value has been stored in
// the registry or ini file for one of the Do not ask/tell/show checkboxes.
// A non-zero value is returned if a value has been stored, otherwise 0.
//
// For more information see
//     http://www.codeproject.com/KB/dialog/xmessagebox.aspx
//
// DWORD XMessageBoxGetCheckBox(XMSGBOXPARAMS& xmb) 
//
// PARAMETERS
//
//     xmb - Struct that contains the company name, module and nLine parameters.
//           Should be the same as passed to XMessageBox().
//
///////////////////////////////////////////////////////////////////////////////
DWORD XMessageBoxGetCheckBox(XMSGBOXPARAMS& xmb)
{
	TRACE(_T("in XMessageBoxGetCheckBox\n"));
	return XMessageBoxGetCheckBox(xmb.szCompanyName, 
								  xmb.lpszModule, 
								  xmb.nLine);
}

#endif	// XMESSAGEBOX_DO_NOT_SAVE_CHECKBOX
