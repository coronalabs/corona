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
        bool IsCompatible( const Geometry* geometry );
    
    protected:
        SharedPtr< ShaderResource > fResource;
        Rtt_Allocator *fAllocator;
        mutable ShaderData *fData;
        ShaderTypes::Category fCategory;
        Paint *fOwner; // weak ptr
        FrameBufferObject *fFBO;
        Texture *fTexture;
        const Shader *fRoot; // Weak reference
        
        
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
