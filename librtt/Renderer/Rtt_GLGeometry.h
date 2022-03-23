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
#include "Renderer/Rtt_Geometry_Renderer.h" // <- STEVE CHANGE

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

        // STEVE CHANGE
        static bool SupportsInstancing();
        static bool SupportsDivisors();
        static const char * InstanceIDSuffix();
    
        static void DrawArraysInstanced( GLenum mode, GLint first, GLsizei count,
                                           GLsizei primcount );
        static void DrawElementsInstanced( GLenum mode, GLsizei count, GLenum type,
                                GLvoid *indices, GLsizei primcount );
        static void VertexAttribDivisor( GLuint index, GLuint divisor);
    
        bool StoredOnGPU() const { return !!fVBO; }
        GLbyte* GetBaseOffset() const { return (GLbyte*)fPositionStart + fVertexOffset * sizeof(Geometry::Vertex); }
    
        void SpliceVertexRateData( const Geometry::Vertex* vertexData, Geometry::Vertex* extendedVertexData, const FormatExtensionList * list, size_t & size );
        // /STEVE CHANGE
    
        virtual void Create( CPUResource* resource );
        virtual void Update( CPUResource* resource );
        virtual void Destroy();
    // STEVE CHANGE
        void BindStockAttributes( size_t size );
        void SetVertexOffset( U32 offset, size_t sizeExtra, bool formatDirty );
    // /STEVE CHANGE
        /*virtual */void Bind(); // <- STEVE CHANGE
    // STEVE CHANGE
        void ResolveVertexFormat( const FormatExtensionList * list, U32 vertexSize, bool mainDirty, const Geometry::Vertex* instancingData, U32 instanceCount );
        U32 GetVertexOffset() const { return fVertexOffset; }
    // /STEVE CHANGE

    private:
        GLvoid* fPositionStart;
        GLvoid* fTexCoordStart;
        GLvoid* fColorScaleStart;
        GLvoid* fUserDataStart;
        GLuint fVAO;
        GLuint fVBO;
        GLuint fIBO;
    // STEVE CHANGE
        GLuint fInstancesVBO;
        S32 fInstancesAllocated;
    // /STEVE CHANGE
        U32 fVertexCount;
        U32 fVertexOffset;
        U32 fIndexCount;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_GLGeometry_H__
