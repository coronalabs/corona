//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ConsoleAuthorizationDelegate_H__
#define _Rtt_ConsoleAuthorizationDelegate_H__

#include "Rtt_MAuthorizationDelegate.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Authorization;
class AuthorizationTicket;

// ----------------------------------------------------------------------------

class ConsoleAuthorizationDelegate : public MAuthorizationDelegate
{
	public:
		ConsoleAuthorizationDelegate();
		virtual ~ConsoleAuthorizationDelegate();

	public:
		void SetLoginCredentials( const char *usr, const char *pwd ) { fUsr = usr; fPwd = pwd; }

	public:
		bool IsPreviouslyAuthorized() const { return fIsPreviouslyAuthorized; }
		bool HasLoginCredentials() const { return fUsr && fPwd; }

	public:
		bool Login( Authorization *authorizer ) const;
		int Authorize( Authorization *authorizer, const char *user, const char *encryptedPassword ) const;

	public:
		virtual bool ShowLicense( const Authorization& sender );
		virtual bool ShowTrial( const Authorization& sender, const AuthorizationTicket& ticket );
		virtual bool ShowDeviceBuildUpsell( const Authorization& sender, const AuthorizationTicket& ticket, TargetDevice::Platform platform );
		virtual bool ShowTrialUpsell( const Authorization& sender, const AuthorizationTicket& ticket, const char *message );
		virtual void ShowRenewal( const Authorization& sender, const AuthorizationTicket& ticket );
		virtual bool TicketNotInstalled( const Authorization& sender );
		virtual const char* PasswordInvalid( const Authorization& sender, const char *usr );
		virtual bool AuthorizationDidFail( const Authorization& sender ) const;
		virtual bool VerificationDidFail( const Authorization& sender ) const;

	private:
		const char *fUsr;
		const char *fPwd;
		bool fIsPreviouslyAuthorized;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

#endif // _Rtt_ConsoleAuthorizationDelegate_H__
