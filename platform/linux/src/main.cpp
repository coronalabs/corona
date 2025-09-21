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

	string arg;
	bool isNotCommandOption = false;
	for (int i = 1; i < argc; ++i)
	{
		arg = argv[i];
		if (arg.compare(0, 1, "-") != 0)
		{
			arg = argv[i];
			isNotCommandOption = true;
			break;
		}
	}

	if (argc > 1 && isNotCommandOption && Rtt_FileExists(arg.c_str()))
	{
		if (std::filesystem::is_regular_file(arg))
		{
			string fileParent = std::filesystem::path(arg).parent_path().string();
			app = new Rtt::SolarSimulator(std::filesystem::absolute(fileParent));
		}
		else
		{
			app = new Rtt::SolarSimulator(std::filesystem::absolute(arg));
		}
	}

	// look for welcomescereen
	else if (Rtt_FileExists((resourcesDir + "/homescreen/main.lua").c_str()))
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

