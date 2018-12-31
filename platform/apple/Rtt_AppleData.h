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

#ifndef _Rtt_AppleData_H__
#define _Rtt_AppleData_H__

#include "Rtt_PlatformData.h"

// ----------------------------------------------------------------------------

@class NSData;
#ifdef Rtt_IPHONE_ENV
	@class UIImage;
#endif

namespace Rtt
{

// ----------------------------------------------------------------------------

class AppleData : public PlatformData
{
	public:
		AppleData( NSData* data );
		virtual ~AppleData();

	public:
		virtual const char* Read( size_t numBytes, size_t& numBytesRead ) const;
		virtual void Seek( SeekOrigin origin, S32 offset ) const;
		virtual size_t Length() const;

	private:
		NSData* fData;
		mutable size_t fOffset;
};

/*
class MacBitmapData : public AppleData
{
	public:
};

class AppleBitmapData : public AppleData
{
	public:
		#ifdef Rtt_IPHONE_ENV
			typedef UIImage* AppleImageRef;
		#else
			typedef struct CGImage* AppleImageRef;
		#endif

	public:
		AppleImageData( AppleImageRef image );
		virtual ~AppleImageData();

	private:
		AppleImageRef fImage;
};
*/

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AppleData_H__
