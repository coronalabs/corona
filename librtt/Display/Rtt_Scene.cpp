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
:	fOwner( owner ),
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
	
	Real inv255 = 1.f / 255.f;
	renderer.Clear( c.rgba.r * inv255, c.rgba.g * inv255, c.rgba.b * inv255, c.rgba.a * inv255 );
}
void
Scene::Render( Renderer& renderer, PlatformSurface& rTarget )
{
	Rtt_ASSERT( fCurrentStage );

	U8 drawMode = fOwner.GetDrawMode();

	if ( ! IsValid() )
	{
		const Rtt::Real kMillisecondsPerSecond = 1000.0f;
		Rtt_AbsoluteTime elapsedTime = fOwner.GetElapsedTime();
		Rtt::Real totalTime = Rtt_AbsoluteToMilliseconds( elapsedTime ) / kMillisecondsPerSecond;
		Rtt::Real deltaTime = Rtt_AbsoluteToMilliseconds( elapsedTime - fOwner.GetPreviousTime() ) / kMillisecondsPerSecond;

		renderer.BeginFrame( totalTime, deltaTime, fOwner.GetSx(), fOwner.GetSy() );
		
		fOwner.GetTextureFactory().Preload( renderer );
		fOwner.GetTextureFactory().UpdateTextures(renderer);

		// Set antialiasing once:
		// NOTE: Assumes Runtime::ReadConfig() has already have been called.
		bool isMultisampleEnabled = fOwner.IsAntialiased();
		renderer.SetMultisampleEnabled( isMultisampleEnabled );

		renderer.SetViewport( 0, 0, fOwner.WindowWidth(), fOwner.WindowHeight() );
		
		glm::mat4 viewMatrix;
		glm::mat4 projMatrix;
		fOwner.GetViewProjectionMatrix(viewMatrix, projMatrix);
		renderer.SetFrustum( glm::value_ptr(viewMatrix), glm::value_ptr(projMatrix) );
		
		Clear( renderer );

		Matrix identity;
		StageObject *canvas = fCurrentStage;
		canvas->UpdateTransform( identity );
		canvas->Prepare( fOwner );

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
		
		renderer.Swap(); // Swap back and front command buffers
		renderer.Render(); // Render front command buffer
		
//		renderer.GetFrameStatistics().Log();
		
		rTarget.Flush();
	}
	
	// This needs to be done at the sync point (DMZ)
	Collect();
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
