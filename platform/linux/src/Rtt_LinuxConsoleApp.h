#ifndef LINUX_CONSOLE_APP_H
#define LINUX_CONSOLE_APP_H

#include "Rtt_LinuxContainer.h"
#include "Rtt_LinuxUtils.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imfilebrowser.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

struct LinuxConsoleApp: public ref_counted
{
	enum MessageType {Normal, Warning, Error};

	LinuxConsoleApp();
	virtual ~LinuxConsoleApp();

	bool Init();
	void Run();

private:

	void Draw();
	void logi(const char* fmt, ...);
	void loge(const char* fmt, ...);
	void logw(const char* fmt, ...);

	SDL_Window* fWindow;
	SDL_GLContext fGLcontext;

	// GUI
	ImGuiContext* imctx;
};


#endif //LINUXCONSOLEAPP_H
