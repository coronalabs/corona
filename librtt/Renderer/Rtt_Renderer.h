//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Renderer_H__
#define _Rtt_Renderer_H__

#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_RenderData.h"
#include "Renderer/Rtt_CPUResource.h"
#include "Renderer/Rtt_GPUResource.h"
#include "Core/Rtt_Assert.h"
#include "Core/Rtt_Config.h"
#include "Core/Rtt_Types.h" // TODO: Fix so this is not required to get Math to compile on iOS
#include "Core/Rtt_Math.h" // TODO: Fix so this is not required to get Array to compile on iOS
#include "Core/Rtt_Array.h"
#include "Core/Rtt_Real.h"
#include "Core/Rtt_Time.h"

// ----------------------------------------------------------------------------

struct Rtt_Allocator;
struct CoronaCommand;
struct CoronaStateBlock;

namespace Rtt
{

class CommandBuffer;
class FrameBufferObject;
class GeometryPool;
class Texture;
class Uniform;
class RenderingStream;
class BufferBitmap;
class ShaderData;
struct CustomGraphicsInfo;
struct TimeTransform;

// ----------------------------------------------------------------------------

class Renderer
{
    public:
        typedef Renderer Self;

    public:
        Renderer( Rtt_Allocator* allocator );

        virtual ~Renderer();

        // Called once at the start of the application. This function may only
        // be called when a valid rendering context is active.
        virtual void Initialize();

		// Perform any per-frame preparation. Total time is the time in seconds
		// since the start of the application. Delta time is the amount of time
		// in seconds it took to complete the previous frame.
		virtual void BeginFrame( Real totalTime, Real deltaTime, const TimeTransform *defTimeTransform, Real contentScaleX, Real contentScaleY, bool isCapture = false );

        // Perform any per-frame finalization.
        virtual void EndFrame();

		void BeginDrawing();

		virtual void CaptureFrameBuffer( RenderingStream & stream, BufferBitmap & bitmap, S32 x_in_pixels, S32 y_in_pixels, S32 w_in_pixels, S32 h_in_pixels );
		virtual void EndCapture() {}

		// Get the current view and projection matrices. These 4x4 matrices are
		// returned via the given pointers, which are assumed to be non-null.
		void GetFrustum( Real* viewMatrix, Real* projectionMatrix ) const;

        // Set the viewing frustum to use while rendering. This function may be
        // called multiple times per frame. Each call to Insert() will use the
        // most recently given frustum.
        void SetFrustum( const Real* viewMatrix, const Real* projectionMatrix );

        // Get the current viewport (the subregion into which normalized device
        // coordinates are remapped).
        void GetViewport( S32& x, S32& y, S32& width, S32& height ) const;

        // Normalized device coordinates will be remapped to the given window
        // coordinates. This function may be called multiple times per frame.
        // Each call to Insert() will use the most recently given viewport.
        void SetViewport( S32 x, S32 y, S32 width, S32 height );

        // Get the current scissor window.
        void GetScissor( S32& x, S32& y, S32& width, S32& height ) const;

        // Set the subregion of the output buffer outside of which all pixels
        // are rejected. This function may be called multiple times per frame.
        // Each call to Insert() will use the most recently given region.
        void SetScissor( S32 x, S32 y, S32 width, S32 height );

        // Get whether or not scissoring is enabled. Scissoring is disabled by
        // default.
        bool GetScissorEnabled() const;

        // Enable or disable scissor testing. The region given by SetScissor()
        // will only be applied if scissor testing is explicitly enabled.
        void SetScissorEnabled( bool enabled );

        // Get/Set the corresponding Renderer state for multisampling.
        // This does NOT take care of any surface-related settings
        // for multisampling b/c that's OS-specific.
        bool GetMultisampleEnabled() const;
        void SetMultisampleEnabled( bool enabled );

        // Get the active FrameBufferObject or NULL if one does not exist.
        FrameBufferObject* GetFrameBufferObject() const;

        // Set the FrameBufferObject. All subsequently inserted RenderData will
        // be drawn to this FBO until SetFrameBufferObject() is called again.
        void SetFrameBufferObject( FrameBufferObject* fbo );

        struct ExtraClearOptions {
            bool clearDepth;
            bool clearStencil;
            float depthClearValue;
            U32 stencilClearValue;
        };
    
        // Assign the given color to all pixels in the back buffer or, if there
        // is a texture bound for offscreen rendering, to pixels in the texture.
        void Clear( Real r, Real g, Real b, Real a, const ExtraClearOptions * extraOptions = NULL );

        // Push a new mask onto the stack. The given matrix describes a 2D
        // transformation and is expected to be of type Uniform::kMat3.
        void PushMask( Texture* maskTexture, Uniform* maskMatrix );

        // Pop the most recently added mask from the stack. This mask will not
        // be applied to any RenderData inserted after this call.
        void PopMask();
        
        // Push a new mask count. Typically called before a render-to-texture
        // operation, so that the mask count prior to the operation is saved.
        void PushMaskCount();

        // Restore the previous mask count. Typically called after a render-to-texture
        // operation, so that the mask count prior to the operation is restored.
        void PopMaskCount();

		// Generate the minimum set of commands needed to ensure that the given
		// RenderData is properly drawn on the next call to Render().
		void Insert( const RenderData* data, const ShaderData * shaderData = NULL );

        // Render all data added since the last call to swap(). It is both safe
        // and expected that Render() is called while another thread is adding
        // new RenderData and preparing it for the subsequent call to Render().
        // This function requires that a valid rendering context is active.
        void Render();

        // Synchronize any data used by the preparation and rendering threads,
        // including the creation, update, and destruction of GPU resources.
        // This function requires that a valid rendering context is active.
        void Swap();
        
        // This function iterates through the CPU resources and removes their GPU resources
        // causing them to be recreated lazily - this operation should only be called in events
        //that require it, such as switching opengl contexts
        //void UnloadGPUResources( );

        // Any GPU-side resources needed for the given data will be created the
        // next time a valid rendering context is available.
        void QueueCreate( CPUResource* resource );

        // Any GPU-side resources previously created for the given data will
        // be updated the next time a valid rendering context is available.
        void QueueUpdate( CPUResource* resource );

        // The GPU resources denoted by the given pointer will be destroyed
        // the next time a valid rendering context is available.
        void QueueDestroy( GPUResource* resource );

        // Return true if wireframe rendering is enabled. Disabled by default.
        bool GetWireframeEnabled() const;

        // Render triangles as outlines with no interior. Useful for debugging.
        void SetWireframeEnabled( bool enabled );

		static U32 GetMaxTextureSize();
		static const char *GetGlString( const char *s );
		static bool GetGpuSupportsHighPrecisionFragmentShaders();
        static U32 GetMaxUniformVectorsCount();
		static U32 GetMaxVertexTextureUnits();

        bool HasFramebufferBlit(  bool * canScale ) const;
        void GetVertexAttributes( VertexAttributeSupport & support ) const;

        struct Statistics
        {
            Statistics();
            void Log() const;

            Real fResourceCreateTime;    // Time spent creating GPU resources in ms
            Real fResourceUpdateTime;    // Time spent updating GPU resources in ms
            Real fResourceDestroyTime;    // Time spent deleting GPU resources in ms
            Real fPreparationTime;        // Time spent from BeginFrame to Swap in ms
            Real fRenderTimeCPU;        // CPU dispatch time in ms
            Real fRenderTimeGPU;        // GPU execution time in ms
            U32 fDrawCallCount;            // Number of draw commands issued
            U32 fTriangleCount;            // Number of triangles drawn
            U32 fLineCount;             // Number of lines drawn
            U32 fGeometryBindCount;        // Number of Geometry bindings
            U32 fProgramBindCount;        // Number of Program bindings
            U32 fTextureBindCount;        // Number of Texture bindings
            U32 fUniformBindCount;        // Number of Uniform bindings
        };

        // Return true if statistics gathering is enabled. Disabled by default.
        bool GetStatisticsEnabled() const;

        // For optimal performance, statistics gathering should only be enabled
        // when needed.
        void SetStatisticsEnabled( bool enabled );

        // Get the previous frame's statistics. For accurate results, do not
        // call this function during preparation or rendering.
        const Statistics& GetFrameStatistics() const;

        // Get the maximum number of RenderData that may be inserted each frame.
        U32 GetMaximumRenderDataCount() const;
        
        // Set the maximum number of RenderData that may be inserted each frame.
        // This is primarily useful as a debugging tool. By increasing the count
        // over time, users can visualize render order, batching, etc.
        void SetMaximumRenderDataCount( U32 count );
	
        void SetCPUResourceObserver(MCPUResourceObserver *resourceObserver);
        void ReleaseGPUResources();

        // When there is a GPU-dependency on time, e.g. the shader code,
        // we need to ensure re-blitting. These functions help tally each
        // time such a situation occurs.
        void TallyTimeDependency( bool usesTime );
        bool IsFrameTimeDependent() const { return fTimeDependencyCount > 0; }

        // During render-to-texture, there could be false positives added to the
        // dependency count, so these functions allow you to save/restore the count.
        // These are false positives b/c they operate offscreen and don't affect
        // whether or not objects *on*screen need to be re-blitted
        void SetTimeDependencyCount( U32 newValue ) { fTimeDependencyCount = newValue; }
        U32 GetTimeDependencyCount() const { return fTimeDependencyCount; }
    
        U16 AddStateBlock( const CoronaStateBlock & block );
        bool GetStateBlockInfo( U16 id, U8 *& start, U32 & size, bool mightDirty );

        U16 AddCustomCommand( const CoronaCommand & command );

        bool IssueCustomCommand( U16 id, const void * data, U32 size );
    
	public:
		void InsertCaptureRect( FrameBufferObject * fbo, Texture * texture, const Rect & clipped, const Rect & unclipped );
		void IssueCaptures( Texture * fill0 );

    public:
        void SetGeometryWriters( const GeometryWriter* list, U32 n = 1 );
        bool AddGeometryWriter( const GeometryWriter& writer, bool isUpdate );
        bool CanAddGeometryWriters() const { return fCanAddGeometryWriters; }

        struct GeometryWriterRAII {
            GeometryWriterRAII( Renderer& renderer );
            ~GeometryWriterRAII();

            Renderer& fRenderer;
        };
        
    protected:
	    void WriteGeometry ( void * dstGeomComp, const void* srcGeom, U32 stride, U32 index, U32 count = 1, GeometryWriter::MaskBits validBits = GeometryWriter::kMain );

    protected:
        // Destroys all queued GPU resources passed into the DestroyQueue() method.
        void DestroyQueuedGPUResources();

        // Derived classes must use this function to provide platform specific
        // and rendering API specific GPUResources.
        virtual GPUResource* Create( const CPUResource* resource ) = 0;

        // Bind resources, taking care that they have properly created backends
        void BindTexture( Texture* maskTexture, U32 unit );
        void BindUniform( Uniform* maskUniform, U32 unit );

        // Check for any unsubmitted RenderData that must be drawn. Typically,
        // this is called in response to a change in render state.
        void CheckAndInsertDrawCommand();

    private:
        U32 EnumerateDirtyBlocks( ArrayS32& dirtyIndices );
        void UpdateDirtyBlocks( const ArrayS32& dirtyIndices, U32 largestDirtySize );
        void RestoreDefaultBlocks();
        void InsertInstancing( const Geometry::ExtensionBlock* block, const FormatExtensionList* programList, const FormatExtensionList* geometryList );
        void FlushBatch();
    
    protected:
        void UpdateBatch( bool batch, bool enoughSpace, bool storedOnGPU, U32 verticesRequired );
        void CopyVertexData( Geometry* geometry, Geometry::Vertex* destination, bool interior );
        void CopyTriangleStripsAsLines( Geometry* geometry, Geometry::Vertex* destination );
        void CopyTriangleFanAsLines( Geometry* geometry, Geometry::Vertex* destination );
        void CopyIndexedTrianglesAsLines( Geometry* geometry, Geometry::Vertex* destination );
        void CopyTrianglesAsLines( Geometry* geometry, Geometry::Vertex* destination );

        void MergeVertexData( Geometry::Vertex** destination, const Geometry::Vertex* mainSrc, const Geometry::Vertex* extensionSrc, int index, int extraCount );
        void MergeVertexDataRange( Geometry::Vertex** destination, Geometry* geometry, int count, int extraCount, int offset = 0 );

        void CopyExtendedVertexData( Geometry* geometry, Geometry::Vertex* destination, bool interior );
        void CopyExtendedTriangleStripsAsLines( Geometry* geometry, Geometry::Vertex* destination );
        void CopyExtendedTriangleFanAsLines( Geometry* geometry, Geometry::Vertex* destination );
        void CopyExtendedIndexedTrianglesAsLines( Geometry* geometry, Geometry::Vertex* destination );
        void CopyExtendedTrianglesAsLines( Geometry* geometry, Geometry::Vertex* destination );
    
    protected:
        // Returns count at top of the mask count stack
        const U32& MaskCount() const { return fMaskCount[fMaskCountIndex]; }
        U32& MaskCount() { return fMaskCount[fMaskCountIndex]; }

    public:
        int GetVersionCode( bool addingMask ) const;

	protected:
		Rtt_Allocator* fAllocator;
		
		MCPUResourceObserver *fCPUResourceObserver;
		
		LightPtrArray<CPUResource> fCreateQueue;
		LightPtrArray<CPUResource> fUpdateQueue;
		Array<GPUResource*> fDestroyQueue;

        GeometryPool* fGeometryPool;
        GeometryPool* fInstancingGeometryPool;
	
        CommandBuffer* fFrontCommandBuffer;
        CommandBuffer* fBackCommandBuffer;
        
        Uniform* fViewProjectionMatrix;
        Uniform* fTotalTime;
        Uniform* fDeltaTime;
        Uniform* fTexelSize;
        Uniform* fContentScale;

        int fMaskCountIndex;
        Array< U32 > fMaskCount; // "Stack" of mask counts
        U32 fCurrentProgramMaskCount;

        bool fWireframeEnabled;
        bool fStatisticsEnabled;
        Statistics fStatistics;
        Rtt_AbsoluteTime fStartTime;

        Real fViewMatrix[16];
        Real fProjMatrix[16];
        S32 fViewport[4];
        S32 fScissor[4];
        bool fScissorEnabled;
        bool fMultisampleEnabled;
        FrameBufferObject* fFrameBufferObject;

        RenderData fPrevious;
        U32 fVertexOffset;
        U32 fVertexCount;
        U32 fVertexExtra;
        U32 fIndexOffset;
        U32 fIndexCount;
        U32 fRenderDataCount;
        U32 fInsertionCount;
        U32 fInsertionLimit;
        U32 fDegenerateVertexCount;
        U32 fCachedVertexOffset;
        U32 fCachedVertexCount;
        U32 fCachedVertexExtra;
		U32 fOffsetCorrection;
        Geometry::PrimitiveType fPreviousPrimitiveType;
        Geometry::Vertex* fCurrentVertex;
        Geometry* fCurrentGeometry;
        Geometry::Vertex* fCurrentInstancingVertex;
        Geometry* fCurrentInstancingGeometry;

        Real fContentScaleX; // Temporary holder.
        Real fContentScaleY; // Temporary holder.

        U32 fTimeDependencyCount;
    
        struct RectPair {
			Rect fClipped;
			Rect fUnclipped;
			RectPair * fNext;
		};
	
		struct CaptureGroup {
			FrameBufferObject * fFBO;
			Texture * fTexture;
			RectPair * fFirst;
			RectPair * fLast;
		};
	
		Array< CaptureGroup > fCaptureGroups;
		Array< RectPair > fCaptureRects;

        Array< U8 > fDefaultState;
        Array< U8 > fCurrentState;
        Array< U8 > fWorkingState;
        
        CustomGraphicsInfo* fCustomInfo; // n.b. avoids some #includes
        U16 fSyncedCount;
        bool fMaybeDirty;

        Array< GeometryWriter > fGeometryWriters;
        const GeometryWriter* fCurrentGeometryWriterList; // to detect change in writer; assumed to be stable object, i.e. either NULL (default) or some static array
        bool fCanAddGeometryWriters;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Renderer_H__
