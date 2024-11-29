//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_VulkanCommandBuffer_H__
#define _Rtt_VulkanCommandBuffer_H__

#include "Renderer/Rtt_CommandBuffer.h"
#include "Renderer/Rtt_Uniform.h"
#include "Renderer/Rtt_VulkanIncludes.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class VulkanFrameBufferObject;
class VulkanProgram;
class VulkanRenderer;
class VulkanContext;
struct Descriptor;
struct BufferDescriptor;
struct FrameResources;
struct TimeTransform;

// cf. shell_default_vulkan:

struct VulkanUniforms
{
	float fData[6 * 4];
};

struct VulkanUserData
{
	float UserData[4][16];
};

struct VulkanPushConstants
{
	float fData[6 * 4];	// masks, time, sampler index, texel size
	float fUniforms[10 * 4];// uniform userdata (compact representation, i.e. <= 2 or 10 vectors)
};

// 
class VulkanCommandBuffer : public CommandBuffer
{
	public:
		typedef CommandBuffer Super;
		typedef VulkanCommandBuffer Self;

	public:
		virtual bool HasFramebufferBlit( bool * canScale ) const { Rtt_ASSERT_NOT_IMPLEMENTED(); return false; }
		virtual void GetVertexAttributes( VertexAttributeSupport & support ) const { Rtt_ASSERT_NOT_IMPLEMENTED(); }

	public:
		VulkanCommandBuffer( Rtt_Allocator* allocator, VulkanRenderer & renderer );
		virtual ~VulkanCommandBuffer();

		virtual void Initialize();
		virtual void Denitialize();
		virtual void ClearUserUniforms();

	private:
		void PopFrameBufferObject();
		void PushFrameBufferObject( FrameBufferObject * fbo );

		void EmitProgram( Program* program, Program::Version version );

	public:
		// Generate the appropriate buffered Vulkan commands to accomplish the
		// specified state changes.
		virtual void BindFrameBufferObject( FrameBufferObject* fbo, bool asDrawBuffer );
		virtual void CaptureRect( FrameBufferObject* fbo, Texture& texture, const Rect& rect, const Rect& rawRect ) { Rtt_ASSERT_NOT_IMPLEMENTED(); }
		virtual void BindGeometry( Geometry* geometry );
		virtual void BindTexture( Texture* texture, U32 unit );
		virtual void BindUniform( Uniform* uniform, U32 unit );
		virtual void BindProgram( Program* program, Program::Version version );
        virtual void BindInstancing( U32 count, Geometry::Vertex* instanceData ) { Rtt_ASSERT_NOT_IMPLEMENTED(); }
        virtual void BindVertexFormat( FormatExtensionList* extensionList, U16 fullCount, U16 vertexSize, U32 offset ) { Rtt_ASSERT_NOT_IMPLEMENTED(); }
		virtual void SetBlendEnabled( bool enabled );
		virtual void SetBlendFunction( const BlendMode& mode );
		virtual void SetBlendEquation( RenderTypes::BlendEquation mode );
		virtual void SetViewport( int x, int y, int width, int height );
		virtual void SetScissorEnabled( bool enabled );
		virtual void SetScissorRegion( int x, int y, int width, int height );
		virtual void SetMultisampleEnabled( bool enabled );
        virtual void ClearDepth( Real depth ) { Rtt_ASSERT_NOT_IMPLEMENTED(); }
        virtual void ClearStencil( U32 stencil ) { Rtt_ASSERT_NOT_IMPLEMENTED(); }
		virtual void Clear( Real r, Real g, Real b, Real a );
		virtual void Draw( U32 offset, U32 count, Geometry::PrimitiveType type );
		virtual void DrawIndexed( U32 offset, U32 count, Geometry::PrimitiveType type );
		virtual S32 GetCachedParam( CommandBuffer::QueryableParams param );
		
		virtual void AddCommand( const CoronaCommand * command ) { Rtt_ASSERT_NOT_IMPLEMENTED(); }
        virtual void IssueCommand( U16 id, const void * data, U32 size ) { Rtt_ASSERT_NOT_IMPLEMENTED(); }

        virtual const unsigned char * GetBaseAddress() const { Rtt_ASSERT_NOT_IMPLEMENTED(); return NULL; }
    
        virtual bool WriteNamedUniform( const char * uniformName, const void * data, unsigned int size ) { Rtt_ASSERT_NOT_IMPLEMENTED(); return false; }
		
		virtual void WillRender();

		// Execute all buffered commands. A valid OpenGL context must be active.
		virtual Real Execute( bool measureGPU );
	
	private:	
		struct PushConstantState : public VulkanPushConstants {
			PushConstantState()
			{
				Reset();
			}

			void Reset();
			void Write( U32 offset, const void * src, size_t size );

			float * GetData( U32 offset );
			bool IsValid() const { return lowerOffset <= upperOffset; }
			U32 Offset() const { return lowerOffset; }
			U32 Range() const { return IsValid() ? upperOffset - lowerOffset + 4U * sizeof( float ) : 0U; }

			U32 lowerOffset;
			U32 upperOffset;
		};

	public:
		enum { kNumTextures = 5 };

		bool Wait( VulkanContext * context, FrameResources * frameResources, VkSwapchainKHR swapchain );
		VkResult GetExecuteResult() const { return fExecuteResult; }
		
		void BeginFrame();
		void ClearExecuteResult() { fExecuteResult = VK_SUCCESS; }
		bool PrepareDraw( VkCommandBuffer commandBuffer, VkPrimitiveTopology topology, std::vector< VkDescriptorImageInfo > & imageInfo, PushConstantState & pushConstants, U32 stages );

	public:
		VkDescriptorSet AddTextureSet( const std::vector< VkDescriptorImageInfo > & imageInfo );
		void RecordTextures();

	private:
		virtual void InitializeFBO();
		virtual void InitializeCachedParams();
		virtual void CacheQueryParam( CommandBuffer::QueryableParams param );

	private:
		// Templatized helper function for reading an arbitrary argument from
		// the command buffer.
		template <typename T>
		T Read();

		// Templatized helper function for writing an arbitrary argument to the
		// command buffer.
		template <typename T>
		void Write(T);

	private:
		struct UniformUpdate
		{
			Uniform* uniform;
			U32 timestamp;
		};

		void ApplyUniforms( GPUResource* resource );
		void ApplyPushConstant( Uniform * uniform, size_t offset, const size_t * translationOffset, VulkanProgram * program = NULL, U32 index = ~0U );
		void ApplyUniform( VulkanProgram & vulkanProgram, U32 index );
		void WriteUniform( Uniform* uniform );
		U8 * PointToUniform( U32 index, size_t offset );

		BufferDescriptor & Buffer( U32 index );
		BufferDescriptor & BufferForIndex( U32 index );

		UniformUpdate fUniformUpdates[Uniform::kNumBuiltInVariables];

		Program::Version fCurrentPrepVersion;
		Program::Version fCurrentDrawVersion;
		
	private:
		Program* fProgram;
		FrameBufferObject * fDefaultFBO;/*
		U32* fTimerQueries;
		U32 fTimerQueryIndex;*/
		Real fElapsedTimeGPU;
		S32 fCachedQuery[kNumQueryableParams];
		VulkanRenderer & fRenderer;
		Uniform fContentSize;

		// non-owned, retained only for frame:
		FrameResources * fFrameResources;
		VkPipeline fPipeline;
		VkSwapchainKHR fSwapchain;

		struct FBONode {
			FrameBufferObject * fFBO;
			U8 * fOldBuffer;
			U32 fOldBytesAllocated;
			U32 fOldBytesUsed;
			U32 fOldNumCommands;
		};

		struct OffscreenNode {
			U8 * fBuffer;
			U32 fBytesAllocated;
			U32 fNumCommands;
			U16 fHeight;
			U16 fOrder;

			bool operator < (const OffscreenNode & other) const;
		};

		std::vector< FBONode > fFBOStack;
		std::vector< OffscreenNode > fOffscreenSequence;
		Geometry * fCurrentGeometry;
		Texture * fCurrentTextures[kNumTextures];
//		dynamic uniform buffers - as a list?
		VkResult fExecuteResult;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_VulkanCommandBuffer_H__
