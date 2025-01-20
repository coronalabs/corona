//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_Scene.h"

#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayDefaults.h"
#include "Rtt_MUpdatable.h"
#include "Display/Rtt_TextureFactory.h"
#include "Renderer/Rtt_Renderer.h"
#include "Renderer/Rtt_CPUResource.h"

#include "Rtt_LuaContext.h"
#include "Rtt_LuaUserdataProxy.h"
#include "Rtt_Matrix.h"
#include "Rtt_PlatformSurface.h"
#include "Rtt_Runtime.h"
#include "Rtt_PhysicsWorld.h"
#include "Rtt_Profiling.h"

////
//
#define GLM_FORCE_RADIANS
#define GLM_FORCE_CXX98
#include "glm/glm.hpp"
#include "glm/ext.hpp"
//
////

// ----------------------------------------------------------------------------

namespace Rtt
{

//For DEBUG only
#define Rtt_DisableOverlay 0

// ----------------------------------------------------------------------------

Scene::Scene( Rtt_Allocator* pAllocator, Display& owner)
:    fOwner( owner ),
    fFrontResourceOrphanage( Rtt_NEW( owner.GetAllocator(), PtrArray< CPUResource >( owner.GetAllocator() ) ) ),
    fBackResourceOrphanage( Rtt_NEW( owner.GetAllocator(), PtrArray< CPUResource >( owner.GetAllocator() ) ) ),
    fCurrentStage( Rtt_NEW( pAllocator, StageObject( pAllocator, * this ) ) ),
    fOffscreenStage( Rtt_NEW( pAllocator, StageObject( pAllocator, * this ) ) ),
    fOrphanage( Rtt_NEW( pAllocator, StageObject( pAllocator, * this ) ) ),
    fSnapshotOrphanage( Rtt_NEW( pAllocator, StageObject( pAllocator, * this ) ) ),
    fOverlay( Rtt_NEW( pAllocator, StageObject( pAllocator, * this ) ) ),
    fProxyOrphanage( owner.GetAllocator() ),
    fIsValid( false ),
    fCounter( 0 ),
    fActiveUpdatable()
{
    fOffscreenStage->SetRenderedOffScreen( true );
 
    // Set restrictions on root display objects
    bool isRestricted = owner.IsRestricted();
    fCurrentStage->SetRestricted( isRestricted );
    fOffscreenStage->SetRestricted( isRestricted );
    fOrphanage->SetRestricted( isRestricted );
    fSnapshotOrphanage->SetRestricted( isRestricted );

#ifdef Rtt_AUTHORING_SIMULATOR
    fOverlay->SetRestricted( isRestricted );
#endif
}

Scene::~Scene()
{
    Rtt_DELETE( fOverlay );
    fOverlay = NULL;

    Rtt_DELETE( fSnapshotOrphanage );
    fSnapshotOrphanage = NULL;

    Rtt_DELETE( fOrphanage );
    fOrphanage = NULL;
    
    // this order is differen than in the constructor because sometimes (TextureResourceCanvas) we want to put
    // things in offscreen stage for deleting. So we want to have it valid when deleting stage.
    Rtt_DELETE( fCurrentStage );
    fCurrentStage = NULL;
    
    Rtt_DELETE( fOffscreenStage );
    fOffscreenStage = NULL;

    Rtt_DELETE( fBackResourceOrphanage );
    fBackResourceOrphanage = NULL;

    Rtt_DELETE( fFrontResourceOrphanage );
    fFrontResourceOrphanage = NULL;
}

void
Scene::Collect()
{
    ++fCounter;

    Rtt_STATIC_ASSERT( sizeof( fCounter ) == sizeof( U8 ) );

    // Every 3 frames, free CPUResources
    if ( ( (0x3) & fCounter ) == 0 )
    {
        // CPUResources are queued on the front queue, so to defer deletion
        // by (at least) 1 frame, empty the back queue. Then, swap the queues.
        fBackResourceOrphanage->Empty();
        Swap( fFrontResourceOrphanage, fBackResourceOrphanage );
    }

    // Every 32 frames, we collect
    if ( ( (0x1F) & fCounter ) == 0 )
    {
        OnCollectUnreachables();
    }
}

void
Scene::ForceCollect()
{
    fBackResourceOrphanage->Empty();
    fFrontResourceOrphanage->Empty();
    OnCollectUnreachables();
}

void
Scene::OnCollectUnreachables()
{
    lua_State *L = fOwner.GetRuntime().VMContext().L();

    // Delete GPU resources.
    if ( fOrphanage )
    {
        GroupObject::CollectUnreachables( L, * this, * fOrphanage );
    }

    // Release native references to Lua user data.
    for ( int i = 0, iMax = fProxyOrphanage.Length(); i < iMax; i++ )
    {
        fProxyOrphanage[i]->ReleaseRef( L );
    }
    fProxyOrphanage.Clear();
}

void
Scene::QueueRelease( CPUResource *resource )
{
    fFrontResourceOrphanage->Append( resource );
}

void
Scene::QueueRelease( LuaUserdataProxy *proxy )
{
    fProxyOrphanage.Append( proxy );
}

bool
Scene::IsValid() const
{
    U8 drawMode = fOwner.GetDrawMode();

    // If any MUpdatable is active, then the scene SHOULDN'T be considered
    // valid. This will trigger a call to DisplayObject::Prepare() of all
    // fActiveUpdatable.
    return ( fIsValid &&
                ( Display::kForceRenderDrawMode != drawMode ) &&
                fActiveUpdatable.empty() );
}

void
Scene::Invalidate()
{
    fIsValid = false;
}

void
Scene::Clear( Renderer& renderer )
{
    DisplayDefaults& defaults = fOwner.GetDefaults();
    ColorUnion c;
    c.pixel = defaults.GetClearColor();

    Renderer::ExtraClearOptions extra;
    
    extra.clearDepth = defaults.GetEnableDepthInScene();
    extra.clearStencil = defaults.GetEnableStencilInScene();
    extra.depthClearValue = defaults.GetSceneDepthClearValue();
    extra.stencilClearValue = defaults.GetSceneStencilClearValue();

    Real inv255 = 1.f / 255.f;
    renderer.Clear( c.rgba.r * inv255, c.rgba.g * inv255, c.rgba.b * inv255, c.rgba.a * inv255, &extra );
    renderer.BeginDrawing();
}

#define ADD_ENTRY( what ) if ( profiling ) PROFILING_ADD( *profiling, what )

void
Scene::Render( Renderer& renderer, PlatformSurface& rTarget, ProfilingEntryRAII* profiling )
{
    Rtt_ASSERT( fCurrentStage );

    U8 drawMode = fOwner.GetDrawMode();

    if ( ! IsValid() )
    {
        const Rtt::Real kMillisecondsPerSecond = 1000.0f;
        Rtt_AbsoluteTime elapsedTime = fOwner.GetElapsedTime();
        Rtt::Real totalTime = Rtt_AbsoluteToMilliseconds( elapsedTime ) / kMillisecondsPerSecond;
        Rtt::Real deltaTime = Rtt_AbsoluteToMilliseconds( elapsedTime - fOwner.GetPreviousTime() ) / kMillisecondsPerSecond;

        renderer.BeginFrame( totalTime, deltaTime, fOwner.GetDefaults().GetTimeTransform(), fOwner.GetSx(), fOwner.GetSy() );
        
		ADD_ENTRY( "Scene: Begin Render" );
		
        fOwner.GetTextureFactory().Preload( renderer );

		ADD_ENTRY( "Scene: Preload" );
		
        fOwner.GetTextureFactory().UpdateTextures(renderer);

		ADD_ENTRY( "Scene: UpdateTextures" );
		
        // Set antialiasing once:
        // NOTE: Assumes Runtime::ReadConfig() has already have been called.
        bool isMultisampleEnabled = fOwner.IsAntialiased();
        renderer.SetMultisampleEnabled( isMultisampleEnabled );

        renderer.SetViewport( 0, 0, fOwner.WindowWidth(), fOwner.WindowHeight() );
        
        glm::mat4 viewMatrix;
        glm::mat4 projMatrix;
        fOwner.GetViewProjectionMatrix(viewMatrix, projMatrix);
        renderer.SetFrustum( glm::value_ptr(viewMatrix), glm::value_ptr(projMatrix) );
        
		ADD_ENTRY( "Scene: Setup" );
		
        Clear( renderer );

        Matrix identity;
        StageObject *canvas = fCurrentStage;

		ENABLE_SUMMED_TIMING( true );

        canvas->UpdateTransform( identity );
        canvas->Prepare( fOwner );

		ADD_ENTRY( "Scene: Issue Clear Command" );
		
        canvas->WillDraw( renderer );
        {
            // In PhysicsDebugDrawMode, do NOT draw display objects
            if ( drawMode < Display::kPhysicsDebugDrawMode )
            {
                fOwner.SetWireframe( Display::kWireframeDrawMode == drawMode );
                canvas->Draw( renderer );
                fOwner.SetWireframe( false );
            }

            // Only draw physics debug info for physics-based draw modes
            if ( drawMode >= Display::kPhysicsHybridDrawMode )
            {
                fOwner.GetRuntime().GetPhysicsWorld().DebugDraw( renderer );
            }
        }
        canvas->DidDraw( renderer );

		ENABLE_SUMMED_TIMING( false );

        // Draw overlay *last* because it goes above everything else.
        // This draws the status bar of the device.

#if Rtt_DisableOverlay == 0
        RenderOverlay( fOwner, renderer, identity );
#endif

        renderer.EndFrame();

        // When shader code depends on time, then frame is time-dependent.
        // So only set valid when frame is *in*dependent of time.
        if ( ! renderer.IsFrameTimeDependent() )
        {
            fIsValid = true;
        }
        
		ADD_ENTRY( "Scene: Issue Draw Commands" );
		
        renderer.Swap(); // Swap back and front command buffers
		
		ADD_ENTRY( "Scene: Swap" );
		
        renderer.Render(); // Render front command buffer
        
//        renderer.GetFrameStatistics().Log();
        
		ADD_ENTRY( "Scene: Process Render Commands" );

        rTarget.Flush();

		ADD_ENTRY( "Scene: Flush" );
    }
    
    // This needs to be done at the sync point (DMZ)
    Collect();
	
	ADD_ENTRY( "Scene: Collect" );
}

void
Scene::Render( Renderer& renderer, PlatformSurface& rTarget, DisplayObject& object )
{
    const StageObject* stage = object.GetStage();
    if ( Rtt_VERIFY( stage == fCurrentStage ) )
    {
        Clear( renderer );

        // This function is used to render a specific object in a scene.
        // This is used in the context of rendering to a texture.
        // In this context, we DON'T want to take the parent (group)
        // transforms into account.
        //
        // It's necessary to call UpdateTransform() before Prepare(),
        // so we'll pass identity.
        Matrix identity;
        object.UpdateTransform( identity );
        object.Prepare( fOwner );

        object.WillDraw( renderer );
        object.Draw( renderer );
        object.DidDraw( renderer );

        rTarget.Flush();

        fIsValid = true;
    }
}

#undef ADD_ENTRY

void
Scene::RenderOverlay( Display& display, Renderer& renderer, const Matrix& srcToDstSpace )
{
    GroupObject* overlay = fOverlay;
    if ( overlay )
    {
        overlay->UpdateTransform( srcToDstSpace );
        overlay->Prepare( display );
        overlay->Draw( renderer );
    }
}

StageObject&
Scene::Overlay()
{
    Rtt_ASSERT( fOverlay );

    return * fOverlay;
}

StageObject*
Scene::PushStage()
{
    Rtt_Allocator* pAllocator = fOwner.GetRuntime().Allocator();
    StageObject* oldHead = fCurrentStage;
    StageObject* newHead = Rtt_NEW( pAllocator, StageObject( pAllocator, * this ) );
    fCurrentStage = newHead;
    newHead->SetNext( oldHead );

    fOwner.GetRuntime().VMContext().UpdateStage( * newHead );

    return newHead;
}

void
Scene::PopStage()
{
    StageObject* oldHead = fCurrentStage;
    StageObject* newHead = oldHead->GetNext();
    fCurrentStage = newHead;
    oldHead->SetNext( NULL );
    Rtt_DELETE( oldHead );
}

void Scene::QueueUpdateOfUpdatables()
{
    for( std::set< MUpdatable * >::iterator u = fActiveUpdatable.begin();
            u != fActiveUpdatable.end();
            ++u )
    {
        (*u)->QueueUpdate();
    }
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
