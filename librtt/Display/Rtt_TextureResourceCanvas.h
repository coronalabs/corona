//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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
							int texW, int texH);
		
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
