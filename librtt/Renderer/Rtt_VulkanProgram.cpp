//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Renderer/Rtt_VulkanRenderer.h"
#include "Renderer/Rtt_VulkanContext.h"
#include "Renderer/Rtt_VulkanProgram.h"

#include "Renderer/Rtt_CommandBuffer.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_ShaderCode.h"
#include "Renderer/Rtt_Texture.h"
#ifdef Rtt_USE_PRECOMPILED_SHADERS
	#include "Renderer/Rtt_ShaderBinary.h"
	#include "Renderer/Rtt_ShaderBinaryVersions.h"
#endif
#include "Core/Rtt_Assert.h"

#include <shaderc/shaderc.h>
#include <algorithm>
#include <utility>
#include <stdarg.h>
#include <stdlib.h>

#ifdef free
#undef free
#endif

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include <spirv_cross/spirv_glsl.hpp>

// To reduce memory consumption and startup cost, defer the
// creation of Vulkan shaders and programs until they're needed.
// Depending on usage, this could result in framerate dips.
#define DEFER_VK_CREATION 1

// ----------------------------------------------------------------------------

namespace /*anonymous*/
{
	using namespace Rtt;

	const char* kWireframeSource =
		"void main()" \
		"{" \
			"gl_FragColor = vec4(1.0);" \
		"}";
}

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

struct VulkanCompilerMaps {
	struct BufferValue {
		BufferValue( size_t offset, size_t range, bool isUniform )
		:	fLocation( offset, range, isUniform ),
			fStages( 0U )
		{
		}

		VulkanProgram::Location fLocation;
		U32 fStages;
	};

	struct SamplerValue {
		SamplerValue( const spirv_cross::SPIRType::ImageType & details, std::vector< U32 > & counts )
		:	fDetails( details ),
			fStages( 0U )
		{
			fCounts.swap( counts );
		}

		spirv_cross::SPIRType::ImageType fDetails;
		std::vector< U32 > fCounts;
		U32 fStages;
	};

	std::map< std::string, BufferValue > buffer_values;
	std::map< std::string, SamplerValue > samplers;
	std::map< std::string, int > varyings;

	U32 CheckForSampler( const std::string & key /* TODO: info... */ );
	VulkanProgram::Location CheckForUniform( const std::string & key );
};

U32
VulkanCompilerMaps::CheckForSampler( const std::string & key /* TODO: info... */ )
{
	auto iter = samplers.find( key );

	if (iter != samplers.end())
	{
		auto imageType = iter->second;

		return 0U;	// TODO: maybe just want binding decorator, from above?
					// imageType would be handy for later expansion...
					// TODO 2: might need array index, etc. now
	}

	else
	{
		return ~0U;
	}
}

VulkanProgram::Location
VulkanCompilerMaps::CheckForUniform( const std::string & key )
{
	auto iter = buffer_values.find( key );

	if (iter != buffer_values.end())
	{
		return iter->second.fLocation;
	}

	else
	{
		return VulkanProgram::Location{};
	}
}

// ----------------------------------------------------------------------------

VulkanProgram::VulkanProgram( VulkanContext * context )
:	fContext( context ),
	fPushConstantStages( 0U ),
	fPushConstantUniforms( false )
{
	for( U32 i = 0; i < Program::kNumVersions; ++i )
	{
		Reset( fData[i] );
	}
}

void 
VulkanProgram::Create( CPUResource* resource )
{
	Rtt_ASSERT( CPUResource::kProgram == resource->GetType() );
	fResource = resource;
	
	#if !DEFER_VK_CREATION
		for( U32 i = 0; i < kMaximumMaskCount + 1; ++i )
		{
			Create( fData[i], i );
		}
	#endif
}

void
VulkanProgram::Update( CPUResource* resource )
{
	Rtt_ASSERT( CPUResource::kProgram == resource->GetType() );
	if( fData[Program::kMaskCount0].IsValid() ) Update( Program::kMaskCount0, fData[Program::kMaskCount0] );
	if( fData[Program::kMaskCount1].IsValid() ) Update( Program::kMaskCount1, fData[Program::kMaskCount1] );
	if( fData[Program::kMaskCount2].IsValid() ) Update( Program::kMaskCount2, fData[Program::kMaskCount2] );
	if( fData[Program::kMaskCount3].IsValid() ) Update( Program::kMaskCount3, fData[Program::kMaskCount3] );
	if( fData[Program::kWireframe].IsValid() ) Update( Program::kWireframe, fData[Program::kWireframe]);
}

void 
VulkanProgram::Destroy()
{
	auto ci = fContext->GetCommonInfo();

	for( U32 i = 0; i < Program::kNumVersions; ++i )
	{
		VersionData& data = fData[i];

	#ifndef Rtt_USE_PRECOMPILED_SHADERS
		vkDestroyShaderModule( ci.device, data.fVertexShader, ci.allocator );
		vkDestroyShaderModule( ci.device, data.fFragmentShader, ci.allocator );
	#endif

		Reset( data );
	}
}

void
VulkanProgram::Bind( VulkanRenderer & renderer, Program::Version version )
{
	VersionData& data = fData[version];
	
	#if DEFER_VK_CREATION
		if( !data.fAttemptedCreation )
		{
			Create( version, data );
		}
	#endif

	std::vector< VkVertexInputAttributeDescription > inputAttributeDescriptions;

	VkVertexInputAttributeDescription description = {};

	description.location = Geometry::kVertexPositionAttribute;
	description.format = VK_FORMAT_R32G32B32_SFLOAT;
	description.offset = offsetof( Geometry::Vertex, x );

	inputAttributeDescriptions.push_back( description );

	description.location = Geometry::kVertexTexCoordAttribute;
	description.offset = offsetof( Geometry::Vertex, u );

	inputAttributeDescriptions.push_back( description );

	description.location = Geometry::kVertexColorScaleAttribute;
	description.format = VK_FORMAT_R8G8B8A8_UNORM;
	description.offset = offsetof( Geometry::Vertex, rs );

	inputAttributeDescriptions.push_back( description );

	description.location = Geometry::kVertexUserDataAttribute;
	description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	description.offset = offsetof( Geometry::Vertex, ux );

	inputAttributeDescriptions.push_back( description );
	
	U32 inputAttributesID = 0U; // TODO: for future use

	renderer.SetAttributeDescriptions( inputAttributesID, inputAttributeDescriptions );

	std::vector< VkPipelineShaderStageCreateInfo > stages;

	VkPipelineShaderStageCreateInfo shaderStageInfo = {};

	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.pName = "main";

	shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageInfo.module = data.fVertexShader;

	stages.push_back( shaderStageInfo );

	shaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageInfo.module = data.fFragmentShader;

	stages.push_back( shaderStageInfo );

	renderer.SetShaderStages( data.fShadersID, stages );
}


void
VulkanProgram::CompileState::Report( const char * prefix )
{
	if (HasError())
	{
		Rtt_TRACE_SIM(( "ERROR: Failed to compile %s:\n\n%s", prefix, fError.c_str() ));
	}
}

void
VulkanProgram::CompileState::SetError( const char * fmt, ... )
{
	char buf[4096];

	va_list argp;

	va_start( argp, fmt );

	vsprintf( buf, fmt, argp );

	va_end( argp );

	fError += buf;
}

bool
VulkanProgram::CompileState::HasError() const
{
	return !fError.empty();
}

U32 VulkanProgram::sID;

void
VulkanProgram::Create( Program::Version version, VersionData& data )
{
	Update( version, data );

	if (data.IsValid())
	{
		data.fShadersID = sID++;
	}

	else
	{
		data.fShadersID = kInvalidID;
	}
}

static int
CountLines( const char **segments, int numSegments )
{
	int result = 0;

	for ( int i = 0; i < numSegments; i++ )
	{
		result += Program::CountLines( segments[i] );
	}

	return result;
}

static bool
NoLeadingCharacters( const std::string & code, size_t pos )
{
	return !isalnum( code[pos - 1] ) && code[pos - 1] != '_'; // TODO: code can be UTF8?
}

static bool
IsAcceptableCodePosition( size_t pos, size_t cpos )
{
	return std::string::npos != pos && pos <= cpos;
}

static bool
GetPrecisionWord( const char * s, char * out, int n )
{
	if ('P' != *s++ || '_' != *s++)
	{
		return false;
	}

	for (int i = 0; i < n; ++i)
	{
		if (isalpha( s[i] ))
		{
			out[i] = s[i];
		}
	
		else
		{
			out[i] = '\0';

			return isspace( s[i] );
		}
	}

	out[n] = '\0';

	return true;
}

static bool
GetPrecision( ShaderCode & code, size_t & pos, size_t cpos, char * precision, int n, VulkanProgram::CompileState & state, const char * what )
{
	pos = code.Skip( pos, isalpha );

	if (!IsAcceptableCodePosition( pos, cpos ))
	{
		state.SetError( "Missing type (and optional precision) after `%s`", what );

		return false;
	}

	if (GetPrecisionWord( code.GetCStr() + pos, precision, n ))
	{
		const char * options[] = { "COLOR", "DEFAULT", "NORMAL", "POSITION", "RANDOM", "UV", NULL }, * choice = NULL;

		for (int i = 0; options[i] && !choice; ++i)
		{
			if (strcmp( precision, options[i] ) == 0)
			{
				choice = options[i];
			}
		}

		if (choice)
		{
			pos += strlen( "P_" ) + strlen( choice ) + 1;
		}

		else
		{
			state.SetError( "Invalid precision" );

			return false;
		}
	}

	return true;
}

static bool
GetTypeWord( const char * s, char * out, int n )
{
	for (int i = 0; i < n; ++i)
	{
		if (isalnum( s[i] ))
		{
			out[i] = s[i];
		}
	
		else
		{
			out[i] = '\0';

			return isspace( s[i] );
		}
	}

	out[n] = '\0';

	return true;
}

static size_t
GetType( ShaderCode & code, size_t & pos, size_t cpos, char * type, int n, VulkanProgram::CompileState & state, const char * what )
{
	pos = code.Skip( pos, isalpha );

	if (std::string::npos == pos || pos > cpos)
	{
		state.SetError( "Missing type after `%s`", what );

		return 0U;
	}

	if (!GetTypeWord( code.GetCStr() + pos, type, n ))
	{
		state.SetError( "Type of %s in shader was ill-formed", what );

		return 0U;
	}

	size_t componentCount = 1U;

	if (strcmp( type, "float" ) != 0)
	{
		Uniform::DataType dataType = Uniform::DataTypeForString( type );

		if (Uniform::kScalar == dataType)
		{
			state.SetError( "Invalid %s type", what );

			return 0U;
		}

		else
		{
			componentCount = Uniform( dataType ).GetNumValues();
		}
	}

	pos += strlen( type ) + 1;

	return componentCount;
}

static int
GetUserDataIndex( ShaderCode & code, size_t & pos, size_t cpos, VulkanProgram::CompileState & state )
{
	pos = code.Skip( pos, isalpha );

	if (!IsAcceptableCodePosition( pos, cpos ))
	{
		state.SetError( "Missing `u_UserData?` after `uniform`" );

		return -1;
	}

	int index;

	if (sscanf( code.GetCStr() + pos, "u_UserData%1i ", &index ) == 0)
	{
		state.SetError( "`u_UserData?` in shader was ill-formed" );

		return -1;
	}

	if (index < 0 || index > 3)
	{
		state.SetError( "Invalid uniform userdata `%i`", index );

		return -1;
	}

	pos += strlen( "u_UserData?" );

	return index;
}

static int
IsPunct( int c )
{
	return ('_' != c && ispunct( c )) ? 1 : 0; // n.b. suppress squigglies on MSVC
}

static char
GetPunctuationAfterDeclaration( ShaderCode & code, size_t & pos, size_t cpos, VulkanProgram::CompileState & state, const char * what )
{
	pos = code.Skip( pos, IsPunct );

	Rtt_ASSERT( IsAcceptableCodePosition( pos, cpos ) );

	char punct = code.GetCStr()[pos++];

	if (punct != ',' && punct != ';')
	{
		state.SetError( "Expected comma-separated %s declaration followed by a semi-colon", what );

		return '\0';
	}

	return punct;
}

#define VP_STR_AND_LEN( str ) str, sizeof( str ) / sizeof( str[0] ) - 1

size_t
VulkanProgram::GatherUniformUserdata( bool isVertexSource, ShaderCode & code, UserdataValue values[], std::vector< UserdataDeclaration > & declarations, CompileState & state )
{
	size_t offset = code.Find( "void main(", 0U ); // skip past declarations in shell; this particular landmark is arbitrary
	size_t result = offset;

	while (true)
	{
		size_t pos = code.Find( "uniform", offset );

		if (std::string::npos == pos)
		{
			return result;
		}

		size_t pastUniformPos = pos + strlen( "uniform" );

		if (NoLeadingCharacters( code.GetString(), pos ) && isspace( code.GetCStr()[pastUniformPos] ))
		{
			size_t cpos = code.Find( ";", pastUniformPos );

			if (std::string::npos == cpos)	// sanity check: must have semi-colon eventually...
			{
				state.SetError( "`uniform` never followed by a semi-colon" );

				return result;
			}

			char precision[16];

			if (!GetPrecision( code, pastUniformPos, cpos, VP_STR_AND_LEN( precision ), state, "uniform" ))
			{
				return result;
			}

			char type[16];
			U32 componentCount = GetType( code, pastUniformPos, cpos, VP_STR_AND_LEN( type ), state, "uniform" );
								
			if (0U == componentCount)
			{
				return result;
			}

			for (char punct = '\0'; ';' != punct; )
			{
				int index = GetUserDataIndex( code, pastUniformPos, cpos, state );

				if (index < 0)
				{
					return result;
				}

				punct = GetPunctuationAfterDeclaration( code, pastUniformPos, cpos, state, "userdata" );

				if ('\0' == punct)
				{
					return result;
				}

				VkShaderStageFlagBits stage = isVertexSource ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;

				if (0 == values[index].fStages)
				{
					values[index].fComponentCount = componentCount;
				}
	
				else if (values[index].fStages & stage)
				{
					state.SetError( "Uniform userdata `%i` already defined in %s stage", index, isVertexSource ? "vertex" : "fragment" );

					return result;
				}

				else if (values[index].fComponentCount != componentCount)
				{
					Rtt_ASSERT( !isVertexSource );

					state.SetError( "Uniform userdata `%i` definition differs between vertex and fragment stages", index );

					return result;
				}

				values[index].fStages |= stage;

				UserdataDeclaration declaration = {};

				declaration.fValue = &values[index];
				declaration.fPosition = pos;

				if (';' == punct) // last on line?
				{
					declaration.fLength = cpos - pos + 1; // include semi-colon as well
				}

				declarations.push_back( declaration );
			}
		}

		offset = pos + 1U;
	}

	return result;
}

bool
VulkanProgram::ReplaceFragCoords( ShaderCode & code, size_t offset, CompileState & state )
{
	std::vector< size_t > posStack;

	while (true)
	{
		size_t pos = code.Find( "gl_FragCoord", offset );

		if (std::string::npos == pos)
		{
			break;
		}

		offset = pos + strlen( "gl_FragCoord" );

		if (NoLeadingCharacters( code.GetString(), pos ))
		{
			size_t cpos = code.Find( ";", offset );

			if (std::string::npos == cpos)	// sanity check: must have semi-colon eventually...
			{
				state.SetError( "`gl_FragCoord` never followed by a semi-colon" );

				return false;
			}

			else if ('_' != code.GetCStr()[offset] && !isalnum( code.GetCStr()[offset] )) // not part of a larger identifier
			{
				posStack.push_back( pos );
			}
		}
	}

	for (auto && iter = posStack.rbegin(); iter != posStack.rend(); ++iter)
	{
		code.Replace( *iter, strlen( "gl_FragCoord" ), "internal_FragCoord" );
	}

	return !posStack.empty();
}

static bool
ReadIdentifier( const char * s, char * out, int n )
{
	for (int i = 0; i < n; ++i)
	{
		if ('_' == s[i] || (i > 0 ? isalnum : isalpha)( s[i] ))
		{
			out[i] = s[i];
		}
	
		else
		{
			out[i] = '\0';

			return i > 0;
		}
	}

	out[n] = '\0';

	return true;
}

static int
IsIdentifierStart( int c )
{
	return ('_' == c || isalpha( c )) ? 1 : 0; // n.b. suppress squigglies on MSVC
}

static bool
GetIdentifier( ShaderCode & code, size_t & pos, size_t cpos, char * name, int n, VulkanProgram::CompileState & state, const char * what )
{
	pos = code.Skip( pos, IsIdentifierStart );

	if (!IsAcceptableCodePosition( pos, cpos ))
	{
		state.SetError( "Missing identifier after `%s`", what );

		return false;
	}

	if (ReadIdentifier( code.GetCStr() + pos, name, n ))
	{
		return true;
	}

	else
	{
		state.SetError( "Missing %s identifier", what ); // does the acceptable code position above rule this out?

		return false;
	}
}

static size_t
FindWordInCode( const ShaderCode & code, const char * word, size_t pos, int (*pred)(int) )
{
	const char * str = code.GetCStr() + pos;
	size_t len = strlen( word );

	if (strncmp( str, word, len ) != 0)
	{
		return 0U;
	}

	if (pred && !pred( str[len] )) // n.b. safe, since we know there is at least a semi-colon up ahead
	{
		return 0U;
	}

	return len;
}

static bool
FoundSamplerKeyword( const ShaderCode & code, size_t & pos, VulkanProgram::CompileState & state )
{
	pos = code.Skip( pos, isalpha );

	if (std::string::npos == pos )
	{
		state.SetError( "Missing identifier after `uniform`" );

		return false;
	}

	size_t len = FindWordInCode( code, "sampler2D", pos, isspace );

	if (len)
	{
		pos += len + 1;
	}

	return !!len;
}

void
VulkanProgram::ReplaceVertexSamplers( ShaderCode & code, CompileState & state )
{
	// In OpenGL 2.* we can declare samplers in the vertex kernel that get picked up as bindings 0, 1, etc.
	// Vulkan assumes a later GLSL that's more stringent about layout and allocation; we can salvage the old
	// behavior by making those declarations, in order, synonyms for the stock samplers.
	// TODO: figure out if this is well-defined behavior

	std::vector< VariablesLine > samplerLineStack;
	size_t offset = 0U;
	int index = 0;

	while (true)
	{
		size_t pos = code.Find( "uniform", offset );

		if (std::string::npos == pos)
		{
			break;
		}

		size_t pastUniformPos = pos + strlen( "uniform" );

		if (NoLeadingCharacters( code.GetString(), pos ) && isspace( code.GetCStr()[pastUniformPos] ))
		{
			size_t cpos = code.Find( ";", pastUniformPos );

			if (std::string::npos == cpos)	// sanity check: must have semi-colon eventually...
			{
				state.SetError( "`uniform` never followed by a semi-colon" );

				return;
			}

			if (FoundSamplerKeyword( code, pastUniformPos, state )) // is this a sampler-type uniform?
			{
				size_t bpos = code.Find( "[", pastUniformPos );

				if (std::string::npos == bpos || cpos < bpos) // not an array? (this rules out u_Samplers) TODO? but of course limits valid usage...
				{
					VariablesLine line;

					for (char punct = '\0'; ';' != punct; )
					{
						char name[64];

						if (!GetIdentifier( code, pastUniformPos, cpos, VP_STR_AND_LEN( name ), state, "sampler2D" ))
						{
							return;
						}

						punct = GetPunctuationAfterDeclaration( code, pastUniformPos, cpos, state, "vertex sampler" );

						if ('\0' == punct)
						{
							return;
						}

						char buf[sizeof( name ) + 32];

						sprintf( buf, "#define %s u_FillSampler%i", name, index++ );

						if (!line.replacement.empty())
						{
							// TODO: find tabs etc?
							line.replacement += '\n';
						}

						line.replacement += buf;
					}

					line.pos = pos;
					line.count = cpos - line.pos + 1;

					samplerLineStack.push_back( line );
				}
			}

			else if (state.HasError())
			{
				return;
			}
		}

		offset = pos + 1U;
	}

	for (auto && iter = samplerLineStack.rbegin(); iter != samplerLineStack.rend(); ++iter)
	{
		code.Replace( iter->pos, iter->count, iter->replacement );
	}
}

void
VulkanProgram::ReplaceVaryings( bool isVertexSource, ShaderCode & code, VulkanCompilerMaps & maps, CompileState & state )
{
	std::vector< VariablesLine > varyingLineStack;
	size_t offset = 0U, varyingLocation = 0U;

	while (true)
	{
		size_t pos = code.Find( "varying", offset );

		if (std::string::npos == pos)
		{
			break;
		}

		size_t pastVaryingPos = pos + strlen( "varying" );

		if (NoLeadingCharacters( code.GetString(), pos ) && isspace( code.GetCStr()[pastVaryingPos] ))
		{
			size_t cpos = code.Find( ";", pastVaryingPos );

			if (std::string::npos == cpos)	// sanity check: must have semi-colon eventually...
			{
				state.SetError( "`varying` never followed by a semi-colon" );

				return;
			}

			char precision[16];

			if (!GetPrecision( code, pastVaryingPos, cpos, VP_STR_AND_LEN( precision ), state, "varying" ))
			{
				return;
			}

			char type[16];
			U32 componentCount = GetType( code, pastVaryingPos, cpos, VP_STR_AND_LEN( type ), state, "varying" );
								
			if (0U == componentCount)
			{
				return;
			}

			VariablesLine line;

			for (char punct = '\0'; ';' != punct; )
			{
				char name[64];

				if (!GetIdentifier( code, pastVaryingPos, cpos, VP_STR_AND_LEN( name ), state, "varying" ))
				{
					return;
				}

				punct = GetPunctuationAfterDeclaration( code, pastVaryingPos, cpos, state, "varying" );

				if ('\0' == punct)
				{
					return;
				}

				char buf[sizeof( name ) + 64];

				if (isVertexSource)
				{
					sprintf( buf, "layout(location = %u) out %s %s;", varyingLocation, type, name );

					maps.varyings[name] = varyingLocation++;
				}

				else
				{
					auto & varying = maps.varyings.find( name );

					if (maps.varyings.end() == varying)
					{
						state.SetError( "Fragment kernel refers to varying `%s`, not found on vertex side", name );

						return;
					}

					sprintf( buf, "layout(location = %u) in %s %s;", varying->second, type, name );
				}
					
				if (!line.replacement.empty())
				{
					// TODO: find tabs etc?
					line.replacement += '\n';
				}

				line.replacement += buf;
			}

			line.pos = pos;
			line.count = cpos - line.pos + 1;

			varyingLineStack.push_back( line );
		}

		offset = pos + 1U;
	}

	for (auto && iter = varyingLineStack.rbegin(); iter != varyingLineStack.rend(); ++iter)
	{
		code.Replace( iter->pos, iter->count, iter->replacement );
	}
}

static void
GetUniformBufferMembersAndAssignStages( spirv_cross::CompilerGLSL & comp, const spirv_cross::Resource & buffer, VulkanCompilerMaps & maps, shaderc_shader_kind kind )
{
	auto ranges = comp.get_active_buffer_ranges( buffer.id );

	for (auto & br : ranges)
	{
		std::string name = comp.get_member_name( buffer.base_type_id, br.index );
		auto iter = maps.buffer_values.insert( std::make_pair( name, VulkanCompilerMaps::BufferValue( br.offset, br.range, true ) ) );

		iter.first->second.fStages |= 1U << kind;
		// TODO: ensure no clashes when found in both
	}
}

static void
GetPushConstantMembersAndAssignStages( spirv_cross::CompilerGLSL & comp, const spirv_cross::Resource & buffer, VulkanCompilerMaps & maps, shaderc_shader_kind kind, U32 & stages )
{
	auto ranges = comp.get_active_buffer_ranges( buffer.id );

	for (auto & br : ranges)
	{
		std::string name = comp.get_member_name( buffer.base_type_id, br.index );

		maps.buffer_values.insert( std::make_pair( name, VulkanCompilerMaps::BufferValue( br.offset, br.range, false ) ) );
	}

	switch (kind)
	{
		case shaderc_vertex_shader:
			stages |= VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case shaderc_fragment_shader:
			stages |= VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		default:
			break;
	}
}

static void
GetSamplersAndAssignStages( spirv_cross::CompilerGLSL & comp, const spirv_cross::Resource & sampler, VulkanCompilerMaps & maps, shaderc_shader_kind kind )
{
	const spirv_cross::SPIRType & type = comp.get_type_from_variable( sampler.id );
	std::vector< U32 > counts;

	for (uint32_t i = 0; i < type.array.size(); ++i)
	{
		counts.push_back( type.array[i] );
	}

	auto iter = maps.samplers.insert( std::make_pair( sampler.name, VulkanCompilerMaps::SamplerValue( type.image, counts ) ) );

	iter.first->second.fStages |= 1U << kind;
}

void
VulkanProgram::Compile( int ikind, ShaderCode & code, VulkanCompilerMaps & maps, VkShaderModule & module, CompileState & state )
{
	if (state.HasError())
	{
		return;
	}

	shaderc_shader_kind kind = shaderc_shader_kind( ikind );
	const char * what = shaderc_vertex_shader == kind ? "vertex shader" : "fragment shader";
	bool isVertexSource = 'v' == what[0];

	ReplaceVaryings( isVertexSource, code, maps, state );

	const std::string & codeStr = code.GetString();

	shaderc_compilation_result_t result = shaderc_compile_into_spv( fContext->GetCompiler(), codeStr.data(), codeStr.size(), kind, what, "main", fContext->GetCompileOptions() );
	shaderc_compilation_status status = shaderc_result_get_compilation_status( result );

	if (shaderc_compilation_status_success == status)
	{
// TODO: the following is also roughly the Rtt_USE_PRECOMPILED_SHADERS logic we would need, making the proper substitutions for the shaderc_* bits:
		const uint32_t * ir = reinterpret_cast< const uint32_t * >( shaderc_result_get_bytes( result ) );
		VkShaderModuleCreateInfo createShaderModuleInfo = {};

		createShaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createShaderModuleInfo.codeSize = shaderc_result_get_length( result );
		createShaderModuleInfo.pCode = ir;

		spirv_cross::CompilerGLSL comp( ir, createShaderModuleInfo.codeSize / 4U );
		spirv_cross::ShaderResources resources = comp.get_shader_resources();

		for (auto & buffer : resources.uniform_buffers)
		{
			GetUniformBufferMembersAndAssignStages( comp, buffer, maps, kind );
		}
		
		for (auto & buffer : resources.push_constant_buffers) // 0 or 1
		{
			GetPushConstantMembersAndAssignStages(  comp, buffer, maps, kind, fPushConstantStages );
		}

		for (auto & sampler : resources.sampled_images)
		{
			GetSamplersAndAssignStages( comp, sampler, maps, kind );
		}

		if (VK_SUCCESS != vkCreateShaderModule( fContext->GetDevice(), &createShaderModuleInfo, fContext->GetAllocator(), &module ))
		{
			state.SetError( "Failed to create shader module!" );
		}
	}

	else
	{
		state.SetError( shaderc_result_get_error_message( result ) );
	}

	shaderc_result_release( result );
}

static bool
AreRowsOpen( const std::vector< int > & used, int row, int nrows, int ncols )
{
	for (int i = 0; i < nrows; ++i)
	{
		int count = used[row++];

		if (count + ncols > 4)
		{
			return false;
		}
	}

	return true;
}

static int
GetFirstRow( const std::vector< int > & used, int nrows, int ncols )
{
	for (int row = 0, last = used.size() - nrows; row <= last; ++row)
	{
		if (AreRowsOpen( used, row, nrows, ncols ))
		{
			return row;
		}
	}

	return -1;
}

std::pair< bool, int >
VulkanProgram::SearchForFreeRows( const UserdataValue values[], UserdataPosition positions[], size_t spareVectorCount )
{
	const VkPhysicalDeviceLimits & limits = fContext->GetDeviceDetails().properties.limits;

	std::vector< int > used( spareVectorCount, 0 );

	for (int i = 0; i < 4 && values[i].IsValid(); ++i)
	{
		int ncols, nrows;

		switch (values[i].fComponentCount)
		{
			case 16:
				ncols = nrows = 4;
				break;
			case 9:
				ncols = 4;
				nrows = 3;
				break;
			default:
				nrows = 1;
				ncols = values[i].fComponentCount;
				break;
		}

		int row = GetFirstRow( used, nrows, ncols );

		if (row >= 0)
		{
			positions[i].fRow = row;
			positions[i].fOffset = used[row];

			for (int j = 0; j < nrows; ++j)
			{
				used[row + j] += ncols;
			}
		}

		else
		{
			return std::make_pair( false, i );
		}
	}

	return std::make_pair( true, 0 );
}

const char *
VulkanProgram::PrepareTotalTimeReplacement( UserdataValue values[], int index, std::string & replacement )
{
	replacement = "float u_UserData";

	replacement += '0' + values[index].fIndex;
	replacement += ';';

	return "float TotalTime;";
}

static void
PadVector( std::string & str, U32 lastUpTo, U32 & paddingCount)
{
	const char * padding[] = { "vec3 pad", "vec2 pad" "float pad" };

	str += padding[lastUpTo - 1];
	str += '0' + paddingCount;
	str += ";  ";

	++paddingCount;
}

const char *
VulkanProgram::PrepareTexelSizeReplacement( UserdataValue values[], int startingIndex, U32 total, U32 & paddingCount, std::string & replacement )
{
	for (int i = startingIndex; i < 4 && values[i].IsValid(); ++i)
	{
		if (i > 0)
		{
			replacement += " ";
		}

		AddToString( replacement, values[i] );
	}

	if (total < 4U)
	{
		replacement += " ";

		PadVector( replacement, total, paddingCount );
	}

	return "vec4 TexelSize;";
}

void
VulkanProgram::PackUserData( UserdataPosition positions[], U32 & paddingCount, std::string & extra )
{
	U32 lastComponentCount;

	for (int i = 0; i < 4 && positions[i].IsValid(); ++i)
	{
		extra += " ";

		if (i > 0 && 0U == positions[i].fOffset)
		{
			U32 lastUpTo = positions[i - 1].fOffset + lastComponentCount;

			if (lastUpTo < 4U) // incomplete vector?
			{
				PadVector( extra, lastUpTo, paddingCount );
			}
		}

		lastComponentCount = AddToString( extra, *positions[i].fValue );
	}
}

void
VulkanProgram::InstallDeclaredUserData( ShaderCode & code, const std::vector< UserdataDeclaration > & declarations, int codeExtra, const std::string & prefix1, const std::string & prefix2 )
{
	for (auto && iter = declarations.rbegin(); iter != declarations.rend(); ++iter)
	{
		const char suffix[] = { '0' + iter->fValue->fIndex, 0 };
		std::string declaration = (prefix1 + suffix) + (prefix2 + suffix);

		if (iter->fLength)
		{
			code.Replace( iter->fPosition + codeExtra, iter->fLength, declaration );
		}

		else
		{
			code.Insert( iter->fPosition + codeExtra, declaration + '\n' );
		}
	}
}

static bool
HasPrefix( const std::string & code, const char * prefix, size_t pos )
{
	const size_t len = strlen( prefix );

	if (pos < len || strncmp( code.c_str() + pos - len, prefix, len ) != 0)
	{
		return false;
	}

	return pos == len || NoLeadingCharacters( code, pos - len );
}

static bool
UsesPushConstant( const ShaderCode & code, const char * what, size_t offset )
{
	size_t pos = code.Find( what, offset );

	if (std::string::npos == pos)
	{
		return false;
	}

	const size_t TotalTimeLen = strlen( "TotalTime" );
	const std::string & codeStr = code.GetString();

	if (isalnum( codeStr[pos + TotalTimeLen] ) || codeStr[pos + TotalTimeLen] == '_' )
	{
		return false;
	}

	return HasPrefix( codeStr, "u_", pos ) || HasPrefix( codeStr, "Corona", pos );
}

U32
VulkanProgram::AddToString( std::string & str, const UserdataValue & value )
{
	U32 componentCount = value.fComponentCount;

	switch (componentCount)
	{
	case 16:
		str += "mat4 ";
		break;
	case 9:
		str += "mat3 ";
		break;
	case 4:
		str += "vec4 ";
		break;
	case 3:
		str += "vec3 ";
		break;
	case 2:
		str += "vec2 ";
		break;
	case 1:
		str += "float ";
		break;
	default:
		Rtt_ASSERT_NOT_REACHED();
	}

	str += "UserData";
	str += '0' + value.fIndex;
	str += ";";

	return componentCount;
}

static void
ModifyUserDataMarker( ShaderCode & code, const char * userDataMarker, const std::string & extra, int & codeExtra, bool canUsePushConstants, VulkanProgram::CompileState & state )
{
	size_t epos = code.Find( userDataMarker, 0U );

	if (std::string::npos != epos)
	{
		if (canUsePushConstants)
		{
			codeExtra = code.Insert( epos + strlen( userDataMarker ), extra );
		}

		else
		{
			codeExtra = code.Replace( epos, strlen( userDataMarker ), extra );
		}
	}

	else
	{
		state.SetError( "Failed to find user data marker" );
	}
}

static void
InstallReplacement( ShaderCode & code, const char * toReplace, const std::string & replacement )
{
	size_t pos = code.Find( toReplace, 0U );

	if (std::string::npos != pos)
	{
		code.Replace( pos, strlen( toReplace ), replacement );
	}
}

void
VulkanProgram::UpdateShaderSource( VulkanCompilerMaps & maps, Program* program, Program::Version version, VersionData& data )
{
#ifndef Rtt_USE_PRECOMPILED_SHADERS
	char maskBuffer[] = "#define MASK_COUNT 0\n";
	switch( version )
	{
		case Program::kMaskCount1:	maskBuffer[sizeof( maskBuffer ) - 3] = '1'; break;
		case Program::kMaskCount2:	maskBuffer[sizeof( maskBuffer ) - 3] = '2'; break;
		case Program::kMaskCount3:	maskBuffer[sizeof( maskBuffer ) - 3] = '3'; break;
		default: break;
	}

	const char *program_header_source = program->GetHeaderSource();
	const char *header = ( program_header_source ? program_header_source : "" );

	const char* shader_source[4];
	memset( shader_source, 0, sizeof( shader_source ) );
	shader_source[0] = header;
	shader_source[1] = "#define FRAGMENT_SHADER_SUPPORTS_HIGHP 1\n"; // TODO? this seems a safe assumption on Vulkan...
	shader_source[2] = maskBuffer;

	if ( program->IsCompilerVerbose() )
	{
		// All the segments except the last one
		int numSegments = sizeof( shader_source ) / sizeof( shader_source[0] ) - 1;
		data.fHeaderNumLines = CountLines( shader_source, numSegments );
	}

	CompileState vertexCompileState, fragmentCompileState;
	ShaderCode vertexCode, fragmentCode;

	// Vertex shader.
	{
		shader_source[3] = program->GetVertexShaderSource();

		vertexCode.SetSources( shader_source, sizeof( shader_source ) / sizeof( shader_source[0] ) );

		ReplaceVertexSamplers( vertexCode, vertexCompileState );
	}


	// Fragment shader.
	{
		shader_source[3] = ( version == Program::kWireframe ) ? kWireframeSource : program->GetFragmentShaderSource();

		fragmentCode.SetSources( shader_source, sizeof( shader_source ) / sizeof( shader_source[0] ) );

		size_t fpos = fragmentCode.Find( "#define USING_GL_FRAG_COORD 0", 0U );

		if (std::string::npos == fpos)
		{
			fragmentCompileState.SetError( "Unable to find gl_FragCoord stub" );
		}

		else if (ReplaceFragCoords( fragmentCode, fpos, fragmentCompileState ))
		{
			size_t offsetTo0 = strlen( "#define USING_GL_FRAG_COORD " );

			fragmentCode.Replace( fpos + offsetTo0, 1, "1" );
		}
	}

	if (!vertexCompileState.HasError() && !fragmentCompileState.HasError())
	{
		std::vector< UserdataDeclaration > vertexDeclarations, fragmentDeclarations;

		UserdataValue values[4];

		size_t vertexOffset = GatherUniformUserdata( true, vertexCode, values, vertexDeclarations, vertexCompileState );
		size_t fragmentOffset = GatherUniformUserdata( false, fragmentCode, values, fragmentDeclarations, fragmentCompileState );

		int vertexExtra = 0, fragmentExtra = 0;

		if (
			!(vertexCompileState.HasError() && fragmentCompileState.HasError()) &&
			!(vertexDeclarations.empty() && fragmentDeclarations.empty())
			)
		{
			for (int i = 0; i < 4; ++i)
			{
				values[i].fIndex = i;
			}

			std::sort( values, values + 4 ); // sort from highest component count to lowest

			UserdataPosition positions[4];

			for (int i = 0; i < 4; ++i)
			{
				positions[i].fValue = &values[i];
			}

			const VkPhysicalDeviceLimits & limits = fContext->GetDeviceDetails().properties.limits;
			auto findResult = SearchForFreeRows( values, positions, limits.maxPushConstantsSize / 16 - 6 ); // cf. shell_default_vulkan.lua

			std::sort( positions, positions + 4 ); // sort from lowest (row, offset) to highest

			bool canUsePushConstants = true;
			const char * toReplace = NULL;
			U32 paddingCount = 0U;
			std::string replacement;

			if (!findResult.first)
			{
				U32 total = 0;

				for (int i = findResult.second; i < 4 && values[i].IsValid(); ++i)
				{
					total += values[i].fComponentCount;
				}

				if (1U == total && !UsesPushConstant( vertexCode, "TotalTime", vertexOffset ) && !UsesPushConstant( fragmentCode, "TotalTime", fragmentOffset ))
				{
					toReplace = PrepareTotalTimeReplacement( values, findResult.second, replacement );
				}

				else if (total <= 4U && !UsesPushConstant( vertexCode, "TexelSize", vertexOffset ) && !UsesPushConstant( fragmentCode, "TexelSize", fragmentOffset ))
				{
					toReplace = PrepareTexelSizeReplacement( values, findResult.second, total, paddingCount, replacement );
				}

				else
				{
					canUsePushConstants = false;

					for (int i = 0; i < 4; ++i)
					{
						positions[i].fValue = &values[i];
					}

					auto result = SearchForFreeRows( values, positions, 16U ); // four userdata matrices

					std::sort( positions, positions + 4 ); // as above

					if (!result.first)
					{
						// error!
					}
				}
			}

			std::string extra;

			PackUserData( positions, paddingCount, extra );

			const char * userDataMarker = canUsePushConstants ? "PUSH_CONSTANTS_EXTRA" : "mat4 Stub[4];";

			if (!vertexDeclarations.empty())
			{
				ModifyUserDataMarker( vertexCode, userDataMarker, extra, vertexExtra, canUsePushConstants, vertexCompileState );
			}

			if (!fragmentDeclarations.empty())
			{
				ModifyUserDataMarker( fragmentCode, userDataMarker, extra, fragmentExtra, canUsePushConstants, fragmentCompileState );
			}

			if (!vertexCompileState.HasError() && !fragmentCompileState.HasError())
			{
				std::string userDataGroup( canUsePushConstants ? " pc" : " userDataObject" );
				std::string prefix1( "#define u_UserData" ), prefix2 = userDataGroup + ".UserData";

				InstallDeclaredUserData( vertexCode, vertexDeclarations, vertexExtra, prefix1, prefix2 );
				InstallDeclaredUserData( fragmentCode, fragmentDeclarations, fragmentExtra, prefix1, prefix2 );

				if (toReplace)
				{
					InstallReplacement( vertexCode, toReplace, replacement );
					InstallReplacement( fragmentCode, toReplace, replacement );
				}

				fPushConstantUniforms = canUsePushConstants;
			}
		}

		// Vertex shader.
		Compile( shaderc_vertex_shader, vertexCode, maps, data.fVertexShader, vertexCompileState );

		// Fragment shader.
		Compile( shaderc_fragment_shader, fragmentCode, maps, data.fFragmentShader, fragmentCompileState );
	}

	vertexCompileState.Report( "vertex shader" );
	fragmentCompileState.Report( "fragment shader" );
#else
	// no need to compile, but reflection here...
#endif
	data.fAttemptedCreation = true;
}

void
VulkanProgram::Update( Program::Version version, VersionData& data )
{
	Program* program = static_cast<Program*>( fResource );

	VulkanCompilerMaps maps;
	
	UpdateShaderSource( maps, program, version,	data );

#ifdef Rtt_USE_PRECOMPILED_SHADERS // TODO! (can probably just load spv?)
	ShaderBinary *shaderBinary = program->GetCompiledShaders()->Get(version);
	glProgramBinaryOES(data.fProgram, GL_PROGRAM_BINARY_ANGLE, shaderBinary->GetBytes(), shaderBinary->GetByteCount());
	GL_CHECK_ERROR();
	GLint linkResult = 0;
	glGetProgramiv(data.fProgram, GL_LINK_STATUS, &linkResult);
	if (!linkResult)
	{
		const int MAX_MESSAGE_LENGTH = 1024;
		char message[MAX_MESSAGE_LENGTH];
		GLint resultLength = 0;
		glGetProgramInfoLog(data.fProgram, MAX_MESSAGE_LENGTH, &resultLength, message);
		Rtt_LogException(message);
	}
	int locationIndex;
	locationIndex = glGetAttribLocation(data.fProgram, "a_Position");
	locationIndex = glGetAttribLocation(data.fProgram, "a_TexCoord");
	locationIndex = glGetAttribLocation(data.fProgram, "a_ColorScale");
	locationIndex = glGetAttribLocation(data.fProgram, "a_UserData");
#else
	bool isVerbose = program->IsCompilerVerbose();
	int kernelStartLine = 0;

	// TODO!

	if ( isVerbose )
	{
		kernelStartLine = data.fHeaderNumLines + program->GetVertexShellNumLines();
	}

	if ( isVerbose )
	{
		kernelStartLine = data.fHeaderNumLines + program->GetFragmentShellNumLines();
	}
#endif

	data.fUniformLocations[Uniform::kViewProjectionMatrix] = maps.CheckForUniform( "ViewProjectionMatrix" );
	data.fUniformLocations[Uniform::kMaskMatrix0] = maps.CheckForUniform( "MaskMatrix0" );
	data.fUniformLocations[Uniform::kMaskMatrix1] = maps.CheckForUniform( "MaskMatrix1" );
	data.fUniformLocations[Uniform::kMaskMatrix2] = maps.CheckForUniform( "MaskMatrix2" );
	data.fUniformLocations[Uniform::kTotalTime] = maps.CheckForUniform( "TotalTime" );
	data.fUniformLocations[Uniform::kDeltaTime] = maps.CheckForUniform( "DeltaTime" );
	data.fUniformLocations[Uniform::kTexelSize] = maps.CheckForUniform( "TexelSize" );
	data.fUniformLocations[Uniform::kContentScale] = maps.CheckForUniform( "ContentScale" );
	data.fUniformLocations[Uniform::kContentSize] = maps.CheckForUniform( "ContentSize" );
	data.fUniformLocations[Uniform::kUserData0] = maps.CheckForUniform( "UserData0" );
	data.fUniformLocations[Uniform::kUserData1] = maps.CheckForUniform( "UserData1" );
	data.fUniformLocations[Uniform::kUserData2] = maps.CheckForUniform( "UserData2" );
	data.fUniformLocations[Uniform::kUserData3] = maps.CheckForUniform( "UserData3" );

	data.fMaskTranslationLocations[0] = maps.CheckForUniform( "MaskTranslation0" );
	data.fMaskTranslationLocations[1] = maps.CheckForUniform( "MaskTranslation1" );
	data.fMaskTranslationLocations[2] = maps.CheckForUniform( "MaskTranslation2" );
}

void
VulkanProgram::Reset( VersionData& data )
{
	data.fVertexShader = VK_NULL_HANDLE;
	data.fFragmentShader = VK_NULL_HANDLE;

	const uint32_t kInactiveLocation = ~0U;

	for( U32 i = 0; i < Uniform::kNumBuiltInVariables; ++i )
	{
		data.fUniformLocations[ i ] = kInactiveLocation;

		// CommandBuffer also initializes timestamp to zero
		const U32 kTimestamp = 0;
		data.fTimestamps[ i ] = kTimestamp;
	}

	for ( U32 i = 0; i < 3; ++i )
	{
		data.fMaskTranslationLocations[ i ] = kInactiveLocation;
	}
	
	data.fAttemptedCreation = false;
	data.fHeaderNumLines = 0;
}

void
VulkanProgram::InitializeCompiler( shaderc_compiler_t * compiler, shaderc_compile_options_t * options )
{
	if (compiler && options)
	{
		*compiler = shaderc_compiler_initialize();
		*options = shaderc_compile_options_initialize();
	}
}

void
VulkanProgram::CleanUpCompiler( shaderc_compiler_t compiler, shaderc_compile_options_t options )
{
	shaderc_compiler_release( compiler );
	shaderc_compile_options_release( options );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#undef VP_STR_AND_LEN