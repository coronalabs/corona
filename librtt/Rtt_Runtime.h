//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Runtime_H__
#define _Rtt_Runtime_H__

#include "Core/Rtt_Time.h"
#include "Display/Rtt_MDisplayDelegate.h"
#include "Rtt_DeviceOrientation.h"
#include "Rtt_MCallback.h"
#include "Rtt_MCriticalSection.h"
#include "Rtt_MPlatform.h"
#include "Rtt_Resource.h"

#if defined(Rtt_AUTHORING_SIMULATOR)
#include <atomic>
#include <thread>
#endif

// TODO: Move VirtualEvent to separate header and then move SystemEvent into Runtime.h
// Then, we can remove Event.h from the include list here:
#include "Rtt_Event.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

class MEvent;
class MPlatform;
class Archive;
class Display;
class DisplayObject;
class LuaContext;
class MRuntimeDelegate;
class RenderingStream;
class PhysicsWorld;
class BitmapPaint;
class PlatformExitCallback;
class PlatformSurface;
class PlatformTimer;
class Scheduler;

// ----------------------------------------------------------------------------

class Runtime : public MCallback,
				public MCachedResourceLibrary,
				public MCriticalSection,
				public MDisplayDelegate
{
	Rtt_CLASS_NO_COPIES( Runtime )

	public:
		typedef enum _Properties
		{
			kIsOrientationLocked      = 0x1,
			kIsDebuggerConnected      = 0x2,
			kIsApplicationLoaded      = 0x4,
			kIsApplicationExecuting   = 0x8,
			kIsUsingCustomCode        = 0x10,
			kUseExitOnErrorHandler    = 0x20,
			kShowRuntimeErrors        = 0x40,
			kIsLuaParserAvailable     = 0x80,
			kIsCustomEffectsAvailable = 0x100,
			kDeferUpdate              = 0x200,
			kRenderAsync              = 0x400,
			kIsApplicationNotArchived = 0x800,
			kShouldVerifyLicense      = 0x1000,
			kIsSimulatorExtension     = 0x2000,
			kShowRuntimeErrorsSet     = 0x4000,
		}
		Properties;

		enum
		{
			// TODO: Remove prefix "Is" to improve clarity
			// TODO: Add suffix "MaskSet" to improve clarity
			//
			// NOTE: Do not use these in IsProperty b/c it violates the semantics, e.g.
			//   IsProperty( kIsCoronaKit ) == IsProperty( kIsLuaParserAvailable )
			kIsEnterpriseFeature =
				( kIsLuaParserAvailable
				  | kIsCustomEffectsAvailable ),
			kIsCoronaKit =
				( kIsLuaParserAvailable
				  | kShouldVerifyLicense ),
			kEmscriptenMaskSet =
				( kIsLuaParserAvailable ),
			kLinuxMaskSet =
				( kIsLuaParserAvailable ),
		};

	public:
		typedef enum _LaunchOptions
		{
			// Mask
			kConnectToDebugger = 0x1,
			kLaunchDeviceShell = 0x2,
			kDisableAudio = 0x4, // Needed for new welcome window because OpenAL based system doesn't handle multiple instances
			kUnlockFeatures = 0x8, // NOTE: see PushConfig

			// Convenience defaults
			kDeviceLaunchOption = kLaunchDeviceShell,
			kSimulatorLaunchOption = kLaunchDeviceShell,
			kCoronaViewOption = (  kLaunchDeviceShell | kDisableAudio | kUnlockFeatures ),
			kHTML5LaunchOption = ( kLaunchDeviceShell ),
			kWebPluginLaunchOption = 0, // TODO: Remove???
			kCoronaCardsOption = ( kUnlockFeatures ),
			kLinuxLaunchOption = ( kLaunchDeviceShell ),

#ifdef Rtt_AUTHORING_SIMULATOR
			kDefaultLaunchOption = kSimulatorLaunchOption,
#else
			kDefaultLaunchOption = kDeviceLaunchOption,
#endif
		}
		LaunchOptions;

		typedef enum _SuspendOverrides
		{
			kSuspendAll = 0x0,
			kBackgroundAudio = 0x1,
			kBackgroundLocation = 0x2,
			kBackgroundVoIP = 0x8,
//			kBackgroundLowPriorityTimer = 0x10,
//			kBackgroundHighPriorityTimer = 0x20,
			kSuspendNone = 0xFFFFFFFF
		}
		SuspendOverrides;
		
		// TODO: Rename to LoadResult (to match LoadParameters below)
		typedef enum _LoadApplicationReturnCodes
		{
			kSuccess		= 0,
			kGeneralFail	= 1,
			kSecurityIssue	= 2,
		}
		LoadApplicationReturnCodes;
	
	public:
		Runtime( const MPlatform& platform, MCallback *viewCallback = NULL );
		virtual ~Runtime();

	public:
		Rtt_INLINE Rtt_Allocator* Allocator() { return & fAllocator; }
		Rtt_INLINE Rtt_Allocator* Allocator() const { return & fAllocator; }
		Rtt_INLINE bool IsDisplayValid() const { return NULL != fDisplay; }
		Rtt_INLINE Display& GetDisplay() { return * fDisplay; }
		Rtt_INLINE const Display& GetDisplay() const { return * fDisplay; }
		Rtt_INLINE Scheduler& GetScheduler() const { return * fScheduler; }
		Rtt_INLINE const MPlatform& Platform() const { return fPlatform; }

		Rtt_INLINE bool IsVMContextValid() const { return NULL != fVMContext; }
		Rtt_INLINE const LuaContext& VMContext() const { return * fVMContext; }
		Rtt_INLINE LuaContext& VMContext() { return * fVMContext; }

		Rtt_INLINE Archive* GetArchive() { return fArchive; }

		PlatformTimer* GetTimer() { return fTimer; }

	public:
		void SetDelegate( const MRuntimeDelegate *newValue ) { fDelegate = newValue; }
		const MRuntimeDelegate *GetDelegate() const { return fDelegate; }

	public:
		S32 ViewableContentWidth() const;
		S32 ViewableContentHeight() const;
		S32 WindowWidth() const;
		S32 WindowHeight() const;

		//const AutoPtr< GPUBitmapTexture >& CreateOrFindTextureWithFilename( const char* filename );

	protected:
		void InitializeArchive( const char *filePath );
		bool VerifyApplication( const char *filePath );

#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
		void InitializeFuse( lua_State *L, int index );
#endif
		static int InitializeMetadataShim( lua_State *L );
		void InitializeMetadata( lua_State *L, int index );

		bool PushConfig( lua_State *L, bool shouldRestrictLibs ); // loads config.lua and pushes application.content to top of stack
		void ReadConfig( lua_State *L ); // call before fDisplay is instantiated
		void PopAndClearConfig( lua_State *L );

	protected:
		void AddDownloadablePlugin(
				lua_State *L, const char *pluginName, const char *publisherId,
				int downloadablePluginsIndex, bool isSupportedOnThisPlatform, const char *pluginEntryJSON);

	public:
		void FindDownloadablePlugins( const char *simPlatformName );
		void PushDownloadablePlugins( lua_State *L );
		bool RequiresDownloadablePlugins() const;
		const char *GetSimulatorPlaformName() const { return fSimulatorPlatformName; }

	public:
		// Pushes Lua-table for launch args on the stack.
		int PushLaunchArgs( bool createIfNotExists );
		void ClearLaunchArgs();

	public:
		struct LoadParameters
		{
			LoadParameters();
			void UpdateConfig( lua_State *L, int configIndex ) const;

			U32 launchOptions;
			DeviceOrientation::Type orientation;
			S32 contentWidth;
			S32 contentHeight;
		};

		// Inits VM and then loads script file.
		LoadApplicationReturnCodes LoadApplication( const LoadParameters& parameters );

		// TODO: Remove this in favor of the LoadParameters version
		// Inits VM and then loads script file.
		LoadApplicationReturnCodes LoadApplication( U32 launchOptions, DeviceOrientation::Type orientation );

		void SetContentOrientation( DeviceOrientation::Type newOrientation );

		void WindowDidRotate( DeviceOrientation::Type newOrientation, bool isNewOrientationSupported );

		// Call when window size changes so viewport of GPUStream can be updated.
		// Implicitly calls UpdateContentScale()
		void WindowSizeChanged();

		// Call when the gl context was destroyed and recreated, e.g. when device wakes from sleep
		void RestartRenderer();
		void RestartRenderer( DeviceOrientation::Type orientation );

	public:
		// Inits renderer and then begins timer
		void BeginRunLoop();

	protected:
		void OnSystemEvent( SystemEvent::Type t );
		void OnInternalSystemEvent( SystemEvent::Type t );

	public:
		// Pauses run loop. If already suspended, then no-op.
		void Suspend(bool sendApplicationEvents = true);

		// Resumes run loop. If already running, then no-op.
		void Resume(bool sendApplicationEvents = true);

		bool IsSuspended() const { return fIsSuspended > 0; }

	private:
		// Internal helper methods to help break up Corona into separate subsystems that can be backgrounded/suspended independently.
		void CoronaAllSuspend();
		void CoronaCoreSuspend();
		void CoronaResumeAll();
		void CoronaCoreResume();
		void CoronaInvokeSystemSuspendEvent();
		void CoronaInvokeSystemResumeEvent();

	public:
		// Broadcast event dispatch, i.e. to Runtime:dispatchEvent()
		void DispatchEvent( const MEvent& e );
//		void DispatchEvent( const MEvent& e, const char *errorMsg, S32 errorCode );

		// Desired fps and interval
		U8 GetFPS() const { return fFPS; }
		float GetFrameInterval() const { return 1.f / ((float)fFPS); }

		U32 GetFrame() const { return fFrame; }

		// Number of ms since app launch
		double GetElapsedMS() const;
		Rtt_AbsoluteTime GetElapsedTime() const;

		void Collect();

		Rtt_INLINE bool IsProperty( U16 mask ) const { return (fProperties & mask) != 0; }
		Rtt_INLINE void ToggleProperty( U16 mask ) { fProperties ^= mask; }
		void SetProperty( U16 mask, bool value );

		bool IsSuspendOverrideProperty( U32 mask ) const { return (fSuspendOverrideProperties & mask) != 0; }
		void ToggleSuspendOverrideProperty( U32 mask ) { fSuspendOverrideProperties ^= mask; }
		void SetSuspendOverrideProperty( U32 mask, bool value );
	
		bool IsCoronaKit() const { return kIsCoronaKit == ( fProperties & kIsCoronaKit ); }
		
		void SetShowingTrialMessage( bool showMsg ) const;
		bool IsShowingTrialMessage();
	
	public:
		PlatformExitCallback* GetExitCallback();

	public:
		void UnloadResources();
		void ReloadResources();

	public:
		// MCachedResourceLibrary
		virtual CachedResource* LookupResource( const char key[] );
		virtual void SetResource( CachedResource* resource, const char key[] );
		virtual Rtt_Allocator* GetAllocator() const;
		virtual void MarkRecentlyUsed( CachedResource& resource );

	public:
		void GetImageSuffix( String& outSuffix );
		bool GetImageFilename( const char *filename, MPlatform::Directory baseDir, String& outFilename );

	protected:
		lua_State* PushResourceRegistry();

//		void SetPrerollSplash( bool newValue ) { fPrerollSplash = newValue; }
/*
	public:
		void BeginOrientationListener();
		void EndOrientationListener();
*/

	public:
		PhysicsWorld& GetPhysicsWorld() { return * fPhysicsWorld; }
		const PhysicsWorld& GetPhysicsWorld() const { return * fPhysicsWorld; }

	public:
		void SetBackend( const char * backend, void * backendState) { fBackend = backend; fBackendState = backendState; }

		const char * GetBackend() const { return fBackend; }

	public:
		// MCallback
		virtual void operator()();

		void Render();

	public:
		void Blit();
		
	public:
		// MCriticalSection
		virtual void Begin() const;
		virtual void End() const;

	public:
		// MDisplayDelegate
		virtual void WillDispatchFrameEvent( const Display& sender );

#if defined(Rtt_AUTHORING_SIMULATOR)
	public:
		static int ShellPluginCollector_Async(lua_State* L);
	private:
		static void FinalizeWorkingThreadWithEvent(Runtime *runtime, lua_State *L);
		std::atomic<std::string*> m_fAsyncResultStr;
		std::atomic<std::thread*> m_fAsyncThread;
		std::atomic<void*> m_fAsyncListener;
#endif

	private:
		String fBuildId;
		Rtt_Allocator& fAllocator;
		const MPlatform& fPlatform;
		const Rtt_AbsoluteTime fStartTime;
		Rtt_AbsoluteTime fStartTimeCorrection;
		Rtt_AbsoluteTime fSuspendTime;
		CachedResource* fResourcesHead; // Dummy node.
		Display *fDisplay;
		LuaContext* fVMContext;
		PlatformTimer* fTimer;
		Scheduler* fScheduler;
		Archive* fArchive;
		PhysicsWorld *fPhysicsWorld;
		const char * fBackend;
		void * fBackendState;
	
#ifdef Rtt_USE_ALMIXER
		PlatformOpenALPlayer* fOpenALPlayer;
#endif

		U8 fFPS;
		S8 fIsSuspended;
		U16 fProperties;
		U32 fSuspendOverrideProperties;
		U32 fFrame;
		int fLaunchArgsRef;
		const char *fSimulatorPlatformName;
		int fDownloadablePluginsRef;
		int fDownloadablePluginsCount;
		const MRuntimeDelegate *fDelegate;
		mutable bool fShowingTrialMessages;

	private:
		friend class LoadMainTask;

	public:
		// to prevent recursive handleError!
		bool fErrorHandlerRecursionGuard;
	public:
		const char* GetBuildId() { return fBuildId.GetString(); }

#ifdef Rtt_USE_ALMIXER
	public:
		PlatformOpenALPlayer* getOpenALPlayer() const{ return fOpenALPlayer; }
#endif
};

// Use this guard to surround code blocks that call Lua as a side effect of
// processing external events, e.g. events driven by the native platform.
class RuntimeGuard
{
	Rtt_CLASS_NO_COPIES( RuntimeGuard )
	Rtt_CLASS_NO_DYNAMIC_ALLOCATION

	public:
		RuntimeGuard( Runtime& runtime );
		~RuntimeGuard();

	private:
		Runtime& fRuntime;
		const MPlatform& fPlatform;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Runtime_H__
