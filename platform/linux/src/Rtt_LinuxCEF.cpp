// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

#include "Rtt_LinuxCEF.h"
#include "Rtt_LinuxApp.h"
#include "Rtt_Display.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Display/Rtt_ShaderFactory.h"
#include "Display/Rtt_DisplayObject.h"

static int s_x0 = 0;
static int s_y0 = 0;
static float s_retina = 1;
static float s_scale = 1;

namespace Rtt
{
	// Method that will be called once for each cookie. |count| is the 0-based
	// index for the current cookie. |total| is the total number of cookies. Set
	// |deleteCookie| to true (1) to delete the cookie currently being visited.
	// Return false (0) to stop visiting cookies. This function may never be
	// called if no cookies are found.
	int CEF_CALLBACK visitCookie(struct _cef_cookie_visitor_t* self, const struct _cef_cookie_t* cookie, int count, int total, int* deleteCookie)
	{
		*deleteCookie = 1; //true;
		return 1;		// true
	}

	void CEF_CALLBACK onResourceRedirect(struct _cef_request_handler_t* self, struct _cef_browser_t* browser, struct _cef_frame_t* frame, struct _cef_request_t* request, cef_string_t* new_url)
	{
		cef_string_utf8_t dst = {};
		int n = cef_string_utf16_to_utf8(new_url->str, new_url->length, &dst);
		//sendMessage("onLoadURL", dst.str);
		cef_string_utf8_clear(&dst);
		printf("onResourceRedirect\n");
	}

	_cef_render_handler_t* CEF_CALLBACK getRenderHandler(struct _cef_client_t* self)
	{
		const weak_ptr<WebView>& thiz = *(const weak_ptr<WebView>*)((Uint8*)self + sizeof(struct _cef_client_t));
		return &thiz->fRender;
	}

	_cef_request_handler_t* CEF_CALLBACK getRequestHandler(struct _cef_client_t* self)
	{
		const weak_ptr<WebView>& thiz = *(const weak_ptr<WebView>*)((Uint8*)self + sizeof(struct _cef_client_t));
		return &thiz->fRequestHandler;
	}

	void CEF_CALLBACK getViewRect(struct _cef_render_handler_t* self, struct _cef_browser_t* browser, cef_rect_t* rect)
	{
		const weak_ptr<WebView>& thiz = *(const weak_ptr<WebView>*)((Uint8*)self + sizeof(struct _cef_render_handler_t));
		rect->x = 0;
		rect->y = 0;
		rect->width = thiz->Width();
		rect->height = thiz->Height();
	}

	void CEF_CALLBACK onPaint(struct _cef_render_handler_t* self, struct _cef_browser_t* browser, cef_paint_element_type_t type, size_t dirtyRectsCount,
		cef_rect_t const* dirtyRects, const void* buffer, int width, int height)
	{
		// BGRA ==> RGBA
		Uint8* src = (Uint8*)buffer;
		Uint8* buf = (Uint8*)malloc(width * height * 4);
		Uint8* dst = buf;
		for (int i = 0; i < width * height; i++)
		{
			dst[0] = src[2];
			dst[1] = src[1];
			dst[2] = src[0];
			dst[3] = src[3];
			src += 4;
			dst += 4;
		}

		const weak_ptr<WebView>& thiz = *(const weak_ptr<WebView>*)((Uint8*)self + sizeof(struct _cef_render_handler_t));
		thiz->UpdateTex(buf, width, height);
		free(buf);
	}

	WebView::WebView(const Rect& outBounds, const char* url)
		: fBounds(outBounds)
		, fUrl(url)
		, fBrowser(NULL)
		, fWindowInfo({})
		, fBrowserSettings({})
		, fClient({})
		, fRender({})
		, fRequestHandler({})
		, this_ptr_for_render(this)
		, this_ptr_for_client(this)
		, this_ptr_for_requesthandler(this)
	{
		int w = outBounds.Width();
		int h = outBounds.Height();

		Display& display = app->GetRuntime()->GetDisplay();
		TextureFactory& factory = display.GetTextureFactory();

		InitGeometry();

		LinuxBaseBitmap* bm = new LinuxBaseBitmap(NULL, w, h, NULL);
		TextureResourceBitmap* resourceY = TextureResourceBitmap::Create(factory, bm, false);
		fTex = SharedPtr< TextureResourceBitmap >(resourceY);
		fData.fFillTexture0 = &fTex->GetTexture();

		fRender.base.size = sizeof(cef_render_handler_t);
		fRender.on_paint = onPaint;
		fRender.get_view_rect = getViewRect;

		fRequestHandler.base.size = sizeof(cef_request_handler_t);
		//fRequesthandler.on_before_browse = onBeforeBrowse;
		//fRequesthandler.on_resource_redirect = onResourceRedirect;

		fClient.base.size = sizeof(cef_client_t);
		fClient.get_render_handler = getRenderHandler;
		fClient.get_request_handler = getRequestHandler;

		fWindowInfo.windowless_rendering_enabled = true;
		//fWindowInfo.transparent_painting_enabled = true;

		fBrowserSettings.size = sizeof(cef_browser_settings_t);
		fBrowserSettings.windowless_frame_rate = 30;

		// initial URL
		cef_string_t cefurl = {};
		cef_string_utf8_to_utf16(url, strlen(url), &cefurl);

		fBrowser = cef_browser_host_create_browser_sync(&fWindowInfo, &fClient, &cefurl, &fBrowserSettings, NULL, NULL);
		Rtt_Log("CEF browser %s\n", fBrowser == 0 ? "failed to start" : "started");
	}

	WebView::~WebView()
	{
		Rtt_Log("CEF browser ended\n");
		if (fBrowser)
		{
			cef_browser_host_t* host = fBrowser->get_host(fBrowser);
			host->close_browser(host, true);
		}
	}

	void WebView::UpdateTex(const uint8_t* buf, int width, int height)
	{
		memcpy(GetBitmap(), buf, width * height * 4);
		fTex->GetTexture().Invalidate(); // Force Renderer to update GPU texture
	}

	void WebView::InitGeometry()
	{
		// Init fData.
		int numVertices = 4;
		Display& display = app->GetRuntime()->GetDisplay();
		fData.fGeometry = new Rtt::Geometry(display.GetAllocator(), Geometry::kTriangleStrip, numVertices, 0, true);
		fData.fGeometry->Resize(4, false);

		Geometry::Vertex* dstVertices = fData.fGeometry->GetVertexData();
		for (U32 i = 0; i < numVertices; i++)
		{
			Geometry::Vertex& dst = dstVertices[i];
			dst.q = 1.f;
			dst.z = 0.f;
			dst.rs = dst.gs = dst.bs = dst.as = 255;
			switch (i)
			{
			case 0:
				dst.x = fBounds.xMin;
				dst.y = fBounds.yMin;
				dst.u = 0;
				dst.v = 0;
				break;
			case 1:
				dst.x = fBounds.xMin;
				dst.y = fBounds.yMax;
				dst.u = 0;
				dst.v = 1;
				break;
			case 2:
				dst.x = fBounds.xMax;
				dst.y = fBounds.yMin;
				dst.u = 1;
				dst.v = 0;
				break;
			case 3:
				dst.x = fBounds.xMax;
				dst.y = fBounds.yMax;
				dst.u = 1;
				dst.v = 1;
				break;
			}
		}

		fData.fGeometry->SetVerticesUsed(numVertices);
		ShaderFactory& factory = display.GetShaderFactory();
		Shader* shader = &factory.GetDefault();
		shader->Prepare(fData, 0, 0, ShaderResource::kDefault);
	}


	int WebView::EventModifiers() const
	{
		// Fetch the mouse's current up/down buttons states.
		int m = 0;
		Uint32 mouseState = SDL_GetMouseState(NULL, NULL);
		m |= mouseState & SDL_BUTTON(SDL_BUTTON_LEFT) ? EVENTFLAG_LEFT_MOUSE_BUTTON : 0;
		m |= mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT) ? EVENTFLAG_RIGHT_MOUSE_BUTTON : 0;
		m |= mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE) ? EVENTFLAG_MIDDLE_MOUSE_BUTTON : 0;

		// Fetch the current state of the "shift", "alt", and "ctrl" keys.
		const Uint8* key = SDL_GetKeyboardState(NULL);
		m |= key[SDL_SCANCODE_LALT] ? EVENTFLAG_ALT_DOWN | EVENTFLAG_IS_LEFT : 0;
		m |= key[SDL_SCANCODE_RALT] ? EVENTFLAG_ALT_DOWN | EVENTFLAG_IS_RIGHT : 0;
		m |= key[SDL_SCANCODE_LSHIFT] ? EVENTFLAG_SHIFT_DOWN | EVENTFLAG_IS_LEFT : 0;
		m |= key[SDL_SCANCODE_RSHIFT] ? EVENTFLAG_SHIFT_DOWN | EVENTFLAG_IS_RIGHT : 0;
		m |= key[SDL_SCANCODE_LCTRL] ? EVENTFLAG_CONTROL_DOWN | EVENTFLAG_IS_LEFT : 0;
		m |= key[SDL_SCANCODE_RCTRL] ? EVENTFLAG_CONTROL_DOWN | EVENTFLAG_IS_RIGHT : 0;
		m |= key[SDL_SCANCODE_LGUI] ? EVENTFLAG_COMMAND_DOWN | EVENTFLAG_IS_LEFT : 0;
		m |= key[SDL_SCANCODE_RGUI] ? EVENTFLAG_COMMAND_DOWN | EVENTFLAG_IS_RIGHT : 0;

		return m;
	}

	void	WebView::MouseMove(int	x, int y)
	{
		if (fBrowser)
		{
			cef_mouse_event_t event = { x, y, EventModifiers() };
			cef_browser_host_t* host = fBrowser->get_host(fBrowser);
			host->set_focus(host, 1);
			host->send_mouse_move_event(host, &event, false);
		}
	}

	void	WebView::MousePress(int	x, int y)
	{
		if (fBrowser)
		{
			cef_mouse_event_t event = { x, y, EventModifiers() };
			cef_browser_host_t* host = fBrowser->get_host(fBrowser);
			host->set_focus(host, 1);
			host->send_mouse_click_event(host, &event, MBT_LEFT, 0, 1);
		}
	}

	void	WebView::MouseRelease(int	x, int y)
	{
		if (fBrowser)
		{
			cef_mouse_event_t event = { x, y, EventModifiers() };
			cef_browser_host_t* host = fBrowser->get_host(fBrowser);
			host->set_focus(host, 1);
			host->send_mouse_click_event(host, &event, MBT_LEFT, 1, 1);
		}
	}

	void	WebView::KeyDown()
	{
		if (fBrowser == NULL)
			return;

		Uint16 utf16char = 0; // ko->get_last_utf16_key_pressed();

		cef_browser_host_t* host = fBrowser->get_host(fBrowser);
		cef_key_event_t event = {};
#ifdef WIN32
		event.windows_key_code = utf16char;
#else
		event.windows_key_code = utf16char;
		event.character = utf16char;
#endif

		event.type = KEYEVENT_RAWKEYDOWN;
		host->send_key_event(host, &event);

		event.type = KEYEVENT_KEYUP;
		host->send_key_event(host, &event);

		// hack, tab,left,right,home,end, del, enter
	//	if (lastkey != 9 && lastkey != 35 && lastkey != 37 && lastkey != 36 && lastkey != 39 && lastkey != 46 && lastkey != 13)
		{
			event.type = KEYEVENT_CHAR;
			host->send_key_event(host, &event);
		}

	}

	void WebView::ClearCookies()
	{
		/*		if (fBrowser == NULL)
					return;

				cef_frame_t* frame = fBrowser->get_main_frame(fBrowser);
				if (!fUrl.empty())
				{
					static cef_cookie_visitor_t it = {};
					it.base.size = sizeof(it);
					it.visit = visitCookie;
		#ifdef WIN32
					cef_cookie_manager_t* cm = cef_cookie_manager_get_global_manager();
		#else
					cef_cookie_manager_t* cm = cef_cookie_manager_get_global_manager(NULL);
		#endif
					cm->visit_all_cookies(cm, &it);

					cef_string_t url = {};
					cef_string_utf8_to_utf16(fUrl.c_str(), fUrl.size(), &url);
					frame->load_url(frame, &url);
					cef_string_clear(&url);
				}
				else
				{
					const char about[] = "about:blank";
					cef_string_utf16_t url = {};
					cef_string_ascii_to_utf16(about, sizeof(about), &url);
					frame->load_url(frame, &url);
					cef_string_utf16_clear(&url);
				}
		*/
	}

	//
	//
	//

	bool InitCEF(int argc, char** argv)
	{
		// This is called many times, because it is also used for subprocesses. 
		// If one of the first args is for example "--type=renderer" then it means that this is a Renderer process. 
		// There may be more subprocesses like GPU (--type=gpu-process) and others.
		// On Linux there are also special Zygote processes.

		/*if (argc > 1)
		{
			Rtt_Log("Subprocess: ");
			for (int i = 1; i < argc; i++)
			{
				Rtt_Log("%s ", argv[i]);
			}
			Rtt_Log("\n");
		}*/

		// Execute subprocesses. 
		// It is also possible to have a separate executable for subprocesses by setting cef_settings_t.browser_subprocess_path.
		// In such case cef_execute_process should not be called here.
		cef_main_args_t main_args = { argc, argv };
		int rc = cef_execute_process(&main_args, NULL, NULL);
		if (rc >= 0)
		{
			_exit(rc);
		}

		// Application settings. It is mandatory to set the "size" member.
		cef_settings_t settings = {};
		settings.size = sizeof(cef_settings_t);
		settings.log_severity = LOGSEVERITY_WARNING; // Show only warnings/errors
		settings.no_sandbox = true;
		settings.windowless_rendering_enabled = true;
		settings.multi_threaded_message_loop = false;

		// Initialize CEF.
		rc = cef_initialize(&main_args, &settings, NULL, NULL);

		// A return value of true (1) indicates that it succeeded and false (0) indicates that it failed.
		//Rtt_Log("Main process: CEF Initialize %s\n", rc == 0 ? "failed" : "successed");
		return rc == 1;
	}

	void FinalizeCEF()
	{
		cef_shutdown();
	}

	void	advanceCEF()
	{
		cef_do_message_loop_work();
	}

}	// namespace bakeinflash
