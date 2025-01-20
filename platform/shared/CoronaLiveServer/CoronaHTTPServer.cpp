//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "CoronaHTTPServer.h"

#include <event2/keyvalq_struct.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/thread.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/stat.h>

#include <memory>

#ifdef _WIN32
	#include <codecvt>
	#include <io.h>
	#include <stdio.h>
	int evbuffer_add_file_no_mmap(struct evbuffer *buf, int fd, ev_off_t offset, ev_off_t length);
	#define open _wopen
	#define close _close
	typedef struct _stat64 statS;
	#define stat _wstat64
	#define fstat _fstat64
	#ifndef PATH_MAX
		#define PATH_MAX 1024
	#endif
#else
	#define O_BINARY 0
	#include <unistd.h>
	#include <dirent.h>
	typedef struct stat statS;
#endif


#define DEFAULT_HTTP_PORT 20605


Rtt_HTTPServer::Rtt_HTTPServer()
: fEventTerminate(nullptr)
, fHttpd(nullptr)
, fBase(nullptr)
, fServerAddress("")
{
#if EVTHREAD_USE_WINDOWS_THREADS_IMPLEMENTED
//	#ifdef _WIN32
//		WSADATA WSAData;
//		WSAStartup(0x101, &WSAData);
//	#endif
	evthread_use_windows_threads();
#elif EVTHREAD_USE_PTHREADS_IMPLEMENTED
	evthread_use_pthreads();
#else
	static_assert(0, "No threading library enabled!");
#endif

	std::unique_lock<std::mutex> lock(fProjectMutex);
	fServerThread = std::thread(&Rtt_HTTPServer::ServerThread, this);
	fProjectNotifier.wait(lock);
}

Rtt_HTTPServer::~Rtt_HTTPServer()
{
	if(fEventTerminate)
		event_active(fEventTerminate, 0, 0);

	fProjectNotifier.notify_all();
	fServerThread.join();
}


void Rtt_HTTPServer::OnRequest(evhttp_request *req)
{
	const evhttp_uri * uri = evhttp_request_get_evhttp_uri(req);
	std::string urlPath;
	const char* c_urlPath = evhttp_uri_get_path(uri);
	if(c_urlPath)
		urlPath = c_urlPath;

	static const std::string sRoot = "/";
	static const std::string sIndex = "/index.html";

	struct evkeyvalq queryStruct = {0};
	std::unique_ptr<evkeyvalq, decltype(&evhttp_clear_headers)> query(&queryStruct, &evhttp_clear_headers);
	evhttp_parse_query_str(evhttp_uri_get_query(uri), query.get());

	LiveFS::Path rootDir;
	{
		std::unique_lock<std::mutex> lock(fProjectMutex);
		rootDir = fHTTPRoot;
	}

	if(rootDir.empty())
	{
		evhttp_send_error(req, 401, "Project is not set");
		return;
	}

	std::string subFilePath = urlPath;
	if(subFilePath == sRoot)
		subFilePath = sIndex;

	const char *decodedFilePath = evhttp_decode_uri(subFilePath.c_str());
	std::string utf8Path(decodedFilePath);
	free((void*)decodedFilePath);

	if(utf8Path.length()<=1)
	{
		evhttp_send_error(req, HTTP_BADREQUEST, "Must specify path");
		return;
	}

	if(utf8Path.find("/../")!=std::string::npos)
	{
		evhttp_send_error(req, HTTP_BADREQUEST, "Invalid path");
		return;
	}
	auto fullFilePath = rootDir + LiveFS::UTF8ToPath(utf8Path);

	int fd = open(fullFilePath.c_str(), O_RDONLY | O_BINARY);

	if(fd < 0)
	{
		evbuffer *buff = evhttp_request_get_output_buffer(req);
		evbuffer_add_printf(buff, "File not found: '%s'", utf8Path.c_str());
		evhttp_send_error(req, HTTP_NOTFOUND, NULL);
		return;
	}

	statS st = {0};
	if(fstat(fd, &st) != 0)
	{
		evbuffer *buff = evhttp_request_get_output_buffer(req);
		evbuffer_add_printf(buff, "No access: '%s' (cannot stat)", utf8Path.c_str());
		evhttp_send_error(req, HTTP_INTERNAL, NULL);
		return;
	}

//	evkeyvalq * headers = evhttp_request_get_output_headers(req);
//	evhttp_add_header(headers, "Content-Type", "application/octet-stream");
//	evhttp_add_header(headers, "Accept-Ranges", "bytes");
//	evhttp_add_header(headers, "Content-Transfer-Encoding", "binary");

	evbuffer *buff = evhttp_request_get_output_buffer(req);

	if(st.st_size > 1024)
		evbuffer_set_flags(buff, EVBUFFER_FLAG_DRAINS_TO_FD);

#ifndef _WIN32
	evbuffer_add_file(buff, fd, 0, st.st_size);
#else
	evbuffer_add_file_no_mmap(buff, fd, 0, st.st_size);
#endif
	evhttp_send_reply(req, HTTP_OK, NULL, buff);
}


void
Rtt_HTTPServer::ServerThread()
{
	std::unique_lock<std::mutex> lock(fProjectMutex);

	auto notifierFunc = [](std::condition_variable* toNotify){toNotify->notify_all();};
	std::unique_ptr<std::condition_variable, decltype(notifierFunc)> notifyOnEarlyReturn(&fProjectNotifier, notifierFunc);

	std::unique_ptr<event_base, decltype(&event_base_free)> eventBase(event_base_new(), &event_base_free);

	std::unique_ptr< event, decltype(&event_free) > terminateEvent(event_new(eventBase.get(), -1, 0, [](evutil_socket_t, short, void *evbase){
		event_base_loopexit((event_base*)evbase, NULL);
	}, eventBase.get()), &event_free);

	std::unique_ptr<evhttp, decltype(&evhttp_free)> httpd(evhttp_new(eventBase.get()), &evhttp_free);

	evhttp_set_allowed_methods(httpd.get(), EVHTTP_REQ_GET);

	evhttp_set_gencb(httpd.get(), [](evhttp_request *req, void *self){ ((Rtt_HTTPServer*)self)->OnRequest(req); }, this);

	fEventTerminate = terminateEvent.get();

	// bind

	if(evhttp_bind_socket(httpd.get(), "127.0.0.1", DEFAULT_HTTP_PORT) == -1) {
		fServerAddress = "";
	} else {
		fServerAddress = "http://127.0.0.1:" + std::to_string(DEFAULT_HTTP_PORT) + "/";
	}

	fBase = eventBase.get();
	fHttpd = httpd.get();
	lock.unlock();
	fProjectNotifier.notify_all();
	
	event_base_loop(eventBase.get(), EVLOOP_NO_EXIT_ON_EMPTY);

	fBase = nullptr;
	fHttpd = nullptr;
	fEventTerminate = nullptr;
	
}

void Rtt_HTTPServer::Set(const LiveFS::Path & path)
{
	std::unique_lock<std::mutex> lock(fProjectMutex);
	fHTTPRoot = path;
}

std::string Rtt_HTTPServer::GetServerAddress() const
{
	return fServerAddress;
}
