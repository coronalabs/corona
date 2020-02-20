//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <unordered_map>
#include <memory>

class Rtt_LiveServer;
class FSWatcher;

class CoronaLiveServerWin
{
public:
	static const unsigned sLiveServerUpdateDelay = 200; //ms.

	CoronaLiveServerWin();
	~CoronaLiveServerWin();

	int Add(const TCHAR* path);
	void Stop(const TCHAR* path);
private:
	std::unordered_map< FSWatcher*, std::shared_ptr<FSWatcher> > mFSWatchers;
	HWND m_hWnd;
	void *mAnnounceHandle;
	Rtt_LiveServer *mServer;
};

