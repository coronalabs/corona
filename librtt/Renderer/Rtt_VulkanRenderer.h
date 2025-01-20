//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_VulkanRenderer_H__
#define _Rtt_VulkanRenderer_H__

#include "Renderer/Rtt_Renderer.h"
#include "Renderer/Rtt_VulkanIncludes.h"

#include <map>
#include <vector>

// ----------------------------------------------------------------------------

struct Rtt_Allocator;

namespace Rtt
{

class GPUResource;
class CPUResource;
class FrameBufferObject;
class Texture;
class VulkanBufferData;
class VulkanFrameBufferObject;
class VulkanContext;

// ----------------------------------------------------------------------------

struct Descriptor {
	enum Index { kUniforms, kUserData, kTexture };

	Descriptor( VkDescriptorSetLayout setLayout );

	virtual void Reset( VkDevice device ) = 0;
	virtual void Wipe( VkDevice device, const VkAllocationCallbacks * allocator ) = 0;

	static bool IsMaskPushConstant( int index );
	static bool IsPushConstant( int index, bool userDataPushConstants );
	static bool IsUserData( int index );

	VkDescriptorSetLayout fSetLayout;
	U32 fDirty;
};

struct BufferData {
	BufferData();

	void Wipe();

	VkDescriptorSet fSet;
	VulkanBufferData * fData;
	void * fMapped;
};

struct BufferDescriptor : public Descriptor {
	BufferDescriptor( VulkanContext * context, VkDescriptorPool pool, VkDescriptorSetLayout setLayout, VkDescriptorType type, size_t count, size_t size );

	void AllowMark() { fMarkWritten = true; }
	void ResetMark() { fWritten = 0U; }
	void TryToMark()
	{
		if (fMarkWritten)
		{
			fWritten |= fDirty;
		}
	}

	virtual void Reset( VkDevice device );
	virtual void Wipe( VkDevice device, const VkAllocationCallbacks * allocator );

	void SetWorkspace( void * workspace );
	void TryToAddMemory( std::vector< VkMappedMemoryRange > & ranges, VkDescriptorSet sets[], size_t & count );
	void TryToAddDynamicOffset( uint32_t offsets[], size_t & count );

	std::vector< BufferData > fBuffers;
	VkDescriptorSet fLastSet;
	VkDescriptorType fType;
	VkDeviceSize fDynamicAlignment;
	U8 * fWorkspace;
	U32 fIndex;
	U32 fOffset;
	U32 fLastOffset;
	size_t fAtomSize;
	size_t fBufferSize;
	size_t fRawSize;
	size_t fNonCoherentRawSize;
	U32 fWritten;
	bool fMarkWritten;
};

struct TexturesDescriptor : public Descriptor {
	TexturesDescriptor( VulkanContext * context, VkDescriptorSetLayout setLayout );

	virtual void Reset( VkDevice device );
	virtual void Wipe( VkDevice device, const VkAllocationCallbacks * allocator );

	VkDescriptorPool fPool;
};

struct FrameResources {
	FrameResources();

	bool AddSynchronizationObjects( VkDevice device, const VkAllocationCallbacks * allocator );
	void CleanUpCommandPool( VkDevice device, const VkAllocationCallbacks * allocator );
	void CleanUpDescriptorObjects( VkDevice device, const VkAllocationCallbacks * allocator );
	void CleanUpSynchronizationObjects( VkDevice device, const VkAllocationCallbacks * allocator );

	BufferDescriptor * fUniforms;
	BufferDescriptor * fUserData;
	TexturesDescriptor * fTextures;
	VkCommandPool fCommands;
	VkSemaphore fImageAvailable;
	VkSemaphore fRenderFinished;
	VkFence fFence;
};

class VulkanRenderer : public Renderer
{
	public:
		typedef Renderer Super;
		typedef VulkanRenderer Self;

		enum { kFramesInFlight = 3 }; // see https://software.intel.com/content/www/us/en/develop/articles/practical-approach-to-vulkan-part-1.html

	public:
		VulkanRenderer( Rtt_Allocator* allocator, VulkanContext * context, void (*invalidate)(void *), void * display );
		virtual ~VulkanRenderer();

		virtual void BeginFrame( Real totalTime, Real deltaTime, const TimeTransform * defTimeTransform, Real contentScaleX, Real contentScaleY, bool isCapture );
		virtual void EndFrame();
		virtual void CaptureFrameBuffer( RenderingStream & stream, BufferBitmap & bitmap, S32 x_in_pixels, S32 y_in_pixels, S32 w_in_pixels, S32 h_in_pixels );
		virtual void EndCapture();

	public:
		VkSwapchainKHR MakeSwapchain();

		void BuildUpSwapchain( VkSwapchainKHR swapchain );
		void TearDownSwapchain();

	public:
		VulkanContext * GetContext() const { return fContext; }
		VkDescriptorSetLayout GetUniformsLayout() const { return fUniformsLayout; }
		VkDescriptorSetLayout GetUserDataLayout() const { return fUserDataLayout; }
		VkDescriptorSetLayout GetTextureLayout() const { return fTextureLayout; }
		VkPipelineLayout GetPipelineLayout() const { return fPipelineLayout; }
		bool GetSwapchainInvalid() const { return fSwapchainInvalid; }

		const std::vector< VkImage > & GetSwapchainImages() const { return fSwapchainImages; }

		void SetSwapchainInvalid( bool newValue ) { fSwapchainInvalid = newValue; }

	public:
		void EnableBlend( bool enabled );
		void SetAttributeDescriptions( U32 id, const std::vector< VkVertexInputAttributeDescription > & descriptions );
		void SetBindingDescriptions( U32 id, const std::vector< VkVertexInputBindingDescription > & descriptions );
		void SetBlendEquations( VkBlendOp color, VkBlendOp alpha );
		void SetBlendFactors( VkBlendFactor srcColor, VkBlendFactor srcAlpha, VkBlendFactor dstColor, VkBlendFactor dstAlpha );
		void SetMultisample( VkSampleCountFlagBits sampleCount );
		void SetPrimitiveTopology( VkPrimitiveTopology topology );
		void SetRenderPass( U32 id, VkRenderPass renderPass );
		void SetShaderStages( U32 id, const std::vector< VkPipelineShaderStageCreateInfo > & stages );

		VkPipeline ResolvePipeline();

		void ResetPipelineInfo();
		VkPipelineColorBlendAttachmentState & GetColorBlendState() { return fColorBlendState; }

	public:
		void ForceInvalidation();
		void PrepareCapture( VulkanFrameBufferObject * fbo, VkFence fence );

	protected:
		virtual GPUResource* Create( const CPUResource* resource );
		
	private:
		void InitializePipelineState();
		void RestartWorkingPipeline();

	private:
		struct PipelineCreateInfo {
			PipelineCreateInfo();

			std::vector< VkPipelineColorBlendAttachmentState > fColorBlendAttachments;
			std::vector< VkPipelineShaderStageCreateInfo > fShaderStages;
			std::vector< VkVertexInputAttributeDescription > fVertexAttributeDescriptions;
			std::vector< VkVertexInputBindingDescription > fVertexBindingDescriptions;
			VkPipelineInputAssemblyStateCreateInfo fInputAssembly;
			VkPipelineRasterizationStateCreateInfo fRasterization;
			VkPipelineMultisampleStateCreateInfo fMultisample;
			VkPipelineDepthStencilStateCreateInfo fDepthStencil;
			VkPipelineColorBlendStateCreateInfo fColorBlend;
			VkRenderPass fRenderPass;
		};

		struct PipelineKey {
			PipelineKey();

			std::vector< U64 > fContents;

			bool operator < ( const PipelineKey & other ) const;
			bool operator == ( const PipelineKey & other ) const;
		};

		VulkanContext * fContext;
		Texture * fSwapchainTexture;
		FrameBufferObject * fPrimaryFBO;
		VulkanFrameBufferObject * fCaptureFBO; // not owned by renderer
		std::vector< VkImage > fSwapchainImages;
		std::map< PipelineKey, VkPipeline > fBuiltPipelines;
		FrameResources fFrameResources[kFramesInFlight];
		VkPipeline fFirstPipeline;
		VkDescriptorPool fPool;
		VkDescriptorSetLayout fUniformsLayout;
		VkDescriptorSetLayout fUserDataLayout;
		VkDescriptorSetLayout fTextureLayout;
		VkPipelineLayout fPipelineLayout;
		VkFence fCaptureFence; // not owned by renderer
		PipelineCreateInfo fPipelineCreateInfo;
		PipelineKey fDefaultKey;
		PipelineKey fWorkingKey;
		VkPipelineColorBlendAttachmentState fColorBlendState;
		void (*fInvalidate)(void *);
		void * fDisplay;
		int fFrameIndex;
		bool fSwapchainInvalid;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_VulkanRenderer_H__
