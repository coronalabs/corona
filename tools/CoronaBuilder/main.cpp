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
#include "Rtt_LinuxUtils.h"
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>

#include "Core/Rtt_FileSystem.h"

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

	// create unique temporary dir
	const char *tmpDir = getenv("TMPDIR");
	if (tmpDir == NULL || tmpDir[0] == '\0')
	{
		tmpDir = "/tmp";
	}

	string sandboxTemplate = string(tmpDir) + "/Solar2DBuilder-XXXXXX";
	vector<char> sandboxPath(sandboxTemplate.begin(), sandboxTemplate.end());
	sandboxPath.push_back('\0');

	if (Rtt_MakeTempDirectory(sandboxPath.data()) == NULL)
	{
		fprintf(stderr, "ERROR: failed to create a temporary directory under %s\n", tmpDir);
		return 1;
	}

	const string sandboxDir(sandboxPath.data());
	documentsDir = sandboxDir + "/Documents";
	temporaryDir = sandboxDir + "/TemporaryFiles";
	cachesDir = sandboxDir + "/CachedFiles";
	systemCachesDir = sandboxDir + "/.system";

	Rtt::LinuxConsolePlatform *platform = new Rtt::LinuxConsolePlatform(pathToApp.c_str(), documentsDir.c_str(), temporaryDir.c_str(), cachesDir.c_str(), systemCachesDir.c_str(), skinDir.c_str(), GetStartupPath(NULL));
	Rtt::LinuxPlatformServices services(platform);

	Rtt::CoronaBuilder builder(*platform, services);
	result = builder.Main(argc, argv);

	// remove this run's temporary dir
	Rtt_DeleteDirectory(sandboxDir.c_str());

	return result;
}
