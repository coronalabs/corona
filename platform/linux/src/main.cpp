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

#include "Rtt_LinuxApp.h"

// global
SolarApp* solarApp = NULL;

class app : public wxApp
{
	bool OnInit() wxOVERRIDE
	{
		// create the main application window
		solarApp = new SolarApp();
		return true;
	}

	virtual ~app()
	{
		// Don't delete frame, it deleted by Core of wxWidgets
		solarApp = NULL;
	}
};

wxIMPLEMENT_APP_CONSOLE(app);
