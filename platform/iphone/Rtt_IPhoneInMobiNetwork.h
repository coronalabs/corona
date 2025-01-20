//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneInMobiNetwork_H__
#define _Rtt_IPhoneInMobiNetwork_H__

#include "Rtt_PlatformAdNetwork.h"

// ----------------------------------------------------------------------------

@class NSString;
@class IMAdView;
@protocol IMAdDelegate;

namespace Rtt
{

// ----------------------------------------------------------------------------

class IPhoneInMobiNetwork : public PlatformAdNetwork
{
	public:
		typedef IPhoneInMobiNetwork Self;
		typedef PlatformAdNetwork Super;

	public:
		IPhoneInMobiNetwork();
		virtual ~IPhoneInMobiNetwork();

	public:
		virtual bool Init( const char *appId, LuaResource *listener );
		virtual bool Show( const char *adUnitType, lua_State *L, int index );
		virtual void Hide();
		virtual const char* GetTestAppId() const;

	protected:
		id< IMAdDelegate > fDelegate;
		IMAdView *fAd;
		NSString *fAppId;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneInMobiNetwork_H__
