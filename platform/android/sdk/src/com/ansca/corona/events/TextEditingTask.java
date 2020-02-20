//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.events;

import com.ansca.corona.ViewManager;


public class TextEditingTask implements com.ansca.corona.CoronaRuntimeTask {
	private int myEditTextId;
	private int startPos;
	private int numDeleted;
	private String newCharacters;
	private String oldString;
	private String newString;

	public TextEditingTask( int id, int start_pos, int num_deleted, String new_characters, String old_string, String new_string )
	{
		myEditTextId = id;
		startPos = start_pos;
		numDeleted = num_deleted;
		newCharacters = new_characters;
		oldString = old_string;
		newString = new_string;
	}
	
	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		com.ansca.corona.JavaToNativeShim.textEditingEvent( runtime, myEditTextId, startPos, numDeleted, newCharacters, oldString, newString );
	}

}
