//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Core/Rtt_String.h"
#include "Display/Rtt_BufferBitmap.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayDefaults.h"
#include "Display/Rtt_PlatformBitmap.h"
#include "Display/Rtt_Scene.h"
#include "Display/Rtt_TextureFactory.h"
#include "Display/Rtt_TextureResource.h"
#include "Renderer/Rtt_Renderer.h"
#include "Renderer/Rtt_VideoTexture.h"

#include "Display/Rtt_TextureResourceBitmap.h"
#include "Display/Rtt_TextureResourceCanvas.h"
#include "Display/Rtt_TextureResourceExternal.h"

#include "Rtt_FilePath.h"
#include "Rtt_MPlatform.h"
#include "Rtt_Runtime.h"
#include "CoronaLua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

TextureFactory::TextureFactory( Display& display )
:	fCache(),
	fDisplay( display ),
	fDefault(),
	fContainerMask(),
	fVideo(),
	fVideoSource(kCamera),
	fTextureMemoryUsed( 0 ),
	fCreateQueue( display.GetAllocator() )
{
}

TextureFactory::~TextureFactory()
{
	fCreateQueue.Empty();
}

void
TextureFactory::PathForFile(
	String& filePath,
	const char *filename,
	MPlatform::Directory baseDir )
{
	// Get an absolute path to the given file name.
	// We'll use this as a string key in the runtime's texture resource table too.
	if (baseDir != MPlatform::kUnknownDir)
	{
		// Get a full path to the given file name.
		fDisplay.GetRuntime().Platform().PathForFile( filename, baseDir, MPlatform::kTestFileExists, filePath );
	}
	else
	{
		// The given file name belongs to an unknown directory that is likely outside of the sandbox.
		// Take it as is and hope it is a valid absolute path to an existing file.
		filePath.Set(filename);
	}
}

PlatformBitmap *
TextureFactory::CreateBitmap(
	const char *filePath, U32 flags, bool convertToGrayscale )
{
	// Validate.
	if (!filePath)
	{
		return NULL;
	}

	// Load the given image file.
	const Display& display = fDisplay;
	const MPlatform& platform = display.GetRuntime().Platform();
	PlatformBitmap* pBitmap = platform.CreateBitmap( filePath, convertToGrayscale );
	if (!pBitmap)
	{
		return NULL;
	}

#ifdef Rtt_AUTHORING_SIMULATOR

	const DisplayDefaults &defaults = display.GetDefaults();
	if ( defaults.GetTextureWrapX() != RenderTypes::TextureWrap::kClampToEdgeWrap || defaults.GetTextureWrapY() != RenderTypes::TextureWrap::kClampToEdgeWrap )
	{
		const U32 width = pBitmap->Width();
		const U32 height = pBitmap->Height();

		if (!(((width != 0) && !(width & (width - 1))) && ((height != 0) && !(height & (height - 1)))))
		{
			Rtt_TRACE_SIM( ( "WARNING: The dimensions of the image (%s) you provided for a repeating fill are not in a power-of-two. You might have some graphical issues on an actual device", filePath ) );
		}
	}

#endif

	pBitmap->SetMagFilter( display.GetDefaults().GetMagTextureFilter() );
	pBitmap->SetMinFilter( display.GetDefaults().GetMinTextureFilter() );
	pBitmap->SetWrapX( display.GetDefaults().GetTextureWrapX() );
	pBitmap->SetWrapY( display.GetDefaults().GetTextureWrapY() );

	if ( flags )
	{
		const PlatformBitmap::PropertyMask kMasks[] =
		{
			PlatformBitmap::kIsPremultiplied,
			PlatformBitmap::kIsBitsFullResolution,
			PlatformBitmap::kIsBitsAutoRotated,
			PlatformBitmap::kIsNearestAvailablePixelDensity
		};

		for ( int i = 0, iMax = sizeof( kMasks ) / sizeof( * kMasks );
			  i < iMax;
			  i++ )
		{
			PlatformBitmap::PropertyMask mask = kMasks[i];
			if ( mask & flags )
			{
				pBitmap->SetProperty( mask, true );
			}
		}
	}

	return pBitmap;
}

SharedPtr< TextureResource >
TextureFactory::Find( const std::string& key )
{
	SharedPtr< TextureResource > result;

	Cache::const_iterator element = fCache.find( key );
	if ( element != fCache.end() )
	{
		const CacheEntry& entry = element->second;
		const WeakPtr< TextureResource >& resource = entry.GetResource();
		if ( resource.NotNull() )
		{
			// Found an entry
			result = SharedPtr< TextureResource >( resource );
		}
		else
		{
			// Entry contains a "null" resource, so remove it
			fCache.erase( key );
		}
	}

	return result;
}

	
void TextureFactory::AddToPreloadQueue(SharedPtr< TextureResource > &resource)
{
	fCreateQueue.Append( resource );
}
	
void TextureFactory::AddToPreloadQueueByKey(std::string cacheKey)
{
	SharedPtr<TextureResource> ptr = Find(cacheKey);
	AddToPreloadQueue(ptr);
}


	
SharedPtr< TextureResource >
TextureFactory::CreateAndAdd( const std::string& key,
								PlatformBitmap *bitmap,
								bool useCache,
								bool isRetina )
{
	TextureResource *resource = TextureResourceBitmap::Create( * this, bitmap, isRetina );
	SharedPtr< TextureResource > result = SharedPtr< TextureResource >( resource );

	bool shouldPreload = fDisplay.GetDefaults().ShouldPreloadTextures();

	if ( shouldPreload )
	{
		// Add to create queue
		AddToPreloadQueue( result );
	}

	if ( useCache )
	{
		fCache[key] = CacheEntry( result );
		result->SetCacheKey(key);
	}
	
	return result;
}

// Every enterframe, this gets called
void
TextureFactory::Preload( Renderer& renderer )
{
	for ( int i = 0; i < fCreateQueue.Length(); i++ )
	{
		const WeakPtr< TextureResource > item = fCreateQueue[i];
		SharedPtr< TextureResource > texture( item );
		if ( texture.NotNull() )
		{
			TextureResource *texResource = & (*texture);
			if ( texResource )
			{
				Texture &tex = texResource->GetTexture();
				GPUResource *gpuResource = tex.GetGPUResource();
				if ( NULL == gpuResource )
				{
					renderer.QueueCreate( & tex );
				}
			}
		}
	}
	fCreateQueue.Empty();
}

SharedPtr< TextureResource >
TextureFactory::FindOrCreate(
	const char *filename,
	MPlatform::Directory baseDir,
	U32 flags,
	bool isMask )
{
	SharedPtr< TextureResource > result;
	
	if( MPlatform::kVirtualTexturesDir == baseDir )
	{
		// Virtual textures can come only from Cache.
		return Find(filename);
	}

	bool isRetina = false;

	// Check for a higher resolution image file using Corona's special suffix notation.
	String suffixedFilename( fDisplay.GetAllocator() );
	if ( PlatformBitmap::kIsNearestAvailablePixelDensity & flags )
	{
		if ( fDisplay.GetImageFilename( filename, baseDir, suffixedFilename ) )
		{
			filename = suffixedFilename.GetString();
			isRetina = true;
		}
	}

	String filePath( fDisplay.GetAllocator() );
	PathForFile( filePath, filename, baseDir );

	if (filePath.IsEmpty())
	{
        CoronaLuaWarning(fDisplay.GetL(), "Failed to find image '%s'", filename);

		Rtt_ASSERT( result.IsNull() );
		return result;
	}

	// Lookup cache
	std::string key( filePath.GetString() );
	result = Find( key );

	// Create on a cache miss
	// NOTE: Even if we found an entry, it could still be null.
	if ( result.IsNull() )
	{
		PlatformBitmap *bitmap = CreateBitmap( filePath.GetString(), flags, isMask );
		result = CreateAndAdd( key, bitmap, true, isRetina );
	}

	return result;
}

SharedPtr< TextureResource >
TextureFactory::FindOrCreate(
	const FilePath& filePath,
	U32 flags,
	bool isMask )
{
	return FindOrCreate( filePath.GetFilename(), filePath.GetBaseDir(), flags, isMask );
}

SharedPtr< TextureResource >
TextureFactory::FindOrCreate(
	PlatformBitmap *bitmap,
	bool useCache )
{
	SharedPtr< TextureResource > result;

	if ( bitmap )
	{
		const size_t kStrSize = sizeof( void* )*2 + 4;
		char str[ kStrSize ];
		snprintf( str, kStrSize, "%p", bitmap );
		std::string key( str );

		if ( useCache )
		{
			// Lookup cache
			result = Find( key );
		}

		// Create on a cache miss, or create if skipping cache lookup.
		// NOTE: Even if we found an entry, it could still be null.
		if ( result.IsNull() )
		{
			result = CreateAndAdd( key, bitmap, useCache, false );
		}
	}

	return result;
}

SharedPtr< TextureResource >
TextureFactory::GetDefault()
{
	SharedPtr< TextureResource > result( fDefault );

	// Check if ptr is valid, recreate resource if needed
	if ( result.IsNull() )
	{
#if defined( Rtt_OPENGLES )
		const Texture::Format kFormat = Texture::kRGBA;
#else
		const Texture::Format kFormat = Texture::kBGRA;
#endif

		result = SharedPtr< TextureResource >( TextureResourceBitmap::CreateDefault(* this, kFormat, Texture::kNearest ) );
		
		fDefault = result; // Store weak ptr to resource
	}
	
	// Return by value to ensure ref count doesn't go to 0
	return result;
}

SharedPtr< TextureResource >
TextureFactory::GetVideo()
{

	SharedPtr< TextureResource > result( fVideo );

	// Check if ptr is valid, recreate resource if needed
	if ( result.IsNull() )
	{
#if defined( Rtt_IPHONE_ENV )

		result = SharedPtr< TextureResource >( TextureResourceBitmap::CreateVideo(* this ) );
		
		VideoTexture *videoTexture = static_cast<VideoTexture*>(& result->GetTexture());
		videoTexture->SetSource(fVideoSource);
		
#else
	#if defined( Rtt_OPENGLES )
		const Texture::Format kFormat = Texture::kRGBA;
	#else
		const Texture::Format kFormat = Texture::kBGRA;
	#endif
		result = SharedPtr< TextureResource >( TextureResourceBitmap::CreateDefault(* this, kFormat, Texture::kNearest ) );
#endif
		
		fVideo = result; // Store weak ptr to resource
	}
	
	// Return by value to ensure ref count doesn't go to 0
	return result;
}

void
TextureFactory::SetVideoSource( VideoSource source )
{

#if defined( Rtt_IPHONE_ENV )
	fVideoSource = source;
	SharedPtr< TextureResource > currentResource( fVideo );
	// Check if ptr is valid, recreate resource if needed
	if ( currentResource.NotNull() )
	{
		VideoTexture *videoTexture = static_cast<VideoTexture*>(& currentResource->GetTexture());
		videoTexture->SetSource(fVideoSource);
	}
#endif

}

// Create an 8x8 mask with a 2 pixel thick black border and a white interior
static BufferBitmap *
NewContainerMaskBitmap( Rtt_Allocator *pAllocator )
{
#if defined(Rtt_LINUX_ENV)
	const size_t kLength = 8;
	const size_t kBorder = 2;
	const PlatformBitmap::Format kFormat = PlatformBitmap::kRGBA;
	const PlatformBitmap::Orientation kOrientation = PlatformBitmap::kUp;
	const U32 kWhite = 0xFFFFFFFF;

	BufferBitmap *result = Rtt_NEW(pAllocator, BufferBitmap(pAllocator, kLength, kLength, kFormat, kOrientation));

	// Initialize bytes to 0
	U32 *bits = (U32 *)result->WriteAccess();
	memset(bits, 0, result->NumBytes());

	// Set kWhite to center 4x4 pixels
	for (int j = kBorder, jMax = kLength - kBorder; j < jMax; j++)
	{
		for (int i = kBorder, iMax = kLength - kBorder; i < iMax; i++)
		{
			bits[j * kLength + i] = kWhite;
		}
	}
#else
	const size_t kLength = 8;
	const size_t kBorder = 2;
	const PlatformBitmap::Format kFormat = PlatformBitmap::kMask;
	const PlatformBitmap::Orientation kOrientation = PlatformBitmap::kUp;
	const size_t kBytesPerRow = kLength * PlatformBitmap::BytesPerPixel( kFormat );
	const U8 kWhite = 0xFF;

	BufferBitmap *result =
		Rtt_NEW( pAllocator, BufferBitmap( pAllocator, kLength, kLength, kFormat, kOrientation ) );

	// Initialize bytes to 0
	U8 *bits = (U8 *)result->WriteAccess();
	memset( bits, 0, result->NumBytes() );

	// Set kWhite to center 4x4 pixels
	for ( int j = kBorder, jMax = kLength - kBorder; j < jMax; j++ )
	{
		U8 *p = bits + j*kBytesPerRow;
		for ( int i = kBorder, iMax = kLength - kBorder; i < iMax; i++ )
		{
			p[i] = kWhite;
		}
	}
#endif
	return result;
}

SharedPtr< TextureResource >
TextureFactory::GetContainerMask()
{
	SharedPtr< TextureResource > result( fContainerMask );

	// Check if ptr is valid, recreate resource if needed
	if ( result.IsNull() )
	{
		BufferBitmap *pBitmap = NewContainerMaskBitmap( fDisplay.GetAllocator() );

		pBitmap->SetMagFilter( RenderTypes::kNearestTextureFilter );
		pBitmap->SetMinFilter( RenderTypes::kNearestTextureFilter );

		result = FindOrCreate( pBitmap, true );

//		TextureResource *resource = TextureResource::Create( * this, pBitmap, false );
//		result = SharedPtr< TextureResource >( resource );

		fContainerMask = result; // Store weak ptr to resource
	}
	
	// Return by value to ensure ref count doesn't go to 0
	return result;
}


SharedPtr< TextureResource >
TextureFactory::Create(
	int w, int h,
	Texture::Format format,
	Texture::Filter filter,
	Texture::Wrap wrap,
	bool save_to_file )
{
	return SharedPtr< TextureResource >( TextureResourceBitmap::Create(
		* this, w, h, format, filter, wrap, save_to_file ) );
}

SharedPtr< TextureResource >
TextureFactory::Create(
	const char *str,
	const PlatformFont& font,
	Real w, Real h,
	const char alignment[],
	Real& baselineOffset)
{
	PlatformBitmap *pBitmap =
		fDisplay.GetRuntime().Platform().CreateBitmapMask( str, font, w, h, alignment, baselineOffset );

	return SharedPtr< TextureResource >( TextureResourceBitmap::Create( * this, pBitmap, false ) );
}

void
TextureFactory::QueueRelease( Texture *texture )
{
	fDisplay.GetScene().QueueRelease( texture );
}

void
TextureFactory::DidAddTexture( const TextureResource& resource )
{
	// Only count images from application towards total texture memory count.
	// In the Corona simulator, we may load other images like the splash screen
	// or status bar which should *not* count towards the total.
	Runtime& runtime = GetDisplay().GetRuntime();
	if ( runtime.IsProperty( Runtime::kIsApplicationExecuting ) )
	{
		const Texture& texture = resource.GetTexture();
		size_t numTextureBytes = texture.GetSizeInBytes();
		fTextureMemoryUsed += numTextureBytes;
	}
}

void
TextureFactory::WillRemoveTexture( const TextureResource& resource )
{
	// Only count images from application towards total texture memory count.
	// In the Corona simulator, we may remove images like the splash screen
	// which should *not* count towards the total.
	Runtime& runtime = GetDisplay().GetRuntime();
	if ( runtime.IsProperty( Runtime::kIsApplicationExecuting ) )
	{
		const Texture& texture = resource.GetTexture();
		size_t numTextureBytes = texture.GetSizeInBytes();
		fTextureMemoryUsed -= numTextureBytes;
		Rtt_ASSERT( fTextureMemoryUsed >= 0 );
	}
}


SharedPtr< TextureResource >
TextureFactory::FindOrCreateCanvas(const std::string &cacheKey,
								Real width, Real height,
								int pixelWidth, int pixelHeight, bool isMask )

{
	SharedPtr< TextureResource > result = Find(cacheKey);
	if (result.NotNull())
	{
		return result;
	}
	
	TextureResourceCanvas *resource = TextureResourceCanvas::Create( * this, width, height, pixelWidth, pixelHeight, isMask ? Texture::kLuminance : Texture::kRGBA );
	result = SharedPtr< TextureResource >( resource );
	
	fCache[cacheKey] = CacheEntry( result );
	result->SetCacheKey(cacheKey);
	
	
	return result;
}

	
	
SharedPtr< TextureResource >
TextureFactory::FindOrCreateExternal(const std::string &cacheKey,
									 const CoronaExternalTextureCallbacks* callbacks,
									 void* context)

{
	if (fOwnedTextures.find(cacheKey) != fOwnedTextures.end())
	{
		Rtt_ASSERT_NOT_REACHED(); //we should create only unique external textures... Something is really wrong.
		return fOwnedTextures[cacheKey];
	}
	
	bool isRetina = GetDisplay().GetDefaults().IsExternalTextureRetina();

	TextureResource *resource = TextureResourceExternal::Create( *this, callbacks, context, isRetina );
	SharedPtr< TextureResource > result = SharedPtr< TextureResource >( resource );
	
	fCache[cacheKey] = CacheEntry( result );
	result->SetCacheKey(cacheKey);
	
	AddToTeardownList(cacheKey);
	
	return result;
}
	
void TextureFactory::Retain(const SharedPtr< TextureResource > &res)
{
	if (res.NotNull())
	{
		if (Rtt_VERIFY( res->GetCacheKey().length() ))
		{
			fOwnedTextures[res->GetCacheKey()] = res;
		}
	}
}

	
void TextureFactory::Release( const std::string &key )
{
	fOwnedTextures.erase(key);
}

	
void TextureFactory::ReleaseByType( TextureResource::TextureResourceType type)
{
	if ( TextureResource::kTextureResource_Any == type )
	{
		fOwnedTextures.clear();
	}
	else
	{
		OwnedTextures::iterator it = fOwnedTextures.begin();
		while (it != fOwnedTextures.end())
		{
			// erase in case it is empty or matching type 
			if (it->second.IsNull() || it->second->GetType() == type)
			{
				// Store item to erase in another iterator. We can not erase iterator and increment it.
				OwnedTextures::iterator toerase = it;
				// iterate to next element before erasing.
				++it;
				// C++ map has guarantee that erasing iterator would not make invalid other iterators
				fOwnedTextures.erase(toerase);
			}
			else
			{
				++it;
			}
		}
	}
}

void TextureFactory::Teardown()
{
	for(TextureKeySet::iterator it = fTeardownList.begin(); it!=fTeardownList.end(); it++)
	{
		SharedPtr< TextureResource > resource = Find(*it);
		if(resource.NotNull())
		{
			resource->Teardown();
		}
	}
	
	fTeardownList.clear();
}
	
void TextureFactory::AddToTeardownList( const std::string &key )
{
	fTeardownList.insert( key );
}
	
void TextureFactory::RemoveFromTeardownList( const std::string &key )
{
	fTeardownList.erase( key );
}
	
	
void TextureFactory::AddTextureToUpdateList( const std::string &key )
{
	fUpdateTextures.insert(key);
}

	
void TextureFactory::UpdateTextures(Renderer &renderer)
{
	for(TextureKeySet::iterator it = fUpdateTextures.begin(); it!=fUpdateTextures.end(); it++)
	{
		OwnedTextures::iterator ownedTex = fOwnedTextures.find(*it);
		if (ownedTex != fOwnedTextures.end())
		{
			SharedPtr<TextureResource> resource = ownedTex->second;
			if(resource.NotNull())
			{
				resource->Render(renderer);
			}
		}
		else
		{
			SharedPtr<TextureResource> resource = Find(*it);
			if(resource.NotNull())
			{
				resource->Render(renderer);
			}
		}
	}
	
	fUpdateTextures.clear();
}
	

	
} // namespace Rtt

// ----------------------------------------------------------------------------

