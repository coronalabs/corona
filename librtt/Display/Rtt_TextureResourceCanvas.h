//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __ratatouille__Rtt_TextureResourceCanvas__
#define __ratatouille__Rtt_TextureResourceCanvas__


#include "Rtt_TextureResource.h"

namespace Rtt {
class FrameBufferObject;

class TextureResourceCanvas : public TextureResource
{
	public:
		typedef TextureResourceCanvas Self;
		
	public:
		static Self *Create(
							TextureFactory& factory,
							Real w, Real h,
							int texW, int texH,
							Texture::Format format);
		
		virtual ~TextureResourceCanvas();
		
	protected:
		TextureResourceCanvas(
							  TextureFactory &factory,
							  Texture *texture,
							  FrameBufferObject* fbo,
							  GroupObject *cache,
							  GroupObject *qeue,
							  Real width,
							  Real height,
							  int texWidth,
							  int texHeight);

	public:
			
		void SetClearColor(Color c){ fClearColor = c; }
		GroupObject* GetCacheGroup() const {return fGroupCache;}
		GroupObject* GetQueueGroup() const {return fGroupQueue;}
	
		void Invalidate(bool cache, bool clear);
		virtual void Render(Renderer & renderer) override;
		virtual void ReleaseLuaReferences( lua_State *L ) override;

		Real GetContentWidth() const {return fContentWidth;}
		void SetContentWidth(Rtt_Real width) {fContentWidth=width;}

		Real GetContentHeight() const {return fContentHeight;}
		void SetContentHeight(Rtt_Real height) {fContentHeight=height;}

		int GetTexWidth() const {return fTexWidth;}
		int GetTexHeight() const {return fTexHeight;}

		Rtt_Real GetAnchorX() const {return fAnchorX;}
		void SetAnchorX(Rtt_Real anchorX) {fAnchorX=anchorX;}
	
		Rtt_Real GetAnchorY() const {return fAnchorY;}
		void SetAnchorY(Rtt_Real anchorY) {fAnchorY=anchorY;}
	
	protected:
		void Render(Rtt::Renderer &renderer, GroupObject *group, bool clear);

		bool fInvalidateCache, fInvalidateClear;
		Color fClearColor;
		
		GroupObject *fGroupCache, *fGroupQueue;
		FrameBufferObject *fDstFBO;
		
		int fTexWidth, fTexHeight;
		Rtt_Real fContentWidth, fContentHeight;
		Rtt_Real fAnchorX, fAnchorY;
			
	private:
		void MoveQueueToCache();
		virtual const MLuaUserdataAdapter& GetAdapter() const override;

};
	
}
#endif /* defined(__ratatouille__Rtt_TextureResourceCanvas__) */
