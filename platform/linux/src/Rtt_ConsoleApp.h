#ifndef Rtt_Console_App
#define Rtt_Console_App

#include "Rtt_LinuxIPCClient.h"

class ConsoleApp
{
public:
	static void Log(const char *message, bool isError = false);
	static void Clear();
	static void Quit();
	static bool isStarted();
};

#endif // Rtt_Console_App
