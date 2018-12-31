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

#include "Renderer/Rtt_ProgramFactory.h"

#include "Core/Rtt_Assert.h"
#include "Renderer/Rtt_Program.h"
#include <cstdio>
#include <cstring>

// ----------------------------------------------------------------------------

namespace /*anonymous*/
{
	using namespace Rtt;

	char* LoadFile( const char* filename )
	{
		std::FILE* file = std::fopen( filename, "rb" );
		if( file )
		{
			std::fseek( file, 0, SEEK_END );
			const long int size = std::ftell( file );
			char* result = new char[ size + 1 ];
			std::rewind( file );
			std::fread( result, 1, size, file );
			result[ size ] = '\0';
			std::fclose( file );
			
			return result;
		}
		return NULL;
	}

	char* AppendFunction( const char* function,
							const char* skeleton,
							unsigned int skeletonLength )
	{
		char* result = new char[strlen(function) + skeletonLength + 1];
		strcpy( result, skeleton );
		strcpy( result + skeletonLength, function);
		return result;
	}
}

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

ProgramFactory::ProgramFactory()
:	fVertSource( NULL ),
	fFragSource( NULL )
{
}

ProgramFactory::~ProgramFactory()
{
	delete [] fVertSource;
	delete [] fFragSource;
}

Program* ProgramFactory::Create( const char* vertexFunction, const char* vertexSkeleton,
                                 const char* fragmentFunction, const char* fragmentSkeleton )
{
	if( !fVertSource || !fFragSource )
	{
		// Load the skeleton shader code
        fVertSource = LoadFile(vertexSkeleton);
        fFragSource = LoadFile(fragmentSkeleton);

		// Cache string lengths for efficiency
		fVertLength = (unsigned int)strlen(fVertSource);
		fFragLength = (unsigned int)strlen(fFragSource);
	}

	// The skeleton code declares the functions, so just append the definitions
	char* finalVertSource = AppendFunction( vertexFunction, fVertSource, fVertLength );
	char* finalFragSource = AppendFunction( fragmentFunction, fFragSource, fFragLength );

	Program* program = new Program(NULL);
	program->SetVertexShaderSource( finalVertSource );
	program->SetFragmentShaderSource( finalFragSource );

	delete [] finalVertSource;
	delete [] finalFragSource;

	return program;
}

bool
ProgramFactory::Initialize( const char* vertexSkeletonSource, const char* fragmentSkeletonSource )
{
	bool result = false;

	if( !fVertSource || !fFragSource )
	{
		fVertLength = (unsigned int)strlen(vertexSkeletonSource);
		fFragLength = (unsigned int)strlen(fragmentSkeletonSource);

        fVertSource = new char[ fVertLength + 1 ];
		memcpy( fVertSource, vertexSkeletonSource, fVertLength + 1 );

        fFragSource = new char[ fFragLength + 1 ];
		memcpy( fFragSource, fragmentSkeletonSource, fVertLength + 1 );
		result = true;
	}

	return result;
}

Program *
ProgramFactory::Create( const char* vertexFunction, const char* fragmentFunction )
{
	Rtt_ASSERT( fVertSource && fFragSource );

	// The skeleton code declares the functions, so just append the definitions
	char* finalVertSource = AppendFunction( vertexFunction, fVertSource, fVertLength );
	char* finalFragSource = AppendFunction( fragmentFunction, fFragSource, fFragLength );

	Program* program = new Program( NULL );
	program->SetVertexShaderSource( finalVertSource );
	program->SetFragmentShaderSource( finalFragSource );

	delete [] finalVertSource;
	delete [] finalFragSource;

	return program;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
