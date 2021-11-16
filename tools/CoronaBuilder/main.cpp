//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_CoronaBuilder.h"
#include "Rtt_LinuxConsolePlatform.h"
#include "Rtt_LinuxSimulatorView.h"
#include "Rtt_LinuxFileUtils.h"
#include <string>

using namespace std;

int main(int argc, const char *argv[])
{
	int result = 0;

	string pathToApp;
	string documentsDir;
	string temporaryDir;
	string cachesDir;
	string systemCachesDir;
	string skinDir;

	// setup directory paths
	documentsDir.append("/Documents");
	temporaryDir.append("/TemporaryFiles");
	cachesDir.append("/CachedFiles");
	systemCachesDir.append("/.system");

	Rtt::LinuxConsolePlatform *platform = new Rtt::LinuxConsolePlatform(pathToApp.c_str(), documentsDir.c_str(), temporaryDir.c_str(), cachesDir.c_str(), systemCachesDir.c_str(), skinDir.c_str(), Rtt::LinuxFileUtils::GetStartupPath(NULL));
	Rtt::LinuxPlatformServices services(platform);

	Rtt::CoronaBuilder builder(*platform, services);
	result = builder.Main(argc, argv);

	return result;
}
