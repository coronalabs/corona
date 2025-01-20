//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_GLProgram_H__
#define _Rtt_GLProgram_H__

#include "Renderer/Rtt_GL.h"
#include "Renderer/Rtt_GPUResource.h"
#include "Renderer/Rtt_Program.h"
#include "Renderer/Rtt_Uniform.h"
#include "Core/Rtt_Assert.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

struct GLProgramUniformsCache;
class GLExtraUniforms;

// ----------------------------------------------------------------------------

class GLProgram : public GPUResource
{
	public:
		typedef GPUResource Super;
		typedef GLProgram Self;

	public:
		GLProgram();

		virtual void Create( CPUResource* resource );
		virtual void Update( CPUResource* resource );
		virtual void Destroy();
		virtual void Bind( Program::Version version );

		// TODO: cleanup these functions
		inline GLint GetUniformLocation( U32 unit, Program::Version version )
		{
			Rtt_ASSERT( version <= Program::kNumVersions );
			return fData[version].fUniformLocations[ unit ];
		}

		inline U32 GetUniformTimestamp( U32 unit, Program::Version version )
		{
			Rtt_ASSERT( version <= Program::kNumVersions );
			return fData[version].fTimestamps[ unit ];
		}

		inline void SetUniformTimestamp( U32 unit, Program::Version version, U32 timestamp)
		{
			Rtt_ASSERT( version <= Program::kNumVersions );
			fData[version].fTimestamps[ unit ] = timestamp;
		}

	private:
		// To make custom shader code work seamlessly with masking, multiple
		// versions of each Program are automatically compiled and linked, 
		// with each version supporting a different number of active masks.
		struct VersionData
		{
			GLuint fProgram;
			GLuint fVertexShader;
			GLuint fFragmentShader;			
			GLint fUniformLocations[Uniform::kNumBuiltInVariables];
			U32 fTimestamps[Uniform::kNumBuiltInVariables];
			
			// Metadata
			int fHeaderNumLines;
		};

		void Create( Program::Version version, VersionData& data );
		void Update( Program::Version version, VersionData& data );
		void UpdateShaderSource( Program* program, Program::Version version, VersionData& data );
		void Reset( VersionData& data );

        enum { kUniformNameBufferSize = 64 };
        
        void GetExtraUniformsInfo( Program::Version version, GLExtraUniforms& extraUniforms );
    
		VersionData fData[Program::kNumVersions];
		CPUResource* fResource;
    
        void (*fCleanupShellTransform)(void *); // compare CoronaGraphics.h
        GLProgramUniformsCache * fUniformsCache;
    
        friend class GLCommandBuffer;
		friend class GLExtraUniforms;
};

class GLExtraUniforms {
	public:
		GLExtraUniforms();
		GLExtraUniforms( Program::Version version, const GLProgram::VersionData * versionData, GLProgramUniformsCache ** cache );
            
		GLint Find( const char * name, GLint & size, GLenum & type );
	private:
		GLProgramUniformsCache ** fCache;
		const GLProgram::VersionData * fVersionData;
		Program::Version fVersion;
            
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_GLProgram_H__
