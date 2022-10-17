//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ImageSheet.h"

#include "Display/Rtt_BitmapPaint.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_ImageFrame.h"
#include "Display/Rtt_ImageSheetUserdata.h"
#include "Display/Rtt_TextureFactory.h"
#include "Display/Rtt_TextureResource.h"
#include "Renderer/Rtt_Texture.h"
#include "Display/Rtt_DisplayDefaults.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_MPlatform.h"
#include "Rtt_Runtime.h"
#include "Core/Rtt_Math.h"

#ifdef Rtt_SPRITE
	#include "Display/Rtt_SpriteSourceFrame.h"
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

const char ImageSheet::kMetatableName[] = "ImageSheet"; // unique identifier for this userdata type

// graphics.newImageSheet( filename, [baseDir, ] options )
int
ImageSheet::CreateAndPush(
	lua_State *L,
	Rtt_Allocator *allocator )
{
	int result = 0;

	int nextArg = 1;

	// Required 1st param is "filename"
	// filename [, baseDirectory]
	MPlatform::Directory baseDir = MPlatform::kResourceDir;
	const char *imageName = LuaLibSystem::GetFilename( L, nextArg, baseDir );

	if ( imageName )
	{
		if ( lua_istable( L, nextArg ) )
		{
			Runtime *runtime = LuaContext::GetRuntime( L );

			// Image sheets should be loaded at full resolution
			// and support dynamic image resolution
			U32 flags = PlatformBitmap::kIsNearestAvailablePixelDensity | PlatformBitmap::kIsBitsFullResolution;

			TextureFactory& factory = runtime->GetDisplay().GetTextureFactory();
			SharedPtr< TextureResource > texture =
				factory.FindOrCreate( imageName, baseDir, flags, false );

///			TextureResource *texture = BitmapPaint::CreateOrFindTexture( * runtime, imageName, baseDir, flags );

			if ( Rtt_VERIFY( texture.NotNull() ) )
			{
				ImageSheet *sheet = Rtt_NEW( allocator, ImageSheet( allocator, texture ) );
				sheet->Initialize( L, nextArg );

				AutoPtr< ImageSheet > pSheet( allocator, sheet );
				ImageSheetUserdata *ud = Rtt_NEW( allocator, ImageSheetUserdata( pSheet ) );

				if ( sheet->IsEmpty() )
				{
					Rtt_TRACE_SIM( ( "ERROR: graphics.newImageSheet() created sheet with no frames defined.\n" ) );
				}

				if ( Rtt_VERIFY( ud ) )
				{
					Lua::PushUserdata( L, ud, Self::kMetatableName );
					result = 1;
				}
			}
		}
		else
		{
			luaL_argerror( L, nextArg, "table (options) expected" );
		}
	}
	else
	{
		luaL_argerror( L, nextArg, "string (filename) expected" );
	}

	return result;
}

ImageSheetUserdata*
ImageSheet::ToUserdata( lua_State *L, int index )
{
	ImageSheetUserdata *result =
		(ImageSheetUserdata*)Lua::CheckUserdata( L, index, Self::kMetatableName );

	return result;
}

int
ImageSheet::Finalizer( lua_State *L )
{
	ImageSheetUserdata **ud = (ImageSheetUserdata **)luaL_checkudata( L, 1, Self::kMetatableName );
	if ( ud )
	{
		Rtt_DELETE( *ud );
	}

	return 0;
}

// Call this to init metatable
void
ImageSheet::Initialize( lua_State *L )
{
	Rtt_LUA_STACK_GUARD( L );

	const luaL_Reg kVTable[] =
	{
		{ "__gc", Self::Finalizer },
		{ NULL, NULL }
	};
		
	Lua::InitializeMetatable( L, Self::kMetatableName, kVTable );
}

ImageSheet::ImageSheet(
	Rtt_Allocator *allocator, const SharedPtr< TextureResource >& resource )
:	fResource( resource ),
	fFrames( allocator ),
	fCorrectTrimOffsets( false )
{
}

ImageSheet::~ImageSheet()
{
}

static void
GetTrimData( lua_State *L, int index, S32& rSrcX, S32& rSrcY, S32& rSrcW, S32& rSrcH )
{
	Rtt_ASSERT( index > 0 ); // need stable index

	lua_getfield( L, index, "sourceX" );
	rSrcX = (S32) lua_tointeger( L, -1 );

	lua_getfield( L, index, "sourceY" );
	rSrcY = (S32) lua_tointeger( L, -1 );

	lua_getfield( L, index, "sourceWidth" );
	rSrcW = (S32) lua_tointeger( L, -1 );

	lua_getfield( L, index, "sourceHeight" );
	rSrcH = (S32) lua_tointeger( L, -1 );

	lua_pop( L, 4 );
}

static bool
IsTrimmed( S32 srcW, S32 srcH, S32 contentW, S32 contentH )
{
	bool isTrimmed = ( srcW > contentW ) || ( srcH > contentH );

	return isTrimmed;
}

/*

Processing the 'options' table
===============================================================================

-- simple case (single frame size)
-- 
-- this case is detected via the presence of 'numFrames' key 
local options = 
{
	-- Required params
	width = 50,
	height = 50,
	numFrames = 3,

	-- Optional params
	-- 
	-- If the original image undergoes texture packing, then that means
	-- the image is cropped before placing it in the texture to remove
	-- transparent borders. In order to reconstruct the image, additional
	-- information about the crop rectangle must be provided.
	-- 
	-- The cropping rectangle has an origin of (originX,originY) relative
	-- to the origin of the original uncropped image. In this case,
	-- the dimensions of this crop rect are 'width' and 'height' from 
	-- above.
	--  
	-- If srcWidth/srcHeight are not provided, then they are
	-- assumed to be 'width' and 'height' from above. Similarly,
	-- originX/originY are both assumed to be 0. 
	sourceWidth = 128,  -- width of original uncropped image
	sourceHeight = 128, -- height of original uncropped image
	sourceX = 38, -- the x-origin of the crop rect relative to the uncropped image
	sourceY = 28, -- the y-origin of the crop rect relative to the uncropped image
}

-- complex case (multiple frame sizes)
local options = 
{
	-- array of frames
	{
		-- Required params
		x = 2,        -- frame's x-location in texture
		y = 70,       -- frame's y-location in texture 
		width = 50,   -- frame's width
		height = 50,  -- frame's height

		-- Optional params
		sourceWidth = 128,  -- width of original uncropped image
		sourceHeight = 128, -- height of original uncropped image
		sourceX = 38, -- the x-origin of the crop rect relative to the uncropped image
		sourceY = 28, -- the y-origin of the crop rect relative to the uncropped image
	},
	{
		x = 2,
		y = 242,
		width = 50,
		height = 52,
		sourceWidth = 128,
		sourceHeight = 128,
		sourceX = 38, 
		sourceY = 36,
	}
}

-- complex case with backward compatibility to 'coordinateData' argument
-- to sprite.newSpriteSheetFromData() function
-- 
-- this case is detected via the presence of 'frames' key 
local options = 
{
	frames = 
	{
		{
			textureRect = { x = 2, y = 70, width = 50, height = 50 },
			spriteTrimmed = true,
			spriteColorRect = { x = 38, y = 28 },
			spriteSourceSize = { width = 128, height = 128 }
		},
		{
			textureRect = { x = 2, y = 242, width = 50, height = 52 },
			spriteTrimmed = true,
			spriteColorRect = { x = 38, y = 36 },
			spriteSourceSize = { width = 128, height = 128 }
		},
	}
}

*/
int
ImageSheet::Initialize( lua_State *L, int optionsIndex )
{
#ifdef Rtt_DEBUG
	int top = lua_gettop( L );
#endif
	Rtt_ASSERT( lua_istable( L, optionsIndex ) );
	Rtt_ASSERT( optionsIndex > 0 ); // need stable index

	Rtt_Allocator *allocator = LuaContext::GetAllocator( L ); Rtt_UNUSED( allocator );

	lua_getfield( L, optionsIndex, "numFrames" );
	int numFrames = (int) lua_tointeger( L, -1 );
	lua_pop( L, 1 );

	const Texture& texture = fResource->GetTexture();
	int textureW = texture.GetWidth();
	int textureH = texture.GetHeight();

	// Content scaling
	lua_getfield( L, optionsIndex, "sheetContentWidth" );
	int textureContentW = (int) lua_tointeger( L, -1 );
	lua_pop( L, 1 );

	lua_getfield( L, optionsIndex, "sheetContentHeight" );
	int textureContentH = (int) lua_tointeger( L, -1 );
	lua_pop( L, 1 );

	Real sx = Rtt_REAL_1;
	Real sy = Rtt_REAL_1;

	if ( textureContentW > Rtt_REAL_0 && textureContentH > Rtt_REAL_0 )
	{
		sx = Rtt_RealDiv( Rtt_IntToReal( textureW ), Rtt_IntToReal( textureContentW ) );
		sy = Rtt_RealDiv( Rtt_IntToReal( textureH ), Rtt_IntToReal( textureContentH ) );
	}
	else
	{
		// No content scaling, so default to texture dimensions
		textureContentW = textureW;
		textureContentH = textureH;
	}
	
	fCorrectTrimOffsets = fResource->GetTextureFactory().GetDisplay().GetDefaults().IsImageSheetFrameTrimCorrected();

	bool intrudeHalfTexel = fResource->GetTextureFactory().GetDisplay().GetDefaults().IsImageSheetSampledInsideFrame();

	if ( numFrames > 0 )
	{
		// Simple case
		lua_getfield( L, optionsIndex, "width" );
		int frameW = (int) lua_tointeger( L, -1 );
		lua_pop( L, 1 );

		lua_getfield( L, optionsIndex, "height" );
		int frameH = (int) lua_tointeger( L, -1 );
		lua_pop( L, 1 );

		lua_getfield( L, optionsIndex, "border" );
		int border = Rtt::Max<int>( 0, (int) lua_tointeger( L, -1 ) ); // Ensure border >= 0
		lua_pop( L, 1 );

		// Ensure a single frame has non-zero size and can fit in the texture.
		if ( ( frameW > 0 && frameW <= textureContentW )
			 && ( frameH > 0 && frameH <= textureContentH ) )
		{
			// How far to advance x,y accounting for the border width
			int dx = frameW + 2*border;
			int dy = frameH + 2*border;

			S32 srcX, srcY, srcW, srcH;
			GetTrimData( L, optionsIndex, srcX, srcY, srcW, srcH );
			bool isTrimmed = IsTrimmed( srcW, srcH, frameW, frameH );

			// border overrides trimming
			if ( border > 0 )
			{
				isTrimmed = false;
			}

			S32 x = border;
			S32 y = border;

			// Generate frames
			for ( int i = 0; i < numFrames; i++ )
			{
				// Verify next frame will fit in image (base case checked above)
				if ( ( y + frameH ) > textureContentH )
				{
					// Lua is 1-based so (i+1)th frame.
					luaL_error( L, "Incorrect number of frames (w,h) = (%d,%d) with border (%d) in texture (w,h) = (%d,%d). Failed after frame %d out of %d.", 
						frameW, frameH, border, textureContentW, textureContentH, (i+1), numFrames );
					break;
				}

				ImageFrame *f = Rtt_NEW( allocator, ImageFrame( * this, x, y, frameW, frameH, sx, sy, intrudeHalfTexel ) );
				if ( isTrimmed )
				{
					f->SetTrimData( srcX, srcY, srcW, srcH );
				}
				fFrames.Append( f );

				x += dx;

				// x already accounts for the left border width (b/c we init'd it to 'border')
				// so make sure there's enough room for the frameW and the right border.
				if ( ( x + frameW + border ) > textureContentW )
				{
					x = border; // New row
					y += dy;
				}
			}
		}
		else
		{
			luaL_argerror( L, optionsIndex, "for single frame size, 'options' table must contain valid 'width' and 'height' values" );
		}
	}
	else
	{
		// Complex case
		lua_getfield( L, optionsIndex, "frames" );
		bool hasFrames = lua_istable( L, -1 );
		lua_pop( L, 1 );

		if ( hasFrames )
		{
			lua_getfield( L, optionsIndex, "frames" );

			int framesIndex = lua_gettop( L );
			for ( int i = 0, iMax = (int) lua_objlen( L, framesIndex ); i < iMax; i++ )
			{
				int index = (i+1); // Lua is 1-based so (i+1)th frame.
				lua_rawgeti( L, framesIndex, index );
				{
					int element = lua_gettop( L );
					if ( lua_istable( L, element ) )
					{
						lua_getfield( L, element, "x" );
						S32 x = (S32) lua_tointeger( L, -1 );
						
						lua_getfield( L, element, "y" );
						S32 y = (S32) lua_tointeger( L, -1 );

						lua_getfield( L, element, "width" );
						S32 frameW = (S32) lua_tointeger( L, -1 );
						
						lua_getfield( L, element, "height" );
						S32 frameH = (S32) lua_tointeger( L, -1 );

						lua_pop( L, 4 );

						S32 srcX, srcY, srcW, srcH;
						GetTrimData( L, element, srcX, srcY, srcW, srcH );
						bool isTrimmed = IsTrimmed( srcW, srcH, frameW, frameH );

						ImageFrame *f = Rtt_NEW( allocator, ImageFrame( * this, x, y, frameW, frameH, sx, sy, intrudeHalfTexel ) );
						if ( isTrimmed )
						{
							f->SetTrimData( srcX, srcY, srcW, srcH );
						}
						fFrames.Append( f );
					}
					else
					{
						// Lua is 1-based so (i+1)th frame.
						luaL_error( L, "for multiple frame sizes, 'options' should contain an numerically-ordered array of tables. However, element %d, i.e. options[%d], was not a table.", index, index );
					}
				}
				lua_pop( L, 1 );
			}
			lua_pop( L, 1 );
		}
		else
		{
			lua_getfield( L, optionsIndex, "spriteSheetFrames" );
			bool isOldFormat = lua_istable( L, -1 );

			if ( isOldFormat )
			{
				// Backward-compatibility to 'coordinateData' from sprite.newSpriteSheetFromData()
				int framesIndex = lua_gettop( L );

				for ( int i = 0, iMax = (int) lua_objlen( L, framesIndex ); i < iMax; i++ )
				{
					int index = (i+1); // Lua is 1-based so (i+1)th frame.
					lua_rawgeti( L, framesIndex, index );
					{
						int element = lua_gettop( L );
						if ( lua_istable( L, element ) )
						{
							SpriteSourceFrame data;
							SpriteSourceFrame::Initialize( L, data );

							S32 x = data.fSheetOrigin.x;
							S32 y = data.fSheetOrigin.y;
							S32 frameW = data.fSheetSize.x;
							S32 frameH = data.fSheetSize.y;

							S32 srcX = 0, srcY = 0, srcW = 0, srcH = 0;
							bool isTrimmed = data.fTrimmed;
							if ( isTrimmed )
							{
								srcX = data.fTrimmedOrigin.x;
								srcY = data.fTrimmedOrigin.y;
								srcW = data.fSourceSize.x;
								srcH = data.fSourceSize.y;
							}

							ImageFrame *f = Rtt_NEW( allocator, ImageFrame( * this, x, y, frameW, frameH, sx, sy, intrudeHalfTexel ) );
							if ( isTrimmed )
							{
								f->SetTrimData( srcX, srcY, srcW, srcH );
							}
							fFrames.Append( f );
						}
						else
						{
							// Lua is 1-based so (i+1)th frame.
							luaL_error( L, "for multiple frame sizes using spritesheet format, 'options.frames' should be a numerically-ordered array of tables. However, element %d, i.e. options.frames[%d], was not a table.", index, index );
						}
					}
					lua_pop( L, 1 );
				}
			}

			lua_pop( L, 1 );
		}
	}

	Rtt_ASSERT( lua_gettop( L ) == top );

	return 0;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

