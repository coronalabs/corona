// ----------------------------------------------------------------------------
// 
// ICoronaControlAdapter.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif

#include "IControlAdapter.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\IRenderSurface.h"

namespace CoronaLabs { namespace Corona { namespace WinRT {
	ref class CoronaRuntime;
} } }


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace UI {

/// <summary>
///  Interface to a Corona control that hosts the 3D rendering surface and is a container for native UI controls.
/// </summary>
[Windows::Foundation::Metadata::WebHostHidden]
public interface class ICoronaControlAdapter : public IControlAdapter
{
	/// <summary>
	///  Gets a reference to the Corona runtime which executes the Corona Lua scripts and renders to the
	///  control's 3D rendering surface.
	/// </summary>
	/// <value>Reference to the Corona runtime used to render to this control.</value>
	property CoronaRuntime^ Runtime { CoronaRuntime^ get(); }

	/// <summary>Gets the 3D rendering surface that the Corona runtime renders to.</summary>
	/// <value>Reference to the control's 3D rendering surface.</value>
	property Graphics::IRenderSurface^ RenderSurface { Graphics::IRenderSurface^ get(); }

#if 0
	/// <summary>Adds and displays the given native control inside of this Corona control.</summary>
	/// <param name="control">
	///  <para>Reference to a native Xaml control to be contained and displayed within this Corona control.</para>
	///  <para>Will be ignored if set to null.</para>
	/// </param>
	void Add(IControl^ control);

	/// <summary>Removes a control once added to this Corona control via the Add() method.</summary>
	/// <param name="control">The control to be removed by reference.</param>
	/// <returns>
	///  <para>Returns true if the control was successfully removed.</para>
	///  <para>Returns false if the given control reference was not found within the Corona control.</para>
	/// </returns>
	bool Remove(IControl^ control);
#endif
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::UI
