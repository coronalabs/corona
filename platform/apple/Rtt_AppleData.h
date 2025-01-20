//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
