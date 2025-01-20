//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_GLGeometry_H__
#define _Rtt_GLGeometry_H__

#include "Renderer/Rtt_GL.h"
#include "Renderer/Rtt_GPUResource.h"
#include "Renderer/Rtt_Geometry_Renderer.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class GLGeometry : public GPUResource
{
    public:
        typedef GPUResource Super;
        typedef GLGeometry Self;

    public:
        GLGeometry();

        static bool SupportsInstancing();
        static bool SupportsDivisors();
        static const char * InstanceIDSuffix();
    
        static void DrawArraysInstanced( GLenum mode, GLint first, GLsizei count,
                                           GLsizei primcount );
        static void DrawElementsInstanced( GLenum mode, GLsizei count, GLenum type,
                                GLvoid *indices, GLsizei primcount );
        static void VertexAttribDivisor( GLuint index, GLuint divisor);
    
        bool StoredOnGPU() const { return !!fVBO; }
    
        void SpliceVertexRateData( const Geometry::Vertex* vertexData, Geometry::Vertex* extendedVertexData, const FormatExtensionList * list, size_t & size );
    
        virtual void Create( CPUResource* resource );
        virtual void Update( CPUResource* resource );
        virtual void Destroy();

        void BindStockAttributes( size_t size, U32 offset );
		void Bind();

        void ResolveVertexFormat( const FormatExtensionList * list, U32 vertexSize, U32 offset, const Geometry::Vertex* instancingData, U32 instanceCount );

    private:
        GLvoid* fPositionStart;
        GLvoid* fTexCoordStart;
        GLvoid* fColorScaleStart;
        GLvoid* fUserDataStart;
        GLuint fVAO;
        GLuint fVBO;
        GLuint fIBO;
        GLuint fInstancesVBO;
        S32 fInstancesAllocated;
        U32 fVertexCount;
        U32 fIndexCount;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_GLGeometry_H__
