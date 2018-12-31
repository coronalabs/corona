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

#include "Core/Rtt_Build.h"

#include "Rtt_MacDeviceIdentifier.h"

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetController.h>

// =============================================================================
// Based on http://developer.apple.com/library/mac/#technotes/tn/tn1103.html
// =============================================================================

static kern_return_t FindPrimaryEthernetInterfaces(io_iterator_t *matchingServices);
static kern_return_t GetMACAddress(io_iterator_t intfIterator, UInt8 *MACAddress, UInt8 bufferSize);

// Returns an iterator containing the primary (built-in) Ethernet interface. The caller is responsible for
// releasing the iterator after the caller is done with it.
static kern_return_t
FindPrimaryEthernetInterfaces(io_iterator_t *matchingServices)
{
	kern_return_t kernResult; 
	CFMutableDictionaryRef matchingDict;
	CFMutableDictionaryRef propertyMatchDict;

	// Ethernet interfaces are instances of class kIOEthernetInterfaceClass. 
	// IOServiceMatching is a convenience function to create a dictionary with the key kIOProviderClassKey and 
	// the specified value.
	matchingDict = IOServiceMatching(kIOEthernetInterfaceClass);

	// Note that another option here would be:
	// matchingDict = IOBSDMatching("en0");

	if (NULL == matchingDict)
	{
		Rtt_ERROR( ("IOServiceMatching returned a NULL dictionary.\n") );
	}
	else
	{
		// Each IONetworkInterface object has a Boolean property with the key kIOPrimaryInterface. Only the
		// primary (built-in) interface has this property set to TRUE.

		// IOServiceGetMatchingServices uses the default matching criteria defined by IOService. This considers
		// only the following properties plus any family-specific matching in this order of precedence 
		// (see IOService::passiveMatch):
		//
		// kIOProviderClassKey (IOServiceMatching)
		// kIONameMatchKey (IOServiceNameMatching)
		// kIOPropertyMatchKey
		// kIOPathMatchKey
		// kIOMatchedServiceCountKey
		// family-specific matching
		// kIOBSDNameKey (IOBSDNameMatching)
		// kIOLocationMatchKey

		// The IONetworkingFamily does not define any family-specific matching. This means that in            
		// order to have IOServiceGetMatchingServices consider the kIOPrimaryInterface property, we must
		// add that property to a separate dictionary and then add that to our matching dictionary
		// specifying kIOPropertyMatchKey.

		propertyMatchDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
														&kCFTypeDictionaryKeyCallBacks,
														&kCFTypeDictionaryValueCallBacks);

		if (NULL == propertyMatchDict)
		{
			Rtt_ERROR( ("CFDictionaryCreateMutable returned a NULL dictionary.\n") );
		}
		else
		{
			// Set the value in the dictionary of the property with the given key, or add the key 
			// to the dictionary if it doesn't exist. This call retains the value object passed in.
			CFDictionarySetValue(propertyMatchDict, CFSTR(kIOPrimaryInterface), kCFBooleanTrue); 

			// Now add the dictionary containing the matching value for kIOPrimaryInterface to our main
			// matching dictionary. This call will retain propertyMatchDict, so we can release our reference 
			// on propertyMatchDict after adding it to matchingDict.
			CFDictionarySetValue(matchingDict, CFSTR(kIOPropertyMatchKey), propertyMatchDict);
			CFRelease(propertyMatchDict);
		}
	}

	// IOServiceGetMatchingServices retains the returned iterator, so release the iterator when we're done with it.
	// IOServiceGetMatchingServices also consumes a reference on the matching dictionary so we don't need to release
	// the dictionary explicitly.
	kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, matchingServices);    
	if (KERN_SUCCESS != kernResult)
	{
		Rtt_ERROR( ("IOServiceGetMatchingServices returned 0x%08x\n", kernResult) );
	}

	return kernResult;
}

static kern_return_t
FindAllEthernetInterfaces(io_iterator_t *matchingServices)
{
	kern_return_t kernResult = KERN_FAILURE;
	CFMutableDictionaryRef matchingDict;

	// Ethernet interfaces are instances of class kIOEthernetInterfaceClass. 
	// IOServiceMatching is a convenience function to create a dictionary with the key kIOProviderClassKey and 
	// the specified value.
	matchingDict = IOServiceMatching(kIOEthernetInterfaceClass);
	if (NULL == matchingDict)
	{
		Rtt_ERROR( ("FindAllEthernetInterfaces() IOServiceMatching returned a NULL dictionary.\n") );
	}
	else
	{
		// IOServiceGetMatchingServices retains the returned iterator, so release the iterator when we're done with it.
		// IOServiceGetMatchingServices also consumes a reference on the matching dictionary so we don't need to release
		// the dictionary explicitly.
		kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, matchingServices);    
		if (KERN_SUCCESS != kernResult)
		{
			Rtt_ERROR( ("FindAllEthernetInterfaces() IOServiceGetMatchingServices returned 0x%08x\n", kernResult) );
		}
	}

	return kernResult;
}

// Given an iterator across a set of Ethernet interfaces, return the MAC address of the last one.
// If no interfaces are found the MAC address is set to an empty string.
// In this sample the iterator should contain just the primary interface.
static kern_return_t
GetMACAddress(io_iterator_t intfIterator, UInt8 *MACAddress, UInt8 bufferSize)
{
	io_object_t intfService;
	io_object_t controllerService;
	kern_return_t kernResult = KERN_FAILURE;

	// Make sure the caller provided enough buffer space. Protect against buffer overflow problems.
	if (bufferSize < kIOEthernetAddressSize)
	{
		return kernResult;
	}

	// Initialize the returned address
	bzero(MACAddress, bufferSize);

	// IOIteratorNext retains the returned object, so release it when we're done with it.
	while ((intfService = IOIteratorNext(intfIterator)))
	{
		CFTypeRef MACAddressAsCFData;        

		// IONetworkControllers can't be found directly by the IOServiceGetMatchingServices call, 
		// since they are hardware nubs and do not participate in driver matching. In other words,
		// registerService() is never called on them. So we've found the IONetworkInterface and will 
		// get its parent controller by asking for it specifically.

		// IORegistryEntryGetParentEntry retains the returned object, so release it when we're done with it.
		kernResult = IORegistryEntryGetParentEntry(intfService,
													kIOServicePlane,
													&controllerService);

		if (KERN_SUCCESS != kernResult)
		{
			Rtt_ERROR( ("IORegistryEntryGetParentEntry returned 0x%08x\n", kernResult) );
		}
		else
		{
			// Retrieve the MAC address property from the I/O Registry in the form of a CFData
			MACAddressAsCFData = IORegistryEntryCreateCFProperty(controllerService,
																	CFSTR(kIOMACAddress),
																	kCFAllocatorDefault,
																	0);
			if (MACAddressAsCFData)
			{
				//CFShow(MACAddressAsCFData); // for display purposes only; output goes to stderr

				// Get the raw bytes of the MAC address from the CFData
				CFDataGetBytes(MACAddressAsCFData, CFRangeMake(0, kIOEthernetAddressSize), MACAddress);
				CFRelease(MACAddressAsCFData);
			}

			// Done with the parent Ethernet controller object so we release it.
			(void) IOObjectRelease(controllerService);
		}

		// Done with the Ethernet interface object so we release it.
		(void) IOObjectRelease(intfService);
	}

	return kernResult;
}

int
Rtt_GetDeviceIdentifier( U8 MACAddress[Rtt_EthernetAddressSize] )
{
	Rtt_STATIC_ASSERT( kIOEthernetAddressSize == Rtt_EthernetAddressSize );

	kern_return_t kernResult = KERN_SUCCESS; // on PowerPC this is an int (4 bytes)
	/*
	*	error number layout as follows (see mach/error.h and IOKit/IOReturn.h):
	*
	*	hi		 		       lo
	*	| system(6) | subsystem(12) | code(14) |
	*/

	io_iterator_t intfIterator;
	// UInt8 MACAddress[kIOEthernetAddressSize];

	// Attempt to get the primary ethernet interface
	kernResult = FindPrimaryEthernetInterfaces(&intfIterator);
	if ( KERN_SUCCESS == kernResult )
	{
		kernResult = GetMACAddress(intfIterator, MACAddress, Rtt_EthernetAddressSize);
	}

	// If failed to get the primary, then get all ethernet interfaces
	if ( KERN_SUCCESS != kernResult )
	{
		(void) IOObjectRelease(intfIterator);	// Release the iterator.

		kernResult = FindAllEthernetInterfaces(&intfIterator);
		if ( KERN_SUCCESS == kernResult )
		{
			// Fetch the last interface if more than one (arbitrary that we choose the last one)
			kernResult = GetMACAddress(intfIterator, MACAddress, Rtt_EthernetAddressSize);

			if (KERN_SUCCESS != kernResult)
			{
				Rtt_ERROR( ("GetMACAddress returned 0x%08x\n", kernResult) );
			}
		}
		else
		{
			Rtt_ERROR( ("FindAllEthernetInterfaces returned 0x%08x\n", kernResult) );
		}
	}

	(void) IOObjectRelease(intfIterator);	// Release the iterator.

	return KERN_SUCCESS == kernResult;
}
