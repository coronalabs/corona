//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
class ShaderResource;

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
			kVulkanGLSL = 2
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

		ShaderResource *GetShaderResource() { return fResource; }
		void SetShaderResource( ShaderResource *resource ) { fResource = resource; }
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
		ShaderResource *fResource;
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
