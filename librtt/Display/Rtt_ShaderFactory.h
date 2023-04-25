//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ShaderFactory_H__
#define _Rtt_ShaderFactory_H__

#include "Core/Rtt_SharedPtr.h"

#include "Display/Rtt_Shader.h"
#include "Display/Rtt_ShaderComposite.h"
#include "Display/Rtt_ShaderTypes.h"
#include "Renderer/Rtt_Geometry_Renderer.h"

// ----------------------------------------------------------------------------

struct lua_State;
struct CoronaEffectCallbacks;
struct CoronaShellTransform;
struct CoronaVertexExtension;

namespace Rtt
{

class Display;
class Program;
class ProgramHeader;
class ShaderName;
class ShaderData;
class ShaderResource;
class LuaMap;
#if defined( Rtt_USE_PRECOMPILED_SHADERS )
    class ShaderBinaryVersions;
#endif

// ----------------------------------------------------------------------------
        
class ShaderFactory
{
    public:
        typedef ShaderFactory Self;
    private:
        typedef std::map<std::string, SharedPtr< Shader> > ShaderMap;

    private:
        static int ShaderFinalizer( lua_State *L );
        static void PushTable( lua_State *L, const char *key );
        static void RegisterBuiltin( lua_State *L, ShaderTypes::Category category );

	public:
		ShaderFactory( Display& owner, const ProgramHeader& programHeader, const char * backend );
		~ShaderFactory();

    protected:
        bool Initialize();
#if defined( Rtt_USE_PRECOMPILED_SHADERS )
		Program *NewProgram(ShaderBinaryVersions &compiledShaders, ShaderResource::ProgramMod mod ) const;
		SharedPtr< ShaderResource > NewShaderResource(
				ShaderTypes::Category category,
				const char *name,
				ShaderBinaryVersions &compiledDefaultShaders,
				ShaderBinaryVersions &compiled25DShaders,
                int localStubsIndex );
#else
		Program *NewProgram(
				const char *shellVert,
				const char *shellFrag,
				const char *kernelVertDefault,
				const char *kernelFragDefault,
				ShaderResource::ProgramMod mod ) const;
		SharedPtr< ShaderResource > NewShaderResource(
				ShaderTypes::Category category,
				const char *name,
				const char *kernelVert,
				const char *kernelFrag,
                int localStubsIndex );
#endif
        Shader *NewShaderPrototype( lua_State *L, int index, const SharedPtr< ShaderResource >& resource );

    private:
        // Helper methods to instantiate Shader
        bool BindVertexDataMap( lua_State *L, int index, const SharedPtr< ShaderResource >& resource );
        bool BindUniformDataMap( lua_State *L, int index, const SharedPtr< ShaderResource >& resource );
        void BindDataType( lua_State * L, int index, const SharedPtr< ShaderResource >& resource );
        void BindDetails( lua_State * L, int index, const SharedPtr< ShaderResource >& resource );
        void BindShellTransform( lua_State * L, int index, const SharedPtr< ShaderResource >& resource );
        void BindTimeTransform( lua_State *L, int index, const SharedPtr< ShaderResource >& resource );
        void BindVertexExtension( lua_State *L, int index, const SharedPtr< ShaderResource >& resource );

        void InitializeBindings( lua_State *L, int shaderIndex, const SharedPtr< ShaderResource >& resource );
#if defined( Rtt_USE_PRECOMPILED_SHADERS )
        bool LoadAllCompiledShaders(
                    lua_State *L, int compiledShadersTableIndex,
                    ShaderBinaryVersions &compiledDefaultShaders, ShaderBinaryVersions &compiled25DShaders);
        bool LoadCompiledShaderVersions(lua_State *L, int modeTableIndex, ShaderBinaryVersions &compiledShaders);
#endif

	protected:
//		Shader *NewShader( lua_State *L, int index );
		ShaderComposite *NewShaderBuiltin( ShaderTypes::Category category, const char *name, int localStubsIndex );
		void AddShader( Shader *shader, const char *name );
		
		void LoadDependency(LuaMap *nodeGraph, std::string nodeKey, ShaderMap &inputNodes, bool createNode, int localStubsIndex );
		void ConnectLocalNodes(ShaderMap &inputNodes, LuaMap *nodeGraph, std::string terminalNodeKey, ShaderComposite *terminalNode);

    private:
        struct EffectInfo {
            ShaderTypes::Category fCategory;
            const char * fCategoryName;
            const char * fEffectName;
            bool fIsBuiltIn;
        };
    
        EffectInfo GetEffectInfo( const char * fullName );
    
        bool GatherEffectStubs( lua_State * L );
    
	public:
		bool DefineEffect( lua_State *L, int shaderIndex );
        bool UndefineEffect( lua_State *L, int nameIndex );

		Shader *NewShaderGraph( lua_State *L, int index, int localStubsIndex );
        
        bool RegisterDataType( const char * name, const CoronaEffectCallbacks & callbacks );
        bool RegisterShellTransform( const char * name, const CoronaShellTransform & transform );
        bool RegisterVertexExtension( const char * name, const CoronaVertexExtension & extension );
    
        SharedPtr<FormatExtensionList> * GetExtensionList( const char * name ) const;

        bool UnregisterDataType( const char * name );
        bool UnregisterShellTransform( const char * name );
        bool UnregisterVertexExtension( const char * name );
	
		void AddExternalInfo( lua_State * L, const char * name, const char * type );
		void RemoveExternalInfo( lua_State * L, const char * name, const char * type );
				
	protected:
		const Shader *FindPrototype( ShaderTypes::Category category, const char *name, int localStubsIndex ) const;

		ShaderComposite *FindOrLoadGraph( ShaderTypes::Category category, const char *name, bool shouldFallback, int localStubsIndex );

    public:
        Shader *FindOrLoad( const ShaderName& shaderName );
        Shader *FindOrLoad( ShaderTypes::Category category, const char *name );

    public:
        void PushList( lua_State *L, ShaderTypes::Category category ) const;

    public:
        Shader& GetDefault() const { return *fDefaultShader; }
        Shader& GetDefaultColorShader() const;

	private:
		Rtt_Allocator *fAllocator;
		Shader *fDefaultShader;
		mutable Shader *fDefaultColorShader;
		lua_State *fL;
		Display& fOwner;
		Program *fDefaultShell;
		Program *fDefaultKernel;
		ProgramHeader *fProgramHeader;
		const char *fBackend;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ShaderFactory_H__
