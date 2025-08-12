//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_Renderer.h"

#include "Renderer/Rtt_CommandBuffer.h"
#include "Renderer/Rtt_FrameBufferObject.h"
#include "Renderer/Rtt_FormatExtensionList.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_GeometryPool.h"
#include "Renderer/Rtt_GPUResource.h"
#include "Renderer/Rtt_HighPrecisionTime.h"
#include "Renderer/Rtt_Matrix_Renderer.h"
#include "Renderer/Rtt_Program.h"
#include "Renderer/Rtt_RenderData.h"
#include "Renderer/Rtt_CPUResource.h"
#include "Renderer/Rtt_Texture.h"
#include "Renderer/Rtt_Uniform.h"
#include "Core/Rtt_Allocator.h"
#include "Core/Rtt_Assert.h"
#include "Core/Rtt_Math.h"
#include "Core/Rtt_Types.h"
#include "Renderer/Rtt_MCPUResourceObserver.h"
#include "Display/Rtt_ObjectHandle.h"
#include "Display/Rtt_ShaderData.h"
#include "Display/Rtt_ShaderResource.h"

#include "Rtt_GPUStream.h"
#include "Corona/CoronaGraphics.h"

#include "Rtt_Profiling.h"

#define ENABLE_DEBUG_PRINT	0

#include <limits>

// ----------------------------------------------------------------------------

namespace /*anonymous*/
{
    // To avoid the cost of a system call, return zero if stats are disabled.
    #define START_TIMING() fStatisticsEnabled ? Rtt_GetPreciseAbsoluteTime() : 0;

	// To avoid loss of precision, Rtt_AbsoluteToMilliseconds() is not used
	// here when converting to milliseconds because it uses integer division.
	#define STOP_TIMING(start) fStatisticsEnabled ? Rtt_PreciseAbsoluteToMilliseconds( Rtt_GetPreciseAbsoluteTime() - start ) : 0.0f;

    // Used to track the frequency of certain events for profiling purposes.
    #define INCREMENT( var ) if( fStatisticsEnabled ) ++var;
    #define INCREMENT_N( var, count ) if( fStatisticsEnabled ) var += count;
    
    // Used to log interaction with Rtt_Renderer through its public interface
    // (but not any implicit actions taken as a result of those interactions).
    #if ENABLE_DEBUG_PRINT
        #define DEBUG_PRINT( ... )    Rtt_LogException( __VA_ARGS__ );
    #else
        #define DEBUG_PRINT( ... )
    #endif

    // ...
    U32 ComputeRequiredVertices( Rtt::Geometry* geometry, bool wireframe )
    {
        if( wireframe )
        {
            switch( geometry->GetPrimitiveType() )
            {
                case Rtt::Geometry::kTriangleStrip:
                case Rtt::Geometry::kTriangleFan:
                    return 2 * ( 2 * ( geometry->GetVerticesUsed() - 2 ) + 1);
                case Rtt::Geometry::kIndexedTriangles:
                    return 2 * geometry->GetIndicesUsed();
                case Rtt::Geometry::kTriangles:
                    return 2 * geometry->GetVerticesUsed();
                default:
                    return geometry->GetVerticesUsed();
            }
        }
        else
        {
            switch( geometry->GetPrimitiveType() )
            {
                case Rtt::Geometry::kTriangleStrip:
                    return geometry->GetVerticesUsed() + 2;
                default:
                    return geometry->GetVerticesUsed();
            }
        }
    }
}

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

struct StateBlockInfo {
    CoronaStateBlockDirty fChanged;
    CoronaStateBlockDirty fRestore;
    void * fData;
    U32 fOffset;
    U32 fSize;
    bool fIgnoredByHash;
};

struct CustomGraphicsInfo
{
    CustomGraphicsInfo( Rtt_Allocator* allocator )
    :   fCommands( allocator ),
        fStateBlocks( allocator )
    {
    }

    PtrArray< const CoronaCommand > fCommands;
    PtrArray< StateBlockInfo > fStateBlocks;
};

// NOT USED: const U32 kElementsPerMat4 = 16;

Renderer::Statistics::Statistics()
:    fResourceCreateTime( 0.0f ),
    fResourceUpdateTime( 0.0f ),
    fResourceDestroyTime( 0.0f ),
    fPreparationTime( 0.0f ),
    fRenderTimeCPU( 0.0f ),
    fRenderTimeGPU( 0.0f ),
    fDrawCallCount( 0 ),
    fTriangleCount( 0 ),
    fLineCount( 0 ),
    fGeometryBindCount( 0 ),
    fProgramBindCount( 0 ),
    fTextureBindCount( 0 ),
    fUniformBindCount( 0 )
{
}

void Renderer::Statistics::Log() const
{
    //Make sure Statistics are enabled before calling!
    Rtt_LogException("PrepTime(%3.2f) CPUTime(%3.2f) GPUTime(%3.2f)",fPreparationTime, fRenderTimeCPU, fRenderTimeGPU );
    Rtt_LogException("\tDrawCount(%d) TriangleCount(%d) LineCount(%d)\n", fDrawCallCount, fTriangleCount, fLineCount );
    Rtt_LogException("\tResourceTimes (create, update, destroy) = (%3.2f, %3.2f, %3.2f)\n", fResourceCreateTime, fResourceUpdateTime, fResourceDestroyTime );
}

Renderer::Renderer( Rtt_Allocator* allocator )
:    fAllocator( allocator ),
    fCreateQueue( allocator ),
    fUpdateQueue( allocator ),
    fDestroyQueue( allocator ),
    fCPUResourceObserver(NULL),
    fGeometryPool( Rtt_NEW( fAllocator, GeometryPool( fAllocator ) ) ),
    fInstancingGeometryPool( Rtt_NEW( fAllocator, GeometryPool( fAllocator ) ) ),
    fFrontCommandBuffer( NULL ),
    fBackCommandBuffer( NULL ),
    fTotalTime( Rtt_NEW( fAllocator, Uniform( fAllocator, Uniform::kScalar ) ) ),
    fDeltaTime( Rtt_NEW( fAllocator, Uniform( fAllocator, Uniform::kScalar ) ) ),
    fTexelSize( Rtt_NEW( fAllocator, Uniform( fAllocator, Uniform::kVec4 ) ) ),
    fContentScale( Rtt_NEW( fAllocator, Uniform( fAllocator, Uniform::kVec2 ) ) ),
    fViewProjectionMatrix( Rtt_NEW( fAllocator, Uniform( fAllocator, Uniform::kMat4 ) ) ),
	fCaptureGroups( allocator ),
	fCaptureRects( allocator ),
    fDefaultState( allocator ),
    fCurrentState( allocator ),
    fWorkingState( allocator ),
    fCustomInfo( Rtt_NEW( fAllocator, CustomGraphicsInfo( fAllocator ) ) ),
    fSyncedCount( 0U ),
    fMaybeDirty( false ),
    fGeometryWriters( allocator ),
    fCurrentGeometryWriterList( NULL ),
    fCanAddGeometryWriters( false ),
    fMaskCountIndex( 0 ),
    fMaskCount( allocator ),
    fCurrentProgramMaskCount( 0 ),
    fWireframeEnabled( false ),
    fStatisticsEnabled( false ),
    fScissorEnabled( false ),
    fMultisampleEnabled( false ),
    fFrameBufferObject( NULL ),
    fInsertionLimit( (std::numeric_limits<U32>::max)() ),
    fRenderDataCount( 0 ),
	fVertexOffset( 0 ),
	fCurrentGeometry( NULL ),
    fTimeDependencyCount( 0 )
{
    // Always have at least 1 mask count.
    fMaskCount.Append( 0 );
}

Renderer::~Renderer()
{
    Rtt_DELETE( fGeometryPool );
    Rtt_DELETE( fInstancingGeometryPool );

    DestroyQueuedGPUResources();
    
    Rtt_DELETE( fBackCommandBuffer );
    Rtt_DELETE( fFrontCommandBuffer );
    
    Rtt_DELETE( fCustomInfo );

    Rtt_DELETE( fTotalTime );
    Rtt_DELETE( fDeltaTime );
    Rtt_DELETE( fTexelSize );
    Rtt_DELETE( fContentScale );
    Rtt_DELETE( fViewProjectionMatrix );
    
    if (fCPUResourceObserver)
    {
        Rtt_DELETE( fCPUResourceObserver );
    }
}

void
Renderer::Initialize()
{
    fBackCommandBuffer->Initialize();
    fFrontCommandBuffer->Initialize();
}

void
Renderer::BeginFrame( Real totalTime, Real deltaTime, const TimeTransform *defTimeTransform, Real contentScaleX, Real contentScaleY, bool )
{
    fContentScaleX = contentScaleX;
    fContentScaleY = contentScaleY;

    // NOTE: No nested calls allowed

    fPrevious = RenderData();
    fVertexOffset = 0;
    fVertexCount = 0;
    fVertexExtra = 0;
    fIndexOffset = 0;
    fIndexCount = 0;
    fRenderDataCount = 0;
    fPreviousPrimitiveType = Geometry::kTriangleStrip;
    fCurrentVertex = NULL;
    fCurrentGeometry = NULL;
    fCurrentInstancingVertex = NULL;
    fCurrentInstancingGeometry = NULL;
    
    fMaskCountIndex = 0;
    fMaskCount[0] = 0;
    fInsertionCount = 0;
    
    SetGeometryWriters( NULL, 0 );

    fStatistics = Statistics();
    fStartTime = START_TIMING();

    fTotalTime->SetValue( totalTime );
    fBackCommandBuffer->BindUniform( fTotalTime, Uniform::kTotalTime );

    fContentScale->SetValue( contentScaleX, contentScaleY );
    fBackCommandBuffer->BindUniform( fContentScale, Uniform::kContentScale );

    fDeltaTime->SetValue( deltaTime );
    fBackCommandBuffer->BindUniform( fDeltaTime, Uniform::kDeltaTime );
    
    fBackCommandBuffer->ClearUserUniforms();
    fBackCommandBuffer->PrepareTimeTransforms( defTimeTransform );
    
    fBackCommandBuffer->SetBlendEnabled( fPrevious.fBlendEquation != RenderTypes::kDisabledEquation );
    fBackCommandBuffer->SetBlendFunction( fPrevious.fBlendMode );
    fBackCommandBuffer->SetBlendEquation( fPrevious.fBlendEquation );

    fTimeDependencyCount = 0;
    
    DEBUG_PRINT( "--Begin Frame: Renderer--\n" );
}

void
Renderer::EndFrame()
{
    CheckAndInsertDrawCommand();

    // We usually want some default state when a frame starts, so
    // invoke "restore"-style handlers for any mismatches. This is
    // done here, rather than the beginning of the frame, to better
    // accommodate captures, relaunches, and so on.
    RestoreDefaultBlocks();

    fStatistics.fPreparationTime = STOP_TIMING(fStartTime);
    
    DEBUG_PRINT( "--End Frame: Renderer--\n\n" );
}


void
Renderer::BeginDrawing()
{
	fBackCommandBuffer->WillRender();
}

void
Renderer::CaptureFrameBuffer( RenderingStream & stream, BufferBitmap & bitmap, S32 x_in_pixels, S32 y_in_pixels, S32 w_in_pixels, S32 h_in_pixels )
{
	stream.CaptureFrameBuffer( bitmap,
		x_in_pixels,
		y_in_pixels,
		w_in_pixels,
		h_in_pixels );
}

void 
Renderer::GetFrustum( Real* viewMatrix, Real* projMatrix ) const
{
    const U32 ELEMENTS_PER_MAT4 = 16;
    memcpy( viewMatrix, fViewMatrix, ELEMENTS_PER_MAT4 * sizeof( Real ) );
    memcpy( projMatrix, fProjMatrix, ELEMENTS_PER_MAT4 * sizeof( Real ) );
}

void
Renderer::SetFrustum( const Real* viewMatrix, const Real* projMatrix )
{
    Rtt_ASSERT( viewMatrix );
    Rtt_ASSERT( projMatrix );

    const U32 ELEMENTS_PER_MAT4 = 16;
    memcpy( fViewMatrix, viewMatrix, ELEMENTS_PER_MAT4 * sizeof( Real ) );
    memcpy( fProjMatrix, projMatrix, ELEMENTS_PER_MAT4 * sizeof( Real ) );

    CheckAndInsertDrawCommand();
    Real* data = reinterpret_cast<Real*>( fViewProjectionMatrix->GetData() );
    Multiply4x4( projMatrix, viewMatrix, data );
    fViewProjectionMatrix->Invalidate();
    fBackCommandBuffer->BindUniform( fViewProjectionMatrix, Uniform::kViewProjectionMatrix );
    
    DEBUG_PRINT( "Set frustum: view=%p, projection=%p\n", viewMatrix, projMatrix );
}

void
Renderer::GetViewport( S32& x, S32& y, S32& width, S32& height ) const
{
    x = fViewport[0];
    y = fViewport[1];
    width = fViewport[2];
    height = fViewport[3];
}

void
Renderer::SetViewport( S32 x, S32 y, S32 width, S32 height )
{
    fViewport[0] = x;
    fViewport[1] = y;
    fViewport[2] = width;
    fViewport[3] = height;

    CheckAndInsertDrawCommand();
    fBackCommandBuffer->SetViewport( x, y, width, height );
    
    DEBUG_PRINT( "Set viewport: x=%i, y=%i, width=%i, height=%i\n", x, y, width, height );
}



void
Renderer::GetScissor( S32& x, S32& y, S32& width, S32& height ) const
{
    x = fScissor[0];
    y = fScissor[1];
    width = fScissor[2];
    height = fScissor[3];
}

void
Renderer::SetScissor( S32 x, S32 y, S32 width, S32 height )
{
    fScissor[0] = x;
    fScissor[1] = y;
    fScissor[2] = width;
    fScissor[3] = height;

    // Multiply bounds by view-projection matrix to account for content scaling
    Real corner0[] = { static_cast<Real>( x ), static_cast<Real>( y ), 0.0f, 1.0f };
    Real corner1[] = { static_cast<Real>( x + width ), static_cast<Real>( y + height ), 0.0f, 1.0f };

    Real* viewProjMatrix = reinterpret_cast<Real*>( fViewProjectionMatrix->GetData() );
    MultiplyVec4Mat4( corner0, viewProjMatrix, corner0 );
    MultiplyVec4Mat4( corner1, viewProjMatrix, corner1 );

    Real windowCoord0[2];
    Real windowCoord1[2];
    ClipToWindow( corner0, fViewport[2], fViewport[3], windowCoord0 );
    ClipToWindow( corner1, fViewport[2], fViewport[3], windowCoord1 );

    CheckAndInsertDrawCommand();

    S32 x0 = static_cast<S32>( windowCoord0[0] );
    S32 y0 = static_cast<S32>( windowCoord0[1] );
    S32 x1 = static_cast<S32>( windowCoord1[0] );
    S32 y1 = static_cast<S32>( windowCoord1[1] );
    fBackCommandBuffer->SetScissorRegion( x0, Min( y0, y1 ), x1 - x0, abs( y1 - y0 ) );
    
    DEBUG_PRINT( "Set scissor window: x=%i, y=%i, width=%i, height=%i\n", x, y, width, height );
}

bool
Renderer::GetScissorEnabled() const
{
    return fScissorEnabled;
}

void
Renderer::SetScissorEnabled( bool enabled )
{
    fScissorEnabled = enabled;
    CheckAndInsertDrawCommand();
    fBackCommandBuffer->SetScissorEnabled( enabled );
    
    DEBUG_PRINT( "Enabled scissor testing\n" );
}

bool
Renderer::GetMultisampleEnabled() const
{
    return fMultisampleEnabled;
}

void
Renderer::SetMultisampleEnabled( bool enabled )
{
    fMultisampleEnabled = enabled;
    CheckAndInsertDrawCommand();
    fBackCommandBuffer->SetMultisampleEnabled( enabled );
    
    DEBUG_PRINT( "Enabled multisample testing\n" );
}

FrameBufferObject*
Renderer::GetFrameBufferObject() const
{
    return fFrameBufferObject;
}

void
Renderer::SetFrameBufferObject( FrameBufferObject* fbo )
{
    fFrameBufferObject = fbo;

    FlushBatch();

    if( fbo )
    {
        Texture* texture = fbo->GetTexture();
        if( !texture->fGPUResource )
        {
            QueueCreate( texture );
        }

        if( !fbo->fGPUResource )
        {
            QueueCreate( fbo );
        }
    }
    fBackCommandBuffer->BindFrameBufferObject( fbo );
    
    DEBUG_PRINT( "Bind FrameBufferObject: %p\n", fbo );
}

void
Renderer::Clear( Real r, Real g, Real b, Real a, const ExtraClearOptions * extraOptions ) 
{
    CheckAndInsertDrawCommand();

    if (extraOptions && extraOptions->clearDepth)
    {
        fBackCommandBuffer->ClearDepth( extraOptions->depthClearValue );

        DEBUG_PRINT( "Clear (depth): %f\n", extraOptions->depthClearValue );
    }
    
    if (extraOptions && extraOptions->clearStencil)
    {
        fBackCommandBuffer->ClearStencil( extraOptions->stencilClearValue );
        
        DEBUG_PRINT( "Clear (stencil): %i\n", extraOptions->stencilClearValue );
    }

    fBackCommandBuffer->Clear( r, g, b, a );
    
    DEBUG_PRINT( "Clear: r=%f, g=%f, b=%f, a=%f\n", r, g, b, a );
}

void
Renderer::PushMask( Texture* maskTexture, Uniform* maskMatrix )
{
    CheckAndInsertDrawCommand();
    
    ++MaskCount();
    BindTexture( maskTexture, Texture::kMask0 + MaskCount() - 1 );
    BindUniform( maskMatrix, Uniform::kMaskMatrix0 + MaskCount() - 1 );

    fPrevious.fMaskTexture = maskTexture;
    fPrevious.fMaskUniform = maskMatrix;
    
    DEBUG_PRINT( "Push mask: texture=%p, uniform=%p\n", maskTexture, maskMatrix );
}

void
Renderer::PopMask()
{
    --MaskCount();

    // fCurrentProgramMaskCount is used to track batches. Thing is if we pop and then push new mask, it thinks we're in same batch.
    // resetting fCurrentProgramMaskCount helps to prevent differently masked draws go in same batch. This should still prevent empty masks breaking batches
    if(fCurrentProgramMaskCount > MaskCount())
    {
        fCurrentProgramMaskCount = (U32)-1;
    }

    DEBUG_PRINT( "Pop mask\n" );
}

void
Renderer::PushMaskCount()
{
    ++fMaskCountIndex;
    
    // Always reset to 0
    if ( fMaskCountIndex < fMaskCount.Length() )
    {
        MaskCount() = 0;
    }
    else
    {
        // Grow if necessary
        fMaskCount.Append( 0 );
    }
}

void
Renderer::PopMaskCount()
{
    Rtt_ASSERT( fMaskCountIndex > 0 );

    --fMaskCountIndex;
}

void
Renderer::Insert( const RenderData* data, const ShaderData * shaderData )
{
    // For debug visualization, the number of insertions may be limited
    if( fInsertionCount++ > fInsertionLimit )
    {
        return;
    }
    
    // Derived Renderers are required to allocate CommandBuffers
    Rtt_ASSERT( fBackCommandBuffer != NULL );
    Rtt_ASSERT( fFrontCommandBuffer != NULL );

	bool blendDirty = data->fBlendMode != fPrevious.fBlendMode;
	bool blendEquationDirty = data->fBlendEquation != fPrevious.fBlendEquation;
	bool fillDirty0 = data->fFillTexture0 != fPrevious.fFillTexture0 && data->fFillTexture0;
	bool fillDirty1 = data->fFillTexture1 != fPrevious.fFillTexture1 && data->fFillTexture1;
	bool maskTextureDirty = data->fMaskTexture != fPrevious.fMaskTexture; // since PushMask() can stomp on the previous texture, a "not NULL" check here is unreliable
	bool maskUniformDirty = data->fMaskUniform != fPrevious.fMaskUniform; // ...ditto
	bool programDirty = data->fProgram != fPrevious.fProgram || MaskCount() != fCurrentProgramMaskCount;
	bool userUniformDirty0 = data->fUserUniform0 != fPrevious.fUserUniform0 && data->fUserUniform0;
	bool userUniformDirty1 = data->fUserUniform1 != fPrevious.fUserUniform1 && data->fUserUniform1;
	bool userUniformDirty2 = data->fUserUniform2 != fPrevious.fUserUniform2 && data->fUserUniform2;
	bool userUniformDirty3 = data->fUserUniform3 != fPrevious.fUserUniform3 && data->fUserUniform3;
	

    ArrayS32 dirtyIndices( fAllocator );
    U32 largestDirtySize = EnumerateDirtyBlocks( dirtyIndices );

	Geometry* geometry = data->fGeometry;
	Rtt_ASSERT( geometry );
	fDegenerateVertexCount = 0;

    const ShaderResource* shaderResource = data->fProgram->GetShaderResource();
    const FormatExtensionList* programList = shaderResource->GetExtensionList();
    const FormatExtensionList *previousGeometryList = NULL;
    
    if (NULL != fPrevious.fGeometry)
    {
        previousGeometryList = fPrevious.fGeometry->GetExtensionList();
    }

    const FormatExtensionList* extensionList = geometry->GetExtensionList();
    const U32 vertexExtra = extensionList ? extensionList->ExtraVertexCount() : 0;
    bool formatsDirty = !FormatExtensionList::Match( previousGeometryList, extensionList );

    if (!formatsDirty && data->fProgram != fPrevious.fProgram)
    {
        const FormatExtensionList* previousProgramList = NULL;
        
        if (fPrevious.fProgram)
        {
            previousProgramList = fPrevious.fProgram->GetShaderResource()->GetExtensionList();
        }
        
        formatsDirty = !FormatExtensionList::Match( previousProgramList, programList );
    }

    const Geometry::ExtensionBlock* block = geometry->GetExtensionBlock();
    bool isInstanced = Geometry::UsesInstancing( block, extensionList );
	bool mustReconcileFormats = formatsDirty;

    // Geometry that is stored on the GPU does not need to be copied
    // over each frame. As a consequence, they can not be batched.
    if( geometry->GetStoredOnGPU() && !fWireframeEnabled )
    {
        FlushBatch();

        if( geometry != fPrevious.fGeometry )
        {
            if( !geometry->fGPUResource )
            {
                QueueCreate( geometry );
            }
        
            fBackCommandBuffer->BindGeometry( geometry );
            fPrevious.fGeometry = geometry;

            if (isInstanced)
            {
                fBackCommandBuffer->BindInstancing( block->fCount, NULL );
            }
			
			mustReconcileFormats = true; // geometry is new
        }

        fCachedVertexOffset = fVertexOffset;
        fCachedVertexCount = fVertexCount;
        fCachedVertexExtra = fVertexExtra;
        fOffsetCorrection = 0;
        fVertexExtra = 0;
        fVertexOffset = 0;
        fVertexCount = geometry->GetVerticesUsed();
        fIndexCount = geometry->GetIndicesUsed();
        fPreviousPrimitiveType = geometry->GetPrimitiveType();
    }
    else
    {
        bool batch =
            !( blendDirty
                || blendEquationDirty
                || fillDirty0
                || fillDirty1
                || maskTextureDirty
                || maskUniformDirty
                || programDirty
                || userUniformDirty0
                || userUniformDirty1
                || userUniformDirty2
                || userUniformDirty3
                || formatsDirty
				|| fCaptureGroups.Length() > 0 
                || dirtyIndices.Length() > 0 );

        // Only triangle strips are batched. All other primitive types
        // force the previous batch to draw and a new one to be started.
        Geometry::PrimitiveType primitiveType = geometry->GetPrimitiveType();
        if( primitiveType != fPreviousPrimitiveType || primitiveType != Geometry::kTriangleStrip )
        {
            batch = false;
        }

        // Instanced draws will also break batching.
        if (isInstanced)
        {
            batch = false;
        }
    
        else if (fPrevious.fProgram)
        {
            const FormatExtensionList* previousProgramList = fPrevious.fProgram->GetShaderResource()->GetExtensionList();
            
            if (previousProgramList && previousProgramList->HasInstanceRateData())
            {
                batch = false;
            }
        }
        
        // If the previous RenderData had its Geometry stored on the GPU,
        // then the current RenderData must begin a new batch.
        bool storedOnGPU = fPrevious.fGeometry && fPrevious.fGeometry->GetStoredOnGPU();
        if( storedOnGPU )
        {
            batch = false;
            mustReconcileFormats = true; // pointers out of date
        }
        fPrevious.fGeometry = geometry;
        
        // Depending on batching, wireframe, etc, the amount of space
        // needed may be more than what is used by the Geometry itself.
        const U32 verticesComputed = ComputeRequiredVertices( geometry, fWireframeEnabled );
        const U32 verticesRequired = verticesComputed * (1 + vertexExtra);
//        bool enoughSpace = fCurrentGeometry;
//        if ( enoughSpace )
//        {
//            U32 geometryLimit = Min( (U32)256, ( fCurrentGeometry->GetVerticesAllocated() - fCurrentGeometry->GetVerticesUsed() ) );
//            enoughSpace = verticesRequired <= geometryLimit;
//        }

        bool enoughSpace = fCurrentGeometry && verticesRequired <=
         ( fCurrentGeometry->GetVerticesAllocated() - fCurrentGeometry->GetVerticesUsed() );
        if( !batch || !enoughSpace )
        {
            UpdateBatch( batch, enoughSpace, storedOnGPU, verticesRequired );

            if ( 0 == fVertexOffset )
			{
				mustReconcileFormats = true; // geometry is new
			}
        }
        
        fVertexExtra = vertexExtra;

        // Copy the the incoming vertex data into the current Geometry
        // pool instance, even if the data will not be batched.
        CopyVertexData( geometry, fCurrentVertex, batch && enoughSpace );

        if (isInstanced)
        {
            Rtt_ASSERT( programList && programList->IsInstanced() );

            InsertInstancing( block, programList, extensionList );
            
            mustReconcileFormats = true; // pointers out of date
        }

        fCurrentVertex += verticesRequired;
        fVertexCount += verticesComputed;
        fCurrentGeometry->SetVerticesUsed( fCurrentGeometry->GetVerticesUsed() + verticesRequired );

        // Update previous batch
        fPreviousPrimitiveType = primitiveType;
        
        if( fWireframeEnabled )
        {
            if( fPreviousPrimitiveType != Geometry::kLineLoop )
            {
                fPreviousPrimitiveType = Geometry::kLines;
            }
            
            if( geometry->GetStoredOnGPU() && !geometry->fGPUResource )
            {
                QueueCreate( geometry );
            }
        }
    }
    fRenderDataCount++;
    
    // Blend mode
    if( data->fBlendMode != fPrevious.fBlendMode )
    {
        fBackCommandBuffer->SetBlendFunction( data->fBlendMode );
        fPrevious.fBlendMode = data->fBlendMode;
    }

    // Blend equation
    if( blendEquationDirty )
    {
        if( fPrevious.fBlendEquation == RenderTypes::kDisabledEquation )
        {
            fBackCommandBuffer->SetBlendEnabled( true );
        }
        else if (data->fBlendEquation == RenderTypes::kDisabledEquation )
        {
            fBackCommandBuffer->SetBlendEnabled( false );
        }

        fBackCommandBuffer->SetBlendEquation( data->fBlendEquation );
        fPrevious.fBlendEquation = data->fBlendEquation;
    }

	// Fill texture [0]
	if( fillDirty0 )
	{
		if( !data->fFillTexture0->fGPUResource )
		{
			QueueCreate( data->fFillTexture0 );
		}

		bool postponeBind = fCaptureGroups.Length() > 0 && !HasFramebufferBlit( NULL );
		if (!postponeBind)
		{
			fBackCommandBuffer->BindTexture( data->fFillTexture0, Texture::kFill0 );
		}

		fPrevious.fFillTexture0 = data->fFillTexture0;
		INCREMENT( fStatistics.fTextureBindCount );

        // TODO: Eliminate duplication with fFillTexture1
        float f0 = 1.0f / (float)data->fFillTexture0->GetWidth();
        float f1 = 1.0f / (float)data->fFillTexture0->GetHeight();

        float f2;
        float f3;
        if( data->fFillTexture0->IsRetina() )
        {
            f2 = ( f0 / fContentScaleX );
            f3 = ( f1 / fContentScaleY );
        }
        else
        {
            f2 = f0;
            f3 = f1;
        }

        fTexelSize->SetValue( f0,
                                f1,
                                f2,
                                f3 );

        fBackCommandBuffer->BindUniform( fTexelSize, Uniform::kTexelSize );
        INCREMENT( fStatistics.fUniformBindCount );
    }

	// Fill texture [1]
	if( fillDirty1 )
	{
		if( !data->fFillTexture1->fGPUResource )
		{
			QueueCreate( data->fFillTexture1 );
		}

        fBackCommandBuffer->BindTexture( data->fFillTexture1, Texture::kFill1 );
        fPrevious.fFillTexture1 = data->fFillTexture1;
        INCREMENT( fStatistics.fTextureBindCount );

        // TODO: Eliminate duplication with above
        // TODO: Need to use a different Uniform since fTexelSize is used for fFillTexture0
        float f0 = 1.0f / (float)data->fFillTexture1->GetWidth();
        float f1 = 1.0f / (float)data->fFillTexture1->GetHeight();
        float f2;
        float f3;
        if( data->fFillTexture1->IsRetina() )
        {
            f2 = ( f0 / fContentScaleX );
            f3 = ( f1 / fContentScaleY );
        }
        else
        {
            f2 = f0;
            f3 = f1;
        }

        fTexelSize->SetValue( f0,
                                f1,
                                f2,
                                f3 );

        fBackCommandBuffer->BindUniform( fTexelSize, Uniform::kTexelSize );
        INCREMENT( fStatistics.fUniformBindCount );
    }

    // Program
    if( data->fMaskTexture )
    {
        ++MaskCount();
    }

    // NOTE: The mask count is incremented just in time to select the correct program version, so we re-compare
    // instead of using programDirty which does the equivalent calculation for batching purposes.
    if( data->fProgram != fPrevious.fProgram || MaskCount() != fCurrentProgramMaskCount )
    {
        if( !data->fProgram->fGPUResource )
        {
            QueueCreate( data->fProgram );
        }

        Program::Version version = fWireframeEnabled ? Program::kWireframe : static_cast<Program::Version>( MaskCount() );
        fBackCommandBuffer->BindProgram( data->fProgram, version );
        fPrevious.fProgram = data->fProgram;
        INCREMENT( fStatistics.fProgramBindCount );
        fCurrentProgramMaskCount = MaskCount();
        
        if (shaderData)
        {
            const CoronaEffectCallbacks * effectCallbacks = shaderResource->GetEffectCallbacks();
        
            if (effectCallbacks && effectCallbacks->shaderBind)
            {
                OBJECT_HANDLE_SCOPE();
                
                OBJECT_HANDLE_STORE( Renderer, renderer, this );
                
                effectCallbacks->shaderBind( renderer, shaderData->GetExtraSpace() );

                if (fMaybeDirty)
                {
                    dirtyIndices.Empty();
                    
                    largestDirtySize = EnumerateDirtyBlocks( dirtyIndices );
                }
            }
        }
    }

	// Mask texture
	if( maskTextureDirty && data->fMaskTexture )
	{
		BindTexture( data->fMaskTexture, Texture::kMask0 + MaskCount() - 1 );
	}
	fPrevious.fMaskTexture = data->fMaskTexture; // avoid NULL textures keeping this value dirty...

	if( maskUniformDirty && data->fMaskUniform )
	{
		BindUniform( data->fMaskUniform, Uniform::kMaskMatrix0 + MaskCount() - 1 );
	}
	fPrevious.fMaskUniform = data->fMaskUniform; // ...as with textures

    if( data->fMaskTexture )
    {
        --MaskCount();
    }

	// User data
	if( userUniformDirty0 )
	{
		BindUniform( data->fUserUniform0, Uniform::kUserData0 );
		fPrevious.fUserUniform0 = data->fUserUniform0;
	}

	if( userUniformDirty1 )
	{
		BindUniform( data->fUserUniform1, Uniform::kUserData1 );
		fPrevious.fUserUniform1 = data->fUserUniform1;
	}

	if( userUniformDirty2 )
	{
		BindUniform( data->fUserUniform2, Uniform::kUserData2 );
		fPrevious.fUserUniform2 = data->fUserUniform2;
	}

    if( userUniformDirty3 )
    {
        BindUniform( data->fUserUniform3, Uniform::kUserData3 );
        fPrevious.fUserUniform3 = data->fUserUniform3;
    }
    
    if (fCaptureGroups.Length() > 0)
	{
		IssueCaptures( data->fFillTexture0 );
	}

    if (mustReconcileFormats)
    {
		if ( !geometry->GetStoredOnGPU() )
		{
			fOffsetCorrection = fVertexOffset;
		}
		
        FormatExtensionList::ReconcileFormats( fAllocator, fBackCommandBuffer, programList, extensionList, fVertexOffset );
    }
    
    if (dirtyIndices.Length() > 0)
    {
        UpdateDirtyBlocks( dirtyIndices, largestDirtySize );
    }
    
    DEBUG_PRINT( "Insert RenderData: data=%p\n", data );
    #if ENABLE_DEBUG_PRINT
        data->Log();
    #endif
}

void
Renderer::Render()
{
    Rtt_AbsoluteTime start = START_TIMING();
    fStatistics.fRenderTimeGPU = fFrontCommandBuffer->Execute( fStatisticsEnabled );
    fStatistics.fRenderTimeCPU = STOP_TIMING(start);
}

void
Renderer::Swap()
{
	ENABLE_SUMMED_TIMING( true );

    // Create GPUResources
    Rtt_AbsoluteTime start = START_TIMING();
    for(S32 i = 0; i < fCreateQueue.Length(); ++i)
    {
        CPUResource* data = fCreateQueue[i];
        GPUResource* gpuResource = data->GetGPUResource();
        gpuResource->Create( data );
    }
    fCreateQueue.Remove(0, fCreateQueue.Length(), false);
    fStatistics.fResourceCreateTime = STOP_TIMING(start);

    // Update GPUResources
    start = START_TIMING();
    for(S32 i = 0; i < fUpdateQueue.Length(); ++i)
    {
        CPUResource* data = fUpdateQueue[i];
        data->GetGPUResource()->Update( data );
    }
    fUpdateQueue.Remove(0, fUpdateQueue.Length(), false);
    fStatistics.fResourceUpdateTime = STOP_TIMING(start);

	ENABLE_SUMMED_TIMING( false );

    // Destroy GPUResources
    start = START_TIMING();
    DestroyQueuedGPUResources();
    fStatistics.fResourceDestroyTime = STOP_TIMING(start);

    CommandBuffer* temp = fFrontCommandBuffer;
    fFrontCommandBuffer = fBackCommandBuffer;
    fBackCommandBuffer = temp;
    fGeometryPool->Swap();
    fInstancingGeometryPool->Swap();

    // Add pending commands
    U16 length = (U16)fCustomInfo->fCommands.Length();

    for (U16 i = fSyncedCount; i < length; ++i)
    {
        fBackCommandBuffer->AddCommand( fCustomInfo->fCommands[i] );
    }

    fSyncedCount = length;
}

void
Renderer::QueueCreate( CPUResource* resource )
{
    Rtt_ASSERT(resource->GetGPUResource() == NULL);
    GPUResource* gpuResource = Create( resource );
    if( gpuResource )
    {
        resource->SetGPUResource( gpuResource );
        resource->fRenderer = this;
        
        //No-OP in null case
        resource->AttachObserver(fCPUResourceObserver);
        fCreateQueue.Append( resource );
    }
}

void
Renderer::SetCPUResourceObserver(MCPUResourceObserver *resourceObserver)
{
    fCPUResourceObserver = resourceObserver;
}

void
Renderer::ReleaseGPUResources()
{
    // Destroy all GPU resources that are currently being used.
    if (fCPUResourceObserver)
    {
        fCPUResourceObserver->ReleaseGPUResources();
    }

    // Destroy all orphaned GPU resources that have been queued for deletion.
    DestroyQueuedGPUResources();
}

void
Renderer::TallyTimeDependency( bool usesTime )
{
	if ( usesTime )
	{
		++fTimeDependencyCount;
	}
}

U16
Renderer::AddCustomCommand( const CoronaCommand & command )
{
    if ( 0xFFFF == (U16)fCustomInfo->fCommands.Length() )
    {
        return 0U;
    }

    const CoronaCommand* copy = Rtt_NEW( fAllocator, CoronaCommand( command ) );

    fBackCommandBuffer->AddCommand( copy );

    fCustomInfo->fCommands.Append( copy );

    return (U16)( fCustomInfo->fCommands.Length() );
}

bool
Renderer::IssueCustomCommand( U16 id, const void * data, U32 size )
{
    if ( id < (U16)fCustomInfo->fCommands.Length() )
    {
        fBackCommandBuffer->IssueCommand( id, data, size );

        return true;
    }

    return false;
}

void
Renderer::InsertCaptureRect( FrameBufferObject * fbo, Texture * texture, const Rect & clipped, const Rect & unclipped )
{
	RectPair pair = {};
	
	pair.fClipped = clipped;
	pair.fUnclipped = unclipped;
	
	fCaptureRects.Append( pair );
	
	RectPair * newPair = &fCaptureRects.WriteAccess()[fCaptureRects.Length() - 1];
	CaptureGroup* captureGroups = fCaptureGroups.WriteAccess(), * group = NULL;
	
	for (S32 i = 0, iMax = fCaptureGroups.Length(); i < iMax; ++i)
	{
		if (captureGroups[i].fFBO == fbo || captureGroups[i].fTexture == texture)
		{
			group = &captureGroups[i];
			
			break;
		}
	}
	
	if (group)
	{
		Rtt_ASSERT( group->fLast );
		
		group->fLast->fNext = newPair;
		group->fLast = newPair;
	}
	
	else
	{
		CaptureGroup group;
		
		group.fFBO = fbo;
		group.fTexture = texture;
		group.fFirst = group.fLast = newPair;
		
		if (NULL == texture->GetGPUResource())
		{
			QueueCreate( texture );
		}
		
		if (fbo && NULL == fbo->GetGPUResource())
		{
			QueueCreate( fbo );
		}
		
		fCaptureGroups.Append( group );
	}
}

U16
Renderer::AddStateBlock( const CoronaStateBlock & block )
{
    if (0xFFFF == fCustomInfo->fStateBlocks.Length())
    {
        return 0U;
    }
    
    S32 length = fCustomInfo->fStateBlocks.Length();
    
    StateBlockInfo info = {};
    
    if (length > 0)
    {
        const StateBlockInfo* lastInfo = fCustomInfo->fStateBlocks.ReadAccess()[length - 1];
        
        info.fOffset = lastInfo->fOffset + lastInfo->fSize;
    }
    
    info.fChanged = block.stateDirty;
    info.fRestore = block.defaultStateDirty;
    info.fData = block.userData;
    info.fSize = block.blockSize;
    info.fIgnoredByHash = block.dontHash;
    
    U32 fullSize = info.fOffset + info.fSize;
    
    fDefaultState.PadToSize( fullSize, 0 );
    fCurrentState.PadToSize( fullSize, 0 );
    fWorkingState.PadToSize( fullSize, 0 );
    
    if (block.defaultContents)
    {
        memcpy( fDefaultState.WriteAccess() + info.fOffset, block.defaultContents, info.fSize );
        memcpy( fCurrentState.WriteAccess() + info.fOffset, block.defaultContents, info.fSize );
        memcpy( fWorkingState.WriteAccess() + info.fOffset, block.defaultContents, info.fSize );
    }
    
    fCustomInfo->fStateBlocks.Append( Rtt_NEW( fAllocator, StateBlockInfo( info ) ) );
    
    return (U16)(length + 1);
}

bool
Renderer::GetStateBlockInfo( U16 id, U8 *& start, U32 & size, bool mightDirty )
{
    if (id < fCustomInfo->fStateBlocks.Length())
    {
        const StateBlockInfo* info = fCustomInfo->fStateBlocks.ReadAccess()[id];

        start = fWorkingState.WriteAccess() + info->fOffset;
        size = info->fSize;

        fMaybeDirty = mightDirty && size > 0;
        
        return true;
    }

    return false;
}

void
Renderer::IssueCaptures( Texture * fill0 )
{
	Rtt_ASSERT( fCaptureGroups.Length() > 0 );
	Rtt_ASSERT( fCaptureRects.Length() > 0 );
	
	bool hasFramebufferBlit = HasFramebufferBlit( NULL );
	FrameBufferObject * oldFBO = NULL;
	Texture * mostRecentTexture = NULL;
	
	if (hasFramebufferBlit)
	{
		oldFBO = GetFrameBufferObject();
	}
	
	else if (fill0) // will need to restore fill texture?
	{
		CaptureGroup* groups = fCaptureGroups.WriteAccess();
		
		for (S32 i = 0, iMax = fCaptureGroups.Length() - 1; i < iMax; ++i) // no need to swap in last spot
		{
			if (groups[i].fTexture == fill0) // found: put in last spot to keep binding
			{
				CaptureGroup temp = groups[iMax];
				
				groups[iMax] = groups[i];
				groups[i] = temp;
				
				break;
			}
		}
	}
	
	const CaptureGroup* groups = fCaptureGroups.ReadAccess();
	
	for (S32 i = 0, iMax = fCaptureGroups.Length(); i < iMax; ++i)
	{
		if (hasFramebufferBlit)
		{
			fBackCommandBuffer->BindFrameBufferObject( groups[i].fFBO, true );
		}
		
		else
		{
			mostRecentTexture = groups[i].fTexture;
		}
		
		for (const RectPair* cur = groups[i].fFirst; cur; cur = cur->fNext)
		{
			fBackCommandBuffer->CaptureRect( groups[i].fFBO, *groups[i].fTexture, cur->fClipped, cur->fUnclipped );
		}
	}
	
	if (hasFramebufferBlit)
	{
		fBackCommandBuffer->BindFrameBufferObject( oldFBO );
	}
	
	else if (fill0 && fill0 != mostRecentTexture) // wasn't copied to? 
	{
		fBackCommandBuffer->BindTexture( fill0, 0 );
	}
	
	fCaptureGroups.Clear();
	fCaptureRects.Clear();
}

void
Renderer::SetGeometryWriters( const GeometryWriter* list, U32 n )
{
    if ( 0 == n || list != fCurrentGeometryWriterList )
    {
        fGeometryWriters.Clear();

        if ( NULL == list )
        {
            list = &GeometryWriter::CopyGeometryWriter();
            n = 1;
        }

        GeometryWriter::MaskBits mask = 0;

        for ( U32 i = 0; i < n; i++ )
        {
            GeometryWriter copy = list[i];

            copy.fSaved = copy.fMask;

            mask |= copy.fMask;

            fGeometryWriters.Append( copy );
        }

        Rtt_ASSERT( GeometryWriter::kAll == mask );

        fCurrentGeometryWriterList = list;
    }
}
        
bool
Renderer::AddGeometryWriter( const GeometryWriter& writer, bool isUpdate )
{
    if ( !fCanAddGeometryWriters )
    {
        Rtt_TRACE_SIM(( "ERROR: geometry writers may only added within a Shader::Draw()'s before or after method" ));

        return false;
    }

    S32 iMax = fGeometryWriters.Length(), insertPos = -1;

    if ( !isUpdate )
    {
        for ( S32 i = 0; i < iMax; i++ )
        {
            // Has "update" writers? insert before them...
            if ( fGeometryWriters[i].fSaved & GeometryWriter::kIsUpdate )
            {
                insertPos = i;

                break;
            }

            // ...else indicate to existing writer that it might be partially overwritten;
            // if this clears the mask entirely, the writer will be disabled.
            fGeometryWriters[i].fMask &= ~writer.fMask; 
        }
    }

    fGeometryWriters.Insert( insertPos, writer );

    if ( isUpdate )
    {
        fGeometryWriters[iMax].fSaved |= GeometryWriter::kIsUpdate;
    }

    return true;
}

Renderer::GeometryWriterRAII::GeometryWriterRAII( Renderer& renderer )
:   fRenderer( renderer )
{
    fRenderer.fCanAddGeometryWriters = true;
}

Renderer::GeometryWriterRAII::~GeometryWriterRAII()
{
    fRenderer.fCanAddGeometryWriters = false;

    auto & writers = fRenderer.fGeometryWriters;

    for ( S32 i = 0, iMax = writers.Length(); i < iMax; i++ )
    {
        // Restore any "established" writers...
        if ( writers[i].fSaved & GeometryWriter::kAll )
        {
            writers[i].fMask = writers[i].fSaved;
            writers[i].fSaved = 0;
        }

        // ...and lop off any "custom" ones.
        else
        {
            writers.Remove( i, iMax - i, false );

            break;
        }
    }
}

void
Renderer::WriteGeometry( void* dstGeomComp, const void* srcGeom, U32 stride, U32 index, U32 count, GeometryWriter::MaskBits validBits )
{
    CoronaGeometryMappingLayout layout = {};
    GeometryWriter::MaskBits mask = 0;

    layout.outStride = stride;

	for ( S32 i = 0, iMax = fGeometryWriters.Length(); i < iMax; i++ )
	{
        const GeometryWriter& cur = fGeometryWriters[i];

        if ( 0 == ( validBits & cur.fMask ) )
        {
            continue;
        }

        mask |= cur.fMask;

		const void* from = cur.fContext;

        if ( NULL == from )
        {
            layout.inStride = stride; // n.b. currently always the case, cf. MergeVertexData() et al.

            from = static_cast< const unsigned char* >( srcGeom ) + index * stride + cur.fOffset; // ditto
        }

        else
        {
            layout.inStride = 0;
        }

		layout.count = cur.fComponents;
		layout.type = (CoronaGeometryAttributeType)cur.fType;

		cur.fWriter( static_cast< unsigned char* >( dstGeomComp ) + cur.fOffset, from, &layout, index, count );
	}

    Rtt_ASSERT( mask );
}

void
Renderer::QueueUpdate( CPUResource* resource )
{
    fUpdateQueue.Append( resource );
}

void
Renderer::QueueDestroy( GPUResource* resource )
{
    fDestroyQueue.Append( resource );
}

void
Renderer::DestroyQueuedGPUResources()
{
    for(S32 i = 0; i < fDestroyQueue.Length(); ++i)
    {
        GPUResource* gpuResource = fDestroyQueue[i];
        gpuResource->Destroy();
        delete gpuResource;
    }
    fDestroyQueue.Remove(0, fDestroyQueue.Length(), false);
}

bool
Renderer::GetWireframeEnabled() const
{
    return fWireframeEnabled;
}

void
Renderer::SetWireframeEnabled( bool enabled )
{
    fWireframeEnabled = enabled;
}

U32
Renderer::GetMaxTextureSize()
{
    U32 result = (U32) CommandBuffer::GetMaxTextureSize();
    return result;
}

const char *
Renderer::GetGlString( const char *s )
{
    return CommandBuffer::GetGlString( s );
}

bool
Renderer::GetGpuSupportsHighPrecisionFragmentShaders()
{
    return CommandBuffer::GetGpuSupportsHighPrecisionFragmentShaders();
}

U32
Renderer::GetMaxUniformVectorsCount()
{
    return CommandBuffer::GetMaxUniformVectorsCount();
}

U32
Renderer::GetMaxVertexTextureUnits()
{
    return CommandBuffer::GetMaxVertexTextureUnits();
}

void
Renderer::GetVertexAttributes( VertexAttributeSupport & support ) const
{
    fBackCommandBuffer->GetVertexAttributes( support );
}

bool
Renderer::HasFramebufferBlit( bool * canScale ) const
{
	return fBackCommandBuffer->HasFramebufferBlit( canScale );
}

bool
Renderer::GetStatisticsEnabled() const
{
    return fStatisticsEnabled;
}

void
Renderer::SetStatisticsEnabled( bool enabled )
{
    fStatisticsEnabled = enabled;
}

const Renderer::Statistics&
Renderer::GetFrameStatistics() const
{
    return fStatistics;
}

U32
Renderer::GetMaximumRenderDataCount() const
{
    return fInsertionLimit;
}

void
Renderer::SetMaximumRenderDataCount( U32 count )
{
    fInsertionLimit = count;
}

void
Renderer::BindTexture( Texture* texture, U32 unit )
{
    if( !texture->fGPUResource )
    {
        QueueCreate( texture );
    }

    fBackCommandBuffer->BindTexture( texture, unit );
    INCREMENT( fStatistics.fTextureBindCount );
}

void
Renderer::BindUniform( Uniform* uniform, U32 unit )
{
    if( !uniform->fGPUResource )
    {
        QueueCreate( uniform );
    }

    fBackCommandBuffer->BindUniform( uniform, unit );
    INCREMENT( fStatistics.fUniformBindCount );
}

void
Renderer::CheckAndInsertDrawCommand()
{
    if( fRenderDataCount != 0 )
    {
        if( fPreviousPrimitiveType == Geometry::kIndexedTriangles )
        {
            fBackCommandBuffer->DrawIndexed( fIndexOffset, fIndexCount, fPreviousPrimitiveType );
        }
        else
        {
            fBackCommandBuffer->Draw( fVertexOffset - fOffsetCorrection, fVertexCount - fDegenerateVertexCount, fPreviousPrimitiveType );
        }
        INCREMENT( fStatistics.fDrawCallCount );

        if( fStatisticsEnabled )
        {
            switch( fPreviousPrimitiveType )
            {
                case Geometry::kTriangleStrip:
                case Geometry::kTriangleFan:
                    fStatistics.fTriangleCount += fVertexCount - ( 2 + fDegenerateVertexCount );
                    break;
                case Geometry::kTriangles:
                    fStatistics.fTriangleCount += fVertexCount / 3;
                    break;
                case Geometry::kIndexedTriangles:
                    fStatistics.fTriangleCount += fIndexCount / 3;
                    break;
                case Geometry::kLines:
                    fStatistics.fLineCount += fVertexCount / 2;
                    break;
                case Geometry::kLineLoop:
                    fStatistics.fLineCount += fVertexCount;
                    break;
                default:
                    Rtt_ASSERT_NOT_REACHED();
            };
        }

        fRenderDataCount = 0;
    }
}

U32
Renderer::EnumerateDirtyBlocks( ArrayS32& dirtyIndices )
{
    U32 largestDirtySize = 0;
    
    if (fMaybeDirty)
    {
        fMaybeDirty = false;
        
        Rtt_ASSERT( fCurrentState.Length() == fWorkingState.Length() );

        const U8* currentState = fCurrentState.ReadAccess();
        const U8* workingState = fWorkingState.ReadAccess();
        
        for (S32 i = 0, iMax = fCustomInfo->fStateBlocks.Length(); i < iMax; ++i)
        {
            if (0 != memcmp( currentState + fCustomInfo->fStateBlocks[i]->fOffset, workingState + fCustomInfo->fStateBlocks[i]->fOffset, fCustomInfo->fStateBlocks[i]->fSize ))
            {
                dirtyIndices.Append( i );
                
                if (fCustomInfo->fStateBlocks[i]->fSize > largestDirtySize)
                {
                    largestDirtySize = fCustomInfo->fStateBlocks[i]->fSize;
                }
            }
        }
    }
    
    return largestDirtySize;
}
    
void
Renderer::UpdateDirtyBlocks( const ArrayS32& dirtyIndices, U32 largestDirtySize )
{
    S32 iMax = dirtyIndices.Length();

    if ( 0 == iMax )
    {
        return;
    }

    Array<U8> newContents( fAllocator ), oldContents( fAllocator );
    
    newContents.Reserve( largestDirtySize );
    oldContents.Reserve( largestDirtySize );
 
    OBJECT_HANDLE_SCOPE();
    
    OBJECT_HANDLE_STORE( CommandBuffer, commandBuffer, fBackCommandBuffer );
    OBJECT_HANDLE_STORE( Renderer, renderer, this );
    
    const U8* workingState = fWorkingState.ReadAccess();
    U8* currentState = fCurrentState.WriteAccess();
    
    for (S32 i = 0; i < iMax; ++i)
    {
        const StateBlockInfo* info = fCustomInfo->fStateBlocks[dirtyIndices[i]];
        
        memcpy( newContents.WriteAccess(), workingState + info->fOffset, info->fSize );
        memcpy( oldContents.WriteAccess(), currentState + info->fOffset, info->fSize );
        memcpy( currentState + info->fOffset, newContents.ReadAccess(), info->fSize );
        
        info->fChanged( commandBuffer, renderer, newContents.ReadAccess(), oldContents.ReadAccess(), info->fSize, false, info->fData );
    }
}
    
void
Renderer::RestoreDefaultBlocks()
{
    S32 iMax = fCustomInfo->fStateBlocks.Length();

    if ( 0 == iMax )
    {
        return;
    }

    OBJECT_HANDLE_SCOPE();
    
    OBJECT_HANDLE_STORE( CommandBuffer, commandBuffer, fBackCommandBuffer );
    OBJECT_HANDLE_STORE( Renderer, renderer, this );
 
    Array< U8 > newContents( fAllocator ), oldContents( fAllocator );
    const U8* defaultState = fDefaultState.ReadAccess();
    const U8* currentState = fCurrentState.ReadAccess();
    bool anyChanged = false;
    
    for (S32 i = 0; i < iMax; ++i)
    {
        const StateBlockInfo* info = fCustomInfo->fStateBlocks[i];
        
        // If the last objects "drawn" in the hierarchy never did an
        // Insert(), any trailing dirties remain uncommitted and the
        // corresponding handlers will not have been invoked. We use
        // the current state as the "old" contents to reflect this.
        if (0 != memcmp( defaultState + info->fOffset, currentState + info->fOffset, info->fSize ))
        {
            anyChanged = true;
            
            oldContents.Reserve( info->fSize );
            newContents.Reserve( info->fSize );
            
            memcpy( newContents.WriteAccess(), defaultState + info->fOffset, info->fSize );
            memcpy( oldContents.WriteAccess(), currentState + info->fOffset, info->fSize );
            
            CoronaStateBlockDirty stateDirty = info->fRestore;
            
            if (!stateDirty)
            {
                stateDirty = info->fChanged;
            }
            
            stateDirty( commandBuffer, renderer, newContents.ReadAccess(), oldContents.ReadAccess(), info->fSize, true, info->fData );
        }
        
        if (anyChanged)
        {
            U32 size = fDefaultState.Length();
            
            memcpy( fCurrentState.WriteAccess(), fDefaultState.ReadAccess(), size );
            memcpy( fWorkingState.WriteAccess(), fDefaultState.ReadAccess(), size );
        }
    }
}
    
void
Renderer::InsertInstancing( const Geometry::ExtensionBlock* block, const FormatExtensionList* programList, const FormatExtensionList* geometryList )
{
    U32 verticesRequired = 0;

    for (auto iter = FormatExtensionList::InstancedGroups( programList ); !iter.IsDone(); iter.Advance())
    {
        const FormatExtensionList::Group* group = iter.GetGroup();
        
        verticesRequired += group->GetVertexCount( block->fCount, iter.GetAttribute() );
    }

    bool enoughSpace = fCurrentInstancingGeometry && verticesRequired <=
     ( fCurrentInstancingGeometry->GetVerticesAllocated() - fCurrentInstancingGeometry->GetVerticesUsed() );
    
    if (verticesRequired > 0 && !enoughSpace)
    {
        fCurrentInstancingGeometry = fInstancingGeometryPool->GetOrCreate( verticesRequired );
        fCurrentInstancingVertex = fCurrentInstancingGeometry->GetVertexData();
    }
    
    fBackCommandBuffer->BindInstancing( block->fCount, verticesRequired > 0 ? fCurrentInstancingVertex : NULL );
    
    for (auto iter = FormatExtensionList::InstancedGroups( programList ); !iter.IsDone(); iter.Advance())
    {
        const FormatExtensionList::Group* programGroup = iter.GetGroup();
        
        // Find the geometry group corresponding to this program group. Merge
        // the corresponding instance data.
        U32 geometryAttributeIndex;
  
        S32 geometryGroupIndex = geometryList->FindCorrespondingInstanceGroup( programGroup, iter.GetAttribute(), &geometryAttributeIndex );

        Rtt_ASSERT( -1 != geometryGroupIndex );
        
        FormatExtensionList::Group geometryGroup = geometryList->GetGroups()[geometryGroupIndex];
        U32 vertexCount = geometryGroup.GetVertexCount( block->fCount, geometryList->GetAttributes() + geometryAttributeIndex );

        if (geometryList->HasVertexRateData())
        {
            --geometryGroupIndex;
        }
        
        const Array<U8>* instanceData = block->fInstanceData[geometryGroupIndex];
    
        memcpy( fCurrentInstancingVertex, instanceData->ReadAccess(), vertexCount * sizeof(Geometry::Vertex) );
        
        fCurrentInstancingVertex += vertexCount;
    }
    
	if (verticesRequired > 0)
	{
		fCurrentInstancingGeometry->SetVerticesUsed( fCurrentInstancingGeometry->GetVerticesUsed() + verticesRequired );
	}
}

void
Renderer::FlushBatch()
{
    bool storedOnGPU = fPrevious.fGeometry && fPrevious.fGeometry->GetStoredOnGPU();

    UpdateBatch( false, NULL != fCurrentGeometry, storedOnGPU, 0 );
}

void
Renderer::UpdateBatch( bool batch, bool enoughSpace, bool storedOnGPU, U32 verticesRequired )
{
    CheckAndInsertDrawCommand();
    
    if( storedOnGPU && !fWireframeEnabled )
    {
        fVertexOffset = fCachedVertexOffset;
        fVertexCount = fCachedVertexCount;
        fVertexExtra = fCachedVertexExtra;

        if( enoughSpace )
        {
            fBackCommandBuffer->BindGeometry( fCurrentGeometry );
        }
    }
    
    fVertexOffset += fVertexCount * (1 + fVertexExtra);
    fVertexCount = 0;
    fIndexCount = 0;

    if( !enoughSpace )
    {
        fCurrentGeometry = fGeometryPool->GetOrCreate( verticesRequired );
        if( !fCurrentGeometry->GetGPUResource() )
        {
            QueueCreate( fCurrentGeometry );
        }

        fCurrentVertex = fCurrentGeometry->GetVertexData();
        fVertexOffset = 0;
        fBackCommandBuffer->BindGeometry( fCurrentGeometry );
        INCREMENT( fStatistics.fGeometryBindCount );
    }
}

void
Renderer::CopyVertexData( Geometry* geometry, Geometry::Vertex* destination, bool interior )
{
    const U32 verticesUsed = geometry->GetVerticesUsed();
    const U32 vertexSize = sizeof(Geometry::Vertex);

    if (0 != fVertexExtra)
    {
        CopyExtendedVertexData( geometry, destination, interior );
    }
    
    else if( fWireframeEnabled )
    {
        // Given the primitive type, convert the vertex data to lines
        switch( geometry->GetPrimitiveType() )
        {
            case Geometry::kTriangleStrip:
                CopyTriangleStripsAsLines( geometry, destination );
                break;
            case Geometry::kTriangleFan:
                CopyTriangleFanAsLines( geometry, destination );
                break;
            case Geometry::kTriangles:
                CopyTrianglesAsLines( geometry, destination );
                break;
            case Geometry::kIndexedTriangles:
                CopyIndexedTrianglesAsLines( geometry, destination );
                break;
            case Geometry::kLineLoop:
                WriteGeometry( fCurrentVertex, geometry->GetVertexData(), vertexSize, 0, verticesUsed );
                /* memcpy( fCurrentVertex, geometry->GetVertexData(), verticesUsed * vertexSize ); */
                break;
            case Geometry::kLines:
                WriteGeometry( fCurrentVertex, geometry->GetVertexData(), vertexSize, 0, verticesUsed );
                /* memcpy( fCurrentVertex, geometry->GetVertexData(), verticesUsed * vertexSize ); */
                break;
        }
    }
    else
    {
        if( geometry->GetPrimitiveType() == Geometry::kTriangleStrip )
        {
            // Triangle strips are batched by adding degenerate triangles
            // at the beginning and end of each strip.
            WriteGeometry( destination + 1, geometry->GetVertexData(), vertexSize, 0, verticesUsed );

            memcpy( destination, destination + 1, vertexSize );

            destination += verticesUsed;

            memcpy( destination + 1, destination, vertexSize );
        /*
            memcpy( destination++, geometry->GetVertexData(), vertexSize );

            memcpy( destination, geometry->GetVertexData(), verticesUsed * vertexSize );
            destination += verticesUsed;

            memcpy( destination++, geometry->GetVertexData() + verticesUsed - 1, vertexSize );
         */
            fDegenerateVertexCount = 1;
        }
        else
        {
            // For data which does not exist on the GPU and is not batched,
            // we still double buffer it to be threadsafe.
            WriteGeometry( fCurrentVertex, geometry->GetVertexData(), vertexSize, 0, verticesUsed );
            /* memcpy( fCurrentVertex, geometry->GetVertexData(), verticesUsed * vertexSize ); */
        }
    }
}

void
Renderer::CopyTriangleStripsAsLines( Geometry* geometry, Geometry::Vertex* destination )
{
    const U32 count = geometry->GetVerticesUsed() - 2;
    const size_t vertexSize = sizeof( Geometry::Vertex );
    const Geometry::Vertex* data = geometry->GetVertexData();

    for( U32 i = 0; i < count; ++i )
    {
        // Line 1
        WriteGeometry( destination, data, vertexSize, i, 2 );

        /* memcpy( destination, &data[i], 2 * vertexSize ); */
        destination += 2;
        
        // Line 2
        WriteGeometry( destination++, data, vertexSize, i );
        WriteGeometry( destination++, data, vertexSize, i + 1 );
    /*
        memcpy( destination++, &data[i], vertexSize );
        memcpy( destination++, &data[i + 2], vertexSize );
    */
    }

    WriteGeometry( destination, data, vertexSize, count, 2 );
    /* memcpy( destination, &data[count], 2 * vertexSize ); */
}

void
Renderer::CopyTriangleFanAsLines( Geometry* geometry, Geometry::Vertex* destination )
{
    const U32 count = geometry->GetVerticesUsed() - 1;
    const size_t vertexSize = sizeof( Geometry::Vertex );
    const Geometry::Vertex* data = geometry->GetVertexData();
    
    for( U32 i = 1; i < count; ++i )
    {
        // Line 1
        WriteGeometry( destination++, data, vertexSize, 0 );
        WriteGeometry( destination++, data, vertexSize, i );
    /*
        memcpy( destination++, &data[0], vertexSize );
        memcpy( destination++, &data[i], vertexSize );
    */
        
        // Line 2
        WriteGeometry( destination++, data, vertexSize, i );
        WriteGeometry( destination++, data, vertexSize, i + 1 );
    /*
        memcpy( destination++, &data[i], vertexSize );
        memcpy( destination++, &data[i + 1], vertexSize );
    */
    }
    
    WriteGeometry( destination++, data, vertexSize, 0 );
    WriteGeometry( destination, data, vertexSize, count );
/*
    memcpy( destination++, &data[0], vertexSize );
    memcpy( destination, &data[count], vertexSize );
*/
}

void
Renderer::CopyTrianglesAsLines( Geometry* geometry, Geometry::Vertex* destination )
{
    const U32 triangleCount = geometry->GetVerticesUsed() / 3;
    const size_t vertexSize = sizeof( Geometry::Vertex );
    const Geometry::Vertex* data = geometry->GetVertexData();
    
    for( U32 i = 0; i < triangleCount; ++i )
    {
        U32 index = i * 3;

        // Line 1
        WriteGeometry( destination++, data, vertexSize, index );
        WriteGeometry( destination++, data, vertexSize, index + 1 );
    /*
        memcpy( destination++, &data[index], vertexSize );
        memcpy( destination++, &data[index + 1], vertexSize );
     */

        // Line 2
        WriteGeometry( destination++, data, vertexSize, index + 1 );
        WriteGeometry( destination++, data, vertexSize, index + 2 );
    /*
        memcpy( destination++, &data[index + 1], vertexSize );
        memcpy( destination++, &data[index + 2], vertexSize );
    */

        // Line 3
        WriteGeometry( destination++, data, vertexSize, index + 2 );
        WriteGeometry( destination++, data, vertexSize, index );
    /*
        memcpy( destination++, &data[index + 2], vertexSize );
        memcpy( destination++, &data[index], vertexSize );
    */
    }
}

void
Renderer::CopyIndexedTrianglesAsLines( Geometry* geometry, Geometry::Vertex* destination )
{
    const Geometry::Vertex* vertexData = geometry->GetVertexData();
    const Geometry::Index* indexData = geometry->GetIndexData();
    const U32 triangleCount = geometry->GetIndicesUsed() / 3;
    const size_t vertexSize = sizeof( Geometry::Vertex );
    
    for( U32 i = 0; i < triangleCount; ++i )
    {
        U32 index = i * 3;
        
        // Line 1
        WriteGeometry( destination++, vertexData, vertexSize, indexData[index] );
        WriteGeometry( destination++, vertexData, vertexSize, indexData[index + 1] );
    /*
        memcpy( destination++, &vertexData[ indexData[index] ], vertexSize );
        memcpy( destination++, &vertexData[ indexData[index + 1] ], vertexSize );
    */
                
        // Line 2
        WriteGeometry( destination++, vertexData, vertexSize, indexData[index + 1] );
        WriteGeometry( destination++, vertexData, vertexSize, indexData[index + 2] );
    /*
        memcpy( destination++, &vertexData[ indexData[index + 1] ], vertexSize );
        memcpy( destination++, &vertexData[ indexData[index + 2] ], vertexSize );
    */

        // Line 3
        WriteGeometry( destination++, vertexData, vertexSize, indexData[index + 2] );
        WriteGeometry( destination++, vertexData, vertexSize, indexData[index] );
    /*
        memcpy( destination++, &vertexData[ indexData[index + 2] ], vertexSize );
        memcpy( destination++, &vertexData[ indexData[index] ], vertexSize );
    */
    }
}

int
Renderer::GetVersionCode( bool addingMask ) const
{
    if (fWireframeEnabled)
    {
        return Program::kWireframe;
    }
    
    else
    {
        int count = fCurrentProgramMaskCount;
        
        if (addingMask)
        {
            ++count;
        }
        
        return count <= 3 ? static_cast<Program::Version>( count ) : -1;
    }
}

void
Renderer::MergeVertexData( Geometry::Vertex** destination, const Geometry::Vertex* mainSrc, const Geometry::Vertex* extensionSrc, int index, int extraCount )
{
    WriteGeometry( *destination, mainSrc, sizeof(Geometry::Vertex), index );

    /* memcpy( *destination, &mainSrc[index], sizeof(Geometry::Vertex) ); */
    
    ++*destination;

    WriteGeometry( *destination, extensionSrc, sizeof(Geometry::Vertex) * extraCount, index, 1, GeometryWriter::kExtra );

    /* memcpy( *destination, &extensionSrc[index * extraCount], sizeof(Geometry::Vertex) * extraCount ); */
    
    *destination += extraCount;
}

void
Renderer::MergeVertexDataRange( Geometry::Vertex** destination, Geometry* geometry, int count, int extraCount, int offset )
{
    const Geometry::Vertex * mainSrc = geometry->GetVertexData(), * extensionSrc = geometry->GetExtendedVertexData();
    
    for (int index = 0; index < count; ++index)
    {
        MergeVertexData( destination, mainSrc, extensionSrc, index + offset, extraCount );
    }
}

void
Renderer::CopyExtendedVertexData( Geometry* geometry, Geometry::Vertex* destination, bool interior )
{
    const U32 verticesUsed = geometry->GetVerticesUsed();

    if( fWireframeEnabled )
    {
        // Given the primitive type, convert the vertex data to lines
        switch( geometry->GetPrimitiveType() )
        {
            case Geometry::kTriangleStrip:
                CopyExtendedTriangleStripsAsLines( geometry, destination );
                break;
            case Geometry::kTriangleFan:
                CopyExtendedTriangleFanAsLines( geometry, destination );
                break;
            case Geometry::kTriangles:
                CopyExtendedTrianglesAsLines( geometry, destination );
                break;
            case Geometry::kIndexedTriangles:
                CopyExtendedIndexedTrianglesAsLines( geometry, destination );
                break;
            case Geometry::kLineLoop:
                MergeVertexDataRange( &destination, geometry, verticesUsed, fVertexExtra );
                break;
            case Geometry::kLines:
                MergeVertexDataRange( &destination, geometry, verticesUsed, fVertexExtra );
                break;
        }
    }
    else
    {
        if( geometry->GetPrimitiveType() == Geometry::kTriangleStrip )
        {
            // Triangle strips are batched by adding degenerate triangles
            // at the beginning and end of each strip.
            MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), 0, fVertexExtra );

            MergeVertexDataRange( &destination, geometry, verticesUsed, fVertexExtra );

            MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), verticesUsed - 1, fVertexExtra );

            fDegenerateVertexCount = 1;
        }
        else
        {
            // For data which does not exist on the GPU and is not batched,
            // we still double buffer it to be threadsafe.
            MergeVertexDataRange( &destination, geometry, verticesUsed, fVertexExtra );
        }
    }
}

void
Renderer::CopyExtendedTriangleStripsAsLines( Geometry* geometry, Geometry::Vertex* destination )
{
    const U32 count = geometry->GetVerticesUsed() - 2;
    const Geometry::Vertex* data = geometry->GetVertexData();
    
    for( U32 i = 0; i < count; ++i )
    {
        // Line 1
        MergeVertexDataRange( &destination, geometry, 2, fVertexExtra );
        
        // Line 2
        MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), i, fVertexExtra );
        MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), i + 2, fVertexExtra );
    }
    
    MergeVertexDataRange( &destination, geometry, 2, fVertexExtra, count );
}

void
Renderer::CopyExtendedTriangleFanAsLines( Geometry* geometry, Geometry::Vertex* destination )
{
    const U32 count = geometry->GetVerticesUsed() - 1;
    const Geometry::Vertex* data = geometry->GetVertexData();
    
    for( U32 i = 1; i < count; ++i )
    {
        // Line 1
        MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), 0, fVertexExtra );
        MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), i, fVertexExtra );
        
        // Line 2
        MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), i, fVertexExtra );
        MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), i + 1, fVertexExtra );
    }
    
    MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), 0, fVertexExtra );
    MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), count, fVertexExtra );
}

void
Renderer::CopyExtendedTrianglesAsLines( Geometry* geometry, Geometry::Vertex* destination )
{
    const U32 triangleCount = geometry->GetVerticesUsed() / 3;
    const Geometry::Vertex* data = geometry->GetVertexData();
    
    for( U32 i = 0; i < triangleCount; ++i )
    {
        U32 index = i * 3;

        // Line 1
        MergeVertexDataRange( &destination, geometry, 2, fVertexExtra, index );
        
        // Line 2
        MergeVertexDataRange( &destination, geometry, 2, fVertexExtra, index + 1 );

        // Line 3
        MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), index + 2, fVertexExtra );
        MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), index, fVertexExtra );
    }
}

void
Renderer::CopyExtendedIndexedTrianglesAsLines( Geometry* geometry, Geometry::Vertex* destination )
{
    const Geometry::Vertex* vertexData = geometry->GetVertexData();
    const Geometry::Index* indexData = geometry->GetIndexData();
    const U32 triangleCount = geometry->GetIndicesUsed() / 3;
    
    for( U32 i = 0; i < triangleCount; ++i )
    {
        U32 index = i * 3;
        
        // Line 1
        MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), indexData[index], fVertexExtra );
        MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), indexData[index + 1], fVertexExtra );
                
        // Line 2
        MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), indexData[index + 1], fVertexExtra );
        MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), indexData[index + 2], fVertexExtra );

        // Line 3
        MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), indexData[index + 2], fVertexExtra );
        MergeVertexData( &destination, geometry->GetVertexData(), geometry->GetExtendedVertexData(), indexData[index], fVertexExtra );
    }
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
