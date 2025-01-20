//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.version.android8;

import android.media.AudioFormat;
import android.text.InputType;
import com.ansca.corona.version.IAndroidVersionSpecific;


public class AndroidVersionSpecific implements IAndroidVersionSpecific {
	
	public AndroidVersionSpecific() {
	}
	
	public int apiVersion()
	{
		return 8;
	}

	public int audioChannelMono()
    {
    	return AudioFormat.CHANNEL_IN_MONO;
    }

	public int inputTypeFlagsNoSuggestions() {
    	return InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
    }
}
