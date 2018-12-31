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

#ifndef _Rtt_Texture_H__
#define _Rtt_Texture_H__

#include "Renderer/Rtt_CPUResource.h"
#include "Core/Rtt_Types.h"
#include <stdlib.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class Texture : public CPUResource
{
	public:
		typedef CPUResource Super;
		typedef Texture Self;

		typedef enum _Format
		{
			kAlpha,
			kLuminance,
			kRGB,
			kRGBA,
			kBGRA,
			kABGR,
			kARGB,
			kNumFormats
		}
		Format;

		typedef enum _Filter
		{
			kNearest,
			kLinear,
			kNumFilters
		}
		Filter;

		typedef enum _Wrap
		{
			kClampToEdge,
			kRepeat,
			kMirroredRepeat,

			kNumWraps
		}
		Wrap;

		typedef enum _Unit
		{
			kFill0,
			kFill1,
			kMask0,
			kMask1,
			kMask2,
			kNumUnits
		}
		Unit;

	public:

		Texture( Rtt_Allocator* allocator );
		virtual ~Texture();

		virtual ResourceType GetType() const;
		virtual void Allocate();
		virtual void Deallocate();

		virtual U32 GetWidth() const = 0;
		virtual U32 GetHeight() const = 0;
		virtual Format GetFormat() const = 0;
		virtual Filter GetFilter() const = 0;
		virtual Wrap GetWrapX() const;
		virtual Wrap GetWrapY() const;
		virtual size_t GetSizeInBytes() const;
		virtual U8 GetByteAlignment() const;

		virtual const U8* GetData() const;
		virtual void ReleaseData();

		virtual void SetFilter( Filter newValue );
		virtual void SetWrapX( Wrap newValue );
		virtual void SetWrapY( Wrap newValue );
	
	public:
		void SetRetina( bool newValue ){ fIsRetina = newValue; }
		bool IsRetina(){ return fIsRetina; }

	private:
		bool fIsRetina;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Texture_H__
