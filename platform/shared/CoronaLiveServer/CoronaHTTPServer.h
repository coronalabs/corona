//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
