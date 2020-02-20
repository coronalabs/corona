//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidDisplayObject_H__
#define _Rtt_AndroidDisplayObject_H__

#include "Rtt_PlatformDisplayObject.h"

class NativeToJavaBridge;
class AndroidDisplayObjectRegistry;


// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AndroidDisplayObject : public PlatformDisplayObject
{
	public:
		AndroidDisplayObject( const Rect& bounds, AndroidDisplayObjectRegistry *displayObjectRegistry, NativeToJavaBridge *ntjb );
		virtual ~AndroidDisplayObject();

		void InitializeView( void * view );
		void SetFocus();
		int GetId() const { return fId; }
		bool HasBackground() const;
		void SetBackgroundVisible(bool isVisible);

	public:
		// DisplayObject
		virtual void DidMoveOffscreen();
		virtual void WillMoveOnscreen();
		virtual bool CanCull() const;

	public:
		// MDrawable
		virtual void Prepare( const Display& display );
		virtual void Draw( Renderer& renderer ) const;
		virtual void GetSelfBounds( Rect& rect ) const;
		virtual void SetSelfBounds( Real width, Real height );

	public:
		// MLuaTableBridge
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );

	protected:
		void * GetView() const { return fView; }
		AndroidDisplayObjectRegistry * GetDisplayObjectRegistry() { return fDisplayObjectRegistry; }

	private:
		Rect fSelfBounds;
		void * fView;
		AndroidDisplayObjectRegistry *fDisplayObjectRegistry;
		int fId;

	protected:
		NativeToJavaBridge *fNativeToJavaBridge;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidDisplayObject_H__
