//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Paint_H__
#define _Rtt_Paint_H__

#include "Core/Rtt_Geometry.h"
#include "Renderer/Rtt_RenderTypes.h"
#include "Core/Rtt_SharedPtr.h"
#include "Display/Rtt_DisplayTypes.h"

// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{

class ColorPaint;
class DisplayObject;
class Matrix;
class LuaUserdataProxy;
class MLuaUserdataAdapter;
class PlatformBitmap;
class Shader;
class ShaderData;
class ShaderFactory;
class Texture;
class TextureResource;
class Transform;
struct RenderData;

// ----------------------------------------------------------------------------

// Pure-color paints should also support filter effects. To avoid duplicating
// shaders (filters that sample a texture vs filters that do not), we unify all
// paints to use textures.
//
// Note that this implies that texture coordinates need to be generated, though
// they can simply all be set to the same uv (0,0), as there is no texture transform
// on a single pixel texture.
class Paint
{
	public:
		typedef enum Type
		{
			kColor = 0,
			kBitmap = 1,
			kImageSheet = 2,
			kGradient = 3,
			kMultitexture = 4,
			kCamera = 5,
			
			kNumTypes,
		}
		Type;

		typedef Paint Self;

		enum RenderFlag
		{
			kColorFlag = 0x1,
			kBlendFlag = 0x2,
			kShaderVertexDataFlag = 0x4,
			kShaderUniformDataFlag = 0x8,
			kTextureTransformFlag = 0x10,
			
			kDefault = kBlendFlag,
		};
		typedef U8 DirtyFlags;

	public:
		static void Finalize();

		static Paint* NewColor( Rtt_Allocator* cntx, const SharedPtr< TextureResource >& resource, U8 r, U8 g, U8 b, U8 a );
		static Paint* NewColor( Rtt_Allocator* cntx, const SharedPtr< TextureResource >& resource, Color c );

		static bool ShouldInvalidateColor( const Paint *oldValue, const Paint *newValue );

	protected:
		Paint();
		Paint( const SharedPtr< TextureResource >& resource );
		void Initialize( Type t );

	public:
		Paint( const SharedPtr< TextureResource >& resource, Color c );
		virtual ~Paint();

	public:
		virtual void UpdatePaint( RenderData& data );
		virtual Texture *GetTexture() const;
		void UpdateColor( RenderData& data, U8 objectAlpha );

	public:
		Color GetColor() const { return fColor; }
		void SetColor( Color newValue );

		RGBA GetRGBA() const { return ((ColorUnion*)(& fColor))->rgba; }
		void SetRGBA( RGBA newValue ) { SetColor( ((ColorUnion*)(& newValue))->pixel ); }

		void SetBlend( const BlendMode& newValue );
		void SetBlend( RenderTypes::BlendType newValue );
		RenderTypes::BlendType GetBlend() const;

		void SetBlendEquation( RenderTypes::BlendEquation newValue );
		RenderTypes::BlendEquation GetBlendEquation() const { return (RenderTypes::BlendEquation)fBlendEquation; }

		bool IsPremultiplied() const { return ( !! fIsPremultiplied ); }
		void SetPremultiplied( bool newValue ) { fIsPremultiplied = newValue; }

	public:
		virtual void Translate( Real dx, Real dy );
		virtual const Paint* AsPaint( Type t ) const;

		bool IsType( Type t ) const { return ( fType == t ); }
		bool IsCompatibleType( Type t ) const { return ( NULL != AsPaint( t ) ); }

		bool CanTransform() const;
		virtual void UpdateTransform( Transform& t ) const;
	
		virtual void ApplyPaintUVTransformations( ArrayVertex2& vertices ) const;

	protected:
		void InvalidateObserver( RenderFlag mask );

	public:
		void Invalidate( RenderFlag mask );
		bool IsValid( DirtyFlags flags ) const { return 0 == (fDirtyFlags & flags); }
		void SetValid( DirtyFlags flags ) { fDirtyFlags &= (~flags); }

	public:
		void SetShader( Shader *newValue );
		const Shader *GetShader(ShaderFactory &factory) const;
		Shader *GetShader(ShaderFactory &factory);

	public:
		virtual const MLuaUserdataAdapter& GetAdapter() const;
		void PushProxy( lua_State *L ) const;
		void DetachProxy();

//		void SetShaderData( ShaderData *newValue );
		void PushShader( lua_State *L ) const;
		void DetachShaderProxy();

		DisplayObject *GetObserver() const { return fObserver; }
		void SetObserver( DisplayObject *newValue ) { fObserver = newValue; }

	public:
		virtual PlatformBitmap *GetBitmap() const;

	private:
		Color fColor;
		BlendMode fBlendMode;
		U8 fBlendEquation;
		U8 fIsPremultiplied; // TODO: Move this to a bit mask
		U8 fType;
		U8 fDirtyFlags;
		Shader *fShader;
		mutable LuaUserdataProxy *fProxy;
		DisplayObject *fObserver; // weak ptr
		
	protected:
		SharedPtr< TextureResource > fResource;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Paint_H__
