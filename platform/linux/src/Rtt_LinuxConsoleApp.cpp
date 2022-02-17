//
//
//

#include "Rtt_LinuxConsoleApp.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netinet/ip.h> 

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
	, fSocketServer(-1)
	, fSocketClient(-1)
{
	fLogData.append("*** Solar2D Simulator Console ***\nListening unix socket ");
	fLogData.append(SOLAR2D_UNIX_SOCKET);
	fLogData.append("\n");
}

LinuxConsoleApp::~LinuxConsoleApp()
{
	CloseServerSocket();

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

	return ListenSocket();
}

void LinuxConsoleApp::Draw()
{
	// connect client
	if (fSocketServer >= 0 && fSocketClient < 0)
	{
		uint32_t ln;
		sockaddr_in client_sockaddr = {};
		ln = sizeof(sockaddr_in);
		fSocketClient = (int) ::accept(fSocketServer, (sockaddr*)&client_sockaddr, &ln);
		if (fSocketClient >= 0)
		{
			SetNonBlocking(fSocketClient);
		}
	}

	// there is a connection, read data
	if (fSocketClient >= 0)
	{
		ReadSocket();
	}


	// set size
	const ImVec2& windowSize = ImGui::GetMainViewport()->WorkSize;
	ImGui::SetNextWindowSize(windowSize);

	// move to center
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	if (ImGui::Begin("##LogWindow", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs) && fLogData.size() > 0)
	{
		ImVec2 tbSize = windowSize;
		tbSize.x -= 20;
		tbSize.y -= 20;
		ImGui::InputTextMultiline("##LogData", (char*)fLogData.data(), fLogData.size(), tbSize, ImGuiInputTextFlags_ReadOnly);

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

// set_nonblock
void LinuxConsoleApp::SetNonBlocking(int sd)
{
	int mode = fcntl(sd, F_GETFL, 0);
	mode |= O_NONBLOCK;
	fcntl(sd, F_SETFL, mode);
}

bool LinuxConsoleApp::ListenSocket()
{
	unlink(SOLAR2D_UNIX_SOCKET);
	fSocketServer = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fSocketServer < 0)
	{
		loge("Failed to create sock channel, err=%d\n", errno);
		return false;
	}

	// set_nonblock
	SetNonBlocking(fSocketServer);

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOLAR2D_UNIX_SOCKET, sizeof(addr.sun_path) - 1);

	if (::bind(fSocketServer, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		loge("Failed to bind usock channel, err=%d\n", errno);
		CloseServerSocket();
		return false;
	}

	int channels = 1;
	if (::listen(fSocketServer, channels) == -1)
	{
		loge("Failed to listen sock channel\n");
		CloseServerSocket();
		return false;
	}
	return true;
}

void LinuxConsoleApp::CloseServerSocket()
{
	if (fSocketClient >= 0)
	{
		close(fSocketClient);
		fSocketClient = -1;
	}
	if (fSocketServer >= 0)
	{
		close(fSocketServer);
		unlink(SOLAR2D_UNIX_SOCKET);
		fSocketServer = -1;
	}
}

int LinuxConsoleApp::ReadSocket()
{
	char buf[4096];
	int bytes_read = ::recv(fSocketClient, buf, sizeof(buf), 0);
	if (bytes_read > 0)
	{
		// truncate
		const int maxsize = 10 * 1024 * 1024;	// 10Mb
		if (fLogData.size() > maxsize)
		{
			fLogData.remove(fLogData.size() - maxsize);
		}
		fLogData.append(buf, bytes_read);
		return bytes_read;
	}
	else if (bytes_read == 0)
	{
		// Socket must close
		loge("Connection was closed by remote side");

		close(fSocketClient);
		fSocketClient = -1;
		return -1;
	}

	int err = errno;
	if (err == EAGAIN || err == EINPROGRESS)
		return 0;    // wait

	loge("%s\n", strerror(err));

	close(fSocketClient);
	fSocketClient = -1;
	return -1;
}
