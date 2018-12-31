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

#if 0

#ifndef _Rtt_MacSurfaceVideoRecorder_H__
#define _Rtt_MacSurfaceVideoRecorder_H__

#include "Rtt_PlatformSurface.h"

#import <QTKit/QTTime.h>

// ----------------------------------------------------------------------------

@class NSDictionary;
@class QTMovie;

namespace Rtt
{

class Runtime;

// ----------------------------------------------------------------------------

class MacSurfaceVideoRecorder : public PlatformSurfaceDelegate
{
	public:
		MacSurfaceVideoRecorder( Runtime& runtime );
		~MacSurfaceVideoRecorder();

	public:
		virtual void WillFlush( const PlatformSurface& surface ) const;
		virtual void DidFlush( const PlatformSurface& surface ) const;

	public:
		bool StartRecording();
		void StopRecording();

		bool IsRecording() const { return fMovie; }

	private:
		Runtime& fRuntime;
		QTMovie* fMovie;
		U32 fFrameCount;
		QTTime fFrameInterval;
		NSDictionary* fAttributes;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MacSurfaceVideoRecorder_H__
#endif
