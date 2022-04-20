#include "Rtt_LinuxUtils.h"
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
#include <sys/time.h>
#include <sys/timeb.h>
#include <fcntl.h>


#include <openssl/evp.h>


using namespace std;

#define SOLAR2D_BASEDIR ".Solar2D"
uint32_t GetTicks()
{
	static timeb s_start_time;
	if (s_start_time.time == 0)
	{
		ftime(&s_start_time);
	}
	struct timeb tv;
	ftime(&tv);
	return  (uint32_t)(tv.time - s_start_time.time) * 1000 + (tv.millitm - s_start_time.millitm);
}

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

string& trim(string& str, const string& chars)
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

string CalculateMD5(const string& filename)
{
	Rtt::LinuxCrypto crypto;
	U8 digest[Rtt::MCrypto::kMaxDigestSize];
	size_t digestLen = crypto.GetDigestLength(Rtt::MCrypto::kMD5Algorithm);
	Rtt::Data<const char> data(filename.c_str(), (int)filename.length());
	crypto.CalculateDigest(Rtt::MCrypto::kMD5Algorithm, data, digest);

	string hex;
	for (unsigned int i = 0; i < digestLen; i++)
	{
		char s[3];
		snprintf(s, sizeof(s), "%02X", digest[i]);
		hex += s;
	}
	return hex;
}

string GetRecentDocsPath()
{
	string path = GetHomePath();
	path.append("/" SOLAR2D_BASEDIR "/recent.conf");
	return path;
}

string GetSandboxPath(const string& appName)
{
	string path = GetHomePath();
	path.append("/" SOLAR2D_BASEDIR "/Sandbox/");
	path.append(appName);
	return path;
}

string GetConfigPath(const string& appName)
{
	string path = GetHomePath();
	path.append("/" SOLAR2D_BASEDIR "/Sandbox/");
	path.append(appName);
	path.append("/app.conf");
	return path;
}

string GetKeystorePath(const string& appName)
{
	string path = GetHomePath();
	path.append("/" SOLAR2D_BASEDIR "/Sandbox/");
	path.append(appName);
	path.append("/keystore.conf");
	return path;
}

string GetPluginsPath()
{
	string path = GetHomePath();
	path.append("/" SOLAR2D_BASEDIR "/Plugins");
	return path;
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
			vector<string> items;
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

void UpdateRecentDocs(const string& appName, const string& path)
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

bool OpenURL(const string& url)
{
	string cmd("xdg-open \"");
	cmd.append(url);
	cmd.append("\"");
	return system(cmd.c_str()) == 0;
}

//
// Enrypt/Decrypt
//

unsigned char* key = (unsigned char*)"01234567890123456789012345678901"; // A 256 bit key
unsigned char* iv = (unsigned char*)"0123456789012345"; // A 128 bit IV

string Encrypt(const string& str)
{
	// Buffer for ciphertext. Ensure the buffer is long enough for the ciphertext which may be longer than the plaintext.
	unique_ptr<unsigned char> ciphertext((unsigned char*)malloc(str.size() + 128));
	int ciphertext_len = -1;

	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	if (ctx)
	{
		if (1 == EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
		{
			int len;
			if (1 == EVP_EncryptUpdate(ctx, ciphertext.get(), &len, (unsigned char*)str.c_str(), str.size()))
			{
				ciphertext_len = len;
				if (1 == EVP_EncryptFinal_ex(ctx, ciphertext.get() + len, &len))
				{
					ciphertext_len += len;
				}
			}
		}
		EVP_CIPHER_CTX_free(ctx);
	}

	string hex;
	if (ciphertext_len > 0)
	{
		for (int i = 0; i < ciphertext_len; i++)
		{
			char s[3];
			snprintf(s, sizeof(s), "%02X", ciphertext.get()[i]);
			hex.append(s);
		}
	}
	return hex;
}

string Decrypt(const string& str)
{
	int ciphertext_len = str.size() / 2;
	unsigned char* ciphertext = (unsigned char*)malloc(ciphertext_len);
	for (int i = 0; i < ciphertext_len; i++)
	{
		string byteString = str.substr(i * 2, 2);
		char byte = (char)strtol(byteString.c_str(), NULL, 16);
		ciphertext[i] = byte;
	}

	// Buffer for ciphertext. Ensure the buffer is long enough for the ciphertext which may be longer than the plaintext.
	unique_ptr<unsigned char> decryptedtext((unsigned char*)malloc(str.size()));

	int decryptedtext_len = -1;
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	if (ctx)
	{
		if (1 == EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
		{
			int len;
			if (1 == EVP_DecryptUpdate(ctx, decryptedtext.get(), &len, ciphertext, ciphertext_len))
			{
				decryptedtext_len = len;
				if (1 == EVP_DecryptFinal_ex(ctx, decryptedtext.get() + len, &len))
				{
					decryptedtext_len += len;
				}
			}
		}
		EVP_CIPHER_CTX_free(ctx);
	}
	return 	decryptedtext_len > 0 ? string((const char*)decryptedtext.get(), decryptedtext_len) : "";
}
