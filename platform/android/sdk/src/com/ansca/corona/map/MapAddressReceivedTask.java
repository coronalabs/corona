//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.maps;


public class MapAddressReceivedTask implements com.ansca.corona.CoronaRuntimeTask {
	private String fStreet;
	private String fStreetDetails;
	private String fCity;
	private String fCityDetails;
	private String fRegion;
	private String fRegionDetails;
	private String fPostalCode;
	private String fCountry;
	private String fCountryCode;

	
	public MapAddressReceivedTask(
		String street, String streetDetails, String city, String cityDetails, String region, String regionDetails,
		String postalCode, String country, String countryCode)
	{
		fStreet = street;
		fStreetDetails = streetDetails;
		fCity = city;
		fCityDetails = cityDetails;
		fRegion = region;
		fRegionDetails = regionDetails;
		fPostalCode = postalCode;
		fCountry = country;
		fCountryCode = countryCode;
	}
	
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		com.ansca.corona.JavaToNativeShim.mapAddressReceivedEvent(
				runtime, fStreet, fStreetDetails, fCity, fCityDetails, fRegion, fRegionDetails,
				fPostalCode, fCountry, fCountryCode);
	}
}
