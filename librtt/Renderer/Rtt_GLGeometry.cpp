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

#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_GL.h"

#if defined( Rtt_EGL )
	#include <EGL/egl.h>
#endif

#include "Rtt_Profiling.h"

#include <stdio.h>

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

		const Geometry::Vertex* vertexData = geometry->GetVertexData();
		if ( !vertexData )
		{
			GL_LOG_ERROR( "Unable to initialize GPU geometry. Data is NULL" );
		}

		// It is valid to pass a NULL pointer, so allocation is done either way
		const U32 vertexCount = geometry->GetVerticesAllocated();
		glBufferData( GL_ARRAY_BUFFER, vertexCount * sizeof(Geometry::Vertex), vertexData, GL_STATIC_DRAW );
		GL_CHECK_ERROR();

		const size_t size = sizeof(Geometry::Vertex);
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

		const Geometry::Vertex* vertexData = geometry->GetVertexData();
		if ( !vertexData )
		{
			GL_LOG_ERROR( "Unable to initialize GPU geometry. Data is NULL" );
		}

		// It is valid to pass a NULL pointer, so allocation is done either way
		const U32 vertexCount = geometry->GetVerticesAllocated();
		glBufferData( GL_ARRAY_BUFFER, vertexCount * sizeof(Geometry::Vertex), vertexData, GL_STATIC_DRAW );
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

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

GLGeometry::GLGeometry()
:	fPositionStart( NULL ),
	fTexCoordStart( NULL ),
	fColorScaleStart( NULL ),
	fUserDataStart( NULL ),
	fVAO( 0 ),
	fVBO( 0 ),
	fIBO( 0 )
{
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

	if ( fVAO )
	{
		// The user may have resized the given Geometry instance
		// since the last call to update (see Geometry::Resize()).
		if ( fVertexCount < geometry->GetVerticesAllocated() ||
			 fIndexCount < geometry->GetIndicesAllocated() )
		{
			destroyVertexArrayObject( fVAO, fVBO, fIBO );
			createVertexArrayObject( geometry, fVAO, fVBO, fIBO );
			fVertexCount = geometry->GetVerticesAllocated();
			fIndexCount = geometry->GetIndicesAllocated();
		}
		
		// Copy the vertex data from main memory to GPU memory.
		const Geometry::Vertex* vertexData = geometry->GetVertexData();
		if ( vertexData )
		{
			glBindBuffer( GL_ARRAY_BUFFER, fVBO );
			glBufferSubData( GL_ARRAY_BUFFER, 0, fVertexCount * sizeof(Geometry::Vertex), vertexData );
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
			 fIndexCount < geometry->GetIndicesAllocated() )
		{
			destroyVBO( fVBO, fIBO );
			createVBO( geometry, fVBO, fIBO );
			fVertexCount = geometry->GetVerticesAllocated();
			fIndexCount = geometry->GetIndicesAllocated();
		}
		
		// Copy the vertex data from main memory to GPU memory.
		const Geometry::Vertex* vertexData = geometry->GetVertexData();
		if ( vertexData )
		{
			glBindBuffer( GL_ARRAY_BUFFER, fVBO );
			glBufferSubData( GL_ARRAY_BUFFER, 0, fVertexCount * sizeof(Geometry::Vertex), vertexData );
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
		
		const size_t size = sizeof(Geometry::Vertex);
		glVertexAttribPointer( Geometry::kVertexPositionAttribute, 3, GL_FLOAT, GL_FALSE, size, fPositionStart ); GL_CHECK_ERROR();
		glVertexAttribPointer( Geometry::kVertexTexCoordAttribute, 3, GL_FLOAT, GL_FALSE, size, fTexCoordStart ); GL_CHECK_ERROR();
		glVertexAttribPointer( Geometry::kVertexColorScaleAttribute, 4, GL_UNSIGNED_BYTE, GL_TRUE, size, fColorScaleStart ); GL_CHECK_ERROR();
		glVertexAttribPointer( Geometry::kVertexUserDataAttribute, 4, GL_FLOAT, GL_FALSE, size, fUserDataStart ); GL_CHECK_ERROR();
		
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, fIBO ); GL_CHECK_ERROR();

		
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
