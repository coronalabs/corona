//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneNativeObject_H__
#define _Rtt_IPhoneNativeObject_H__

#include "Rtt_IPhoneDisplayObject.h"

// ----------------------------------------------------------------------------

@protocol CoronaNativeObjectAdapter;

namespace Rtt
{

// ----------------------------------------------------------------------------

class IPhoneNativeObject : public IPhoneDisplayObject
{
	public:
		typedef IPhoneDisplayObject Super;

	public:
		IPhoneNativeObject( const Rect& bounds, id<CoronaNativeObjectAdapter> adapter );
		virtual ~IPhoneNativeObject();

	public:
		// PlatformDisplayObject
		virtual bool Initialize();

	public:
		// MLuaTableBridge
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex );

	protected:
		virtual id GetNativeTarget() const;
	
	private:
		id<CoronaNativeObjectAdapter> fAdapter;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneNativeObject_H__
