//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
		bool CorrectsTrimOffsets() const { return fCorrectTrimOffsets; }

	private:
		SharedPtr< TextureResource > fResource;
		PtrArray< ImageFrame > fFrames;
		bool fCorrectTrimOffsets;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // __Rtt_ImageSheet__
