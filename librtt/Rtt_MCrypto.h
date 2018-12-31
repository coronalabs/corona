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

#ifndef _Rtt_MCrypto_H__
#define _Rtt_MCrypto_H__

#include "Core/Rtt_Data.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class MCrypto
{
	public:
		typedef enum _Algorithm
		{
			kMD4Algorithm = 0, // HMAC not available for MD4
			kMD5Algorithm,
			kSHA1Algorithm,
			kSHA224Algorithm,
			kSHA256Algorithm,
			kSHA384Algorithm,
			kSHA512Algorithm,

			kNumAlgorithms
		}
		Algorithm;

		enum
		{
			kMaxDigestSize = 64  // longest known SHA512
		};

	public:
		virtual size_t GetDigestLength( Algorithm algorithm ) const = 0;
		virtual void CalculateDigest( Algorithm algorithm, const Rtt::Data<const char> & data, U8 *md ) const = 0;
		virtual void CalculateHMAC( Algorithm algorithm, const Rtt::Data<const char> & key, const Rtt::Data<const char> & data, U8 *outMac ) const = 0;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MCrypto_H__
