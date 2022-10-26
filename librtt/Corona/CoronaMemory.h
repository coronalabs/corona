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
#include <stdint.h>

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
	const unsigned char * ( *getReadableBytes )( CoronaMemoryWorkspace *ws );

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
	unsigned char * ( *getWriteableBytes )( CoronaMemoryWorkspace *ws );

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
	size_t ( *getAlignment )(CoronaMemoryWorkspace *ws );

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
 TODO TODO TODO TODO TODO
*/
struct CoronaMemoryInterfaceInfo {
	/**
	 Required
	 The callbacks to register with the interface.
	*/
	CoronaMemoryCallbacks callbacks;

	/**
	 Required
	*/
	int ( *getObject )( lua_State *L, int arg, CoronaMemoryWorkspace *ws ); // required
																	// used by `MemoryAcquireInterface()` to prepare the callbacks' workspace
																	// returning non-0 indicates success
																	// `arg` will be normalized
																	// cf. the details about the light userdata policy, below
																	// once this call begins, `ws` is never modified internally
																	// similarly, the Lua object is no longer used internally
																							// any changes to the Lua stack will remain after the call

	/**
	 Optional
	 This is used to associate some user-defined data with the interface, provided through some members in `CoronaMemoryWorkspace`.
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
 Memory operations built atop the user-provided callbacks.
 Always-fail / no-op stubs will be provided for absent callbacks.
*/
struct CoronaMemoryInterface {
	// The following details are found in version 0+:

	/**
	 TODO TODO TODO TODO TODO
	*/
	const unsigned char * ( *getReadableBytes )( CoronaMemoryAcquireState *state );

	/**
	 TODO TODO TODO TODO TODO
	*/
	const unsigned char * ( *getReadableBytesOfSize )( CoronaMemoryAcquireState *state, size_t n ); // possible `resize()`, then `getReadableBytes()`

	/**
	 TODO TODO TODO TODO TODO
	*/
	void ( *copyBytesTo )( CoronaMemoryAcquireState *state, unsigned char *output, size_t outputSize, int ignoreExtra ); // `getReadableBytes()`, copy of `min(getByteCount(), outputSize)` to `output`
																																			   // if `outputSize` > `getByteCount()`
																																				  // the rest is set to 0, unless `ignoreExtra` is non-0

	/**
	 TODO TODO TODO TODO TODO
	*/
	unsigned char * ( *getWriteableBytes )( CoronaMemoryAcquireState *state );

	/**
	 TODO TODO TODO TODO TODO
	*/
	unsigned char * ( *getWriteableBytesOfSize )( CoronaMemoryAcquireState *state, size_t n ); // possible `resize()`, then `getWriteableBytes()`

	/**
	 TODO TODO TODO TODO TODO
	*/
	int ( *resize )( CoronaMemoryAcquireState *state, size_t size, int writeable );

	/**
	 TODO TODO TODO TODO TODO
	*/
	size_t ( *getByteCount )( CoronaMemoryAcquireState *state );

	/**
	 TODO TODO TODO TODO TODO
	*/
	size_t ( *getAlignment )( CoronaMemoryAcquireState *state );

	/**
	 TODO TODO TODO TODO TODO
	*/
	int ( *getSize )( CoronaMemoryAcquireState *state, unsigned int index, size_t *size );

	/**
	 TODO TODO TODO TODO TODO
	*/
	int ( *getStride )( CoronaMemoryAcquireState *state, unsigned int index, size_t *stride );
};

/**
	TODO TODO TODO TODO TODO 
*/
struct CoronaMemoryAcquireState {
	/**
	 This is the "proper" way to use the interface's callbacks, rather than directly invoking them.
	 An example raw call, given an instance `state`: `state.interface.resize(&state, newSize, NULL)`.
	 See also `CORONA_MEMORY_IFC` and `CORONA_MEMORY_IFC_WITH_ARGS`.
	*/
	CoronaMemoryInterface interface; // these build on `callbacks` and provide the "proper" way to call them

	// The following should generally be considered implementation details, cf. `MemoryInterfaceInfo`, and not touched by users.

	/**
	 Callbacks used by the interface methods.
	*/
	const CoronaMemoryCallbacks * callbacks;
	
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
 For example, one can do `bytes = CORONA_MEMORY_IFC(getReadableBytes, state);` to get the readable bytes, or
 `ok = CORONA_MEMORY_IFC_WITH_ARGS(resize, state, newSize, writeable)` to attempt a resize of writeable bytes.
*/
#define CORONA_MEMORY_IFC( NAME, CMAS ) OBJECT.interface.NAME( &CMAS )
#define CORONA_MEMORY_IFC_WITH_ARGS( NAME, CMAS, ... ) OBJECT.interface.NAME( &CMAS, __VA_ARGS__ )

// C API
// ----------------------------------------------------------------------------

/**
 *
 * Memory providers:
 *
 */

/**
	TODO TODO TODO TODO TODO
	@param L Lua state pointer.
	@param mii Callback and interface-specific data information.
	@return If non-0, success, and a memory interface proxy will be on top of the stack.
*/
CORONA_API
int CoronaMemoryCreateInterface( lua_State *L, const CoronaMemoryInterfaceInfo *mii ) CORONA_PUBLIC_SUFFIX; // returns non-0 on success, and pushes a memory interface proxy onto the stack
																			// at a minimum, the interface must implement `getByteCount()`, plus either `getReadableBytes()` or `getWriteableBytes()`
																			// if `dataSize` was > 0, do a `lua_touserdata()` on the proxy to retrieve and populate `data`
																			// to use the interface, assign the proxy to a metatable's `__memory` field
																			// the proxy will be assigned a memory API version based on the Solar library being linked against
																			// the proxy is given a unique environment
																				// integer keys are used internally
																				// others are fine for custom use

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
int CoronaMemoryBindLookupSlot( lua_State *L, uint16_t *id ) CORONA_PUBLIC_SUFFIX; // returns non-0 on success, and populates `id`
													// per the above, the `id` would reference a memory proxy that was on top of the stack, adding it to the registry table
/**
	Unbind a lookup slot and detach the proxy associated with it.
	@param L Lua state pointer.
	@param id An ID returned by `CoronaMemoryBindLookupSlot`.
	@return If non-0, the slot was in use.
*/
CORONA_API
int CoronaMemoryReleaseLookupSlot( lua_State *L, uint16_t id ) CORONA_PUBLIC_SUFFIX; // this would be called, say, in an object's `__gc`, a "finalize" event, or some other `destroy()` logic
											// `id` may again be allocated after this; any lingering encodings are invalidated

/**
	Encode an ID / context pair as a light userdata.
	If `CoronaMemoryAcquireInterface()` encounters such a value, it will use the proxy bound to the ID. Furthermore, before
	`getObject()` is called, `vars[0].u` will be non-0, and `vars[1].u` and `vars[2].u` will be set to `id` and `context`, respectively.
	This API is meant, via `context`, to allow multiple values to be provided from a common data source, e.g. an array.
	@param L Lua state pointer.
	@param id An ID returned by `CoronaMemoryBindLookupSlot`.
	@param context 16-bit user-defined value to pair with the ID.
	@return If non-0, success, and the userdata will be on top of the stack.
*/
CORONA_API
int CoronaMemoryPushLookupEncoding( lua_State *L, uint16_t id, uint16_t context ) CORONA_PUBLIC_SUFFIX;

/**
 *
 * Memory consumers:
 *
 */

/**
	TODO TODO TODO TODO TODO
	Strings -> TODO
	Light userdata -> TODO
	else __memory metafield -> TODO
	vars[0].u will be 0, before getObject()
	okay to pre-populate remaining vars, but see note about lookup encoding
	Appropriate interface
	@param L Lua state pointer.
	@param arg Object that will provide the memory.
	@param state State used to interface with the acquired memory.
	@return If non-0, success, and `state` is populated. (On failure, a dummy interface will be assigned.)
*/
CORONA_API
int CoronaMemoryAcquireInterface( lua_State *L, int arg, CoronaMemoryAcquireState *state ) CORONA_PUBLIC_SUFFIX;

#endif // _CoronaGraphics_H__
