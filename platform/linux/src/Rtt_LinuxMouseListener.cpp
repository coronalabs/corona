#include "Rtt_LinuxMouseListener.h"
#include "Rtt_MPlatformDevice.h"
#include "Rtt_LinuxContext.h"

namespace Rtt
{
	LinuxMouseListener::LinuxMouseListener(Runtime &runtime)
		: fRuntime(runtime), fScaleX(1), fScaleY(1)
	{
	}

	//  touch
	void LinuxMouseListener::TouchDown(int x, int y, int fid)
	{
		bool notifyMultitouch = fRuntime.Platform().GetDevice().DoesNotify(MPlatformDevice::kMultitouchEvent);
		x = (int)(x / fScaleX);
		y = (int)(y / fScaleY);

		// sanity check
		if (fStartPoint.find(fid) != fStartPoint.end() || (!notifyMultitouch && fStartPoint.size() > 0))
		{
			return;
		}

		fStartPoint[fid] = pt(x, y);

		TouchEvent t((float)x, (float)y, (float)x, (float)y, TouchEvent::kBegan);

		// it must not be ZERO!
		t.SetId((void *)(fid + 1));

#if Rtt_DEBUG_TOUCH
		printf("TouchDown %d(%d, %d)\n", fid, x, y);
#endif

		if (notifyMultitouch)
		{
			MultitouchEvent t2(&t, 1);
			DispatchEvent(t2);
		}
		else
		{
			DispatchEvent(t);
		}
	}

	void LinuxMouseListener::TouchMoved(int x, int y, int fid)
	{
		bool notifyMultitouch = fRuntime.Platform().GetDevice().DoesNotify(MPlatformDevice::kMultitouchEvent);

		x = (int)(x / fScaleX);
		y = (int)(y / fScaleY);

#if Rtt_DEBUG_TOUCH
// Commented out b/c it's very noisy
//printf("TouchMoved %d(%d, %d)\n", id, x, y);
#endif

		// sanity check
		if (fStartPoint.find(fid) == fStartPoint.end())
		{
			return;
		}

		TouchEvent t((float)x, (float)y, (float)fStartPoint[fid].x, (float)fStartPoint[fid].y, TouchEvent::kMoved);

		// it must not be ZERO!
		t.SetId((void *)(fid + 1));

		if (notifyMultitouch)
		{
			MultitouchEvent t2(&t, 1);
			DispatchEvent(t2);
		}
		else
		{
			DispatchEvent(t);
		}
	}

	void LinuxMouseListener::TouchUp(int x, int y, int fid)
	{
		bool notifyMultitouch = fRuntime.Platform().GetDevice().DoesNotify(MPlatformDevice::kMultitouchEvent);

		x = (int)(x / fScaleX);
		y = (int)(y / fScaleY);

		// sanity check
		if (fStartPoint.find(fid) == fStartPoint.end())
		{
			return;
		}

		TouchEvent t((float)x, (float)y, (float)fStartPoint[fid].x, (float)fStartPoint[fid].y, TouchEvent::kEnded);

		// it must not be ZERO!
		t.SetId((void *)(fid + 1));

#if Rtt_DEBUG_TOUCH
		printf("TouchUp %d(%d, %d)\n", fid, x, y);
#endif

		if (notifyMultitouch)
		{
			MultitouchEvent t2(&t, 1);
			DispatchEvent(t2);
		}
		else
		{
			DispatchEvent(t);
		}

		fStartPoint.erase(fid);
	}

	void LinuxMouseListener::DispatchEvent(const MEvent &e) const
	{
		fRuntime.DispatchEvent(e);
	}

	void LinuxMouseListener::OnMouseLeftDoubleClick(wxMouseEvent &event)
	{
		SolarAppContext *context = wxGetApp().GetFrame()->fContext;
		Runtime *runtime = context->GetRuntime();
		const int x = event.GetX();
		const int y = event.GetY();
		const bool isPrimaryDown = true;
		MouseEvent::MouseEventType eventType = MouseEvent::kUp;
		MouseEvent mouseEvent(eventType, x, y, Rtt_FloatToReal(0), Rtt_FloatToReal(0), 2, isPrimaryDown, false, false, false, false, false, false);
		TapEvent tapEvent(x, y, 2);

		runtime->DispatchEvent(mouseEvent);
		runtime->DispatchEvent(tapEvent);
	}

	void LinuxMouseListener::OnMouseLeftDown(wxMouseEvent &event)
	{
		SolarAppContext *context = wxGetApp().GetFrame()->fContext;
		Runtime *runtime = context->GetRuntime();
		const int x = event.GetX();
		const int y = event.GetY();
		const bool isPrimaryDown = true;
		MouseEvent::MouseEventType eventType = MouseEvent::kDown;
		MouseEvent mouseEvent(eventType, x, y, Rtt_FloatToReal(0), Rtt_FloatToReal(0), 0, isPrimaryDown, false, false, false, false, false, false);

		runtime->DispatchEvent(mouseEvent);
		context->GetMouseListener()->TouchDown(x, y, 0);
	}

	void LinuxMouseListener::OnMouseLeftUp(wxMouseEvent &event)
	{
		SolarAppContext *context = wxGetApp().GetFrame()->fContext;
		Runtime *runtime = context->GetRuntime();
		const int x = event.GetX();
		const int y = event.GetY();
		MouseEvent::MouseEventType eventType = MouseEvent::kUp;
		MouseEvent mouseEvent(eventType, x, y, Rtt_FloatToReal(0), Rtt_FloatToReal(0), 1, false, false, false, false, false, false, false);
		TapEvent tapEvent(x, y, 1);

		runtime->DispatchEvent(mouseEvent);
		runtime->DispatchEvent(tapEvent);
		context->GetMouseListener()->TouchUp(x, y, 0);
	}

	void LinuxMouseListener::OnMouseRightDoubleClick(wxMouseEvent &event)
	{
		SolarAppContext *context = wxGetApp().GetFrame()->fContext;
		Runtime *runtime = context->GetRuntime();
		const bool isSecondaryDown = true;
		MouseEvent::MouseEventType eventType = MouseEvent::kDown;
		MouseEvent mouseEvent(eventType, event.GetX(), event.GetY(), Rtt_FloatToReal(0), Rtt_FloatToReal(0), 2, false, isSecondaryDown, false, false, false, false, false);

		runtime->DispatchEvent(mouseEvent);
	}

	void LinuxMouseListener::OnMouseRightDown(wxMouseEvent &event)
	{
		SolarAppContext *context = wxGetApp().GetFrame()->fContext;
		Runtime *runtime = context->GetRuntime();
		const int x = event.GetX();
		const int y = event.GetY();
		const bool isSecondaryDown = true;
		MouseEvent::MouseEventType eventType = MouseEvent::kDown;
		MouseEvent mouseEvent(eventType, x, y, Rtt_FloatToReal(0), Rtt_FloatToReal(0), 0, false, isSecondaryDown, false, false, false, false, false);

		runtime->DispatchEvent(mouseEvent);
		context->GetMouseListener()->TouchDown(x, y, 0);
	}

	void LinuxMouseListener::OnMouseRightUp(wxMouseEvent &event)
	{
		SolarAppContext *context = wxGetApp().GetFrame()->fContext;
		Runtime *runtime = context->GetRuntime();
		const int x = event.GetX();
		const int y = event.GetY();
		MouseEvent::MouseEventType eventType = MouseEvent::kUp;
		MouseEvent mouseEvent(eventType, x, y, Rtt_FloatToReal(0), Rtt_FloatToReal(0), 1, false, false, false, false, false, false, false);

		runtime->DispatchEvent(mouseEvent);
		context->GetMouseListener()->TouchUp(x, y, 0);
	}

	void LinuxMouseListener::OnMouseMiddleDoubleClick(wxMouseEvent &event)
	{
		SolarAppContext *context = wxGetApp().GetFrame()->fContext;
		Runtime *runtime = context->GetRuntime();
		const bool isMiddleDown = true;
		MouseEvent::MouseEventType eventType = MouseEvent::kUp;
		MouseEvent mouseEvent(eventType, event.GetX(),  event.GetY(), Rtt_FloatToReal(0), Rtt_FloatToReal(0), 2, false, false, isMiddleDown, false, false, false, false);

		runtime->DispatchEvent(mouseEvent);
	}

	void LinuxMouseListener::OnMouseMiddleDown(wxMouseEvent &event)
	{
		SolarAppContext *context = wxGetApp().GetFrame()->fContext;
		Runtime *runtime = context->GetRuntime();
		const bool isMiddleDown = true;
		MouseEvent::MouseEventType eventType = MouseEvent::kDown;
		MouseEvent mouseEvent(eventType, event.GetX(), event.GetY(), Rtt_FloatToReal(0), Rtt_FloatToReal(0), 0, false, false, isMiddleDown, false, false, false, false);

		runtime->DispatchEvent(mouseEvent);
	}

	void LinuxMouseListener::OnMouseMiddleUp(wxMouseEvent &event)
	{
		SolarAppContext *context = wxGetApp().GetFrame()->fContext;
		Runtime *runtime = context->GetRuntime();
		MouseEvent::MouseEventType eventType = MouseEvent::kUp;
		MouseEvent mouseEvent(eventType,  event.GetX(), event.GetY(), Rtt_FloatToReal(0), Rtt_FloatToReal(0), 1, false, false, false, false, false, false, false);

		runtime->DispatchEvent(mouseEvent);
	}

	void LinuxMouseListener::OnMouseMove(wxMouseEvent &event)
	{
		SolarAppContext *context = wxGetApp().GetFrame()->fContext;
		Runtime *runtime = context->GetRuntime();
		const int x = event.GetX();
		const int y = event.GetY();
		const bool isPrimaryDown = event.LeftIsDown();
		const bool isSecondaryDown = event.RightIsDown();
		const bool isMiddleDown = event.MiddleIsDown();
		const bool isDragEvent = (isPrimaryDown || isSecondaryDown || isMiddleDown);
		const MouseEvent::MouseEventType eventType = isDragEvent ? MouseEvent::kDrag : MouseEvent::kMove;
		MouseEvent mouseEvent(eventType, x, y, Rtt_FloatToReal(0), Rtt_FloatToReal(0), 0, isPrimaryDown, isSecondaryDown, isMiddleDown, false, false, false, false);

		runtime->DispatchEvent(mouseEvent);
		context->GetMouseListener()->TouchMoved(x, y, 0);
	}

	void LinuxMouseListener::OnMouseWheel(wxMouseEvent &event)
	{
		SolarAppContext *context = wxGetApp().GetFrame()->fContext;
		Runtime *runtime = context->GetRuntime();
		const float wheelRotation = event.GetWheelRotation();
		const bool scrolledVertically = event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL;
		const float scrollWheelDeltaX = scrolledVertically ? 0 : wheelRotation * -1;
		const float scrollWheelDeltaY = scrolledVertically ? wheelRotation * -1 : 0;
		MouseEvent mouseEvent(MouseEvent::kScroll, event.GetX(), event.GetY(), Rtt_FloatToReal(scrollWheelDeltaX), Rtt_FloatToReal(scrollWheelDeltaY), 0,
		                      false, false, false, false, false, false, false);

		runtime->DispatchEvent(mouseEvent);
	}
};
