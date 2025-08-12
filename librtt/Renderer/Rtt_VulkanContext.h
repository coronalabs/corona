//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_VulkanContext_H__
#define _Rtt_VulkanContext_H__

#include "Renderer/Rtt_VulkanIncludes.h"
#include "Core/Rtt_Types.h"

#include <map>
#include <vector>

// ----------------------------------------------------------------------------

struct shaderc_compiler;
struct shaderc_compile_options;

namespace Rtt
{

struct VulkanSurfaceParams;

// ----------------------------------------------------------------------------

class RenderPassKey {
	public:
		void SetContents( std::vector< U8 > & contents );

		bool operator == ( const RenderPassKey & other ) const;
		bool operator < ( const RenderPassKey & other ) const;

	private:
		std::vector< U8 > fContents;
};

struct RenderPassData {
	U32 fID;
	VkRenderPass fPass;
};

class VulkanBufferData {
public:
	VulkanBufferData( VkDevice device, const VkAllocationCallbacks * allocator );
	~VulkanBufferData();

	VkDevice GetDevice() const { return fDevice; }
	VkBuffer GetBuffer() const { return fBuffer; }
	VkDeviceMemory GetMemory() const { return fMemory; }

	VulkanBufferData * Extract( Rtt_Allocator * allocator );

	void Clear();
	void Disown();

private:
	const VkAllocationCallbacks * fAllocator;
	VkDevice fDevice;
	VkBuffer fBuffer;
	VkDeviceMemory fMemory;

	friend class VulkanContext;
};

class VulkanContext
{
	public:
		typedef VulkanContext Self;

	public:
		VulkanContext();
		~VulkanContext();

	public:
		struct Features {
			bool samplerAnisotropy;
			bool shaderSampledImageArrayDynamicIndexing;
			bool supportsBlit;
		};

	public:
		const VkAllocationCallbacks * GetAllocator() const { return fAllocator; }
		VkInstance GetInstance() const { return fInstance; }
		VkDevice GetDevice() const { return fDevice; }
		VkPhysicalDevice GetPhysicalDevice() const { return fPhysicalDevice; }
		VkQueue GetGraphicsQueue() const { return fGraphicsQueue; }
		VkQueue GetPresentQueue() const { return fPresentQueue; }
		VkCommandPool GetSingleTimeCommandsPool() const { return fSingleTimeCommandsPool; }
		VkSurfaceKHR GetSurface() const { return fSurface; }
		VkPipelineCache GetPipelineCache() const { return fPipelineCache; }
		VkSampleCountFlagBits GetSampleCountFlags() const { return fSampleCountFlags; }
		VkSwapchainKHR GetSwapchain() const { return fSwapchain; }
		const std::vector< uint32_t > & GetQueueFamilies() const { return fQueueFamilies; }
		const Features & GetFeatures() const { return fDeviceDetails.features; }
		const VkPhysicalDeviceProperties & GetProperties() const { return fDeviceDetails.properties; }
		uint32_t GetGraphicsFamilyIndex() const { return 0U; }
		// TODO: GetComputeFamilyIndex()...

		struct CommonInfo {
			CommonInfo( VulkanContext * ctx )
			:	context( ctx ),
				device( ctx->GetDevice() ),
				allocator( ctx->GetAllocator() )
			{
			}

			VulkanContext * context;
			VkDevice device;
			const VkAllocationCallbacks * allocator;
		};

		CommonInfo GetCommonInfo() { return CommonInfo( this ); }

		void SetSwapchain( VkSwapchainKHR swapchain ) { fSwapchain = swapchain; }

		shaderc_compiler * GetCompiler() const { return fCompiler; }
		shaderc_compile_options * GetCompileOptions() const { return fCompileOptions; }

	public:
		const RenderPassData * AddRenderPass( const RenderPassKey & key, VkRenderPass renderPass );
		const RenderPassData * FindRenderPassData( const RenderPassKey & key ) const;
	public:
		bool CreateBuffer( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VulkanBufferData & bufferData );
		bool FindMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t & type );
	
	public:
	    VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands( VkCommandBuffer commandBuffer );
		void CopyBuffer( VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size );
		void * MapData( VkDeviceMemory memory, VkDeviceSize count = VK_WHOLE_SIZE, VkDeviceSize offset = 0U );
		void StageData( VkDeviceMemory stagingMemory, const void * data, VkDeviceSize count, VkDeviceSize offset = 0U );

	public:
		void WaitOnFence( VkFence fence );

	public:
		struct DeviceDetails {
			Features features;
			VkPhysicalDeviceProperties properties;
		};

		const DeviceDetails & GetDeviceDetails() const { return fDeviceDetails; }

	public:
		void PrepareCompiler();
		VkCommandPool MakeCommandPool( uint32_t queueFamily, bool resetCommandBuffer = false );

		static bool PopulateMultisampleDetails( VulkanContext & context );
		static bool PopulatePreSwapchainDetails( VulkanContext & context, const VulkanSurfaceParams & params );
		static bool PopulateSwapchainDetails( VulkanContext & context );
		static void UpdateSwapchainDetails( VulkanContext & context );

		static bool VolkInitialize();
		static void VolkLoadInstance( VkInstance instance );
		static void VolkLoadDevice( VkDevice device );

		struct SwapchainDetails {
			VkSurfaceTransformFlagBitsKHR fTransformFlagBits;
			VkExtent2D fExtent;
			VkSurfaceFormatKHR fFormat;
			uint32_t fImageCount;
			VkPresentModeKHR fPresentMode;
		};

		const SwapchainDetails & GetSwapchainDetails() const { return fSwapchainDetails; }

	private:
		VkAllocationCallbacks * fAllocator;
		VkInstance fInstance;
	#ifdef Rtt_DEBUG
		VkDebugUtilsMessengerEXT fDebugMessenger;
	#endif
		VkDevice fDevice;
		VkPhysicalDevice fPhysicalDevice;
		VkQueue fGraphicsQueue;
		VkQueue fPresentQueue;
		VkCommandPool fSingleTimeCommandsPool;
		VkSurfaceKHR fSurface;
		VkPipelineCache fPipelineCache;
		VkSampleCountFlagBits fSampleCountFlags;
		VkSwapchainKHR fSwapchain;
		SwapchainDetails fSwapchainDetails;
		DeviceDetails fDeviceDetails;
		std::vector< uint32_t > fQueueFamilies;
		std::map< RenderPassKey, RenderPassData > fRenderPasses;
		shaderc_compiler * fCompiler;
		shaderc_compile_options * fCompileOptions;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_VulkanContext_H__
