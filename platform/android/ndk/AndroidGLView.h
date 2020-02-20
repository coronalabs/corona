//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _AndroidGLView_H__
#define _AndroidGLView_H__

#include "librtt/Rtt_DeviceOrientation.h"

// ----------------------------------------------------------------------------

namespace Rtt
{
	class Runtime;
}	

class NativeToJavaBridge;

// ----------------------------------------------------------------------------

class AndroidGLView {
private:

	Rtt::Runtime * fRuntime;
	int fWidth;
	int fHeight;
	int fUprightWidth;
	int fUprightHeight;
	bool fReloadTextures;
	NativeToJavaBridge *fNativeToJavaBridge;
	
	Rtt::DeviceOrientation::Type fOrientation;


	
public:
	AndroidGLView()
	{
		fRuntime = NULL;
		fWidth = 0; // TODO: this is not right
		fHeight = 0;
		fUprightWidth = 0;
		fUprightHeight = 0;
		fReloadTextures = false;
		fOrientation = Rtt::DeviceOrientation::kUpright;
		fNativeToJavaBridge = NULL;
	}
	
	void SetCallback( Rtt::Runtime *c )
	{
		fRuntime = c;
	}

	void SetNativeToJavaBridge( NativeToJavaBridge *ntjb )
	{
		fNativeToJavaBridge = ntjb;
	}
	
	void Flush();
	bool CreateFramebuffer( int width, int height, Rtt::DeviceOrientation::Type orientation );
	void DestroyFramebuffer();
	void Render();
	bool HasAlphaChannel();

	int Width() const
	{
		return fWidth;
	}

	int Height() const
	{
		return fHeight;
	}

	// Returns physical width and height of device screen when device
	// is oriented in the upright position (NOTE: upright does not necessarily
	// mean portrait, e.g. tablets)
	int DeviceWidth() const { return fUprightWidth; }
	int DeviceHeight() const { return fUprightHeight; }

	void Resize( int width, int height );
	
	void ReinitializeRenderingStream();
	
	Rtt::DeviceOrientation::Type GetOrientation() const
	{
		return fOrientation;
	}

	void SetOrientation( Rtt::DeviceOrientation::Type orientation )
	{
		fOrientation = orientation;
	}
};

// ----------------------------------------------------------------------------
	
#endif
