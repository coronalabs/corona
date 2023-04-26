//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_ShaderResource_H__
#define _Rtt_ShaderResource_H__

#include <map>
#include <string>

#include "Core/Rtt_SharedPtr.h"
#include "Display/Rtt_ShaderTypes.h"
#include "Renderer/Rtt_Uniform.h"

#include <vector>

// ----------------------------------------------------------------------------

struct lua_State;
struct CoronaEffectCallbacks;
struct CoronaEffectDetail;
struct CoronaShellTransform;

namespace Rtt
{

class Program;
class ShaderData;
class FormatExtensionList;

struct TimeTransform
{
    typedef void (*Func)( Real *time, Real arg1, Real arg2, Real arg3 );

    TimeTransform() : func( NULL ), arg1( 0 ), arg2( 0 ), arg3( 0 ), timestamp( ~0 )
    {
    }

    bool Apply( Uniform *time, Real *old, U32 now );
    int Push( lua_State *L ) const;
    void SetDefault();
    void SetFunc( lua_State *L, int arg, const char *what, const char *fname );

    static bool Matches( const TimeTransform *xform1, const TimeTransform *xform2 );
    static const char* FindFunc( lua_State *L, int arg, const char *what );

    Func func;
    Real arg1, arg2, arg3, cached;
    U32 timestamp;
};

// ----------------------------------------------------------------------------

class ShaderResource
{
    public:

        typedef enum ProgramMod
        {
            kDefault    = 0,
            k25D        = 1,
            kNumProgramMods,
        }
        ProgramMod;
    
        typedef std::map< std::string, int > VertexDataMap;

        struct UniformData
        {
            int index;
            Uniform::DataType dataType;
        };
        typedef std::map< std::string, UniformData > UniformDataMap;

    public:
        // Shader takes ownership of the program
        ShaderResource( Program *program, ShaderTypes::Category category );
        ShaderResource( Program *program, ShaderTypes::Category category, const char *name );
        
    public:
        ~ShaderResource();

    public:
        ShaderTypes::Category GetCategory() const { return fCategory; }
        const std::string& GetName() const { return fName; }
        const char *GetTag( int index ) const { return NULL; }
        int GetNumTags() const { return 0; }

    public:
        bool UsesUniforms() const { return fUsesUniforms; }
        void SetUsesUniforms( bool newValue ) { fUsesUniforms = newValue; }

        bool UsesTime() const { return fUsesTime; }
        void SetUsesTime( bool newValue ) { fUsesTime = newValue; }
    
        const CoronaEffectCallbacks * GetEffectCallbacks() const { return fEffectCallbacks; }
        void SetEffectCallbacks( CoronaEffectCallbacks * callbacks );
        const CoronaShellTransform * GetShellTransform() const { return fShellTransform; }
        void SetShellTransform( CoronaShellTransform * shellTransform );

        const FormatExtensionList * GetExtensionList() const { return &*fExtensionList; }
        void SetExtensionList( const SharedPtr<FormatExtensionList>& list ) { fExtensionList = list; }

        void AddEffectDetail( const char * name, const char * value );

        int GetEffectDetail( int index, CoronaEffectDetail & detail ) const;

        TimeTransform *GetTimeTransform() const { return fTimeTransform; }
        void SetTimeTransform( TimeTransform *transform ) { fTimeTransform = transform; }
    public:
        // Shader either stores params on per-vertex basis or in uniforms.
        // Batching most likely breaks as soon as you use uniforms,
        // so params are either per-vertex OR uniforms --- never both.
        // The mapping between the (Lua API) property name and the internal
        // location in per-vertex/uniform data is stored by the maps.
        int GetDataIndex( const char *key ) const;
        UniformData GetUniformData( const char *key ) const;
        
        /*
        const VertexDataMap& GetVertexDataMap() const { return fVertexDataMap; }
        const UniformDataMap& GetUniformDataMap() const { return fUniformDataMap; }
        */

    //protected:
        VertexDataMap& GetVertexDataMap() { return fVertexDataMap; }
        UniformDataMap& GetUniformDataMap() { return fUniformDataMap; }

    public:
        // A filter's default effect param values are stored here.
        ShaderData *GetDefaultData() const { return fDefaultData; }
        void SetDefaultData( ShaderData *defaultData );
        
    public:
        void SetProgramMod(ProgramMod mod, Program *program);
        Program *GetProgramMod(ProgramMod mod) const;
        
    private:
        void Init(Program *defaultProgram);

    private:
        Program *fPrograms[kNumProgramMods];
        
        ShaderTypes::Category fCategory;
        std::string fName;
        VertexDataMap fVertexDataMap;
        UniformDataMap fUniformDataMap;
        ShaderData *fDefaultData;
        CoronaEffectCallbacks *fEffectCallbacks;
        CoronaShellTransform *fShellTransform;
        SharedPtr<FormatExtensionList> fExtensionList;
        std::vector< std::string > fDetailNames;
        std::vector< std::string > fDetailValues;
        U32 fDetailsCount;
        TimeTransform *fTimeTransform;
        bool fUsesUniforms;
        bool fUsesTime;

};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ShaderResource_H__
