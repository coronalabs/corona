//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_VulkanContext.h"
#include "Renderer/Rtt_VulkanCommandBuffer.h"

#include "Renderer/Rtt_FrameBufferObject.h"
#include "Renderer/Rtt_VulkanFrameBufferObject.h"
#include "Renderer/Rtt_VulkanGeometry.h"
#include "Renderer/Rtt_VulkanProgram.h"
#include "Renderer/Rtt_VulkanTexture.h"
#include "Renderer/Rtt_VulkanRenderer.h"
#include "Display/Rtt_ShaderResource.h"

#include <algorithm>
#include <cinttypes> // https://stackoverflow.com/questions/8132399/how-to-printf-uint64-t-fails-with-spurious-trailing-in-format#comment9979590_8132440
#include <limits>

#include "CoronaLog.h"

// ----------------------------------------------------------------------------

namespace /*anonymous*/
{
	enum Command
	{
		kCommandBeginCapture,
		kCommandBindImageIndex,
		kCommandBindFrameBufferObject,
		kCommandUnBindFrameBufferObject,
		kCommandBeginRenderPass,
		kCommandBindGeometry,
		kCommandFetchGeometry,
		kCommandFetchTextures,
		kCommandFetchRenderState,
		kCommandBindTexture,
		kCommandBindProgram,
		kCommandApplyPushConstantScalar,
		kCommandApplyPushConstantVec2,
		kCommandApplyPushConstantVec3,
		kCommandApplyPushConstantVec4,
		kCommandApplyPushConstantMat3,
		kCommandApplyPushConstantMat4,
		kCommandApplyPushConstantMaskTransform,
		kCommandApplyUniformScalar,
		kCommandApplyUniformVec2,
		kCommandApplyUniformVec3,
		kCommandApplyUniformVec4,
		kCommandApplyUniformMat3,
		kCommandApplyUniformMat4,
		kCommandApplyPushConstantFromPointerScalar,
		kCommandApplyPushConstantFromPointerVec2,
		kCommandApplyPushConstantFromPointerVec3,
		kCommandApplyPushConstantFromPointerVec4,
		kCommandApplyPushConstantFromPointerMat3,
		kCommandApplyPushConstantFromPointerMat4,
		kCommandApplyPushConstantFromPointerMaskTransform,
		kCommandApplyUniformFromPointerScalar,
		kCommandApplyUniformFromPointerVec2,
		kCommandApplyUniformFromPointerVec3,
		kCommandApplyUniformFromPointerVec4,
		kCommandApplyUniformFromPointerMat3,
		kCommandApplyUniformFromPointerMat4,
		kCommandEnableBlend,
		kCommandDisableBlend,
		kCommandSetBlendFunction,
		kCommandSetBlendEquation,
		kCommandSetViewport,
		kCommandEnableScissor,
		kCommandDisableScissor,
		kCommandSetScissorRegion,
		kCommandEnableMultisample,
		kCommandDisableMultisample,
		kCommandClear,
		kCommandDraw,
		kCommandDrawIndexed,
		kNumCommands
	};

	// To ease reading/writing of arrays
	struct Vec2 { Rtt::Real data[2]; };
	struct Vec3 { Rtt::Real data[3]; };
	struct Vec4 { Rtt::Real data[4]; };
	struct Mat3 { Rtt::Real data[9]; };
	struct Mat4 { Rtt::Real data[16]; };

	// NOT USED: const Rtt::Real kNanosecondsToMilliseconds = 1.0f / 1000000.0f;
	const U32 kTimerQueryCount = 3;
	
	// The Uniform timestamp counter must be the same for both the
	// front and back CommandBuffers, though only one CommandBuffer
	// will ever write the timestamp on any given frame. If it were
	// ever the case that more than two CommandBuffers were used,
	// this would need to be made a shared member variable.
	static U32 gUniformTimestamp = 0;

	// Extract location and data from buffer
	#define READ_UNIFORM_DATA( Type ) \
		VulkanProgram::Location location = Read<VulkanProgram::Location>(); \
		Type value = Read<Type>();

	// Extract data but query for location
	#define READ_UNIFORM_DATA_WITH_PROGRAM( Type ) \
				Rtt::VulkanProgram* program = Read<Rtt::VulkanProgram*>(); \
				U32 index = Read<U32>(); \
				VulkanProgram::Location location = program->GetUniformLocation( index, fCurrentDrawVersion ); \
				Type value = Read<Type>();
	
	#define CHECK_ERROR_AND_BREAK /* something... */ break;

	// Ensure command count is incremented
	#define WRITE_COMMAND( command ) Write<Command>( command ); ++fNumCommands;
#define ENABLE_DEBUG_PRINT 0
	// Used to validate that the appropriate Vulkan commands
	// are being generated and that their arguments are correct
	#if ENABLE_DEBUG_PRINT 
		#define DEBUG_PRINT( ... ) Rtt_LogException( __VA_ARGS__ ); Rtt_LogException("\n");
		#define DEBUG_PRINT_MATRIX( message, data, count ) \
			Rtt_LogException( "%s\n", message ); \
			Rtt_LogException( "[ %.3f", data[0] ); \
			for( U32 i = 1; i < count; ++i ) \
				Rtt_LogException( ", %.3f", data[i] ); \
			Rtt_LogException ("]\n" );
	#else 
		#define DEBUG_PRINT( ... )
		#define DEBUG_PRINT_MATRIX( message, data, count )
	#endif
}

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

VulkanCommandBuffer::VulkanCommandBuffer( Rtt_Allocator* allocator, VulkanRenderer & renderer )
:	CommandBuffer( allocator ),
	fCurrentPrepVersion( Program::kMaskCount0 ),
	fCurrentDrawVersion( Program::kMaskCount0 ),
	fProgram( NULL ),
	fDefaultFBO( NULL ),
	fCurrentGeometry( NULL ),
//	fTimerQueries( new U32[kTimerQueryCount] ),
//	fTimerQueryIndex( 0 ),
	fElapsedTimeGPU( 0.0f ),
	fRenderer( renderer ),
	fContentSize( allocator, Uniform::kVec2 ),
	fPipeline( VK_NULL_HANDLE ),
	fSwapchain( VK_NULL_HANDLE )
{
	for(U32 i = 0; i < Uniform::kNumBuiltInVariables; ++i)
	{
		fUniformUpdates[i].uniform = NULL;
		fUniformUpdates[i].timestamp = 0;
	}

	for (U32 i = 0; i < kNumTextures; ++i)
	{
		fCurrentTextures[i] = NULL;
	}

	ClearExecuteResult();
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
//	delete [] fTimerQueries;
}

void
VulkanCommandBuffer::Initialize()
{
#ifdef ENABLE_GPU_TIMER_QUERIES
	// Used to measure GPU execution time
	glGenQueries( kTimerQueryCount, fTimerQueries );
	for( U32 i = 0; i < kTimerQueryCount; ++i)
	{
		glBeginQuery( GL_TIME_ELAPSED, fTimerQueries[i] );
		glEndQuery( GL_TIME_ELAPSED );
	}
	GL_CHECK_ERROR();
#endif
	InitializeFBO();
	InitializeCachedParams();
	CacheQueryParam( kMaxTextureSize );
}

void
VulkanCommandBuffer::InitializeFBO()
{
}

void 
VulkanCommandBuffer::InitializeCachedParams()
{
	for (int i = 0; i < kNumQueryableParams; i++)
	{
		fCachedQuery[i] = -1;
	}
}

void 
VulkanCommandBuffer::CacheQueryParam( CommandBuffer::QueryableParams param )
{
	const VkPhysicalDeviceProperties & properties = fRenderer.GetContext()->GetProperties();

	if (CommandBuffer::kMaxTextureSize == param)
	{
		fCachedQuery[param] = S32( properties.limits.maxImageDimension2D );
	}
}

void 
VulkanCommandBuffer::Denitialize()
{
#ifdef ENABLE_GPU_TIMER_QUERIES
//	glDeleteQueries( kTimerQueryCount, fTimerQueries );
#endif
}

void
VulkanCommandBuffer::ClearUserUniforms()
{
	fUniformUpdates[Uniform::kMaskMatrix0].uniform = NULL;
	fUniformUpdates[Uniform::kMaskMatrix1].uniform = NULL;
	fUniformUpdates[Uniform::kMaskMatrix2].uniform = NULL;
	fUniformUpdates[Uniform::kUserData0].uniform = NULL;
	fUniformUpdates[Uniform::kUserData1].uniform = NULL;
	fUniformUpdates[Uniform::kUserData2].uniform = NULL;
	fUniformUpdates[Uniform::kUserData3].uniform = NULL;
}

void
VulkanCommandBuffer::PopFrameBufferObject()
{
	WRITE_COMMAND( kCommandUnBindFrameBufferObject );

	Rtt_ASSERT( !fFBOStack.empty() );

	FBONode back = fFBOStack.back();

	fFBOStack.pop_back();

	if (!fFBOStack.empty())
	{
		OffscreenNode entry;

		entry.fBuffer = fBuffer;
		entry.fBytesAllocated = fBytesAllocated;
		entry.fNumCommands = fNumCommands;
		entry.fHeight = fFBOStack.size();
		entry.fOrder = fOffscreenSequence.size();

		fOffscreenSequence.push_back( entry );

		fBuffer = back.fOldBuffer;
		fBytesAllocated = back.fOldBytesAllocated;
		fBytesUsed = back.fOldBytesUsed;
		fNumCommands = back.fOldNumCommands;

		if (fCurrentGeometry)
		{
			WRITE_COMMAND( kCommandFetchGeometry );

			Write< GPUResource * >( fCurrentGeometry->GetGPUResource() );
		}

		if (fProgram)
		{
			EmitProgram( fProgram, fCurrentPrepVersion );
		}

		RecordTextures();
	}
}

void
VulkanCommandBuffer::PushFrameBufferObject( FrameBufferObject * fbo )
{
	FBONode node;

	node.fOldBytesAllocated = fBytesAllocated;
	node.fOldBytesUsed = fBytesUsed;
	node.fOldNumCommands = fNumCommands;
	node.fOldBuffer = fBuffer;
	node.fFBO = fbo;

	bool isOffscreen = !fFBOStack.empty();

	if (isOffscreen)
	{
		fBytesAllocated = 0U;
		fBytesUsed = 0U;
		fNumCommands = 0U;
		fBuffer = NULL;
	}

	fFBOStack.push_back( node );

	WRITE_COMMAND( kCommandBindFrameBufferObject );

	Write< GPUResource * >( fbo->GetGPUResource() );

	if (isOffscreen)
	{
		WRITE_COMMAND( kCommandFetchRenderState );

		Write<VkPipelineColorBlendAttachmentState>( fRenderer.GetColorBlendState() );

		if (fProgram)
		{
			EmitProgram( fProgram, fCurrentPrepVersion );
		}
	}
}

void
VulkanCommandBuffer::EmitProgram( Program * program, Program::Version version )
{
	WRITE_COMMAND( kCommandBindProgram );
	Write<Program::Version>( version );
	Write<GPUResource*>( program->GetGPUResource() );
}

void
VulkanCommandBuffer::BindFrameBufferObject( FrameBufferObject* fbo, bool ) // TODO: as draw buffer
{
	size_t height = fFBOStack.size();

	if (
		NULL == fbo || // done with un-nested framebuffer?
		(height >= 2U && fbo == fFBOStack[height - 2U].fFBO) // was this the previous framebuffer?
	)
	{
		if (height > 0U) // ignore end-of-frame NULL fbo during capture
		{
			PopFrameBufferObject();
		}
	}

	else
	{
		PushFrameBufferObject( fbo );
	}
}

void 
VulkanCommandBuffer::BindGeometry( Geometry* geometry )
{
	WRITE_COMMAND( kCommandBindGeometry );
	Write<GPUResource*>( geometry->GetGPUResource() );

	fCurrentGeometry = geometry;
}

void 
VulkanCommandBuffer::BindTexture( Texture* texture, U32 unit )
{
	WRITE_COMMAND( kCommandBindTexture );
	Write<U32>( unit );
	Write<GPUResource*>( texture->GetGPUResource() );

	fCurrentTextures[unit] = texture;
}

void 
VulkanCommandBuffer::BindProgram( Program* program, Program::Version version )
{
	EmitProgram( program, version );

	fCurrentPrepVersion = version;
	fProgram = program;

	AcquireTimeTransform( program->GetShaderResource() );
}

void
VulkanCommandBuffer::BindUniform( Uniform* uniform, U32 unit )
{
	Rtt_ASSERT( unit < Uniform::kNumBuiltInVariables );
	
	UniformUpdate& update = fUniformUpdates[ unit ];
	update.uniform = uniform;
	update.timestamp = gUniformTimestamp++;
}

void
VulkanCommandBuffer::SetBlendEnabled( bool enabled )
{
	WRITE_COMMAND( enabled ? kCommandEnableBlend : kCommandDisableBlend );

	VkPipelineColorBlendAttachmentState & state = fRenderer.GetColorBlendState();

	state.blendEnable = enabled ? VK_TRUE : VK_FALSE;
}

static VkBlendFactor
VulkanFactorForBlendParam( BlendMode::Param param )
{
	VkBlendFactor result = VK_BLEND_FACTOR_SRC_ALPHA;

	switch ( param )
	{
		case BlendMode::kZero:
			result = VK_BLEND_FACTOR_ZERO;
			break;
		case BlendMode::kOne:
			result = VK_BLEND_FACTOR_ONE;
			break;
		case BlendMode::kSrcColor:
			result = VK_BLEND_FACTOR_SRC_COLOR;
			break;
		case BlendMode::kOneMinusSrcColor:
			result = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			break;
		case BlendMode::kDstColor:
			result = VK_BLEND_FACTOR_DST_COLOR;
			break;
		case BlendMode::kOneMinusDstColor:
			result = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			break;
		case BlendMode::kSrcAlpha:
			result = VK_BLEND_FACTOR_SRC_ALPHA;
			break;
		case BlendMode::kOneMinusSrcAlpha:
			result = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			break;
		case BlendMode::kDstAlpha:
			result = VK_BLEND_FACTOR_DST_ALPHA;
			break;
		case BlendMode::kOneMinusDstAlpha:
			result = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			break;
		case BlendMode::kSrcAlphaSaturate:
			result = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}

	return result;
}

void
VulkanCommandBuffer::SetBlendFunction( const BlendMode& mode )
{
	WRITE_COMMAND( kCommandSetBlendFunction );

	VkBlendFactor srcColor = VulkanFactorForBlendParam( mode.fSrcColor );
	VkBlendFactor dstColor = VulkanFactorForBlendParam( mode.fDstColor );

	VkBlendFactor srcAlpha = VulkanFactorForBlendParam( mode.fSrcAlpha );
	VkBlendFactor dstAlpha = VulkanFactorForBlendParam( mode.fDstAlpha );

	VkPipelineColorBlendAttachmentState & state = fRenderer.GetColorBlendState();

	state.srcColorBlendFactor = srcColor;
	state.srcAlphaBlendFactor = srcAlpha;
	state.dstColorBlendFactor = dstColor;
	state.dstAlphaBlendFactor = dstAlpha;

	Write<VkBlendFactor>( srcColor );
	Write<VkBlendFactor>( dstColor );
	Write<VkBlendFactor>( srcAlpha );
	Write<VkBlendFactor>( dstAlpha );
}

void 
VulkanCommandBuffer::SetBlendEquation( RenderTypes::BlendEquation mode )
{
	WRITE_COMMAND( kCommandSetBlendEquation );

	VkBlendOp equation = VK_BLEND_OP_ADD;

	switch( mode )
	{
		case RenderTypes::kSubtractEquation:
			equation = VK_BLEND_OP_SUBTRACT;
			break;
		case RenderTypes::kReverseSubtractEquation:
			equation = VK_BLEND_OP_REVERSE_SUBTRACT;
			break;
		default:
			break;
	}
	
	VkPipelineColorBlendAttachmentState & state = fRenderer.GetColorBlendState();

	state.alphaBlendOp = state.colorBlendOp = equation;

	Write<VkBlendOp>( equation );
}

void
VulkanCommandBuffer::SetViewport( int x, int y, int width, int height )
{
	WRITE_COMMAND( kCommandSetViewport );
	Write<int>(x);
	Write<int>(height - y);
	Write<int>(width);
	Write<int>(-height);
	
	fContentSize.SetValue( width, abs( height ) );

	BindUniform( &fContentSize, Uniform::kContentSize );
}

void 
VulkanCommandBuffer::SetScissorEnabled( bool enabled )
{
	WRITE_COMMAND( enabled ? kCommandEnableScissor : kCommandDisableScissor );
	// No-op: always want scissor, possibly fullscreen
}

void 
VulkanCommandBuffer::SetScissorRegion( int x, int y, int width, int height )
{
	WRITE_COMMAND( kCommandSetScissorRegion );

	// TODO? seems to be dead code
}

void
VulkanCommandBuffer::SetMultisampleEnabled( bool enabled )
{
	WRITE_COMMAND( enabled ? kCommandEnableMultisample : kCommandDisableMultisample );
}

void 
VulkanCommandBuffer::Clear( Real r, Real g, Real b, Real a )
{
	WRITE_COMMAND( kCommandClear );

	VkClearValue value;

	// TODO: allow this to accommodate float targets?

	value.color.float32[0] = r;
	value.color.float32[1] = g;
	value.color.float32[2] = b;
	value.color.float32[3] = a;

	Write<VkClearValue>(value);

	// TODO: write others, if necessary...
}

void 
VulkanCommandBuffer::Draw( U32 offset, U32 count, Geometry::PrimitiveType type )
{
	Rtt_ASSERT( fProgram && fProgram->GetGPUResource() );
	ApplyUniforms( fProgram->GetGPUResource() );
	
	WRITE_COMMAND( kCommandDraw );
	switch( type )
	{
		case Geometry::kTriangleStrip:	Write<VkPrimitiveTopology>(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);	break;
		case Geometry::kTriangleFan:	Write<VkPrimitiveTopology>(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN);		break;
		case Geometry::kTriangles:		Write<VkPrimitiveTopology>(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);		break;
		case Geometry::kLines:			Write<VkPrimitiveTopology>(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);			break;
		case Geometry::kLineLoop:		Write<VkPrimitiveTopology>(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);		break;
		default: Rtt_ASSERT_NOT_REACHED(); break;
	}
	Write<U32>(offset);
	Write<U32>(count);
}

void 
VulkanCommandBuffer::DrawIndexed( U32, U32 count, Geometry::PrimitiveType type )
{
	// The first argument, offset, is currently unused. If support for non-
	// VBO based indexed rendering is added later, an offset may be needed.

	Rtt_ASSERT( fProgram && fProgram->GetGPUResource() );
	ApplyUniforms( fProgram->GetGPUResource() );
	
	WRITE_COMMAND( kCommandDrawIndexed );
	switch( type )
	{
		case Geometry::kIndexedTriangles:	Write<VkPrimitiveTopology>(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);	break;
		default: Rtt_ASSERT_NOT_REACHED(); break;
	}
	Write<U32>(count);
}

S32
VulkanCommandBuffer::GetCachedParam( CommandBuffer::QueryableParams param )
{
	S32 result = -1;

	if (param < kNumQueryableParams)
	{
		result = fCachedQuery[param];
	}

	Rtt_ASSERT_MSG(result != -1, "Parameter not cached");
	
	return result;
}

void
VulkanCommandBuffer::WillRender()
{
	if (fFBOStack.size() > 1U)
	{
		if (fCurrentGeometry)
		{
			WRITE_COMMAND( kCommandFetchGeometry );

			Write< GPUResource * >( fCurrentGeometry->GetGPUResource() );
		}

		RecordTextures();
	}

	WRITE_COMMAND( kCommandBeginRenderPass );
}

bool
AddToGeometryList( std::vector< VulkanGeometry * > & list, VulkanGeometry * geometry )
{
	for (VulkanGeometry * vg : list)
	{
		if (vg == geometry)
		{
			return false;
		}
	}

	list.push_back( geometry );

	return true;
}

Real 
VulkanCommandBuffer::Execute( bool measureGPU )
{
	Rtt_ASSERT( fFBOStack.empty() );

	DEBUG_PRINT( "--Begin Rendering: VulkanCommandBuffer --" );

	InitializeFBO();

#ifdef ENABLE_GPU_TIMER_QUERIES
	if( measureGPU )
	{/*
		GLint available = 0;
		GLint id = fTimerQueries[fTimerQueryIndex];
		while( !available )
		{
			glGetQueryObjectiv( id, GL_QUERY_RESULT_AVAILABLE, &available);
		}

		GLuint64 result = 0;
		glGetQueryObjectui64vEXT( id, GL_QUERY_RESULT, &result );
		fElapsedTimeGPU = result * kNanosecondsToMilliseconds;

		glBeginQuery( GL_TIME_ELAPSED, id);
		fTimerQueryIndex = ( fTimerQueryIndex + 1 ) % kTimerQueryCount;
	*/}

	if( measureGPU )
	{
	//	glEndQuery( GL_TIME_ELAPSED );
	}
#endif

	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	const VulkanContext * context = fRenderer.GetContext();
	VkDevice device = context->GetDevice();

	if (fFrameResources)
	{
		Rtt_ASSERT( fFrameResources->fCommands );

		vkResetCommandPool( device, fFrameResources->fCommands, 0U );

		VkCommandBufferAllocateInfo allocInfo = {};

		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandBufferCount = 1U;
		allocInfo.commandPool = fFrameResources->fCommands;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		if (VK_SUCCESS == vkAllocateCommandBuffers( device, &allocInfo, &commandBuffer ))
		{
			VkCommandBufferBeginInfo beginInfo = {};

			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (VK_SUCCESS == vkBeginCommandBuffer( commandBuffer, &beginInfo ))
			{
				fFrameResources->fUniforms->Reset( device );
				fFrameResources->fUserData->Reset( device );
				fFrameResources->fTextures->Reset( device );

				fProgram = NULL;
				fCurrentGeometry = NULL;

				for (int i = 0; i < kNumTextures; ++i)
				{
					fCurrentTextures[i] = NULL;
				}

				Buffer( 0 ).AllowMark();
			}

			else
			{
				Rtt_TRACE_SIM(( "ERROR: Failed to begin recording command buffer!" ));

				commandBuffer = VK_NULL_HANDLE;
			}
		}
	}

	std::sort( fOffscreenSequence.begin(), fOffscreenSequence.end() );

	VulkanUniforms uniforms;
	VulkanUserData userData;
	PushConstantState pushConstants;

	U32 numPasses = 1U + fOffscreenSequence.size();

	if (VK_NULL_HANDLE != commandBuffer)
	{
		Buffer( 0 ).SetWorkspace( &uniforms );
		Buffer( 1 ).SetWorkspace( &userData );
	}

	else
	{
		numPasses = 0U;
	}

	std::vector< VulkanGeometry * > geometryList;

	uint32_t imageIndex = ~0U;

	U8 * savedBuffer = fBuffer;
	U32 savedBytesAllocated = fBytesAllocated, savedNumCommands = fNumCommands;

	for ( U32 pass = 0; pass < numPasses; ++pass )
	{
		fRenderer.ResetPipelineInfo();

		bool isPrimaryPass = fOffscreenSequence.empty(), isCapture = false;

		if (isPrimaryPass)
		{
			// Reset the offset pointer to the start of the buffer.
			// This is safe to do here, as preparation work is done
			// on another CommandBuffer while this one is executing.
			fBuffer = fOffset = savedBuffer;
			fBytesAllocated = savedBytesAllocated;
			fNumCommands = savedNumCommands;
		}

		else
		{
			OffscreenNode current = fOffscreenSequence.back();

			fBuffer = fOffset = current.fBuffer;
			fBytesAllocated = current.fBytesAllocated;
			fNumCommands = current.fNumCommands;

			fOffscreenSequence.pop_back();
		}

		std::vector< VkDescriptorImageInfo > descriptorImageInfo( kNumTextures, VkDescriptorImageInfo{} );
		std::vector< VkClearValue > clearValues;
		VkViewport viewport;

		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		VkRenderPassBeginInfo renderPassBeginInfo = {};

		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		
		VulkanGeometry * geometry = NULL;
		VulkanFrameBufferObject * fbo = NULL;
		U32 stages = 0U;

		for( U32 i = 0; i < fNumCommands; ++i )
		{
			Command command = Read<Command>();

			Rtt_ASSERT( command < kNumCommands );
			switch( command )
			{
				case kCommandBeginCapture:
				{
					Rtt_ASSERT( ~0U == imageIndex );

					isCapture = true;

					DEBUG_PRINT( "Begin Capture" );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandBindImageIndex:
				{
					Rtt_ASSERT( !isCapture );

					imageIndex = Read<uint32_t>();

					DEBUG_PRINT( "Image index %i", imageIndex );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandBindFrameBufferObject:
				{
					fbo = Read<VulkanFrameBufferObject*>();

					clearValues.clear();

					if (isCapture)
					{
						fRenderer.PrepareCapture( fbo, fFrameResources->fFence );
					}
				
					DEBUG_PRINT( "Bind FrameBufferObject, %p (texture = %p)", fbo, fbo->GetTextureName() );/*: Vulkan ID=%" PRIx64 ", Vulkan Texture ID, if any: %" PRIx64,
									fbo->,
									fbo->GetTextureName() );*/
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandUnBindFrameBufferObject:
				{
					Rtt_ASSERT( renderPassBeginInfo.renderPass );

					vkCmdEndRenderPass( commandBuffer );

					renderPassBeginInfo.renderPass = VK_NULL_HANDLE;

					DEBUG_PRINT( "Unbind FrameBufferObject: Vulkan name: %p (fDefaultFBO)", fDefaultFBO );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandBeginRenderPass:
				{
					Rtt_ASSERT( fbo );

					uint32_t index;
					
					if (isPrimaryPass)
					{
						index = !isCapture ? imageIndex : 0U;
					}
						
					else
					{
						index = clearValues.empty(); // use the buffer / pass for the desired clear behavior

						fbo->BeginOffscreenPass( fRenderer, commandBuffer, clearValues.empty() );

						DEBUG_PRINT( "Offscreen pass: %s", clearValues.empty() ? "load" : "clear" );
					}

					fbo->Bind( fRenderer, index, renderPassBeginInfo );

					renderPassBeginInfo.clearValueCount = clearValues.size();
					renderPassBeginInfo.pClearValues = clearValues.data();
		
					vkCmdBeginRenderPass( commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
					vkCmdSetViewport( commandBuffer, 0U, 1U, &viewport );
					vkCmdSetScissor( commandBuffer, 0U, 1U, &renderPassBeginInfo.renderArea );

					if (geometry)
					{
						geometry->Bind( fRenderer, commandBuffer );
					}

					Buffer( 0 ).ResetMark();

					DEBUG_PRINT( "BEGIN RENDER PASS " );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandBindGeometry: // fallthrough
				case kCommandFetchGeometry:
				{
					VulkanGeometry * vg = Read<VulkanGeometry*>();

					if (AddToGeometryList( geometryList, vg ))
					{
						vg->Populate();
					}

					if (vg != geometry)
					{
						vg->Bind( fRenderer, commandBuffer );

						geometry = vg;
					}

					DEBUG_PRINT( "%s Geometry %p", kCommandBindGeometry == command ? "Bind" : "Fetch", geometry );
					CHECK_ERROR_AND_BREAK;

				}
				case kCommandFetchTextures:
				{
					for (U32 i = 0; i < kNumTextures; ++i)
					{		
						VulkanTexture * texture = Read<VulkanTexture *>();

						if (texture)
						{
							texture->Bind( *fFrameResources->fTextures, descriptorImageInfo[i] );
						}
					}

					DEBUG_PRINT( "Fetch Textures" );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandFetchRenderState:
				{
					VkPipelineColorBlendAttachmentState state = Read<VkPipelineColorBlendAttachmentState>();

					fRenderer.EnableBlend( state.blendEnable );
					fRenderer.SetBlendFactors( state.srcColorBlendFactor, state.srcAlphaBlendFactor, state.dstColorBlendFactor, state.dstAlphaBlendFactor );
					fRenderer.SetBlendEquations( state.colorBlendOp, state.alphaBlendOp );
					// write mask, etc.

					DEBUG_PRINT( "Fetch render state" );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandBindTexture:
				{
					U32 unit = Read<U32>();
					VulkanTexture* texture = Read<VulkanTexture*>();
					texture->Bind( *fFrameResources->fTextures, descriptorImageInfo[unit] );

					DEBUG_PRINT( "Bind Texture: texture=%p unit=%i Vulkan ID=%" PRIx64,
									texture,
									unit,
									texture->GetImage() );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandBindProgram:
				{
					fCurrentDrawVersion = Read<Program::Version>();
					VulkanProgram* program = Read<VulkanProgram*>();
					program->Bind( fRenderer, fCurrentDrawVersion );

					stages = Program::Version::kWireframe != fCurrentDrawVersion ? program->GetPushConstantStages() : 0U;

					DEBUG_PRINT( "Bind Program: program=%p version=%i", program, fCurrentDrawVersion );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyPushConstantScalar:
				{
					U32 offset = Read<U32>();
					Real value = Read<Real>();
					pushConstants.Write( offset, &value, sizeof( Real ) );
					DEBUG_PRINT( "Set Push Constant: value=%f location=%i", value, offset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyPushConstantVec2:
				{
					U32 offset = Read<U32>();
					Vec2 value = Read<Vec2>();
					pushConstants.Write( offset, &value, sizeof( Vec2 ) );
					DEBUG_PRINT( "Set Push Constant: value=(%f, %f) location=%i", value.data[0], value.data[1], offset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyPushConstantVec3:
				{
					U32 offset = Read<U32>();
					Vec3 value = Read<Vec3>();
					pushConstants.Write( offset, &value, sizeof( Vec3 ) );
					DEBUG_PRINT( "Set Push Constant: value=(%f, %f, %f) location=%i", value.data[0], value.data[1], value.data[2], offset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyPushConstantVec4:
				{
					U32 offset = Read<U32>();
					Vec4 value = Read<Vec4>();
					pushConstants.Write( offset, &value, sizeof( Vec4 ) );
					DEBUG_PRINT( "Set Push Constant: value=(%f, %f, %f, %f) location=%i", value.data[0], value.data[1], value.data[2], value.data[3], offset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyPushConstantMat3:
				{
					U32 offset = Read<U32>();
					Mat3 value = Read<Mat3>();
					for (int i = 0; i < 3; ++i)
					{
						pushConstants.Write( offset, &value.data[i * 3], sizeof( Vec3 ) );

						offset += sizeof( float ) * 4;
					}
					DEBUG_PRINT_MATRIX( "Set Push Constant: value=", value.data, 9 );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyPushConstantMat4:
				{
					U32 offset = Read<U32>();
					Mat4 value = Read<Mat4>();
					pushConstants.Write( offset, &value, sizeof( Mat4 ) );
					DEBUG_PRINT_MATRIX( "Set Push Constant: value=", value.data, 16 );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyPushConstantMaskTransform:
				{
					U32 maskOffset = Read<U32>();
					Vec4 maskMatrix = Read<Vec4>();
					U32 translationOffset = Read<U32>();
					Vec2 maskTranslation = Read<Vec2>();
					pushConstants.Write( maskOffset, &maskMatrix, sizeof( Vec4 ));
					pushConstants.Write( translationOffset, &maskTranslation, sizeof( Vec2 ) );
					DEBUG_PRINT( "Set Push Constant, mask matrix: value=(%f, %f, %f, %f) location=%i", maskMatrix.data[0], maskMatrix.data[1], maskMatrix.data[2], maskMatrix.data[3], maskOffset );
					DEBUG_PRINT( "Set Push Constant, mask translation: value=(%f, %f) location=%i", maskTranslation.data[0], maskTranslation.data[1], translationOffset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyUniformScalar:
				{
					READ_UNIFORM_DATA( Real );
					U32 index = Read<U32>();
					U8 * data = PointToUniform( index, location.fOffset );
					memcpy( data, &value, sizeof( Real ) );
					DEBUG_PRINT( "Set Uniform: value=%f location=%i", value, location.fOffset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyUniformVec2:
				{
					READ_UNIFORM_DATA( Vec2 );
					U32 index = Read<U32>();
					U8 * data = PointToUniform( index, location.fOffset );
					memcpy( data, &value, sizeof( Vec2 ) );
					DEBUG_PRINT( "Set Uniform: value=(%f, %f) location=%i", value.data[0], value.data[1], location.fOffset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyUniformVec3:
				{
					READ_UNIFORM_DATA( Vec3 );
					U32 index = Read<U32>();
					U8 * data = PointToUniform( index, location.fOffset );
					memcpy( data, &value, sizeof( Vec3 ) );
					DEBUG_PRINT( "Set Uniform: value=(%f, %f, %f) location=%i", value.data[0], value.data[1], value.data[2], location.fOffset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyUniformVec4:
				{
					READ_UNIFORM_DATA( Vec4 );
					U32 index = Read<U32>();
					U8 * data = PointToUniform( index, location.fOffset );
					memcpy( data, &value, sizeof( Vec4 ) );
					DEBUG_PRINT( "Set Uniform: value=(%f, %f, %f, %f) location=%i", value.data[0], value.data[1], value.data[2], value.data[3], location.fOffset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyUniformMat3:
				{
					READ_UNIFORM_DATA( Mat3 );
					U32 index = Read<U32>();
					U8 * data = PointToUniform( index, location.fOffset );
					for (int i = 0; i < 3; ++i)
					{
						memcpy( data, &value.data[i * 3], sizeof( Vec3 ) );

						data += sizeof( float ) * 4;
					}
					DEBUG_PRINT_MATRIX( "Set Uniform: value=", value.data, 9 );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyUniformMat4:
				{
					READ_UNIFORM_DATA( Mat4 );
					U32 index = Read<U32>();
					U8 * data = PointToUniform( index, location.fOffset );
					memcpy( data, &value, sizeof( Mat4 ) );
					DEBUG_PRINT_MATRIX( "Set Uniform: value=", value.data, 16 );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyPushConstantFromPointerScalar:
				{
					READ_UNIFORM_DATA_WITH_PROGRAM( Real );
					if (location.IsValid())
					{
						pushConstants.Write( location.fOffset, &value, sizeof( Real ) );
					}
					DEBUG_PRINT( "Set Push Constant: value=%f location=%i", value, location.fOffset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyPushConstantFromPointerVec2:
				{
					READ_UNIFORM_DATA_WITH_PROGRAM( Vec2 );
					if (location.IsValid())
					{
						pushConstants.Write( location.fOffset, &value, sizeof( Vec2 ) );
					}
					DEBUG_PRINT( "Set Push Constant: value=(%f, %f) location=%i", value.data[0], value.data[1], location.fOffset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyPushConstantFromPointerVec3:
				{
					READ_UNIFORM_DATA_WITH_PROGRAM( Vec3 );
					if (location.IsValid())
					{
						pushConstants.Write( location.fOffset, &value, sizeof( Vec3 ) );
					}
					DEBUG_PRINT( "Set Push Constant: value=(%f, %f, %f) location=%i", value.data[0], value.data[1], value.data[2], location.fOffset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyPushConstantFromPointerVec4:
				{
					READ_UNIFORM_DATA_WITH_PROGRAM( Vec4 );
					if (location.IsValid())
					{
						pushConstants.Write( location.fOffset, &value, sizeof( Vec4 ) );
					}
					DEBUG_PRINT( "Set Push Constant: value=(%f, %f, %f, %f) location=%i", value.data[0], value.data[1], value.data[2], value.data[3], location.fOffset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyPushConstantFromPointerMat3:
				{
					READ_UNIFORM_DATA_WITH_PROGRAM( Mat3 );
					if (location.IsValid())
					{
						U8 * data = PointToUniform( index, location.fOffset );
						for (int i = 0; i < 3; ++i)
						{
							memcpy( data, &value.data[i * 3], sizeof( Vec3 ) );

							data += sizeof( float ) * 4;
						}
					}
					DEBUG_PRINT_MATRIX( "Set Push Constant: value=", value.data, 9 );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyPushConstantFromPointerMat4:
				{
					READ_UNIFORM_DATA_WITH_PROGRAM( Mat4 );
					if (location.IsValid())
					{
						pushConstants.Write( location.fOffset, &value, sizeof( Mat4 ) );
					}
					DEBUG_PRINT_MATRIX( "Set Push Constant: value=", value.data, 16 );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyPushConstantFromPointerMaskTransform:
				{
					READ_UNIFORM_DATA_WITH_PROGRAM( Vec4 );
					VulkanProgram::Location translationLocation = program->GetTranslationLocation( Uniform::kViewProjectionMatrix + index, fCurrentDrawVersion );
					Vec2 maskTranslation = Read<Vec2>();
					if (location.IsValid())
					{
						pushConstants.Write( location.fOffset, &value, sizeof( Vec4 ));
						pushConstants.Write( translationLocation.fOffset, &maskTranslation, sizeof( Vec2 ) );
					}
					DEBUG_PRINT( "Set Push Constant, mask matrix: value=(%f, %f, %f, %f) location=%i", value.data[0], value.data[1], value.data[2], value.data[3], location.fOffset );
					DEBUG_PRINT( "Set Push Constant, mask translation: value=(%f, %f) location=%i", maskTranslation.data[0], maskTranslation.data[1], translationLocation.fOffset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyUniformFromPointerScalar:
				{
					READ_UNIFORM_DATA_WITH_PROGRAM( Real );
					if (program->HavePushConstantUniforms() && Descriptor::IsPushConstant( index, true ))
					{
						pushConstants.Write( location.fOffset, &value, sizeof( Real ) );
					}
					else if (location.IsValid())
					{
						U8 * data = PointToUniform( index, location.fOffset );
						memcpy( data, &value, sizeof( Real ) );
					}
					DEBUG_PRINT( "Set Uniform: value=%f location=%i", value, location.fOffset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyUniformFromPointerVec2:
				{
					READ_UNIFORM_DATA_WITH_PROGRAM( Vec2 );
					if (program->HavePushConstantUniforms() && Descriptor::IsPushConstant( index, true ))
					{
						pushConstants.Write( location.fOffset, &value, sizeof( Vec2 ) );
					}
					else if (location.IsValid())
					{
						U8 * data = PointToUniform( index, location.fOffset );
						memcpy( data, &value, sizeof( Vec2 ) );
					}
					DEBUG_PRINT( "Set Uniform: value=(%f, %f) location=%i", value.data[0], value.data[1], location.fOffset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyUniformFromPointerVec3:
				{
					READ_UNIFORM_DATA_WITH_PROGRAM( Vec3 );
					if (program->HavePushConstantUniforms() && Descriptor::IsPushConstant( index, true ))
					{
						pushConstants.Write( location.fOffset, &value, sizeof( Vec3 ) );
					}
					else if (location.IsValid())
					{
						U8 * data = PointToUniform( index, location.fOffset );
						memcpy( data, &value, sizeof( Vec3 ) );
					}
					DEBUG_PRINT( "Set Uniform: value=(%f, %f, %f) location=%i", value.data[0], value.data[1], value.data[2], location.fOffset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyUniformFromPointerVec4:
				{
					READ_UNIFORM_DATA_WITH_PROGRAM( Vec4 );
					if (program->HavePushConstantUniforms() && Descriptor::IsPushConstant( index, true ))
					{
						pushConstants.Write( location.fOffset, &value, sizeof( Vec4 ) );
					}
					else if (location.IsValid())
					{
						U8 * data = PointToUniform( index, location.fOffset );
						memcpy( data, &value, sizeof( Vec4 ) );
					}
					DEBUG_PRINT( "Set Uniform: value=(%f, %f, %f, %f) location=%i", value.data[0], value.data[1], value.data[2], value.data[3], location.fOffset );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyUniformFromPointerMat3:
				{
					READ_UNIFORM_DATA_WITH_PROGRAM( Mat3 );
					if (program->HavePushConstantUniforms() && Descriptor::IsPushConstant( index, true ))
					{
						size_t offset = location.fOffset;

						for (int i = 0; i < 3; ++i)
						{
							pushConstants.Write( offset, &value.data[i * 3], sizeof( Vec3 ) );

							offset += sizeof( float ) * 4;
						}
					}
					else if (location.IsValid())
					{
						U8 * data = PointToUniform( index, location.fOffset );
						for (int i = 0; i < 3; ++i)
						{
							memcpy( data, &value.data[i * 3], sizeof( Vec3 ) );

							data += sizeof( float ) * 4;
						}
					}
					DEBUG_PRINT_MATRIX( "Set Uniform: value=", value.data, 9 );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandApplyUniformFromPointerMat4:
				{
					READ_UNIFORM_DATA_WITH_PROGRAM( Mat4 );
					if (program->HavePushConstantUniforms() && Descriptor::IsPushConstant( index, true ))
					{
						pushConstants.Write( location.fOffset, &value, sizeof( Mat4 ) );
					}
					else if (location.IsValid())
					{
						U8 * data = PointToUniform( index, location.fOffset );
						memcpy( data, &value, sizeof( Mat4 ) );
					}
					DEBUG_PRINT_MATRIX( "Set Uniform: value=", value.data, 16 );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandEnableBlend:
				{
					fRenderer.EnableBlend( true );
					DEBUG_PRINT( "Enable blend" );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandDisableBlend:
				{
					fRenderer.EnableBlend( false );
					DEBUG_PRINT( "Disable blend" );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandSetBlendFunction:
				{
					VkBlendFactor srcColor = Read<VkBlendFactor>();
					VkBlendFactor dstColor = Read<VkBlendFactor>();

					VkBlendFactor srcAlpha = Read<VkBlendFactor>();
					VkBlendFactor dstAlpha = Read<VkBlendFactor>();

					fRenderer.SetBlendFactors( srcColor, srcAlpha, dstColor, dstAlpha );
					DEBUG_PRINT(
						"Set blend function: srcColor=%i, dstColor=%i, srcAlpha=%i, dstAlpha=%i",
						srcColor, dstColor, srcAlpha, dstAlpha );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandSetBlendEquation:
				{
					VkBlendOp equation = Read<VkBlendOp>();
					fRenderer.SetBlendEquations( equation, equation );
					DEBUG_PRINT( "Set blend equation: equation=%i", equation );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandSetViewport:
				{
					int x = Read<int>();
					int y = Read<int>();
					int width = Read<int>();
					int height = Read<int>();

					if (!isPrimaryPass || isCapture)
					{
						y = -(y + height); // undo -height - y
						height = -height;  // undo -height
					}

					viewport.x = float( x );
					viewport.y = float( y );
					viewport.width = float( width );
					viewport.height = float( height );
					DEBUG_PRINT( "Set viewport: x=%i, y=%i, width=%i, height=%i", x, y, width, height );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandEnableScissor:
				{
					// TODO?
					DEBUG_PRINT( "Enable scissor test" );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandDisableScissor:
				{
					// TODO?
					DEBUG_PRINT( "Disable scissor test" );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandSetScissorRegion:
				{
					// TODO?
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandEnableMultisample:
				{
					fRenderer.SetMultisample( fRenderer.GetContext()->GetSampleCountFlags() );

					DEBUG_PRINT( "Enable multisample test" );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandDisableMultisample:
				{
					fRenderer.SetMultisample( VK_SAMPLE_COUNT_1_BIT );

					DEBUG_PRINT( "Disable multisample test" );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandClear:
				{
					VkClearValue value = Read<VkClearValue>();

					clearValues.push_back( value );

					DEBUG_PRINT( "Clear " );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandDraw:
				{
					VkPrimitiveTopology mode = Read<VkPrimitiveTopology>();
					U32 offset = Read<U32>();
					U32 count = Read<U32>();

					if (PrepareDraw( commandBuffer, mode, descriptorImageInfo, pushConstants, stages ))
					{
						vkCmdDraw( commandBuffer, count, 1U, offset, 0U );
					}

					DEBUG_PRINT( "Draw: mode=%i, offset=%u, count=%u", mode, offset, count );
					CHECK_ERROR_AND_BREAK;
				}
				case kCommandDrawIndexed:
				{
					VkPrimitiveTopology mode = Read<VkPrimitiveTopology>();
					U32 count = Read<U32>();

					if (PrepareDraw( commandBuffer, mode, descriptorImageInfo, pushConstants, stages ))
					{
						// The first argument, offset, is currently unused. If support for non-
						// VBO based indexed rendering is added later, an offset may be needed.

						vkCmdDrawIndexed( commandBuffer, count, 1U, 0U, 0U, 0U );
					}

					DEBUG_PRINT( "Draw indexed: mode=%i, count=%u", mode, count );
					CHECK_ERROR_AND_BREAK;
				}
				default:
					DEBUG_PRINT( "Unknown command(%d)", command );
					Rtt_ASSERT_NOT_REACHED();
					break;
			}
		}

		if (!isPrimaryPass)
		{
			Rtt_DELETE( fBuffer );
		}
	}

	fBytesUsed = 0;
	fNumCommands = 0;
	
#ifdef ENABLE_GPU_TIMER_QUERIES
	if( measureGPU )
	{
	//	glEndQuery( GL_TIME_ELAPSED );
	}
#endif
	
	DEBUG_PRINT( "--End Rendering: VulkanCommandBuffer --\n" );

//	VULKAN_CHECK_ERROR();
	VkResult endResult = VK_SUCCESS, submitResult = VK_SUCCESS;
	bool usingSwapchainImage = ~0U != imageIndex;

	if (VK_NULL_HANDLE != commandBuffer)
	{
		endResult = vkEndCommandBuffer( commandBuffer );

		if (VK_SUCCESS == endResult)
		{
			VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			VkSubmitInfo submitInfo = {};

			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1U;
			submitInfo.pCommandBuffers = &commandBuffer;

			if (usingSwapchainImage)
			{
				submitInfo.pSignalSemaphores = &fFrameResources->fRenderFinished;
				submitInfo.pWaitDstStageMask = &waitStage;
				submitInfo.pWaitSemaphores = &fFrameResources->fImageAvailable;
				submitInfo.signalSemaphoreCount = 1U;
				submitInfo.waitSemaphoreCount = 1U;
			}

			vkResetFences( context->GetDevice(), 1U, &fFrameResources->fFence );

			submitResult = vkQueueSubmit( context->GetGraphicsQueue(), 1U, &submitInfo, fFrameResources->fFence );
		}
	}

	if (usingSwapchainImage && fSwapchain != VK_NULL_HANDLE)
	{
		VkPresentInfoKHR presentInfo = {};

		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pSwapchains = &fSwapchain;
		presentInfo.pWaitSemaphores = &fFrameResources->fRenderFinished;
		presentInfo.swapchainCount = 1U;
		presentInfo.waitSemaphoreCount = 1U;

		VkResult presentResult = vkQueuePresentKHR( context->GetPresentQueue(), &presentInfo );

		if (VK_ERROR_OUT_OF_DATE_KHR == presentResult || VK_SUBOPTIMAL_KHR == presentResult)
		{
			vkQueueWaitIdle( fRenderer.GetContext()->GetGraphicsQueue() ); // do this once now, else we might call many times if minimized

			fRenderer.ForceInvalidation(); // kludge: if nothing gets updated, the scene would have been updated THIS frame if the orientation / size changes,
											// we will be reconstructing the swapchain NEXT frame, so we manually invalidate to account for this case
			fRenderer.SetSwapchainInvalid( true );
			
			presentResult = VK_SUCCESS;
		}

		else if (presentResult != VK_SUCCESS)
		{
			Rtt_TRACE_SIM(( "ERROR: Failed to present swap chain image!" ));
		}

		fExecuteResult = presentResult;
	}

	else
	{
		if (usingSwapchainImage)
		{
			Rtt_TRACE_SIM(( "ERROR: Failed to submit draw command buffer!" ));
		}

		fExecuteResult = submitResult;
	}

	if (endResult != VK_SUCCESS)
	{
		Rtt_TRACE_SIM(( "ERROR: Failed to record command buffer!" ));

		fExecuteResult = endResult;
	}

	Rtt_ASSERT( fOffscreenSequence.empty() );

	return fElapsedTimeGPU;
}

bool VulkanCommandBuffer::Wait( VulkanContext * context, FrameResources * frameResources, VkSwapchainKHR swapchain )
{
	Rtt_ASSERT( NULL == fFrameResources );
	Rtt_ASSERT( frameResources );

	bool ok = VK_NULL_HANDLE != frameResources->fFence;

	if (ok)
	{
		context->WaitOnFence( frameResources->fFence );
		
		if (VK_NULL_HANDLE != swapchain)
		{
			uint32_t index;

			if (!fRenderer.GetSwapchainInvalid())
			{
				VkResult result = vkAcquireNextImageKHR( context->GetDevice(), swapchain, (std::numeric_limits< uint64_t >::max)(), frameResources->fImageAvailable, VK_NULL_HANDLE, &index );

				ok = VK_SUCCESS == result || VK_SUBOPTIMAL_KHR == result;
			}

			if (ok)
			{
				fSwapchain = swapchain;

				WRITE_COMMAND( kCommandBindImageIndex );

				Write<uint32_t>( index );
			}

			else
			{
				Rtt_TRACE_SIM(( "ERROR: Failed to acquire swap chain image!" ));
			}
		}

		else
		{
			WRITE_COMMAND( kCommandBeginCapture );
		}

		if (ok)
		{
			fFrameResources = frameResources;
		}
	}

	return ok;
}

void
VulkanCommandBuffer::BeginFrame()
{
	fFrameResources = NULL;
	fPipeline = VK_NULL_HANDLE;
	fSwapchain = VK_NULL_HANDLE;
}

bool VulkanCommandBuffer::PrepareDraw( VkCommandBuffer commandBuffer, VkPrimitiveTopology topology, std::vector< VkDescriptorImageInfo > & descriptorImageInfo, PushConstantState & pushConstants, U32 stages )
{
	bool canDraw = VK_NULL_HANDLE != commandBuffer;

	if (canDraw)
	{
		fRenderer.SetPrimitiveTopology( topology );

        VkPipeline pipeline = fRenderer.ResolvePipeline();

		canDraw = VK_NULL_HANDLE != pipeline;

		if (canDraw)
		{
			if (pipeline != fPipeline)
			{
				vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );

				fPipeline = pipeline;
			}

			VkDevice device = fRenderer.GetContext()->GetDevice();
			VkDescriptorSet sets[3] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };
			uint32_t dynamicOffsets[2] = {}, count = 0U, nsets = 0U;

			std::vector< VkMappedMemoryRange > memoryRanges;

			static_assert( Descriptor::kUniforms < Descriptor::kUserData, "Uniforms / buffer in unexpected order" );
			static_assert( Descriptor::kUserData < Descriptor::kTexture, "Buffer / textures in unexpected order" );

			U32 first = 2U; // try to do better

			for (U32 i = 0; i < 2; ++i)
			{
				BufferDescriptor & desc = Buffer( i );

				if (desc.fDirty)
				{
					if (i < first)
					{
						first = i;
					}

					desc.TryToAddDynamicOffset( dynamicOffsets, count );
					desc.TryToAddMemory( memoryRanges, sets, nsets );
					desc.TryToMark();
				}
			}

			if (fFrameResources->fTextures->fDirty)
			{
				sets[nsets++] = AddTextureSet( descriptorImageInfo );
			}

			VkPipelineLayout pipelineLayout = fRenderer.GetPipelineLayout();

			if (nsets > 0U)
			{
				if (!memoryRanges.empty())
				{
					vkFlushMappedMemoryRanges( device, memoryRanges.size(), memoryRanges.data() );
				}

				if (2U == nsets && !fFrameResources->fUserData->fDirty) // split?
				{
					Rtt_ASSERT( 0U == first );
					Rtt_ASSERT( 1U == count );

					vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0U, 1U, &sets[0], 1U, dynamicOffsets );
					vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2U, 1U, &sets[1], 0U, NULL );
				}

				else
				{
					vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, first, nsets, sets, count, dynamicOffsets );
				}
			}

			if (pushConstants.IsValid())
			{
				U32 offset = pushConstants.Offset(), size = pushConstants.Range();

				vkCmdPushConstants( commandBuffer, pipelineLayout, stages, offset, size, pushConstants.GetData( offset ) );
			}
		}
	}

	fFrameResources->fUniforms->fDirty = 0U;
	fFrameResources->fUserData->fDirty = 0U;
	fFrameResources->fTextures->fDirty = 0U;

	pushConstants.Reset();

	return canDraw;
}

VkDescriptorSet VulkanCommandBuffer::AddTextureSet( const std::vector< VkDescriptorImageInfo > & imageInfo )
{
	VulkanContext * context = fRenderer.GetContext();
	TexturesDescriptor & desc = *static_cast< TexturesDescriptor * >( fFrameResources->fTextures );

	VkDescriptorSetAllocateInfo allocInfo = {};
	VkDescriptorSetLayout layout = fRenderer.GetTextureLayout();

	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = desc.fPool;
	allocInfo.descriptorSetCount = 1U;
	allocInfo.pSetLayouts = &layout;
				
	VkDescriptorSet set = VK_NULL_HANDLE;
	VkResult result = vkAllocateDescriptorSets( context->GetDevice(), &allocInfo, &set );

	if (VK_ERROR_OUT_OF_POOL_MEMORY == result)
	{
		Rtt_TRACE_SIM(( "ERROR: Exhausted texture descriptors" ));

		return VK_NULL_HANDLE;
	}

	else if (VK_SUCCESS == result)
	{
		std::vector< VkWriteDescriptorSet > writes;

		VkWriteDescriptorSet wds = {};

		wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		wds.dstSet = set;

		Rtt_ASSERT( imageInfo.size() <= 31 );

		U32 inUse = 0U, previousMask = 0U;

		for (size_t i = 0; i < imageInfo.size(); ++i)
		{
			U32 currentMask = 1U << i;

			if (imageInfo[i].imageView != VK_NULL_HANDLE) // valid entry...
			{
				if (0U == (inUse & previousMask)) // ...but previous entry was not?
				{
					wds.dstArrayElement = i;
					wds.pImageInfo = imageInfo.data() + i;

					writes.push_back( wds );
				}

				inUse |= currentMask;

				++writes.back().descriptorCount;
			}

			previousMask = currentMask;
		}
         
		Rtt_ASSERT( inUse );

		// If the images were only partially bound, but this is not supported by the hardware, populate
		// the unoccupied slots with one of the valid entries.
		// if (!ThatFeatureThatLetsUsSkipThis && (inUse + 1U != (1U << imageInfo.size()) - 1U)
		{
			wds.descriptorCount = 1U;
			wds.pImageInfo = writes.back().pImageInfo;

			for (size_t i = 0; i < imageInfo.size(); ++i)
			{
				U32 used = inUse & (1U << i);

				if (!used)
				{
					wds.dstArrayElement = i;

					writes.push_back( wds );
				}
			}
		}

		vkUpdateDescriptorSets( context->GetDevice(), writes.size(), writes.data(), 0U, NULL );

		return set;
	}

	Rtt_TRACE_SIM(( "ERROR: Failed to allocate texture descriptor set!" ));

	return VK_NULL_HANDLE;
}

void VulkanCommandBuffer::RecordTextures()
{
	bool hasTextures = false;

	for (U32 i = 0; i < kNumTextures; ++i)
	{
		if (fCurrentTextures[i])
		{
			hasTextures = true;
		}
	}

	if (hasTextures)
	{
		WRITE_COMMAND( kCommandFetchTextures );

		for (U32 i = 0; i < kNumTextures; ++i)
		{
			GPUResource * texture = fCurrentTextures[i] ? fCurrentTextures[i]->GetGPUResource() : NULL;

			Write< GPUResource * >( texture );
		}
	}
}

template <typename T>
T 
VulkanCommandBuffer::Read()
{
	T result;
	ReadBytes( &result, sizeof( T ) );
	return result;
}

template <typename T>
void 
VulkanCommandBuffer::Write( T value )
{
	WriteBytes( &value, sizeof( T ) );
}

void VulkanCommandBuffer::ApplyUniforms( GPUResource* resource )
{
	Real rawTotalTime;
	bool transformed = false;

	VulkanProgram* vulkanProgram = static_cast< VulkanProgram * >( resource );

	if (fUsesTime)
    {
        const UniformUpdate& time = fUniformUpdates[Uniform::kTotalTime];
        if (fTimeTransform)
        {
            transformed = fTimeTransform->Apply( time.uniform, &rawTotalTime, time.timestamp );
        }
        if (transformed || !TimeTransform::Matches( fTimeTransform, fLastTimeTransform ))
        {
            fUniformUpdates[Uniform::kTotalTime].timestamp = vulkanProgram->GetUniformTimestamp( Uniform::kTotalTime, fCurrentPrepVersion ) - 1; // force a refresh
        }
    }

	for( U32 i = 0; i < Uniform::kNumBuiltInVariables; ++i)
	{
		const UniformUpdate& update = fUniformUpdates[i];
		if( update.uniform && update.timestamp != vulkanProgram->GetUniformTimestamp( i, fCurrentPrepVersion ) )
		{		
			ApplyUniform( *vulkanProgram, i );
		}
	}

	if (transformed) // restore raw value (lets us avoid a redundant variable; will also be in place for un-transformed time dependencies)
	{
		fUniformUpdates[Uniform::kTotalTime].uniform->SetValue(rawTotalTime);
	}
}

void VulkanCommandBuffer::ApplyPushConstant( Uniform * uniform, size_t offset, const size_t * translationOffset, VulkanProgram * program, U32 index )
{
	Uniform::DataType dataType = uniform->GetDataType();

	if ( translationOffset )
	{
		Rtt_ASSERT( Uniform::kMat3 == dataType );

		// Mindful of the troubles described in https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o,
		// each mask matrix is represented as a vec2[2] and vec2. (Three components are never used and thus omitted.)
		// The vec2 array avoids consuming two vectors, cf. https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL).
		// The elements are columns, to allow mat2(vec[0], vec[1]) on the shader side.
		float * src = reinterpret_cast< float * >( uniform->GetData() );
		Vec4 maskMatrix = {
			src[0], // row 1, col 1
			src[1], // row 2, col 1
			src[3], // row 1, col 2
			src[4]  // row 2, col 2
		};

		if (program)
		{
			WRITE_COMMAND(kCommandApplyPushConstantFromPointerMaskTransform);
			Write<VulkanProgram*>(program);
			Write<U32>(index);
			Write<Vec4>(maskMatrix);
		}

		else
		{
			WRITE_COMMAND(kCommandApplyPushConstantMaskTransform);
			Write<U32>(offset);
			Write<Vec4>(maskMatrix);
			Write<U32>(*translationOffset);
		}

		Vec2 maskTranslation = { src[6], src[7] };

		Write<Vec2>(maskTranslation);
	}

	else
	{
		Command command;

		switch (dataType)
		{
		case Uniform::kScalar:
			command = program ? kCommandApplyPushConstantFromPointerScalar : kCommandApplyPushConstantScalar; break;
		case Uniform::kVec2:
			command = program ? kCommandApplyPushConstantFromPointerVec2 : kCommandApplyPushConstantVec2; break;
		case Uniform::kVec3:
			command = program ? kCommandApplyPushConstantFromPointerVec3 : kCommandApplyPushConstantVec3; break;
		case Uniform::kVec4:
			command = program ? kCommandApplyPushConstantFromPointerVec4 : kCommandApplyPushConstantVec4; break;
		case Uniform::kMat3:
			command = program ? kCommandApplyPushConstantFromPointerMat3 : kCommandApplyPushConstantMat3; break;
		case Uniform::kMat4:
			command = program ? kCommandApplyPushConstantFromPointerMat4 : kCommandApplyPushConstantMat4; break;
		default:
			Rtt_ASSERT_NOT_REACHED();
		}

		WRITE_COMMAND(command);

		if (program)
		{
			Write<VulkanProgram*>(program);
			Write<U32>(index);
		}

		else
		{
			Write<U32>(offset);
		}

		WriteUniform( uniform );
	}
}

void VulkanCommandBuffer::WriteUniform( Uniform* uniform )
{
	switch( uniform->GetDataType() )
	{
		case Uniform::kScalar:	Write<Real>(*reinterpret_cast<Real*>(uniform->GetData()));	break;
		case Uniform::kVec2:	Write<Vec2>(*reinterpret_cast<Vec2*>(uniform->GetData()));	break;
		case Uniform::kVec3:	Write<Vec3>(*reinterpret_cast<Vec3*>(uniform->GetData()));	break;
		case Uniform::kVec4:	Write<Vec4>(*reinterpret_cast<Vec4*>(uniform->GetData()));	break;
		case Uniform::kMat3:	Write<Mat3>(*reinterpret_cast<Mat3*>(uniform->GetData()));	break;
		case Uniform::kMat4:	Write<Mat4>(*reinterpret_cast<Mat4*>(uniform->GetData()));	break;
		default:				Rtt_ASSERT_NOT_REACHED();									break;
	}
}

void VulkanCommandBuffer::ApplyUniform( VulkanProgram & vulkanProgram, U32 index )
{
	const UniformUpdate& update = fUniformUpdates[index];
	vulkanProgram.SetUniformTimestamp( index, fCurrentPrepVersion, update.timestamp );

	bool isValid = vulkanProgram.IsValid( fCurrentPrepVersion );
	VulkanProgram::Location location = vulkanProgram.GetUniformLocation( Uniform::kViewProjectionMatrix + index, fCurrentPrepVersion );
	Uniform* uniform = update.uniform;

	if (isValid && location.IsValid())
	{
		if (Descriptor::IsPushConstant( index, vulkanProgram.HavePushConstantUniforms() ))
		{
			if (Descriptor::IsMaskPushConstant( index ))
			{
				VulkanProgram::Location translationLocation = vulkanProgram.GetTranslationLocation( Uniform::kViewProjectionMatrix + index, fCurrentPrepVersion );

				ApplyPushConstant( uniform, location.fOffset, &translationLocation.fOffset );
			}

			else
			{
				ApplyPushConstant( uniform, location.fOffset, NULL );
			}
		}

		else
		{
			switch( uniform->GetDataType() )
			{
				case Uniform::kScalar:
					WRITE_COMMAND(kCommandApplyUniformScalar); break;
				case Uniform::kVec2:
					WRITE_COMMAND(kCommandApplyUniformVec2); break;
				case Uniform::kVec3:
					WRITE_COMMAND(kCommandApplyUniformVec3); break;
				case Uniform::kVec4:
					WRITE_COMMAND(kCommandApplyUniformVec4); break;
				case Uniform::kMat3:
					WRITE_COMMAND(kCommandApplyUniformMat3); break;
				case Uniform::kMat4:
					WRITE_COMMAND(kCommandApplyUniformMat4); break;
				default:
					Rtt_ASSERT_NOT_REACHED();
			}
			
			Write<VulkanProgram::Location>(location);
			WriteUniform( uniform );
			Write<U32>(index);
		}
	}

	else if (!isValid)
	{
		Uniform* uniform = update.uniform;

		if (Descriptor::IsPushConstant( index, false ))
		{
			const U32 translationOffset = 0U; // used only to check if mask constant

			ApplyPushConstant( uniform, 0U, Descriptor::IsMaskPushConstant( index ) ? &translationOffset : NULL, &vulkanProgram, index );
		}

		else
		{
			switch( uniform->GetDataType() )
			{
				case Uniform::kScalar:
					WRITE_COMMAND(kCommandApplyUniformFromPointerScalar);	break;
				case Uniform::kVec2:
					WRITE_COMMAND(kCommandApplyUniformFromPointerVec2);	break;
				case Uniform::kVec3:
					WRITE_COMMAND(kCommandApplyUniformFromPointerVec3);	break;
				case Uniform::kVec4:
					WRITE_COMMAND(kCommandApplyUniformFromPointerVec4);	break;
				case Uniform::kMat3:
					WRITE_COMMAND(kCommandApplyUniformFromPointerMat3);	break;
				case Uniform::kMat4:
					WRITE_COMMAND(kCommandApplyUniformFromPointerMat4);	break;
				default:
					Rtt_ASSERT_NOT_REACHED();
			}
			
			Write<VulkanProgram *>(&vulkanProgram);
			Write<U32>(index);
			WriteUniform(uniform);
		}
	}
}

U8 * VulkanCommandBuffer::PointToUniform( U32 index, size_t offset )
{
	BufferDescriptor & desc = BufferForIndex( index );

	desc.fDirty |= 1U << index;

	return desc.fWorkspace + offset;
}

BufferDescriptor & VulkanCommandBuffer::Buffer( U32 index )
{
	Rtt_ASSERT( fFrameResources );

	switch (index)
	{
	case Descriptor::kUniforms:
		return *fFrameResources->fUniforms;
	case Descriptor::kUserData:
		return *fFrameResources->fUserData;
	default:
		Rtt_ASSERT_NOT_REACHED();
	}
}

BufferDescriptor & VulkanCommandBuffer::BufferForIndex( U32 index )
{
	Descriptor::Index descIndex = Descriptor::IsUserData( index ) ? Descriptor::kUserData : Descriptor::kUniforms;

	return Buffer( descIndex );
}

void VulkanCommandBuffer::PushConstantState::Reset()
{
	upperOffset = 0U;
	lowerOffset = 1U;
}

void VulkanCommandBuffer::PushConstantState::Write( U32 offset, const void * src, size_t size )
{
	memcpy( GetData( offset ), src, size );

	U32 offset1 = offset & 0xF0;
	U32 offset2 = (offset + size - 1) & 0xF0;

	if (IsValid())
	{
		if (offset1 < lowerOffset)
		{
			lowerOffset = offset1;
		}

		if (offset2 > upperOffset)
		{
			upperOffset = offset2;
		}
	}

	else
	{
		lowerOffset = upperOffset = offset;
	}
}

float * VulkanCommandBuffer::PushConstantState::GetData( U32 offset )
{
	U8 * bytes = reinterpret_cast< U8 * >( fData );

	return reinterpret_cast< float * >( bytes + offset );
}

bool VulkanCommandBuffer::OffscreenNode::operator < (const OffscreenNode & other) const
{
	if (fHeight == other.fHeight)
	{
		return fOrder > other.fOrder;	// follow sequence as is
	}

	else
	{
		return fHeight < other.fHeight; // lower heights depend on upper ones
	}

	// we pull these off in reverse, thus the backward operations
}

// ----------------------------------------------------------------------------

} // namespace Rtt

#undef READ_UNIFORM_DATA
#undef READ_UNIFORM_DATA_WITH_PROGRAM
#undef CHECK_ERROR_AND_BREAK
#undef WRITE_COMMAND
#undef DEBUG_PRINT
#undef DEBUG_PRINT_MATRIX

// ----------------------------------------------------------------------------
