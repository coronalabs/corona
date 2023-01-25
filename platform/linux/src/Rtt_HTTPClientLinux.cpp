//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_HTTPClient.h"
#include "Rtt_Lua.h"
#include "Rtt_LinuxContainer.h"
#include <curl/curl.h>

using namespace std;

namespace Rtt
{
	static size_t write2buf(void* contents, size_t size, size_t nmemb, membuf* buf)
	{
		size_t bytes = size * nmemb;
		buf->append((char*)contents, bytes);
		return bytes;
	}

	static bool httpDownload(const char* url, String& errorMesg, const map<string, string>& headers, membuf* buf)
	{
		Rtt_ASSERT(url && "nullptr");

		errorMesg.Set("");
		CURL* curl = curl_easy_init();
		if (curl == NULL)
		{
			errorMesg.Set("HttpDownload: Failed to init CURL");
			return false;
		}

		// set headers
		struct curl_slist* chunk = NULL;
		for (const auto it : headers)
		{
			// Add a custom header 
			string h = it.first + ": " + it.second;
			chunk = curl_slist_append(chunk, h.c_str());
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);	// set custom set of headers

		curl_easy_setopt(curl, CURLOPT_URL, url);	// set URL
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);	// turn off full protocol/debug output (set to 1L to enable it)
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);	// disable progress meter (set to 0L to enable it)
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);	// follow redirection
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write2buf);	// set CURL callback, send all data to this function
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, buf);		// set response ptr for callback

		// execute the request
		CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			errorMesg.Set(curl_easy_strerror(res));
			curl_easy_cleanup(curl);
			return false;
		}

		curl_easy_cleanup(curl);
		return true;
	}

	bool HTTPClient::HttpDownload(const char* url, String& resultData, String& errorMesg, const map<string, string>& headers)
	{
		resultData.Set("");
		membuf buf;
		bool ok = httpDownload(url, errorMesg, headers, &buf);
		if (ok)
		{
			buf.append((uint8_t) 0);	// ensure ZERO
			resultData.Append((const char*) buf.data());
		}
		return ok;
	}

	bool HTTPClient::HttpDownloadFile(const char* url, const char* filename, String& errorMesg, const map<string, string>& headers)
	{
		membuf buf;
		bool ok = httpDownload(url, errorMesg, headers, &buf);
		if (ok)
		{
			FILE* f = fopen(filename, "wb");
			if (f == NULL)
			{
				errorMesg.Append(strerror(errno));
				return false;
			}

			size_t bytes = fwrite(buf.data(), 1, buf.size(), f);
			if (bytes != buf.size())
			{
				errorMesg.Append(strerror(errno));
				fclose(f);
				return false;
			}
			fclose(f);
		}
		return ok;
	}

} // namespace Rtt
