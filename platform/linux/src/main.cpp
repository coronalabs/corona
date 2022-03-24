//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_FileSystem.h"
#include "Rtt_LinuxSimulator.h"
#include "Rtt_LinuxUtils.h"
#include "Rtt_LinuxCEF.h"

using namespace std;

smart_ptr<Rtt::SolarApp> app;

int main(int argc, char* argv[])
{
	Rtt::InitCEF(argc, argv);

	string resourcesDir = GetStartupPath(NULL);
	resourcesDir.append("/Resources");

	// look for welcomescereen
	if (Rtt_FileExists((resourcesDir + "/homescreen/main.lua").c_str()))
	{
		resourcesDir.append("/homescreen");
		app = new Rtt::SolarSimulator(resourcesDir);
	}
	else if (Rtt_IsDirectory(resourcesDir.c_str()))
	{
		app = new Rtt::SolarApp(resourcesDir);
	}
	else
	{
		return -1;
	}

	if (app->Init())
	{
		app->Run();
	}

	Rtt::FinalizeCEF();
	app = NULL;

	return 0;
}

