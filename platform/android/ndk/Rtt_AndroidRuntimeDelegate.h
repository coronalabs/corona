//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidRuntimeDelegate_H__
#define _Rtt_AndroidRuntimeDelegate_H__

#include "Rtt_RuntimeDelegatePlayer.h"

class NativeToJavaBridge;

namespace Rtt
{

/// Delegate used to receive events from the Corona Runtime.
/// This is done by assigning an instance of this class to the Runtime.SetDelegate() function.
class AndroidRuntimeDelegate : public RuntimeDelegatePlayer
{
	public:
		AndroidRuntimeDelegate(NativeToJavaBridge *ntjb, bool isCoronaKit);
		virtual ~AndroidRuntimeDelegate();
		
		virtual void DidInitLuaLibraries( const Runtime& sender ) const;
		virtual bool HasDependencies( const Runtime& sender ) const;
		virtual void WillLoadMain(const Runtime& sender) const;
		virtual void DidLoadMain(const Runtime& sender) const;
		virtual void WillLoadConfig(const Runtime& sender, lua_State *L) const;
		virtual void DidLoadConfig(const Runtime& sender, lua_State *L) const;
		virtual void InitializeConfig(const Runtime& sender, lua_State *L) const;

	private:
		NativeToJavaBridge *fNativeToJavaBridge;
		bool fIsCoronaKit;
};

} // namespace Rtt

#endif // _Rtt_AndroidRuntimeDelegate_H__
