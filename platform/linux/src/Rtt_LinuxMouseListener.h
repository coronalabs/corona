#ifndef Rtt_Linux_Mouse_Listener
#define Rtt_Linux_Mouse_Listener

#include "Rtt_Event.h"
#include "Core/Rtt_Types.h"
#include "Rtt_Runtime.h"
#include "Rtt_LinuxContainer.h"
#include <SDL.h>

namespace Rtt
{
	struct LinuxMouseListener : public ref_counted
	{
		float fScaleX, fScaleY;

		LinuxMouseListener();

		void TouchDown(int x, int y, int id);
		void TouchMoved(int x, int y, int id);
		void TouchUp(int x, int y, int id);
		void DispatchEvent(const MEvent& e) const;
		void OnEvent(const SDL_Event& evt, SDL_Window* window);

	private:
		struct pt
		{
			pt() : x(0), y(0) {}
			pt(int xx, int yy) : x(xx), y(yy) {}
			int x;
			int y;
		};

		std::map<int, pt> fStartPoint; // finger id ==> point
	};
};

#endif // Rtt_Linux_Mouse_Listener
