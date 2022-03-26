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
		printf("getRenderHandler\n");
		const weak_ptr<CefClient>& thiz = *(const weak_ptr<CefClient>*)((Uint8*)self + sizeof(struct _cef_client_t));
		return &thiz->fRender;
	}

	_cef_request_handler_t* CEF_CALLBACK getRequestHandler(struct _cef_client_t* self)
	{
		printf("getRequestHandler\n");
		const weak_ptr<CefClient>& thiz = *(const weak_ptr<CefClient>*)((Uint8*)self + sizeof(struct _cef_client_t));
		return &thiz->fRequestHandler;
	}

	void CEF_CALLBACK getViewRect(struct _cef_render_handler_t* self, struct _cef_browser_t* browser, cef_rect_t* rect)
	{
		const weak_ptr<CefClient>& thiz = *(const weak_ptr<CefClient>*)((Uint8*)self + sizeof(struct _cef_render_handler_t));

		rect->x = 0;
		rect->y = 0;
		rect->width = thiz->Width();
		rect->height = thiz->Height();
		printf("getViewRect %dx%d\n", rect->width, rect->height);
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

		const weak_ptr<CefClient>& thiz = *(const weak_ptr<CefClient>*)((Uint8*)self + sizeof(struct _cef_render_handler_t));
		thiz->UpdateTex(buf, width, height);
		free(buf);
	}

	void	CefClient_mouse_move()
	{
		/*		as_value val;
				mc->get_member("_CefPtr_", &val);
				CefClient* obj = cast_to<CefClient>(val.to_object());
				if (obj && obj->fBrowser != NULL && obj->m_url != "")
				{
					// Local coord of mouse IN PIXELS.
					int	x, y, buttons;
					mc->get_mouse_state(&x, &y, &buttons);

					matrix	m;
					mc->get_world_matrix(&m);

					point	a(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
					point	b;
					m.transforfBy_inverse(&b, a);

					cef_mouse_event_t event;
					event.modifiers = 0;
					event.x = (int)ceil(TWIPS_TO_PIXELS(b.m_x));
					event.y = (int)ceil(TWIPS_TO_PIXELS(b.m_y));

					cef_browser_host_t* host = obj->fBrowser->get_host(obj->fBrowser);
					host->set_focus(host, 1);
					host->send_focus_event(host, 1);
					host->send_mouse_move_event(host, &event, false);
				}*/
	}

	void	CefClient_mouse_press()
	{
		/*as_value val;
		mc->get_member("_CefPtr_", &val);
		CefClient* obj = cast_to<CefClient>(val.to_object());
		if (obj && obj->fBrowser && obj->m_url != "")
		{
			// Local coord of mouse IN PIXELS.
			int	x, y, buttons;
			mc->get_mouse_state(&x, &y, &buttons);

			matrix	m;
			mc->get_world_matrix(&m);

			point	a(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
			point	b;
			m.transforfBy_inverse(&b, a);

			cef_mouse_event_t event;
			event.modifiers = 0; // EVENTFLAG_LEFT_MOUSE_BUTTON;
			event.x = (int)ceil(TWIPS_TO_PIXELS(b.m_x));
			event.y = (int)ceil(TWIPS_TO_PIXELS(b.m_y));

			cef_browser_host_t* host = obj->fBrowser->get_host(obj->fBrowser);
			host->set_focus(host, 1);
			host->send_focus_event(host, 1);
			host->send_mouse_click_event(host, &event, MBT_LEFT, 0, 1);
		}*/
	}

	void	CefClient_mouse_release()
	{
		/*as_value val;
		mc->get_member("_CefPtr_", &val);
		CefClient* obj = cast_to<CefClient>(val.to_object());
		if (obj && obj->fBrowser && obj->m_url != "")
		{
			// Local coord of mouse IN PIXELS.
			int	x, y, buttons;
			mc->get_mouse_state(&x, &y, &buttons);

			matrix	m;
			mc->get_world_matrix(&m);

			point	a(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
			point	b;
			m.transforfBy_inverse(&b, a);

			cef_mouse_event_t event;
			event.modifiers = 0;
			event.x = (int)ceil(TWIPS_TO_PIXELS(b.m_x));
			event.y = (int)ceil(TWIPS_TO_PIXELS(b.m_y));

			cef_browser_host_t* host = obj->fBrowser->get_host(obj->fBrowser);
			host->send_mouse_click_event(host, &event, MBT_LEFT, 1, 1);
		}*/
	}

	void	CefClient_keydown()
	{
		/*as_value val;
		mc->get_member("_CefPtr_", &val);
		CefClient* obj = cast_to<CefClient>(val.to_object());
		if (obj && obj->fBrowser != NULL && obj->m_url != "")
		{
			as_value key;
			get_global()->get_member("Key", &key);
			as_key* ko = cast_to<as_key>(key.to_object());
			int lastkey = ko->get_last_key_pressed();
			Uint16 utf16char = ko->get_last_utf16_key_pressed();

			cef_browser_host_t* host = obj->fBrowser->get_host(obj->fBrowser);

			cef_key_event_t event = {};
#ifdef WIN32
			event.windows_key_code = utf16char;
#else
			event.windows_key_code = utf16char;
			event.character = utf16char;
#endif

			//			printf("cef lastkey: %d, utf16key %d\n", lastkey, utf16char);

			event.type = KEYEVENT_RAWKEYDOWN;
			host->send_key_event(host, &event);

			event.type = KEYEVENT_KEYUP;
			host->send_key_event(host, &event);

			// hack, tab,left,right,home,end, del, enter
			if (lastkey != 9 && lastkey != 35 && lastkey != 37 && lastkey != 36 && lastkey != 39 && lastkey != 46 && lastkey != 13)
			{
				event.type = KEYEVENT_CHAR;
				host->send_key_event(host, &event);
			}
		}*/
	}

	/*	void	CefClient_url_setter(const fn_call& fn)
		{
			CefClient* obj = cast_to<CefClient>(fn.this_ptr);
			if (obj && fn.nargs > 0 && obj->fBrowser != NULL)
			{
				obj->m_url = fn.arg(0).to_tu_string();

				sprite_instance* mc = obj->m_parent.get();
				assert(mc);
				if (obj->m_url == "")
				{
					// clear
					mc->builtin_member("onMouseMove", as_value());
					mc->builtin_member("onPress", as_value());
					mc->builtin_member("onRelease", as_value());
					mc->builtin_member("onKeyDown", as_value());
				}
				else
				{
					mc->builtin_member("onMouseMove", CefClient_mouse_move);
					mc->builtin_member("onPress", CefClient_mouse_press);
					mc->builtin_member("onRelease", CefClient_mouse_release);

					// Key.addListener(mc)
					as_value key;
					get_global()->get_member("Key", &key);

					as_value func;
					key.to_object()->get_member("addListener", &func);

					as_environment* env = mc->get_environment();
					env->push(mc);
					call_method(func, env, key, 1, env->get_top_index());
					env->drop(1);

					mc->builtin_member("onKeyDown", CefClient_keydown);
				}

				sendMessage("onLoadURL", fn.arg(0));

				cef_frame_t* frame = obj->fBrowser->get_main_frame(obj->fBrowser);
				if (obj->m_url.size() > 0)
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
					cef_string_utf8_to_utf16(obj->m_url.c_str(), obj->m_url.size(), &url);
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
			}
		}*/


	CefClient::CefClient(const Rect& outBounds, const char* url)
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
		printf("ctor CefClient\n");

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

		fRequestHandler.base.size = sizeof(fRequestHandler);
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
		Rtt_Log("cef_browser_host_create_browser %s\n", fBrowser == 0 ? "failed" : "successed");
	}

	CefClient::~CefClient()
	{
		printf("dtor CefClient\n");
		if (fBrowser)
		{
			cef_browser_host_t* host = fBrowser->get_host(fBrowser);
			host->close_browser(host, true);
		}
	}

	void CefClient::UpdateTex(const uint8_t* buf, int width, int height)
	{
		printf("onPaint\n");
		memcpy(GetBitmap(), buf, width * height * 4);
		fTex->GetTexture().Invalidate(); // Force Renderer to update GPU texture
	}

	void CefClient::InitGeometry()
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

#ifdef USE_NV12_SHADER
		Shader* shader = factory.FindOrLoad(ShaderTypes::kCategoryComposite, "yuv420v");
#else
		Shader* shader = &factory.GetDefault();
#endif
		shader->Prepare(fData, 0, 0, ShaderResource::kDefault);
	}

	void	CefClient::advance()
	{
		cef_do_message_loop_work();
	}

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

}	// namespace bakeinflash
