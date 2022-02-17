//
// Solar2D Simulator Console
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
	, ImCtx(NULL)
	, fSocketServer(-1)
	, fSocketClient(-1)
{
	fParentPID = getppid();
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
	// Initialize SDL (Note: video is required to start event loop) 
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
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

	uint32_t windowStyle = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
	fWindow = SDL_CreateWindow("Solar2D Simulator Console", 0, 0, 640, 480, windowStyle);
	fGLcontext = SDL_GL_CreateContext(fWindow);
	SDL_GL_MakeCurrent(fWindow, fGLcontext);
	SDL_GL_SetSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImCtx = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsLight(); // StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(fWindow, fGLcontext);
	const char* glsl_version = "#version 130";
	ImGui_ImplOpenGL3_Init(glsl_version);

	// start unix socket server
	return ListenSocket();
}

int LinuxConsoleApp::Advance()
{
	if (fParentPID != getppid())
	{
		// parent was killed, needs to close console
		return -1;
	}

	// accept client
	uint32_t ln;
	sockaddr_in client_sockaddr = {};
	ln = sizeof(sockaddr_in);
	int client = (int) ::accept(fSocketServer, (sockaddr*)&client_sockaddr, &ln);
	if (client >= 0)
	{
		CloseClient();
		fSocketClient = client;
		SetNonBlocking(fSocketClient);
	}

	// there is a connection, read data
	if (ReadLog() < 0)
	{
		// Simulator closed
		CloseClient();
		return -1;
	}

	// set size
	const ImVec2& windowSize = ImGui::GetMainViewport()->WorkSize;
	ImGui::SetNextWindowSize(windowSize);

	// move to center
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	if (ImGui::Begin("##LogWindow", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs))
	{
		ImVec2 tbSize = windowSize;
		tbSize.x -= 15;	// hack
		tbSize.y -= 15;
		int len = fLogData.size();
		if (len > 0)
		{
			ImGui::InputTextMultiline("##LogData", (char*)fLogData.c_str(), len, tbSize, ImGuiInputTextFlags_ReadOnly);
		}
		ImGui::End();
	}
	return 0;
}

void LinuxConsoleApp::Run()
{
	while (true)
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

		if (app->Advance() < 0)
		{
			// Sumulator was closed
			break;
		}

		ImGui::EndFrame();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(fWindow);

		// Don't hog the CPU.
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
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

	// set_nonblock
	SetNonBlocking(fSocketServer);

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOLAR2D_UNIX_SOCKET, sizeof(addr.sun_path) - 1);

	if (::bind(fSocketServer, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		printf("bind() failed, %s\n", strerror(errno));
		CloseServerSocket();
		return false;
	}

	int channels = 1;
	if (::listen(fSocketServer, channels) == -1)
	{
		printf("listen() failed, %s\n", strerror(errno));
		CloseServerSocket();
		return false;
	}
	return true;
}

void LinuxConsoleApp::CloseServerSocket()
{
	CloseClient();
	if (fSocketServer >= 0)
	{
		close(fSocketServer);
		unlink(SOLAR2D_UNIX_SOCKET);
		fSocketServer = -1;
	}
}

int LinuxConsoleApp::ReadLog()
{
	if (fSocketClient < 0)
	{
		// wait
		return 0;
	}

	char buf[4096];
	int bytes_read = (int)recv(fSocketClient, buf, sizeof(buf), 0);
	if (bytes_read > 0)
	{
		// truncate
		const int maxsize = 20000;
		if (fLogData.size() > maxsize)
		{
			fLogData.erase(0, (fLogData.size() - maxsize) * 0.9);	// 10%
		}
		fLogData.append(buf, bytes_read);
		return bytes_read;
	}
	else if (bytes_read == 0)
	{
		// Socket must close
		printf("Connection was closed by remote side");
		CloseClient();
		return -1;
	}

	int err = errno;
	if (err == EAGAIN || err == EINPROGRESS)
	{
		return 0;    // wait
	}

	printf("%s\n", strerror(err));
	CloseClient();
	return -1;
}

void LinuxConsoleApp::CloseClient()
{
	fLogData.clear();
	if (fSocketClient >= 0)
	{
		close(fSocketClient);
		fSocketClient = -1;
	}
}