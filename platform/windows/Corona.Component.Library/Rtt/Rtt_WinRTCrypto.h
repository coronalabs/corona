// ----------------------------------------------------------------------------
// 
// Rtt_WinRTCrypto.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Rtt_MCrypto.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

class WinRTCrypto : public MCrypto
{
	public:
		virtual size_t GetDigestLength(Algorithm algorithm) const;
		virtual void CalculateDigest(Algorithm algorithm, const Rtt::Data<const char> & data, U8 *md) const;
		virtual void CalculateHMAC(Algorithm algorithm, const Rtt::Data<const char> & key, const Rtt::Data<const char> & data, U8 *outMac) const;
};

} // namespace Rtt
