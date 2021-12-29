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
#include "Rtt_ConsoleApp.h"

using namespace std;

// global
SolarApp* solarApp = NULL;

class app : public wxApp
{
	bool OnInit() wxOVERRIDE
	{
		// look for welcomescereen
		string resourcesDir = GetStartupPath(NULL);
		resourcesDir.append("/Resources");

		if (Rtt_FileExists((resourcesDir + "/homescreen/main.lua").c_str()))
		{
			// start the console
			if (ConsoleApp::isStarted())
			{
				ConsoleApp::Clear();
			}
			else
			{
				std::string cmd(GetStartupPath(NULL));
				cmd.append("/Solar2DConsole");
				wxExecute(cmd);
			}
			// create the main simulator window
			solarApp = new SolarSimulator(resourcesDir);
		}
		else if (Rtt_IsDirectory(resourcesDir.c_str()))
		{
			// create the main application window
			solarApp = new SolarApp(resourcesDir);
		}
		return solarApp != NULL;
	}

	virtual ~app()
	{
		// Don't delete frame, it deleted by Core of wxWidgets
		solarApp = NULL;
	}
};

wxIMPLEMENT_APP_CONSOLE(app);
