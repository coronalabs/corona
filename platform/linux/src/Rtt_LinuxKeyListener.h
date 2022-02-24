#ifndef Rtt_Linux_Key_Listener
#define Rtt_Linux_Key_Listener

#include <string>
#include <map>
#include "Rtt_Runtime.h"

namespace Rtt
{
	class LinuxKeyListener
	{
	public:
		LinuxKeyListener(Runtime &runtime);
		void notifyKeyEvent(bool down);
		void notifyCharEvent();

	public:
		static void OnChar();
		static void OnKeyDown();
		static void OnKeyUp();

	private:
		Runtime &fRuntime;
		std::map<int, std::string> fKeyName;
	};
};

#endif // Rtt_Linux_Key_Listener
