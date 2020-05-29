//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "CoronaLiveServerWin.h"
#include "..\..\shared\CoronaLiveServer\CoronaHTTPServer.h"


// CCoronaLiveServerApp:
// See Corona.LiveServer.cpp for the implementation of this class
//

class CCoronaLiveServerApp : public CWinApp
{
public:
	static LPCTSTR sLiveDataPrefix;
private:
	static LPCTSTR sNamedMutexId;

public:
	CCoronaLiveServerApp();
	virtual ~CCoronaLiveServerApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()

public:
	CoronaLiveServerWin *GetServer();
	Rtt_HTTPServer *GetHttpServer();

private:
	CoronaLiveServerWin *m_Server;
	Rtt_HTTPServer *m_HttpServer;
public:
	virtual int ExitInstance();
};

extern CCoronaLiveServerApp theApp;