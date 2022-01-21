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
Rtt::SolarApp* solarApp = NULL;

class app : public wxApp
{
	bool OnInit() wxOVERRIDE
	{
		if (!wxApp::OnInit())
			return false;

		string resourcesDir = GetStartupPath(NULL);
		resourcesDir.append("/Resources");

		// look for welcomescereen
		if (Rtt_FileExists((resourcesDir + "/homescreen/main.lua").c_str()))
		{
			resourcesDir.append("/homescreen");
			solarApp = new Rtt::SolarSimulator();
		}
		else if (Rtt_IsDirectory(resourcesDir.c_str()))
		{
			solarApp = new Rtt::SolarApp();
		}
		else
		{
			return false;
		}
		return solarApp->Start(resourcesDir);
	}

	virtual ~app()
	{
		// Don't delete frame, it deleted by Core of wxWidgets
		solarApp = NULL;
	}
};

wxIMPLEMENT_APP_CONSOLE(app);
