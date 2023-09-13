//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_Paint.h"

#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_PaintAdapter.h"
#include "Display/Rtt_Shader.h"
#include "Display/Rtt_ShaderData.h"
#include "Display/Rtt_ShaderFactory.h"
#include "Display/Rtt_TextureResource.h"
#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_RenderData.h"
#include "Renderer/Rtt_TextureBitmap.h"
#include "Rtt_LuaUserdataProxy.h"
#include "Rtt_Lua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

	// ----------------------------------------------------------------------------

	void
		Paint::Finalize()
	{

	}

	Paint*
		Paint::NewColor(Rtt_Allocator* cntx, const SharedPtr< TextureResource >& resource, U8 r, U8 g, U8 b, U8 a)
	{
		RGBA rgba = { r, g, b, a };
		ColorUnion* c = (ColorUnion*)(&rgba);
		return NewColor(cntx, resource, c->pixel);
	}

	Paint*
		Paint::NewColor(Rtt_Allocator* cntx, const SharedPtr< TextureResource >& resource, Color c)
	{
		return Rtt_NEW(cntx, Paint(resource, c));
	}

	// If the color values are different or the paint is NULL
	bool
		Paint::ShouldInvalidateColor(const Paint* oldValue, const Paint* newValue)
	{
		return (!oldValue || !newValue
			|| oldValue->GetColor() != newValue->GetColor());
	}

	// ----------------------------------------------------------------------------

	Paint::Paint()
		: fColor(ColorWhite()),
		fBlendMode(),
		fBlendEquation(RenderTypes::kAddEquation),
		fIsPremultiplied(true),
		fType(kColor),
		fDirtyFlags(kDefault),
		fShader(NULL),
		fProxy(NULL),
		fObserver(NULL),
		fResource()
	{
	}

	Paint::Paint(const SharedPtr< TextureResource >& resource)
		: fColor(ColorWhite()),
		fBlendMode(),
		fBlendEquation(RenderTypes::kAddEquation),
		fIsPremultiplied(true),
		fType(kColor),
		fDirtyFlags(kDefault),
		fShader(NULL),
		fProxy(NULL),
		fObserver(NULL),
		fResource(resource)
	{
	}

	void
		Paint::Initialize(Type t)
	{
		fType = t;
	}

	Paint::Paint(const SharedPtr< TextureResource >& resource, Color c)
		: fColor(c),
		fBlendMode(),
		fBlendEquation(RenderTypes::kAddEquation),
		fIsPremultiplied(true),
		fType(kColor),
		fDirtyFlags(kDefault),
		fShader(NULL),
		fProxy(NULL),
		fObserver(NULL),
		fResource(resource)
	{
	}

	Paint::~Paint()
	{
		Rtt_DELETE(fShader);

		if (fProxy)
		{
			GetObserver()->QueueRelease(fProxy); // Release native ref to Lua-side proxy
			fProxy->DetachUserdata(); // Notify proxy that object is invalid
		}
	}

	void
		Paint::UpdatePaint(RenderData& data)
	{
		if (!IsValid(kBlendFlag))
		{
			data.fBlendMode = fBlendMode;
			data.fBlendEquation = (RenderTypes::BlendEquation)fBlendEquation;
			SetValid(kBlendFlag);
		}

		data.fFillTexture0 = GetTexture();

		if (!IsValid(kShaderUniformDataFlag))
		{
			ShaderData* d = (fShader ? fShader->GetData() : NULL);
			if (d)
			{
				data.fUserUniform0 = d->GetUniform(ShaderData::kData0);
				data.fUserUniform1 = d->GetUniform(ShaderData::kData1);
				data.fUserUniform2 = d->GetUniform(ShaderData::kData2);
				data.fUserUniform3 = d->GetUniform(ShaderData::kData3);
			}
			else
			{
				data.fUserUniform0 = NULL;
				data.fUserUniform1 = NULL;
				data.fUserUniform2 = NULL;
				data.fUserUniform3 = NULL;
			}
			SetValid(kShaderUniformDataFlag);
		}
	}

	Texture*
		Paint::GetTexture() const
	{
		Rtt_ASSERT(fResource.NotNull());
		return &fResource->GetTexture();
	}

	inline U8 Scale2(U16 a, U16 b)
	{
		return (U8)((a * b) >> 8U);
	}

	void
		Paint::UpdateColor(RenderData& data, U8 objectAlpha)
	{
		ColorUnion c;
		c.pixel = fColor;

		c.rgba.ModulateAlpha(objectAlpha);

		// Premultiply alpha;
		bool isPremultiplied = IsPremultiplied(); // RenderTypes::BlendModeIsPremultiplied( data.fBlendMode );
		if (isPremultiplied)
		{
			c.rgba.PremultiplyAlpha();
		}

		Real ux = 0, uy = 0, uz = 0, uw = 0;
		bool shouldUpdateShader = (fShader && !IsValid(kShaderVertexDataFlag));
		if (shouldUpdateShader)
		{
			fShader->GetData()->CopyVertexData(ux, uy, uz, uw);
		}

		Geometry* g = data.fGeometry; Rtt_ASSERT(g); // Ensure fGeometry is initialized
		Geometry::Vertex* vertices = g->GetVertexData();

		const U32* perVertexColors = g->GetPerVertexColorData();
		const bool scaleColor = c.pixel != ColorWhite();

		for (U32 i = 0, iMax = g->GetVerticesUsed(); i < iMax; i++)
		{
			Geometry::Vertex& v = vertices[i];

			// Faster way to assign (rs,gs,bs,as)
	//		ColorUnion *dstScale = (ColorUnion *)& v.rs;
	//		dstScale->pixel = pixel;

			if (!perVertexColors)
			{
				v.rs = c.rgba.r;
				v.gs = c.rgba.g;
				v.bs = c.rgba.b;
				v.as = c.rgba.a;
			}

			else
			{
				ColorUnion vc;
				vc.pixel = perVertexColors[i];

				v.rs = Scale2(vc.rgba.r, vc.rgba.a);
				v.gs = Scale2(vc.rgba.g, vc.rgba.a);
				v.bs = Scale2(vc.rgba.b, vc.rgba.a);
				v.as = vc.rgba.a;

				if (scaleColor)
				{
					v.rs = Scale2(v.rs, c.rgba.r);
					v.gs = Scale2(v.gs, c.rgba.g);
					v.bs = Scale2(v.bs, c.rgba.b);
					v.as = Scale2(v.as, c.rgba.a);
				}
			}

			if (shouldUpdateShader)
			{
				v.ux = ux;
				v.uy = uy;
				v.uz = uz;
				v.uw = uw;
			}
		}

		SetValid(kShaderVertexDataFlag);
	}

	void
		Paint::SetColor(Color newValue)
	{
		if (newValue != fColor)
		{
			fColor = newValue;

			InvalidateObserver(kColorFlag);
		}
	}

	void
		Paint::SetBlend(const BlendMode& newValue)
	{
		if (newValue != fBlendMode)
		{
			Invalidate(kBlendFlag);
			fBlendMode = newValue;
		}
	}

	void
		Paint::SetBlend(RenderTypes::BlendType newValue)
	{
		// Verify that only preset blend types are used here.
		Rtt_ASSERT(RenderTypes::kCustom != newValue);

		if (newValue != GetBlend())
		{
			Invalidate(kBlendFlag);

			fBlendMode = BlendMode(newValue, IsPremultiplied());
		}
	}

	RenderTypes::BlendType
		Paint::GetBlend() const
	{
		return RenderTypes::BlendTypeForBlendMode(fBlendMode, IsPremultiplied());
	}

	/*
	void
	Paint::GetBlendParams(
		BlendMode::Param& outSrc, BlendMode::Param& outDst,
		BlendMode::Param& outSrcAlpha, BlendMode::Param& outDstAlpha ) const
	{
		outSrc = (BlendMode::Param)fBlendColorSrc;
		outDst = (BlendMode::Param)fBlendColorDst;
		outSrcAlpha = (BlendMode::Param)fBlendAlphaSrc;
		outDstAlpha = (BlendMode::Param)fBlendAlphaDst;
	}
	*/

	void
		Paint::SetBlendEquation(RenderTypes::BlendEquation newValue)
	{
		if (newValue != fBlendEquation)
		{
			Invalidate(kBlendFlag);
			fBlendEquation = newValue;
		}
	}

	void
		Paint::Translate(Real dx, Real dy)
	{
	}

	const Paint*
		Paint::AsPaint(Type type) const
	{
		return (kColor == type ? this : NULL);
	}

	void
		Paint::ApplyPaintUVTransformations(ArrayVertex2&) const
	{
	}

	bool
		Paint::CanTransform() const
	{
		bool result = false;

		switch (fType)
		{
		case kBitmap:
		case kImageSheet:
		case kGradient:
		case kMultitexture:
		case kCamera:
			result = true;
			break;
		default:
			break;
		}

		return result;
	}

	void
		Paint::UpdateTransform(Transform& t) const
	{
	}

	void
		Paint::InvalidateObserver(RenderFlag mask)
	{
		// Trigger a Prepare() in the DisplayObject
		DisplayObject* observer = GetObserver();
		if (observer)
		{
			// TODO: Make the Paint DirtyFlags the same values
			// as the DisplayObject ones
			DisplayObject::DirtyFlags flags = 0;

			switch (mask)
			{
			case kColorFlag:
				flags = DisplayObject::kColorFlag;
				break;
			case kBlendFlag:
				flags = DisplayObject::kPaintFlag;
				break;
			case kShaderVertexDataFlag:
				flags = DisplayObject::kColorFlag;
				break;
			case kShaderUniformDataFlag:
				flags = DisplayObject::kPaintFlag;
				break;
			case kTextureTransformFlag:
				flags = DisplayObject::kGeometryFlag;
				break;
			default:
				break;
			}

			observer->Invalidate(flags);
		}
	}

	void
		Paint::Invalidate(RenderFlag mask)
	{
		fDirtyFlags |= mask;
		InvalidateObserver(mask);
	}

	void
		Paint::SetShader(Shader* newValue)
	{
		if (newValue != fShader)
		{
			// Default shader does not have data
			if (newValue && newValue->GetData())
			{
				bool usesUniforms = newValue->UsesUniforms();

				// Invalidate paint
				RenderFlag paintFlag = (usesUniforms ? kShaderUniformDataFlag : kShaderVertexDataFlag);
				Invalidate(paintFlag);
			}

			// Ensure uniforms are updated if the previous shader used them
			if (fShader && fShader->UsesUniforms())
			{
				Invalidate(kShaderUniformDataFlag);
			}

			DisplayObject* observer = GetObserver();
			if (observer)
			{
				observer->Invalidate(DisplayObject::kProgramFlag | DisplayObject::kPaintFlag);
			}

			// Delete shader b/c we have a new effect
			Rtt_DELETE(fShader);

			// Assign new shader (or NULL)
			fShader = newValue;

			if (fShader)
			{
				fShader->SetPaint(this);
			}
		}
	}

	const Shader*
		Paint::GetShader(ShaderFactory& factory) const
	{
		Shader* result = fShader;
		if (!result)
		{
			if (kColor == fType)
			{
				result = &factory.GetDefaultColorShader();
			}
			else
			{
				result = &factory.GetDefault();
			}
		}

		return result;
	}

	Shader*
		Paint::GetShader(ShaderFactory& factory)
	{
		return const_cast<Shader*>(static_cast<const Paint&>(*this).GetShader(factory));
	}

	const MLuaUserdataAdapter&
		Paint::GetAdapter() const
	{
		return PaintAdapter::Constant();
	}

	void
		Paint::PushProxy(lua_State* L) const
	{
		if (!fProxy)
		{
			fProxy = LuaUserdataProxy::New(L, const_cast<Self*>(this));
			fProxy->SetAdapter(&GetAdapter());
		}

		fProxy->Push(L);
	}

	void
		Paint::DetachProxy()
	{
		fProxy = NULL;
	}

	/*
	void
	Paint::SetShaderData( ShaderData *newValue )
	{
		if ( fShaderData != newValue )
		{
			Rtt_DELETE( fShaderData );
			fShaderData = newValue;
			fShaderData->SetPaint( this );

			bool usesUniforms = fShaderData->GetShader()->UsesUniforms();

			// Invalidate paint
			RenderFlag paintFlag = ( usesUniforms ? kShaderUniformDataFlag : kShaderVertexDataFlag );
			Invalidate( paintFlag );

			// Invalidate object
			DisplayObject *observer = GetObserver();
			if ( observer )
			{
				DisplayObject::RenderFlag objectFlag =
					( usesUniforms ? DisplayObject::kPaintFlag : DisplayObject::kColorFlag );
				observer->Invalidate( objectFlag );
			}
		}
	}
	*/

	void
		Paint::PushShader(lua_State* L) const
	{
		if (fShader)
		{
			fShader->PushProxy(L);
		}
		else
		{
			lua_pushnil(L);
		}
	}

	void
		Paint::DetachShaderProxy()
	{
		fShader->DetachProxy();
		fShader = NULL;
	}

	PlatformBitmap*
		Paint::GetBitmap() const
	{
		return NULL;
	}

	// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

