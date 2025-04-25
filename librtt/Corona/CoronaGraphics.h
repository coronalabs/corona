//-----------------------------------------------------------------------------
//
// Corona Labs
//
// easing.lua
//
// Code is MIT licensed; see https://www.coronalabs.com/links/code/license
//
//-----------------------------------------------------------------------------

#ifndef _CoronaGraphics_H__
#define _CoronaGraphics_H__

#include "CoronaMacros.h"
#include "CoronaPublicTypes.h"

#ifdef __cplusplus
extern "C" {
#endif
    typedef struct lua_State lua_State;
#ifdef __cplusplus
}
#endif

/**
 Ennumeration describing format of the bitmap
 Bitmap channels are (left to right) from MSB to LSB. For example RGBA, A is in the least-significant 8 bits
 RGBA format uses premultiplied alpha. This means that if "raw" values of channels are r,g,b and a, red channel should be r*(a/255)
*/
typedef enum {
    /**
     If not defined, RGBA would be used
    */
    kExternalBitmapFormat_Undefined = 0, // kExternalBitmapFormat_RGBA would be used
    
    /**
     Textures with bitmaps of this format can be used only as masks
     Alpha, 1 byte per pixel
     Important: if this format is used, width must be multiple of 4
    */
    kExternalBitmapFormat_Mask,
    
    /**
     RGB, 3 bytes per pixel
    */
    kExternalBitmapFormat_RGB,
    
    /**
     RGBA, 4 bytes per pixel
     Important: Red, Green and Blue channels must have premultiplied alpha
     */
    kExternalBitmapFormat_RGBA,
    
} CoronaExternalBitmapFormat;


/**
 This structure contains callbacks required for TextureResource's life cycle
 When Corona would require some information about Texture or it's bitmap, a callback would be invoked
 `userData` parameter is passed to all callbacks is same with which TextureResourceExternal was created
 Typical workflow & callbacks descriptions:
  - time to time Corona would request `getWidth()` and `getHeight()` for various calculation
  - when texture's bitmap data is required:
      * to retrieve raw data pointer `onRequestBitmap()` is called
      * `onRequestBitmap()` should always return valid pointer
        to `getWidth()*getHeight()*CoronaExternalFormatBPP(getFormat())` bytes
      * pixels should be aligned row-by-row; first byte of pixel on row `Y` and column `X`:
        `((unsigned char*)onRequestBitmap())[ (Y*getWidth() + X) * CoronaExternalFormatBPP(getFormat())]`
      * Corona would read data for a short time and call `onReleaseBitmap()` when done
      * bitmap pointer need not be valid after `onReleaseBitmap()` is called
      * `onRequestBitmap()` (and consequent `onReleaseBitmap()`) may be called several times when data access is required
  - `getFormat()` should return bitmap format; if NULL RGBA would be used
      * if `kExternalBitmapFormat_Mask` is returned, texture would be treated as mask
  - `onFinalize()` if present, would be called when texture is no longer needed. Usually happens when `texture:releaseSelf()`
    is called and all objects using texture are destroyed. Also called when app is shutting down or restarted
  - `onGetField()` is used when user queries texture for unknown field from Lua. Returned number
    must be a number of values pushed on Lua stack

 In order to create external bitmap you must provide width, height and bitmap callbacks
 all other are optional and will be ignored if set to NULL
*/
typedef struct CoronaExternalTextureCallbacks
{
    /**
     Required
     When creating instance of this type set this member to `size = sizeof(CoronaExternalTextureCallbacks)`.
     This is required for identifying version of API used.
    */
    unsigned long size;
    
    /**
     Required
     called when Texture bitmap's width is required
     @param userData Pointer passed to CoronaExternalPushTexture
     @return The width of Texture's bitmap; Important: if it is a Mask, width should be a multiple of 4
    */
    unsigned int (*getWidth)(void* userData);
    
    /**
     Required
     called when Texture bitmap's height is required
     @param userData Pointer passed to CoronaExternalPushTexture
     @return The width of Texture's height
    */
    unsigned int (*getHeight)(void* userData);
    
    /**
     Required
     called when Texture bitmap's data is required. Always followed by @see onReleaseBitmap call.
     @param userData Pointer passed to CoronaExternalPushTexture
     @return Valid pointer to data containing bitmap information. Corona expects bitmap data to be row-by-row array of pixels
             starting from top of the image, each pixel represented by `bpp = CoronaExternalFormatBPP(getFormat())` bytes.
             Each channel use 1 byte and ordered same as format name, left to right. So, with RGBA format, R index is 0
             Overall size of memory must me at least `getWidth()*getHeight()*bpp`
             Accessing left most (R in RGBA) value of bitmap could be written as
             `((unsigned char*)onRequestBitmap())[ (Y*getWidth() + X) * CoronaExternalFormatBPP(getFormat()) ]`
             RGBA format (default) uses premultiplied alpha
    */
    const void* (*onRequestBitmap)(void* userData);
    
    /**
     Optional
     Called when Texture bitmap's data is no longer required.
     After this callback is invoked, pointer returned by `onRequestBitmap` need not longer be valid
     @param userData Pointer passed to CoronaExternalPushTexture
    */
    void (*onReleaseBitmap)(void* userData);

    /**
     Optional
     called when Texture bitmap's format is required
     @param userData Pointer passed to CoronaExternalPushTexture
     @return One of the CoronaExternalBitmapFormat entries. Default format is RGBA (kExternalBitmapFormat_RGBA)
    */
    CoronaExternalBitmapFormat (*getFormat)(void* userData);
    
    /**
     Optional
     Called when TextureResource is about to be destroyed
     After this callback is invoked, no callbacks or returned bitmap pointers would be accessed
     @param userData Pointer passed to CoronaExternalPushTexture
    */
    void (*onFinalize)(void *userData);     // optional; texture will not be used again
    
    /**
     Optional
     Called when unknown property of Texture is requested from Lua
     @param L Lua state pointer
     @param field String containing name of requested field
     @param userData Pointer passed to CoronaExternalPushTexture
     @return number of values pushed on Lua stack
    */
    int (*onGetField)(lua_State *L, const char *field, void* userData);   // optional; called Lua texture property lookup
} CoronaExternalTextureCallbacks;

// C API
// ----------------------------------------------------------------------------

/**
 Pushes TextureResourseExternal instance onto stack
 @param L Lua state pointer
 @param callbacks set of callbacks used to create texture. @see CoronaExternalTextureCallbacks
 @param userData pointer which would be passed to callbacks
 @return number of values pushed onto Lua stack;
         1 - means texture was successfully created and is on stack
         0 - error occurred and nothing was pushed on stack
*/
CORONA_API
int CoronaExternalPushTexture( lua_State *L, const CoronaExternalTextureCallbacks *callbacks, void* userData ) CORONA_PUBLIC_SUFFIX;

/**
 Retrieves userData from TextureResourceExternal on Lua stack
 @param index: location of texture resource on Lua stack
 @return `userData` value texture was created with or
         NULL if stack doesn't contain valid external texture resource at specified index
*/
CORONA_API
void* CoronaExternalGetUserData( lua_State *L, int index ) CORONA_PUBLIC_SUFFIX;

/**
 Helper function, returns how many Bytes Per Pixel bitmap of specified format has
 @param format CoronaExternalBitmapFormat to check
 @return number of bytes per pixel (bpp) of bitmap with specified bitmap format
*/
CORONA_API
int CoronaExternalFormatBPP(CoronaExternalBitmapFormat format) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

/**
 Invalidate the display hierarchy.
 @param L Lua state.
*/
CORONA_API
void CoronaRendererInvalidate( lua_State * L ) CORONA_PUBLIC_SUFFIX;

/**
 Operation performed when processing a dirty state block, i.e. preparing to draw, with the new contents to apply.
 When `restore` is 0, the working contents are supplied as "new"; those committed before the last draw (or failing
 that, the block's defaults), as "old". This happens after any other built-in state is updated, e.g. shader binds.
 Otherwise, this is a "restore": it happens at the start of frame (`commandBuffer` will have been reset), the "new"
 contents are the block's defaults, and the "old" the last commit as above.
*/
typedef void (*CoronaStateBlockDirty)( const CoronaCommandBuffer * commandBuffer, const CoronaRenderer * renderer, const void * newContents, const void * oldContents, unsigned int size, int restore, void * userData );

/**
 This structure describes a state block.
*/
typedef struct CoronaStateBlock {
    /**
     Required
     Size of block, in bytes.
    */
    unsigned int blockSize;
    
    /**
     Optional
     If non-`NULL`, the default block contents (>= `blockSize` bytes, in size); else all 0s.
    */
    void * defaultContents;

    /**
     Optional
     If non-`NULL`, data supplied to the dirty state handler.
    */
    void * userData;

    /**
     Required
     Draw-time dirty state handler.
    */
    CoronaStateBlockDirty stateDirty;
    
    /**
     Optional
     If non-`NULL`, the `restore`-time dirty state handler; else `stateDirty`.
    */
    CoronaStateBlockDirty defaultStateDirty;

    /**
     Optional
     TODO: used e.g. in Vulkan
    */
    int dontHash;
} CoronaStateBlock;

/**
 Permanently register a state block.
 @param L Lua state pointer.
 @param block Block configuration.
 @param blockID This is populated with a non-0 ID on success.
 @return If non-0, the block was registered.
*/
CORONA_API
int CoronaRendererRegisterStateBlock( lua_State * L, const CoronaStateBlock * block, unsigned long * blockID ) CORONA_PUBLIC_SUFFIX;

/**
 Read the working contents of a state block.
 @param renderer Boxed renderer.
 @param blockID  An ID returned by `CoronaRendererRegisterStateBlock`.
 @param data If not `NULL`, populated with the block's working contents.
 @param size On input, the size of `data` (if not `NULL`), in bytes; on output, the block size.
 @return If non-0, the block was read.
*/
CORONA_API
int CoronaRendererReadStateBlock( const CoronaRenderer * renderer, unsigned long blockID, void * data, unsigned int * size ) CORONA_PUBLIC_SUFFIX;

/**
 Update the working contents of a state block.
 When drawing, any block with uncommitted changes will have its dirty handler called.
 Similarly, when the frame ends, "restore" handlers are called for any block with non-default contents.
 Changes are considered committed after a draw or restore.
 @param renderer Boxed renderer.
 @param blockID An ID returned by `CoronaRendererRegisterStateBlock`.
 @param data Data to write.
 @param size Size of `data`, in bytes. If the block size is smaller, it will be clamped.
 @return If non-0, the block was written.
*/
CORONA_API
int CoronaRendererWriteStateBlock( const CoronaRenderer * renderer, unsigned long blockID, const void * data, unsigned int size ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

/**
 Operation that responds to data read from a command's buffer.
*/
typedef void (*CoronaCommandReader)( const CoronaCommandBuffer * commandBuffer, const unsigned char * from, unsigned int size );

/**
 Operation that writes data into a command's buffer.
*/
typedef void (*CoronaCommandWriter)( const CoronaCommandBuffer * commandBuffer, unsigned char * to, const void * data, unsigned int size );

/**
 IO operations that constitute a command.
*/
typedef struct CoronaCommand {
    /**
     The operation where the command executes, given the data written earlier.
    */
    CoronaCommandReader reader;
    
    /**
     When the command is issued, a range of bytes are allocated and this is called to write
     into them from a data source.
    */
    CoronaCommandWriter writer;
} CoronaCommand;

/**
 Permanently register a custom graphics command.
 @param L Lua state pointer.
 @param command Command operations. If `writer` is NULL, data will be `memcpy()`'d.
 @param commandID This is populated with a non-0 ID on success.
 @return If non-0, the command was registered.
*/
CORONA_API
int CoronaRendererRegisterCommand( lua_State * L, const CoronaCommand * command, unsigned long * commandID ) CORONA_PUBLIC_SUFFIX;

/**
 Issue a registered graphics command, adding it to the command buffer being built.
 @param renderer Boxed renderer.
 @param commandID An ID returned by `CoronaRendererRegisterCommand`.
 @param data Data source used by the command's writer.
 @param size Size in bytes to reserve for the command's payload. Used by the command's writer.
 @return If non-0, the command was issued.
*/
CORONA_API
int CoronaRendererIssueCommand( const CoronaRenderer * renderer, unsigned long commandID, void * data, unsigned int size ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

/**
 Get the current base address of the command buffer's working memory.
 During a given command buffer execution, the address is stable among `reader` operations; while
 building the buffer up, on the other hand, it may change from one `writer` to the next, owing to
 reallocations. In the latter case, subtracting the base address from the `to` parameter will give the
 relative offset, which is invariant during buildup and the subsequent execution.
 Both `from` and `to` will point at a command's payload, not its own base address, so the relative
 offsets in these cases will be > 0.
 @param commandBuffer Boxed command buffer.
 @return If non-NULL, the base address.
*/
CORONA_API
const unsigned char * CoronaCommandBufferGetBaseAddress( const CoronaCommandBuffer * commandBuffer ) CORONA_PUBLIC_SUFFIX;

/**
 This structure accounts for different uniform writing needs.
*/
typedef struct CoronaWriteUniformParams {
    union {
        /**
         Memory data source.
        */
        const void * data;
        
        /**
         Offset in command buffer's working memory that holds the data, cf. the details for
         `CoronaCommandBufferGetBaseAddress`.
        */
        unsigned long offset;
    } u;
    
    /**
     If non-0, the data is available through `offset; else `data`.
    */
    int useOffset;
} CoronaWriteUniformParams;

/**
 Attempt to write data into one of the currently bound shader's uniforms. In particular, this is meant to allow
 use of uniforms outside of Solar's dedicated set, including arrays. Since updates are done explicitly through
 this API, changes will persist; any display object using the shader will see these same uniforms, as opposed
 to per-object uniform userdata.
 This sharing does not apply across shader versions or mods.
 If the uniform exists, `Type` is one of `{ float, vec2/3/4, mat2/3/4 }`, and the name is not unusually long, the
 write will proceed. The `Count` is also determined in this process: >= 1 in the case of an array, 1 otherwise.
 Uniform data is interpreted as a `Type` array. On success, `min( Count, size / sizeof( Type ) )` items are written.
 It is meant to be called from a `CoronaCommand` reader.
 Solar's own built-in uniforms are not supported.
 @param commandBuffer Boxed command buffer.
 @param uniformName Name of uniform.
 @param params Uniform write configuration.
 @param size Size of uniform data, in bytes.
 @return If non-0, the write occurred.
*/
CORONA_API
int CoronaCommandBufferWriteNamedUniform( const CoronaCommandBuffer * commandBuffer, const char * uniformName, const CoronaWriteUniformParams * params, unsigned int size ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

/**
 Primitive types used by Solar's vertex geometry.
*/
typedef enum {
    /**
     8-bit unsigned integer.
    */
    kAttributeType_Byte,

    /**
     Single-precision floating point.
    */
    kAttributeType_Float,

    /**
     Number of available types.
    */
    kAttributeType_Count
} CoronaGeometryAttributeType;

/**
 Structured form to allow reads / writes of specific vertex components.
*/
typedef struct CoronaGeometryMappingLayout {
    /**
     Number of primitives (1 to 4) that constitute the value.
    */
    unsigned int count;
    
    /**
     If geometry vertices are the source, the number of bytes from one source value to the next. Otherwise, 0.
    */
    unsigned int inStride;

    /**
     Number of bytes from one destination value to the next.
    */
    unsigned int outStride;

    /**
     Primitive type of value.
    */
    CoronaGeometryAttributeType type;
} CoronaGeometryMappingLayout;

/**
 Operation performed when submitting geometry, used to assign or modify a specific component or attribute.
 The first such value will be pointed at by `dest`, and `index` says which output vertex contains the value
 (typically this will matter to user-provided contexts).
 Starting from `dest`, the layout's `outStride` may be used to step from one vertex to the next, up to the
 `n`-th instance.
 A well-behaved "normal" writer is write-only: `dest` is undefined initially and must be valid afterward.
 Update-style writers, on the other hand, are assumed to already contain valid data and thus may both read
 and write.
 In either case, a writer should restrict itself to the component or attribute it has claimed.
 When a context is supplied with the writer, it will be available as that parameter.
 Otherwise, if `context` is non-`NULL`, the input had associated geometry and it points to the corresponding
 value in the `index`-th input vertex. Similar to `dest`, the layout's `outStride` may be used to iterate
 over these values.
*/
typedef void (*CoronaGeometryComponentWriter)( void * dest, const void * context, const CoronaGeometryMappingLayout * layout, unsigned int index, unsigned int n );

/**
 Append a writer to the renderer's list.
 When rendering, each "normal" writer is called, then any "update" ones (in order), to create the sequence of
 vertices to be submitted. The list will always begin with some built-in behavior, e.g. a writer that copies
 source vertices over directly.
 Writers may be added within a before or after function belonging to a `CoronaShaderDrawParams`, and will remain
 in effect until said function exits.
 A custom writer might be added, say, to repurpose the "z" component for an effect, or supply "texCoord" from an
 alternate data source.
 In the case of "normal" writers, a new "position" writer will supercede any previous one, or "x" writers for
 that matter; similarly for other combinations. If possible, the shadowed functions will not even be called.
 Writers with side effects should be written with this in mind, or avoided altogether.
 Adding a writer does not affect batching.
 @param renderer Boxed renderer.
 @param name One of the following:
  "position", "texCoord", "color", "userData" (full attributes)
  "x", "y", "z" (position components)
  "u", "v", "q" (texCoord components)
  "r", "g", "b", "a" (color components)
  "ux", "uy", "uz", "uw" (userData components)
  TODO?: vertex extensions NYI
 @param writer Writer responsible for creating or updating the named vertex component or attribute.
 @param context Supplied as `context` to writer, cf. `CoronaGeometryComponentWriter`. May be `NULL`.
 @param update If non-0, this is an update-style writer, cf. `CoronaGeometryComponentWriter`.
 @return If non-0, the writer was set.
*/
CORONA_API
int CoronaGeometrySetComponentWriter ( const CoronaRenderer * renderer, const char * name, CoronaGeometryComponentWriter writer, const void * context, int update ) CORONA_PUBLIC_SUFFIX;

/**
 Primitive types that may be used by extended attributes; these extend the set used by Solar's vertices.
*/
typedef enum {
    /**
     Signed 32-bit integer.
    */
    kAttributeType_Int = kAttributeType_Count,

    // TODO: signed / unsigned (short, int); float16 / 32; etc.
    
    /**
     Attempt to keep the underlying type stable yet allow new values.
    */
    kMaxVertexMemberType = 0xFFFF
} CoronaVertexExtensionAttributeType;

/**
 Configuration for a vertex extension attribute.
*/
typedef struct CoronaVertexExtensionAttribute {
    /**
     Name of attribute in shader.
    */
    const char * name;

    /**
     Primitive type of attribute.
    */
    CoronaVertexExtensionAttributeType type;

    /**
     If non-0 and the type is integral, on the GPU side components resolve to
     floating point values in [0, 1] or [-1, +1], according to signedness.
    */
    unsigned char normalized;

    /**
     Number of components, from 1 to 4.
    */
    unsigned char components;

    /**
     If > 1, attributes `name .. 1`, ..., `name .. N` are registered, each with the
     remaining (non-instancing) properties that would normally go to `name`.
     The input is coalesced so that `name .. 1` sees the first value, `name .. 2`
     the second, and so on; when the attribute does advance, the window slides
     forward: `name .. 1` will be on the second value, `name .. 2` the third, etc.
     Given `M` instances, the combined stream will have `max(ceil(M / R), N)`
     elements, `R` being the instances-to-replicate count.
    */
    unsigned short windowSize;
    
    /**
     If > 0, the attribute has instance granularity: in particular, over the course of (at
     most) this many instances, all vertices use value `N`; the next batch then goes
     with value `N + 1`, and so on.
     If 0 but windowed, this is interpreted as 1.
     If not windowed, the attribute's data will comprise `ceil(M / R)` elements, where
     `M` is the instance count and `R` the instances-to-replicate count.
     To query support, call `system.getInfo( "instancingSupport" )`.
    */
    unsigned int instancesToReplicate;
} CoronaVertexExtensionAttribute;

/**
 Configuration for a vertex format that extends Solar's own, for use by effects and geometry.
*/
typedef struct CoronaVertexExtension {
    /**
     Required
     When creating an instance of this type, set this member to `size = sizeof(CoronaVertexExtension)`.
     This is required for identifying the API version used.
    */
    unsigned long size;

	/**
	 If non-0, extension is instanced.
	 This is instancing via a shader ID, and redundant if any attributes also request instancing.
	 To query support, call `system.getInfo( "instancingSupport" )`.
	*/
	int instanceByID;
	
    /**
     Number of extension attributes.
    */
    unsigned int count;
    
    /**
     Additional vertex attributes.
    */
    CoronaVertexExtensionAttribute * attributes;
} CoronaVertexExtension;

/**
 Register a vertex extension, i.e. a type expected by an effect&mdash;and supplied by some
 geometry&mdash;that includes new members as well as those in stock Solar vertices.
 @param L Lua state.
 @param name Unused extension name.
 @param extension Extension configuration.
 @return If non-0, the extension was registered.
*/
CORONA_API
int CoronaGeometryRegisterVertexExtension( lua_State * L, const char * name, const CoronaVertexExtension * extension ) CORONA_PUBLIC_SUFFIX;

/**
 Unregister a vertex extension. @see CoronaGeometryRegisterVertexExtension
 Effects currently using the transform are unaffected.
 @param L Lua state.
 @param name Name of registered extension.
 @return If non-0, the extension was unregistered.
*/
CORONA_API
int CoronaGeometryUnregisterVertexExtension( lua_State * L, const char * name ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

/**
 Read-only details that may be provided to `graphics.defineEffect()`. These are made available
 to shell transform and data type callbacks in particular, in order to allow user-defined tuning.
*/
typedef struct CoronaEffectDetail {
    /**
     Name of some property the effect or transform might understand.
    */
    const char * name;

    /**
     Value to associate with the named property.
    */
    const char * value;
} CoronaEffectDetail;

/**
 Query some user-provided details about the shader's effect.
 @param shader Boxed shader.
 @param index 0-based detail index.
 @param detail On success, populated.
 @return If non-0, the detail was found.
*/
CORONA_API
int CoronaShaderGetEffectDetail( const CoronaShader * shader, int index, CoronaEffectDetail * detail ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

/**
 Any structure that extends `CoronaShellTransformParams` will take this as its first member, to effect C-style inheritance.
*/
typedef struct CoronaShellTransformParamsExtensionHeader {
    /**
     Link to the next extension structure, or `NULL` if this is the last one.
    */
    struct CoronaShellTransformParamsExtensionHeader * next;
    
    /**
     An identifier for the extension structure type.
    */
    unsigned short type;
} CoronaShellTransformParamsExtensionHeader;

/**
 This is populated and then passed to a shell transform callback.
*/
typedef struct CoronaShellTransformParams {
    /**
     One of "vertex" or "fragment".
    */
    const char * type;
    
    /**
     Snippets that will be pieced together, in order, to yield the final shader source. More
     specifically, the contents of the array pointed to by `sources` after the transform
     callback will be used to do so: this is the crux of the "transformation".
     If the original array is left intact, only its "vertexSource" / "fragmentSource" pieces
     may be modified.
     Otherwise, the caller must replace it with a new array, although this may include
     strings from the original.
    */
    const char ** sources;

    /**
     When a callback transform begins, these are one-to-one with `sources`. Each
     string gives a little description of the corresponding source piece, the most
     useful being "vertexSource" / "fragmentSource". (The general idea is to not
     hard-code the indices of any piece.)
    */
    const char ** hints;
    
    /**
     Details as provided to `graphics.defineEffect()`.
    */
    const CoronaEffectDetail * details;
    
    /**
     Arbitrary value provided to `CoronaShellTransform`.
    */
    const void * userData;
    
    /**
     Number of strings in the `sources` and `hints` arrays.
     If the transform changes the number of strings in `sources`, this must be updated to match.
    */
    unsigned int nsources; // n.b. must agree with output after call
    
    /**
     Number of elements in `details` array.
    */
    unsigned int ndetails;
    
    /**
     Not yet used; for possible future expansion.
    */
    CoronaShellTransformParamsExtensionHeader * next;
} CoronaShellTransformParams;

/**
 Callback where the transformation occurs, i.e. an updated list of shader source snippets is compiled.
 The workspace is a user-defined. If a new array and / or strings need to be allocated, for example, it
 can be used to provide some data structure. Similarly, the key can be used to identify the set of
 transformations in progress.
*/
typedef const char ** (*CoronaShellTransformBegin)( CoronaShellTransformParams * params, void * workSpaceData, void * key );

/**
 Callback following transformation, after the strings have been consumed.
 This is meant to allow for workspace cleanup.
*/
typedef void (*CoronaShellTransformFinish)( void * workSpaceData, void * key );

/**
 Callback after all transformations have concluded.
 This is meant for any "final" workspace cleanup.
*/
typedef void (*CoronaShellTransformStateCleanup)( void * key );

/**
 Configuration for shell transform registration.
*/
typedef struct CoronaShellTransform {
    /**
     Required
     When creating an instance of this type, set this member to `size = sizeof(CoronaShellTransform)`.
     This is required for identifying the API version used.
    */
    unsigned long size;

    /**
     Optional
     If non-0, the size of a region to be allocated and provided to each of the transform's callbacks.
    */
    unsigned int workSpace;

    /**
     Required
     Called before compiling a shader, to update its source.
    */
    CoronaShellTransformBegin begin;

    /**
     Optional
     Called after compiling a shader, successful or not, to release resources.
    */
    CoronaShellTransformFinish finish;

    /**
     Optional
     Called after all shader compilations, to clean up master resources.
    */
    CoronaShellTransformStateCleanup cleanup;

    /**
     Optional
     Arbitrary data made available during `begin` callback.
     It is assumed to exist until any relevant shaders have been compiled. Since transforms are
     never unloaded, this could mean indefinitely.
    */
    void * userData;
} CoronaShellTransform;

/**
 Register a shell transform, i.e. a mutation of an effect's shader source before final compilation.
 The "shell" nomenclature is used since the backend's shader shell is what a user typically wants to
 modify--the effect kernels after all, are user-provided, so could be "transformed" already--but is not
 strictly accurate.
 @param L Lua state.
 @param name Unused transform name.
 @param transform Transform configuration.
 @return If non-0, the transform was registered.
*/
CORONA_API
int CoronaShaderRegisterShellTransform( lua_State * L, const char * name, const CoronaShellTransform * transform ) CORONA_PUBLIC_SUFFIX;

/**
 Unregister a shell transform. @see CoronaShaderRegisterShellTransform
 Effects currently using the transform are unaffected.
 @param L Lua state.
 @param name Name of registered transform.
 @return If non-0, the transform was unregistered.
*/
CORONA_API
int CoronaShaderUnregisterShellTransform( lua_State * L, const char * name ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

typedef void (*CoronaShaderDrawBookend)( const CoronaShader * shader, void * userData, const CoronaRenderer * renderer, const CoronaRenderData * renderData );

/**
 This may be used to augment and/or override how a shader draws an object.
 
 With a given method, this can take on the form:
 
 ```
   before( ... )
   original( ... )
   after( ... )
 ```
 
 where all three functions take the same arguments.

 The `before` and `after` functions may be NULL, in which case the respective function is
 not called. Similarly, the stock behavior is skipped if `ignoreOriginal` is non-0.
*/
typedef struct CoronaShaderDrawParams {
    /**
     Optional
     If non-0, skip the regular draw behavior.
    */
    unsigned int ignoreOriginal;
    
    /**
     Optional
     Actions to perform before and / or after the regular draw behavior.
    */
    CoronaShaderDrawBookend before, after;
} CoronaShaderDrawParams;

/**
 Draw an instance of the current render data.
 It is meant to be called from a `CoronaShaderDrawBookend`.
 @param shader Boxed shader.
 @param renderData Boxed render data.
 @param renderer Boxed renderer.
 @return If non-0, the draw was performed.
 */
CORONA_API
int CoronaShaderRawDraw( const CoronaShader * shader, const CoronaRenderData * renderData, const CoronaRenderer * renderer ) CORONA_PUBLIC_SUFFIX;

/**
 Get the version that the shader is prepared to draw.
 @param renderData Boxed render data.
 @param renderer Boxed renderer.
 @param version On success, the shader version.
 @return -1 on error; otherwise, the version.
*/
CORONA_API
int CoronaShaderGetVersion( const CoronaRenderData * renderData, const CoronaRenderer * renderer ) CORONA_PUBLIC_SUFFIX;

/**
 Operation called when a particular mod / version of a shader becomes bound.
 This can happen via a regular draw or during a `CoronaShaderRawDraw()`.
 In this and the many operations that follow, if extra space was requested for the data type, `userData` will point to it.
*/
typedef void (*CoronaShaderBind)( const CoronaRenderer * renderer, void * userData );

/**
 Operation called when a shader is detached from a paint, say to clean up resources.
*/
typedef void (*CoronaShaderDetach)( const CoronaShader * shader, void * userData );

/**
 Operation called before a shader mod is used to draw, say to set up some resources.
*/
typedef void (*CoronaShaderPrepare)( const CoronaShader * shader, void * userData, const CoronaRenderData * renderData, int w, int h, int mod );

/**
 Operation called if `name` is not found in the vertex data / uniform userdata. A return value >= 0 is a "data index",
 cf. `CoronaEffectGetData` and `CoronaEffectSetData`.
*/
typedef int (*CoronaEffectNameToIndex)( const char * name );

/**
 Operation called to return a custom result from an object's effect. It must 1 if it pushed a result, 0 otherwise.
 If `hadError` was 0 (the default), a pushed result is treated as the retrieved data.
 Otherwise, an error message is printed and nothing is returned. If there was a pushed object and it was a
 string, it will be incorporated into the message.
*/
typedef int (*CoronaEffectGetData)( lua_State * L, int dataIndex, void * userData, int * hadError );

/**
 Operation called to assign a custom value to an object's effect.
 If `hadError` was 0 (the default), a non-0 return value indicates an assignment occurred.
 Otherwise, an error message is printed. A non-0 value says an object was pushed onto the stack, and is
 incorporated into the message if it was a string.
*/
typedef int (*CoronaEffectSetData)( lua_State * L, int dataIndex, int valueIndex, void * userData, int * shouldInvalidate, int * hadError );

/**
 Any structure that extends `CoronaEffectCallbacks` will take this as its first member, to effect C-style inheritance.
*/
typedef struct CoronaEffectCallbacksExtensionHeader {
    /**
     Link to the next extension structure, or `NULL` if this is the last one.
    */
    struct CoronaObjectParamsHeader * next;
    
    /**
     An identifier for the extension structure type.
    */
    unsigned short type;
} CoronaEffectCallbacksExtensionHeader;

/**
 Configuration for effect data type.
*/
typedef struct CoronaEffectCallbacks {
    /**
     Required
     When creating an instance of this type, set this member to `size = sizeof(CoronaEffectCallbacks)`.
     This is required for identifying the API version used.
    */
    unsigned long size;

    /**
     Optional
     Called after a shader variant is bound.
    */
    CoronaShaderBind shaderBind;
    
    /**
     Optional
     Called when an effect is detached from an object.
    */
    CoronaShaderDetach shaderDetach;
    
    /**
     Optional
     Called before an effect is used to draw an object.
    */
    CoronaShaderPrepare prepare;

    /**
     Optional
     Used when drawing an object with the effect attached.
    */
    CoronaShaderDrawParams drawParams;

    /**
     Optional
     Called when reading or writing an effect instance with a name not belonging to
     the vertex data / uniform userdata.
    */
    CoronaEffectNameToIndex getDataIndex;

    /**
     Optional
     Called when a custom data index is provided when reading from an effect instance.
    */
    CoronaEffectGetData getData;

    /**
     Optional
     Called when a custom data index is provided when writing to an effect instance.
    */
    CoronaEffectSetData setData;
    
    /**
     Optional
     If > 0, each effect instance will get a userdata with this many bytes.
    */
    unsigned int extraSpace;
    
    /**
     Not yet used; for possible future expansion.
    */
    CoronaEffectCallbacksExtensionHeader * next;
} CoronaEffectCallbacks;

/**
 Register an effect data type, i.e. a set of operations that augment and / or override the stock behaviors
 an effect will perform when attached to a display object.
 @param L Lua state.
 @param name Unused data type.
 @param callbacks Data type callbacks.
 @return If non-0, the data type was registered.
*/
CORONA_API
int CoronaShaderRegisterEffectDataType( lua_State * L, const char * name, const CoronaEffectCallbacks * callbacks ) CORONA_PUBLIC_SUFFIX;

/**
 Unregister an effect data type. @see CoronaShaderRegisterEffectDataType
 Effects currently using the data type are unaffected.
 @param L Lua state.
 @param name Name of registered data type.
 @return If non-0, the data type was unregistered.
*/
CORONA_API
int CoronaShaderUnregisterEffectDataType( lua_State * L, const char * name ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

/**
 A 4-by-4 matrix.
*/
typedef float CoronaMatrix4x4[16];

/**
 Multiply two matrices.
 @param m1 Left-hand matrix.
 @param m2 Right-hand matrix.
 @param result Populated with product.
*/
CORONA_API
void CoronaMultiplyMatrix4x4( const CoronaMatrix4x4 m1, const CoronaMatrix4x4 m2, CoronaMatrix4x4 result ) CORONA_PUBLIC_SUFFIX;

/**
 A 3-component vector, i.e. x, y, z.
*/
typedef float CoronaVector3[3];

/**
 Construct a view matrix.
 @param eye Eye or camera position.
 @param center Center position. A look-at direction is derived from `eye` to `center`.
 @param up Fixed up vector.
 @param result Populated with view matrix.
*/
CORONA_API
void CoronaCreateViewMatrix( const CoronaVector3 eye, const CoronaVector3 center, const CoronaVector3 up, CoronaMatrix4x4 result ) CORONA_PUBLIC_SUFFIX;

/**
 Construct an orthographic projection matrix.
 @param left Distance to vertical clipping plane to the left.
 @param right Distance to vertical clipping plane to the right.
 @param bottom Distance to horizontal clipping plane below.
 @param top Distance to horizontal clipping plane above.
 @param zNear Distance from eye to near clipping plane.
 @param zFar Distance from eye to far clipping plane.
 @param result Populated with projection matrix.
*/
CORONA_API
void CoronaCreateOrthoMatrix( float left, float right, float bottom, float top, float zNear, float zFar, CoronaMatrix4x4 result ) CORONA_PUBLIC_SUFFIX;

/**
 Construct a perspective projection matrix.
 @param fovy The y field-of-view, in degrees.
 @param aspectRatio Ratio of width to height; determines the x field-of-view.
 @param zNear Distance from eye to near clipping plane.
 @param zFar Distance from eye to far clipping plane.
 @param result Populated with projection matrix.
*/
CORONA_API
void CoronaCreatePerspectiveMatrix( float fovy, float aspectRatio, float zNear, float zFar, CoronaMatrix4x4 result ) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

#endif // _CoronaGraphics_H__
