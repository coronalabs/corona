
#include "WinRTServices.h"
#include <thread>


void WinRTServices_Sleep(int milliseconds)
{
	if (milliseconds > 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	}
}
