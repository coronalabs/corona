//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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

		VersionData fData[Program::kNumVersions];
		CPUResource* fResource;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_GLProgram_H__
