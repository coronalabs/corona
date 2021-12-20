#include "Rtt_ConsoleApp.h"
#include "Rtt_LinuxIPCClient.h"
#include <time.h>
#include <sys/time.h>
#include <netdb.h> 
#include <sys/un.h>

#define LINUX_CONSOLE_CLEAR_CMD "###clear###"
#define LINUX_CONSOLE_QUIT_CMD "###quit###"

static Rtt_LinuxIPCClient* consoleClient = NULL;
static bool instanceCreated = false;
using namespace std;

bool ConsoleApp::isStarted()
// checking if unix socket IPC_SERVICE is available
{
	int rc = -1;
	int sd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sd >= 0)
	{
		struct sockaddr_un serveraddr;
		memset(&serveraddr, 0, sizeof(serveraddr));
		serveraddr.sun_family = AF_UNIX;
		strcpy(serveraddr.sun_path, IPC_SERVICE);

		rc = connect(sd, (struct sockaddr*)&serveraddr, SUN_LEN(&serveraddr));
		close(sd);
	}
	return rc >= 0;
}

void ConsoleApp::Log(const char* message, bool isError)
{
	if (consoleClient == wxNullPtr)
	{
		consoleClient = new Rtt_LinuxIPCClient();
	}

	if (!instanceCreated)
	{
		sleep(1);
		instanceCreated = true;
	}

	while (!consoleClient->IsConnected())
	{
		consoleClient->Connect(IPC_HOST, IPC_SERVICE, IPC_TOPIC);
	}

	time_t timeNow = time(NULL);
	char buffer[32];
	char msBuffer[32];
	int millisec;
	struct tm* timeInfo;
	struct timeval timeValue;

	gettimeofday(&timeValue, NULL);
	millisec = lrint(timeValue.tv_usec / 1000.0);

	if (millisec >= 1000)
	{
		millisec -= 1000;
		timeValue.tv_sec++;
	}

	timeInfo = localtime(&timeValue.tv_sec);
	strftime(buffer, 26, "%H:%M:%S.", timeInfo);
	sprintf(msBuffer, "%03d ", millisec);
	strcat(buffer, msBuffer);
	int bufferLen = strlen(buffer);
	string outputMessage;
	string messageCopy(message);
	size_t currentMsgPos = 0;
	string topic("information");
	string content;
	const string delimiter = "\n";
	const string warningPrefix = "WARNING:";
	const string errorPrefix = "ERROR:";

	while ((currentMsgPos = messageCopy.find(delimiter)) != string::npos)
	{
		content = messageCopy.substr(0, currentMsgPos);
		outputMessage.append(buffer).append(content).append(delimiter);
		messageCopy.erase(0, currentMsgPos + delimiter.length());
	}

	if (isError)
	{
		topic = "error";
	}
	else
	{
		if (outputMessage.length() > bufferLen + warningPrefix.length())
		{
			if (outputMessage.compare(bufferLen, warningPrefix.length(), warningPrefix) == 0)
			{
				topic = "warning";
			}
			else if (outputMessage.compare(bufferLen, errorPrefix.length(), errorPrefix) == 0)
			{
				topic = "error";
			}
		}
	}

	if (strcmp(message, LINUX_CONSOLE_CLEAR_CMD) == 0)
	{
		topic = "clear";
	}
	else if (strcmp(message, LINUX_CONSOLE_QUIT_CMD) == 0)
	{
		topic = "quit";
	}

	if (outputMessage.find("kShowRuntimeErrorsSet") == string::npos && outputMessage.find("luaDebugAvailable") == string::npos)
	{
		consoleClient->GetConnection()->Poke(topic, outputMessage.c_str());
	}
}

void ConsoleApp::Clear()
{
	Log(LINUX_CONSOLE_CLEAR_CMD);
}

void ConsoleApp::Quit()
{
	Log(LINUX_CONSOLE_QUIT_CMD);
}
