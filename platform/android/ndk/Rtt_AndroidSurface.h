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


#ifndef _Rtt_AndroidSurface_H__
#define _Rtt_AndroidSurface_H__

#include "librtt/Rtt_PlatformSurface.h"

#include "librtt/Rtt_GPU.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

class GLView;

// ----------------------------------------------------------------------------

// this may be mac specific
/*
class AndroidSurfaceDelegate : public PlatformSurfaceDelegate
{
	public:
		virtual void WillFlush( const AndroidSurface& surface ) const;
		virtual void DidFlush( const AndroidSurface& surface ) const;
};
*/

class AndroidSurface : public PlatformSurface
{
    Rtt_CLASS_NO_COPIES( AndroidSurface )

	public:
		AndroidSurface( GLView* view );

	public:
		virtual ~AndroidSurface();

	public:
		virtual void SetCurrent() const;
		virtual void Flush() const;

	private:
        GLView* fView;
};

// this is probably unused
/*
class AndroidOffscreenSurface : public PlatformSurface
{
    Rtt_CLASS_NO_COPIES( AndroidOffscreenSurface )

    public:
        AndroidOffscreenSurface( const PlatformSurface& parent );
        virtual ~AndroidOffscreenSurface();

    public:
        virtual void SetCurrent() const;
        virtual void Flush() const;
        virtual const SurfaceProperties& Properties() const;

    private:
};
*/

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidSurface_H__
