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
class Texture;
class TextureList;
class FormatExtensionList;
struct RenderDataState;

// ----------------------------------------------------------------------------

struct SamplerTypeDetails {
	bool IsDefault() const { return ( 0 == family ) && ( 0 == target ) && !isImage && !isArray; }
	bool Matches( const SamplerTypeDetails& rhs) const { return 0 == memcmp( this, &rhs, sizeof(*this) ); }

	U8 family : 2;
	U8 target : 3;
	U8 isImage : 1;
	U8 isArray : 1;
};

// ----------------------------------------------------------------------------

struct ExtraTextureInfo
{
	// The info is a big blob of bytes, used by shaders and paints in
	// slightly different ways. Both cases include a list of names.

	// Counts are 6-bit values, per the details that follow, and can
	// be stored in a byte. One of the leftover bits is reserved as
	// a behavior flag, e.g. "uses compression", to be interpreted
	// in the same way by both shaders and paints; the high bit is
	// intended for "local" use.

	// At the moment, counts are interspersed between names. While
	// slightly wasteful in the general case, owing to padding, it
	// does make for a simpler decoding process.

	enum {
		// Names are basically "identifiers" as in C89 or GLSL, i.e.
		// some combination of underscores, ASCII letters, and non-
		// leading digits with a terminating NUL byte, which lends
		// itself perfectly to a Base64-style, 6-bits-per-element
		// encoding. Every three bytes can thus hold four of these
		// elements, and we can eke out longer names by storing the
		// count in terms of triples instead. (For purposes of this
		// length, the terminating NUL is not included. Also, NULs
		// will be added to pad any shortfall in the final triple.)
		kMaxPackedNameCount = 64,
		kMaxPackedNameLength = kMaxPackedNameCount * 3,
	
		// Longest length of raw name that may be packed.
		kMaxNameLength = kMaxPackedNameCount * 4,
	};

	Rtt_STATIC_ASSERT( ( kMaxPackedNameLength % 3 == 0 ) && ( kMaxNameLength % 4 == 0 ) );

	static int FindNameInList( const U8* name, const U8* listOfNames, int n );
	static U32 NamesSize( const U8* listOfNames, int n );
	static int EncodeName( U8* buf, const char* name );

	// N.B. name must have a terminating NUL (when encoding) or an
	// extra slot to receive the same (when decoding).

	U8 fData[1];
};

class NamesReader {
public:
	NamesReader( const U8* stream );

	NamesReader Clone() const { return NamesReader( fStream ); }
		
	void PullNext();
	const U8* Current() const;
		
public:
	int FindCurrentNameInList( NamesReader& headOfList, int n ) const;
	bool MatchesList( const NamesReader& headOfOtherList, int n ) const;
	U32 SizeOfList( int n ) const;
	
public:
	void Decode( char* name ) const;
	int GetPullCount() const { return fPulls; } // times PullNext() has been called / number of "count" bytes
	int GetTotalBytes() const { return GetNameBytes() + fPulls; } // all counts
	int GetNameBytes() const { return fTally + fCount; } // current count plus previous results

private:
	const U8* fStream;
	int fCount;
	int fPulls;
	int fTally;
};

class LengthAccumulator {
public:
	LengthAccumulator() : fTotalTriples( 0 ), fCount( 0 ) {}

	int GetTotalBytes() const;
	int GetCount() const { return fCount; }
	void AddLength( int length );

private:
	int fTotalTriples;
	int fCount;
};

class NamesEncoder {
public:
	NamesEncoder( U8* stream, const LengthAccumulator& acc );

	bool Encode( const char* name, int length );
	void CheckTotalCount();

private:
	LengthAccumulator fRef;
	U8* fStream;
	int fPos;
};

// ----------------------------------------------------------------------------

struct TimeTransform
{
	typedef enum _Method
	{
		kNone = 0,
		kModulo,
		kPingPong,
		kSine,
		kSmoothedModulo,
		kNoise_Lissajous,
		kNoise_R2,
		kNumMethodTypes
	}
	Method;

    TimeTransform() : fMethod( kNone ), fArg1( 0 ), fArg2( 0 ), fArg3( 0 )
    {
    }

	Real Apply( Real value ) const;
    int Push( lua_State *L ) const;
    bool Matches( const TimeTransform* other ) const;
    void SetMethod( lua_State *L, int arg, const char *what, Method method );

	static const char* StringForMethod( Method method );
	static Method MethodForString( const char *name );
    static Method FindMethod( lua_State *L, int arg, const char *what );

	Method fMethod;
    Real fArg1, fArg2, fArg3;
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

        const CoronaEffectCallbacks * GetEffectCallbacks() const { return fEffectCallbacks; }
        void SetEffectCallbacks( CoronaEffectCallbacks * callbacks );
        const CoronaShellTransform * GetShellTransform() const { return fShellTransform; }
        void SetShellTransform( CoronaShellTransform * shellTransform );

        const FormatExtensionList * GetExtensionList() const { return &*fExtensionList; }
        void SetExtensionList( const SharedPtr<FormatExtensionList>& list ) { fExtensionList = list; }

        void AddEffectDetail( const char * name, const char * value );

        int GetEffectDetail( int index, CoronaEffectDetail & detail ) const;

        const TimeTransform& GetTimeTransform() const { return fTimeTransform; }
        void SetTimeTransform( const TimeTransform& transform ) { fTimeTransform = transform; }
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
        
	public:
		static void SetAddedUsesTime( bool newValue ) { sAddedUsesTime = newValue; }
		static bool GetAddedUsesTime() { return sAddedUsesTime; }

	public:
		void SetTextureInfo( const U8* info, U8 count, SamplerTypeDetails fillInfo[2] );

		static bool DetailsAgree( U32 formatBackingValue, const SamplerTypeDetails& details );

		bool HasTextureInfo() const { return fTextureInfoIsSet; }
        U32 GetExtraTextureCount() const { return fExtraTextureCount; }
        SamplerTypeDetails GetFillInfo(int index) const { return fFillTextureInfo[index]; }
        const SamplerTypeDetails* GetExtraTextureDetails() const;
        const U8* GetExtraTextureNames() const;

	public:
		bool AreFormatsConsistent( U32 fillBackingValues[], U32 extraTextureBackingValues[], U32 extraCount, const U8* paintNames, RenderDataState* renderDataState = NULL ) const;
		bool AreTexturesConsistent( const TextureList& list, const U8* paintNames, RenderDataState* renderDataState = NULL ) const;

	public:
		const Program* GetFirstBoundProgram() const;

		bool IsSyncPending() const { return fSyncPending; }
		bool IsFirstBoundMod25D() const { return fIsFirstMod25D; }
		int GetFirstBoundVersion() const { return fFirstVersion; }
		void PrepareFirstBind( const Program* program, int version );
		void SyncBinding();
        
	public:
		void SetExtensionPrelude( const char* prelude );
		const char* GetExtensionPrelude() const { return fExtensionPrelude; }
        
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
        char* fExtensionPrelude;
        const ExtraTextureInfo *fExtraTextureInfo;
        U32 fDetailsCount;
        TimeTransform fTimeTransform;
        SamplerTypeDetails fFillTextureInfo[2];
        U8 fExtraTextureCount;
        U8 fFirstVersion : 2;
        U8 fIsFirstMod25D : 1;
        U8 fAnyVersionBound : 1;
        U8 fSyncPending : 1;
        bool fUsesUniforms;
        bool fUsesTime;
        bool fTextureInfoIsSet;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_ShaderResource_H__
