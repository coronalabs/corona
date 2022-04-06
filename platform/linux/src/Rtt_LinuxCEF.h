//
// CEF based embedd browser implementation
// This source code has been donated to the Public Domain.  Do whatever you want with it.
//

#pragma once

#include "Rtt_Types.h"
#include "Rtt_LinuxContainer.h"
#include "Rtt_LinuxBitmap.h"
#include "Rtt_Geometry.h"
#include "Rtt_RenderData.h"
#include "Display/Rtt_TextureResourceBitmap.h"

#if USE_LIBCEF == 1

# include "include/capi/cef_app_capi.h"	
# include "include/capi/cef_browser_capi.h"		// browser API
# include "include/capi/cef_client_capi.h"		// client API
# include "include/capi/cef_app_capi.h"

namespace Rtt
{
	struct WebView : public ref_counted
	{
		WebView(const Rect& outBounds, const char* url);
		virtual ~WebView();

		int Width() { return fBounds.Width(); }
		int Height() { return fBounds.Height(); }
		uint8_t* GetBitmap()
		{
			if (fTex.NotNull())
			{
				PlatformBitmap* bitmap = fTex->GetBitmap();
				return (uint8_t*)bitmap->GetBits(NULL);
			}
			return NULL;
		}

		void InitGeometry();
		void UpdateTex(const uint8_t* buf, int width, int height);
		void MouseMove(int	x, int y);
		void MousePress(int	x, int y);
		void MouseRelease(int	x, int y);
		void MouseWheel(int deltaX, int deltaY);
		int EventModifiers() const;
		void KeyDown();
		void ClearCookies();

		//private:

		Rect fBounds;
		std::string fUrl;
		RenderData fData;
		SharedPtr< TextureResourceBitmap > fTex;

		cef_browser_t* fBrowser;
		cef_window_info_t fWindowInfo;
		cef_browser_settings_t fBrowserSettings;

		cef_client_t fClient;
		weak_ptr<WebView> this_ptr_for_client;

		cef_render_handler_t fRender;
		weak_ptr<WebView> this_ptr_for_render;

		cef_request_handler_t fRequestHandler;
		weak_ptr<WebView> this_ptr_for_requesthandler;
	};

} // namespace 

#else

// dummy class
namespace Rtt
{
	struct WebView : public ref_counted
	{
		WebView(const Rect& outBounds, const char* url) {}
		virtual ~WebView() {};

		int Width() { return 0; }
		int Height() { return 0; }
		uint8_t* GetBitmap() { return NULL; }
		void InitGeometry() {}
		void UpdateTex(const uint8_t* buf, int width, int height) {}
		void MouseMove(int	x, int y) {}
		void MousePress(int	x, int y) {}
		void MouseRelease(int	x, int y) {}
		void MouseWheel(int deltaX, int deltaY) {}
		int EventModifiers() const { return 0; }
		void KeyDown() {}
		void ClearCookies() {}

		Rect fBounds;
		RenderData fData;
	};

} // namespace 

#endif

namespace Rtt
{
	bool InitCEF(int argc, char** argv);
	void FinalizeCEF();
	void	advanceCEF();
}
