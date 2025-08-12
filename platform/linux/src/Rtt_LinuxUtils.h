#ifndef Rtt_LinuxUtils
#define Rtt_LinuxUtils

#include <stdlib.h>
#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <chrono>
#include "Rtt_LinuxContainer.h"

uint32_t GetTicks();
int SortVectorByName(std::string a, std::string b);

const char* GetStartupPath(std::string* exeFileName);
const char* GetHomePath();
std::string CalculateMD5(const std::string& filename);
std::string GetRecentDocsPath();
std::string GetSandboxPath(const std::string& appName);
std::string GetConfigPath(const std::string& appName);
std::string GetKeystorePath(const std::string& appName);
std::string GetPluginsPath();
bool ReadRecentDocs(std::vector<std::pair<std::string, std::string>>& recentDocs);
void UpdateRecentDocs(const std::string& appName, const std::string& path);

std::string& ltrim(std::string& str, const std::string& chars);
std::string& rtrim(std::string& str, const std::string& chars);
std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ");
void splitString(std::vector<std::string>& cont, const std::string& str, const std::string& delims);

bool OpenURL(const std::string& url);

// EVP Symmetric Encryption and Decryption
std::string Encrypt(const std::string& str);
std::string Decrypt(const std::string& str);

//
// simple thread wrapper
//
struct mythread : public ref_counted
{
	mythread()
		: m_go(false)
	{
	}

	virtual ~mythread()
	{
		stop();
	}

	// template functions MUST be implemented in the.h file, not in the.cpp
	template<typename F>
	void start(F func)
	{
		if (!m_go)
		{
			// [=] means keep alive 'func'
			m_go = true;
			m_promise = std::promise<bool>();
			m_thread = std::thread([=] {thread_func(func); });
		}
	}

	void stop()
	{
		if (m_go)
		{
			m_go = false;
			m_promise.get_future().get();
		}
		if (m_thread.joinable())
			m_thread.detach();
	}

	inline bool is_running() const { return m_go; }

private:

	// template functions MUST be implemented in the.h file, not in the.cpp
	template<typename F>
	void thread_func(F func)
	{
		// running in separate thread
		func();

		m_promise.set_value_at_thread_exit(true);
		stop();
	}

	std::atomic<bool> m_go;
	std::thread m_thread;
	std::promise<bool> m_promise;
};


#endif // Rtt_LinuxUtils
