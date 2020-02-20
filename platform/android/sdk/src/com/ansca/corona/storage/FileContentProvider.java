//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.storage;


/**
 * This provider allows other applications read-only access to this application's files under its
 * private directory under internal storage, as well as its asset files within the APK's "assets"
 * directory or within Google Play expansion files. This class is needed because other applications
 * do not have permission to access this application's private files for tasks such as attaching files
 * to an e-mail, posting a photo to a social media app, playing a local video file on an external
 * video player, etc.
 * <p>
 * In order for another application to access a file belonging to this application, you must create
 * a "content://" URI which references that file by calling this class' createContentUriForFile() method.
 * The other application will then use this content URI to communicate with this provider to retrieve
 * information about that file and open an input stream to it.
 * <p>
 * If you do not want to expose your application's files to other applications,
 * then you should set the provider's "exported" attribute to false in the "AndroidManifest.xml" file.
 * <p>
 * An instance of this class will be automatically created by the Android OS upon application startup.
 * You should never create an instance of this class yourself.
 */
public class FileContentProvider extends android.content.ContentProvider {
	/**
	 * Called on application startup if registered in the "AndroidManifest.xml" file. Initializes this object.
	 * @return Returns true if the provider was successfully loaded. Returns false if not.
	 */
	@Override
	public boolean onCreate() {
		return true;
	}
	
	/**
	 * Handles a client request to open a file belonging to this application.
	 * @param uri The URI whose file is to be opened.
	 * @param mode Access mode for the file. May be "r" for read-only access, "rw" for read and write access,
	 *             or "rwt" for read and write access that truncates any existing file.
	 * @return Returns a new ParcelFileDescriptor which you can use to access the file.
	 */
	@Override
	public android.os.ParcelFileDescriptor openFile(android.net.Uri uri, String mode)
		throws java.io.FileNotFoundException
	{
		// Validate.
		if (uri == null)
		{
			throw new IllegalArgumentException();
		}

		// Retrieve the requested file and pass the file reference to the calling client/application via a parcel.
		java.io.File dataDir = new java.io.File( getContext().getApplicationInfo().dataDir );
		java.io.File file = new java.io.File(dataDir, uri.getPath());

		boolean validPath = false;
		try
		{
			String rootCanonicalPath = dataDir.getCanonicalPath();
			String fileCanonicalPath = file.getCanonicalPath();
			validPath = fileCanonicalPath.startsWith(rootCanonicalPath);
		}
		catch (java.io.IOException e)
		{
			android.util.Log.e("Corona", "Error while reading canonical file path", e);
		}

		if (!validPath)
		{
			android.util.Log.e("Corona", "Error while getting file path for " + uri.toString());
			throw new IllegalArgumentException();
		}

		return android.os.ParcelFileDescriptor.open(file, android.os.ParcelFileDescriptor.MODE_READ_ONLY);
	}
	
	/**
	 * Handles a client request to open a file belonging to this application via a file descriptor.
	 * @param uri The URI whose file is to be opened.
	 * @param mode Access mode for the file. May be "r" for read-only access, "w" for write-only access
	 *             (erasing whatever data is currently in the file), "wa" for write-only access to append to
	 *             any existing data, "rw" for read and write access on any existing data, and "rwt" for read
	 *             and write access that truncates any existing file.
	 * @return Returns a new AssetFileDescriptor which you can use to access the file.
	 */
	@Override
	public android.content.res.AssetFileDescriptor openAssetFile(android.net.Uri uri, String mode)
		throws java.io.FileNotFoundException
	{
		android.content.res.AssetFileDescriptor descriptor = null;
		
		// Validate.
		if (uri == null)
		{
			throw new IllegalArgumentException();
		}

		// Resolve relative path symbols such as "." and "..".
		// We must do this because apk/zip entry lookups must use absolute paths.
		try {
			java.net.URI uriConverter = java.net.URI.create(uri.toString()).normalize();
			android.net.Uri normalizedUri = android.net.Uri.parse(uriConverter.toString());
			if (normalizedUri != null) {
				uri = normalizedUri;
			}
		}
		catch (Exception ex) { }

		// Fetch a file descriptor for the given file, assuming it is in the APK's assets directory.
		try
		{
			// Convert the URI to a relative path in the assets directory, if possible.
			boolean isAssetFile = true;
			String filePath = uri.getPath();
			String assetsPath = "files/coronaResources/";
			int index = filePath.indexOf(assetsPath);
			if ((index >= 0) && ((index + assetsPath.length()) < filePath.length()))
			{
				filePath = filePath.substring(index + assetsPath.length());
			}
			else
			{
				assetsPath = "android_asset/";
				index = filePath.indexOf(assetsPath);
				if ((index >= 0) && ((index + assetsPath.length()) < filePath.length()))
				{
					filePath = filePath.substring(index + assetsPath.length());
				}
				else
				{
					assetsPath = "/assets/";
					if (filePath.startsWith(assetsPath))
					{
						filePath = filePath.substring(index + assetsPath.length());
					}
					else
					{
						isAssetFile = false;
					}
				}
			}
			
			// Attempt to fetch a file descriptor for the given path.
			// An exception will be thrown here if the file cannot be found.
			if (isAssetFile)
			{
				// This is an asset file. Open it via the AssetManager.
				// Remove the leading slash from the path. The file descriptor won't accept it.
				if (filePath.startsWith(java.io.File.separator))
				{
					filePath = filePath.substring(1);
				}
				FileServices fileServices = new FileServices(getContext());
				descriptor = fileServices.openAssetFileDescriptorFor(filePath);
			}
			else
			{
				// This is an external file. Open it with a ParcelFileDescriptor.
				long length = android.content.res.AssetFileDescriptor.UNKNOWN_LENGTH;
				descriptor = new android.content.res.AssetFileDescriptor(openFile(uri, mode), 0, length);
			}
		}
		catch (Exception ex)
		{
			// File not found in the APK's assets directory.
			// The ContentProvider client should be calling this object's openFile() method next
			// in case the requested file is not in the assets directory.
			throw new java.io.FileNotFoundException();
		}
		return descriptor;
	}
	
	/**
	 * Handles a delete request from a client.
	 * <p>
	 * Not supported by this class. Calling this method will cause an exception to be thrown.
	 * @param uri The full URI to query, including a row ID (if a specific record is requested).
	 * @param selection An optional restriction to apply to rows when deleting.
	 * @return Returns the number of rows affected.
	 */
	@Override
	public int delete(android.net.Uri uri, String selection, String[] selectionArgs) {
		throw new UnsupportedOperationException("Not supported by this provider");
	}
	
	/**
	 * Fetches the mime type of the file that the given URI references.
	 * @param uri The URI to query.
	 * @return Returns a MIME type string for the given URI.
	 *         <p>
	 *         Returns null if of an unknown type.
	 */
	@Override
	public String getType(android.net.Uri uri) {
		FileServices fileServices = new FileServices(getContext());
		return fileServices.getMimeTypeFrom(uri);
	}
	
	/**
	 * Handles an insert request from a client.
	 * <p>
	 * Not supported by this class. Calling this method will cause an exception to be thrown.
	 * @param uri The content:// URI of the insertion request.
	 * @param values A set of column_name/value pairs to add to the database.
	 * @return Returns the URI for the newly inserted item.
	 */
	@Override
	public android.net.Uri insert(android.net.Uri uri, android.content.ContentValues values) {
		throw new UnsupportedOperationException("Not supported by this provider");
	}
	
	/**
	 * Handles query requests from clients.
	 * @param uri The URI to query. This will be the full URI sent by the client; if the client is requesting
	 *            a specific record, the URI will end in a record number that the implementation should parse
	 *            and add to a WHERE or HAVING clause, specifying that _id value.
	 * @param projection The list of columns to put into the cursor. If null all columns are included.
	 * @param selection A selection criteria to apply when filtering rows. If null then all rows are included.
	 * @param selectionArgs You may include ?s in selection, which will be replaced by the values from selectionArgs,
	 *                      in order that they appear in the selection. The values will be bound as Strings.
	 * @param sortOrder How the rows in the cursor should be sorted. If null then the provider is free to define the sort order.
	 * @return Returns a Cursor or null.
	 */
	@Override
	public android.database.Cursor query(
		android.net.Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder)
	{
		// Validate.
		if (uri == null) {
			return null;
		}

		// Resolve relative path symbols such as "." and "..".
		// We must do this because apk/zip entry lookups must use absolute paths.
		try {
			java.net.URI uriConverter = java.net.URI.create(uri.toString()).normalize();
			android.net.Uri normalizedUri = android.net.Uri.parse(uriConverter.toString());
			if (normalizedUri != null) {
				uri = normalizedUri;
			}
		}
		catch (Exception ex) { }

		// Set up the columns to be returned by this query.
		String[] columnNames;
		if ((projection != null) && (projection.length > 0)) {
			// Only provide the columns that were requested.
			columnNames = projection;
		}
		else {
			// Provide all columns.
			// Note: Some Android deveopers claim that the "_id" column is needed by some 3rd party apps.
			//       We have not confirmed this for ourselves, but it doesn't hurt to add it.
			columnNames = new String[] {
				"_id",
				android.provider.MediaStore.MediaColumns.DISPLAY_NAME,
				android.provider.MediaStore.MediaColumns.SIZE,
				android.provider.MediaStore.MediaColumns.MIME_TYPE,
			};
		}

		// Fetch the requested information for the given URI/file.
		Object[] columnData = new Object[columnNames.length];
		for (int index = 0; index < columnNames.length; index++) {
			String name = columnNames[index];
			Object data = null;
			if (name.equals("_id")) {
				data = Integer.valueOf(index);
			}
			else if (name.equals(android.provider.MediaStore.MediaColumns.DISPLAY_NAME) ||
			         name.equals(android.provider.MediaStore.MediaColumns.TITLE)) {
				java.io.File file = new java.io.File(uri.getPath());
				data = file.getName();
			}
			else if (name.equals(android.provider.MediaStore.MediaColumns.SIZE)) {
				android.content.res.AssetFileDescriptor fileDescriptor = null;
				try { fileDescriptor = openAssetFile(uri, "r"); }
				catch (Exception ex) { }
				if (fileDescriptor != null) {
					data = Long.valueOf(fileDescriptor.getLength());
				}
			}
			else if (name.equals(android.provider.MediaStore.MediaColumns.MIME_TYPE)) {
				data = getType(uri);
			}
			columnData[index] = data;
		}

		// Return the requested file information as a single row within a cursor object.
		android.database.MatrixCursor cursor = new android.database.MatrixCursor(columnNames);
		cursor.addRow(columnData);
		return cursor;
	}
	
	/**
	 * Handles an update request from a client.
	 * Not supported by this class. Calling this method will cause an exception to be thrown.
	 * @param uri The URI to query. This can potentially have a record ID if this is an update request for a specific record.
	 * @param values A Bundle mapping from column names to new column values (NULL is a valid value).
	 * @param selection An optional filter to match rows to update.
	 * @return Returns the number of rows affected.
	 */
	@Override
	public int update(
		android.net.Uri uri, android.content.ContentValues values, String selection, String[] selectionArgs)
	{
		throw new UnsupportedOperationException("Not supported by this provider");
	}

	/**
	 * Checks if the AndroidManifest.xml file is correctly configured for this provider.
	 * <p>
	 * Will throw an exception if the manifest is missing this provider or if its "authorities"
	 * attribute is misconfigured.
	 * <p>
	 * This method is only expected to be called by the CoronaActivity class.
	 * @param context Context needed to access this application's manifest settings. Cannot be null.
	 */
	public static void validateManifest(android.content.Context context) {
		android.content.pm.PackageInfo packageInfo = null;
		android.content.pm.ProviderInfo providerInfo = null;

		// Validate argument.
		if (context == null) {
			throw new NullPointerException();
		}

		// Fetch the name of this provider and the old deprecated version of this provider.
		String thisProviderName = FileContentProvider.class.getName();
		String oldProviderName = com.ansca.corona.FileContentProvider.class.getName();

		// Fetch all providers from this application's manifest file.
		try {
			packageInfo = context.getPackageManager().getPackageInfo(
								context.getPackageName(), android.content.pm.PackageManager.GET_PROVIDERS);
		}
		catch (Exception ex) { }

		// Throw an exception if the old deprecated version of this provider was found in the manifest.
		if ((packageInfo != null) && (packageInfo.providers != null)) {
			for (android.content.pm.ProviderInfo nextProviderInfo : packageInfo.providers) {
				if (oldProviderName.equals(nextProviderInfo.name)) {
					String message = "Provider \"" + oldProviderName +
							"\" in the AndroidManifest.xml file has been deprecated. Please change its name to \"" +
							thisProviderName + "\".";
					android.util.Log.i("Corona", message);
					throw new RuntimeException(message);
				}
			}
		}

		// Fetch this provider's information from the manifest.
		if ((packageInfo != null) && (packageInfo.providers != null)) {
			for (android.content.pm.ProviderInfo nextProviderInfo : packageInfo.providers) {
				if (thisProviderName.equals(nextProviderInfo.name)) {
					providerInfo = nextProviderInfo;
					break;
				}
			}
		}

		// Throw an exception if this provider was not found in the manifest file.
		if (providerInfo == null) {
			String message = "Provider \"" + thisProviderName + "\" not found in the AndroidManifest.xml file.";
			android.util.Log.i("Corona", message);
			throw new RuntimeException(message);
		}

		// Throw an exception if this provider's "authorities" attribute is not using this app's package name.
		String expectedAuthorityString = context.getPackageName() + ".files";
		if (expectedAuthorityString.equals(providerInfo.authority) == false) {
			String message = "The AndroidManifest.xml provider \"" + thisProviderName +
						"\" is misconfigured. Please change its 'authorities' attribute value to \"" +
						expectedAuthorityString + "\".";
			android.util.Log.i("Corona", message);
			throw new RuntimeException(message);
		}
	}
	
	/**
	 * Creates a "content://" URI for the given file making it accessible to other applications via this provider.
	 * @param context A reference to this application's context.
	 *                Needed to fetch this application's package name. Cannot be null.
	 * @param file The file to create a content URI for.
	 *             <p>
	 *             If this file is set to a relative path, then this method will treat it as an asset file
	 *             within the APK's "assets" directory or within the Google Play expansion file.
	 *             <p>
	 *             Cannot be null or else an exception will be thrown.
	 * @return Returns a matching "content://" URI for the given file.
	 */
	public static android.net.Uri createContentUriForFile(android.content.Context context, java.io.File file) {
		// Validate.
		if (file == null) {
			throw new IllegalArgumentException();
		}

		// Create a URI for the given file.
		return createContentUriForFile(context, file.getPath());
	}

	/**
	 * Creates a "content://" URI for the given file making it accessible to other applications via this provider.
	 * @param context A reference to this application's context.
	 *                Needed to fetch this application's package name. Cannot be null.
	 * @param filePath A relative or absolute path to a file belonging to this application.
	 *                 <p>
	 *                 If this file is set to a relative path, then this method will treat it as an asset file
	 *                 within the APK's "assets" directory or within the Google Play expansion file.
	 *                 <p>
	 *                 An exception will be thrown if set to null or empty string.
	 * @return Returns a matching "content://" URI for the given file.
	 */
	public static android.net.Uri createContentUriForFile(android.content.Context context, String filePath) {
		// Validate.
		if ((context == null) || (filePath == null) || (filePath.length() <= 0)) {
			throw new IllegalArgumentException();
		}
		
		// Get a relative path for the given file.
		boolean isAssetFile = true;
		String localFilePath = filePath;
		String absolutePathPrefix = context.getApplicationInfo().dataDir;
		int index = filePath.indexOf(absolutePathPrefix);
		if ((index >= 0) && ((index + absolutePathPrefix.length()) < filePath.length())) {
			localFilePath = filePath.substring(index + absolutePathPrefix.length());
			isAssetFile = false;
		}
		else {
			absolutePathPrefix = "file:///android_asset";
			index = filePath.indexOf(absolutePathPrefix);
			if ((index >= 0) && ((index + absolutePathPrefix.length()) < filePath.length())) {
				localFilePath = filePath.substring(index + absolutePathPrefix.length());
			}
		}
		if (localFilePath.startsWith(java.io.File.separator) == false) {
			localFilePath = java.io.File.separator + localFilePath;
		}
		if (isAssetFile) {
			localFilePath = java.io.File.separator + "assets" + localFilePath;
		}
		
		// Set up a URI to access the given file via this application's FileContentProvider class.
		localFilePath = "content://" + context.getPackageName() + ".files" + localFilePath;
		localFilePath = android.net.Uri.encode(localFilePath, ":/\\.");
		return android.net.Uri.parse(localFilePath);
	}
}
