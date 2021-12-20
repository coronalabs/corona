#ifndef Rtt_Linux_Key_Listener
#define Rtt_Linux_Key_Listener

#include <string>
#include <map>
#include "Rtt_Runtime.h"
#include "wx/app.h"

namespace Rtt
{
	class LinuxKeyListener
	{
	public:
		LinuxKeyListener(Runtime &runtime);
		void notifyKeyEvent(wxKeyEvent &e, bool down);
		void notifyCharEvent(wxKeyEvent &e);

	public:
		static void OnChar(wxKeyEvent &event);
		static void OnKeyDown(wxKeyEvent &event);
		static void OnKeyUp(wxKeyEvent &event);

	private:
		Runtime &fRuntime;
		std::map<int, std::string> fKeyName;
	};
};

#endif // Rtt_Linux_Key_Listener
