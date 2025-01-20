//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Renderer/Rtt_VulkanRenderer.h"
#include "Renderer/Rtt_VulkanContext.h"
#include "Renderer/Rtt_VulkanGeometry.h"

#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Core/Rtt_Assert.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

VulkanGeometry::VulkanGeometry( VulkanContext * context )
:	fContext( context ),
	fVertexBufferData( NULL ),
	fIndexBufferData( NULL ),
	fResource( NULL ),
	fMappedVertices( NULL ),
	fVertexCount( 0U ),
	fIndexCount( 0U )
{
}

VulkanGeometry::~VulkanGeometry()
{
	Destroy(); // TODO: not being put on destroy queue... bug?
}

void
VulkanGeometry::Create( CPUResource* resource )
{
	Rtt_ASSERT( CPUResource::kGeometry == resource->GetType() );
	Geometry* geometry = static_cast< Geometry * >( resource );
	
	fVertexCount = geometry->GetVerticesAllocated();
	fIndexCount = geometry->GetIndicesAllocated();

	if ( geometry->GetStoredOnGPU() )
	{
		fVertexBufferData = CreateBufferOnGPU( fVertexCount * sizeof( Geometry::Vertex ), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT );

		TransferToGPU( fVertexBufferData->GetBuffer(), geometry->GetVertexData(), geometry->GetVerticesUsed() * sizeof( Geometry::Vertex ) );

		Geometry::Index * indices = geometry->GetIndexData();

		if (indices)
		{
			fIndexBufferData = CreateBufferOnGPU( fIndexCount * sizeof( Geometry::Index ), VK_BUFFER_USAGE_INDEX_BUFFER_BIT );

			TransferToGPU( fIndexBufferData->GetBuffer(), indices, geometry->GetIndicesUsed() * sizeof( Geometry::Index ) );
		}
	}

	else
	{
        VulkanBufferData bufferData( fContext->GetDevice(), fContext->GetAllocator() );

		fContext->CreateBuffer( fVertexCount * sizeof( Geometry::Vertex ), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferData );

		fMappedVertices = fContext->MapData( bufferData.GetMemory() );
		fVertexBufferData = bufferData.Extract( NULL );

		fResource = geometry;
	}
}

void
VulkanGeometry::Update( CPUResource* resource )
{
	Rtt_ASSERT( CPUResource::kGeometry == resource->GetType() );
	Geometry* geometry = static_cast<Geometry*>( resource );
	const Geometry::Vertex* vertexData = geometry->GetVertexData();

	if ( !fMappedVertices )
	{
		// The user may have resized the given Geometry instance
		// since the last call to update (see Geometry::Resize()).
		if ( fVertexCount < geometry->GetVerticesAllocated() ||
			 fIndexCount < geometry->GetIndicesAllocated() )
		{
			Destroy();
			Create( resource );
		}
		
		// Copy the vertex data from main memory to GPU memory.
		else if ( vertexData )
		{
			TransferToGPU( fVertexBufferData->GetBuffer(), vertexData, geometry->GetVerticesUsed() * sizeof( Geometry::Vertex ) );
		}

		else
		{
			Rtt_TRACE_SIM(( "ERROR: Unable to update GPU geometry. Data is NULL" ));
		}
	}

	else
	{
		memcpy( fMappedVertices, vertexData, geometry->GetVerticesUsed() * sizeof( Geometry::Vertex ) );
	}
}

void
VulkanGeometry::Destroy()
{
	if (fMappedVertices)
	{
		vkUnmapMemory( fContext->GetDevice(), fVertexBufferData->GetMemory() );

		fMappedVertices = NULL;
	}

	Rtt_DELETE( fVertexBufferData );
	Rtt_DELETE( fIndexBufferData );

	fVertexBufferData = fIndexBufferData = NULL;
	fResource = NULL;
}

void 
VulkanGeometry::Bind( VulkanRenderer & renderer, VkCommandBuffer commandBuffer )
{
	U32 bindingID = 0U; // n.b. for future use?

	VkVertexInputBindingDescription description;

	description.binding = 0U;
	description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	description.stride = sizeof( Geometry::Vertex );

	std::vector< VkVertexInputBindingDescription > inputBindingDescriptions;

	inputBindingDescriptions.push_back( description );

	renderer.SetBindingDescriptions( bindingID, inputBindingDescriptions );

	VkBuffer vertexBuffer = fVertexBufferData->GetBuffer();
	VkDeviceSize offset = 0U;

	vkCmdBindVertexBuffers( commandBuffer, 0U, 1U, &vertexBuffer, &offset );

	if (fIndexBufferData != VK_NULL_HANDLE)
	{
		vkCmdBindIndexBuffer( commandBuffer, fIndexBufferData->GetBuffer(), 0U, VK_INDEX_TYPE_UINT16 );
	}
}

void
VulkanGeometry::Populate()
{
	if (fResource && fResource->GetVerticesUsed())
	{
		Rtt_ASSERT( fMappedVertices );

		memcpy( fMappedVertices, fResource->GetVertexData(), fResource->GetVerticesUsed() * sizeof( Geometry::Vertex ) );
	}
}

VulkanBufferData *
VulkanGeometry::CreateBufferOnGPU( VkDeviceSize bufferSize, VkBufferUsageFlags usage )
{
    VulkanBufferData bufferData( fContext->GetDevice(), fContext->GetAllocator() );
    
	fContext->CreateBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferData );

	return bufferData.Extract( NULL );
}

bool
VulkanGeometry::TransferToGPU( VkBuffer bufferOnGPU, const void * data, VkDeviceSize bufferSize )
{
    VulkanBufferData stagingData( fContext->GetDevice(), fContext->GetAllocator() );
    bool ableToTransfer = fContext->CreateBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingData );

	if (ableToTransfer)
	{
		fContext->StageData( stagingData.GetMemory(), data, bufferSize );
		fContext->CopyBuffer( stagingData.GetBuffer(), bufferOnGPU, bufferSize );

		// TODO: lifetime issue...
	}

	return ableToTransfer;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------