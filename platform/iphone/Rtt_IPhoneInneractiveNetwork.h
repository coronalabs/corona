//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneInneractiveNetwork_H__
#define _Rtt_IPhoneInneractiveNetwork_H__

#include "Rtt_PlatformAdNetwork.h"

// ----------------------------------------------------------------------------

@class NSString;
@class UIView;
namespace Rtt
{

// ----------------------------------------------------------------------------

class IPhoneInneractiveNetwork : public PlatformAdNetwork
{
	public:
		typedef IPhoneInneractiveNetwork Self;
		typedef PlatformAdNetwork Super;

	public:
		IPhoneInneractiveNetwork();
		virtual ~IPhoneInneractiveNetwork();

	public:
		virtual bool Init( const char *appId, LuaResource *listener );
		virtual bool Show( const char *adUnitType, lua_State *L, int index );
		virtual void Hide();
		virtual const char* GetTestAppId() const;

	protected:
		id fDelegate;
		UIView *fAd;
		NSString *fAppId;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneInneractiveNetwork_H__
