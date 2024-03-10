//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_VulkanRenderer.h"
#include "Renderer/Rtt_VulkanContext.h"
#include "Renderer/Rtt_VulkanFrameBufferObject.h"
#include "Renderer/Rtt_VulkanTexture.h"

#include "Renderer/Rtt_FrameBufferObject.h"
#include "Renderer/Rtt_Texture.h"
#include "Core/Rtt_Assert.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static VkAttachmentLoadOp
GetLoadOp( const RenderPassBuilder::AttachmentOptions & options )
{
	return options.isResolve ? VK_ATTACHMENT_LOAD_OP_DONT_CARE : VK_ATTACHMENT_LOAD_OP_CLEAR;
}

static VkAttachmentDescription
PrepareAttachmentDescription( VkFormat format )
{
	VkAttachmentDescription attachment = {};
	
	attachment.format = format;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	return attachment;
}

void
RenderPassBuilder::AddColorAttachment( VkFormat format, const AttachmentOptions & options )
{
	VkAttachmentDescription colorAttachment = PrepareAttachmentDescription( format );

	if (options.isResolve)
	{
		Rtt_ASSERT( options.isResult );

		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}

	else
	{
		colorAttachment.samples = options.samples;
	}

	if (options.isResult)
	{
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	}

	AddAttachment( colorAttachment, options.isResolve ? fResolveReferences : fColorReferences, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, options.finalLayout );
}

void
RenderPassBuilder::AddDepthStencilAttachment( VkFormat format, const AttachmentOptions & options )
{
	VkAttachmentDescription depthAttachment = PrepareAttachmentDescription( format );

	AddAttachment( depthAttachment, fDepthStencilReferences, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL );
}

void
RenderPassBuilder::AddSubpassDependency( const VkSubpassDependency & dependency )
{
	fDependencies.push_back( dependency );
}

VkRenderPass
RenderPassBuilder::Build( VkDevice device, const VkAllocationCallbacks * allocator ) const
{
	VkSubpassDescription subpass = {};

	subpass.colorAttachmentCount = fColorReferences.size();
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = fColorReferences.data();
	subpass.pDepthStencilAttachment = fDepthStencilReferences.data(); // 0 or 1
	subpass.pResolveAttachments = fResolveReferences.data(); // 0 or same as color references

	VkRenderPassCreateInfo createRenderPassInfo = {};

    createRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createRenderPassInfo.attachmentCount = fDescriptions.size();
    createRenderPassInfo.dependencyCount = fDependencies.size();
    createRenderPassInfo.pAttachments = fDescriptions.data();
    createRenderPassInfo.pDependencies = fDependencies.data();
    createRenderPassInfo.pSubpasses = &subpass;
    createRenderPassInfo.subpassCount = 1U;

	VkRenderPass renderPass = VK_NULL_HANDLE;

    if (VK_SUCCESS == vkCreateRenderPass( device, &createRenderPassInfo, allocator, &renderPass ))
	{
		return renderPass;
	}

	else
	{
        Rtt_TRACE_SIM(( "ERROR: Failed to create render pass!" ));
		
		return VK_NULL_HANDLE;
    }
}

void
RenderPassBuilder::GetKey( RenderPassKey & key ) const
{
	U8 descriptionCount = U8( fDescriptions.size() ), dependencyCount = U8( fDependencies.size() );

	std::vector< U8 > contents(
		2U + // counts
		descriptionCount * 5U + // descriptions
		dependencyCount * sizeof( VkSubpassDependency ) // dependencies
	);

	contents[0] = descriptionCount;
	contents[1] = dependencyCount;

	int index = 2;

	for ( const VkAttachmentDescription & desc : fDescriptions )
	{
		contents[index++] = U8( desc.format );
		contents[index++] = U8( (desc.loadOp & 0x0F) | ((desc.storeOp << 4U) & 0xF0) );
		contents[index++] = U8( desc.samples );
		contents[index++] = U8( desc.initialLayout );

		if (desc.finalLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) // non-byte extension enum
		{
			contents[index++] = U8( desc.finalLayout );
		}

		else
		{
			contents[index++] = 0xFF;
		}
	}

	for ( const VkSubpassDependency & dep : fDependencies )
	{
		memcpy( contents.data() + index, &dep, sizeof( VkSubpassDependency ) );

		index += sizeof( VkSubpassDependency );
	}

	key.SetContents( contents );
}

void
RenderPassBuilder::AddReadAfterWriteStages()
{
	for (VkSubpassDependency & dependency : fDependencies)
	{
		if (VK_SUBPASS_EXTERNAL == dependency.srcSubpass)
		{
			dependency.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
	}
}

void
RenderPassBuilder::ReplaceClearsWithLoads()
{
	for (VkAttachmentDescription & desc : fDescriptions)
	{
		if (VK_ATTACHMENT_LOAD_OP_CLEAR == desc.loadOp)
		{
			desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
	}
}

void
RenderPassBuilder::AddAttachment( VkAttachmentDescription & description, std::vector< VkAttachmentReference > & references, VkImageLayout layout, VkImageLayout finalLayout )
{
	VkAttachmentReference attachmentRef = {};

	attachmentRef.attachment = fDescriptions.size();
	attachmentRef.layout = layout;

	references.push_back( attachmentRef );

	description.finalLayout = finalLayout;
	description.samples = description.samples;

	fDescriptions.push_back( description );
}

TextureSwapchain::TextureSwapchain( Rtt_Allocator * allocator, VulkanContext * context )
:	Super( allocator ),
	fContext( context )
{
}

TextureSwapchain::~TextureSwapchain()
{
}

U32
TextureSwapchain::GetWidth() const
{
	return fContext->GetSwapchainDetails().fExtent.width;
}

U32
TextureSwapchain::GetHeight() const
{
	return fContext->GetSwapchainDetails().fExtent.height;
}

VulkanFrameBufferObject::VulkanFrameBufferObject( VulkanRenderer & renderer )
:	fRenderer( renderer )
{
	fRenderPassData[0] = fRenderPassData[1] = NULL;
}

void 
VulkanFrameBufferObject::Create( CPUResource* resource )
{
	Rtt_ASSERT( CPUResource::kFrameBufferObject == resource->GetType() );

	Update( resource );
}

void 
VulkanFrameBufferObject::Update( CPUResource* resource )
{
	Rtt_ASSERT( CPUResource::kFrameBufferObject == resource->GetType() );
	FrameBufferObject * fbo = static_cast< FrameBufferObject * >( resource );

	fTexture = fbo->GetTexture();

	fExtent.width = fTexture->GetWidth();
	fExtent.height = fTexture->GetHeight();

	CleanUpImageData();

	bool isSwapchain = Texture::kNumFilters == fTexture->GetFilter(), wantMultisampleResources = isSwapchain && fRenderer.GetMultisampleEnabled();
	auto ci = fRenderer.GetContext()->GetCommonInfo();
	VkComponentMapping mapping = {};
	VkFormat format = isSwapchain ? ci.context->GetSwapchainDetails().fFormat.format : VulkanTexture::GetVulkanFormat( fTexture->GetFormat(), mapping );

	RenderPassBuilder builder;

	fSampleCount = VK_SAMPLE_COUNT_1_BIT;

	if (wantMultisampleResources)
	{
		fSampleCount = VkSampleCountFlagBits( ci.context->GetSampleCountFlags() );

		VulkanTexture::ImageData color = VulkanTexture::CreateImage(
			ci.context,
			fExtent.width, fExtent.height, 1U,
			fSampleCount,
			format,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		VkImageView colorView = VulkanTexture::CreateImageView( ci.context, color.fImage, format, VK_IMAGE_ASPECT_COLOR_BIT, 1U );

		fImages.push_back( color.fImage );
		fMemory.push_back( color.fMemory );
		fImageViews.push_back( colorView );

		RenderPassBuilder::AttachmentOptions options;

		options.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		options.samples = fSampleCount;

		builder.AddColorAttachment( format, options );
	}

	RenderPassBuilder::AttachmentOptions finalResultOptions;
	
	VulkanTexture * vulkanTexture = static_cast< VulkanTexture * >( fTexture->GetGPUResource() );
	size_t currentSize = fImageViews.size();

	if (isSwapchain)
	{
		auto & swapchainImages = fRenderer.GetSwapchainImages();

		for (VkImage swapchainImage : swapchainImages)
		{
			VkImageView swapchainView = VulkanTexture::CreateImageView( ci.context, swapchainImage, format, VK_IMAGE_ASPECT_COLOR_BIT, 1U );

			fImageViews.push_back( swapchainView );
		}

		finalResultOptions.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
	
	else
	{
		fImageViews.push_back( vulkanTexture->GetImageView() );

		VkSubpassDependency srcDependency;

		srcDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		srcDependency.dstSubpass = 0;
		srcDependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		srcDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		srcDependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		srcDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		srcDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		builder.AddSubpassDependency( srcDependency );

		VkSubpassDependency dstDependency;

		dstDependency.srcSubpass = 0;
		dstDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
		dstDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dstDependency.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dstDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dstDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dstDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		builder.AddSubpassDependency( dstDependency );

		finalResultOptions.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	finalResultOptions.isResolve = wantMultisampleResources;
	finalResultOptions.isResult = true;

	builder.AddColorAttachment( format, finalResultOptions );

	size_t passCount = fbo->GetMustClear() ? 1 : 2;

	if (!isSwapchain)
	{
		// TODO: leave as is for now, but might be able to refine the dependencies above based on GetMustClear()
	//	builder.AddReadAfterWriteStages();
	}

	for (size_t i = 0; i < passCount; ++i)
	{
		RenderPassKey key;

		builder.GetKey( key );

		fRenderPassData[i] = ci.context->FindRenderPassData( key );

		if (!fRenderPassData[i])
		{
			VkRenderPass renderPass = builder.Build( ci.device, ci.allocator );

			fRenderPassData[i] = ci.context->AddRenderPass( key, renderPass );
		}

		builder.ReplaceClearsWithLoads();
	}

	size_t count = fImageViews.size() - currentSize;

	for (size_t i = 0; i < passCount; ++i)
	{
		for (size_t j = 0; j < count; ++j)
		{
			VkFramebufferCreateInfo createFramebufferInfo = {};

			createFramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			createFramebufferInfo.renderPass = fRenderPassData[i]->fPass;
			createFramebufferInfo.attachmentCount = currentSize + 1; // n.b. ignore "extra" image views, cf. note a few lines below
			createFramebufferInfo.height = fExtent.height;
			createFramebufferInfo.layers = 1U;
			createFramebufferInfo.pAttachments = fImageViews.data();
			createFramebufferInfo.width = fExtent.width;
	// TODO: look into VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT...
			VkFramebuffer framebuffer = VK_NULL_HANDLE;

		//	vulkanTexture->Toggle();

			if (VK_SUCCESS == vkCreateFramebuffer( ci.device, &createFramebufferInfo, ci.allocator, &framebuffer ))
			{
				fFramebuffers.push_back( framebuffer );

				if (isSwapchain && j + 1 != count) // move relevant swapchain image view to front; will only be used afterward for cleanup, so order only matters here
				{
					VkImageView temp = fImageViews[currentSize];

					fImageViews[currentSize] = fImageViews[currentSize + j + 1];
					fImageViews[currentSize + j + 1] = temp;
				}
			}

			else
			{
				Rtt_TRACE_SIM(( "ERROR: Failed to create framebuffer!" ));

				// TODO?
			}
		}

		// todo? set to 1 on second pass for load op version?
	}

	if (!isSwapchain)
	{
		fImageViews.clear(); // owned by texture
	}
}

void 
VulkanFrameBufferObject::Destroy()
{
	CleanUpImageData();
}

void
VulkanFrameBufferObject::Bind( VulkanRenderer & renderer, uint32_t index, VkRenderPassBeginInfo & passBeginInfo )
{
	uint32_t passIndex = fRenderPassData[1] ? index : 0;
	const RenderPassData * renderPassData = fRenderPassData[passIndex];

	passBeginInfo.renderPass = renderPassData->fPass;

	fRenderer.SetMultisample( fSampleCount );
	fRenderer.SetRenderPass( renderPassData->fID, renderPassData->fPass );

	VulkanTexture * texture = static_cast< VulkanTexture * >( GetTextureName() );

	passBeginInfo.framebuffer = fFramebuffers[index];
	passBeginInfo.renderArea.extent = fExtent;
}

void
VulkanFrameBufferObject::BeginOffscreenPass( VulkanRenderer & renderer, VkCommandBuffer commandBuffer, bool load )
{
	if (load)
	{
		VulkanTexture * texture = static_cast< VulkanTexture * >( this->GetTextureName() );
		VkImageMemoryBarrier barrier = {};

		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = texture->GetImage();
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.layerCount = 1U;
		barrier.subresourceRange.levelCount = 1U;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, 0U, NULL, 0U, NULL,
			1U, &barrier
		);
	}
}

void
VulkanFrameBufferObject::CleanUpImageData()
{
	auto ci = fRenderer.GetContext()->GetCommonInfo();

	for (VkFramebuffer framebuffer : fFramebuffers)
	{
		vkDestroyFramebuffer( ci.device, framebuffer, ci.allocator );
	}

	for (VkImageView view : fImageViews)
	{
		vkDestroyImageView( ci.device, view, ci.allocator );
	}

	for (VkImage image : fImages)
	{
		vkDestroyImage( ci.device, image, ci.allocator );
	}

	for (VkDeviceMemory memory : fMemory)
	{
		vkFreeMemory( ci.device, memory, ci.allocator );
	}

	fFramebuffers.clear();
	fImageViews.clear();
	fImages.clear();
	fMemory.clear();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
