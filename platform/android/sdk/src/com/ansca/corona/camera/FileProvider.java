package com.ansca.corona.camera;





//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


import com.ansca.corona.storage.FileServices;

/**
 * Note:This is a a extends storage.FileContentProvider and used for Camera Storage
 */
public class FileProvider extends com.ansca.corona.storage.FileContentProvider {



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
        localFilePath = "content://" + context.getPackageName() + ".photos" + localFilePath;
        localFilePath = android.net.Uri.encode(localFilePath, ":/\\.");
        return android.net.Uri.parse(localFilePath);
    }
}
