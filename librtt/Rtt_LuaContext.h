//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_LuaContext_H__
#define _Rtt_LuaContext_H__

// ----------------------------------------------------------------------------

#include "Core/Rtt_ResourceHandle.h"

#include "Rtt_LuaAux.h"

namespace Rtt
{

class MEvent;
class MPlatform;
class Runtime;
class StageObject;

// ----------------------------------------------------------------------------

class LuaContext
{
	public:
		typedef LuaContext Self;

	public:
		static LuaContext* New( Rtt_Allocator* pContext, const MPlatform& platform, Runtime *runtime = NULL );
		static void Delete( LuaContext *context );

		static Rtt_Allocator* GetAllocator( lua_State* L );
		static const MPlatform& GetPlatform( lua_State* L );
		static Runtime* GetRuntime( lua_State* L );
        static bool HasRuntime( lua_State* L );
		static LuaContext* GetContext( lua_State *L );

		static bool IsBinaryLua( const char* filename );

	protected:
		static void* Alloc(void *ud, void *ptr, size_t osize, size_t nsize);
		static int Panic( lua_State* );

	public:
		static int OpenJson( lua_State *L );
		static int OpenWidget( lua_State *L );
		static int OpenStoryboard( lua_State *L );
		static int JsonEncode( lua_State *L, int index );
		static int JsonDecode( lua_State *L, const char *json );

	public:
		// Generic, re-entrant Lua callbacks
		static int traceback( lua_State* L );
		static int handleError( lua_State* L, const char *errorType, bool callErrorListener );
		static bool callUnhandledErrorHandler( lua_State* L, const char *message, const char *stacktrace );
	
		#if defined( Rtt_DEBUG )
		static void stackdump( lua_State* L );
		#endif
		
		#if defined( Rtt_DEBUG ) || defined( Rtt_DEBUGGER )
		static void lstop( lua_State* L, lua_Debug *ar );
		#endif

		static int pcall_exit_on_error( lua_State* L );

		static lua_CFunction GetDefaultErrorHandler();

		static void l_message( const char *pname, const char *msg );

	public:
		static void InitializeLuaPath( lua_State* L, const MPlatform& platform );

	protected:
		static void InitializeLibraries( lua_State* L, Runtime *runtime );
		static void InitializeLuaCore( lua_State* L );
		static void InitializeRttCore( lua_State* L, Runtime *runtime );

		static void StartDebugger( lua_State* L );

	public:
		static void RegisterModuleLoader( lua_State *L, const char *name, lua_CFunction loader, int nupvalues = 0 );
		static void RegisterModuleLoaders( lua_State *L, const luaL_Reg moduleLoaders[], int nupvalues = 0 );

	public:
		// For platform-specific code (i.e. outside librtt), you should *not* 
		// use this function for the Corona runtime's instance of the Lua state. 
		// In other words, for code outside librtt, you should only use this method
		// when you are using an instance of the Lua state that's not owned 
		// by the Corona runtime.
		// 
		// For event dispatch to the Corona runtime, use Runtime::DispatchEvent() instead.
		static int DoCall( lua_State* L, int narg, int nresults );
		#ifdef Rtt_AUTHORING_SIMULATOR
			static int DoCPCall( lua_State* L, lua_CFunction func, void* ud );
		#endif

	public:
		static int DoBuffer( lua_State *L, lua_CFunction loader, bool connectToDebugger, lua_CFunction pushargs = NULL );
		static int DoFile( lua_State *L, const char* file, int narg, bool clear = false );

	protected:
		LuaContext( lua_State* L );

	public:
		// If runtime is NULL then only core Lua libs are init'd
		void Initialize( const MPlatform& platform, Runtime* runtime );
		void DisableParser( bool isDebuggerConnected );
		void RestrictFunction( const char *libName, const char *functionName );

//		void Initialize( Runtime& runtime, const MPlatform& platform, bool connectToDebugger );
//		void Initialize( const char scriptPath[], const MPlatform& platform, bool connectToDebugger );
		lua_State *L() const { return fL; }
		const ResourceHandle< lua_State >& LuaState() const { return fHandle; }

	public:
		~LuaContext();

	public:
		void Collect();

	public:
		// See comments for DoCall(): do *not* call from outside librtt *if* the receiver
		// belongs to the Corona runtime.
		int DoBuffer( lua_CFunction loader, bool connectToDebugger, lua_CFunction pushargs = NULL );
		int DoFile( const char* file, bool connectToDebugger, int narg );
		int DoFile( const char* file, int narg, bool clear = false );

//		void DispatchEvent( const MEvent& e );

	public:
		void UpdateStage( StageObject& stage ) const ;

		#ifdef Rtt_AUTHORING_SIMULATOR
			// Bit mask for modules
			typedef enum
			{
				kPhysicsModuleMask = 0x1,
				kSpriteModuleMask = 0x2,
			}
			ModuleMask;
		
			U32 GetModules() const { return fModules; }

			// Register a lua module. This is used by the simulator to indicate to the build server
			// that the module is being used, so it can select the appropriate template.
			static void RegisterModule( lua_State *L, ModuleMask module );
		#endif
		
	private:
		lua_State* fL;
		ResourceHandleOwner< lua_State > fHandle;
		U32 fModules; // Used by Simulator to determine what modules are in use
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Matrix_H__
