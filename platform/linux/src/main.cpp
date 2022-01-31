//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#if !wxUSE_GLCANVAS
#error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif

#include "Rtt_LinuxSimulator.h"
#include "Rtt_LinuxUtils.h"
#include "Rtt_FileSystem.h"

using namespace std;

// global
wxFrame* solarApp = NULL;
smart_ptr<Rtt::SolarApp> app;

int main(int argc, char* argv[])
{
	string resourcesDir = GetStartupPath(NULL);
	resourcesDir.append("/Resources");

	// look for welcomescereen
	if (Rtt_FileExists((resourcesDir + "/homescreen/main.lua").c_str()))
	{
		resourcesDir.append("/homescreen");
		app = new Rtt::SolarSimulator();
	}
	else if (Rtt_IsDirectory(resourcesDir.c_str()))
	{
		app = new Rtt::SolarApp();
	}
	else
	{
		return -1;
	}


	if (app->Initialize())
	{
		app->Run();
	}

	//app->Start(resourcesDir);

	app = NULL;
	return 0;
}

