//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#ifndef LiveServerCore_hpp
#define LiveServerCore_hpp

#undef HAVE_CLOCK_GETTIME

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <unordered_map>
#include <unordered_set>


struct event;
struct evhttp;
struct event_base;
struct evhttp_request;

class LiveFS {
public:
#ifdef _WIN32
	using Path = std::wstring;
#else
	using Path = std::string;
#endif
	static const Path sep;
	static Path UTF8ToPath(const std::string&);
	static std::string PathToUTF8(const Path&);
};

class Rtt_LiveServer
{
	static const LiveFS::Path CONFIG_FILENAME;
	static const unsigned MAX_LONG_POLL_QUEUE = 15;

	std::thread fServerThread;

	void OnRequest(evhttp_request *req);

	void OnComm(short flags);
	void ServerThread();

	static void SendUpdateRequest(evhttp_request * req, unsigned long long time);

	std::mutex fUpdaterMutex;
	std::condition_variable fUpdaterNotifier;

	std::mutex fProjectMutex;
	std::condition_variable fProjectNotifier;


	event* fEventTerminate;
	event_base* fBase;
	evhttp* fHttpd;

	std::unordered_map<std::string, LiveFS::Path> fProjectForKey;
	std::unordered_map<LiveFS::Path, std::list<evhttp_request*>> fPendingForPath;
	std::unordered_map<LiveFS::Path, long long> fLastUpdateForPath;
	std::unordered_map< std::string, std::string > fHashesMap;

	std::unordered_set<int> fBoundPorts;
	int BindPort(int configPort);

	static unsigned long long SteadyClock();
	void Update(const LiveFS::Path& path, unsigned long long clock);

public:
	Rtt_LiveServer();

	int Add(const LiveFS::Path& path);
	void Remove(const LiveFS::Path& path);
	void Update(const LiveFS::Path& path);

	~Rtt_LiveServer();
};


#endif /* LiveServerCore_hpp */
