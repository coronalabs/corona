//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "RenderSurfaceProxy.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

#pragma region Consructors/Destructors
RenderSurfaceProxy::RenderSurfaceProxy()
:	fSurface(nullptr),
	fLastOrientation(Corona::WinRT::Interop::RelativeOrientation2D::Unknown),
	fRenderFrameHandler(nullptr)
{
}

#pragma endregion


#pragma region Public Properties/Methods
IRenderSurface^ RenderSurfaceProxy::RenderSurface::get()
{
	return fSurface;
}

void RenderSurfaceProxy::RenderSurface::set(IRenderSurface^ surface)
{
	int lastWidth = 0;
	int lastHeight = 0;
	int newWidth = 0;
	int newHeight = 0;

	// Do not continue if this proxy has already been assigned the given surface reference.
	if (surface == fSurface)
	{
		return;
	}

	// Handle this proxy's last assigned surface.
	if (fSurface)
	{
		// Remove this proxy's event handlers from last assigned surface.
		RemoveEventHandlers();
		fSurface->SetRenderFrameHandler(nullptr);

		// Store the surface's last known orientation.
		fLastOrientation = fSurface->Orientation;

		// Fetch the surface's dimensions.
		lastWidth = fSurface->WidthInPixels;
		lastHeight = fSurface->HeightInPixels;

		// Raise a "LostRenderContext" since the owner of this proxy will no longer be able to render to this surface.
		if (fSurface->IsReadyToRender)
		{
			LostRenderContext(this, CoronaLabs::WinRT::EmptyEventArgs::Instance);
		}
	}

	// Store the given surface reference.
	fSurface = surface;

	// Handle the newly given surface reference, if one was assigned (not null).
	if (fSurface)
	{
		// Add this proxy's event handlers to the newly assigned surface.
		AddEventHandlers();
		fSurface->SetRenderFrameHandler(fRenderFrameHandler);

		// Fetch the new surface's dimensions.
		newWidth = fSurface->WidthInPixels;
		newHeight = fSurface->HeightInPixels;

		// Raise a "ReceivedRenderContext" event if the given surface already has a context.
		// This notifies the owner of this proxy to reload all textures/shaders into this new surface.
		if (fSurface->IsReadyToRender)
		{
			AcquiringRenderContext(this, CoronaLabs::WinRT::EmptyEventArgs::Instance);
			ReceivedRenderContext(this, CoronaLabs::WinRT::EmptyEventArgs::Instance);
		}
	}

	// Raise a "Resized" event if the new surface is a different size than the last surface.
	// Note: A null reference to a surface is considered to have zero width and height.
	if ((newWidth != lastWidth) || (newHeight != lastHeight))
	{
		Resized(this, CoronaLabs::WinRT::EmptyEventArgs::Instance);
	}
}

bool RenderSurfaceProxy::IsReadyToRender::get()
{
	return fSurface ? fSurface->IsReadyToRender : false;
}

int RenderSurfaceProxy::WidthInPixels::get()
{
	return fSurface ? fSurface->WidthInPixels : 0;
}

int RenderSurfaceProxy::HeightInPixels::get()
{
	return fSurface ? fSurface->HeightInPixels : 0;
}

Corona::WinRT::Interop::RelativeOrientation2D^ RenderSurfaceProxy::Orientation::get()
{
	return fSurface ? fSurface->Orientation : fLastOrientation;
}

void RenderSurfaceProxy::RequestRender()
{
	if (fSurface)
	{
		fSurface->RequestRender();
	}
}

void RenderSurfaceProxy::ForceRender()
{
	if (fSurface)
	{
		fSurface->ForceRender();
	}
}

void RenderSurfaceProxy::SetRenderFrameHandler(RenderFrameHandler^ handler)
{
	fRenderFrameHandler = handler;
	if (fSurface)
	{
		fSurface->SetRenderFrameHandler(handler);
	}
}

#pragma endregion


#pragma region Private Methods
void RenderSurfaceProxy::AddEventHandlers()
{
	// Do not continue if a surface has not been assigned.
	if (!fSurface)
	{
		return;
	}

	// Remove the last event handlers if still assigned.
	// Note: This should never be the case.
	RemoveEventHandlers();

	// Add event handlers to this proxy's currently assigned surface.
	fAcquiringRenderContextEventToken = fSurface->AcquiringRenderContext += ref new Windows::Foundation::TypedEventHandler<IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>(this, &RenderSurfaceProxy::OnAcquiringRenderContext);
	fReceivedRenderContextEventToken = fSurface->ReceivedRenderContext += ref new Windows::Foundation::TypedEventHandler<IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>(this, &RenderSurfaceProxy::OnReceivedRenderContext);
	fLostRenderContextEventToken = fSurface->LostRenderContext += ref new Windows::Foundation::TypedEventHandler<IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>(this, &RenderSurfaceProxy::OnLostRenderContext);
	fResizedEventToken = fSurface->Resized += ref new Windows::Foundation::TypedEventHandler<IRenderSurface^, CoronaLabs::WinRT::EmptyEventArgs^>(this, &RenderSurfaceProxy::OnResized);
}

void RenderSurfaceProxy::RemoveEventHandlers()
{
	// Do not continue if a surface has not been assigned.
	if (!fSurface)
	{
		return;
	}

	// Remove all event handlers from this proxy's currently assigned surfacce.
	if (fAcquiringRenderContextEventToken.Value)
	{
		fSurface->AcquiringRenderContext -= fAcquiringRenderContextEventToken;
		fAcquiringRenderContextEventToken.Value = 0;
	}
	if (fReceivedRenderContextEventToken.Value)
	{
		fSurface->ReceivedRenderContext -= fReceivedRenderContextEventToken;
		fReceivedRenderContextEventToken.Value = 0;
	}
	if (fLostRenderContextEventToken.Value)
	{
		fSurface->LostRenderContext -= fLostRenderContextEventToken;
		fLostRenderContextEventToken.Value = 0;
	}
	if (fResizedEventToken.Value)
	{
		fSurface->Resized -= fResizedEventToken;
		fResizedEventToken.Value = 0;
	}
}

void RenderSurfaceProxy::OnAcquiringRenderContext(IRenderSurface ^sender, CoronaLabs::WinRT::EmptyEventArgs ^args)
{
	AcquiringRenderContext(this, args);
}

void RenderSurfaceProxy::OnReceivedRenderContext(IRenderSurface ^sender, CoronaLabs::WinRT::EmptyEventArgs ^args)
{
	ReceivedRenderContext(this, args);
}

void RenderSurfaceProxy::OnLostRenderContext(IRenderSurface ^sender, CoronaLabs::WinRT::EmptyEventArgs ^args)
{
	LostRenderContext(this, args);
}

void RenderSurfaceProxy::OnResized(IRenderSurface ^sender, CoronaLabs::WinRT::EmptyEventArgs ^args)
{
	Resized(this, args);
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
