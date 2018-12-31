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

package com.ansca.corona;


/**
 * @deprecated This class has been replaced by the {@link com.ansca.corona.storage.FileContentProvider} class.
 *             <p>
 *             This class has been left here so that older Corona Enterprise built apps will not crash
 *             on startup and instead display a developer warning dialog instructing them to change their
 *             "AndroidManifest.xml" file to use the new FileContentProvider class. This class no longer
 *             provides access to the application's files, making this change manifest change necessary.
 */
@Deprecated
public class FileContentProvider extends android.content.ContentProvider {
	@Override
	public boolean onCreate() {
		return false;
	}

	@Override
	public android.net.Uri insert(android.net.Uri uri, android.content.ContentValues values) {
		throw new UnsupportedOperationException("Not supported by this provider");
	}

	@Override
	public int update(
		android.net.Uri uri, android.content.ContentValues values, String selection, String[] selectionArgs)
	{
		throw new UnsupportedOperationException("Not supported by this provider");
	}

	@Override
	public int delete(android.net.Uri uri, String selection, String[] selectionArgs) {
		throw new UnsupportedOperationException("Not supported by this provider");
	}

	@Override
	public android.database.Cursor query(
		android.net.Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder)
	{
		throw new UnsupportedOperationException("Not supported by this provider");
	}

	@Override
	public String getType(android.net.Uri uri) {
		throw new UnsupportedOperationException("Not supported by this provider");
	}
}
