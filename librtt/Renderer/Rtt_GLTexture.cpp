//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Config.h"

#include "Renderer/Rtt_GLTexture.h"

#include "Renderer/Rtt_GL.h"
#include "Renderer/Rtt_Texture.h"
#include "Renderer/Rtt_Renderer.h"
#include "Core/Rtt_Assert.h"

#include "Rtt_Profiling.h"

#include "Corona/CoronaGraphics.h"

// ----------------------------------------------------------------------------

#define ENABLE_DEBUG_PRINT    0

#if ENABLE_DEBUG_PRINT
    #define DEBUG_PRINT( ... ) Rtt_LogException( __VA_ARGS__ );
#else
    #define DEBUG_PRINT( ... )
#endif


#if !defined(Rtt_OPENGLES) && !defined(GL_ABGR_EXT)
#define GL_ABGR_EXT 0x8000
#endif
// ----------------------------------------------------------------------------

namespace /*anonymous*/
{
    using namespace Rtt;

    void getFormatTokens( Texture::Format format, GLint& internalFormat, GLenum& sourceFormat, GLenum& sourceType, bool hasRed )
    {
        switch( format.GetValue() )
        {
            case Texture::kAlpha:        internalFormat = GL_ALPHA;        sourceFormat = GL_ALPHA;        sourceType = GL_UNSIGNED_BYTE; break;
#if defined( Rtt_MetalANGLE)
            case Texture::kLuminance:   internalFormat = GL_RED_EXT;    sourceFormat = GL_RED_EXT;        sourceType = GL_UNSIGNED_BYTE; break;
#else
            case Texture::kLuminance: {
				internalFormat = hasRed ? 0x8229 : GL_LUMINANCE;
				sourceFormat = hasRed ? 0x1903 : GL_LUMINANCE;
				sourceType = GL_UNSIGNED_BYTE;
			}	break;
#endif
            case Texture::kRGB:            internalFormat = GL_RGB;        sourceFormat = GL_RGB;            sourceType = GL_UNSIGNED_BYTE; break;
            case Texture::kRGBA:        internalFormat = GL_RGBA;        sourceFormat = GL_RGBA;            sourceType = GL_UNSIGNED_BYTE; break;
#if defined( Rtt_WIN_PHONE_ENV )
            case Texture::kBGRA:        internalFormat = GL_BGRA_EXT;    sourceFormat = GL_BGRA_EXT;        sourceType = GL_UNSIGNED_BYTE; break;
#elif !defined( Rtt_OPENGLES )
            case Texture::kARGB:        internalFormat = GL_RGBA8;        sourceFormat = GL_BGRA;            sourceType = GL_UNSIGNED_INT_8_8_8_8_REV; break;
            case Texture::kBGRA:        internalFormat = GL_RGBA8;        sourceFormat = GL_BGRA;            sourceType = GL_UNSIGNED_INT_8_8_8_8; break;
            #ifdef GL_ABGR_EXT
            case Texture::kABGR:
                internalFormat = GL_ABGR_EXT;
                sourceFormat = GL_ABGR_EXT;
                sourceType = GL_UNSIGNED_BYTE;
                break;
            #endif
#else
            // NOTE: These are not available on OpenGL-ES
            // case Texture::kARGB:        internalFormat = GL_RGBA;        sourceFormat = GL_RGBA;            sourceType = GL_UNSIGNED_BYTE; break;
            // case Texture::kBGRA:        internalFormat = GL_RGBA;        sourceFormat = GL_RGBA;            sourceType = GL_UNSIGNED_BYTE; break;
            // case Texture::kABGR:        internalFormat = GL_RGBA;        sourceFormat = GL_RGBA;            sourceType = GL_UNSIGNED_BYTE; break;
#endif
#ifdef Rtt_NXS_ENV
            case Texture::kLuminanceAlpha:        internalFormat = GL_LUMINANCE_ALPHA;    sourceFormat = GL_LUMINANCE_ALPHA;        sourceType = GL_UNSIGNED_BYTE; break;
#endif
            default: Rtt_ASSERT_NOT_REACHED();
        }
    }

    void getFilterTokens( Texture::Filter filter, GLenum& minFilter, GLenum& magFilter )
    {
        switch( filter )
        {
            case Texture::kNearest:    minFilter = GL_NEAREST;    magFilter = GL_NEAREST;    break;
            case Texture::kLinear:    minFilter = GL_LINEAR;    magFilter = GL_LINEAR;    break;
            default: Rtt_ASSERT_NOT_REACHED();
        }
    }

    GLenum convertWrapToken( Texture::Wrap wrap )
    {
        GLenum result = GL_CLAMP_TO_EDGE;

        switch( wrap )
        {
            case Texture::kClampToEdge:        result = GL_CLAMP_TO_EDGE; break;
            case Texture::kRepeat:            result = GL_REPEAT; break;
            case Texture::kMirroredRepeat:    result = GL_MIRRORED_REPEAT; break;
            default: Rtt_ASSERT_NOT_REACHED();
        }

        return result;
    }
}

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

GLint CalculateOptimalAlignment(U32 width, GLenum format)
{
    int bytesPerPixel = 4;
    
    switch(format) {
        case GL_ALPHA:
#if defined( Rtt_MetalANGLE)
        case GL_RED_EXT:
#else
        case GL_LUMINANCE:
        case 0x8229:
#endif
            bytesPerPixel = 1;
            break;
            
        case GL_RGB:
            bytesPerPixel = 3;
            break;
            
        
        #if defined(GL_ABGR_EXT)
        case GL_ABGR_EXT:
        #endif
        case GL_RGBA:
            bytesPerPixel = 4;
            break;
            
#ifdef Rtt_NXS_ENV
        case GL_LUMINANCE_ALPHA:
            bytesPerPixel = 2;
            break;
#endif
        default: // Default
            bytesPerPixel = 4;
            break;
    }

    const U32 rowBytes = width * bytesPerPixel;
   
    if(rowBytes % 8 == 0) return 8;
    if(rowBytes % 4 == 0) return 4;
    if(rowBytes % 2 == 0) return 2;
    
    return 1; // No alignment
}

static GLint
CalculateAlignementFromBytesPerPixel(U32 width, int bytesPerPixel)
{
    const U32 rowBytes = width * bytesPerPixel;
   
    if(rowBytes % 8 == 0) return 8;
    if(rowBytes % 4 == 0) return 4;
    if(rowBytes % 2 == 0) return 2;
    
    return 1; // No alignment
}

void
GLTexture::Create( CPUResource* resource, const RenderContext* context )
{
    Rtt_ASSERT( CPUResource::kTexture == resource->GetType() || CPUResource::kVideoTexture == resource->GetType() );
    Texture* texture = static_cast<Texture*>( resource );

    SUMMED_TIMING( gltc, "Texture GPU Resource: Create" );

    GLuint name = 0;
    glGenTextures( 1, &name );
    fHandle = NameToHandle( name );
    GL_CHECK_ERROR();

    TextureFormatDescription texDesc;
    Texture::Format textureFormat = texture->GetFormat();
    bool isNonCore = textureFormat.IsNonCore();
    bool mightHaveLinearFiltering = true;
    if ( isNonCore )
    {
		U32 formatIndex = FormatDetails::GetFormatIndex( textureFormat.GetBackingValue() );
		Rtt_ASSERT( formatIndex <= context->fCustomFormatCount );
		texDesc = context->fCustomFormats[ formatIndex - 1 ];

		if ( 0 == ( texDesc.fFlags & TextureFormatDescription::kHasLinearFiltering ) )
		{
			mightHaveLinearFiltering = false;
		}
    }
    
    GLenum minFilter;
    GLenum magFilter;
    if ( mightHaveLinearFiltering )
    {
		getFilterTokens( texture->GetFilter(), minFilter, magFilter );
	}
	else
	{
		minFilter = GL_NEAREST;
		magFilter = GL_NEAREST;
	}
// SAS TDOO: generalize target...
    glBindTexture( GL_TEXTURE_2D, name );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter );
    GL_CHECK_ERROR();

    GLenum wrapS = convertWrapToken( texture->GetWrapX() );
    GLenum wrapT = convertWrapToken( texture->GetWrapY() );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT );
    GL_CHECK_ERROR();

    GLint internalFormat;
    GLenum format;
    GLenum type;
    if ( !isNonCore )
    {
		getFormatTokens( textureFormat, internalFormat, format, type, context->fHasRed );
	}
	else
	{
		internalFormat = texDesc.fInternal;
		format = texDesc.fFormat;
		type = texDesc.fDataType;
	}

    const U32 w = texture->GetWidth();
    const U32 h = texture->GetHeight();
    const U8* data = texture->GetData();
    {
//#if defined( Rtt_EMSCRIPTEN_ENV )
//        glPixelStorei( GL_UNPACK_ALIGNMENT, texture->GetByteAlignment() );
//        GL_CHECK_ERROR();
//#endif
		if ( !isNonCore )
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, CalculateOptimalAlignment(w, internalFormat));
		}
		else if ( !texDesc.IsCompressed() ) // some digging suggest this is irrelevant when compressed
		{
			size_t bytesPerPixel = FormatDetails::GetSize( w, 1, textureFormat.GetBackingValue() );
			glPixelStorei(GL_UNPACK_ALIGNMENT, CalculateAlignementFromBytesPerPixel(w, bytesPerPixel));
		}
        GL_CHECK_ERROR();

        // It is valid to pass a NULL pointer, so allocation is done either way
		if ( isNonCore && texDesc.IsCompressed() )
		{
			U32 imageSize = Texture::Format::GetCompressedSize( w, h, texDesc.fBlockWidth, texDesc.fBlockHeight, texDesc.fBlockSize );
			glCompressedTexImage2D( GL_TEXTURE_2D, 0, internalFormat, w, h, 0, imageSize, data );
		}
		else
		{
			glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, data );
		}
        GL_CHECK_ERROR();
        
        fCachedFormat = internalFormat;
        fCachedWidth = w;
        fCachedHeight = h;
    }
    texture->ReleaseData();

    DEBUG_PRINT( "%s : OpenGL name: %d\n",
                    Rtt_FUNCTION,
                    name );
}

void
GLTexture::Update( CPUResource* resource, const RenderContext* context )
{
    Rtt_ASSERT( CPUResource::kTexture == resource->GetType() );
    Texture* texture = static_cast<Texture*>( resource );

    SUMMED_TIMING( gltu, "Texture GPU Resource: Update" );

    const U8* data = texture->GetData();
    if( data )
    {
        const U32 w = texture->GetWidth();
        const U32 h = texture->GetHeight();
        GLint internalFormat;
        GLenum format;
        GLenum type;
		TextureFormatDescription texDesc;
		Texture::Format textureFormat = texture->GetFormat();
		bool isNonCore = textureFormat.IsNonCore();
		if ( !isNonCore )
		{
			getFormatTokens( texture->GetFormat(), internalFormat, format, type, context->fHasRed );
		}
		else
		{
			U32 formatIndex = FormatDetails::GetFormatIndex( textureFormat.GetBackingValue() );
			Rtt_ASSERT( formatIndex <= context->fCustomFormatCount );
			texDesc = context->fCustomFormats[ formatIndex - 1 ];
			internalFormat = texDesc.fInternal;
			format = texDesc.fFormat;
			type = texDesc.fDataType;
		}

// SAS TODO: generalize target

        glBindTexture( GL_TEXTURE_2D, GetName() );

		if ( !isNonCore )
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, CalculateOptimalAlignment(w, internalFormat));
		}
		else if ( !texDesc.IsCompressed() ) // some digging suggest this is irrelevant when compressed
		{
			size_t bytesPerPixel = FormatDetails::GetSize( w, 1, textureFormat.GetBackingValue() );
			glPixelStorei(GL_UNPACK_ALIGNMENT, CalculateAlignementFromBytesPerPixel(w, bytesPerPixel));
		}
        GL_CHECK_ERROR();

        if ( internalFormat == fCachedFormat && w == fCachedWidth && h == fCachedHeight )
        {
			if ( isNonCore && texDesc.IsCompressed() )
			{
				U32 imageSize = Texture::Format::GetCompressedSize( w, h, texDesc.fBlockWidth, texDesc.fBlockHeight, texDesc.fBlockSize );
				glCompressedTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, w, h, internalFormat, imageSize, data );
			}
			else
			{
				glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, w, h, format, type, data );
			}
        }
        else
        {
			if ( isNonCore && texDesc.IsCompressed() )
			{
				U32 imageSize = Texture::Format::GetCompressedSize( w, h, texDesc.fBlockWidth, texDesc.fBlockHeight, texDesc.fBlockSize );
				glCompressedTexImage2D( GL_TEXTURE_2D, 0, internalFormat, w, h, 0, imageSize, data );
			}
			else
			{
				glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, data );
			}
            fCachedFormat = internalFormat;
            fCachedWidth = w;
            fCachedHeight = h;
        }
        GL_CHECK_ERROR();
    }
    texture->ReleaseData();
}

void
GLTexture::Destroy()
{
    GLuint name = GetName();
    if ( 0 != name )
    {
        glDeleteTextures( 1, &name );
        GL_CHECK_ERROR();
        fHandle = 0;
    }

    DEBUG_PRINT( "%s : OpenGL name: %d\n",
                    Rtt_FUNCTION,
                    name );
}

void
GLTexture::Bind( U32 unit )
{
    glActiveTexture( GL_TEXTURE0 + unit );
    glBindTexture( GL_TEXTURE_2D, GetName() );
// ^^^ SAS TODO: allow other targets... U32 can very easily accommodate a (target | unit) pair,
// or just break up into two parameters
    GL_CHECK_ERROR();
}

GLuint
GLTexture::GetName()
{
    return HandleToName( fHandle );
}

// ----------------------------------------------------------------------------

struct ProbeRAII {
	ProbeRAII( const CoronaTextureFormatDetails* details )
	{
		while ( GL_NO_ERROR != glGetError() ) { /* empty */ } // eat any errors
		
		glGenTextures( 1, &fTexture );
		GL_CHECK_ERROR();

		const GLsizei kDim = 1;

		glBindTexture( GL_TEXTURE_2D, fTexture );
		glTexImage2D( GL_TEXTURE_2D, 0, details->internalFormat, kDim, kDim, 0, details->format, details->type, NULL );

		GLenum err = glGetError();
		fOK = ( GL_NO_ERROR == err );
		
		if ( !fOK )
		{
			Rtt_TRACE_SIM((
				"ERROR: (%u): defining format with internal = %u, source = %u, type = %u",
				err,
				details->internalFormat,
				details->format,
				details->type
			));
		}
	}

	~ProbeRAII()
	{
		glDeleteTextures( 1, &fTexture );
	}

	bool CheckRenderability( GLenum attachment = GL_COLOR_ATTACHMENT0 )
	{
		GLuint fbo;
		glGenFramebuffers( 1, &fbo );
		GL_CHECK_ERROR();

		// https://community.khronos.org/t/depth-only-fbo-incomplete-draw-buffer/65283/4
	#if !defined(Rtt_OPENGLES)
		if ( GL_COLOR_ATTACHMENT0 != attachment )
		{
			glDrawBuffer( GL_NONE );
			GL_CHECK_ERROR();
			glReadBuffer( GL_NONE );
			GL_CHECK_ERROR();
		}
	#endif

		glBindFramebuffer( GL_FRAMEBUFFER, fbo );
		GL_CHECK_ERROR();
		glFramebufferTexture2D( GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, fTexture, 0 );
		GL_CHECK_ERROR();
		GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
		GL_CHECK_ERROR();

		glDeleteFramebuffers( 1, &fbo );
		GL_CHECK_ERROR();

		return ( GL_FRAMEBUFFER_COMPLETE == status );
	}
	
	GLuint fTexture;
	bool fOK;
};

static bool
CheckMaybeColorRenderable( const CoronaTextureFormatDetails* details, U8 inputKind, U8 family )
{
	if ( (U8)details->inputKind != inputKind )
	{
		Rtt_TRACE_SIM(( "ERROR: unknown input kind (%i)\n", details->inputKind ));
		return false;
	}

	if ( (U8)details->family != family )
	{
		Rtt_TRACE_SIM(( "ERROR: unknown family (%i)\n", details->family ));
		return false;
	}

	const U16 renderabilityFlags = ( kTextureFormatFlag_ProbeRenderability | kTextureFormatFlag_IsRenderable1 );
	if ( ( details->flags & renderabilityFlags ) == renderabilityFlags )
	{
		Rtt_TRACE_SIM(( "ERROR: %s\n", "Mixing renderability probe with assertion (#1)" ));
		return false;
	}
	
	return true;
}

static bool
MatchStandardFormat( const CoronaTextureFormatDetails* details, TextureFormatDescription* desc )
{
	U8 inputKind = details->inputKind & TextureFormatDescription::kInputKindsMask;
	U8 family = details->family & TextureFormatDescription::kFamiliesMask;
	
	if ( !CheckMaybeColorRenderable( details, inputKind, family ) )
	{
		return false;
	}

	ProbeRAII probe( details );
	if ( !probe.fOK )
	{
		return false;
	}
	
	desc->fInternal = details->internalFormat;
	desc->fDataType = details->type;
	desc->fFormat = details->format;

	struct {
		int to, from;
	} pairs[] = {
		TextureFormatDescription::kHasLinearFiltering, kTextureFormatFlag_HasLinearFiltering,
		TextureFormatDescription::kIsRenderable1, kTextureFormatFlag_IsRenderable1,
	};
	
	for ( auto && p : pairs )
	{
		if ( details->flags & p.from )
		{
			desc->fFlags |= p.to;
		}
	}

	if ( details->flags & kTextureFormatFlag_ProbeRenderability )
	{
		bool canRender = probe.CheckRenderability();
		if ( canRender )
		{
			desc->fFlags |= TextureFormatDescription::kIsRenderable1;
		}
	}
	
	// n.b. component info already set

	desc->fInputInfo |= inputKind << TextureFormatDescription::kInputKindsShift;
	desc->fInputInfo |= family << TextureFormatDescription::kFamiliesShift;

	return true;
}

static bool
MatchWordPackedFormat( const CoronaTextureFormatDetails* details, TextureFormatDescription* desc )
{
	U8 inputKind = details->inputKind & TextureFormatDescription::kInputKindsMask;
	U8 family = details->family & TextureFormatDescription::kFamiliesMask;
	
	if ( !CheckMaybeColorRenderable( details, inputKind, family ) )
	{
		return false;
	}
	
	int zi = 0, n = 0;
	for ( ; zi < 4 && 0 != desc->fSizes[zi]; zi++)
	{
		n += desc->fSizes[zi];
	}

	if ( ( n % 8 != 0 ) || ( 24 == n ) || ( n > 32 ) )
	{
		Rtt_TRACE_SIM(( "Word-packed layout has %i total bits (must be 8, 16, or 32", n ));
		return false;
	}

	if ( zi < 4 )
	{
		if ( zi < 2 )
		{
			Rtt_TRACE_SIM(( "Too few non-zero components in word-packed layout: %i", zi ));
			return false;
		}

		for (int i = zi + 1; i < 4; i++)
		{
			if ( 0 != desc->fSizes[i] )
			{
				Rtt_TRACE_SIM(( "Non-trailing zero in word-packed layout at position %i", zi ));
				return false;
			}
		}
	}
	
	ProbeRAII probe( details );
	if ( !probe.fOK )
	{
		return false;
	}

	desc->fInternal = details->internalFormat;
	desc->fDataType = details->type;
	desc->fFormat = details->format;

	struct {
		int to, from;
	} pairs[] = {
		TextureFormatDescription::kHasLinearFiltering, kTextureFormatFlag_HasLinearFiltering,
		TextureFormatDescription::kIsRenderable1, kTextureFormatFlag_IsRenderable1
	};
	
	for ( auto && p : pairs )
	{
		if ( details->flags & p.from )
		{
			desc->fFlags |= p.to;
		}
	}

	if ( details->flags & kTextureFormatFlag_ProbeRenderability )
	{
		bool canRender = probe.CheckRenderability();
		if ( canRender )
		{
			desc->fFlags |= TextureFormatDescription::kIsRenderable1;
		}
	}

	// n.b. flag, sizes already set

	desc->fInputInfo |= inputKind << TextureFormatDescription::kInputKindsShift;
	desc->fInputInfo |= family << TextureFormatDescription::kFamiliesShift;

	return true;
}

static bool
AuxDoCompressedFormat( GLenum internalFormat, TextureFormatDescription* desc, int w, int h, int blockSize )
{
	GLuint tex;
	glGenTextures( 1, &tex );
	GL_CHECK_ERROR();

	glBindTexture( GL_TEXTURE_2D, tex );
	GL_CHECK_ERROR();
	
	glCompressedTexImage2D( GL_TEXTURE_2D, 0, internalFormat, w, h, 0, blockSize, NULL );
	GLenum err = glGetError();

	glDeleteTextures( 1, &tex );

	if ( GL_NO_ERROR != err )
	{
		Rtt_TRACE_SIM((
			"ERROR: (%u): defining compressed format with internal = %u",
			err,
			internalFormat ));
		return false;
	}

	desc->fInternal = internalFormat;

	desc->fBlockWidth = (U8)w;
	desc->fBlockHeight = (U8)h;
	desc->fBlockSize = (U8)blockSize; // overwrites bogus size
	
	return true;
}

static bool
MatchCompressedFormat( const CoronaCompressedTextureFormatDetails* details, TextureFormatDescription* desc )
{
	if ( 0 != details->depth )
	{
		Rtt_TRACE_SIM((
			"WARNING: depth %u ignored; no such formats supported yet",
			details->depth ));
	}

	U8 family = details->family & TextureFormatDescription::kFamiliesMask;

	if ( (U8)details->family != family )
	{
		Rtt_TRACE_SIM(( "ERROR: unknown family (%i)\n", details->family ));
		return false;
	}

	int w = 4, h = 4;
	bool has16Bytes = 0 != details->has16Bytes;
	if ( 0 != details->width || 0 != details->height )
	{
		bool isValid = -1 != Texture::Format::BlockDimsID( (U8)details->width, (U8)details->height );
		if ( isValid )
		{
			w = details->width;
			h = details->height;
			has16Bytes = true;
		}
		else
		{
			Rtt_TRACE_SIM((
				"ERROR: (%u, %u) are not recognized block dimensions",
				details->width, details->height ));
			return false;
		}
	}

	desc->fInputInfo |= family << TextureFormatDescription::kFamiliesShift;
	
	return AuxDoCompressedFormat( details->internalFormat, desc, w, h, has16Bytes ? 16 : 8 );
}

static bool
MatchDepthStencilFormat( /*const CoronaDepthStencilTextureFormat* texDef, */TextureFormatDescription* desc )
{
/*
texDef->common.type;
texDef->common.flags;
texDef->common.format;
texDef->common.internalFormat;
	texDef->depthBits;
	texDef->isDepthFloat;
	texDef->stencilBits;
// TODO: ^^^ haven't done anything with this yet
*/
//	if ( 0 != texDef->depthBits && 0 != texDef->stencilBits && ( texDef->common.flags & kProbeRenderability ) )
	{
		// TODO: renderablity1, *2
	}

	return false;
}

bool
Renderer::MatchToFormatDescription( TextureFormatDescription* desc, const void* data )
{
// TODO: a lot of this isn't GL-specific, outside the probes...
	if ( desc->IsWordPacked() )
	{
		return MatchWordPackedFormat( (const CoronaTextureFormatDetails*)data, desc );
	}
	else if ( desc->IsCompressed() )
	{
		return MatchCompressedFormat( (const CoronaCompressedTextureFormatDetails*)data, desc );
	}
	else
	{
		// Rtt_ASSERT( !desc->IsDepthStencil() );
		return MatchStandardFormat( (const CoronaTextureFormatDetails*)data, desc );
		//	return MatchDepthStencilFormat( (CoronaDepthStencilTextureFormat*)texDef, desc );
	}

	return false;
}

void
Renderer::DetectOneComponentTextureFormatSupport()
{
	const char *version = reinterpret_cast<const char *>( glGetString( GL_VERSION ) );

	fIsES2 = ( strstr( version, "OpenGL ES 2" ) == version ) || ( strstr( version, "WebGL 1" ) == version );

#ifdef GL_RED
	const GLenum kRed = GL_RED, kR8 = GL_R8;
#elif defined( GL_RED_EXT )
	const GLenum kRed = GL_RED_EXT, kR8 = GL_R8_EXT;
#else
	error Missing RED component defines
#endif

	CoronaTextureFormatDetails details = {};
    TextureFormatDescription desc = {};
    
    details.type = GL_UNSIGNED_BYTE;
    details.format = kRed;
    details.internalFormat = fIsES2 ? details.format : kR8;
    
    if ( MatchToFormatDescription( &desc, &details ) )
    {
		fHas8BitRed = true;
    }
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
