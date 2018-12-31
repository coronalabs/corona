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

#include "Rtt_IPhoneGLVideoTexture.h"

#include "Renderer/Rtt_GL.h"
#include "Renderer/Rtt_VideoSource.h"
#include "Renderer/Rtt_VideoTexture.h"
#include "Core/Rtt_Assert.h"

#import <AVFoundation/AVFoundation.h>
#import <UIKit/UIDevice.h>
#import <UIKit/UIScreen.h>

// ----------------------------------------------------------------------------

@interface AVCaptureController : NSObject< AVCaptureVideoDataOutputSampleBufferDelegate >
{
	Rtt::IPhoneGLVideoTexture *_owner;
    AVCaptureSession *_session;
    CVOpenGLESTextureCacheRef _videoTextureCache;    

    NSString *_sessionPreset;
    size_t _textureWidth;
    size_t _textureHeight;
    EAGLContext *_context;

	CGFloat _screenWidth;
	CGFloat _screenHeight;
	
	CVImageBufferRef _pixelBuffer;
	size_t _width;
	size_t _height;
	
	AVCaptureDevicePosition _capturePosition;
}

- (id)initWithOwner:(Rtt::IPhoneGLVideoTexture *)owner devicePosition:(AVCaptureDevicePosition) position;
- (void)changeDevicePosition:(AVCaptureDevicePosition) devicePosition;
- (void)setup;
- (void)tearDown;
- (void)cleanUpTextures;
- (void)bufferToTexture;

@end

@implementation AVCaptureController

- (id)initWithOwner:(Rtt::IPhoneGLVideoTexture *)owner devicePosition:(AVCaptureDevicePosition) position
{
	self = [super init];
	if ( self )
	{
		_owner = owner;
		_session = nil;
		_videoTextureCache = NULL;
		_context = nil;
		_pixelBuffer = NULL;
		_capturePosition = position;

		[self updateSessionPreset];

		_screenWidth = [UIScreen mainScreen].bounds.size.width;
		_screenHeight = [UIScreen mainScreen].bounds.size.height;
		
	}
	return self;
}
- (void)updateSessionPreset
{
	if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad &&
		AVCaptureDevicePositionFront != _capturePosition )
	{
		// Choosing bigger preset for bigger screen.
		_sessionPreset = AVCaptureSessionPreset1280x720;
	}
	else
	{
		_sessionPreset = AVCaptureSessionPreset640x480;
	}
}

- (void)changeDevicePosition:(AVCaptureDevicePosition) devicePosition
{
	if(_capturePosition != devicePosition)
	{
		[self tearDown];
		
		_capturePosition = devicePosition;
		//_screenWidth = [UIScreen mainScreen].bounds.size.width;
		//_screenHeight = [UIScreen mainScreen].bounds.size.height;
		
		[self updateSessionPreset];
		[self setup];
	}
}

//TODO - if we need to set the orientation for the front facing/back facing camera, we could subscribe to notifications
//then change the device connection video orientation setup here
//- (void)orientationChanged:(NSNotification *)notification
//{
//	UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];
//
//	if (orientation == UIInterfaceOrientationPortrait || orientation == UIInterfaceOrientationPortraitUpsideDown)
//	{
//		//load the portrait view
//	}
//	else if (orientation == UIInterfaceOrientationLandscapeLeft || orientation == UIInterfaceOrientationLandscapeRight)
//	{
//		//load the landscape view
//	}
//}

- (void)setup
{

	//TODO - if we wish to change the image orientation, subscribe to the notifications here, see comment above
	//[[NSNotificationCenter defaultCenter] addObserver:self
	//	selector:@selector(orientationChanged:)  name:UIDeviceOrientationDidChangeNotification  object:nil];


	Rtt_ASSERT( nil == _context );
	_context = [EAGLContext currentContext];

	// Create CVOpenGLESTextureCacheRef for optimal CVImageBufferRef to GLES texture conversion.
	CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, _context, NULL, &_videoTextureCache);

	if (err)
	{
		NSLog(@"Error at CVOpenGLESTextureCacheCreate %d", err);
		return;
	}

	// Setup Capture Session.
	_session = [[AVCaptureSession alloc] init];
	[_session beginConfiguration];

	// Set preset session size.
	[_session setSessionPreset:_sessionPreset];

	// Create a video device and input from that Device.  Add the input to the capture session.
	AVCaptureDevice * videoDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
	
	NSArray *videoDevices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
	
	for (AVCaptureDevice *device in videoDevices)
	{
		if (device.position == _capturePosition)
		{
			videoDevice = device;
			break;
		}
	}

	// Add the device to the session.
	NSError *error = nil;
	AVCaptureDeviceInput *input = [AVCaptureDeviceInput deviceInputWithDevice:videoDevice error:&error];
	if ( ! error )
	{
		[_session addInput:input];

		// Create the output for the capture session.
		AVCaptureVideoDataOutput * dataOutput = [[AVCaptureVideoDataOutput alloc] init];
		[dataOutput setAlwaysDiscardsLateVideoFrames:YES]; // Probably want to set this to NO when recording

		// Set to YUV420.
		[dataOutput setVideoSettings:[NSDictionary dictionaryWithObject:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA] 
					forKey:(id)kCVPixelBufferPixelFormatTypeKey]]; // Necessary for manual preview

		// Set dispatch to be on the main thread so OpenGL can do things with the data
		[dataOutput setSampleBufferDelegate:self queue:dispatch_get_main_queue()];        

		[_session addOutput:dataOutput];
		[_session commitConfiguration];
		
		
		//For front facing camera in portrait mode we must mirror and set the video orientation to landscapeRight
		if ( AVCaptureDevicePositionFront == _capturePosition )
		{
			for ( AVCaptureConnection *connection in dataOutput.connections )
			{
				[connection setVideoMirrored:YES];
				[connection setVideoOrientation:AVCaptureVideoOrientationLandscapeRight];
				//[connection setVideoOrientation:AVCaptureVideoOrientationPortraitUpsideDown];
			}
		}
	

		[_session startRunning];
	}
}
- (void)tearDown
{
	NSArray *outputs = _session.outputs;
	for ( AVCaptureOutput *o in outputs )
	{
		[_session removeOutput:o];
	}

	[self cleanUpTextures];

	CFRelease(_videoTextureCache);
	
	_context = nil;
}

- (void)cleanUpTextures
{    	
	if (_pixelBuffer)
	{
		CVBufferRelease(_pixelBuffer);
		_pixelBuffer = NULL;
	}

	// Periodic texture cache flush every frame
	CVOpenGLESTextureCacheFlush(_videoTextureCache, 0);
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput 
		didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer 
		fromConnection:(AVCaptureConnection *)connection
{
	Rtt_ASSERT( [EAGLContext currentContext] == _context );

	[self cleanUpTextures];
	
	_pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
	CVBufferRetain(_pixelBuffer);

	_width = CVPixelBufferGetWidth(_pixelBuffer);
	_height = CVPixelBufferGetHeight(_pixelBuffer);

	if (!_videoTextureCache)
	{
		NSLog(@"No video texture cache");
		return;
	}
}

- (void)bufferToTexture
{
	if (_pixelBuffer)
	{
		CVPixelBufferLockBaseAddress(_pixelBuffer, 0);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)_width, (GLsizei)_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, CVPixelBufferGetBaseAddress(_pixelBuffer));
		
		CVPixelBufferUnlockBaseAddress(_pixelBuffer, 0);
	}
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AVCapture
{
	public:
		AVCapture( IPhoneGLVideoTexture& owner, AVCaptureDevicePosition devicePosition );
		~AVCapture();
	public:
		AVCaptureController *GetController() const { return fController; }
		static AVCaptureDevicePosition GetDevicePosition( VideoSource source);

	private:
		AVCaptureController *fController;
};

AVCapture::AVCapture( IPhoneGLVideoTexture& owner, AVCaptureDevicePosition position )
:	fController( [[AVCaptureController alloc] initWithOwner:&owner devicePosition:position ] )
{
}

AVCapture::~AVCapture()
{
	[fController release];
}

AVCaptureDevicePosition AVCapture::GetDevicePosition( VideoSource source)
{
	AVCaptureDevicePosition result = AVCaptureDevicePositionBack;
	if ( kCameraFront == source )
	{
		result = AVCaptureDevicePositionFront;
	}
	return result;
}

// ----------------------------------------------------------------------------
IPhoneGLVideoTexture::IPhoneGLVideoTexture()
:fAVCapture(NULL)
{
}

IPhoneGLVideoTexture::~IPhoneGLVideoTexture()
{
	delete fAVCapture;
}

void 
IPhoneGLVideoTexture::Create( CPUResource* resource )
{

	if (!fAVCapture)
	{
		VideoTexture *video = (VideoTexture *)resource;
		VideoSource source = video->GetSource();
		AVCaptureDevicePosition devicePosition = AVCapture::GetDevicePosition( source );
	
		fAVCapture = new AVCapture( * this, devicePosition);

	}

	Super::Create(resource);
	
	Rtt_ASSERT( CPUResource::kVideoTexture == resource->GetType() );

	AVCaptureController *controller = fAVCapture->GetController();
	
	[controller setup];
}

void 
IPhoneGLVideoTexture::Update( CPUResource* resource )
{
	VideoTexture *video = (VideoTexture *)resource;

	VideoSource source = video->GetSource();

	// Do something to AVCaptureController
	
	AVCaptureDevicePosition devicePosition = AVCapture::GetDevicePosition( source );
	AVCaptureController *controller = fAVCapture->GetController();
	
	[controller changeDevicePosition:devicePosition];
}

void 
IPhoneGLVideoTexture::Destroy()
{
	AVCaptureController *controller = fAVCapture->GetController();
	[controller tearDown];
}

void 
IPhoneGLVideoTexture::Bind( U32 unit )
{
	Super::Bind(unit);

	AVCaptureController *controller = fAVCapture->GetController();
	[controller bufferToTexture];
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
