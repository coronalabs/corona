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

#ifndef __Rtt_ImageSheet__
#define __Rtt_ImageSheet__

#include "Core/Rtt_Array.h"
#include "Core/Rtt_Geometry.h"
#include "Core/Rtt_SharedPtr.h"
#include "Renderer/Rtt_RenderTypes.h"
#include "Rtt_ImageFrame.h"
#include "Rtt_TextureResource.h"

// ----------------------------------------------------------------------------

extern "C"
{
	struct lua_State;
}

namespace Rtt
{

class ImageSheetUserdata;
class PlatformBitmap;

// ----------------------------------------------------------------------------

class ImageSheet
{
	public:
		typedef ImageSheet Self;

	public:
		static const char kMetatableName[];

	public:
		static int CreateAndPush(
			lua_State *L,
			Rtt_Allocator *allocator );

		static ImageSheetUserdata* ToUserdata( lua_State *L, int index );

	// Metatable methods
	protected:
		static int Finalizer( lua_State *L );

	public:
		static void Initialize( lua_State *L );

	private:
		// Prevent subclass and
		// Only CreateAndPush can create
		ImageSheet(
			Rtt_Allocator *allocator,
			const SharedPtr< TextureResource >& texture );

	public:
		~ImageSheet();

	protected:
		int Initialize( lua_State *L, int optionsIndex );

	public:
		const SharedPtr< TextureResource >& GetTextureResource() const { return fResource; }
		const ImageFrame* GetFrame( int index ) const { return fFrames[index]; }
		S32 GetNumFrames() const { return fFrames.Length(); }
		bool IsEmpty() const { return fFrames.Length() <= 0; }

	private:
		SharedPtr< TextureResource > fResource;
		PtrArray< ImageFrame > fFrames;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_ImageSheet__
