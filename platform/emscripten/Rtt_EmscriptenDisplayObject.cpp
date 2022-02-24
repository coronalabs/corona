//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#if defined(EMSCRIPTEN)
extern "C"
{
	// JS API
	extern void jsDisplayObjectDelete(int id);
	extern bool jsDisplayObjectGetVisible(int id);
	extern void	jsDisplayObjectShowControls(int id, bool val);
	extern void jsDisplayObjectSetVisible(int id, bool val);
	extern void jsDisplayObjectHasBackground(int id, bool val);
	extern void jsDisplayObjectsetBackgroundColor(int id, const char*);
	extern int jsDisplayObjectCreate(float x, float y, float w, float h, const char* type, void* thiz);
	extern void jsDisplayObjectSetBounds(int id, float x, float y, float w, float h);
}
#else
	void jsDisplayObjectDelete(int id) {};
	bool jsDisplayObjectGetVisible(int id) { return 0; };
	void	jsDisplayObjectShowControls(int id, bool val) {}
	void jsDisplayObjectSetVisible(int id, bool val) {}
	void jsDisplayObjectHasBackground(int id, bool val) {}
	void jsDisplayObjectsetBackgroundColor(int id, const char*) {}
	int jsDisplayObjectCreate(float x, float y, float w, float h, const char* type, void* thiz) { return 0; };
	void jsDisplayObjectSetBounds(int id, float x, float y, float w, float h) {}
#endif

#include "Core/Rtt_Build.h"
#include "Rtt_EmscriptenDisplayObject.h"
#include "Rtt_Lua.h"
#include "Rtt_Runtime.h"
#include "Rtt_RenderingStream.h"

namespace Rtt
{

	EmscriptenDisplayObject::EmscriptenDisplayObject(const Rect& bounds, const char* elementType)
		: PlatformDisplayObject()
		, fSelfBounds(bounds)
		, fElementID(0)
		, fType(elementType)
	{
		// Note: Setting the reference point to center is not done in any of the other implementations because
		// fSelfBounds is already centered/converted unlike this implementation.
		// This solves the problem, but will possibly be a problem if/when we support object resizing.
		// Either this code should be converted to center fSelfBounds initially or the 
		// subclass virtual function will need to account for the offset.
	
		Real w = bounds.Width();
		Real h = bounds.Height();
		Real halfW = Rtt_RealDiv2( w );
		Real halfH = Rtt_RealDiv2( h );

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
		Translate( fViewCenterX, fViewCenterY );

		// The self bounds needs to be centered around DisplayObject's local origin
		// even though UIView's bounds will not be.
		fSelfBounds.MoveCenterToOrigin();
	}

	EmscriptenDisplayObject::~EmscriptenDisplayObject()
	{
		jsDisplayObjectDelete(fElementID);
	}

	void EmscriptenDisplayObject::SetFocus()
	{
	}

	void EmscriptenDisplayObject::DidMoveOffscreen(StageObject *oldStage)
	{
	}

	void EmscriptenDisplayObject::WillMoveOnscreen(StageObject *newStage)
	{
	}

	bool EmscriptenDisplayObject::CanCull() const
	{
		// Disable culling for all native UI objects.
		// Note: This is needed so that the Build() function will get called when a native object
		//       is being moved partially or completely offscreen.
		return false;
	}

	void EmscriptenDisplayObject::Draw(Renderer& renderer) const
	{
	}

	void EmscriptenDisplayObject::GetSelfBounds(Rect& rect) const
	{
		rect = fSelfBounds;
	}

	bool EmscriptenDisplayObject::HasBackground() const
	{
		return true;
	}

	void EmscriptenDisplayObject::SetBackgroundVisible(bool isVisible)
	{
	}

	int EmscriptenDisplayObject::ValueForKey(lua_State *L, const char key[]) const
	{
		Rtt_ASSERT(key);

		int result = 1;

		if (strcmp("isVisible", key) == 0)
		{
			bool visible = jsDisplayObjectGetVisible(fElementID); 
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

	void EmscriptenDisplayObject::showControls(bool val)
	{
		jsDisplayObjectShowControls(fElementID, val);
	}

	bool EmscriptenDisplayObject::SetValueForKey(lua_State *L, const char key[], int valueIndex)
	{
		Rtt_ASSERT(key);

		bool result = true;

		if (strcmp("isVisible", key) == 0)
		{
			bool visible = lua_toboolean(L, valueIndex) ? true : false;
			jsDisplayObjectSetVisible(fElementID, visible);
		}
		else if (strcmp("alpha", key) == 0)
		{
		}
		else if (strcmp("hasBackground", key) == 0)
		{
			bool visible = lua_toboolean(L, valueIndex) ? true : false;
			jsDisplayObjectHasBackground(fElementID, visible);
		}
		else
		{
			result = false;
		}

		return result;
	}

	void EmscriptenDisplayObject::setBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
	{
		char rgba[16];
		snprintf(rgba, sizeof(rgba), "#%02X%02X%02X", r, g, b, 255);
		jsDisplayObjectsetBackgroundColor(fElementID, rgba);
	}

	bool EmscriptenDisplayObject::Initialize()
	{
		// suppoted element ?
		if (fType.size() > 0)
		{
			Rect outBounds = StageBounds();
			fElementID = jsDisplayObjectCreate(outBounds.xMin, outBounds.yMin, outBounds.Width(), outBounds.Height(), fType.c_str(), this);
			return true;
		}
		return false;
	}

	void EmscriptenDisplayObject::Prepare(const Display& display)
	{
		Super::Prepare(display);
		if (ShouldPrepare())
		{
			Rect outBounds = StageBounds();
			jsDisplayObjectSetBounds(fElementID, outBounds.xMin, outBounds.yMin, outBounds.Width(), outBounds.Height());
		}
	}

} // namespace Rtt
