//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.version;


public interface IAndroidVersionSpecific {
	public int apiVersion();
	
    public int audioChannelMono();
    
    public int inputTypeFlagsNoSuggestions();
}
