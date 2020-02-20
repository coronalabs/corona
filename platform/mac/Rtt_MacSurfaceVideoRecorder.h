//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
