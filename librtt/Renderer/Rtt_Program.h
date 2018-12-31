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

#ifndef _Rtt_Program_H__
#define _Rtt_Program_H__

#include "Renderer/Rtt_CPUResource.h"
#include "Core/Rtt_Types.h"

// ----------------------------------------------------------------------------

#if defined( Rtt_USE_PRECOMPILED_SHADERS )
	namespace Rtt { class ShaderBinaryVersions; }
#endif
struct Rtt_Allocator;

namespace Rtt
{

class ProgramHeader;

// ----------------------------------------------------------------------------

class Program : public CPUResource
{
	public:
		typedef CPUResource Super;
		typedef CPUResource Self;

	public:
		typedef enum _Language
		{
			kDefault = 0,
			kOpenGL_2_1 = 1,
			kOpenGL_ES_2 = kDefault,
		}
		Language;

		typedef enum _Version
		{
			kMaskCount0 = 0,
			kMaskCount1,
			kMaskCount2,
			kMaskCount3,
			kWireframe,
			kNumVersions
		}
		Version;

		static const char *HeaderForLanguage( Language language, const ProgramHeader& headerData );

		static int CountLines( const char *str );

	public:
		Program( Rtt_Allocator* allocator );
		virtual ~Program();

		virtual ResourceType GetType() const;
		virtual void Allocate();
		virtual void Deallocate();

		const char *GetVertexShaderSource() const;
		void SetVertexShaderSource( const char *source );

		const char *GetFragmentShaderSource() const;
		void SetFragmentShaderSource( const char *source );

		const char *GetHeaderSource() const { return fHeaderSource; }
		void SetHeaderSource( const char *source );

#if defined( Rtt_USE_PRECOMPILED_SHADERS )
		ShaderBinaryVersions* GetCompiledShaders() const { return fCompiledShaders; }
#endif

		int GetVertexShellNumLines() const { return fVertexShellNumLines; }
		void SetVertexShellNumLines( int newValue ) { fVertexShellNumLines = newValue; }

		int GetFragmentShellNumLines() const { return fFragmentShellNumLines; }
		void SetFragmentShellNumLines( int newValue ) { fFragmentShellNumLines = newValue; }

		bool IsCompilerVerbose() const { return fCompilerVerbose; }
		void SetCompilerVerbose( bool newValue ) { fCompilerVerbose = newValue; }


	private:
		char *fVertexShaderSource;
		char *fFragmentShaderSource;
		char *fHeaderSource;
#if defined( Rtt_USE_PRECOMPILED_SHADERS )
		ShaderBinaryVersions *fCompiledShaders;
#endif
		int fVertexShellNumLines;
		int fFragmentShellNumLines;
		bool fCompilerVerbose;
};

// ----------------------------------------------------------------------------

class ProgramHeader
{
	public:
		typedef enum _Type
		{
			kUnknownType = -1,
			kDefaultType = 0,
			kRandomType,
			kPositionType,
			kNormalType,
			kUVType,
			kColorType,

			// NOTE: Always the last one
			kNumType
		}
		Type;

		typedef enum _Precision
		{
			kUnknownPrecision = -1,
			kLowPrecision = 0,
			kMediumPrecision,
			kHighPrecision,
		}
		Precision;

	public:
		// static const char *StringForType( Type value );
		static Type TypeForString( const char *value );

		static const char *StringForPrecision( Precision value );
		static Precision PrecisionForString( const char *value );

	public:
		ProgramHeader();

	public:
		void CopyHeaderSource( Program::Language language, char *dst, int dstSize ) const;
		void SetPrecision( Type t, Precision p );
		void SetPrecision( Precision p );

	private:
		S8 fPrecision[kNumType]; // Precision values for each Type index 
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Program_H__
