//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
