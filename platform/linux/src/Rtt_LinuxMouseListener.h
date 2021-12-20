#ifndef Rtt_Linux_Mouse_Listener
#define Rtt_Linux_Mouse_Listener

#include "Rtt_Event.h"
#include "Core/Rtt_Types.h"
#include "Rtt_Runtime.h"
#include "wx/wx.h"
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

	public:
		static void OnMouseLeftDoubleClick(wxMouseEvent &event);
		static void OnMouseLeftDown(wxMouseEvent &event);
		static void OnMouseLeftUp(wxMouseEvent &event);
		static void OnMouseRightDoubleClick(wxMouseEvent &event);
		static void OnMouseRightDown(wxMouseEvent &event);
		static void OnMouseRightUp(wxMouseEvent &event);
		static void OnMouseMiddleDoubleClick(wxMouseEvent &event);
		static void OnMouseMiddleDown(wxMouseEvent &event);
		static void OnMouseMiddleUp(wxMouseEvent &event);
		static void OnMouseMove(wxMouseEvent &event);
		static void OnMouseWheel(wxMouseEvent &event);

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
