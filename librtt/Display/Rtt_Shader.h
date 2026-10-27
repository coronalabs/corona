//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Shader_H__
#define _Rtt_Shader_H__

#include "Core/Rtt_SharedPtr.h"
#include "Display/Rtt_ShaderTypes.h"
#include "Display/Rtt_ShaderResource.h"
#include "Renderer/Rtt_Texture.h"

#include <string>

// ----------------------------------------------------------------------------

struct lua_State;
struct CoronaEffectCallbacks;
struct CoronaShaderDrawParams;

namespace Rtt
{

struct TextureInfo
{
    int fWidth;
    int fHeight;
    Texture::Format fFormat;
    Texture::Filter fFilter;
    Texture::Wrap fWrap;
};

class FrameBufferObject;
class Display;
class Paint;
struct RenderData;
struct GeometryWriter;
class Renderer;
class ShaderData;
class ShaderResource;
class Texture;
class Geometry;

// ----------------------------------------------------------------------------

// This is mutable state related to RenderData that might possibly
// be resolved via Renderer::Insert(), as an inout argument.
struct RenderDataState {
	RenderDataState() : fState( 0 ) {}

	enum SyncState {
		kUnsynced, // not yet able to check for consistency
		kSyncConsistent, // sync attempt made and successful
		kSyncInconsistent, // sync attempt failed
	};
	
	enum {
		kSyncBits = 2,
		kOccupancyBits = 30,

		kSyncShift = 0,
		kOccupancyShift = kSyncBits,

		kSyncMask = ( 1 << kSyncBits ) - 1,
		kOccupancyMask = ( 1 << kOccupancyBits ) - 1,

		kSyncWipeMask = ~( kSyncMask << kSyncShift ),
		kOccupancyWipeMask = ~( kOccupancyMask << kOccupancyShift )
	};

	Rtt_STATIC_ASSERT( kOccupancyBits + kSyncBits <= sizeof(int) * 8 );

	#define GET_BITS( NAME, TYPE ) (TYPE)( ( fState >> k##NAME##Shift ) & k##NAME##Mask )
	#define SET_BITS( NAME, ARG ) fState = ( fState & ~( k##NAME##Mask << k##NAME##Shift ) ) | ( ( ARG & k##NAME##Mask ) << k##NAME##Shift )
	
	void SetSyncState( SyncState state ) { SET_BITS( Sync, state ); }
	SyncState GetSyncState() const { return GET_BITS( Sync, SyncState ); }

	void SetOccupancy( U32 occ ) { SET_BITS( Occupancy, occ ); }
	U32 GetOccupancy() const { return GET_BITS( Occupancy, U32 ); }
	
	#undef GET_BITS
	#undef SET_BITS
	
	int fState;
};

// ----------------------------------------------------------------------------

// Shader instances are per-paint
// Each shader:
// * has a weak reference to a shared ShaderResource
// * owns a ShaderData instance which stores the params for the ShaderResource's program
class Shader
{
    public:
        typedef Shader Self;

    public:
        Shader( Rtt_Allocator *allocator, const SharedPtr< ShaderResource >& resource, ShaderData *data );
        virtual ~Shader();
        
    protected:
        Shader();
        
        virtual void Initialize(){}
        
    public:
        virtual Shader *Clone( Rtt_Allocator *allocator ) const;
                
        virtual void Prepare( RenderData& objectData, int w, int h, ShaderResource::ProgramMod mod );

        virtual void Draw( Renderer& renderer, const RenderData& objectData, const GeometryWriter* writers = NULL, U32 n = 1 ) const;
        virtual void Log(std::string preprend, bool last);
        virtual void Log();

    public:    //Proxy uses these, they should be treated as protected
        virtual void UpdatePaint( RenderData& data ) const;
        virtual void UpdateCache( const TextureInfo& textureInfo, const RenderData& objectData );

        virtual Texture *GetTexture() const;
        virtual void RenderToTexture( Renderer& renderer, Geometry& cache ) const;
        virtual void SetTextureBounds( const TextureInfo& textureInfo);
        
    public:
        virtual void PushProxy( lua_State *L ) const;
        virtual void DetachProxy(); // Called by Adapter's WillFinalize()???

    public:
        const ShaderData *GetData() const { return fData; }
        ShaderData *GetData() { return fData; }
        ShaderTypes::Category GetCategory() const { return fCategory; }

        // TODO: Rename to observer???
        Paint *GetPaint() const; //{ return fOwner; }
        void SetPaint( Paint *newValue ) { fOwner = newValue; }

        virtual bool UsesUniforms() const;
        virtual bool HasChildren(){return false;}
        virtual bool IsTerminal(Shader *shader) const;
    
        //Shaders need to know about the root node so that
        //changes to their shader data can invalidate the corresponding
        //paint object
        void SetRoot( const Shader *root ) { fRoot = root; }
        bool IsOutermostTerminal() const { return NULL != fOwner; }

        class DrawState {
        public:
            DrawState( const CoronaEffectCallbacks * callbacks, bool & drawing );
            ~DrawState();

        public:
            const CoronaShaderDrawParams* fParams;

        private:
            bool & fDrawing;
            bool fWasDrawing;
        };

        bool DoAnyBeforeDrawAndThenOriginal( const DrawState & state, Renderer & renderer, const RenderData & objectData ) const;
        void DoAnyAfterDraw( const DrawState & state, Renderer & renderer, const RenderData & objectData ) const;

    public:
        bool IsCompatible( const Geometry* geometry ) const;
		bool IsPaintConsistent( const Paint* paint ) const;
		bool CanCheckConsistency() const;
    
    public:
		RenderDataState& GetRenderDataState() const { return fRenderDataState; }
    
    protected:
        SharedPtr< ShaderResource > fResource;
        Rtt_Allocator *fAllocator;
        mutable ShaderData *fData;
        ShaderTypes::Category fCategory;
        Paint *fOwner; // weak ptr
        FrameBufferObject *fFBO;
        Texture *fTexture;
        const Shader *fRoot; // Weak reference
        mutable RenderDataState fRenderDataState; // state that may be modified by Renderer::Insert()
        
        
        // Cache for a shader's output
        mutable RenderData *fRenderData;
        mutable bool fOutputReady;
        mutable bool fDirty;
        mutable bool fIsDrawing;

    // TODO: Figure out better alternative
    friend class ShaderComposite;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Shader_H__
