//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_VulkanFrameBufferObject_H__
#define _Rtt_VulkanFrameBufferObject_H__

#include "Renderer/Rtt_GPUResource.h"
#include "Renderer/Rtt_Texture.h"
#include "Renderer/Rtt_VulkanIncludes.h"

#include <vector>

// ----------------------------------------------------------------------------

namespace Rtt
{

class VulkanRenderer;
class RenderPassKey;
struct RenderPassData;

// ----------------------------------------------------------------------------

class RenderPassBuilder {
	public:
		struct AttachmentOptions {
			VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
			VkImageLayout finalLayout;
			bool isResolve = false;
			bool isResult = false; // n.b. already implied by isResolve
		};

		void AddColorAttachment( VkFormat format, const AttachmentOptions & options = AttachmentOptions() );
		void AddDepthStencilAttachment( VkFormat format, const AttachmentOptions & options = AttachmentOptions() );
		void AddSubpassDependency( const VkSubpassDependency & dependency );

		VkRenderPass Build( VkDevice device, const VkAllocationCallbacks * allocator ) const;
		void GetKey( RenderPassKey & key ) const;

		void AddReadAfterWriteStages();
		void ReplaceClearsWithLoads();

	private:
		void AddAttachment( VkAttachmentDescription & description, std::vector< VkAttachmentReference > & references, VkImageLayout layout, VkImageLayout finalLayout );

		std::vector< VkSubpassDependency > fDependencies;
		std::vector< VkAttachmentDescription > fDescriptions;
		std::vector< VkAttachmentReference > fColorReferences;
		std::vector< VkAttachmentReference > fDepthStencilReferences;
		std::vector< VkAttachmentReference > fResolveReferences;
};

class TextureSwapchain : public Texture
{
	public:
		typedef Texture Super;
		typedef TextureSwapchain Self;

	public:
		TextureSwapchain( Rtt_Allocator * allocator, VulkanContext * context );
		virtual ~TextureSwapchain();

	public:
		virtual U32 GetWidth() const;
		virtual U32 GetHeight() const;
		virtual Format GetFormat() const { return kNumFormats; }
		virtual Filter GetFilter() const { return kNumFilters; }

	private:
		VulkanContext * fContext;
};

class VulkanFrameBufferObject : public GPUResource
{
	public:
		typedef GPUResource Super;
		typedef VulkanFrameBufferObject Self;

	public:
		VulkanFrameBufferObject( VulkanRenderer & renderer );

	public:
		virtual void Create( CPUResource* resource );
		virtual void Update( CPUResource* resource );
		virtual void Destroy();

		void Bind( VulkanRenderer & renderer, uint32_t index, VkRenderPassBeginInfo & passBeginInfo );
		void BeginOffscreenPass( VulkanRenderer & renderer, VkCommandBuffer commandBuffer, bool load );

	public:
		GPUResource * GetTextureName() const { return fTexture->GetGPUResource(); }
	private:
		void CleanUpImageData();

	private:
		VulkanRenderer & fRenderer;
		VkSampleCountFlagBits fSampleCount;
		VkExtent2D fExtent;
		std::vector< VkFramebuffer > fFramebuffers;
		std::vector< VkDeviceMemory > fMemory;
		std::vector< VkImage > fImages;
		std::vector< VkImageView > fImageViews;
		const RenderPassData * fRenderPassData[2];
		Texture * fTexture;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_VulkanFrameBufferObject_H__
