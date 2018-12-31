// ----------------------------------------------------------------------------
// 
// IResourceServices.h
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


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Storage {

/// <summary>Provides access to embedded resources, such as Corona's widget library images.</summary>
public interface class IResourceServices
{
	/// <summary>Determines if the given resource name exists as an embedded file within the application's library.</summary>
	/// <param name="resourceName">Unique name of the resource.</param>
	/// <returns>Returns true if the resource name was found and is an embedded file. Returns false if not found.</returns>
	bool ContainsFile(Platform::String^ resourceName);
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Storage
