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

#ifndef _Rtt_FrameBufferObject_H__
#define _Rtt_FrameBufferObject_H__

#include "Renderer/Rtt_CPUResource.h"
#include "Core/Rtt_Types.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Display;
class Texture;

// ----------------------------------------------------------------------------

class FrameBufferObject : public CPUResource
{
	public:
		typedef CPUResource Super;
		typedef FrameBufferObject Self;

	public:
		FrameBufferObject( Rtt_Allocator* allocator, Texture* texture );
		virtual ResourceType GetType() const;
		Texture* GetTexture() const;

		U8 GetDepthBits() const { return fDepthBits; }
		U8 GetStencilBits() const { return fStencilBits; }

		void AddAnyAttachments( Display& display );

	private:
		Texture* fTexture;
		U8 fDepthBits;
		U8 fStencilBits;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_FrameBufferObject_H__
