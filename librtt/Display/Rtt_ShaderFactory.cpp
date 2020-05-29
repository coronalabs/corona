//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_ShaderFactory.h"

#include "Corona/CoronaLua.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayDefaults.h"
#include "Display/Rtt_Paint.h"
#include "Display/Rtt_ShaderBuiltin.h"
#include "Display/Rtt_ShaderData.h"
#include "Display/Rtt_ShaderName.h"
#include "Display/Rtt_ShaderResource.h"
#include "Renderer/Rtt_Program.h"
#if defined( Rtt_USE_PRECOMPILED_SHADERS )
	#include "Renderer/Rtt_ShaderBinary.h"
	#include "Renderer/Rtt_ShaderBinaryVersions.h"
#endif
#include "Renderer/Rtt_Uniform.h"
#include "Rtt_Lua.h"
#include "Rtt_Runtime.h"

#include "Display/Rtt_ShaderInput.h"
#include "Display/Rtt_ShaderProxy.h"
#include "Rtt_LuaContainer.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static const char kMetatableName[] = __FILE__; // Globally unique string to prevent collision

int
ShaderFactory::ShaderFinalizer( lua_State *L )
{
	Shader *shader = (Shader *)CoronaLuaToUserdata( L, 1 );

	Rtt_DELETE( shader );

	return 0;
}

void
ShaderFactory::PushTable( lua_State *L, const char *key )
{
	lua_pushstring( L, key );
	lua_gettable( L, LUA_REGISTRYINDEX );

	// Create table for key
	if ( lua_isnil( L, -1 ) )
	{
		lua_pop( L, 1 ); // pop nil

		lua_newtable( L ); // t

		lua_pushstring( L, key );
		lua_pushvalue( L, -2 );
		lua_settable( L, LUA_REGISTRYINDEX ); // LUA_REGISTRY[key] = t

		// Leave t at top of stack
	}
}

void
ShaderFactory::RegisterBuiltin( lua_State *L, ShaderTypes::Category category )
{
	PushTable( L, ShaderBuiltin::KeyForCategory( category ) );
	ShaderBuiltin::Register( L, category );
	lua_pop( L, 1 );
}

// ----------------------------------------------------------------------------

ShaderFactory::ShaderFactory( Display& owner, const ProgramHeader& programHeader )
:	fAllocator( owner.GetAllocator() ),
	fDefaultColorShader( NULL ),
	fDefaultShader( NULL ),
	fL( CoronaLuaNew( kCoronaLuaFlagOpenStandardLibs ) ),
	fOwner( owner ),
	fDefaultShell( NULL ),
	fDefaultKernel( NULL ),
	fProgramHeader( Rtt_NEW( fAllocator, ProgramHeader( programHeader ) ) )
{
	lua_State *L = fL;

	// Register __gc callback
	CoronaLuaInitializeGCMetatable( L, kMetatableName, ShaderFinalizer );

	Initialize();
}

ShaderFactory::~ShaderFactory()
{
	Rtt_DELETE( fProgramHeader );
	Rtt_DELETE( fDefaultKernel );
	Rtt_DELETE( fDefaultShell );
	CoronaLuaDelete( fL );
	Rtt_DELETE( fDefaultColorShader );
	Rtt_DELETE( fDefaultShader );
}

bool
ShaderFactory::Initialize()
{
	bool result = false;

	lua_State *L = fL;

	int top = lua_gettop( L );

	lua_checkstack( L, 6 );

#if defined( Rtt_USE_PRECOMPILED_SHADERS )
	// Load precompiled shaders from the default kernel Lua script.
	if ( ShaderBuiltin::PushDefaultKernel( L ) )
	{
		lua_getfield( L, -1, "compiledShaders" );
		if ( lua_istable( L, -1 ) )
		{
			ShaderBinaryVersions compiledDefaultShaders(fAllocator);
			ShaderBinaryVersions compiled25DShaders(fAllocator);
			bool wasLoaded = LoadAllCompiledShaders(L, lua_gettop(L), compiledDefaultShaders, compiled25DShaders);
			if (wasLoaded)
			{
				Program *program = NewProgram(compiledDefaultShaders, ShaderResource::kDefault);
				if (program)
				{
					SharedPtr< ShaderResource > resource(
									Rtt_NEW(fAllocator, ShaderResource(program, ShaderTypes::kCategoryDefault)));
					Program *program25D = NewProgram(compiled25DShaders, ShaderResource::k25D);
					resource->SetProgramMod(ShaderResource::k25D, program25D);
					fDefaultShader = Rtt_NEW(fAllocator, Shader(fAllocator, resource, NULL));
					result = true;
				}
			}
		}
	}
#else
	// Load the default shell and kernel shader source code to be dynamically compiled later.
	if ( ShaderBuiltin::PushDefaultShell( L ) )
	{
		int tableIndex = lua_gettop( L );

		lua_getfield( L, tableIndex, "vertex" );
		const char *shellVert = lua_tostring( L, -1 );

		lua_getfield( L, tableIndex, "fragment" );
		const char *shellFrag = lua_tostring( L, -1 );

		if ( ShaderBuiltin::PushDefaultKernel( L ) )
		{
			tableIndex = lua_gettop( L );

			lua_getfield( L, tableIndex, "vertex" );
			const char *kernelVert = lua_tostring( L, -1 );

			lua_getfield( L, tableIndex, "fragment" );
			const char *kernelFrag = lua_tostring( L, -1 );

			
			Program *program = NewProgram( shellVert, shellFrag, kernelVert, kernelFrag, ShaderResource::kDefault );
			if ( program )
			{
				SharedPtr< ShaderResource > resource( Rtt_NEW( fAllocator, ShaderResource( program, ShaderTypes::kCategoryDefault ) ) );
				
				Program *program25D = NewProgram( shellVert, shellFrag, kernelVert, kernelFrag, ShaderResource::k25D );
				resource->SetProgramMod(ShaderResource::k25D, program25D);
				
				fDefaultShader = Rtt_NEW( fAllocator, Shader(fAllocator, resource, NULL ) );
				result = true;

				Rtt_Allocator *allocator = fOwner.GetRuntime().Allocator();
				fDefaultShell = Rtt_NEW( allocator, Program( allocator ) );
				fDefaultShell->SetVertexShaderSource( shellVert );
				fDefaultShell->SetFragmentShaderSource( shellFrag );

				fDefaultKernel = Rtt_NEW( allocator, Program( allocator ) );
				fDefaultKernel->SetVertexShaderSource( kernelVert );
				fDefaultKernel->SetFragmentShaderSource( kernelFrag );
			}
		}
	}
#endif

	lua_settop( L, top );

	// Register built-ins
	RegisterBuiltin( L, ShaderTypes::kCategoryFilter );
	RegisterBuiltin( L, ShaderTypes::kCategoryComposite );
	RegisterBuiltin( L, ShaderTypes::kCategoryGenerator );

	return result;
}

#if defined( Rtt_USE_PRECOMPILED_SHADERS )
Program *
ShaderFactory::NewProgram(ShaderBinaryVersions &compiledShaders, ShaderResource::ProgramMod mod) const
{
	// Fetch the allocator.
	Rtt_Allocator *allocatorPointer = fOwner.GetRuntime().Allocator();

	// Store the given compiled shaders into a new program object.
	Program *programPointer = Rtt_NEW(allocatorPointer, Program(allocatorPointer));
	if (programPointer)
	{
		programPointer->GetCompiledShaders()->CopyFrom(&compiledShaders);
	}

	// Return the new program containing the given shaders.
	return programPointer;
}

SharedPtr< ShaderResource >
ShaderFactory::NewShaderResource(
	ShaderTypes::Category category, const char *name,
	ShaderBinaryVersions &compiledDefaultShaders, ShaderBinaryVersions &compiled25DShaders)
{
	// Caller is not allowed to create a "default" shader.
	if (ShaderTypes::kCategoryDefault == category)
	{
		return SharedPtr< ShaderResource >();
	}

	// Check if the given shader was already created.
	// Note: Caller should check for existence and avoid creating a duplicate shader resource.
	Rtt_ASSERT( NULL == FindPrototype( category, name ) );

	// Create a new resource object and have it store the given compiled shaders.
	Program *program = NewProgram( compiledDefaultShaders, ShaderResource::kDefault );
	SharedPtr< ShaderResource > result( Rtt_NEW( fAllocator, ShaderResource( program, category, name ) ) );
	Program *program25D = NewProgram( compiled25DShaders, ShaderResource::k25D );
	result->SetProgramMod( ShaderResource::k25D, program25D );

	// Return the resource storing the given compiled shaders.
	return result;
}

#else
Program *
ShaderFactory::NewProgram(
		const char *shellVert,
		const char *shellFrag,
		const char *kernelVert,
		const char *kernelFrag,
		ShaderResource::ProgramMod mod ) const
{
	Rtt_ASSERT( shellVert );
	Rtt_ASSERT( shellFrag );
	Rtt_ASSERT( kernelVert );
	Rtt_ASSERT( kernelFrag );

	lua_State *L = fL;

	Rtt_Allocator *allocator = fOwner.GetRuntime().Allocator();

	Program *program = Rtt_NEW( allocator, Program( allocator ) );
	
	bool isCompilerVerbose = fOwner.GetDefaults().IsShaderCompilerVerbose();
	program->SetCompilerVerbose( isCompilerVerbose );

	if ( isCompilerVerbose )
	{
		program->SetVertexShellNumLines( Program::CountLines( shellVert ) );
	}

#if defined( Rtt_EMSCRIPTEN_ENV )
	const char *sourceVert = lua_pushfstring( L, shellVert, kernelVert );
#else
	const char *sourceVert = lua_pushfstring( L, "%s%s", shellVert, kernelVert );
#endif

	program->SetVertexShaderSource( sourceVert );
//	Rtt_TRACE( ( "Vertex source:\n%s\n", program->GetVertexShaderSource() ) );
	lua_pop( L, 1 );

	if ( isCompilerVerbose )
	{
		program->SetFragmentShellNumLines( Program::CountLines( shellFrag ) );
	}

#if defined( Rtt_EMSCRIPTEN_ENV )
	const char *sourceFrag = lua_pushfstring( L, shellFrag, kernelFrag );
#else
	const char *sourceFrag = lua_pushfstring( L, "%s%s", shellFrag, kernelFrag );
#endif

	program->SetFragmentShaderSource( sourceFrag );
//	Rtt_TRACE( ( "Fragment source:\n%s\n", program->GetFragmentShaderSource() ) );
	lua_pop( L, 1 );

	//TODO - move this into a delegate block, rather than extending this
	{
		#if defined( Rtt_OPENGLES )
			// We are using OpenGL ES, so assume it's v.2.0
			// We could also look at GL_ES_VERSION_2_0 and GL_ES_VERSION_3_0.
			Program::Language language = Program::kOpenGL_ES_2;
		#else // NOT Rtt_OPENGLES
			// We are using Desktop OpenGL, so assume it's OpenGL 2.1
			// We could also look at GL_VERSION_2_0 and GL_VERSION_2_1.
			Program::Language language = Program::kOpenGL_2_1;
		#endif
		
		std::string header = Program::HeaderForLanguage( language, * fProgramHeader );
		
		if (ShaderResource::k25D == mod)
		{
			header = header + std::string("#define TEX_COORD_Z 1\n");
		}

#if defined( Rtt_EMSCRIPTEN_ENV )
		header = header +  "#define Rtt_WEBGL_ENV\n";
#endif
		
		if (header.length() != 0)
		{
			program->SetHeaderSource( header.c_str() );
		}
	}

	return program;
}

SharedPtr< ShaderResource >
ShaderFactory::NewShaderResource(
	ShaderTypes::Category category,
	const char *name,
	const char *kernelVert,
	const char *kernelFrag )
{
	// Cannot create default
	if ( ShaderTypes::kCategoryDefault == category )
	{
		return SharedPtr< ShaderResource >();
	}

	// Caller should check for existence
	Rtt_ASSERT( NULL == FindPrototype( category, name ) );

	if ( ! kernelVert )
	{
		// Fallback to default
		kernelVert = fDefaultKernel->GetVertexShaderSource();
	}

	if ( ! kernelFrag )
	{
		// Fallback to default
		kernelFrag = fDefaultKernel->GetFragmentShaderSource();
	}

	Program *program = NewProgram(
		fDefaultShell->GetVertexShaderSource(),
		fDefaultShell->GetFragmentShaderSource(),
		kernelVert,
		kernelFrag,
		ShaderResource::kDefault );

	SharedPtr< ShaderResource > result( Rtt_NEW( fAllocator, ShaderResource( program, category, name ) ) );
	
	Program *program25D = NewProgram( fDefaultShell->GetVertexShaderSource(), fDefaultShell->GetFragmentShaderSource(), kernelVert, kernelFrag, ShaderResource::k25D );
	result->SetProgramMod( ShaderResource::k25D, program25D );

	return result;
}
#endif

// This is an internal factory method:
// * it creates the full-loaded prototype
// * it adds it into the registry, so subsequent calls to FindPrototype() succeed
Shader *
ShaderFactory::NewShaderPrototype( lua_State *L, int index, const SharedPtr< ShaderResource >& resource )
{
	// index for table that defines the shader
	// This also inits the resource's defaultData, so need to do this before we create the Shader
	InitializeBindings( L, index, resource );

	ShaderData *data = resource->GetDefaultData()->Clone( fAllocator );
	//Shader *result = Rtt_NEW( fAllocator, Shader(fAllocator, resource, data ) );

	Shader *result = NULL;
	//Create composite
	ShaderComposite *comp = Rtt_NEW( fAllocator, ShaderComposite(fAllocator, resource, data ) );
	result = comp;
	
	AddShader( result, resource->GetName().c_str() );

	return result;
}

bool
ShaderFactory::BindVertexDataMap( lua_State *L, int index, const SharedPtr< ShaderResource >& resource )
{
	Rtt_LUA_STACK_GUARD( L );

	lua_getfield( L, index, "vertexData" );

	bool result = false;

	if ( lua_istable( L, -1 ) )
	{
		int t = lua_gettop( L );
		
		ShaderResource::VertexDataMap& map = resource->GetVertexDataMap();
		
		// NOTE: Lua is 1-based
		for ( int i = 1, iMax = (int) lua_objlen( L, t ); i <= iMax; i++ )
		{
			Rtt_LUA_STACK_GUARD( L );
			
			lua_rawgeti( L, t, i );
			{
				int valueIndex = lua_gettop( L );
				if ( lua_istable( L, valueIndex )  )
				{
					// Get index position into vec4 (userData)
					lua_getfield( L, valueIndex, "index" );
					int index = (int) lua_tointeger( L, -1 );
					index = Min( Max( 0, index ), (int)ShaderData::kDataMax );
					lua_pop( L, 1 );
					
					// Store mapping between effect property (Lua)
					// and underlying vec4 position
					lua_getfield( L, valueIndex, "name" );
					const char *key = lua_tostring( L, -1 );
					if( NULL != key )
					{
						map[key] = index;
					}
					lua_pop( L, 1 );
					
					lua_getfield( L, valueIndex, "default" );
					Real value = Rtt_FloatToReal( (float) lua_tonumber( L, -1 ) );
					ShaderData *defaultData = resource->GetDefaultData();
					Rtt_ASSERT( defaultData ); // Caller should init
					defaultData->SetVertexData( (ShaderData::DataIndex)index, value );
					lua_pop( L, 1 );
				}
			}
			lua_pop( L, 1 );
		}
		result = true;
	}

	lua_pop( L, 1 );

	return result;
}

bool
ShaderFactory::BindUniformDataMap( lua_State *L, int index, const SharedPtr< ShaderResource >& resource )
{
	Rtt_LUA_STACK_GUARD( L );

	lua_getfield( L, index, "uniformData" );
	
	bool result = false;
	if ( lua_istable( L, -1 ) )
	{
		int t = lua_gettop( L );
		
		ShaderResource::UniformDataMap& map = resource->GetUniformDataMap();
		
		ShaderData *defaultData = resource->GetDefaultData();
		Rtt_ASSERT( defaultData ); // Caller should init
		
		// NOTE: Lua is 1-based
		for ( int i = 1, iMax = (int) lua_objlen( L, t ); i <= iMax; i++ )
		{
			lua_rawgeti( L, t, i );
			{
				int valueIndex = lua_gettop( L );
				if ( lua_istable( L, valueIndex )  )
				{
					lua_getfield( L, valueIndex, "index" );
					int idx = (int) lua_tointeger( L, -1 );
					idx = Min( Max( 0, idx ), (int)ShaderData::kDataMax );
					lua_pop( L, 1 );
					
					lua_getfield( L, valueIndex, "type" );
					const char *value = lua_tostring( L, -1 );
					Uniform::DataType dataType = Uniform::DataTypeForString( value );
					lua_pop( L, 1 );
					
					lua_getfield( L, valueIndex, "name" );
					const char *key = lua_tostring( L, -1 );
					ShaderResource::UniformData data = { idx, dataType };
					if( NULL != key)
					{
						map[key] = data;
					}
					lua_pop( L, 1 );
					
					lua_getfield( L, valueIndex, "default" );
					ShaderData::DataIndex dataIndex = (ShaderData::DataIndex)idx;
					defaultData->InitializeUniform( fOwner.GetAllocator(), dataIndex, dataType );
					defaultData->SetUniform( L, -1, dataIndex );
					lua_pop( L, 1 );
				}
			}
			lua_pop( L, 1 );
		}
		result = true;
	}
	lua_pop( L, 1 );
	
	return result;
}

static void
Modulo( Real *x, Real range, Real, Real )
{
	*x = fmod( *x, range ); // TODO?: Rtt_RealFmod
}

static void
PingPong( Real *x, Real range, Real, Real )
{
	Real pos = fmod( *x, Rtt_REAL_2 * range ); // TODO?: Rtt_RealFmod

	if (pos > range)
	{
		pos = Rtt_REAL_2 * range - pos;
	}

	*x = pos;
}

static void
Sine( Real *x, Real amplitude, Real speed, Real shift )
{
	Real t = *x;

	*x = amplitude * Rtt_RealSin( speed * t + shift );
}

static void
GetNumberArg( lua_State * L, Real * value, const char * func, const char * name )
{
	lua_getfield( L, -2, name ); // ..., xform, func, value?
		
	if (!lua_isnil( L, -1 ))
	{
		if (lua_isnumber( L, -1 ))
		{
			*value = (Real)lua_tonumber( L, -1 );
		}

		else
		{
			CoronaLuaWarning( L, "graphics.defineEffect() ignoring invalid '%s' parameter for %s time transform (expected number but got %s)",
						name, func, lua_typename( L, lua_type( L, -1 ) ) );
		}
	}

	lua_pop( L, 1 ); // ..., xform, func
}

static void
GetPositiveNumberArg( lua_State * L, Real * value, const char * func, const char * name )
{
	Real old = *value;

	GetNumberArg( L, value, func, name );

	if (*value <= Rtt_REAL_0)
	{
		*value = old;

		CoronaLuaWarning( L, "graphics.defineEffect() ignoring invalid '%s' parameter for %s time transform (must be positive number)",
			name, func );
	}
}

void
ShaderFactory::BindTimeTransform(lua_State *L, int index, const SharedPtr< ShaderResource >& resource)
{
	Rtt_LUA_STACK_GUARD(L);

	lua_getfield( L, index, "timeTransform" ); // ..., xform?

	if (lua_istable( L, -1 ))
	{
		lua_getfield( L, -1, "func" );	// ..., xform, func

		if (lua_isstring( L, -1 ))
		{
			const char *func = lua_tostring( L, -1 );
			TimeTransform *transform = NULL;
			bool isModulo = strcmp(func, "modulo") == 0;

			if (isModulo || strcmp( func, "pingpong" ) == 0)
			{
				Real range = Rtt_REAL_1;
				
				GetPositiveNumberArg( L, &range, func, "range" );

				transform = Rtt_NEW( fAllocator, TimeTransform );

				transform->func = isModulo ? &Modulo : &PingPong;
				transform->arg1 = range;
			}

			else if (strcmp( func, "sine" ) == 0)
			{
				Real amplitude = Rtt_REAL_1, period = Rtt_REAL_2 * M_PI, shift = Rtt_REAL_0;

				GetNumberArg( L, &amplitude, func, "amplitude" );
				GetPositiveNumberArg( L, &period, func, "period" );
				GetNumberArg( L, &shift, func, "shift" );

				transform = Rtt_NEW( fAllocator, TimeTransform );

				transform->func = &Sine;
				transform->arg1 = amplitude;
				transform->arg2 = (Rtt_REAL_2 * M_PI) / period;
				transform->arg3 = shift;
			}

			else
			{
				CoronaLuaWarning( L, "graphics.defineEffect() ignoring unknown %s time transform", func );
			}

			resource->SetTimeTransform( transform );
		}

		lua_pop( L, 1 ); // ..., xform
	}

	lua_pop( L, 1 ); // ...
}

// shaderIndex is the index into the Lua table that defines the shader
void
ShaderFactory::InitializeBindings( lua_State *L, int shaderIndex, const SharedPtr< ShaderResource >& resource )
{
	bool usesUniforms = false;

	ShaderName name( resource->GetCategory(), resource->GetName().c_str() );
	ShaderData *defaultData = Rtt_NEW( fOwner.GetAllocator(), ShaderData( resource ) );
	resource->SetDefaultData( defaultData );

	if (resource->UsesTime())
	{
		BindTimeTransform( L, shaderIndex, resource );
	}

	bool has_vertex_data = BindVertexDataMap( L, shaderIndex, resource );
	if( has_vertex_data )
	{
		// We've arbitrarily decided that we want vertex data and uniform
		// data to be mutually exclusive. Therefore, we want to verify that
		// uniform HASN'T been provided here.
		Rtt_LUA_STACK_GUARD( L );

		lua_getfield( L, shaderIndex, "uniformData" );
		if( lua_istable( L, -1 ) )
		{
			CORONA_LOG_ERROR( "Shader (%s) for category (%s) has BOTH vertexData and uniformData, which are mutually exclusive.",
								resource->GetName().c_str(), ShaderTypes::StringForCategory( resource->GetCategory() ) );
		}
		lua_pop( L, 1 );
	}
	else
	{
		BindUniformDataMap( L, shaderIndex, resource );
		usesUniforms = true;
	}

	resource->SetUsesUniforms( usesUniforms );
}

#if defined( Rtt_USE_PRECOMPILED_SHADERS )
bool
ShaderFactory::LoadAllCompiledShaders(
	lua_State *L, int compiledShadersTableIndex,
	ShaderBinaryVersions &compiledDefaultShaders, ShaderBinaryVersions &compiled25DShaders)
{
	int shaderModesLoadedCount = 0;

	// Validate arguments.
	if (!L || !compiledShadersTableIndex)
	{
		return false;
	}

	// Make sure that the given index references a Lua table.
	if (!lua_istable(L, compiledShadersTableIndex))
	{
		return false;
	}

	// Free the previously loaded shader binaries, if any.
	compiledDefaultShaders.FreeAllShaders();
	compiled25DShaders.FreeAllShaders();

	// Fetch the default compiled shaders.
	lua_getfield(L, compiledShadersTableIndex, "default");
	if (lua_istable(L, -1))
	{
		bool wasLoaded = LoadCompiledShaderVersions(L, lua_gettop(L), compiledDefaultShaders);
		if (wasLoaded)
		{
			shaderModesLoadedCount++;
		}
	}
	lua_pop(L, 1);

	// Fetch the 2.5D compiled shaders.
	lua_getfield(L, compiledShadersTableIndex, "mode25D");
	if (lua_istable(L, -1))
	{
		bool wasLoaded = LoadCompiledShaderVersions(L, lua_gettop(L), compiled25DShaders);
		if (wasLoaded)
		{
			shaderModesLoadedCount++;
		}
	}
	lua_pop(L, 1);

	// Return true if all shader modes/versions were successfully loaded from Lua.
	bool wereAllShadersLoaded = (shaderModesLoadedCount >= ShaderResource::kNumProgramMods);
	Rtt_ASSERT(wereAllShadersLoaded);
	return wereAllShadersLoaded;
}

bool
ShaderFactory::LoadCompiledShaderVersions(lua_State *L, int modeTableIndex, ShaderBinaryVersions &compiledShaders)
{
	int shadersLoadedCount = 0;

	// Validate arguments.
	if (!L || !modeTableIndex)
	{
		return false;
	}

	// Make sure that the given index references a Lua table.
	if (!lua_istable(L, modeTableIndex))
	{
		return false;
	}

	// Free the previously loaded shader binaries, if any.
	compiledShaders.FreeAllShaders();

	// Load all of the compiled shader versions.
	lua_getfield(L, modeTableIndex, "maskCount0");
	if (lua_type(L, -1) == LUA_TSTRING)
	{
		bool wasLoaded = compiledShaders.Get(Program::kMaskCount0)->CopyFromHexadecimalString(lua_tostring(L, -1));
		if (wasLoaded)
		{
			shadersLoadedCount++;
		}
	}
	lua_pop(L, 1);
	lua_getfield(L, modeTableIndex, "maskCount1");
	if (lua_type(L, -1) == LUA_TSTRING)
	{
		bool wasLoaded = compiledShaders.Get(Program::kMaskCount1)->CopyFromHexadecimalString(lua_tostring(L, -1));
		if (wasLoaded)
		{
			shadersLoadedCount++;
		}
	}
	lua_pop(L, 1);
	lua_getfield(L, modeTableIndex, "maskCount2");
	if (lua_type(L, -1) == LUA_TSTRING)
	{
		bool wasLoaded = compiledShaders.Get(Program::kMaskCount2)->CopyFromHexadecimalString(lua_tostring(L, -1));
		if (wasLoaded)
		{
			shadersLoadedCount++;
		}
	}
	lua_pop(L, 1);
	lua_getfield(L, modeTableIndex, "maskCount3");
	if (lua_type(L, -1) == LUA_TSTRING)
	{
		bool wasLoaded = compiledShaders.Get(Program::kMaskCount3)->CopyFromHexadecimalString(lua_tostring(L, -1));
		if (wasLoaded)
		{
			shadersLoadedCount++;
		}
	}
	lua_pop(L, 1);
	lua_getfield(L, modeTableIndex, "wireframe");
	if (lua_type(L, -1) == LUA_TSTRING)
	{
		bool wasLoaded = compiledShaders.Get(Program::kWireframe)->CopyFromHexadecimalString(lua_tostring(L, -1));
		if (wasLoaded)
		{
			shadersLoadedCount++;
		}
	}
	lua_pop(L, 1);

	// Return true if all shader versions were successfully loaded.
	bool wereAllShadersLoaded = (shadersLoadedCount >= Program::kNumVersions);
	Rtt_ASSERT(wereAllShadersLoaded);
	return wereAllShadersLoaded;
}

#endif


ShaderComposite *
ShaderFactory::NewShaderBuiltin( ShaderTypes::Category category, const char *name)
{
	ShaderComposite *result = NULL;

	// Lazily load built-ins
	const char *key = ShaderBuiltin::KeyForCategory( category );
	if ( key )
	{
		lua_State *L = fL;

		Rtt_LUA_STACK_GUARD(L);

		PushTable( L, key ); // push loaders ( = registry[key] )
		{
			lua_getfield( L, -1, name ); // loaders[name]
			lua_CFunction f = lua_tocfunction( L, -1 );
			if ( f )
			{
				// NOTE: DoCall will automatically pop function
				if ( Rtt_VERIFY( 0 == Corona::Lua::DoCall( L, 0, 1 ) ) )
				{
					int tableIndex = lua_gettop( L );
					
					lua_getfield( L, tableIndex, "graph" );
					{
						Rtt_LUA_STACK_GUARD( L );

						if ( ! lua_istable( L, -1 ) )
						{
							Rtt_LUA_STACK_GUARD( L );

							SharedPtr< ShaderResource > resource;
#if defined( Rtt_USE_PRECOMPILED_SHADERS )
							ShaderBinaryVersions compiledDefaultShaders( fAllocator );
							ShaderBinaryVersions compiled25DShaders( fAllocator );

							lua_getfield( L, tableIndex, "compiledShaders" );
							bool wasLoaded = LoadAllCompiledShaders(
													L, lua_gettop( L ), compiledDefaultShaders, compiled25DShaders );
							if ( wasLoaded )
							{
								resource = NewShaderResource( category, name, compiledDefaultShaders, compiled25DShaders );
							}
							lua_pop( L, 1 );

#else
							lua_getfield( L, tableIndex, "vertex" );
							const char *kernelVert = lua_tostring( L, -1 );

							lua_getfield( L, tableIndex, "fragment" );
							const char *kernelFrag = lua_tostring( L, -1 );

							resource = NewShaderResource( category, name, kernelVert, kernelFrag );
							lua_pop( L, 2 ); // pop 2 strings
#endif

							if (resource.NotNull())
							{
								lua_getfield( L, tableIndex, "isTimeDependent" );
								bool usesTime = lua_toboolean( L, -1 ) ? true : false;
								resource->SetUsesTime( usesTime );
								lua_pop( L, 1 );

								Shader *prototype = NULL;
								prototype = NewShaderPrototype( L, tableIndex, resource );
								result = (ShaderComposite*)prototype->Clone( fAllocator );
							}
						}
						else
						{
							Rtt_LUA_STACK_GUARD( L );

							int graphIndex = lua_gettop( L );
							
							result = (ShaderComposite*)NewShaderGraph( L, graphIndex );
						}
					}
					lua_pop( L, 1 ); // pop table (graph)

					lua_pop( L, 1 ); // pop table (that defines the shader)
				}
			}
			else
			{
				lua_pop( L, 1 ); // pop value (if not a function)
			}
		}
		lua_pop( L, 1 );
	}

	return result;	
}

void
ShaderFactory::AddShader( Shader *shader, const char *name )
{
	const char *categoryName = ShaderTypes::StringForCategory( shader->GetCategory() );

	// Store shader in Lua registry
	// NOTE: Lua will own the shader instance
	lua_State *L = fL;
	PushTable( L, categoryName ); // push registry[categoryName]
	{
		CoronaLuaPushUserdata( L, shader, kMetatableName ); // push shader
		lua_setfield( L, -2, name ); // registry[categoryName][name] = shader
	}
	lua_pop( L, 1 );
}

// Closure (the function part)
static int
KernelWrapper( lua_State *L )
{
	lua_pushvalue( L, lua_upvalueindex( 1 ) );
	return 1;
}

bool
ShaderFactory::DefineEffect( lua_State *L, int index )
{
	Rtt_LUA_STACK_GUARD( L );

	Rtt_ASSERT( index > 0 );

	Shader *shader = NULL;

	lua_getfield( L, index, "category" );
	ShaderTypes::Category category = ShaderTypes::CategoryForString( lua_tostring( L, -1 ) );
	lua_pop( L, 1 );

	if ( ShaderTypes::kCategoryDefault != category )
	{
		int top = lua_gettop( L );

		lua_getfield( L, index, "name" );
		const char *name = lua_tostring( L, -1 );

		lua_getfield( L, index, "group" );
		const char *group = lua_tostring( L, -1 );
		if ( ! group )
		{
			group = "custom"; // Default group name if none exists
		}

		const char *fullName = lua_pushfstring( L, "%s.%s", group, name );

		if ( NULL == FindPrototype( category, fullName )
			 && ! ShaderBuiltin::Exists( category, fullName ) )
		{
			lua_State *factoryL = fL;
			
			Lua::CopyTable(factoryL, L, index);
			int tableIndex = lua_gettop( factoryL );
			{
				// name => fullName
				lua_pushstring( factoryL, fullName );
				lua_setfield( factoryL, tableIndex, "name" );

				// Register that table
				PushTable( factoryL, ShaderBuiltin::KeyForCategory( category ) );
				{
					lua_pushvalue( factoryL, tableIndex ); // push kernel to be used in closure
					lua_pushcclosure( factoryL, & KernelWrapper, 1 ); // push KernelWrapper (pops kernel)
					lua_setfield( factoryL, -2, fullName ); // registry[name] = KernelWrapper
				}
				lua_pop( factoryL, 1 ); // pop registry table
			}
			lua_pop( factoryL, 1 ); // pop copied table
		}
		else
		{
			CORONA_LOG_ERROR( "Could not create custom effect. An effect (%s) for category (%s) already exists!",
				fullName, ShaderTypes::StringForCategory( category ) );
		}

		lua_settop( L, top );
	}
	else
	{
		CORONA_LOG_ERROR( "Could not create custom effect. The 'category' property is missing" );
	}

	return ( NULL != shader );
}

void ShaderFactory::LoadDependency(LuaMap *nodeGraph, std::string nodeKey, ShaderMap &inputNodes, bool createNode)
{
	if (nodeKey == "paint1" || nodeKey == "paint2")
	{
		std::string strNodeNameCpy = nodeKey;
		ShaderInput *input = Rtt_NEW(fAllocator, ShaderInput(0,strNodeNameCpy) );
		SharedPtr< Shader > s( input );
		ShaderProxy *shader =  Rtt_NEW(fAllocator, ShaderProxy() );
		shader->SetShader(s);
		SharedPtr< Shader > namedShader( shader );
		inputNodes[nodeKey] = namedShader;		
		return;
	}
	
	LuaMap *keyedEffectInfo = static_cast<LuaMap*>(nodeGraph->GetData(nodeKey));
	LuaString *effectName = static_cast<LuaString*>(keyedEffectInfo->GetData("effect"));
	ShaderName shaderName(effectName->GetString().c_str());
	
	LuaString *input1 = static_cast<LuaString*>(keyedEffectInfo->GetData("input1"));
	if (input1)
	{
		LoadDependency(nodeGraph, input1->GetString(), inputNodes, true);
	}
	
	LuaString *input2 = static_cast<LuaString*>(keyedEffectInfo->GetData("input2"));
	if (input2 != NULL)
	{
		LoadDependency(nodeGraph, input2->GetString(), inputNodes, true);
	}
	
	if (createNode)
	{
		ShaderComposite *shader = FindOrLoadGraph( shaderName.GetCategory(), shaderName.GetName(), true );
		SharedPtr< Shader > namedShader( shader );
		inputNodes[nodeKey] = namedShader;
	}
}

void ShaderFactory::ConnectLocalNodes(ShaderMap &inputNodes, LuaMap *nodeGraph, std::string terminalNodeKey, ShaderComposite *terminalNode)
{
	if (inputNodes.size() == 0)
		return;
	
	std::vector<std::string> keys = nodeGraph->GetKeys();
	for (int i = 0; i < (int)keys.size(); i++)
	{
		std::string curKey = keys[i];
		
		if (inputNodes.find(curKey) != inputNodes.end() || curKey == terminalNodeKey)
		{
			ShaderComposite* outputNode = terminalNode;
			if (curKey != terminalNodeKey)
			{
				SharedPtr<Shader> outputShader = inputNodes[curKey];
				outputNode = (ShaderComposite*)(&(*outputShader));
			}
			
			LuaMap *keyedEffectInfoCur = static_cast<LuaMap*>(nodeGraph->GetData(curKey));
			LuaString *input1NodeName = static_cast<LuaString*>(keyedEffectInfoCur->GetData("input1"));
			
			if ( input1NodeName )
			{
				std::string inputEffect = input1NodeName->GetString();
				SharedPtr<Shader> inputShader = inputNodes[inputEffect];
				Shader* inputNode = (Shader*)(&(*inputShader));
				outputNode->SetInput( 0, inputShader);
				
				terminalNode->SetNamedShader(inputEffect, inputNode);
			}
			
			LuaString *input2NodeName = static_cast<LuaString*>(keyedEffectInfoCur->GetData("input2"));
			if ( input2NodeName )
			{
				std::string inputEffect = input2NodeName->GetString();
				SharedPtr<Shader> inputShader = inputNodes[inputEffect];
				Shader* inputNode = (Shader*)(&(*inputShader));
				outputNode->SetInput( 1, inputShader );
				
				terminalNode->SetNamedShader(inputEffect, inputNode);
			}
		}
	}
	
	terminalNode->SetNamedShader(terminalNodeKey, terminalNode);
}

Shader*
ShaderFactory::NewShaderGraph( lua_State *L, int index)
{
	Rtt_LUA_STACK_GUARD(L);
	LuaMap nodeGraph(L,index);
	LuaMap *nodes = static_cast<LuaMap*>(nodeGraph.GetData("nodes"));
	
	ShaderMap inputNodes;
	LuaString *terminalName = static_cast<LuaString*>(nodeGraph.GetData("output"));
	std::string terminalNodeName = terminalName->GetString();

	LoadDependency(nodes, terminalNodeName, inputNodes, false);
	
	LuaMap *keyedEffectInfo = static_cast<LuaMap*>(nodes->GetData(terminalNodeName));
	LuaString *effectName = static_cast<LuaString*>(keyedEffectInfo->GetData("effect"));
	ShaderName shaderName(effectName->GetString().c_str());
	ShaderComposite *terminalNode = FindOrLoadGraph( shaderName.GetCategory(), shaderName.GetName(), true );

	ConnectLocalNodes(inputNodes, nodes, terminalNodeName, terminalNode);
	
	terminalNode->Initialize();
	
	//terminalNode->Shader::Log();
	
	return terminalNode;
}

const Shader *
ShaderFactory::FindPrototype( ShaderTypes::Category category, const char *name ) const
{
	const Shader *result = NULL;

	if ( ShaderTypes::kCategoryDefault == category )
	{
		result = fDefaultShader;
	}
	else
	{
		const char *categoryName = ShaderTypes::StringForCategory( category );

		lua_State *L = fL;
		PushTable( L, categoryName ); // push registry[categoryName]
		{
			lua_getfield( L, -1, name ); // push ud = registry[categoryName][name]
			if ( lua_isuserdata( L, -1 ) )
			{
				result = (const Shader *)CoronaLuaToUserdata( L, -1 );
			}
			else
			{
				Rtt_ASSERT( lua_isnil( L, -1 ) );
			}
			lua_pop( L, 1 ); // pop ud
		}
		lua_pop( L, 1 );
	}

	return result;
}

Shader *
ShaderFactory::FindOrLoad( const ShaderName& shaderName )
{
	return FindOrLoad( shaderName.GetCategory(), shaderName.GetName() );
}

Shader&
ShaderFactory::GetDefaultColorShader() const
{
	if ( !fDefaultColorShader )
	{
		const char *name = "color";
		
		ShaderFactory *factory = const_cast<ShaderFactory*>(this);
		
		fDefaultColorShader = factory->FindOrLoad(ShaderTypes::kCategoryFilter,name);
	}
	return *fDefaultColorShader;
}
		
ShaderComposite *
ShaderFactory::FindOrLoadGraph( ShaderTypes::Category category, const char *name, bool shouldFallback )
{
	ShaderComposite *result = NULL;

	const ShaderComposite *prototype = (ShaderComposite*)FindPrototype( category, name );
	if ( prototype )
	{
		result = (ShaderComposite*)prototype->Clone( fAllocator );
	}

	if ( ! result && name )
	{
		// Lazily instantiate built-in
		result = NewShaderBuiltin( category, name );
	}

	// Fallback to default if nothing found
	if ( ! result && shouldFallback )
	{
		Rtt_TRACE_SIM( ( "ERROR: Could not find or load shader (%s.%s). Falling back to default shader.\n", ShaderTypes::StringForCategory( category ), name ? name : "(unknown)" ) );
		result = (ShaderComposite*)fDefaultShader->Clone( fAllocator );
	}

	Rtt_WARN_SIM( result, ( "ERROR: Could not find or load shader (%s.%s).\n", ShaderTypes::StringForCategory( category ), name ? name : "(unknown)" ) );

	return result;
}

Shader *
ShaderFactory::FindOrLoad( ShaderTypes::Category category, const char *name )
{
	return FindOrLoadGraph( category, name, false );
}

void
ShaderFactory::PushList( lua_State *L, ShaderTypes::Category category ) const
{
	lua_createtable( L, 0, 0 );
	int index = lua_gettop( L );

	int base = ShaderBuiltin::Append( L, index, category ) + 1;

	const char *categoryName = ShaderTypes::StringForCategory( category );

	// Store shader in Lua registry
	// NOTE: Lua will own the shader instance
	lua_State *factoryL = fL; // ShaderFactory's Lua state is different from L!!!
	PushTable( factoryL, categoryName ); // push registry[categoryName]
	{
		int t = lua_gettop( factoryL );
		lua_pushnil( factoryL );  // first key
		while ( lua_next( factoryL, t ) != 0 )
		{
			if ( lua_type( factoryL, -2 ) == LUA_TSTRING )
			{
//				const Shader *shader =
//					(const Shader *)CoronaLuaToUserdata( L, -1 );

				const char *name = lua_tostring( factoryL, -2 );
				lua_pushstring( L, name );
				lua_rawseti( L, index, base++ );
			}

			// removes 'value'; keeps 'key' for next iteration
			lua_pop( factoryL, 1 );
		}
	}
	lua_pop( factoryL, 1 );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

