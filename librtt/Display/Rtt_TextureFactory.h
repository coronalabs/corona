//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_TextureResource_H__
#define _Rtt_TextureResource_H__

#include "Core/Rtt_SharedPtr.h"
#include "Renderer/Rtt_Texture.h"
#include "Renderer/Rtt_VideoSource.h"
#include "Display/Rtt_TextureResource.h"

#include <string>
#include <map>
#include <set>

// ----------------------------------------------------------------------------

extern "C"
{
	struct CoronaExternalTextureCallbacks;
}

namespace Rtt
{

class Display;
class FilePath;
class TextureResource;

// ----------------------------------------------------------------------------

class TextureFactory
{
	public:
		TextureFactory( Display& display );
		~TextureFactory();

		void Preload( Renderer& renderer );

	protected:
		void PathForFile(
			String& filePath,
			const char *filename,
			MPlatform::Directory baseDir );

		PlatformBitmap *CreateBitmap(
			const char *filePath,
			U32 flags = 0, bool convertToGrayscale = false );

		SharedPtr< TextureResource > Find( const std::string& key );
		SharedPtr< TextureResource > CreateAndAdd( const std::string& key,
													PlatformBitmap *bitmap,
													bool useCache,
													bool isRetina );
	// Cached texture resources
	public:
		SharedPtr< TextureResource > FindOrCreate(
			const char *filename,
			MPlatform::Directory baseDir,
			U32 flags,
			bool isMask );

		SharedPtr< TextureResource > FindOrCreate(
			const FilePath& filePath,
			U32 flags,
			bool isMask );

		SharedPtr< TextureResource > FindOrCreate(
			PlatformBitmap *bitmap,
			bool useCache );

		SharedPtr< TextureResource > FindOrCreateCanvas(
			const std::string &cacheKey,
			Real w, Real h,
			int pixelW, int pixelH, bool isMask );

		SharedPtr< TextureResource > FindOrCreateCapture(
			const std::string &cacheKey,
			Real w, Real h,
			int pixelW, int pixelH );
	
		SharedPtr< TextureResource > FindOrCreateExternal(
			const std::string &cacheKey,
			const CoronaExternalTextureCallbacks* callbacks,
			void* context);


	// One-off texture resources
	public:
		SharedPtr< TextureResource > Create(
			int w, int h,
			Texture::Format format,
			Texture::Filter filter,
			Texture::Wrap wrap,
			bool save_to_file );

		SharedPtr< TextureResource > Create(
			const char *str,
			const PlatformFont& font,
			Real w, Real h,
			const char alignment[],
			Real& baselineOffset);
			
		SharedPtr< TextureResource > GetDefault();
		SharedPtr< TextureResource > GetContainerMask();
		SharedPtr< TextureResource > GetVideo();
		void SetVideoSource( VideoSource source );

	public:
		void AddToPreloadQueueByKey(std::string cacheKey);
		void AddToPreloadQueue(SharedPtr< TextureResource > &resource);

		void QueueRelease( Texture *texture );

		Display& GetDisplay() { return fDisplay; }

		void DidAddTexture( const TextureResource& resource );
		void WillRemoveTexture( const TextureResource& resource );
		S32 GetTextureMemoryUsed() const { return fTextureMemoryUsed; }

	protected:
		class CacheEntry
		{
			public:
				CacheEntry() : fResource() {}
				CacheEntry( const SharedPtr< TextureResource >& resource )
				:	fResource( resource )
				{
				}

			public:
				const WeakPtr< TextureResource >& GetResource() const
				{
					return fResource;
				}

			private:
				WeakPtr< TextureResource > fResource;
		};

		typedef std::map< std::string, CacheEntry > Cache;
	
		// This map holds strong reference to textures, so they wouldn't be released without being assigned to
		// display object's paint. Example is preloaded textures and Canvas texture objects.
		typedef std::map< std::string, SharedPtr<TextureResource> > OwnedTextures;
		OwnedTextures fOwnedTextures;
	public:
		void Retain(const SharedPtr< TextureResource > & );
		void Release(const std::string &key );
		void ReleaseByType( TextureResource::TextureResourceType type );
	
	protected:
		typedef std::set< std::string > TextureKeySet;
		TextureKeySet fUpdateTextures;
	public:
		void AddTextureToUpdateList( const std::string &key );
		void UpdateTextures(Renderer &renderer);
	
	protected:
		TextureKeySet fTeardownList;
	public:
		void Teardown();
		void AddToTeardownList( const std::string &key );
		void RemoveFromTeardownList( const std::string &key );

	private:
		Cache fCache;
		Array< WeakPtr< TextureResource > > fCreateQueue;
		Display &fDisplay;
		WeakPtr< TextureResource > fDefault;
		WeakPtr< TextureResource > fContainerMask;
		WeakPtr< TextureResource > fVideo;
		VideoSource fVideoSource;
		
		S32 fTextureMemoryUsed;
		
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TextureResource_H__
