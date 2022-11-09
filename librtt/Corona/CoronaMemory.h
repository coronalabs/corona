//-----------------------------------------------------------------------------
//
// Corona Labs
//
// easing.lua
//
// Code is MIT licensed; see https://www.coronalabs.com/links/code/license
//
//-----------------------------------------------------------------------------

#ifndef _CoronaMemory_H__
#define _CoronaMemory_H__

#include "CoronaMacros.h"

#ifdef __cplusplus
extern "C" {
#endif
	typedef struct lua_State lua_State;
#ifdef __cplusplus
}
#endif

/**
 A value available to various memory routines.
 In a few cases, described below, these will have an initial value.
 Otherwise, the value is to be user-provided.
*/
union CoronaMemoryWorkVar {
	/**
	 Pointer value, e.g. from a `lua_touserdata()`. Does not take ownership.
	*/
	void* p;

	/**
	 Const pointer value, e.g. from a `lua_tostring()`. Does not take ownership.
	*/
	const void* cp;

	/**
	 Size value, e.g. from a `lua_objlen()`.
	*/
	size_t size;

	/**
	 Double-precision value, e.g. a Lua number.
	*/
	double n;

	/**
	 Unsigned integer value.
	*/
	unsigned int u;

	/**
	 Signed integer value.
	*/
	int i;
};

/**
 This structure contains the working state of an acquired interface.
*/
struct CoronaMemoryWorkspace {
	/**
	 Scratch memory available to callbacks. This should be enough for most needs.
	 A few of these will be pre-populated by `CoronaMemoryAcquireInterface()` and `CoronaMemoryPushLookupEncoding()`,
	 per their docs. These values simply provide some information to users and may safely be overwritten.
	*/
	CoronaMemoryWorkVar vars[8];

	/**
	 Available for error reporting; `error[0]` is set to `\0` before `getObject()` is called.
	*/
	char error[64];

	/**
	 The interface's data, as described in `CoronaMemoryInterfaceInfo`.
	*/
	void* data;

	/**
	 The interface's data size, as described in `CoronaMemoryInterfaceInfo`.
	*/
	size_t dataSize;
};

/**
 This structure contains all the callbacks that together make up a memory type interface.
 Many callbacks are optional and may be left `NULL`.
 The C API does not call any of these directly.
*/
struct CoronaMemoryCallbacks {
	// An interface may provide both `getReadableBytes()` and `getWriteableBytes()`; it MUST provide at least one of them.

	/**
	 Optional, but see note above
	 Get the readable memory available from an object through the interface.
	 @param ws Workspace.
	 @param context Used to provide call-specific input to the getter. May be `NULL`, and this must be valid or at least safe.
	 @return Read-only memory corresponding to `object`.
	 It CAN return `NULL`, say if `object` is temporarily invalid or empty, but `getByteCount()` MUST then return 0; conversely,
	 if the latter returns non-0, `getReadableBytes()`'s result MUST point to memory at least satisfying that number of bytes.
	*/
	const void* ( *getReadableBytes )( CoronaMemoryWorkspace *ws );

	/**
	 Optional, but see note above
	 Get the writeable memory available from an object through the interface.
	 The memory might also be readable, but this is at the interface provider's discretion.
	 @param ws Workspace.
	 @param context Used to provide call-specific input to the getter. May be `NULL`, and this must be valid or at least safe.
	 An example of `context` might be as a "credential": while the memory might indeed be writeable, only some callers should be
	 "trusted" with it (for instance, structured data); passing in a valid key indicates the caller has this trust.
	 ^^^ TODO: not used...
	 @return Writeable memory corresponding to `object`.
	 `NULL` results are as per `getReadableBytes()`.
	*/
	void* ( *getWriteableBytes )( CoronaMemoryWorkspace *ws );

	/**
	 Required
	 Get the amount of memory available from an object through the interface.
	 @param ws Workspace.
	 @return Number of bytes that may be read from or written to `object`'s memory.
	*/	
	size_t ( *getByteCount )( CoronaMemoryWorkspace *ws );

	/**
	 Optional
	 Update the amount of memory available from an object through the interface.
	 Earlier results of `getReadableBytes()` and `getWriteableBytes()` can be invalidated by this operation.
	 Typically this makes most sense for writeable memory.
	 It is at the interface provider's discretion whether the previous contents are preserved and how new bytes, when `size` is
	 larger than before, are populated.
	 @param ws Workspace.
	 @param size Memory size after resize.
	 @param writeable If non-0, this is intended for `getWriteableBytes()`, else `getReadableBytes()`.
	 On success, a new call to `getByteCount()` must return `size` or larger.
	 Failure must also be robust, e.g. returning 0, or the old size, if the resources are still intact.
	 @param context Used to provide call-specific input to the resize.
	 @return If non-0, success.
	*/
	int ( *resize )( CoronaMemoryWorkspace *ws, size_t size, int writeable );

	// These are fairly specialized routines, and may each be `NULL`: while some special-purpose logic might
	// always check them just to be thorough, it would be too much trouble accounting for these properties on
	// each and every `CoronaMemoryAcquireInterface()` call: as a "good citizen", a memory provider will want
	// to advertise (in the docs, say) when any are valid, so any consumer knows to bother consulting them.

	/**
	 Optional
	 Report any particular memory alignment, say if the object is in shared memory or intended for SIMD operations.
	 @param ws Workspace.
	 @return May be 0 ("normal" memory) or a suitable power of 2.
	*/
	size_t ( *getAlignment )( CoronaMemoryWorkspace *ws );

	/**
	 Optional
	 Query sizes of structured memory, e.g. for texture data 0 = row count, 1 = column count, 2 = bytes per pixel.
	 @param ws Workspace.
	 @param index Zero-based size index.
	 @param size Requested size.
	 @return If non-0, `index` < size count and `dim` must be populated.
	*/
	int ( *getSize )( CoronaMemoryWorkspace *ws, unsigned int index, size_t *size );

	/**
	 Optional
	 Query strides of structured memory, i.e. the number of bytes between successive entries along an axis.
	 Usually the stride will be the product of lower-dimensional sizes, e.g. row-to-row stride = column count * bytes per pixel, but
	 sometimes padding is useful, e.g. for alignment or when using a subregion of memory.
	 @param index Zero-based stride index.
	 @param stride Requested stride.
	 @return If non-0, `index` < stride count and `stride` must be populated.
	*/
	int ( *getStride )( CoronaMemoryWorkspace *ws, unsigned int index, size_t *stride );
};

/**
 This structure provides the information needed to create an interface.
*/
struct CoronaMemoryInterfaceInfo {
	/**
	 Required
	 The callbacks to register with the interface.
	*/
	CoronaMemoryCallbacks callbacks;

	/**
	 Required
	 Called by `CoronaMemoryAcquireInterface()` to finish the acquire process.
	 Any state that will be needed by the methods may be assigned to `workspace` and / or the Lua stack. Once
	 this call begins, neither will be further modified internally.
	*/
	int ( *getObject )( lua_State *L, int arg, CoronaMemoryWorkspace *workspace );

	/**
	 Optional
	 This is used to associate some user-defined data with the interface, provided through  `CoronaMemoryWorkspace`.
	 If `dataSize` is 0, `data` will be `NULL` and `dataSize` 0.
	 When `dataSize` > 0, the proxy will be created with `lua_newuserdata(L, dataSize`); its block address will be used as `data`
	 and `dataSize` will supply the workspace member of the same name.
	 When `dataSize` < 0 (any value), a userdata MUST be on top of the stack; it will be added to the proxy's environment. The
	 result of `lua_touserdata(L, -1)` will be supplied as `data`, and `lua_objlen(L, -1)` as `dataSize` (n.b. for light userdata,
	 this will be 0).
	*/
	int dataSize;
};

struct CoronaMemoryAcquireState; // forward reference

/**
 Memory operations built atop the user-provided callbacks, provided when the interface has been acquired.
 Always-fail / no-op stubs will be provided for absent callbacks.
*/
struct CoronaMemoryInterface {
	// The following details are found in version 0+:

	/**
	 Passthrough wrapper to `getReadableBytes()`, if available, else returns `NULL`.
	*/
	const void* ( *getReadableBytes )( CoronaMemoryAcquireState *state );

	/**
	 If `getReadableBytes()` is absent, returns `NULL`.
	 Otherwise, if the byte count is >= `n`, gets the bytes.
	 Failing that, it will call `resize()`, if present, in read mode.
	 If the resize was successful, gets the bytes; else returns `NULL`.
	*/
	const void* ( *getReadableBytesOfSize )( CoronaMemoryAcquireState *state, size_t n );

	/**
	 If `getReadableBytes()` is absent, does nothing.
	 Otherwise, gets the bytes and writes them to `output`, up to a maximum of `outputSize` bytes. If fewer than
	 `outputSize` bytes were available and `ignoreExtra` is 0, the leftover bytes will be set to 0.
	*/
	void ( *copyBytesTo )( CoronaMemoryAcquireState *state, void* output, size_t outputSize, int ignoreExtra );

	/**
	 Passthrough wrapper to `getWriteableBytes()`, if available, else returns `NULL`.
	*/
	void* ( *getWriteableBytes )( CoronaMemoryAcquireState *state );

	/**
	 If `getWriteableBytes()` is absent, returns `NULL`.
	 Otherwise, if the byte count is >= `n`, gets the bytes.
	 Failing that, it will call `resize()`, if present, in write mode.
	 If the resize was successful, gets the bytes; else returns `NULL`.	 
	*/
	void* ( *getWriteableBytesOfSize )( CoronaMemoryAcquireState *state, size_t n );

	/**
	 Passthrough wrapper to `resize()`, if available, else returns 0.
	*/
	int ( *resize )( CoronaMemoryAcquireState *state, size_t size, int writeable );

	/**
	 Passthrough wrapper to `getByteCount()`. (As a dummy, returns 0.)
	*/
	size_t ( *getByteCount )( CoronaMemoryAcquireState *state );

	/**
	 Passthrough wrapper to `getAlignment()`, if available, else returns 0.
	*/
	size_t ( *getAlignment )( CoronaMemoryAcquireState *state );

	/**
	  Passthrough wrapper to `getSize()`, if available, else returns 0.
	*/
	int ( *getSize )( CoronaMemoryAcquireState *state, unsigned int index, size_t *size );

	/**
	 Passthrough wrapper to `getStride()`, if available, else returns 0.
	*/
	int ( *getStride )( CoronaMemoryAcquireState *state, unsigned int index, size_t *stride );
};

/**
	This structure maintains some details needed by the memory interface after an acquisition, as well
	as the workspace provided for the underlying methods.
*/
struct CoronaMemoryAcquireState {
	/**
	 This is the "proper" way to use the interface's callbacks, rather than directly invoking them.
	 An example raw call, given an instance `state`: `state.methods.resize(&state, newSize, 0)`.
	 See also `CORONA_MEMORY_IFC` and related macros.
	*/
	CoronaMemoryInterface methods;

	/**
	 Callbacks used by the interface methods.
	*/
	const CoronaMemoryCallbacks *callbacks;
	
	/**
	 Workspace for the current acquire, provided to underlying callbacks.
	 This will also be seen by `getObject()`; initial values may be supplied to it through `vars`. (N.B. a few
	 variables will be stomped on, however, q.v. the comments on `CoronaMemoryAcquireInterface()` and
	 `CoronaMemoryPushLookupEncoding()`.)
	*/
	CoronaMemoryWorkspace workspace;

	/**
	 Version of memory API known to the object, describing its feature set.
	 It is supplied by `CoronaMemoryCreateInterface()`, corresponding to the Solar library linked by its caller.
	 Currently, it is always 0 (base feature set), but would be incremented if important new features were added.
	 The upshot is that consumers and providers of memory might be in separate plugins, say, that were linked
	 against different feature levels. The version must therefore be consulted to provide a suitable  `interface`.
	*/
	int version;
};

/**
 Helpers to use interface callbacks, given a method name and `CoronaMemoryAcquireState` object.
 For example, one can do `bytes = CORONA_MEMORY_IFC(state, getReadableBytes);` to get the readable bytes, or
 `ok = CORONA_MEMORY_IFC_WITH_ARGS(state, resize, newSize, writeable)` to attempt a resize of writeable bytes.
*/
#define CORONA_MEMORY_IFC( STATE, NAME ) STATE.methods.NAME( &STATE )
#define CORONA_MEMORY_IFC_WITH_ARGS( STATE, NAME, ... ) STATE.methods.NAME( &STATE, __VA_ARGS__ )

/**
 Variant for use with `get`-prefixed methods; it will dispense with that part.
*/
#define CORONA_MEMORY_GET( STATE, NAME ) CORONA_MEMORY_IFC( STATE, get##NAME )
#define CORONA_MEMORY_GET_WITH_ARGS( STATE, NAME, ... ) CORONA_MEMORY_IFC_WITH_ARGS( STATE, get##NAME, __VA_ARGS__ )

/**
 Indicates whether a callbacks exists. (N.B. it will still have a dummy method, if not.)
*/
#define CORONA_MEMORY_HAS( STATE, NAME ) !!STATE.callbacks.NAME

/**
 Pointer-based variants of the above.
*/
#define CORONA_MEMORY_IFC_P( PSTATE, NAME ) PSTATE->methods.NAME( PSTATE )
#define CORONA_MEMORY_IFC_WITH_ARGS_P( PSTATE, NAME, ... ) PSTATE->methods.NAME( PSTATE, __VA_ARGS__ )
#define CORONA_MEMORY_GET_P( PSTATE, NAME ) CORONA_MEMORY_IFC_P( PSTATE, get##NAME )
#define CORONA_MEMORY_GET_WITH_ARGS_P( PSTATE, NAME, ... ) CORONA_MEMORY_IFC_WITH_ARGS_P( PSTATE, get##NAME, __VA_ARGS__ )
#define CORONA_MEMORY_HAS_P( PSTATE, NAME ) !!PSTATE->callbacks.NAME

// C API
// ----------------------------------------------------------------------------

/**
 *
 * Memory providers:
 *
 */

/**
	Create an interface that may be used to provide access to objects' memory.
	Details may be found under `CoronaMemoryInterfaceInfo` and `CoronaMemoryAcquireInterface()`.
	The interface is made available through a proxy object. Each proxy receives a unique environment
	table, whose integer keys are reserved for internal use; other keys are free for custom use.
	@param L Lua state pointer.
	@param info Callback and interface-specific data information.
	@return If non-0, success, and a memory interface proxy will be on top of the stack.
*/
CORONA_API
int CoronaMemoryCreateInterface( lua_State *L, const CoronaMemoryInterfaceInfo *info ) CORONA_PUBLIC_SUFFIX;

/**
	Given a memory proxy, as returned by `CoronaMemoryCreateInterface()`, on top of the stack,
	associates it with a lookup slot. While the proxy is bound to the slot, encodings may be made
	referencing it via calls to `CoronaMemoryPushLookupEncoding()`.
	N.B. A few bits are reserved, so there are fewer than 2^16 available slots. If all slots happen
	to be in use, the bind will fail.
	@param L Lua state pointer.
	@param id ID used to refer to the proxy / lookup slot.
	@return If non-0, success, `id` is populated, and the proxy is popped from the stack.
*/
CORONA_API
int CoronaMemoryBindLookupSlot( lua_State *L, unsigned short *id ) CORONA_PUBLIC_SUFFIX;

/**
	Unbind a lookup slot and detach the proxy associated with it.
	It is up to the caller to handle any lingering encodings made from `id`, which this will invalidate.
	@param L Lua state pointer.
	@param id An ID returned by `CoronaMemoryBindLookupSlot`.
	@return If non-0, the slot was in use.
*/
CORONA_API
int CoronaMemoryReleaseLookupSlot( lua_State *L, unsigned short id ) CORONA_PUBLIC_SUFFIX;

/**
	Encode an ID / context pair as a light userdata.
	If `CoronaMemoryAcquireInterface()` encounters such a value, it will use the proxy bound to the ID. Furthermore, before
	`getObject()` is called, its workspace is pre-populated: `vars[0].u` will be non-0, and `vars[1].u` and `vars[2].u` will be set
	to `id` and `context`, respectively.
	This API is meant, via `context`, to allow multiple values to be provided from a common data source, e.g. an array.
	@param L Lua state pointer.
	@param id An ID returned by `CoronaMemoryBindLookupSlot`.
	@param context 16-bit user-defined value to pair with the ID.
	@return If non-0, success, and the userdata will be on top of the stack.
*/
CORONA_API
int CoronaMemoryPushLookupEncoding( lua_State *L, unsigned short id, unsigned short context ) CORONA_PUBLIC_SUFFIX;

/**
 *
 * Memory consumers:
 *
 */

/**
	Acquire a reference to the memory interface of an object on the stack, in order to read from and / or
	write to its memory.
	Strings are a special case, having a default interface. Its `getByteCount()` and `getReadableBytes()`
	will return `lua_objlen( L, arg )` and `lua_tostring( L, arg )`, respectively.
	Otherwise, a memory interface proxy, as created by `CoronaMemoryCreateInterface()`, must be found.
	Usually, object will be expected to have a proxy in the `__memory` key of their metatables. The exception
	are light userdata, described in `CoronaMemoryBindLookupSlot()` and `CoronaMemoryPushLookupEncoding()`.
	In the non-light userdata case, `vars[0].u` in the workspace will be set to 0 on a successful acquire.
	To complete the acquisition, the call `ok = getObject( L, arg, &workspace )` is performed, with `ok` being
	non-0 understood as success.
	Any changes to the stack made by `getObject()` are left intact.
	@param L Lua state pointer.
	@param arg Stack index of object that will provide the memory.
	@param state State used to interface with the acquired memory.
	@return If non-0, success, and `state` is populated. (On failure, a dummy interface will be assigned.)
*/
CORONA_API
int CoronaMemoryAcquireInterface( lua_State *L, int arg, CoronaMemoryAcquireState *state ) CORONA_PUBLIC_SUFFIX;

#endif // _CoronaGraphics_H__
