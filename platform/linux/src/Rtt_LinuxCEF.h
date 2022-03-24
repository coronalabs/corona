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
	struct as_cef : public ref_counted
	{
		as_cef();
		virtual ~as_cef();
		virtual void advance();

		LinuxBaseBitmap* m_bi;
		std::string m_url;
		cef_browser_t* m_browser;cef_window_info_t m_window_info;
		cef_browser_settings_t m_bsettings;

		cef_client_t m_client;
		weak_ptr<as_cef> this_ptr_for_client;

		cef_render_handler_t m_render;
		weak_ptr<as_cef> this_ptr_for_render;

		cef_request_handler_t m_requesthandler;
		weak_ptr<as_cef> this_ptr_for_requesthandler;
	};

	bool InitCEF(int argc, char** argv);
	void FinalizeCEF();

}	// namespace 
