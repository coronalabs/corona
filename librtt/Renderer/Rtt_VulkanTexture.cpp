//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Config.h"

#include "Renderer/Rtt_VulkanRenderer.h"
#include "Renderer/Rtt_VulkanContext.h"
#include "Renderer/Rtt_VulkanTexture.h"
#include "Core/Rtt_Assert.h"

#include <algorithm>
#include <cmath>

// ----------------------------------------------------------------------------

namespace /*anonymous*/ 
{ 
	using namespace Rtt;

	void getFilterTokens( Texture::Filter filter, VkFilter & minFilter, VkFilter & magFilter )
	{
		switch( filter )
		{
			case Texture::kNearest:	minFilter = VK_FILTER_NEAREST;	magFilter = VK_FILTER_NEAREST;	break;
			case Texture::kLinear:	minFilter = VK_FILTER_LINEAR;	magFilter = VK_FILTER_LINEAR;	break;
			default: Rtt_ASSERT_NOT_REACHED();
		}
	}

	VkSamplerAddressMode convertWrapToken( Texture::Wrap wrap )
	{
		VkSamplerAddressMode result = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		switch( wrap )
		{
			case Texture::kClampToEdge:		result = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; break;
			case Texture::kRepeat:			result = VK_SAMPLER_ADDRESS_MODE_REPEAT; break;
			case Texture::kMirroredRepeat:	result = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; break;
            // also available: clamp to border, mirrored clamp to edge
			default: Rtt_ASSERT_NOT_REACHED();
		}

		return result;
	}
}

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

VulkanTexture::VulkanTexture( VulkanContext * context )
:	fContext( context ),
    fSampler( VK_NULL_HANDLE ),
    fFormat( VK_FORMAT_UNDEFINED )/*,
    fToggled( false )*/
{
}

void 
VulkanTexture::Create( CPUResource* resource )
{
	Rtt_ASSERT( CPUResource::kTexture == resource->GetType() || CPUResource::kVideoTexture == resource->GetType() );
	Texture* texture = static_cast< Texture* >( resource );

    if (Texture::kNumFilters == texture->GetFilter()) // dummy swapchain texture?
    {
        return;
    }

    uint32_t imageCount = texture->IsTarget() ? fContext->GetSwapchainDetails().fImageCount : 1U;

    VkComponentMapping mapping = {};
    VkFormat format = GetVulkanFormat( texture->GetFormat(), mapping );
    
    VkDeviceSize imageSize = texture->GetSizeInBytes();
    U32 mipLevels = /* static_cast< uint32_t >( std::floor( std::log2( std::max( texture->GetWidth(), texture->GetHeight() ) ) ) ) + */ 1U;
    
	VulkanBufferData bufferData( fContext->GetDevice(), fContext->GetAllocator() );

    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    bool ok = true;

    if (texture->IsTarget())
    {
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    else
    {
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        ok = fContext->CreateBuffer( imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferData );
                                        // ^^ TODO: also non-buffered approach? (suggestions that we should recycle a buffer)
                                        // is it okay to let this go away or should it be backed for a while still?
    }

    if (ok)
    {
        fData = CreateImage(
            fContext,
            texture->GetWidth(), texture->GetHeight(),
            1U, // mip levels
            VK_SAMPLE_COUNT_1_BIT,
            format,
            VK_IMAGE_TILING_OPTIMAL,
            usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        ok = fData.fImage != VK_NULL_HANDLE;

        if (ok)
        {
            ok = Load( texture, format, bufferData, mipLevels );
        }
    }
    
    texture->ReleaseData();

    if (ok)
    {
        VkSamplerCreateInfo samplerInfo = {};

        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        getFilterTokens( texture->GetFilter(), samplerInfo.minFilter, samplerInfo.magFilter );
     
        samplerInfo.addressModeU = convertWrapToken( texture->GetWrapX() );
        samplerInfo.addressModeV = convertWrapToken( texture->GetWrapY() );
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        if (fContext->GetFeatures().samplerAnisotropy)
        {
            samplerInfo.anisotropyEnable = VK_TRUE;
            samplerInfo.maxAnisotropy = 16.f;
        }

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.maxLod = 1.f;

        VkSampler sampler = VK_NULL_HANDLE;

        if (VK_SUCCESS == vkCreateSampler( fContext->GetDevice(), &samplerInfo, fContext->GetAllocator(), &sampler ))
        {
            fData.fView = CreateImageView( fContext, fData.fImage, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, &mapping );

            fSampler = sampler;
            fFormat = format;
        }
        
        else
        {
            Rtt_TRACE_SIM(( "ERROR: Failed to create texture sampler!" ));
        }
    }
}

void 
VulkanTexture::Update( CPUResource* resource )
{
	Rtt_ASSERT( CPUResource::kTexture == resource->GetType() );
	Texture* texture = static_cast<Texture*>( resource );

    if (!texture->IsTarget())
    {
        VulkanBufferData bufferData( fContext->GetDevice(), fContext->GetAllocator() );

        if (fContext->CreateBuffer( texture->GetSizeInBytes(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferData ))
        {
            Load( texture, GetFormat(), bufferData, 1U );
        }
    }
    
    texture->ReleaseData();
}

void 
VulkanTexture::Destroy()
{
    auto ci = fContext->GetCommonInfo();

    vkDestroySampler( ci.device, fSampler, ci.allocator );

    fData.Destroy( ci.device, ci.allocator );
}

void
VulkanTexture::Bind( Descriptor & desc, VkDescriptorImageInfo & imageInfo )
{
    VkImageView view = GetImageView();

    if (imageInfo.imageView != view)
    {
        desc.fDirty = true;

	    imageInfo.imageView = view;
	    imageInfo.sampler = fSampler;
	    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
}

void
VulkanTexture::CopyBufferToImage( VkBuffer buffer, VkImage image, uint32_t width, uint32_t height )
{
    VkCommandBuffer commandBuffer = fContext->BeginSingleTimeCommands();
    VkBufferImageCopy region = {};

    region.imageExtent = { width, height, 1U };
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1U;

    vkCmdCopyBufferToImage( commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, &region );

    fContext->EndSingleTimeCommands( commandBuffer );
}

static VkImageMemoryBarrier
PrepareBarrier( VkImage image, VkImageAspectFlags aspectFlags, uint32_t mipLevels )
{
    VkImageMemoryBarrier barrier = {};

    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.layerCount = 1U;
    barrier.subresourceRange.levelCount = mipLevels;

    return barrier;
}

bool
VulkanTexture::Load( Texture * texture, VkFormat format, const VulkanBufferData & bufferData, U32 mipLevels )
{
    const void * data = !texture->IsTarget() ? texture->GetData() : NULL;

    if (data)
    {
        fContext->StageData( bufferData.GetMemory(), data, texture->GetSizeInBytes() );
    }

    VkImage image = fData.fImage;
    VkImageLayout newLayout = texture->IsTarget() ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    bool ok = TransitionImageLayout( fContext, image, format, VK_IMAGE_LAYOUT_UNDEFINED, newLayout, mipLevels );

    if (ok && !texture->IsTarget())
    {
        CopyBufferToImage( bufferData.GetBuffer(), image, texture->GetWidth(), texture->GetHeight() );

        VkCommandBuffer commandBuffer = fContext->BeginSingleTimeCommands();
        VkImageMemoryBarrier barrier = PrepareBarrier( image, VK_IMAGE_ASPECT_COLOR_BIT, 1U );

        // generateMipmaps(textureImage, format, texWidth, texHeight, mipLevels);
             
        barrier.subresourceRange.baseMipLevel = mipLevels - 1U;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0U, NULL,
            0U, NULL,
            1U, &barrier
        );

        fContext->EndSingleTimeCommands( commandBuffer );
    }

    return ok;
}

static bool
HasStencilComponent( VkFormat format )
{
    return VK_FORMAT_D32_SFLOAT_S8_UINT == format || VK_FORMAT_D24_UNORM_S8_UINT == format;
}

bool
VulkanTexture::TransitionImageLayout( VulkanContext * context, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, VkCommandBuffer commandBuffer )
{
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

    if (VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL == newLayout)
    {
        aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (HasStencilComponent( format ))
        {
            aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }

    bool haveCommandBuffer = VK_NULL_HANDLE != commandBuffer;

    if (!haveCommandBuffer)
    {
        commandBuffer = context->BeginSingleTimeCommands();
    }

    VkImageMemoryBarrier barrier = PrepareBarrier( image, aspectFlags, mipLevels );

    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    VkPipelineStageFlags destinationStage, sourceStage;

    // TODO: where do we want unstaged writes? (frequent texture updates)

    switch (oldLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        break;
    default:
        Rtt_ASSERT_NOT_IMPLEMENTED();
    }

    switch (newLayout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

        break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        break;
    case VK_IMAGE_LAYOUT_GENERAL:
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        break;
    default:
        Rtt_ASSERT_NOT_IMPLEMENTED();
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0U, NULL, // memory
        0U, NULL, // memory buffer
        1U, &barrier // image memory
    );

    if (!haveCommandBuffer)
    {
        context->EndSingleTimeCommands( commandBuffer );
    }

    return true;
}

void 
VulkanTexture::ImageData::Destroy( VkDevice device, const VkAllocationCallbacks * allocator )
{
    vkDestroyImageView( device, fView, allocator );
    vkDestroyImage( device, fImage, allocator );
    vkFreeMemory( device, fMemory, allocator );
}

VulkanTexture::ImageData
VulkanTexture::CreateImage( VulkanContext * context, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties )
{
    VkImageCreateInfo createImageInfo = {};

    createImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createImageInfo.arrayLayers = 1U;
    createImageInfo.extent = { width, height, 1U };
    createImageInfo.format = format;
    createImageInfo.imageType = VK_IMAGE_TYPE_2D;
    createImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createImageInfo.mipLevels = mipLevels;
    createImageInfo.samples = numSamples;
    createImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createImageInfo.tiling = tiling;
    createImageInfo.usage = usage;

    auto ci = context->GetCommonInfo();
    VkImage image = VK_NULL_HANDLE;
    ImageData imageData;

    if (VK_SUCCESS == vkCreateImage( ci.device, &createImageInfo, ci.allocator, &image ))
    {
        VkMemoryRequirements memRequirements;

        vkGetImageMemoryRequirements( ci.device, image, &memRequirements );

        VkMemoryAllocateInfo allocInfo = {};

        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;

        bool foundMemoryType = context->FindMemoryType( memRequirements.memoryTypeBits, properties, allocInfo.memoryTypeIndex );

        if (foundMemoryType && vkAllocateMemory( ci.device, &allocInfo, ci.allocator, &imageData.fMemory ) != VK_SUCCESS)
        {
            Rtt_TRACE_SIM(( "ERROR: Failed to allocate image memory!" ));
        }

        if (imageData.fMemory != VK_NULL_HANDLE)
        {
            vkBindImageMemory( ci.device, image, imageData.fMemory, 0U );

            imageData.fImage = image;
        }

        else
        {
            vkDestroyImage( ci.device, image, ci.allocator );
        }
    }

    else
    {
        Rtt_TRACE_SIM(( "ERROR: Failed to create image!" ));
    }

    return imageData;
}

VkImageView
VulkanTexture::CreateImageView( VulkanContext * context, VkImage image, VkFormat format, VkImageAspectFlags flags, uint32_t mipLevels, const VkComponentMapping * componentMapping )
{
	VkImageViewCreateInfo createImageViewInfo = {};

	createImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

    if (componentMapping)
    {
        createImageViewInfo.components = *componentMapping;
    }

    else
    {
	    createImageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	    createImageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	    createImageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	    createImageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    }

	createImageViewInfo.format = format;
	createImageViewInfo.image = image;
	createImageViewInfo.subresourceRange.aspectMask = flags;
	createImageViewInfo.subresourceRange.layerCount = 1U;
	createImageViewInfo.subresourceRange.levelCount = mipLevels;
	createImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	VkImageView view = VK_NULL_HANDLE;

	if (VK_SUCCESS == vkCreateImageView( context->GetDevice(), &createImageViewInfo, context->GetAllocator(), &view ))
	{
        return view;
    }

    else
    {
        Rtt_TRACE_SIM(( "ERROR: Failed to create image view!" ));

        return VK_NULL_HANDLE;
    }
}

VkFormat
VulkanTexture::GetVulkanFormat( Texture::Format format, VkComponentMapping & mapping )
{
    VkFormat vulkanFormat = VK_FORMAT_R8G8B8A8_UNORM; // TODO: allow sR* forms, floats, etc.

	switch( format )
	{
		case Texture::kAlpha:
            mapping.g = mapping.b = mapping.a = VK_COMPONENT_SWIZZLE_R;

            break;
        // ^^ TODO: guess!
		case Texture::kLuminance:
            vulkanFormat = VK_FORMAT_R8_UNORM;
                
            break;
        // ^^ TODO: guess!
        case Texture::kRGB:
            vulkanFormat = VK_FORMAT_R8G8B8_UNORM;
                
            break;
        case Texture::kRGBA:
            break;
		case Texture::kARGB: // cf. GLTexture
		case Texture::kBGRA:
            vulkanFormat = VK_FORMAT_B8G8R8A8_UNORM; // we need this for FBOs, so no swizzling

            break;
		case Texture::kABGR:
            mapping.r = VK_COMPONENT_SWIZZLE_A;
            mapping.g = VK_COMPONENT_SWIZZLE_B;
            mapping.b = VK_COMPONENT_SWIZZLE_G;
            mapping.a = VK_COMPONENT_SWIZZLE_R;

            break;
		default: Rtt_ASSERT_NOT_REACHED();
	}

    return vulkanFormat;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------