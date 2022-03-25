//
// CEF based embedd browser implementation
// This source code has been donated to the Public Domain.  Do whatever you want with it.
//

#pragma once

#include "Rtt_Types.h"
#include "Rtt_LinuxContainer.h"
#include "Rtt_LinuxBitmap.h"
#include "Rtt_Geometry.h"

// cef
#include "include/capi/cef_app_capi.h"	
#include "include/capi/cef_browser_capi.h"		// browser API
#include "include/capi/cef_client_capi.h"		// client API
#include "include/capi/cef_app_capi.h"

namespace Rtt
{
	struct CefClient : public ref_counted
	{
		CefClient(const Rect& outBounds, const char* url);
		virtual ~CefClient();

		void advance();

	//private:

		LinuxBaseBitmap* fBitmap;
		Rect fBounds;
		std::string fUrl;

		cef_browser_t* fBrowser;
		cef_window_info_t fWindowInfo;
		cef_browser_settings_t fBrowserSettings;

		cef_client_t fClient;
		weak_ptr<CefClient> this_ptr_for_client;

		cef_render_handler_t fRender;
		weak_ptr<CefClient> this_ptr_for_render;

		cef_request_handler_t fRequestHandler;
		weak_ptr<CefClient> this_ptr_for_requesthandler;
	};

	bool InitCEF(int argc, char** argv);
	void FinalizeCEF();

}	// namespace 
