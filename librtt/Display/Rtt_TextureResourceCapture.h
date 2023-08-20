//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __ratatouille__Rtt_TextureResourceCapture__
#define __ratatouille__Rtt_TextureResourceCapture__


#include "Rtt_TextureResource.h"

namespace Rtt {
class FrameBufferObject;

class TextureResourceCapture : public TextureResource
{
	public:
		typedef TextureResourceCapture Self;
		
	public:
		static Self *Create(
							TextureFactory& factory,
							Real w, Real h,
							int texW, int texH);
		
		virtual ~TextureResourceCapture();
		
	protected:
		TextureResourceCapture(
							  TextureFactory &factory,
							  Texture *texture,
							  FrameBufferObject* fbo,
							  Real width,
							  Real height,
							  int texWidth,
							  int texHeight);

	public:
		Real GetContentWidth() const {return fContentWidth;}
		void SetContentWidth(Rtt_Real width) {fContentWidth=width;}

		Real GetContentHeight() const {return fContentHeight;}
		void SetContentHeight(Rtt_Real height) {fContentHeight=height;}

		int GetTexWidth() const {return fTexWidth;}
		int GetTexHeight() const {return fTexHeight;}
	
	public:
		FrameBufferObject* GetFBO() const { return fDstFBO; }
	
	public:
		WeakPtr<TextureResource> & GetWeakResource() { return fWeakResource; }
		void SetWeakResource( const SharedPtr<TextureResource> & resource ) { fWeakResource = resource; }
	
	protected:
		FrameBufferObject *fDstFBO;
		
		int fTexWidth, fTexHeight;
		Rtt_Real fContentWidth, fContentHeight;
			
	private:
		virtual const MLuaUserdataAdapter& GetAdapter() const override;
		WeakPtr<TextureResource> fWeakResource;
};
	
}
#endif /* defined(__ratatouille__Rtt_TextureResourceCapture__) */
