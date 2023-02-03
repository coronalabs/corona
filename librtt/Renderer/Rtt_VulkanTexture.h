//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_VulkanTexture_H__
#define _Rtt_VulkanTexture_H__

#include "Renderer/Rtt_GPUResource.h"
#include "Renderer/Rtt_VulkanIncludes.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class Texture;
class VulkanBufferData;
class VulkanContext;
struct Descriptor;

// ----------------------------------------------------------------------------

class VulkanTexture : public GPUResource
{
	public:
		typedef GPUResource Super;
		typedef VulkanTexture Self;

	public:
		VulkanTexture( VulkanContext * context );

	public:
		virtual void Create( CPUResource* resource );
		virtual void Update( CPUResource* resource );
		virtual void Destroy();

		void Bind( Descriptor & desc, VkDescriptorImageInfo & imageInfo );

	public:
		void CopyBufferToImage( VkBuffer buffer, VkImage image, uint32_t width, uint32_t height );
		bool Load( Texture * texture, VkFormat format, const VulkanBufferData & bufferData, U32 mipLevels );
		static bool TransitionImageLayout( VulkanContext * context, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, VkCommandBuffer = VK_NULL_HANDLE );

		VkImage GetImage() const { return fData.fImage; }
		VkImageView GetImageView() const { return fData.fView; }
		VkSampler GetSampler() const { return fSampler; }
		VkFormat GetFormat() const { return fFormat; }

	public:
		struct ImageData {
			ImageData()
			:	fImage( VK_NULL_HANDLE ),
				fView( VK_NULL_HANDLE ),
				fMemory( VK_NULL_HANDLE )
			{
			}

			void Destroy( VkDevice device, const VkAllocationCallbacks * allocator );

			VkImage fImage;
			VkImageView fView;
			VkDeviceMemory fMemory;
		};

		static ImageData CreateImage( VulkanContext * context, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties );
		static VkImageView CreateImageView( VulkanContext * context, VkImage image, VkFormat format, VkImageAspectFlags flags, uint32_t mipLevels, const VkComponentMapping * componentMapping = NULL );
		static VkFormat GetVulkanFormat( Texture::Format format, VkComponentMapping & mapping );

	private:
		ImageData fData;
		VulkanContext * fContext;
		VkSampler fSampler;
		VkFormat fFormat;
//		uint32_t fMipLevels;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_VulkanTexture_H__
