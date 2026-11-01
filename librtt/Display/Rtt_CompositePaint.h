//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_CompositePaint_H__
#define _Rtt_CompositePaint_H__

#include "Display/Rtt_Paint.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class LengthAccumulator;
struct ExtraTextureInfo;

// ----------------------------------------------------------------------------

class CompositePaint : public Paint
{
	public:
		typedef Paint Super;
		typedef CompositePaint Self;

	public:
		CompositePaint( Paint *paint0, Paint *paint1 );
		virtual ~CompositePaint();

	public:
		void PopulateTextureList( TextureList& list ) const;
	
		virtual void UpdatePaint( RenderData& data );
		virtual Texture *GetTexture() const;

	public:
		virtual const Paint* AsPaint( Type t ) const;
		virtual void ApplyPaintUVTransformations( ArrayVertex2& vertices ) const override;

		void PrepareExtraTextures( U32 count, const LengthAccumulator& names );
		void CommitExtraTextures(); // merge extra textures after populating list
		void ClearExtraInfo();
		U8* GetNameList() const;
		U32 GetExtraCount() const { return fExtraCount; }
		void* GetTextureResourceList() const; // void* = SharedPtr<TextureResource>*
		Texture** GetTexturesList() const;
		
		static const U8* GetNameListGivenTextureList( Texture** list, U32 extraCount );
		static const U32 GetNamesSizeGivenTextureList( Texture** list, U32 extraCount );
//		virtual const MLuaUserdataAdapter& GetAdapter() const;

	private:
		Paint *fPaint0;
		Paint *fPaint1;
		ExtraTextureInfo *fExtraInfo;
		U32 fExtraCount;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_CompositePaint_H__
