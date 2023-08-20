//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_CommandBuffer_H__
#define _Rtt_CommandBuffer_H__

#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_Program.h"
#include "Renderer/Rtt_RenderTypes.h"
#include "Core/Rtt_Types.h"
#include "Core/Rtt_Real.h"

// ----------------------------------------------------------------------------

struct Rtt_Allocator;
struct CoronaCommand;

namespace Rtt
{

class FrameBufferObject;
class Program;
class Texture;
class Uniform;
class ShaderResource;
class FormatExtensionList;
struct TimeTransform;

// ----------------------------------------------------------------------------

// The CommandBuffer has two primary responsibilities. The first is to provide
// an abstract interface through which render state may be specified, without
// knowledge of the underlying rendering API. The second is to facilitate
// multithreaded rendering by storing all of the commands (and the data needed
// by those commands) during a given frame N and then actually executing those
// commands on frame N + 1.
class CommandBuffer
{
    public:
        typedef CommandBuffer Self;

		typedef enum QueryableParams
		{
			kMaxTextureSize		= 0,
			kNumQueryableParams,
		}
		QueryableParams;
	
	public:
		void ReadBytes( void * value, size_t size );
		void WriteBytes( const void * value, size_t size );
		
	public:
		static size_t GetMaxUniformVectorsCount();
		static size_t GetMaxVertexTextureUnits();
		static size_t GetMaxTextureSize();
		static const char *GetGlString( const char *s );
		static bool GetGpuSupportsHighPrecisionFragmentShaders();

        virtual bool HasFramebufferBlit( bool * canScale ) const = 0;
		virtual void GetVertexAttributes( VertexAttributeSupport & support ) const = 0;

    public:
        CommandBuffer( Rtt_Allocator* allocator );
        virtual ~CommandBuffer();

        // Called once at the start of the application. Derived classes may
        // assume a valid rendering context exists (unlike the constructor).
        virtual void Initialize() = 0;

        // Called once at the end of the application. Derived classes may
        // assume a valid rendering context exists (unlike the destructor).
        virtual void Denitialize() = 0;
        
        virtual void ClearUserUniforms() = 0;

        // Derived classes are responsible for taking state changes specified
        // here and transcribing them into equivalent, buffered commands used
        // by the underlying rendering API.
        virtual void BindFrameBufferObject( FrameBufferObject* fbo, bool asDrawBuffer = false ) = 0;
		virtual void CaptureRect( FrameBufferObject* fbo, Texture& texture, const Rect& rect, const Rect& rawRect ) = 0;
		virtual void BindGeometry( Geometry* geometry ) = 0;
        virtual void BindTexture( Texture* texture, U32 unit ) = 0;
        virtual void BindUniform( Uniform* uniform, U32 unit ) = 0;
        virtual void BindProgram( Program* program, Program::Version version ) = 0;
        virtual void BindInstancing( U32 count, Geometry::Vertex* instanceData ) = 0;
        virtual void BindVertexFormat( FormatExtensionList* extensionList, U16 fullCount, U16 vertexSize, U32 offset ) = 0;
        virtual void SetBlendEnabled( bool enabled ) = 0;
        virtual void SetBlendFunction( const BlendMode& mode ) = 0;
        virtual void SetBlendEquation( RenderTypes::BlendEquation equation ) = 0;
        virtual void SetViewport( int x, int y, int width, int height ) = 0;
        virtual void SetScissorEnabled( bool enabled ) = 0;
        virtual void SetScissorRegion( int x, int y, int width, int height ) = 0;
        virtual void SetMultisampleEnabled( bool enabled ) = 0;
        virtual void ClearDepth( Real depth ) = 0;
        virtual void ClearStencil( U32 stencil ) = 0;
        virtual void Clear( Real r, Real g, Real b, Real a ) = 0;
        virtual void Draw( U32 offset, U32 count, Geometry::PrimitiveType type ) = 0;
        virtual void DrawIndexed( U32 offset, U32 count, Geometry::PrimitiveType type ) = 0;
        virtual S32 GetCachedParam( CommandBuffer::QueryableParams param ) = 0;

        virtual void AddCommand( const CoronaCommand * command ) = 0;
        virtual void IssueCommand( U16 id, const void * data, U32 size ) = 0;

        virtual const unsigned char * GetBaseAddress() const = 0;
    
        virtual bool WriteNamedUniform( const char * uniformName, const void * data, unsigned int size ) = 0;
        
        virtual void WillRender() {}
    
        // Execute the generated command buffer. This function should only be
        // called from a thread with an active rendering context. If requested
        // (and the platform supports it), this function should return the time
        // in ms taken to execute the commands on the GPU. To minimize latency,
        // it is valid if the time returned is actually for a previous frame.
        virtual Real Execute( bool measureGPU ) = 0;

    public:
        void PrepareTimeTransforms( const TimeTransform* transform );

    protected:
        void AcquireTimeTransform( ShaderResource* resource );

    private:
        virtual void InitializeFBO() = 0;
        virtual void InitializeCachedParams() = 0;
        virtual void CacheQueryParam( CommandBuffer::QueryableParams param ) = 0;

    protected:
        Rtt_Allocator* fAllocator;
        U8* fBuffer;
        U8* fOffset;
        U32 fNumCommands;
        U32 fBytesAllocated;
        U32 fBytesUsed;
        TimeTransform* fDefaultTimeTransform;
		TimeTransform* fTimeTransform;
        TimeTransform* fLastTimeTransform;
        bool fUsesTime;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_CommandBuffer_H__
