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

#ifndef HTTPServerCore_hpp
#define HTTPServerCore_hpp

#undef HAVE_CLOCK_GETTIME

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <unordered_map>
#include <unordered_set>

#include "CoronaLiveServerCore.h"

struct event;
struct evhttp;
struct event_base;
struct evhttp_request;


class Rtt_HTTPServer
{
	std::thread fServerThread;
	std::string fServerAddress;

	void OnRequest(evhttp_request *req);

	void ServerThread();

	std::mutex fProjectMutex;
	std::condition_variable fProjectNotifier;


	event* fEventTerminate;
	event_base* fBase;
	evhttp* fHttpd;

	LiveFS::Path fHTTPRoot;


public:
	Rtt_HTTPServer();

	void Set(const LiveFS::Path& path);
	std::string GetServerAddress() const;

	~Rtt_HTTPServer();
};


#endif /* HTTPServerCore_hpp */
