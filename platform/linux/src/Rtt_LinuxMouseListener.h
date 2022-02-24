#ifndef Rtt_Linux_Mouse_Listener
#define Rtt_Linux_Mouse_Listener

#include "Rtt_Event.h"
#include "Core/Rtt_Types.h"
#include "Rtt_Runtime.h"
#include <string>
#include <map>

namespace Rtt
{
	class LinuxMouseListener
	{
	public:
		float fScaleX, fScaleY;

	public:
		LinuxMouseListener(Runtime &runtime);
		void TouchDown(int x, int y, int id);
		void TouchMoved(int x, int y, int id);
		void TouchUp(int x, int y, int id);
		void DispatchEvent(const MEvent &e) const;

	private:
		struct pt
		{
			pt() : x(0), y(0) {}
			pt(int xx, int yy) : x(xx), y(yy) {}
			int x;
			int y;
		};

		Runtime &fRuntime;
		std::map<int, pt> fStartPoint; // finger id ==> point
	};
};

#endif // Rtt_Linux_Mouse_Listener
