//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_LinuxApp.h"

namespace Rtt
{

	class SolarSimulator : public SolarApp
	{
	public:
		SolarSimulator();
		virtual ~SolarSimulator();


		void OnFileSystemEvent(wxFileSystemWatcherEvent& event);
		void WatchFolder(const char* path, const char* appName) override;
		bool Start(const std::string& resourcesDir) override;


	private:
		wxFileSystemWatcher* fWatcher;

	};
}

