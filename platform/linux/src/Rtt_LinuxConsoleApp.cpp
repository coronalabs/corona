//
//
//

#include "Rtt_LinuxConsoleApp.h"

smart_ptr<LinuxConsoleApp> app;
int main(int argc, char** argv)
{
	app = new LinuxConsoleApp();
	if (app->Init())
	{
		app->Run();
	}
	app = NULL;
	return 0;
}

LinuxConsoleApp::LinuxConsoleApp()
	: fWindow(NULL)
	, imctx(NULL)
{

}

LinuxConsoleApp::~LinuxConsoleApp()
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(fGLcontext);
	SDL_DestroyWindow(fWindow);
	SDL_Quit();
}

bool LinuxConsoleApp::Init()
{
	SDL_version ver;
	SDL_GetVersion(&ver);
	logi("SDL version %d.%d.%d\n", ver.major, ver.minor, ver.patch);

	// Initialize SDL (Note: video is required to start event loop) 
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		loge("Couldn't initialize SDL: %s\n", SDL_GetError());
		return false;
	}

	// GL 3.0 + GLSL 130
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE | SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

	uint32_t windowStyle = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI; // | SDL_WINDOW_BORDERLESS;
	windowStyle |= SDL_WINDOW_RESIZABLE;
	fWindow = SDL_CreateWindow("Solar2D Simulator Console", 0, 0, 640, 480, windowStyle);
	assert(fWindow);
	fGLcontext = SDL_GL_CreateContext(fWindow);
	assert(fGLcontext);
	SDL_GL_MakeCurrent(fWindow, fGLcontext);
	SDL_GL_SetSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context

	IMGUI_CHECKVERSION();
	imctx = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsLight(); // StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(fWindow, fGLcontext);
	const char* glsl_version = "#version 130";
	ImGui_ImplOpenGL3_Init(glsl_version);
}

void LinuxConsoleApp::Draw()
{
	if (ImGui::Begin("##DrawActivity", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs))
	{
		ImGui::Text("%c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
		ImGui::End();
	}
}


void LinuxConsoleApp::Run()
{
	while (1)
	{
		SDL_Event evt;
		while (SDL_PollEvent(&evt))
		{
			// GUI
			ImGui_ImplSDL2_ProcessEvent(&evt);

			if (evt.type == SDL_QUIT)
				return;
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		app->Draw();

		ImGui::EndFrame();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(fWindow);

		// Don't hog the CPU.
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
}

void LinuxConsoleApp::logi(const char* fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	const int bufsize = 1024 * 16;
	char* buf = (char*)malloc(bufsize);
	int len = vsnprintf(buf, bufsize - 1, fmt, arg);
	if (len > 0)
	{
		buf[len] = 0;
		printf("%s\n", buf);
	}
	free(buf);
	va_end(arg);
}

void LinuxConsoleApp::loge(const char* fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	const int bufsize = 1024 * 16;
	char* buf = (char*)malloc(bufsize);
	int len = vsnprintf(buf, bufsize - 1, fmt, arg);
	if (len > 0)
	{
		buf[len] = 0;
		printf("%s\n", buf);
	}
	free(buf);
	va_end(arg);
}