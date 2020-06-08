//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "CoronaLiveServerCore.h"

#include <event2/keyvalq_struct.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/thread.h>

#include <fcntl.h>
#include <sys/stat.h>

#include <fstream>
#include <sstream>

#include <memory>

#ifdef _WIN32
#include <codecvt>
#include <io.h>
#include <stdio.h>
#pragma comment(lib, "event_core.lib")
#pragma comment(lib, "event_extra.lib")
#define open _wopen
#define close _close
typedef struct _stat64 statS;
#define stat _wstat64
#define fstat _fstat64
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
long readlink(const TCHAR *, char*, long) { return -1; };
#else
#define O_BINARY 0
#include <unistd.h>
#include <dirent.h>
typedef struct stat statS;
#endif


#define DEFAULT_PORT 20604

#define ENABLE_SSL 1

#if ENABLE_SSL

#define SHA_CLIENT_SALT "^eC*4K1ButGpA1Q8"
#define SHA_SERVER_SALT "C@!GEB@v2tLN_CG$e"

#ifdef _WIN32
#pragma comment(lib, "event_openssl.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Ws2_32.lib")
#endif

#include <event2/bufferevent_ssl.h>
#include <openssl/ssl.h>
namespace SSL_Certs {

	static const unsigned char crt[] = {48,130,3,80,48,130,2,56,2,9,0,164,194,100,150,121,67,245,238,48,13,6,9,42,134,72,134,247,13,1,1,11,5,0,48,105,49,11,48,9,6,3,85,4,6,19,2,85,83,49,19,48,17,6,3,85,4,8,19,10,67,97,108,105,102,111,114,110,105,97,49,18,48,16,6,3,85,4,7,19,9,80,97,108,111,32,65,108,116,111,49,20,48,18,6,3,85,4,10,19,11,67,111,114,111,110,97,32,76,97,98,115,49,27,48,25,6,3,85,4,11,19,18,67,111,114,111,110,97,32,76,105,118,101,32,83,101,114,118,101,114,48,32,23,13,49,54,49,48,49,52,49,53,48,54,49,52,90,24,15,50,48,53,48,48,49,48,49,49,53,48,54,49,52,90,48,105,49,11,48,9,6,3,85,4,6,19,2,85,83,49,19,48,17,6,3,85,4,8,19,10,67,97,108,105,102,111,114,110,105,97,49,18,48,16,6,3,85,4,7,19,9,80,97,108,111,32,65,108,116,111,49,20,48,18,6,3,85,4,10,19,11,67,111,114,111,110,97,32,76,97,98,115,49,27,48,25,6,3,85,4,11,19,18,67,111,114,111,110,97,32,76,105,118,101,32,83,101,114,118,101,114,48,130,1,34,48,13,6,9,42,134,72,134,247,13,1,1,1,5,0,3,130,1,15,0,48,130,1,10,2,130,1,1,0,210,225,179,26,172,133,117,104,15,226,15,31,78,239,185,50,166,138,253,101,54,5,181,50,219,173,239,132,88,201,82,64,124,124,202,15,76,188,172,203,236,199,214,158,30,111,222,237,214,204,243,209,35,109,231,149,112,232,112,235,58,110,83,101,18,31,61,86,251,189,213,65,127,253,224,74,145,192,53,5,140,21,251,18,85,140,214,103,70,6,189,51,32,18,69,253,167,127,85,156,120,88,8,247,5,207,118,111,29,82,254,12,240,49,10,150,156,0,77,65,134,126,219,1,156,16,104,139,133,238,170,168,193,192,158,141,18,8,185,64,81,173,78,87,53,211,169,110,24,238,26,187,6,56,243,142,189,92,211,93,192,245,204,208,234,65,120,164,80,124,184,32,166,115,14,245,217,236,192,67,63,188,27,208,98,241,37,38,217,247,119,169,83,185,77,126,221,204,82,40,174,119,225,92,47,36,189,183,138,252,10,92,216,179,66,153,255,211,147,215,81,179,223,114,167,52,198,72,20,28,93,27,17,214,131,200,135,52,233,211,152,207,161,112,40,246,27,112,104,103,204,185,175,143,201,141,2,3,1,0,1,48,13,6,9,42,134,72,134,247,13,1,1,11,5,0,3,130,1,1,0,133,144,44,124,214,129,16,117,215,13,194,176,246,5,172,240,108,24,192,19,68,9,26,200,137,44,228,53,45,122,152,207,36,117,73,247,109,249,232,58,79,125,4,231,187,97,56,143,73,60,168,92,183,54,128,227,119,197,137,172,7,161,110,142,180,133,164,14,58,24,196,92,112,25,167,65,151,129,160,73,211,83,63,249,139,249,66,234,105,23,53,157,40,165,81,231,165,118,215,183,67,205,188,163,101,97,77,140,21,247,2,33,154,203,107,69,151,219,50,252,152,110,165,235,231,86,11,35,251,18,36,171,228,109,214,231,25,197,30,2,109,207,64,216,57,44,40,191,198,62,239,169,142,249,64,144,164,115,223,80,215,58,242,85,54,5,136,118,109,57,138,98,201,124,93,181,39,35,86,17,66,145,129,255,85,50,99,33,147,86,162,127,187,163,171,158,242,181,66,7,218,36,37,198,70,55,1,48,56,211,191,228,167,101,66,23,146,60,117,236,244,199,252,223,116,98,10,36,93,148,24,175,32,113,211,66,210,54,82,172,20,124,134,88,91,107,38,113,144,172,207,2,234,215,214,237 };

	static const unsigned char key[] = {48,130,4,162,2,1,0,2,130,1,1,0,210,225,179,26,172,133,117,104,15,226,15,31,78,239,185,50,166,138,253,101,54,5,181,50,219,173,239,132,88,201,82,64,124,124,202,15,76,188,172,203,236,199,214,158,30,111,222,237,214,204,243,209,35,109,231,149,112,232,112,235,58,110,83,101,18,31,61,86,251,189,213,65,127,253,224,74,145,192,53,5,140,21,251,18,85,140,214,103,70,6,189,51,32,18,69,253,167,127,85,156,120,88,8,247,5,207,118,111,29,82,254,12,240,49,10,150,156,0,77,65,134,126,219,1,156,16,104,139,133,238,170,168,193,192,158,141,18,8,185,64,81,173,78,87,53,211,169,110,24,238,26,187,6,56,243,142,189,92,211,93,192,245,204,208,234,65,120,164,80,124,184,32,166,115,14,245,217,236,192,67,63,188,27,208,98,241,37,38,217,247,119,169,83,185,77,126,221,204,82,40,174,119,225,92,47,36,189,183,138,252,10,92,216,179,66,153,255,211,147,215,81,179,223,114,167,52,198,72,20,28,93,27,17,214,131,200,135,52,233,211,152,207,161,112,40,246,27,112,104,103,204,185,175,143,201,141,2,3,1,0,1,2,130,1,0,71,155,152,99,224,93,89,38,194,237,242,80,161,79,73,197,231,67,159,19,245,42,47,3,92,17,78,25,212,196,88,7,201,189,222,39,31,217,153,155,141,133,182,92,238,220,242,8,53,62,87,52,151,33,68,216,60,4,210,3,46,78,52,71,71,198,15,224,47,143,160,144,163,83,164,11,238,62,36,67,3,244,154,210,248,245,224,0,37,137,56,73,218,4,26,124,45,154,81,137,214,91,158,127,179,27,101,142,9,153,26,231,109,208,89,86,23,23,11,36,202,244,220,141,73,95,81,0,33,129,186,11,222,77,37,244,71,162,175,185,34,59,83,208,213,1,45,110,194,102,124,77,1,226,176,156,155,183,64,149,61,187,55,194,250,23,80,37,44,88,0,28,243,108,109,69,203,144,102,102,108,18,231,63,143,99,211,95,250,78,175,197,173,99,233,195,162,94,30,191,75,93,115,88,242,58,113,108,66,168,211,48,78,215,69,57,179,252,124,227,5,36,56,141,119,58,124,34,21,1,70,126,252,60,85,24,187,75,144,57,103,165,26,199,123,187,227,145,224,186,148,143,29,45,196,73,2,129,129,0,249,137,71,57,167,159,160,141,58,234,96,169,0,1,221,179,52,182,7,75,124,130,2,202,199,85,5,50,198,186,38,164,209,7,139,34,231,13,232,113,172,17,78,104,102,17,202,61,147,140,136,122,169,189,21,80,246,19,234,252,2,173,185,84,153,160,11,183,150,117,71,130,140,246,211,140,240,110,40,73,219,37,72,58,223,182,20,241,218,119,176,118,5,167,158,83,42,238,128,122,12,202,173,136,197,84,135,102,120,130,178,188,113,117,19,163,53,129,189,119,196,68,178,78,114,221,27,247,2,129,129,0,216,88,24,111,125,59,85,22,123,232,67,132,198,156,6,82,176,21,170,248,124,168,122,165,14,106,250,31,186,32,221,125,229,187,183,62,104,33,86,196,57,190,195,46,243,80,54,80,152,100,215,190,16,244,50,99,213,23,232,214,234,43,83,194,5,50,211,219,16,49,223,79,112,87,67,205,122,114,89,65,190,11,120,38,223,110,213,41,10,112,61,76,39,56,228,69,26,197,95,9,125,37,85,121,1,113,133,61,114,215,184,232,138,247,246,174,229,215,113,147,115,215,6,227,174,170,61,155,2,129,128,12,212,220,78,192,169,218,173,168,123,232,224,135,217,144,182,232,92,251,223,11,213,251,162,197,193,63,117,46,246,223,137,214,29,215,47,194,69,218,208,13,147,14,63,28,16,138,31,196,120,147,132,11,58,103,224,51,231,46,76,43,47,23,186,155,243,184,230,133,220,30,3,62,144,106,172,114,168,151,117,196,201,19,255,195,5,47,234,135,219,99,189,141,232,226,161,80,49,215,117,111,54,244,217,117,156,53,34,53,204,50,184,177,244,221,63,165,62,184,169,113,244,62,94,254,85,20,93,2,129,128,63,169,100,191,97,245,148,171,66,24,237,205,99,120,68,48,14,218,193,109,207,152,240,70,35,192,253,11,91,5,18,200,90,82,235,176,52,138,34,243,41,30,230,102,169,120,160,19,5,85,31,91,49,183,250,32,0,115,193,179,132,66,237,169,185,71,228,169,160,209,92,42,15,31,206,182,231,98,64,187,164,197,209,60,119,33,49,30,155,144,189,168,236,29,196,166,69,35,113,60,228,147,152,220,59,189,128,252,189,15,151,188,238,47,219,106,73,254,36,208,196,20,177,85,76,111,146,35,2,129,128,83,86,66,138,63,66,150,171,143,222,79,230,130,167,138,154,86,225,118,212,144,250,156,206,70,217,0,186,242,100,50,223,96,22,209,163,106,124,203,158,202,131,53,3,221,246,175,183,171,3,229,193,33,167,138,55,46,142,0,246,27,191,191,121,56,93,21,168,170,115,245,216,1,72,217,242,61,112,66,146,157,226,169,112,147,154,74,188,96,162,243,74,109,188,193,44,198,27,182,95,91,149,197,157,209,179,43,134,119,127,50,57,17,9,163,203,237,187,105,228,195,44,146,29,40,60,242,27};

}

class SSLSetup
{

	static std::once_flag sOnceSSLInit;

	static bufferevent* createSSLBuffer (event_base *base, void *arg)
	{
		SSL_CTX *ctx = (SSL_CTX *) arg;

		bufferevent* res = bufferevent_openssl_socket_new(base,
														  -1,
														  SSL_new(ctx),
														  BUFFEREVENT_SSL_ACCEPTING,
														  BEV_OPT_CLOSE_ON_FREE);
		return res;
	}

public:

	SSL_CTX * ctx;
	EC_KEY *ecdh;

	SSLSetup(evhttp *httpd)
	{
		std::call_once(sOnceSSLInit, []{
			SSL_library_init();
			SSL_load_error_strings();
			OpenSSL_add_all_algorithms();
		});

		ctx = SSL_CTX_new(SSLv23_method());

		ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
		SSL_CTX_set_tmp_ecdh (ctx, ecdh);

		int res = 0;

		res = SSL_CTX_use_certificate_ASN1(ctx, sizeof SSL_Certs::crt, SSL_Certs::crt);
		res = SSL_CTX_use_PrivateKey_ASN1(EVP_PKEY_RSA, ctx, SSL_Certs::key, sizeof SSL_Certs::key);

		res = SSL_CTX_check_private_key (ctx);

		evhttp_set_bevcb(httpd, &createSSLBuffer, ctx);

	}

	~SSLSetup()
	{
		SSL_CTX_free(ctx);
		EC_KEY_free(ecdh);
	}
};

std::once_flag SSLSetup::sOnceSSLInit;

#else

class SSLSetup
{
public:
	SSLSetup(void *){}
	~SSLSetup(){}
};

#endif

#ifdef _WIN32

const LiveFS::Path Rtt_LiveServer::CONFIG_FILENAME = L".CoronaLiveBuild";
const LiveFS::Path LiveFS::sep = L"\\";

LiveFS::Path LiveFS::UTF8ToPath(const std::string &strOrig)
{
	std::string str(strOrig);
	size_t found = str.find_first_of('/');
	while (found != std::string::npos)
	{
		str[found] = '\\';
		found = str.find_first_of('/', found + 1);
	}
	std::wstring_convert<std::codecvt_utf8<wchar_t>> c;
	return c.from_bytes(str);
}

std::string LiveFS::PathToUTF8(const LiveFS::Path &path)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> c;
	return c.to_bytes(path);
}

#else

const LiveFS::Path Rtt_LiveServer::CONFIG_FILENAME = ".CoronaLiveBuild";
const LiveFS::Path LiveFS::sep = "/";

LiveFS::Path LiveFS::UTF8ToPath(const std::string &str)
{
	return str;
}

std::string LiveFS::PathToUTF8(const LiveFS::Path &path)
{
	return path;
}


#endif

Rtt_LiveServer::Rtt_LiveServer()
: fEventTerminate(nullptr)
, fHttpd(nullptr)
, fBase(nullptr)
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
	fServerThread = std::thread(&Rtt_LiveServer::ServerThread, this);
	fProjectNotifier.wait(lock);
}

Rtt_LiveServer::~Rtt_LiveServer()
{
	if(fEventTerminate)
		event_active(fEventTerminate, 0, 0);

	fProjectNotifier.notify_all();
	fServerThread.join();
}

void
Rtt_LiveServer::Update(const LiveFS::Path& path)
{
	return Update(path, SteadyClock());
}

void
Rtt_LiveServer::Update(const LiveFS::Path& path, unsigned long long clock)
{
	if(path.empty())
		return;

	std::unique_lock<std::mutex> lock(fUpdaterMutex);


	fLastUpdateForPath[path] = clock;
	std::list<evhttp_request*> & requests = fPendingForPath[path];

	if(requests.empty())
		return;

	struct UpdateRequestInfo {
		unsigned long long clock;
		std::list<evhttp_request*> &requests;
		std::mutex* mutex;
		std::condition_variable *cv;
	} info = { clock, requests, &fUpdaterMutex, &fUpdaterNotifier};

	event* e = event_new(fBase, -1, 0, [](evutil_socket_t, short, void* userdata) {
		UpdateRequestInfo * info = (UpdateRequestInfo*)userdata;
		std::unique_lock<std::mutex> lock(*info->mutex);

		for(evhttp_request*req : info->requests) {
			SendUpdateRequest(req, info->clock);
		}
		info->requests.clear();
		info->cv->notify_one();

	}, &info);

	event_active(e, 0, 0);
	while (!requests.empty()) fUpdaterNotifier.wait(lock);
	event_free(e);

}

unsigned long long Rtt_LiveServer::SteadyClock()
{
	auto t = std::chrono::steady_clock::now();
	auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(t);
	auto ret = ms.time_since_epoch().count();
	return ret;
}

void Rtt_LiveServer::SendUpdateRequest(evhttp_request * req, unsigned long long time) 
{
	evbuffer *buff = evhttp_request_get_output_buffer(req);
	evkeyvalq * headers = evhttp_request_get_output_headers(req);
	evhttp_add_header(headers, "Content-Type", "text/plain");
	evbuffer_add_printf(buff, "%llu", time);
	evhttp_send_reply(req, HTTP_OK, NULL, buff);
}

inline bool IfNameValid(const std::string &n, const LiveFS::Path &fullPath)
{
	if( n.empty() )
		return false;

	if (n[0] == '.')
		return false;

#ifdef _WIN32
	DWORD attributes = GetFileAttributes(fullPath.c_str());
	if (attributes & FILE_ATTRIBUTE_HIDDEN || attributes & FILE_ATTRIBUTE_SYSTEM)
		return false;
#endif

	return true;
}

std::string sha256(std::string str)
{
	unsigned char hashBin[SHA256_DIGEST_LENGTH];
	SHA256((const unsigned char *)str.c_str(), str.length(), hashBin);
	char hashString[SHA256_DIGEST_LENGTH*2+1] = {0};
	for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		sprintf(hashString + (i * 2), "%02x", hashBin[i]);
	}
	hashString[SHA256_DIGEST_LENGTH*2]=0;
	return hashString;
}

#ifdef _WIN32
int
evbuffer_add_file_no_mmap(struct evbuffer *buf, int fd, ev_off_t offset, ev_off_t length)
{
	struct evbuffer_file_segment *seg;
	unsigned flags = EVBUF_FS_CLOSE_ON_FREE | EVBUF_FS_DISABLE_MMAP;
	int r;

	seg = evbuffer_file_segment_new(fd, offset, length, flags);
	if (!seg)
		return -1;
	r = evbuffer_add_file_segment(buf, seg, 0, length);
	if (r == 0)
		evbuffer_file_segment_free(seg);
	return r;
}
#endif


void Rtt_LiveServer::OnRequest(evhttp_request *req)
{
	const evhttp_uri * uri = evhttp_request_get_evhttp_uri(req);
	std::string urlPath;
	const char* c_urlPath = evhttp_uri_get_path(uri);
	if(c_urlPath)
		urlPath = c_urlPath;

	static const std::string sRoot = "/";
	static const std::string sIndex = "/index.html";
	static const std::string sFiles = "/files";
	static const std::string sStatus = "/status";
	static const std::string sUpdate = "/update";

	struct evkeyvalq queryStruct = {0};
	std::unique_ptr<evkeyvalq, decltype(&evhttp_clear_headers)> query(&queryStruct, &evhttp_clear_headers);
	evhttp_parse_query_str(evhttp_uri_get_query(uri), query.get());

	LiveFS::Path rootDir;

	const char* projectKey = evhttp_find_header(query.get(), "key");
	if(projectKey != nullptr && *projectKey)
	{
		std::unique_lock<std::mutex> lock(fProjectMutex);

		const auto itHash = fHashesMap.find(projectKey);
		if(itHash != fHashesMap.end())
		{
			evbuffer *buff = evhttp_request_get_output_buffer(req);
			evkeyvalq * headers = evhttp_request_get_output_headers(req);
			evhttp_add_header(headers, "Content-Type", "text/plain; charset=ascii");
			evbuffer_add_printf(buff, "%s", itHash->second.c_str());
			evhttp_send_reply(req, HTTP_OK, NULL, buff);
			return;
		}

		const auto itProject = fProjectForKey.find(projectKey);
		if(itProject!=fProjectForKey.end()) {
			rootDir = itProject->second;
		}
	}

	if(rootDir.empty())
	{
		evhttp_send_error(req, 401, "Invalid project");
		return;
	}

	if(urlPath == sStatus)
	{
		using PathViewPair = std::pair<LiveFS::Path, std::string>;
		std::list< PathViewPair > files;
		std::list< std::string > directories;
		std::list< std::pair<std::string,std::string> > symlinks;

		std::list< PathViewPair > pathsToBrowse;
		pathsToBrowse.push_back(std::make_pair(rootDir, std::string("/")));
		enum class EntryType {Invalid, File, Directory, Symlink};
		auto processEntry = [&files, &directories, &pathsToBrowse, &symlinks](const LiveFS::Path &fsName, EntryType type, const LiveFS::Path& fsDir, const std::string& strDir) {
			auto strName = LiveFS::PathToUTF8(fsName);
			auto fsFullPath = fsDir + LiveFS::sep + fsName;
			if (IfNameValid(strName, fsFullPath))
			{
				if (type == EntryType::Directory)
				{
					std::string strResult = strDir + strName + '/';
					directories.push_back(strResult);
					pathsToBrowse.push_back(std::make_pair(fsFullPath, strResult));
				}
				else if(type == EntryType::File)
				{
					files.push_back(std::make_pair(fsFullPath, strDir + strName));
				}
				else if(type == EntryType::Symlink)
				{
					char *buff = new char[PATH_MAX];
					long len = -1;
					if(buff && (len = readlink(fsFullPath.c_str(), buff, PATH_MAX))>0) {
						buff[len] = 0;
						symlinks.push_back( std::make_pair( strDir + strName, buff) );
					}
					delete [] buff;
				}
			}
		};

		while(!pathsToBrowse.empty())
		{
			auto currDir = pathsToBrowse.front();
			const auto & fsDir = currDir.first;
			const auto & strDir = currDir.second;
			pathsToBrowse.pop_front();

#if _WIN32
			HANDLE dir = INVALID_HANDLE_VALUE;
			WIN32_FIND_DATA e = {0};

			dir = FindFirstFile((fsDir+_T("\\*")).c_str(), &e);
			if(dir != INVALID_HANDLE_VALUE)
			{
				do {
					LiveFS::Path fsName(e.cFileName);
					EntryType type = ((e.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)?EntryType::Directory:EntryType::File;
					processEntry(fsName, type, fsDir, strDir);
				} while(FindNextFile(dir, &e));
				FindClose(dir);
			}
#else
			DIR *dir;
			struct dirent *e;
			if ((dir = opendir(fsDir.c_str())) != NULL) {
				while ((e = readdir(dir)) != NULL) {
					LiveFS::Path fsName(e->d_name);
					EntryType type;
					switch (e->d_type) {
						case DT_REG:
							type = EntryType::File;
							break;
						case DT_DIR:
							type = EntryType::Directory;
							break;
						case DT_LNK:
							type = EntryType::Symlink;
							break;
						default:
							type = EntryType::Invalid;
							break;
					}
					processEntry(fsName, type, fsDir, strDir);
				}
				closedir(dir);
			}
#endif
		}

		evkeyvalq * headers = evhttp_request_get_output_headers(req);
		evhttp_add_header(headers, "Content-Type", "text/plain");

		evbuffer *buff = evhttp_request_get_output_buffer(req);

		evbuffer_add_printf(buff, "%lld / OK //\n", SteadyClock());

		for(const std::string & f : directories)
		{
			evbuffer_add_printf(buff, "0 / 0 / %s //\n", f.c_str());
		}

		statS st;
		for (const auto & f : files)
		{
			if(stat(f.first.c_str(), &st) == 0)
			{
				evbuffer_add_printf(buff, "%lld / %ld / %s //\n", (long long unsigned)st.st_size, (long unsigned)st.st_mtime, f.second.c_str());
			}
		}

		for(const auto & f : symlinks)
		{
			char * file = evhttp_encode_uri(f.first.c_str());
			char * dest = evhttp_encode_uri(f.second.c_str());
			if(file && dest)
			{
				evbuffer_add_printf(buff, "S&%s&%s //\n", file, dest);
			}
			if(file)
			{
				free(file);
			}
			if(dest)
			{
				free(dest);
			}
		}

		evhttp_send_reply(req, HTTP_OK, NULL, buff);

	}
	else if(urlPath == sUpdate)
	{
		const char* modified = evhttp_find_header(query.get(), "modified");

		std::unique_lock<std::mutex> lock(fUpdaterMutex);

		auto clock = fLastUpdateForPath[rootDir];

		if(modified)
		{
			auto mod = strtoll(modified, NULL, 10);
			if(mod >= clock) // let the long polling begin! Store requests, to answer them when something is modified
			{
				auto& requests = fPendingForPath.emplace(rootDir, std::list<evhttp_request*>() ).first->second;

				if(requests.size()>MAX_LONG_POLL_QUEUE)
				{
					evhttp_send_error(requests.front(), 429, "Too many requests!");
					requests.pop_front();
				}
				requests.push_back(req);
			}
			else
			{
				SendUpdateRequest(req, clock);
			}
		}
		else
		{
			SendUpdateRequest(req, clock);
		}

	}
	else if(urlPath.compare(0, sFiles.length(), sFiles) == 0)
	{
		std::string subFilePath = urlPath.substr(sFiles.length());
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

		int fd = open((rootDir + LiveFS::UTF8ToPath(utf8Path)).c_str(), O_RDONLY | O_BINARY);

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

		evkeyvalq * headers = evhttp_request_get_output_headers(req);
		evhttp_add_header(headers, "Content-Type", "application/octet-stream");
		evhttp_add_header(headers, "Accept-Ranges", "bytes");
		evhttp_add_header(headers, "Content-Transfer-Encoding", "binary");

		evbuffer *buff = evhttp_request_get_output_buffer(req);

#if ENABLE_SSL
		evbuffer_clear_flags(buff, EVBUFFER_FLAG_DRAINS_TO_FD);
#else
		if(st.st_size > 1024)
			evbuffer_set_flags(buff, EVBUFFER_FLAG_DRAINS_TO_FD);
#endif

#ifndef _WIN32
		evbuffer_add_file(buff, fd, 0, st.st_size);
#else
		evbuffer_add_file_no_mmap(buff, fd, 0, st.st_size);
#endif
		evhttp_send_reply(req, HTTP_OK, NULL, buff);

	}
	else if(urlPath.empty() || urlPath == sRoot || urlPath == sIndex )
	{
		evbuffer *buff = evhttp_request_get_output_buffer(req);
		evkeyvalq * headers = evhttp_request_get_output_headers(req);
		evhttp_add_header(headers, "Content-Type", "text/plain; charset=utf-8");
		evbuffer_add_printf(buff, "\xf0\x9f\x91\xbb");
		evhttp_send_reply(req, HTTP_OK, NULL, buff);

	}
	else
	{
		evhttp_send_error(req, HTTP_NOTFOUND, "File Not Found");
	}
}


static void
trimString(std::string& s)
{
	size_t p = s.find_first_not_of(" \t");
	s.erase(0, p);

	p = s.find_last_not_of(" \t");
	if (std::string::npos != p)
		s.erase(p+1);
}

void
Rtt_LiveServer::ServerThread()
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

	evhttp_set_gencb(httpd.get(), [](evhttp_request *req, void *self){ ((Rtt_LiveServer*)self)->OnRequest(req); }, this);

	auto sslSocket = SSLSetup(httpd.get());

	fEventTerminate = terminateEvent.get();

	fBase = eventBase.get();
	fHttpd = httpd.get();
	lock.unlock();
	fProjectNotifier.notify_all();
	
	event_base_loop(eventBase.get(), EVLOOP_NO_EXIT_ON_EMPTY);

	fBase = nullptr;
	fHttpd = nullptr;
	fEventTerminate = nullptr;
	
}

int Rtt_LiveServer::BindPort(int configPort)
{
	std::unique_lock<std::mutex> lock(fProjectMutex);

	if(configPort != INADDR_ANY && fBoundPorts.find(configPort) != fBoundPorts.end())
	{
		return configPort;
	}
	else if(configPort == INADDR_ANY && fBoundPorts.size() > 0)
	{
		return *fBoundPorts.begin();
	}


	struct HttpdCreateInfo {
		evhttp* httpd;
		int inPort;
		int outPort;
		std::mutex* mutex;
		std::condition_variable* cv;
	} info = { fHttpd, configPort, -2,&fProjectMutex, &fProjectNotifier};

	event* e = event_new(fBase, -1, 0, [](evutil_socket_t, short, void* userdata) {
		HttpdCreateInfo * info = (HttpdCreateInfo*)userdata;
		std::unique_lock<std::mutex> lock(*info->mutex);

		auto notifierFunc = [](std::condition_variable* toNotify){toNotify->notify_all(); };
		std::unique_ptr<std::condition_variable, decltype(notifierFunc)> notifyOnEarlyReturn(info->cv, notifierFunc);

		evhttp_bound_socket *socket = nullptr;
		if(info->inPort == 0) //Attempt to bind to hardcoded port to improve discoverability
		{
			socket = evhttp_bind_socket_with_handle(info->httpd, "0.0.0.0", DEFAULT_PORT);
		}

		if(socket == nullptr)
		{
			socket = evhttp_bind_socket_with_handle(info->httpd, "0.0.0.0", info->inPort);
		}

		if(!socket)
		{
			info->outPort = -4;
			return;
		}
		evutil_socket_t fd = evhttp_bound_socket_get_fd(socket);

		evutil_make_listen_socket_reuseable_port(fd);

		sockaddr_storage sst = { 0 };
		socklen_t len = sizeof(sst);

		int port = -5;
		getsockname(fd, (sockaddr*)&sst, &len);
		if (sst.ss_family == AF_INET)
		{
			port = ntohs(((sockaddr_in*)&sst)->sin_port);
		}
		else if (sst.ss_family == AF_INET6)
		{
			port = ntohs(((sockaddr_in6*)&sst)->sin6_port);
		}

		if(port>0) // attempt binding ipv6 on same port
		{
			evhttp_bind_socket(info->httpd, "::", port);
		}

		info->outPort = port>0?port:-5;

	}, &info);

	if(!e) {
		return -5;
	}

	event_active(e, 0, 0);
	while (info.outPort == -2) fProjectNotifier.wait(lock);
	event_free(e);

	return info.outPort;
}

int Rtt_LiveServer::Add(const LiveFS::Path & path)
{
	if(path.empty())
		return -1;

	std::string projectKey;
	LiveFS::Path rootPath(path);
	LiveFS::Path configPath = rootPath + LiveFS::sep + CONFIG_FILENAME;
	std::ifstream fsconfig (configPath, std::ifstream::in);

	static const std::string sProjectKey = "key";
	static const std::string sAddress = "ip";
	static const std::string sPort = "port";

	int configPort = INADDR_ANY;
	bool hasAddress = false;


	for(std::string line; std::getline(fsconfig, line); )
	{
		std::istringstream iss(line);
		std::string k, v;
		iss>>std::ws;
		bool readRes = true; // inner res
		readRes = readRes && std::getline(iss, k, '=');
		iss>>std::ws;
		readRes = readRes && std::getline(iss, v);

		trimString(k);
		trimString(v);
		readRes = readRes && k.length() && v.length();

		if(!readRes)
		{
			continue;
		}
		else if(k == sPort)
		{
			configPort = std::stoi(v);
		}
		else if(k == sProjectKey)
		{
			projectKey = v;
		}
		else if(k == sAddress)
		{
			hasAddress = true;
		}
	}

	if(projectKey.empty())
	{
		return -2;
	}
	else
	{
		std::unique_lock<std::mutex> lock(fProjectMutex);
		decltype(fProjectForKey)::const_iterator it = fProjectForKey.find(projectKey);
		if(it != fProjectForKey.end() && it->second != rootPath)
		{
			return -3;
		}
	}

	int port = BindPort(configPort);

	if(port>0)
	{
		std::unique_lock<std::mutex> lock(fProjectMutex);
		fBoundPorts.insert(port);
		fProjectForKey[projectKey] = rootPath;
		fHashesMap[sha256(projectKey + SHA_CLIENT_SALT)] = sha256(projectKey + SHA_SERVER_SALT);
	}

	return port;

}

void Rtt_LiveServer::Remove(const LiveFS::Path& path)
{
	if(path.empty()) {
		return;
	}
	Update(path, 0);
	std::unique_lock<std::mutex> lock1(fProjectMutex);
	std::unique_lock<std::mutex> lock2(fUpdaterMutex);

	for (auto it = fProjectForKey.begin(); it != fProjectForKey.end();) {
		if(it->second == path) {
			fHashesMap.erase(sha256(it->first + SHA_CLIENT_SALT));
			it = fProjectForKey.erase(it);
		}
		else {
			it++;
		}
	}

	fLastUpdateForPath.erase(path);
	fPendingForPath.erase(path);
}


