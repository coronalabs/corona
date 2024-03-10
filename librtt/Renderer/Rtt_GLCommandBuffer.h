//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_GLCommandBuffer_H__
#define _Rtt_GLCommandBuffer_H__

#include "Renderer/Rtt_CommandBuffer.h"
#include "Renderer/Rtt_Uniform.h"

#include "Core/Rtt_Array.h"

// ----------------------------------------------------------------------------

struct CoronaCommand;

namespace Rtt
{

class GLExtraUniforms;

// ----------------------------------------------------------------------------

//
class GLCommandBuffer : public CommandBuffer
{
    public:
        typedef CommandBuffer Super;
        typedef GLCommandBuffer Self;

		bool HasFramebufferBlit( bool * canScale ) const;
        void GetVertexAttributes( VertexAttributeSupport & support ) const;

    public:
        GLCommandBuffer( Rtt_Allocator* allocator );
        virtual ~GLCommandBuffer();

        virtual void Initialize();
        
        virtual void Denitialize();

        virtual void ClearUserUniforms();

        // Generate the appropriate buffered OpenGL commands to accomplish the
        // specified state changes.
        virtual void BindFrameBufferObject( FrameBufferObject* fbo, bool asDrawBuffer );
		virtual void CaptureRect( FrameBufferObject* fbo, Texture& texture, const Rect& rect, const Rect& rawRect );
		virtual void BindGeometry( Geometry* geometry );
        virtual void BindTexture( Texture* texture, U32 unit );
        virtual void BindUniform( Uniform* uniform, U32 unit );
        virtual void BindProgram( Program* program, Program::Version version );
        virtual void BindInstancing( U32 count, Geometry::Vertex* instanceData );
        virtual void BindVertexFormat( FormatExtensionList* list, U16 fullCount, U16 vertexSize, U32 offset );
        virtual void SetBlendEnabled( bool enabled );
        virtual void SetBlendFunction( const BlendMode& mode );
        virtual void SetBlendEquation( RenderTypes::BlendEquation mode );
        virtual void SetViewport( int x, int y, int width, int height );
        virtual void SetScissorEnabled( bool enabled );
        virtual void SetScissorRegion( int x, int y, int width, int height );
        virtual void SetMultisampleEnabled( bool enabled );
        virtual void ClearDepth( Real depth );
        virtual void ClearStencil( U32 stencil );
        virtual void Clear( Real r, Real g, Real b, Real a );
        virtual void Draw( U32 offset, U32 count, Geometry::PrimitiveType type );
        virtual void DrawIndexed( U32 offset, U32 count, Geometry::PrimitiveType type );
        virtual S32 GetCachedParam( CommandBuffer::QueryableParams param );

        virtual void AddCommand( const CoronaCommand * command );
        virtual void IssueCommand( U16 id, const void * data, U32 size );

        virtual const unsigned char * GetBaseAddress() const { return fBuffer; }

        virtual bool WriteNamedUniform( const char * uniformName, const void * data, unsigned int size );

        // Execute all buffered commands. A valid OpenGL context must be active.
        virtual Real Execute( bool measureGPU );
    
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
		
		struct UniformUpdate
		{
			Uniform* uniform;
			U32 timestamp;
		};
		
		void ApplyUniforms( GPUResource* resource );
		void ApplyUniform( GPUResource* resource, U32 index );
		void WriteUniform( Uniform* uniform );
    
        U8 * Reserve( U32 size );

		UniformUpdate fUniformUpdates[Uniform::kNumBuiltInVariables];
		Program::Version fCurrentPrepVersion;
		Program::Version fCurrentDrawVersion;
	
		Program* fProgram;
		S32 fDefaultFBO;
		U32* fTimerQueries;
		U32 fTimerQueryIndex;
		Real fElapsedTimeGPU;
		S32 fCachedQuery[kNumQueryableParams];
    
        LightPtrArray< const CoronaCommand > fCustomCommands;

        GLExtraUniforms* fExtraUniforms;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_GLCommandBuffer_H__
