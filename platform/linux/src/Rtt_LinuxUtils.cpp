#include "Rtt_LinuxUtils.h"
#include "Rtt_Assert.h"
#include "Rtt_LinuxCrypto.h"
#include "string.h"
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <algorithm>
#include <iterator>

using namespace std;

int SortVectorByName(string a, string b)
{
	return a < b;
}

string& ltrim(string& str, const string& chars)
{
	str.erase(0, str.find_first_not_of(chars));
	return str;
}

string& rtrim(string& str, const string& chars)
{
	str.erase(str.find_last_not_of(chars) + 1);
	return str;
}

string& trim(string& str, const std::string& chars)
{
	return ltrim(rtrim(str, chars), chars);
}

void splitString(vector<string>& cont, const string& str, const string& delims)
{
	const char* s = str.c_str();
	const char* delim = strstr(s, delims.c_str());
	while (delim)
	{
		string item(s, delim - s);
		cont.push_back(trim(item));
		s = delim + delims.size();
		delim = strstr(s, delims.c_str());
	}
	if (*s != 0)
		cont.push_back(s);
}

const char* GetStartupPath(string* exeFileName)
{
	static char buf[PATH_MAX + 1];
	ssize_t count = readlink("/proc/self/exe", buf, PATH_MAX);
	buf[count] = 0;

	// remove file name
	char* filename = strrchr(buf, '/');
	Rtt_ASSERT(filename);

	if (exeFileName)
	{
		*exeFileName = filename + 1;
	}

	*filename = 0;

	return buf;
}

const char* GetHomePath()
{
	const char* homeDir = NULL;

	if ((homeDir = getenv("HOME")) == NULL)
	{
		homeDir = getpwuid(getuid())->pw_dir;
	}

	return homeDir;
}

char* CalculateMD5(string filename)
{
	Rtt::LinuxCrypto crypto;
	U8 digest[Rtt::MCrypto::kMaxDigestSize];
	size_t digestLen = crypto.GetDigestLength(Rtt::MCrypto::kMD5Algorithm);
	Rtt::Data<const char> data(filename.c_str(), (int)filename.length());
	crypto.CalculateDigest(Rtt::MCrypto::kMD5Algorithm, data, digest);

	char* hex = (char*)calloc(sizeof(char), digestLen * 2 + 1);

	for (unsigned int i = 0; i < digestLen; i++)
	{
		char* p = hex;
		p += sprintf(hex + 2 * i, "%02x", digest[i]);
	}

	return hex;
}

string GetRecentDocsPath()
{
	std::string recent_path = GetHomePath();
	recent_path += "/.Solar2D/recent_projects.conf";
	return recent_path;
}

bool ReadRecentDocs(vector<pair<string, string>>& recentDocs)
{
	recentDocs.clear();
	ifstream f(GetRecentDocsPath());
	if (f.is_open())
	{
		string line;
		while (getline(f, line))
		{
			std::vector<std::string> items;
			splitString(items, line, "=");
			if (items.size() == 2 && items[0].size() > 0 && items[1].size() > 0)
			{
				recentDocs.push_back(make_pair(items[0], items[1]));
			}
		}
		f.close();
		return recentDocs.size() > 0;
	}
	return false;
}

void UpdateRecentDocs(const std::string& appName, const std::string& path)
{
	vector<pair<string, string>> recentDocs;
	if (ReadRecentDocs(recentDocs))
	{
		// delete appName if it is in recentDocs already
		auto it = find_if(recentDocs.begin(), recentDocs.end(), [&](const pair<string, string>& elem) { return elem.first == appName; });
		if (it != recentDocs.end())
		{
			recentDocs.erase(it);
		}

		// max size is 7
		while (recentDocs.size() > 6)
		{
			recentDocs.pop_back();
		}
	}

	// save
	recentDocs.insert(recentDocs.begin(), make_pair(appName, path));
	ofstream f(GetRecentDocsPath());
	if (f.is_open())
	{
		for (int i = 0; i < recentDocs.size(); i++)
		{
			f << recentDocs[i].first << "=" << recentDocs[i].second << "\n";
		}
	}
}
