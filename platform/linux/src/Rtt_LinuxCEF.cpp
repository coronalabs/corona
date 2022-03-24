// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

#include "Rtt_LinuxCEF.h"

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


#ifdef WIN32
	void CEF_CALLBACK onResourceRedirect(struct _cef_request_handler_t* self, struct _cef_browser_t* browser, struct _cef_frame_t* frame, const cef_string_t* old_url, cef_string_t* new_url)
#else
	void CEF_CALLBACK onResourceRedirect(struct _cef_request_handler_t* self, struct _cef_browser_t* browser, struct _cef_frame_t* frame, struct _cef_request_t* request, cef_string_t* new_url)
#endif
	{
		cef_string_utf8_t dst = {};
		int n = cef_string_utf16_to_utf8(new_url->str, new_url->length, &dst);
		//sendMessage("onLoadURL", dst.str);
		cef_string_utf8_clear(&dst);
	}

	_cef_render_handler_t* CEF_CALLBACK getRenderHandler(struct _cef_client_t* self)
	{
		const weak_ptr<as_cef>& thiz = *(const weak_ptr<as_cef>*)((Uint8*)self + sizeof(struct _cef_client_t));
		return &thiz->m_render;
	}

	_cef_request_handler_t* CEF_CALLBACK getRequestHandler(struct _cef_client_t* self)
	{
		const weak_ptr<as_cef>& thiz = *(const weak_ptr<as_cef>*)((Uint8*)self + sizeof(struct _cef_client_t));
		return &thiz->m_requesthandler;
	}

	int CEF_CALLBACK getViewRect(struct _cef_render_handler_t* self, struct _cef_browser_t* browser, cef_rect_t* rect)
	{
		const weak_ptr<as_cef>& thiz = *(const weak_ptr<as_cef>*)((Uint8*)self + sizeof(struct _cef_render_handler_t));

		rect->x = 0;
		rect->y = 0;
		rect->width = thiz->m_bi->Width();
		rect->height = thiz->m_bi->Height();
		return 1;
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

		const weak_ptr<as_cef>& thiz = *(const weak_ptr<as_cef>*)((Uint8*)self + sizeof(struct _cef_render_handler_t));

		// todo sanity checks
		memcpy((char*)thiz->m_bi->GetBits(NULL), buf, width * height * 4);
		free(buf);
	}

	void	as_cef_mouse_move()
	{
		/*		as_value val;
				mc->get_member("_CefPtr_", &val);
				as_cef* obj = cast_to<as_cef>(val.to_object());
				if (obj && obj->m_browser != NULL && obj->m_url != "")
				{
					// Local coord of mouse IN PIXELS.
					int	x, y, buttons;
					mc->get_mouse_state(&x, &y, &buttons);

					matrix	m;
					mc->get_world_matrix(&m);

					point	a(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
					point	b;
					m.transform_by_inverse(&b, a);

					cef_mouse_event_t event;
					event.modifiers = 0;
					event.x = (int)ceil(TWIPS_TO_PIXELS(b.m_x));
					event.y = (int)ceil(TWIPS_TO_PIXELS(b.m_y));

					cef_browser_host_t* host = obj->m_browser->get_host(obj->m_browser);
					host->set_focus(host, 1);
					host->send_focus_event(host, 1);
					host->send_mouse_move_event(host, &event, false);
				}*/
	}

	void	as_cef_mouse_press()
	{
		/*as_value val;
		mc->get_member("_CefPtr_", &val);
		as_cef* obj = cast_to<as_cef>(val.to_object());
		if (obj && obj->m_browser && obj->m_url != "")
		{
			// Local coord of mouse IN PIXELS.
			int	x, y, buttons;
			mc->get_mouse_state(&x, &y, &buttons);

			matrix	m;
			mc->get_world_matrix(&m);

			point	a(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
			point	b;
			m.transform_by_inverse(&b, a);

			cef_mouse_event_t event;
			event.modifiers = 0; // EVENTFLAG_LEFT_MOUSE_BUTTON;
			event.x = (int)ceil(TWIPS_TO_PIXELS(b.m_x));
			event.y = (int)ceil(TWIPS_TO_PIXELS(b.m_y));

			cef_browser_host_t* host = obj->m_browser->get_host(obj->m_browser);
			host->set_focus(host, 1);
			host->send_focus_event(host, 1);
			host->send_mouse_click_event(host, &event, MBT_LEFT, 0, 1);
		}*/
	}

	void	as_cef_mouse_release()
	{
		/*as_value val;
		mc->get_member("_CefPtr_", &val);
		as_cef* obj = cast_to<as_cef>(val.to_object());
		if (obj && obj->m_browser && obj->m_url != "")
		{
			// Local coord of mouse IN PIXELS.
			int	x, y, buttons;
			mc->get_mouse_state(&x, &y, &buttons);

			matrix	m;
			mc->get_world_matrix(&m);

			point	a(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
			point	b;
			m.transform_by_inverse(&b, a);

			cef_mouse_event_t event;
			event.modifiers = 0;
			event.x = (int)ceil(TWIPS_TO_PIXELS(b.m_x));
			event.y = (int)ceil(TWIPS_TO_PIXELS(b.m_y));

			cef_browser_host_t* host = obj->m_browser->get_host(obj->m_browser);
			host->send_mouse_click_event(host, &event, MBT_LEFT, 1, 1);
		}*/
	}

	void	as_cef_keydown()
	{
		/*as_value val;
		mc->get_member("_CefPtr_", &val);
		as_cef* obj = cast_to<as_cef>(val.to_object());
		if (obj && obj->m_browser != NULL && obj->m_url != "")
		{
			as_value key;
			get_global()->get_member("Key", &key);
			as_key* ko = cast_to<as_key>(key.to_object());
			int lastkey = ko->get_last_key_pressed();
			Uint16 utf16char = ko->get_last_utf16_key_pressed();

			cef_browser_host_t* host = obj->m_browser->get_host(obj->m_browser);

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

/*	void	as_cef_url_setter(const fn_call& fn)
	{
		as_cef* obj = cast_to<as_cef>(fn.this_ptr);
		if (obj && fn.nargs > 0 && obj->m_browser != NULL)
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
				mc->builtin_member("onMouseMove", as_cef_mouse_move);
				mc->builtin_member("onPress", as_cef_mouse_press);
				mc->builtin_member("onRelease", as_cef_mouse_release);

				// Key.addListener(mc)
				as_value key;
				get_global()->get_member("Key", &key);

				as_value func;
				key.to_object()->get_member("addListener", &func);

				as_environment* env = mc->get_environment();
				env->push(mc);
				call_method(func, env, key, 1, env->get_top_index());
				env->drop(1);

				mc->builtin_member("onKeyDown", as_cef_keydown);
			}

			sendMessage("onLoadURL", fn.arg(0));

			cef_frame_t* frame = obj->m_browser->get_main_frame(obj->m_browser);
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


	as_cef::as_cef()
		: m_bi(NULL)
		, m_browser(NULL)
		, this_ptr_for_render(this)
		, this_ptr_for_client(this)
		, this_ptr_for_requesthandler(this)
	{
		memset(&m_window_info, 0, sizeof(m_window_info));
		memset(&m_bsettings, 0, sizeof(m_bsettings));
		memset(&m_render, 0, sizeof(m_render));
		memset(&m_client, 0, sizeof(m_client));
		memset(&m_requesthandler, 0, sizeof(m_requesthandler));

		int w = 111; 
		int h = 111;
		m_bi = new LinuxBaseBitmap(NULL, w, h, NULL);

		m_requesthandler.base.size = sizeof(m_requesthandler);
		//m_requesthandler.on_before_browse = onBeforeBrowse;
		//m_requesthandler.on_resource_redirect = onResourceRedirect;

		m_client.base.size = sizeof(m_client);
		m_client.get_render_handler = getRenderHandler;
		m_client.get_request_handler = getRequestHandler;

		m_window_info.windowless_rendering_enabled = true;
		//m_window_info.transparent_painting_enabled = true;

		m_bsettings.size = sizeof(m_bsettings);
		m_bsettings.windowless_frame_rate = 30;

		cef_string_t url = {};
		cef_client_t* m_client_ptr = &m_client;

		m_browser = cef_browser_host_create_browser_sync(&m_window_info, &m_client, &url, &m_bsettings,0,0);
	}

	as_cef::~as_cef()
	{
		if (m_browser)
		{
			cef_browser_host_t* host = m_browser->get_host(m_browser);
			host->close_browser(host, true);
		}
	}

	void	as_cef::advance()
	{
		cef_do_message_loop_work();
	}

	// factory
	as_cef* CreateWebview(const Rect& outBounds)
	{
		static int s_needs_to_init = true;
		if (s_needs_to_init)
		{
			s_needs_to_init = false;

			cef_main_args_t args = {};
			cef_settings_t settings = {};
			settings.multi_threaded_message_loop = false;
			settings.command_line_args_disabled = false;
			settings.no_sandbox = 1;

			int ok = cef_initialize(&args, &settings, NULL, NULL);
			if (!ok)
			{
				Rtt_LogException("cef_initialize failed\n");
			}
		}
		return new as_cef();
	}

	bool InitCEF(int argc, char** argv)
	{
		// This is called many times, because it is also used for subprocesses. 
		// If one of the first args is for example "--type=renderer" then it means that this is a Renderer process. 
		// There may be more subprocesses like GPU (--type=gpu-process) and others.
		// On Linux there are also special Zygote processes.

		if (argc > 1)
		{
			Rtt_Log("Subprocess: ");
			for (int i = 1; i < argc; i++)
			{
				Rtt_Log("%s ", argv[i]);
			}
			Rtt_Log("\n");
		}

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
		settings.no_sandbox = 1;

		// Initialize CEF.
		rc = cef_initialize(&main_args, &settings, NULL, NULL);

		// A return value of true (1) indicates that it succeeded and false (0) indicates that it failed.
		Rtt_Log("Main process: CEF Initialize %s\n", rc == 0 ? "failed" : "successed");
		return rc == 1;
	}

	void FinalizeCEF()
	{
		cef_shutdown();
	}

}	// namespace bakeinflash
