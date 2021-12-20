//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_LinuxDisplayObject.h"
#include "Rtt_Lua.h"
#include "Rtt_Runtime.h"
#include "Rtt_RenderingStream.h"
#include "Rtt_LinuxContext.h"
#include "wx/textctrl.h"
#include "wx/colour.h"

namespace Rtt
{
	LinuxDisplayObject::LinuxDisplayObject(const Rect &bounds, const char *elementType)
		: PlatformDisplayObject()
		, fSelfBounds(bounds)
		, fWindow(NULL)
	{
		// Note: Setting the reference point to center is not done in any of the other implementations because
		// fSelfBounds is already centered/converted unlike this implementation.
		// This solves the problem, but will possibly be a problem if/when we support object resizing.
		// Either this code should be converted to center fSelfBounds initially or the
		// subclass virtual function will need to account for the offset.

		Real w = bounds.Width();
		Real h = bounds.Height();
		Real halfW = Rtt_RealDiv2(w);
		Real halfH = Rtt_RealDiv2(h);

		// The UIView's self transform is relative to it's center, but the DisplayObject's
		// transform included that translation, so we need to factor this out during Build()
		// NOTE: The incoming bounds are in content coordinates, not native UIView coordinates,
		// so we must record these separately instead of relying on the values of [fView center]
		float fViewCenterX = bounds.xMin + halfW;
		float fViewCenterY = bounds.yMin + halfH;

		// If running in the Corona simulator, then fetch its current zoom level scale.
		//CacheSimulatorScale();

		// Update DisplayObject so that it corresponds to the actual position of the UIView
		// where DisplayObject's self bounds will be centered around its local origin.
		Translate(fViewCenterX, fViewCenterY);

		// The self bounds needs to be centered around DisplayObject's local origin
		// even though UIView's bounds will not be.
		fSelfBounds.MoveCenterToOrigin();
	}

	LinuxDisplayObject::~LinuxDisplayObject()
	{
		if (wxTheApp)
		{
			delete fWindow;
		}
	}

	void LinuxDisplayObject::SetFocus()
	{
	}

	void LinuxDisplayObject::DidMoveOffscreen(StageObject *oldStage)
	{
	}

	void LinuxDisplayObject::WillMoveOnscreen(StageObject *newStage)
	{
	}

	bool LinuxDisplayObject::CanCull() const
	{
		// Disable culling for all native UI objects.
		// Note: This is needed so that the Build() function will get called when a native object
		// is being moved partially or completely offscreen.
		return false;
	}

	void LinuxDisplayObject::Draw(Renderer &renderer) const
	{
	}

	void LinuxDisplayObject::GetSelfBounds(Rect &rect) const
	{
		rect = fSelfBounds;
	}

	bool LinuxDisplayObject::HasBackground() const
	{
		return true;
	}

	void LinuxDisplayObject::SetBackgroundVisible(bool isVisible)
	{
	}

	int LinuxDisplayObject::ValueForKey(lua_State *L, const char key[]) const
	{
		Rtt_ASSERT(key);

		if (fWindow == NULL)
		{
			return 0;
		}

		int result = 1;

		if (strcmp("isVisible", key) == 0)
		{
			bool visible = fWindow->IsShown();
			lua_pushboolean(L, visible);
		}
		else if (strcmp("alpha", key) == 0)
		{
		}
		else if (strcmp("hasBackground", key) == 0)
		{
			lua_pushboolean(L, HasBackground() ? 1 : 0);
		}
		else
		{
			result = 0;
		}

		return result;
	}

	void LinuxDisplayObject::showControls(bool val)
	{
		if (fWindow)
		{
			fWindow->Show(val);
		}
	}

	bool LinuxDisplayObject::SetValueForKey(lua_State *L, const char key[], int valueIndex)
	{
		Rtt_ASSERT(key);

		if (fWindow == NULL)
		{
			return false;
		}

		bool result = true;

		if (strcmp("isVisible", key) == 0)
		{
			bool visible = lua_toboolean(L, valueIndex) ? true : false;
			fWindow->Show(visible);
		}
		else if (strcmp("alpha", key) == 0)
		{
		}
		else if (strcmp("hasBackground", key) == 0)
		{
		}
		else
		{
			result = false;
		}

		return result;
	}

	void LinuxDisplayObject::setBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
	{
		if (fWindow)
		{
			fWindow->SetBackgroundColour(wxColour(r, g, b, a));
		}
	}

	bool LinuxDisplayObject::Initialize()
	{
		return true;
	}

	void LinuxDisplayObject::Prepare(const Display &display)
	{
		Super::Prepare(display);

		if (ShouldPrepare() && fWindow)
		{
			Rect outBounds = StageBounds();
			fWindow->Move(outBounds.xMin, outBounds.yMin);
			fWindow->SetSize(outBounds.Width(), outBounds.Height());
		}
	}
} // namespace Rtt
