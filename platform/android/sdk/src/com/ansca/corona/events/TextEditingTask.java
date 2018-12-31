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
