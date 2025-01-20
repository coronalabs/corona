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
#include "Renderer/Rtt_VulkanCommandBuffer.h"
#include "Renderer/Rtt_VulkanFrameBufferObject.h"
#include "Renderer/Rtt_VulkanGeometry.h"
#include "Renderer/Rtt_VulkanProgram.h"
#include "Renderer/Rtt_VulkanTexture.h"
#include "Renderer/Rtt_CPUResource.h"
#include "Renderer/Rtt_FrameBufferObject.h"
#include "Display/Rtt_BufferBitmap.h"
#include "Core/Rtt_Assert.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

Descriptor::Descriptor(  VkDescriptorSetLayout setLayout )
:	fSetLayout( setLayout ),
	fDirty( 0U )
{
}

bool
Descriptor::IsMaskPushConstant( int index )
{
	return index >= Uniform::kMaskMatrix0 && index <= Uniform::kMaskMatrix2;
}

bool
Descriptor::IsPushConstant( int index, bool userDataPushConstants )
{
	return Uniform::kTotalTime == index
		|| Uniform::kTexelSize == index
		|| IsMaskPushConstant( index )
		|| (userDataPushConstants && index >= Uniform::kUserData0 && index <= Uniform::kUserData3);
}

bool
Descriptor::IsUserData( int index )
{
	return index >= Uniform::kUserData0;
}

BufferData::BufferData()
:	fData( NULL ),
	fMapped( NULL ),
	fSet( VK_NULL_HANDLE )
{
}

void
BufferData::Wipe()
{
	if (fMapped)
	{
		Rtt_ASSERT( fData );

		vkUnmapMemory( fData->GetDevice(), fData->GetMemory() );
	}

	Rtt_DELETE( fData );
}

BufferDescriptor::BufferDescriptor( VulkanContext * context, VkDescriptorPool pool, VkDescriptorSetLayout setLayout, VkDescriptorType type, size_t count, size_t size )
:	Descriptor( setLayout ),
	fLastSet( VK_NULL_HANDLE ),
	fType( type ),
	fDynamicAlignment( 0U ),
	fWorkspace( NULL ),
	fIndex( 0U ),
	fOffset( 0U ),
	fLastOffset( 0U ),
	fAtomSize( 0U ),
	fBufferSize( 0U ),
	fRawSize( size ),
	fNonCoherentRawSize( size ),
	fWritten( 0U ),
	fMarkWritten( false )
{
	const VkPhysicalDeviceLimits & limits = context->GetProperties().limits;
	VulkanBufferData bufferData( context->GetDevice(), context->GetAllocator() );

	VkBufferUsageFlags usageFlags = 0;
	uint32_t alignment = 0U, maxSize = ~0U;

	if (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER == type || VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC == type)
	{
		usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		alignment = limits.minUniformBufferOffsetAlignment;
		maxSize = limits.maxUniformBufferRange;
	}

	else if (VK_DESCRIPTOR_TYPE_STORAGE_BUFFER == type || VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC == type)
	{
		usageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		alignment = limits.minStorageBufferOffsetAlignment;
		maxSize = limits.maxStorageBufferRange;
	}

	bool isDynamic = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC == type || VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC == type;

	fDynamicAlignment = fRawSize;

	if (isDynamic && alignment > 0U)
	{
		fDynamicAlignment = (fDynamicAlignment + alignment - 1) & ~(alignment - 1);
	}

	fAtomSize = limits.nonCoherentAtomSize;

	U32 remainder = fNonCoherentRawSize % fAtomSize;

	if (remainder)
	{
		fNonCoherentRawSize += limits.nonCoherentAtomSize - remainder;

		Rtt_ASSERT( !isDynamic || fNonCoherentRawSize <= fDynamicAlignment );
	}

	fBufferSize = isDynamic ? U32( count * fDynamicAlignment ) : fRawSize;

	U32 n = fBufferSize / maxSize + (fBufferSize % maxSize ? 1 : 0);

	if (n > 1U)
	{
		fBufferSize = maxSize;
	}

// ^^ TODO: this is somewhat limiting for SSBO case

	for (U32 i = 0; i < n; ++i)
	{
		if (context->CreateBuffer( fBufferSize, usageFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, bufferData ))
		{
			BufferData buffer;

			buffer.fMapped = context->MapData( bufferData.GetMemory() );
			buffer.fData = bufferData.Extract( NULL );

			VkDescriptorSetAllocateInfo allocInfo = {};

			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = pool;
			allocInfo.descriptorSetCount = 1U;
			allocInfo.pSetLayouts = &fSetLayout;

			if (VK_SUCCESS == vkAllocateDescriptorSets( context->GetDevice(), &allocInfo, &buffer.fSet ))
			{
				VkWriteDescriptorSet descriptorWrite = {};
				VkDescriptorBufferInfo bufferInfo = {};

				bufferInfo.buffer = buffer.fData->GetBuffer();
				bufferInfo.range = fRawSize;// BufferSize;

				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.descriptorCount = 1U;
				descriptorWrite.descriptorType = type;
				descriptorWrite.dstSet = buffer.fSet;
				descriptorWrite.pBufferInfo = &bufferInfo;

				vkUpdateDescriptorSets( context->GetDevice(), 1U, &descriptorWrite, 0U, NULL );

				fBuffers.push_back( buffer );
			}

			else
			{
				buffer.Wipe();

				Rtt_TRACE_SIM(( "ERROR: Failed to allocate buffer descriptor" ));
			}
		}

		else
		{
			Rtt_TRACE_SIM(( "ERROR: Failed to create buffer" ));

			break;
		}
	}
}

void
BufferDescriptor::Reset( VkDevice )
{
	fLastSet = VK_NULL_HANDLE;
	fIndex = fOffset = fLastOffset = 0U;
	fDirty = fWritten = 0U;
}

void
BufferDescriptor::Wipe( VkDevice, const VkAllocationCallbacks * )
{
	for (BufferData & buffer : fBuffers)
	{
		buffer.Wipe();
	}

	fBuffers.clear();
}

void
BufferDescriptor::SetWorkspace( void * workspace )
{
	fWorkspace = static_cast< U8 * >( workspace );
}

void
BufferDescriptor::TryToAddMemory( std::vector< VkMappedMemoryRange > & ranges, VkDescriptorSet sets[], size_t & count )
{
	const BufferData & buffer = fBuffers[fIndex];
	bool allWritten = (fWritten & fDirty) == fDirty;

	if (!allWritten)
	{
		bool dynamicBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC == fType || VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC == fType;

		if (dynamicBuffer && fIndex == fBuffers.size())
		{
			Rtt_TRACE_SIM(( "ERROR: Failed to add memory: buffer full!" ));

			return; // TODO: much more than this needs hardening
		}

		memcpy( static_cast< U8 * >( buffer.fMapped ) + fOffset, fWorkspace, fRawSize );
			
		VkMappedMemoryRange range = {};

		bool isFull = fOffset + fNonCoherentRawSize >= fBufferSize;

		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.memory = buffer.fData->GetMemory();
		range.offset = fOffset; // TODO: must be multiple of atom size... how to handle if alignment disagrees?
		range.size =  isFull ? VK_WHOLE_SIZE : fNonCoherentRawSize;

		ranges.push_back( range );

		fLastSet = buffer.fSet;
		fLastOffset = fOffset;

		if (dynamicBuffer)
		{
			if (isFull)
			{
				fOffset = 0U;

				++fIndex;
			}

			else
			{
				fOffset += U32( fDynamicAlignment );
			}
		}
	}

	sets[count++] = allWritten ? fLastSet : buffer.fSet;
}
	
void
BufferDescriptor::TryToAddDynamicOffset( uint32_t offsets[], size_t & count )
{
	if (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC == fType || VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC == fType)
	{
		offsets[count++] = (fWritten & fDirty) == fDirty ? fLastOffset : fOffset;
	}
}

static VkDescriptorPool
AddPool( VulkanContext * context, const VkDescriptorPoolSize * sizes, uint32_t sizeCount, U32 maxSets, VkDescriptorPoolCreateFlags flags = 0 )
{
	VkDescriptorPoolCreateInfo poolInfo = {};

	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = flags;
	poolInfo.maxSets = maxSets;
	poolInfo.poolSizeCount = sizeCount;
	poolInfo.pPoolSizes = sizes;

	VkDescriptorPool pool = VK_NULL_HANDLE;

	if (VK_SUCCESS == vkCreateDescriptorPool( context->GetDevice(), &poolInfo, context->GetAllocator(), &pool ))
	{
		return pool;
	}

	else
	{
		Rtt_TRACE_SIM(( "ERROR: Failed to create descriptor pool!" ));

		return VK_NULL_HANDLE;
	}
}

TexturesDescriptor::TexturesDescriptor( VulkanContext * context, VkDescriptorSetLayout setLayout )
:	Descriptor( setLayout )
{
	const U32 arrayCount = 1024U; // TODO: is this how to allocate this? (maybe arrays are just too complex / wasteful for the common case)
	const U32 descriptorCount = arrayCount * 5U; // 2 + 3 masks (TODO: but could be more flexible? e.g. already reflected in VulkanProgram)

	VkDescriptorPoolSize poolSize;

	poolSize.descriptorCount = descriptorCount;
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	fPool = AddPool( context, &poolSize, 1U, arrayCount );
}

void
TexturesDescriptor::Reset( VkDevice device )
{
	vkResetDescriptorPool( device, fPool, 0 );

	fDirty = 0U;
}

static void
WipeDescriptorPool( VkDevice device, VkDescriptorPool pool, const VkAllocationCallbacks * allocator )
{
	if (VK_NULL_HANDLE != pool)
	{
		vkResetDescriptorPool( device, pool, 0 );
		vkDestroyDescriptorPool( device, pool, allocator );
	}
}

void
TexturesDescriptor::Wipe( VkDevice device, const VkAllocationCallbacks * allocator )
{
	WipeDescriptorPool( device, fPool, allocator );
}

FrameResources::FrameResources()
:	fUniforms( NULL ),
	fUserData( NULL ),
	fTextures( NULL ),
	fCommands( VK_NULL_HANDLE ),
	fImageAvailable( VK_NULL_HANDLE ),
	fRenderFinished( VK_NULL_HANDLE ),
	fFence( VK_NULL_HANDLE )
{
}

bool
FrameResources::AddSynchronizationObjects( VkDevice device, const VkAllocationCallbacks * allocator )
{
	VkFenceCreateInfo createFenceInfo = {};

	createFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (VK_SUCCESS != vkCreateFence( device, &createFenceInfo, allocator, &fFence ))
	{
		// TODO...
	}

	VkSemaphoreCreateInfo createSemaphoreInfo = {};

	createSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (VK_SUCCESS != vkCreateSemaphore( device, &createSemaphoreInfo, allocator, &fImageAvailable ))
	{
		// TODO...
	}

	if (VK_SUCCESS != vkCreateSemaphore( device, &createSemaphoreInfo, allocator, &fRenderFinished ))
	{
		// TODO...
	}

	return true;
}

void
FrameResources::CleanUpCommandPool( VkDevice device, const VkAllocationCallbacks * allocator )
{
	vkDestroyCommandPool( device, fCommands, allocator );

	fCommands = VK_NULL_HANDLE;
}

static void
CleanUpDescriptor( Descriptor * descriptor, VkDevice device, const VkAllocationCallbacks * allocator )
{
	if (descriptor)
	{
		descriptor->Wipe( device, allocator );

		Rtt_DELETE( descriptor );
	}
}

void
FrameResources::CleanUpDescriptorObjects( VkDevice device, const VkAllocationCallbacks * allocator )
{
	CleanUpDescriptor( fUniforms, device, allocator );
	CleanUpDescriptor( fUserData, device, allocator );
	CleanUpDescriptor( fTextures, device, allocator );

	fUniforms = NULL;
	fUserData = NULL;
	fTextures = NULL;
}

void
FrameResources::CleanUpSynchronizationObjects( VkDevice device, const VkAllocationCallbacks * allocator )
{
	vkDestroyFence( device, fFence, allocator );
	vkDestroySemaphore( device, fImageAvailable, allocator );
	vkDestroySemaphore( device, fRenderFinished, allocator );

	fFence = VK_NULL_HANDLE;
	fImageAvailable = VK_NULL_HANDLE;
	fRenderFinished = VK_NULL_HANDLE;
}

VulkanRenderer::VulkanRenderer( Rtt_Allocator* allocator, VulkanContext * context, void (*invalidate)(void *), void * display )
:   Super( allocator ),
	fContext( context ),
    fSwapchainTexture( NULL ),
	fPrimaryFBO( NULL ),
	fCaptureFBO( NULL ),
	fFirstPipeline( VK_NULL_HANDLE ),
	fPool( VK_NULL_HANDLE ),
	fUniformsLayout( VK_NULL_HANDLE ),
	fUserDataLayout( VK_NULL_HANDLE ),
	fTextureLayout( VK_NULL_HANDLE ),
	fPipelineLayout( VK_NULL_HANDLE ),
	fCaptureFence( VK_NULL_HANDLE ),
	fInvalidate( invalidate ),
	fDisplay( display ),
	fFrameIndex( 0 ),
	fSwapchainInvalid( false )
{
	fFrontCommandBuffer = Rtt_NEW( allocator, VulkanCommandBuffer( allocator, *this ) );
	fBackCommandBuffer = Rtt_NEW( allocator, VulkanCommandBuffer( allocator, *this ) );

	VkPushConstantRange pushConstantRange;

	pushConstantRange.offset = 0U;
	pushConstantRange.size = sizeof( VulkanPushConstants );
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo createDescriptorSetLayoutInfo = {};
	VkDescriptorSetLayoutBinding bindings[2] = {};

	createDescriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createDescriptorSetLayoutInfo.bindingCount = 1U;
	createDescriptorSetLayoutInfo.pBindings = bindings;

	bindings[0].descriptorCount = 1U;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	if (VK_SUCCESS == vkCreateDescriptorSetLayout( context->GetDevice(), &createDescriptorSetLayoutInfo, context->GetAllocator(), &fUniformsLayout ))
	{
	}

	else
	{
		Rtt_TRACE_SIM(( "ERROR: Failed to create UBO descriptor set layout!" ));
	}

	if (VK_SUCCESS == vkCreateDescriptorSetLayout( context->GetDevice(), &createDescriptorSetLayoutInfo, context->GetAllocator(), &fUserDataLayout ))
	{
	}

	else
	{
		Rtt_TRACE_SIM(( "ERROR: Failed to create uniform user data descriptor set layout!" ));
	}

	// if samplerIndexing...
		//	createDescriptorSetLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT; TODO: this seems right?
	bindings[0].descriptorCount = 5U; // TODO: locks in texture count, maybe later we'll want a higher value?
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	// ^^ this will be MUCH different if the hardware can support push constant-indexed samplers, e.g. something like
		// easy to do this without post-bind update?
		// bindingCount = 2U
		// binging 0: 1 (VK_DESCRIPTOR_TYPE_SAMPLER) descriptor
		// binding 1: 4096 (VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) descriptors

	if (VK_SUCCESS == vkCreateDescriptorSetLayout( context->GetDevice(), &createDescriptorSetLayoutInfo, context->GetAllocator(), &fTextureLayout ))
	{
	}

	else
	{
		Rtt_TRACE_SIM(( "ERROR: Failed to create texture descriptor set layout!" ));
	}
	
	VkPipelineLayoutCreateInfo createPipelineLayoutInfo = {};
	VkDescriptorSetLayout layouts[] = { fUniformsLayout, fUserDataLayout, fTextureLayout };

	createPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createPipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	createPipelineLayoutInfo.pSetLayouts = layouts;
	createPipelineLayoutInfo.pushConstantRangeCount = 1U;
	createPipelineLayoutInfo.setLayoutCount = 3U;

	if (VK_SUCCESS == vkCreatePipelineLayout( context->GetDevice(), &createPipelineLayoutInfo, context->GetAllocator(), &fPipelineLayout ))
	{
	}

	else
	{
		Rtt_TRACE_SIM(( "ERROR: Failed to create pipeline layout!" ));
	}

	fSwapchainTexture = Rtt_NEW( allocator, TextureSwapchain( allocator, context ) );

	VkDescriptorPoolSize size;

	size.descriptorCount = kFramesInFlight * 64U;
	size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

	fPool = AddPool( context, &size, 1U, kFramesInFlight * 80U ); // TODO: wild guess this many uniform buffers can satisfy 256 "render targets" / 1024 uniform blocks, cf. below

	const std::vector< uint32_t > & families = context->GetQueueFamilies();
	uint32_t graphicsFamilyIndex = context->GetGraphicsFamilyIndex();

	Rtt_ASSERT( !families.empty() );
	Rtt_ASSERT( graphicsFamilyIndex <= families.size() );

	uint32_t graphicsFamily = families[graphicsFamilyIndex];

	for (int i = 0; i < kFramesInFlight; ++i)
	{
		static_assert( Descriptor::kUniforms < Descriptor::kUserData, "Uniforms / buffer in unexpected order" );
		static_assert( Descriptor::kUserData < Descriptor::kTexture, "Buffer / textures in unexpected order" );

		fFrameResources[i].fUniforms = Rtt_NEW( NULL, BufferDescriptor( context, fPool, fUniformsLayout, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 256U, sizeof( VulkanUniforms ) ) );
		fFrameResources[i].fUserData = Rtt_NEW( NULL, BufferDescriptor( context, fPool, fUserDataLayout, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024U, sizeof( VulkanUserData ) ) );
		fFrameResources[i].fTextures = Rtt_NEW( NULL, TexturesDescriptor( context, fTextureLayout ) );
		fFrameResources[i].fCommands = context->MakeCommandPool( graphicsFamily );

		if (!fFrameResources[i].AddSynchronizationObjects( context->GetDevice(), context->GetAllocator() )) // TODO: could fail at other steps...
		{
			Rtt_TRACE_SIM(( "ERROR: Failed to create some synchronziation objects!" ));

			// for (j = 0; j <= i; ++j)
			fFrameResources[i].CleanUpCommandPool( context->GetDevice(), context->GetAllocator() );
			fFrameResources[i].CleanUpDescriptorObjects( context->GetDevice(), context->GetAllocator() );
			fFrameResources[i].CleanUpSynchronizationObjects( context->GetDevice(), context->GetAllocator() );

			// break...
		}
	}
}

VulkanRenderer::~VulkanRenderer()
{
	vkQueueWaitIdle( fContext->GetGraphicsQueue() );

	TearDownSwapchain();

	Rtt_DELETE( fSwapchainTexture );

	auto ci = GetContext()->GetCommonInfo();

	for (FrameResources & resources : fFrameResources)
	{
		resources.CleanUpCommandPool( ci.device, ci.allocator );
		resources.CleanUpDescriptorObjects( ci.device, ci.allocator );
		resources.CleanUpSynchronizationObjects( ci.device, ci.allocator );
	}

	WipeDescriptorPool( ci.device, fPool, ci.allocator );

	for (auto & pipeline : fBuiltPipelines)
	{
		vkDestroyPipeline( ci.device, pipeline.second, ci.allocator );
	}

	vkDestroyPipelineLayout( ci.device, fPipelineLayout, ci.allocator );
	vkDestroyDescriptorSetLayout( ci.device, fUniformsLayout, ci.allocator );
	vkDestroyDescriptorSetLayout( ci.device, fUserDataLayout, ci.allocator );
	vkDestroyDescriptorSetLayout( ci.device, fTextureLayout, ci.allocator );

	// ^^ might want more descriptor set layouts, e.g. for compute or to allow different kinds of buffer inputs
	// make vectors of both layout types
}

void
VulkanRenderer::BeginFrame( Real totalTime, Real deltaTime, const TimeTransform * defTimeTransform, Real contentScaleX, Real contentScaleY, bool isCapture )
{
	InitializePipelineState();

	VulkanCommandBuffer * vulkanCommandBuffer = static_cast< VulkanCommandBuffer * >( fBackCommandBuffer );
	VkResult result = vulkanCommandBuffer->GetExecuteResult();

	vulkanCommandBuffer->BeginFrame();

	bool canContinue = VK_SUCCESS == result;
	VkSwapchainKHR swapchain = !GetSwapchainInvalid() ? fContext->GetSwapchain() : VK_NULL_HANDLE;

	if (!isCapture && VK_NULL_HANDLE == swapchain)
	{
		VulkanContext::PopulateSwapchainDetails( *fContext );

	#ifdef _WIN32
		VkExtent2D extent = fContext->GetSwapchainDetails().fExtent;

		canContinue = extent.width > 0 && extent.height > 0; // not minimized?

		if (canContinue)
	#endif
		{
			swapchain = MakeSwapchain();

			if (GetSwapchainInvalid()) // out-of-date or suboptimal
			{
				TearDownSwapchain();
			}

			if (swapchain != VK_NULL_HANDLE)
			{
				BuildUpSwapchain( swapchain );

				fPrimaryFBO = Rtt_NEW( fAllocator, FrameBufferObject( fAllocator, fSwapchainTexture ) );

				SetSwapchainInvalid( false );
			}

			else
			{
				canContinue = false;
			}
		}
	}

	if (canContinue && vulkanCommandBuffer->Wait( fContext, &fFrameResources[fFrameIndex], !isCapture ? swapchain : VK_NULL_HANDLE ))
	{
		fFrameIndex = (fFrameIndex + 1) % kFramesInFlight;
	}

	if (!isCapture && fPrimaryFBO)
	{
		SetFrameBufferObject( fPrimaryFBO );
	}

	Super::BeginFrame( totalTime, deltaTime, defTimeTransform, contentScaleX, contentScaleY );

	vulkanCommandBuffer->ClearExecuteResult();
}

void
VulkanRenderer::EndFrame()
{
	Super::EndFrame();

	SetFrameBufferObject(NULL);
}

static void
BlitImage( VkCommandBuffer commandBuffer, VkImage srcImage, VkImage dstImage, S32 w_in_pixels, S32 h_in_pixels )
{
	VkOffset3D size;

	size.x = w_in_pixels;
	size.y = h_in_pixels;
	size.z = 1U;

	VkImageBlit blitRegion = {};

	blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.srcSubresource.layerCount = 1U;
	blitRegion.srcOffsets[1] = size;
	blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.dstSubresource.layerCount = 1U;
	blitRegion.dstOffsets[1] = size;

	vkCmdBlitImage( commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, &blitRegion, VK_FILTER_NEAREST );
}

static void
CopyImage( VkCommandBuffer commandBuffer, VkImage srcImage, VkImage dstImage, S32 w_in_pixels, S32 h_in_pixels )
{
	VkImageCopy copyRegion = {};

	copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.srcSubresource.layerCount = 1;
	copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.dstSubresource.layerCount = 1;
	copyRegion.extent.width = w_in_pixels;
	copyRegion.extent.height = h_in_pixels;
	copyRegion.extent.depth = 1U;

	vkCmdCopyImage( commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion );
}

static void
ReadAfterBlit( uint8_t * bits, uint8_t * dataBytes, S32 w_in_pixels, S32 h_in_pixels, VkDeviceSize rowPitch )
{
	for (U32 y = 0; y < h_in_pixels; ++y)
	{
		uint8_t * line = (uint8_t *)bits + y * w_in_pixels * 4U;

		memcpy( line, dataBytes, w_in_pixels * 4U );

		dataBytes += rowPitch;
	}
}

static void
ReadAfterCopy( uint8_t * bits, uint8_t * dataBytes, S32 w_in_pixels, S32 h_in_pixels, VkDeviceSize rowPitch )
{
	for (U32 y = 0; y < h_in_pixels; ++y)
	{
		uint8_t * line = (uint8_t *)bits + y * w_in_pixels * 4U;
		uint8_t * dataLine = dataBytes;

		for (U32 x = 0; x < w_in_pixels; ++x)
		{
			line[0] = dataLine[3];
			line[1] = dataLine[2];
			line[2] = dataLine[1];
			line[3] = dataLine[0];

			line += 4;
			dataLine += 4;
		}

		dataBytes += rowPitch;
	}
}

void
VulkanRenderer::CaptureFrameBuffer( RenderingStream & stream, BufferBitmap & bitmap, S32 x_in_pixels, S32 y_in_pixels, S32 w_in_pixels, S32 h_in_pixels )
{
	// originally adapted from https://community.khronos.org/t/readpixels-on-vulkan/6797
	// revised to follow https://github.com/SaschaWillems/Vulkan/blob/master/examples/screenshot/screenshot.cpp

	VulkanContext * context = GetContext();

	// n.b. currently bitmap.Width(), bitmap.Height() are always w_in_pixels, h_in_pixels

	VulkanTexture::ImageData imageData = VulkanTexture::CreateImage(
		context,
		w_in_pixels, h_in_pixels,
		1U,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_LINEAR,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	void * data = NULL;
	VkDevice device = context->GetDevice();
	VkResult errorCode = vkMapMemory( device, imageData.fMemory, 0, VK_WHOLE_SIZE, 0, (void **)&data );
	
	context->WaitOnFence( fCaptureFence );

	VulkanTexture * texture = static_cast< VulkanTexture * >( fCaptureFBO->GetTextureName() );
	VkImage image = texture->GetImage();
	VkCommandBuffer commandBuffer = context->BeginSingleTimeCommands();

	VulkanTexture::TransitionImageLayout( context, image, texture->GetFormat(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1U, commandBuffer );
    VulkanTexture::TransitionImageLayout( context, imageData.fImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, commandBuffer );
	
	bool supportsBlit = context->GetFeatures().supportsBlit;

	if (supportsBlit)
	{
		BlitImage( commandBuffer, image, imageData.fImage, w_in_pixels, h_in_pixels );
	}

	else
	{
		CopyImage( commandBuffer, image, imageData.fImage, w_in_pixels, h_in_pixels );
	}

	VulkanTexture::TransitionImageLayout( context, image, texture->GetFormat(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1U, commandBuffer );
	VulkanTexture::TransitionImageLayout( context, imageData.fImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, 1U, commandBuffer );

	context->EndSingleTimeCommands( commandBuffer );

	VkImageSubresource subResource = {};
	
	subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	VkSubresourceLayout subResourceLayout;

	vkGetImageSubresourceLayout( device, imageData.fImage, &subResource, &subResourceLayout );

	uint8_t * bits = (uint8_t *)bitmap.WriteAccess();
	uint8_t * dataBytes = (uint8_t *)data + subResourceLayout.offset;

	if (supportsBlit)
	{
		ReadAfterBlit( bits, dataBytes, bitmap.Width(), bitmap.Height(), subResourceLayout.rowPitch );
	}

	else
	{
		ReadAfterCopy( bits, dataBytes, bitmap.Width(), bitmap.Height(), subResourceLayout.rowPitch );
	}

	vkUnmapMemory( device, imageData.fMemory );

	imageData.Destroy( device, context->GetAllocator() );

	// TODO: do we need to guard the FBO memory?

	PrepareCapture( NULL, VK_NULL_HANDLE );
}

void
VulkanRenderer::EndCapture()
{
	if (VK_NULL_HANDLE != fCaptureFence) // TODO: is this just fFrameResources[(fFrameIndex - 1) % 3]?
	{
		fContext->WaitOnFence( fCaptureFence );
	}

	PrepareCapture( NULL, VK_NULL_HANDLE );
}

VkSwapchainKHR
VulkanRenderer::MakeSwapchain()
{
    const VulkanContext::SwapchainDetails & details = fContext->GetSwapchainDetails();
    auto & queueFamilies = fContext->GetQueueFamilies();
	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};

	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.imageArrayLayers = 1U;
	swapchainCreateInfo.imageColorSpace = details.fFormat.colorSpace;
	swapchainCreateInfo.imageExtent = details.fExtent;
	swapchainCreateInfo.imageFormat = details.fFormat.format;
	swapchainCreateInfo.imageSharingMode = 1U == queueFamilies.size() ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.minImageCount = details.fImageCount;
	swapchainCreateInfo.oldSwapchain = fContext->GetSwapchain();
	swapchainCreateInfo.presentMode = details.fPresentMode;
	swapchainCreateInfo.preTransform = details.fTransformFlagBits; // TODO: relevant to portrait, landscape, etc?
	swapchainCreateInfo.surface = fContext->GetSurface();

	if (VK_SHARING_MODE_CONCURRENT == swapchainCreateInfo.imageSharingMode)
	{
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilies.data();
		swapchainCreateInfo.queueFamilyIndexCount = queueFamilies.size();
	}

	VkSwapchainKHR swapchain = VK_NULL_HANDLE;

	if (VK_SUCCESS == vkCreateSwapchainKHR( fContext->GetDevice(), &swapchainCreateInfo, fContext->GetAllocator(), &swapchain ))
	{
		return swapchain;
	}

	else
	{
		Rtt_TRACE_SIM(( "ERROR: Failed to create swap chain!" ));

		return VK_NULL_HANDLE;
	}
}

void
VulkanRenderer::BuildUpSwapchain( VkSwapchainKHR swapchain )
{
	fContext->SetSwapchain( swapchain );

	uint32_t imageCount;

	vkGetSwapchainImagesKHR( fContext->GetDevice(), swapchain, &imageCount, NULL );
	
	fSwapchainImages.resize( imageCount );

	vkGetSwapchainImagesKHR( fContext->GetDevice(), swapchain, &imageCount, fSwapchainImages.data() );
}

void
VulkanRenderer::TearDownSwapchain()
{
    auto ci = fContext->GetCommonInfo();

	vkDestroySwapchainKHR( ci.device, fContext->GetSwapchain(), ci.allocator );

	for (FrameResources & resources : fFrameResources)
	{
		vkResetCommandPool( ci.device, resources.fCommands, 0U );
	}

    fContext->SetSwapchain( VK_NULL_HANDLE );

	Rtt_DELETE( fPrimaryFBO );

	fPrimaryFBO = NULL;
}

const size_t kFinalBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
const size_t kFinalBlendOp = VK_BLEND_OP_MAX; // n.b. this is the max() operation; its use in this way is a coincidence

constexpr int BitsNeeded( int x ) // n.b. x > 0
{
	int result = 0;

	for (int power = 1; power <= x; power *= 2)
	{
		++result;
	}

	return result;
}

struct PackedBlendAttachment {
	U32 fEnable : 1;
	U32 fSrcColorFactor : BitsNeeded( kFinalBlendFactor );
	U32 fDstColorFactor : BitsNeeded( kFinalBlendFactor );
	U32 fColorOp : BitsNeeded( kFinalBlendOp );
	U32 fSrcAlphaFactor : BitsNeeded( kFinalBlendFactor );
	U32 fDstAlphaFactor : BitsNeeded( kFinalBlendFactor );
	U32 fAlphaOp : BitsNeeded( kFinalBlendOp );
	U32 fColorWriteMask : 4;
};

const size_t kFinalDynamicState = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
const size_t kDynamicStateCountRoundedUp = (kFinalDynamicState + 7U) & ~7U;
const size_t kDynamicStateByteCount = kDynamicStateCountRoundedUp / 8U;

const size_t kFinalCompareOp = VK_COMPARE_OP_ALWAYS;
const size_t kFinalFrontFace = VK_FRONT_FACE_CLOCKWISE;
const size_t kFinalLogicOp = VK_LOGIC_OP_SET;
const size_t kFinalPolygonMode = VK_POLYGON_MODE_POINT;
const size_t kFinalPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
const size_t kFinalStencilOp = VK_STENCIL_OP_DECREMENT_AND_WRAP;

struct PackedPipeline {
    U64 fTopology : BitsNeeded( kFinalPrimitiveTopology );
    U64 fPrimitiveRestartEnable : 1;
	U64 fRasterizerDiscardEnable : 1;
	U64 fPolygonMode : BitsNeeded( kFinalPolygonMode );
	U64 fLineWidth : 4; // lineWidth = (X + 1) / 16
	U64 fCullMode : 2;
	U64 fFrontFace : BitsNeeded( kFinalFrontFace );
	U64 fRasterSamplesFlags : 7;
	U64 fSampleShadingEnable : 1;
	U64 fSampleShading : 5; // minSampleShading = X / 32
	U64 fAlphaToCoverageEnable : 1;
	U64 fAlphaToOneEnable : 1;
	U64 fDepthTestEnable : 1;
	U64 fDepthWriteEnable : 1;
	U64 fDepthCompareOp : BitsNeeded( kFinalCompareOp );
	U64 fDepthBoundsTestEnable : 1;
	U64 fStencilTestEnable : 1;
	U64 fFront : BitsNeeded( kFinalStencilOp );
	U64 fBack : BitsNeeded( kFinalStencilOp );
	U64 fMinDepthBounds : 5; // minDepthBounds = X / 32
	U64 fMaxDepthBounds : 5; // maxDepthBounds = (X + 1) / 32
	U64 fLogicOpEnable : 1;
	U64 fLogicOp : BitsNeeded( kFinalLogicOp );
	U64 fBlendConstant1 : 4; // blendConstants = X / 15
	U64 fBlendConstant2 : 4;
	U64 fBlendConstant3 : 4;
	U64 fBlendConstant4 : 4;
	U64 fLayoutID : 4;
	U64 fRenderPassID : 4;
	U64 fShaderID : 16;
	U64 fAttributeDescriptionID : 3;
	U64 fBindingDescriptionID : 3;
	U64 fBlendAttachmentCount : 3; // 0-7
	PackedBlendAttachment fBlendAttachments[8];
	uint8_t fDynamicStates[kDynamicStateByteCount];
};

static PackedPipeline &
GetPackedPipeline( std::vector< U64 > & contents )
{
    return *reinterpret_cast< PackedPipeline * >( contents.data() );
}

void
VulkanRenderer::EnableBlend( bool enabled )
{
    fPipelineCreateInfo.fColorBlendAttachments.front().blendEnable = enabled ? VK_TRUE : VK_FALSE;

	GetPackedPipeline( fWorkingKey.fContents ).fBlendAttachments[0].fEnable = enabled;
}

void
VulkanRenderer::SetAttributeDescriptions( U32 id, const std::vector< VkVertexInputAttributeDescription > & descriptions )
{
	fPipelineCreateInfo.fVertexAttributeDescriptions = descriptions;

	GetPackedPipeline( fWorkingKey.fContents ).fAttributeDescriptionID = id;
}

void
VulkanRenderer::SetBindingDescriptions( U32 id, const std::vector< VkVertexInputBindingDescription > & descriptions )
{
	fPipelineCreateInfo.fVertexBindingDescriptions = descriptions;

	GetPackedPipeline( fWorkingKey.fContents ).fBindingDescriptionID = id;
}

void
VulkanRenderer::SetBlendEquations( VkBlendOp color, VkBlendOp alpha )
{
	VkPipelineColorBlendAttachmentState & attachment = fPipelineCreateInfo.fColorBlendAttachments.front();

    attachment.alphaBlendOp = alpha;
	attachment.colorBlendOp = color;
    	
    PackedPipeline & packedPipeline = GetPackedPipeline( fWorkingKey.fContents );

	packedPipeline.fBlendAttachments[0].fAlphaOp = alpha;
	packedPipeline.fBlendAttachments[0].fColorOp = color;
}

void
VulkanRenderer::SetBlendFactors( VkBlendFactor srcColor, VkBlendFactor srcAlpha, VkBlendFactor dstColor, VkBlendFactor dstAlpha )
{
	VkPipelineColorBlendAttachmentState & attachment = fPipelineCreateInfo.fColorBlendAttachments.front();

	attachment.srcColorBlendFactor = srcColor;
	attachment.dstColorBlendFactor = dstColor;
	attachment.srcAlphaBlendFactor = srcAlpha;
	attachment.dstAlphaBlendFactor = dstAlpha;
    	
    PackedPipeline & packedPipeline = GetPackedPipeline( fWorkingKey.fContents );

	packedPipeline.fBlendAttachments[0].fSrcColorFactor = srcColor;
	packedPipeline.fBlendAttachments[0].fDstColorFactor = dstColor;
	packedPipeline.fBlendAttachments[0].fSrcAlphaFactor = srcAlpha;
	packedPipeline.fBlendAttachments[0].fDstAlphaFactor = dstAlpha;
}

void
VulkanRenderer::SetMultisample( VkSampleCountFlagBits sampleCount )
{
	fPipelineCreateInfo.fMultisample.rasterizationSamples = sampleCount;
    	
    PackedPipeline & packedPipeline = GetPackedPipeline( fWorkingKey.fContents );

	packedPipeline.fRasterSamplesFlags = sampleCount;
}

void
VulkanRenderer::SetPrimitiveTopology( VkPrimitiveTopology topology )
{
    fPipelineCreateInfo.fInputAssembly.topology = topology;

	GetPackedPipeline( fWorkingKey.fContents ).fTopology = topology;
}

void
VulkanRenderer::SetRenderPass( U32 id, VkRenderPass renderPass )
{
	fPipelineCreateInfo.fRenderPass = renderPass;

	GetPackedPipeline( fWorkingKey.fContents ).fRenderPassID = id;
}

void
VulkanRenderer::SetShaderStages( U32 id, const std::vector< VkPipelineShaderStageCreateInfo > & stages )
{
	fPipelineCreateInfo.fShaderStages = stages;

	GetPackedPipeline( fWorkingKey.fContents ).fShaderID = id;
}

static uint8_t &
WithDynamicByte( uint8_t states[], uint8_t value, uint8_t & bit )
{
	uint8_t byteIndex = value / 8U;

	bit = 1U << (value - byteIndex * 8U);

	return states[byteIndex];
}

static void
SetDynamicStateBit( uint8_t states[], uint8_t value )
{
	uint8_t bit, & byte = WithDynamicByte( states, value, bit );

	byte |= bit;
}

static bool
IsDynamicBitSet( uint8_t states[], uint8_t value )
{
	uint8_t bit, & byte = WithDynamicByte( states, value, bit );

	return !!(byte & bit);
}

VkPipeline
VulkanRenderer::ResolvePipeline()
{
	PackedPipeline & packedPipeline = GetPackedPipeline( fWorkingKey.fContents );

	if (VulkanProgram::kInvalidID == packedPipeline.fShaderID)
	{
		return VK_NULL_HANDLE;
	}

	auto iter = fBuiltPipelines.find( fWorkingKey );
	VkPipeline pipeline = VK_NULL_HANDLE;

	if (iter == fBuiltPipelines.end())
	{
		std::vector< VkDynamicState > dynamicStates;

		for (uint8_t i = 0; i < kFinalDynamicState; ++i)
		{
			if (IsDynamicBitSet( packedPipeline.fDynamicStates, i ))
			{
				dynamicStates.push_back( VkDynamicState( i ) );
			}
		}

		VkPipelineDynamicStateCreateInfo createDynamicStateInfo = {};

		createDynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		createDynamicStateInfo.dynamicStateCount = dynamicStates.size();
		createDynamicStateInfo.pDynamicStates = dynamicStates.data();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};

        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.pVertexAttributeDescriptions = fPipelineCreateInfo.fVertexAttributeDescriptions.data();
        vertexInputInfo.pVertexBindingDescriptions = fPipelineCreateInfo.fVertexBindingDescriptions.data();
        vertexInputInfo.vertexAttributeDescriptionCount = fPipelineCreateInfo.fVertexAttributeDescriptions.size();
        vertexInputInfo.vertexBindingDescriptionCount = fPipelineCreateInfo.fVertexBindingDescriptions.size();

		VkPipelineViewportStateCreateInfo viewportInfo = {};

		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInfo.scissorCount = 1U;
		viewportInfo.viewportCount = 1U;

		fPipelineCreateInfo.fColorBlend.attachmentCount = fPipelineCreateInfo.fColorBlendAttachments.size();
		fPipelineCreateInfo.fColorBlend.pAttachments = fPipelineCreateInfo.fColorBlendAttachments.data();

        VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};

        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.basePipelineHandle = fFirstPipeline;
		pipelineCreateInfo.basePipelineIndex = -1;
		pipelineCreateInfo.flags = fFirstPipeline != VK_NULL_HANDLE ? VK_PIPELINE_CREATE_DERIVATIVE_BIT : VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
		pipelineCreateInfo.layout = fPipelineLayout;
        pipelineCreateInfo.pColorBlendState = &fPipelineCreateInfo.fColorBlend;
        pipelineCreateInfo.pDepthStencilState = &fPipelineCreateInfo.fDepthStencil;
		pipelineCreateInfo.pDynamicState = &createDynamicStateInfo;
        pipelineCreateInfo.pInputAssemblyState = &fPipelineCreateInfo.fInputAssembly;
        pipelineCreateInfo.pMultisampleState = &fPipelineCreateInfo.fMultisample;
        pipelineCreateInfo.pRasterizationState = &fPipelineCreateInfo.fRasterization;
        pipelineCreateInfo.pStages = fPipelineCreateInfo.fShaderStages.data();
        pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
		pipelineCreateInfo.pViewportState = &viewportInfo;
		pipelineCreateInfo.renderPass = fPipelineCreateInfo.fRenderPass;
        pipelineCreateInfo.stageCount = fPipelineCreateInfo.fShaderStages.size();

		auto ci = fContext->GetCommonInfo();
// TODO: can we use another queue to actually build this while still recording commands?
        if (VK_SUCCESS == vkCreateGraphicsPipelines( ci.device, fContext->GetPipelineCache(), 1U, &pipelineCreateInfo, ci.allocator, &pipeline ))
		{
			if (VK_NULL_HANDLE == fFirstPipeline)
			{
				fFirstPipeline = pipeline;
			}

			fBuiltPipelines[fWorkingKey] = pipeline;
		}

		else
		{
			Rtt_TRACE_SIM(( "ERROR: Failed to create pipeline!" ));
        }
	}

	else
	{
		pipeline = iter->second;
	}

	return pipeline;
}

void
VulkanRenderer::ResetPipelineInfo()
{
	fWorkingKey = fDefaultKey;

	RestartWorkingPipeline();
}

void
VulkanRenderer::ForceInvalidation()
{
	fInvalidate( fDisplay );
}
		
void
VulkanRenderer::PrepareCapture( VulkanFrameBufferObject * fbo, VkFence fence )
{
	fCaptureFBO = fbo;
	fCaptureFence = fence;
}

GPUResource* 
VulkanRenderer::Create( const CPUResource* resource )
{
	switch( resource->GetType() )
	{
		case CPUResource::kFrameBufferObject: return new VulkanFrameBufferObject( *this );
		case CPUResource::kGeometry: return new VulkanGeometry( fContext );
		case CPUResource::kProgram: return new VulkanProgram( fContext );
		case CPUResource::kTexture: return new VulkanTexture( fContext );
		case CPUResource::kUniform: return NULL;
		default: Rtt_ASSERT_NOT_REACHED(); return NULL;
	}
}

void
VulkanRenderer::InitializePipelineState()
{
	PackedPipeline packedPipeline = {};

	// TODO: this should be fleshed out with defaults
	// these need not be relevant, but should properly handle being manually updated...

	packedPipeline.fRasterSamplesFlags = VK_SAMPLE_COUNT_1_BIT;
	packedPipeline.fBlendAttachmentCount = 1U;
	packedPipeline.fBlendAttachments[0].fEnable = VK_TRUE;
	packedPipeline.fBlendAttachments[0].fColorWriteMask = 0xF;
	packedPipeline.fBlendAttachments[0].fSrcColorFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	packedPipeline.fBlendAttachments[0].fSrcAlphaFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	packedPipeline.fBlendAttachments[0].fDstColorFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	packedPipeline.fBlendAttachments[0].fDstAlphaFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

	SetDynamicStateBit( packedPipeline.fDynamicStates, VK_DYNAMIC_STATE_SCISSOR );
	SetDynamicStateBit( packedPipeline.fDynamicStates, VK_DYNAMIC_STATE_VIEWPORT );

	memcpy( fDefaultKey.fContents.data(), &packedPipeline, sizeof( PackedPipeline ) );

	ResetPipelineInfo();

	fColorBlendState = fPipelineCreateInfo.fColorBlendAttachments[0];
}

void
VulkanRenderer::RestartWorkingPipeline()
{
    new (&fPipelineCreateInfo) PipelineCreateInfo;
}

VulkanRenderer::PipelineCreateInfo::PipelineCreateInfo()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

	fInputAssembly = inputAssembly;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};

	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.lineWidth = 1.0f;

	fRasterization = rasterizer;

	VkPipelineMultisampleStateCreateInfo multisampling = {};

	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.rasterizationSamples = VkSampleCountFlagBits( VK_SAMPLE_COUNT_1_BIT );

    fMultisample = multisampling;

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};

	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

	fDepthStencil = depthStencil;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};

	colorBlendAttachment.srcAlphaBlendFactor = colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstAlphaBlendFactor = colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	fColorBlendAttachments.clear();
	fColorBlendAttachments.push_back( colorBlendAttachment );

	VkPipelineColorBlendStateCreateInfo colorBlending = {};

	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	fColorBlend = colorBlending;

	fRenderPass = VK_NULL_HANDLE;
}

const size_t kByteCountRoundedUp = (sizeof( PackedPipeline ) + 7U) & ~7U;
const size_t kU64Count = kByteCountRoundedUp / 8U;

VulkanRenderer::PipelineKey::PipelineKey()
:   fContents( kU64Count, U64{} )
{
}

bool
VulkanRenderer::PipelineKey::operator < ( const PipelineKey & other ) const
{
	return fContents < other.fContents;
}

bool
VulkanRenderer::PipelineKey::operator == ( const PipelineKey & other ) const
{
	return fContents == other.fContents;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
