//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Solar2D game engine.
// With contributions from Dianchu Technology
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Renderer/Rtt_GLGeometry.h"

#include "Renderer/Rtt_FormatExtensionList.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_GL.h"

#include "Corona/CoronaGraphics.h"

#if defined( Rtt_EGL )
    #include <EGL/egl.h>
#endif

#include "Rtt_Profiling.h"

#include <stdio.h>
#include <stddef.h>

// ----------------------------------------------------------------------------

namespace /*anonymous*/
{
    using namespace Rtt;

#if defined( Rtt_WIN_PHONE_ENV )
    bool isVertexArrayObjectSupported()
    {
        return false;
    }
#elif defined( Rtt_EMSCRIPTEN_ENV )
    #ifdef Rtt_EGL
        PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES = NULL;
        PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES = NULL;
        PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES = NULL;
    #endif

    bool isVertexArrayObjectSupported()
    {
        return false;
    }
#elif defined( Rtt_EGL )
    PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES = NULL;
    PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES = NULL;
    PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES = NULL;

    bool isVertexArrayObjectSupported()
    {
        static bool sIsInitialized = false;
        static bool sIsSupported = false;

        if ( sIsInitialized )
        {
            sIsInitialized = true;
            glBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC) eglGetProcAddress( "glBindVertexArrayOES" );
            glDeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC) eglGetProcAddress( "glDeleteVertexArraysOES" );
            glGenVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC) eglGetProcAddress( "glGenVertexArraysOES" );

            sIsSupported = ( NULL != glBindVertexArrayOES )
                && ( NULL != glDeleteVertexArraysOES )
                && ( NULL != glGenVertexArraysOES );
        }
        
        return sIsSupported;
    }
#else
    bool isVertexArrayObjectSupported()
    {
        return true;
    }
#endif

    static const Geometry::Vertex* GetGPUVertexData( Geometry* geometry )
    {
        const Geometry::Vertex* extendedData = geometry->GetExtendedVertexData();

        if (extendedData)
        {
            return extendedData;
        }
        
        else
        {
            return geometry->GetVertexData();
        }
    }

    void createVertexArrayObject(Geometry* geometry, GLuint& VAO, GLuint& VBO, GLuint& IBO)
    {
        Rtt_glGenVertexArrays( 1, &VAO );
        GL_CHECK_ERROR();

        Rtt_glBindVertexArray( VAO );
        glGenBuffers( 1, &VBO ); GL_CHECK_ERROR();
        glBindBuffer( GL_ARRAY_BUFFER, VBO ); GL_CHECK_ERROR();

        glEnableVertexAttribArray( Geometry::kVertexPositionAttribute );
        glEnableVertexAttribArray( Geometry::kVertexTexCoordAttribute );
        glEnableVertexAttribArray( Geometry::kVertexColorScaleAttribute );
        glEnableVertexAttribArray( Geometry::kVertexUserDataAttribute );
        GL_CHECK_ERROR();

        const Geometry::Vertex* vertexData = GetGPUVertexData( geometry );
		
        if ( !vertexData )
        {
            GL_LOG_ERROR( "Unable to initialize GPU geometry. Data is NULL" );
        }

        // It is valid to pass a NULL pointer, so allocation is done either way
        const FormatExtensionList* extensionList = geometry->GetExtensionList();
        const size_t size = FormatExtensionList::GetVertexSize( extensionList );
        const U32 vertexCount = geometry->GetVerticesAllocated();
        glBufferData( GL_ARRAY_BUFFER, vertexCount * size, vertexData, GL_STATIC_DRAW );
        GL_CHECK_ERROR();
    
        glVertexAttribPointer( Geometry::kVertexPositionAttribute, 3, GL_FLOAT, GL_FALSE, size, (void*)0 );
        glVertexAttribPointer( Geometry::kVertexTexCoordAttribute, 3, GL_FLOAT, GL_FALSE, size, (void*)12 );
        glVertexAttribPointer( Geometry::kVertexColorScaleAttribute, 4, GL_UNSIGNED_BYTE, GL_TRUE, size, (void*)24 );
        glVertexAttribPointer( Geometry::kVertexUserDataAttribute, 4, GL_FLOAT, GL_FALSE, size, (void*)28 );
     
        GL_CHECK_ERROR();
        
        const Geometry::Index* indexData = geometry->GetIndexData();
        if ( indexData )
        {
            const U32 indexCount = geometry->GetIndicesAllocated();
            glGenBuffers( 1, &IBO );
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, IBO );
            glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(Geometry::Index), indexData, GL_STATIC_DRAW );
        }

        Rtt_glBindVertexArray( 0 );
        GL_CHECK_ERROR();
    }
    
    void destroyVertexArrayObject(GLuint VAO, GLuint VBO, GLuint IBO)
    {
        if ( VAO != 0 )
        {
            Rtt_glDeleteVertexArrays( 1, &VAO );
        }
        
        if ( VBO != 0 )
        {
            glDeleteBuffers( 1, &VBO );
        }
        
        if ( IBO != 0)
        {
            glDeleteBuffers( 1, &IBO );
        }
        
        GL_CHECK_ERROR();
    }

    void createVBO(Geometry* geometry, GLuint& VBO, GLuint& IBO)
    {
        glGenBuffers( 1, &VBO ); GL_CHECK_ERROR();
        glBindBuffer( GL_ARRAY_BUFFER, VBO ); GL_CHECK_ERROR();

        glEnableVertexAttribArray( Geometry::kVertexPositionAttribute );
        glEnableVertexAttribArray( Geometry::kVertexTexCoordAttribute );
        glEnableVertexAttribArray( Geometry::kVertexColorScaleAttribute );
        glEnableVertexAttribArray( Geometry::kVertexUserDataAttribute );
        GL_CHECK_ERROR();

        const Geometry::Vertex* vertexData = GetGPUVertexData( geometry );
		
        if ( !vertexData )
        {
            GL_LOG_ERROR( "Unable to initialize GPU geometry. Data is NULL" );
        }

        // It is valid to pass a NULL pointer, so allocation is done either way
        const U32 vertexCount = geometry->GetVerticesAllocated();
        const FormatExtensionList* extensionList = geometry->GetExtensionList();
        const size_t size = FormatExtensionList::GetVertexSize( extensionList );
        glBufferData( GL_ARRAY_BUFFER, vertexCount * size, vertexData, GL_STATIC_DRAW );
        GL_CHECK_ERROR();
        
        const Geometry::Index* indexData = geometry->GetIndexData();
        if ( indexData )
        {
            const U32 indexCount = geometry->GetIndicesAllocated();
            glGenBuffers( 1, &IBO );
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, IBO );
            glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(Geometry::Index), indexData, GL_STATIC_DRAW );
        }

    }

    void destroyVBO(GLuint VBO, GLuint IBO)
    {
        if ( VBO != 0 )
        {
            glDeleteBuffers( 1, &VBO );
        }
        if ( IBO != 0) {
            glDeleteBuffers( 1, &IBO);
        }
    }

}

void createInstanceVBO( Geometry* geometry, GLuint& instancesVBO )
{
    glGenBuffers( 1, &instancesVBO ); GL_CHECK_ERROR();
    glBindBuffer( GL_ARRAY_BUFFER, instancesVBO ); GL_CHECK_ERROR();

    const FormatExtensionList* extensionList = geometry->GetExtensionList();
    U32 instanceCount = geometry->GetExtensionBlock()->fCount, vertexCount = 0;
    
    for (auto iter = FormatExtensionList::InstancedGroups( extensionList ); !iter.IsDone(); iter.Advance())
    {
        vertexCount += iter.GetGroup()->GetVertexCount( instanceCount, iter.GetAttribute() );
    }

    glBufferData( GL_ARRAY_BUFFER, vertexCount * sizeof(Geometry::Vertex), NULL, GL_DYNAMIC_DRAW );
    GL_CHECK_ERROR();
}

void destroyInstanceVBO( GLuint instancesVBO )
{
    if (instancesVBO != 0)
    {
        glDeleteBuffers( 1, &instancesVBO );
    }
}

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

GLGeometry::GLGeometry()
:    fPositionStart( NULL ),
    fTexCoordStart( NULL ),
    fColorScaleStart( NULL ),
    fUserDataStart( NULL ),
    fVAO( 0 ),
    fVBO( 0 ),
    fIBO( 0 ),
    fInstancesVBO( 0 ),
    fInstancesAllocated( -1 )
{
}

#if defined( Rtt_WIN_ENV )
    #define GL_TYPE_PREFIX GLAPIENTRY
#elif defined( Rtt_IPHONE_ENV )
	#define GL_TYPE_PREFIX GL_APIENTRY
#else
    #define GL_TYPE_PREFIX
#endif

typedef void (GL_TYPE_PREFIX *DrawArraysInstancedPtr)( GLenum mode, GLint first, GLsizei count,
                           GLsizei primcount );
typedef void (GL_TYPE_PREFIX *DrawElementsInstancedPtr)( GLenum mode, GLsizei count, GLenum type,
                const GLvoid *indices, GLsizei primcount );
typedef void (GL_TYPE_PREFIX *VertexAttribDivisorPtr)( GLuint index, GLuint divisor);

static DrawArraysInstancedPtr sDrawArraysInstanced;
static DrawElementsInstancedPtr sDrawElementsInstanced;
static VertexAttribDivisorPtr sVertexAttribDivisor;
static const char * sSuffix;

#if defined( Rtt_EGL )
	#define GL_GET_PROC(name, suffix) (name ## Ptr) eglGetProcAddress( "gl" #name #suffix )
#else
    #define GL_GET_PROC(name, suffix) gl ## name ## suffix
#endif

#define GL_RESET() DrawArrays = NULL; DrawElements = NULL; AttribDivisor = NULL; Suffix = NULL
#define GL_HAS_SUPPORT() ((NULL != DrawArrays) && (NULL != DrawElements))

bool
GLGeometry::SupportsInstancing()
{
    static bool sIsInitialized;
    
    if (!sIsInitialized)
    {
        sIsInitialized = true;
        
        DrawArraysInstancedPtr DrawArrays;
        DrawElementsInstancedPtr DrawElements;
        VertexAttribDivisorPtr AttribDivisor;
        const char * Suffix;

        GL_RESET();
        
    #if !defined( Rtt_OPENGLES ) // GL_ARB_instanced_arrays
        DrawArrays = GL_GET_PROC( DrawArraysInstanced, ARB );
        DrawElements = GL_GET_PROC( DrawElementsInstanced, ARB );
        AttribDivisor = GL_GET_PROC( VertexAttribDivisor, ARB );
        
        #if defined( GL_ARB_draw_instanced )
            Suffix = "*ARB"; // enable + ARB suffix
        #endif
    #endif
        
		const char * extensions = (const char *)glGetString( GL_EXTENSIONS );
		
    #if defined( GL_EXT_instanced_arrays )
        if (strstr( extensions, "GL_EXT_instanced_arrays" ) && !GL_HAS_SUPPORT())
        {
            GL_RESET();
        
            DrawArrays = GL_GET_PROC( DrawArraysInstanced, EXT );
            DrawElements = GL_GET_PROC( DrawElementsInstanced, EXT );
            AttribDivisor = GL_GET_PROC( VertexAttribDivisor, EXT );
        }
    #endif

    #if defined( GL_ANGLE_instanced_arrays )
        if (strstr( extensions, "GL_ANGLE_instanced_arrays" ) && !GL_HAS_SUPPORT())
        {
            GL_RESET();
        
            DrawArrays = GL_GET_PROC( DrawArraysInstanced, ANGLE );
            DrawElements = GL_GET_PROC( DrawElementsInstanced, ANGLE );
            AttribDivisor = GL_GET_PROC( VertexAttribDivisor, ANGLE );
        }
    #endif
        
    #if defined( GL_EXT_draw_instanced ) // extension notes suggest not ES-only...
        if (strstr( extensions, "GL_EXT_draw_instanced" ) && !GL_HAS_SUPPORT())
        {
            GL_RESET();
            
            DrawArrays = GL_GET_PROC( DrawArraysInstanced, EXT );
            DrawElements = GL_GET_PROC( DrawElementsInstanced, EXT );

        #if defined( Rtt_OPENGLES )
            Suffix = "*EXT"; // enable + EXT suffix, cf. note in GLProgram.cpp
        #else
            Suffix = ""; // has ID
        #endif
        }
    #endif

        if (GL_HAS_SUPPORT())
        {
            sDrawArraysInstanced = DrawArrays;
            sDrawElementsInstanced = DrawElements;
            sVertexAttribDivisor = AttribDivisor;
            sSuffix = Suffix;
        }
    }

    return NULL != sDrawArraysInstanced;
}

bool
GLGeometry::SupportsDivisors()
{
    SupportsInstancing(); // for side effects
    
    return NULL != sVertexAttribDivisor;
}

const char*
GLGeometry::InstanceIDSuffix()
{
    SupportsInstancing(); // for side effects
    
    return sSuffix;
}

#undef GL_GET_PROC
#undef GL_RESET
#undef GL_HAS_SUPPORT
#undef GL_TYPE_PREFIX

void
GLGeometry::DrawArraysInstanced( GLenum mode, GLint first, GLsizei count,
                                   GLsizei primcount )
{
    Rtt_ASSERT( sDrawArraysInstanced );
    
    sDrawArraysInstanced( mode, first, count, primcount );
}

void
GLGeometry::DrawElementsInstanced( GLenum mode, GLsizei count, GLenum type,
                        GLvoid *indices, GLsizei primcount )
{
    Rtt_ASSERT( sDrawElementsInstanced );
    
    sDrawElementsInstanced( mode, count, type, indices, primcount );
}

void
GLGeometry::VertexAttribDivisor( GLuint index, GLuint divisor)
{
    Rtt_ASSERT( sVertexAttribDivisor );
    
    sVertexAttribDivisor( index, divisor );
}

void
GLGeometry::SpliceVertexRateData( const Geometry::Vertex* vertexData, Geometry::Vertex* extendedVertexData, const FormatExtensionList * list, size_t & size )
{
    U32 total = 1 + list->ExtraVertexCount();
    
    for (U32 i = 0, j = 0, iMax = fVertexCount * total; i < iMax; i += total, ++j)
    {
        extendedVertexData[i] = vertexData[j]; // other slots already occupied
    }
    
    size *= total;
}

void
GLGeometry::Create( CPUResource* resource )
{
    Rtt_ASSERT( CPUResource::kGeometry == resource->GetType() );
    Geometry* geometry = static_cast<Geometry*>( resource );

    bool shouldStoreOnGPU = geometry->GetStoredOnGPU();
    if ( shouldStoreOnGPU )
    {
		SUMMED_TIMING( glgcs, "Geometry GPU Resource (stored on GPU): Create" );

        if ( isVertexArrayObjectSupported() )
        {
            createVertexArrayObject( geometry, fVAO, fVBO, fIBO );
        }
        else
        {
            createVBO( geometry, fVBO, fIBO );

            Geometry::Vertex kVertex; // Uninitialized! Used for offset calculation only.

            // Initialize offsets
            fPositionStart = NULL;
            fTexCoordStart = (GLvoid *)((S8*)&kVertex.u - (S8*)&kVertex);
            fColorScaleStart = (GLvoid *)((S8*)&kVertex.rs - (S8*)&kVertex);
            fUserDataStart = (GLvoid *)((S8*)&kVertex.ux - (S8*)&kVertex);
        }

        fVertexCount = geometry->GetVerticesAllocated();
        fIndexCount = geometry->GetIndicesAllocated();

        const Geometry::ExtensionBlock* block = geometry->GetExtensionBlock();
        
        if (block && block->fCount > 0 && geometry->GetExtensionList()->HasInstanceRateData())
        {
            createInstanceVBO( geometry, fInstancesVBO );
            fInstancesAllocated = block->fCount;
        }
    }
    else
    {
        Update( resource );
    }
}

void
GLGeometry::Update( CPUResource* resource )
{
	SUMMED_TIMING( glgu, "Geometry GPU Resource: Update" );

    Rtt_ASSERT( CPUResource::kGeometry == resource->GetType() );
    Geometry* geometry = static_cast<Geometry*>( resource );

    const FormatExtensionList* extensionList = geometry->GetExtensionList();
    bool gainedExtension = -1 == fInstancesAllocated && NULL != extensionList;
    bool hasInstancedData = extensionList ? extensionList->HasInstanceRateData() : false;
    
    if (gainedExtension)
    {
        Rtt_ASSERT( extensionList->GetGroupCount() > 0 );
        
        fInstancesAllocated = 0;
    }
    
    if ( fVAO )
    {
        // The user may have resized the given Geometry instance
        // since the last call to update (see Geometry::Resize()).
        if ( fVertexCount < geometry->GetVerticesAllocated() ||
             fIndexCount < geometry->GetIndicesAllocated() ||
            (gainedExtension && extensionList->HasVertexRateData()) )
        {
            destroyVertexArrayObject( fVAO, fVBO, fIBO);
            createVertexArrayObject( geometry, fVAO, fVBO, fIBO );
            fVertexCount = geometry->GetVerticesAllocated();
            fIndexCount = geometry->GetIndicesAllocated();
        }

        if (hasInstancedData && fInstancesAllocated < geometry->GetExtensionBlock()->fCount)
        {
            destroyInstanceVBO( fInstancesVBO );
            createInstanceVBO( geometry, fInstancesVBO );
            fInstancesAllocated = geometry->GetExtensionBlock()->fCount;
        }
        
        // Copy the vertex data from main memory to GPU memory.
        const Geometry::Vertex* vertexData = geometry->GetVertexData();
        if ( vertexData )
        {
            size_t size = sizeof(Geometry::Vertex);
            
            if (extensionList && extensionList->HasVertexRateData())
            {
                Geometry::Vertex* extendedData = geometry->GetWritableExtendedVertexData();
                
                SpliceVertexRateData( vertexData, extendedData, extensionList, size );
                
                vertexData = extendedData;
            }
			
            glBindBuffer( GL_ARRAY_BUFFER, fVBO );
            glBufferSubData( GL_ARRAY_BUFFER, 0, fVertexCount * size, vertexData );

            const Geometry::ExtensionBlock* block = geometry->GetExtensionBlock();
                        
            Rtt_ASSERT( !hasInstancedData || block->fInstanceData );
            
            if (hasInstancedData && block->fCount > 0)
            {
                Rtt_ASSERT( fInstancesVBO );
                
                glBindBuffer( GL_ARRAY_BUFFER, fInstancesVBO );
                
                U32 offset = 0;
                
                for (auto iter = FormatExtensionList::InstancedGroups( extensionList ); !iter.IsDone(); iter.Advance())
                {
                    size_t dataSize = iter.GetGroup()->GetDataSize( block->fCount, iter.GetAttribute() );
                    U32 groupIndex = iter.GetGroupIndex();
                    
                    if (extensionList->HasVertexRateData())
                    {
                        --groupIndex;
                    }

                    glBufferSubData( GL_ARRAY_BUFFER, (GLintptr)offset, (GLsizeiptr)dataSize, block->fInstanceData[groupIndex]->ReadAccess() );
                    
                    offset += Geometry::Vertex::SizeInVertices( dataSize ) * sizeof(Geometry::Vertex);
                }
                // ^^^ only update if dirty, etc. (ditto for fVBO)
            }

            glBindBuffer( GL_ARRAY_BUFFER, 0 );
            
            const Geometry::Index* indexData = geometry->GetIndexData();
            if ( indexData )
            {
                Rtt_glBindVertexArray( 0 );
                glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, fIBO );
                glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, fIndexCount * sizeof(Geometry::Index), indexData );
                glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
            }
        }
        else
        {
            GL_LOG_ERROR( "Unable to update GPU geometry. Data is NULL" );
        }
    }
    else if ( fVBO )
    {
        // The user may have resized the given Geometry instance
        // since the last call to update (see Geometry::Resize()).
        if ( fVertexCount < geometry->GetVerticesAllocated() ||
             fIndexCount < geometry->GetIndicesAllocated() ||
             (gainedExtension && extensionList->HasVertexRateData()) )
        {
            destroyVBO( fVBO, fIBO );
            createVBO( geometry, fVBO, fIBO );
            fVertexCount = geometry->GetVerticesAllocated();
            fIndexCount = geometry->GetIndicesAllocated();
        }

        if (hasInstancedData && fInstancesAllocated < geometry->GetExtensionBlock()->fCount)
        {
            destroyInstanceVBO( fInstancesVBO );
            createInstanceVBO( geometry, fInstancesVBO );
            fInstancesAllocated = geometry->GetExtensionBlock()->fCount;
        }
        
        // Copy the vertex data from main memory to GPU memory.
        const Geometry::Vertex* vertexData = geometry->GetVertexData();
        if ( vertexData )
        {
            size_t size = sizeof(Geometry::Vertex);
            
            if (extensionList && extensionList->HasVertexRateData())
            {
                Geometry::Vertex* extendedData = geometry->GetWritableExtendedVertexData();
                
                SpliceVertexRateData( vertexData, extendedData, extensionList, size );
                
                vertexData = extendedData;
            }

            glBindBuffer( GL_ARRAY_BUFFER, fVBO );
            glBufferSubData( GL_ARRAY_BUFFER, 0, fVertexCount * size, vertexData );

            const Geometry::ExtensionBlock* block = geometry->GetExtensionBlock();
                        
            Rtt_ASSERT( !hasInstancedData || block->fInstanceData );
            
            if (hasInstancedData)
            {
                Rtt_ASSERT( fInstancesVBO );
                
                glBindBuffer( GL_ARRAY_BUFFER, fInstancesVBO );
                
                U32 offset = 0;
                
                for (auto iter = FormatExtensionList::InstancedGroups( extensionList ); !iter.IsDone(); iter.Advance())
                {
                    size_t dataSize = iter.GetGroup()->GetDataSize( block->fCount, iter.GetAttribute() );
                    U32 groupIndex = iter.GetAttributeIndex();
                    
                    if (extensionList->HasVertexRateData())
                    {
                        --groupIndex;
                    }
                    
                    glBufferSubData( GL_ARRAY_BUFFER, (GLintptr)offset, (GLsizeiptr)dataSize, block->fInstanceData[groupIndex]->ReadAccess() );
                    
                    offset += Geometry::Vertex::SizeInVertices( dataSize ) * sizeof(Geometry::Vertex);
                }
            }
            // ^^^ TODO: make these updates more fine-grained

            glBindBuffer( GL_ARRAY_BUFFER, 0 );

            //#390 mesh.path:update() fix 
			const Geometry::Index* indexData = geometry->GetIndexData();
			if ( indexData )
			{
				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, fIBO );
				glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, fIndexCount * sizeof(Geometry::Index), indexData );
				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
			}
        }
        else
        {
            GL_LOG_ERROR( "Unable to update GPU geometry. Data is NULL" );
        }
    }
    else
    {
        Geometry::Vertex* data = geometry->GetVertexData();
        fPositionStart = data;
        fTexCoordStart = &data[0].u;
        fColorScaleStart = &data[0].rs;
        fUserDataStart = &data[0].ux;
    }
    GL_CHECK_ERROR();
}

void
GLGeometry::Destroy()
{
    if ( fVAO )
    {
        destroyVertexArrayObject( fVAO, fVBO, fIBO );
        fVAO = 0;
        fVBO = 0;
        fIBO = 0;
    }
    else
    {
        if ( fVBO )
        {
            destroyVBO( fVBO, fIBO );
            fVBO = 0;
            fIBO = 0;
        }

        fPositionStart = NULL;
        fTexCoordStart = NULL;
        fColorScaleStart = NULL;
        fUserDataStart = NULL;
    }

    if (fInstancesVBO)
    {
        Rtt_ASSERT( -1 != fInstancesAllocated );
        
        glDeleteBuffers( 1, &fInstancesVBO );
        
        fInstancesVBO = 0;
    }
}

void
GLGeometry::BindStockAttributes( size_t size, U32 offset )
{
    const GLbyte* positionStart = (const GLbyte*)fPositionStart;
    const GLbyte* texCoordStart = (const GLbyte*)fTexCoordStart;
    const GLbyte* colorScaleStart = (const GLbyte*)fColorScaleStart;
    const GLbyte* userDataStart = (const GLbyte*)fUserDataStart;
    
    if (!StoredOnGPU())
    {
        positionStart += offset;
        texCoordStart += offset;
        colorScaleStart += offset;
        userDataStart += offset;
    }
    
    glVertexAttribPointer( Geometry::kVertexPositionAttribute, 3, GL_FLOAT, GL_FALSE, (GLsizei)size, positionStart ); GL_CHECK_ERROR();
    glVertexAttribPointer( Geometry::kVertexTexCoordAttribute, 3, GL_FLOAT, GL_FALSE, (GLsizei)size, texCoordStart ); GL_CHECK_ERROR();
    glVertexAttribPointer( Geometry::kVertexColorScaleAttribute, 4, GL_UNSIGNED_BYTE, GL_TRUE, (GLsizei)size, colorScaleStart ); GL_CHECK_ERROR();
    glVertexAttribPointer( Geometry::kVertexUserDataAttribute, 4, GL_FLOAT, GL_FALSE, (GLsizei)size, userDataStart ); GL_CHECK_ERROR();
}

void
GLGeometry::Bind()
{
    if ( fVAO )
    {
        Rtt_glBindVertexArray( fVAO );
    }
    else
    {
        Rtt_ASSERT( fPositionStart || fVBO); // offset is 0 when VBO is available
        Rtt_ASSERT( fTexCoordStart );
        Rtt_ASSERT( fColorScaleStart );
        Rtt_ASSERT( fUserDataStart );
        
        // A previous GLGeometry may have left a VAO (and its VBO bound). Unbinding a
        // VAO does not alter its VBO, however, so both are explicitly unbound here.
        if(isVertexArrayObjectSupported())
        {
            Rtt_glBindVertexArray( 0 );
        }

        glBindBuffer( GL_ARRAY_BUFFER, fVBO ); GL_CHECK_ERROR();
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, fIBO ); GL_CHECK_ERROR();
		
		// cf. BindStockAttributes() for where "true" binds happen
    }
}

static void
BindExtensionAttribute( const FormatExtensionList::Attribute& attribute, GLuint attributeIndex, size_t size, GLbyte* start, U32 offsetExtra )
{
    GLenum type = GL_FLOAT;
    
    if (kAttributeType_Byte == attribute.type)
    {
        type = GL_UNSIGNED_BYTE;
    }
    // TODO: other types!

    U32 offset = attribute.offset + offsetExtra;
    
    glVertexAttribPointer( attributeIndex, attribute.components, type, attribute.normalized, (GLsizei)size, start ? (GLvoid*)(start + offset) : (GLvoid*)offset ); GL_CHECK_ERROR();
}

void
GLGeometry::ResolveVertexFormat( const FormatExtensionList * list, U32 vertexSize, U32 offset, const Geometry::Vertex* instancingData, U32 instanceCount )
{
    bool storedOnGPU = StoredOnGPU();

	if (storedOnGPU && ( !fVAO || ( list && list->HasVertexRateData() ) ) )
    {
        glBindBuffer( GL_ARRAY_BUFFER, fVBO );
    }
    
	offset *= sizeof( Geometry::Vertex );
    if ( !fVAO ) // a VAO does not have this info, but already has it bound
    {
        BindStockAttributes( vertexSize, offset );
    }

    Rtt_ASSERT( list );
    
    for ( auto iter = FormatExtensionList::AllGroups( list ); !iter.IsDone(); iter.Advance() )
    {
        const FormatExtensionList::Group* group = iter.GetGroup();
        const FormatExtensionList::Attribute* first = iter.GetAttribute();
        GLbyte* start = NULL;
        U32 offsetExtra = 0;
        size_t stride;
        
        if (group->IsInstanceRate())
        {
            if (group->IsWindowed())
            {
                stride = first->GetSize();
            }
            
            else
            {
                stride = group->size;
            }
            
            U32 vertexCount = group->GetVertexCount( instanceCount, first );
            
            if (storedOnGPU)
            {
                glBindBuffer( GL_ARRAY_BUFFER, fInstancesVBO );
            }
            
            else
            {
                Rtt_ASSERT( instancingData );
                
                start = (GLbyte*)instancingData;

                instancingData += vertexCount;
            }
        }
        
        else
        {
            stride = FormatExtensionList::GetVertexSize( list );
            offsetExtra = sizeof(Geometry::Vertex);
            
            if (!storedOnGPU)
            {
                start = ((GLbyte*)fPositionStart) + offset;
            }
        }
        
        U32 firstIndex = iter.GetAttributeIndex();
        
        for (U32 i = 0; i < group->count; ++i)
        {
            GLuint attributeIndex = Geometry::FirstExtraAttribute() + firstIndex + i;

            BindExtensionAttribute( first[i], attributeIndex, stride, start, offsetExtra );
        }
    }
    
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
