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

package com.ansca.corona.storage;


/**
 * Indicates where an asset can be found such as the package file that contains it, the byte offset
 * within the package file the asset starts at, the byte count within the package, etc.
 * <p>
 * This information can be used to directly access an asset within an APK or Google Play expansion file
 * via a ZipFile class or, if the asset is not compressed, access the byte stream directly for fast access.
 */
public class AssetFileLocationInfo {
	/** Stores information about the asset file's location within a package file. */
	private AssetFileLocationInfo.Settings fSettings;


	/**
	 * Creates a new immutable information object about an asset file's location within a package file.
	 * @param settings Provides information about the asset's location.
	 *                 Will be cloned by this constructor.
	 *                 <p>
	 *                 Cannot be null or else an exception will be thrown.
	 */
	public AssetFileLocationInfo(AssetFileLocationInfo.Settings settings) {
		// Validate.
		if (settings == null) {
			throw new NullPointerException();
		}
		if ((settings.getPackageFile() == null) ||
		    (settings.getAssetFilePath() == null) ||
		    (settings.getAssetFilePath().length() <= 0) ||
		    (settings.getZipEntryName() == null) ||
		    (settings.getZipEntryName().length() <= 0))
		{
			throw new IllegalArgumentException();
		}

		// Clone the given settings and store it.
		fSettings = settings.clone();
	}

	/**
	 * Gets the package file which contains the asset.
	 * @return Returns the path to the package file which contains the asset.
	 *         This is typically the path to the APK file or the Google Play expansion file.
	 */
	public java.io.File getPackageFile() {
		return fSettings.getPackageFile();
	}

	/**
	 * Gets the asset's relative path and file name.
	 * <p>
	 * Note that the asset's path is really a virtual path within the package file.
	 * The path is expected to be relative to the APK's "asssets" directory or relative to the
	 * Google Play expansion file's root directory.
	 * @return Returns the asset's relative path and file name.
	 */
	public String getAssetFilePath() {
		return fSettings.getAssetFilePath();
	}

	/**
	 * Gets the asset's unique entry name within the zip/package file that contains it.
	 * <p>
	 * Note that an APK is really a zip file and its assets can be accessed via the Java ZipFile class.
	 * Google Play expansion files are expected to be zip files too.
	 * @return Returns the asset's unique entry name.
	 */
	public String getZipEntryName() {
		return fSettings.getZipEntryName();
	}

	/**
	 * Gets the index to the first byte within the package file where the asset can be found.
	 * @return Returns the index to the asset's first byte within the package file.
	 */
	public long getByteOffsetInPackage() {
		return fSettings.getByteOffsetInPackage();
	}

	/**
	 * Gets the number of bytes the asset takes within the package file.
	 * <p>
	 * This byte count is expected to be used in conjuction with the byte offset when reading
	 * the asset within the package file.
	 * @return Returns the number of bytes asset takes within the package file.
	 *         If the asset is compressed, then this is the compressed byte count.
	 */
	public long getByteCountInPackage() {
		return fSettings.getByteCountInPackage();
	}

	/**
	 * Determines if the asset is compressed within the package file.
	 * @return Returns true if the asset is compressed within the package file.
	 *         Expected to use a zip compression method if true.
	 *         <p>
	 *         Returns false if the asset is not compressed within the package file.
	 *         This means that the asset can be read without a zip decompessor for fast access.
	 */
	public boolean isCompressed() {
		return fSettings.isCompressed();
	}


	/**
	 * Provides information about where an asset file can be located such as the package file which contains it,
	 * the byte offset within that package file, the asset file's byte count, etc.
	 * <p>
	 * Used to create an immutable AssetFileLocationInfo object.
	 */
	public static class Settings implements Cloneable {
		/** The file which contains the asset file. */
		private java.io.File fPackageFile;

		/** The asset's relative path and file name. */
		private String fAssetFilePath;

		/** The asset's zip entry name in the package. */
		private String fZipEntryName;

		/** Index to the asset's first byte within the package file. */
		private long fByteOffsetInPackage;

		/** The number of bytes the asset takes within the package. */
		private long fByteCountInPackage;

		/** Set to true if the asset is compressed within the package file. Set to false if not. */
		private boolean fIsCompressed;


		/** Creates a new settings object used to configure and create an AssetFileLocationInfo object. */
		public Settings() {
			fPackageFile = null;
			fAssetFilePath = null;
			fZipEntryName = null;
			fByteOffsetInPackage = 0;
			fByteCountInPackage = 0;
			fIsCompressed = false;
		}

		/**
		 * Creates a new copy of this object.
		 * @return Returns a copy of this object.
		 */
		@Override
		public Settings clone() {
			Settings clone = null;
			try {
				clone = (Settings)super.clone();
			}
			catch (Exception ex) { }
			return clone;
		}

		/**
		 * Sets the package file which contains the asset.
		 * Expected to be set to the APK's file path or to the Google Play expansion file's path.
		 * @param file The path to the package file which contains the asset file.
		 */
		public void setPackageFile(java.io.File file) {
			fPackageFile = file;
		}

		/**
		 * Gets the package file which contains the asset.
		 * @return Returns the path to the package file which contains the asset.
		 *         This is typically the path to the APK file or the Google Play expansion file.
		 *         <p>
		 *         Returns null if the file was not assigned via the setPackageFile() method.
		 */
		public java.io.File getPackageFile() {
			return fPackageFile;
		}

		/**
		 * Sets the asset's relative path and file name.
		 * <p>
		 * Note that the asset's path is really a virtual path within the package file.
		 * The path is expected to be relative to the APK's "asssets" directory or relative to the
		 * Google Play expansion file's root directory.
		 * @param filePath The asset's relative path and file name.
		 */
		public void setAssetFilePath(String filePath) {
			fAssetFilePath = filePath;
		}

		/**
		 * Gets the asset's relative path and file name.
		 * <p>
		 * Note that the asset's path is really a virtual path within the package file.
		 * The path is expected to be relative to the APK's "asssets" directory or relative to the
		 * Google Play expansion file's root directory.
		 * @return Returns the asset's relative path and file name.
		 *         <p>
		 *         Returns null if the path was not assigned via the setAssetFilePath() method.
		 */
		public String getAssetFilePath() {
			return fAssetFilePath;
		}

		/**
		 * Sets the asset's unique entry name within the zip/package file that contains it.
		 * <p>
		 * Note that an APK is really a zip file and its assets can be accessed via the Java ZipFile class.
		 * Google Play expansion files are expected to be zip files too.
		 * @param entryName The asset's unique entry name within the zip file.
		 */
		public void setZipEntryName(String entryName) {
			fZipEntryName = entryName;
		}

		/**
		 * Gets the asset's unique entry name within the zip/package file that contains it.
		 * <p>
		 * Note that an APK is really a zip file and its assets can be accessed via the Java ZipFile class.
		 * Google Play expansion files are expected to be zip files too.
		 * @return Returns the asset's unique entry name.
		 *         <p>
		 *         Returns null if the entry name was not assigned by calling the setZipEntryName() method.
		 */
		public String getZipEntryName() {
			return fZipEntryName;
		}

		/**
		 * Sets the first byte within the package file where the asset can be found.
		 * @param offset Index to the asset's first byte within the package file.
		 *               <p>
		 *               Cannot be less than zero or else an exception will be thrown.
		 */
		public void setByteOffsetInPackage(long offset) {
			if (offset < 0) {
				throw new IllegalArgumentException();
			}
			fByteOffsetInPackage = offset;
		}

		/**
		 * Gets the index to the first byte within the package file where the asset can be found.
		 * @return Returns the index to the asset's first byte within the package file.
		 */
		public long getByteOffsetInPackage() {
			return fByteOffsetInPackage;
		}

		/**
		 * Sets the number of bytes the asset takes within the package file.
		 * <p>
		 * This byte count is expected to be used in conjuction with the byte offset when reading
		 * the asset within the package file.
		 * @param count The number of bytes the asset takes within the package file.
		 *              If the asset is compressed, then this is the compressed byte count.
		 *              <p>
		 *              Cannot be less than one or else an exception will be thrown.
		 */
		public void setByteCountInPackage(long count) {
			if (count <= 0) {
				throw new IllegalArgumentException();
			}
			fByteCountInPackage = count;
		}

		/**
		 * Gets the number of bytes the asset takes within the package file.
		 * <p>
		 * This byte count is expected to be used in conjuction with the byte offset when reading
		 * the asset within the package file.
		 * @return Returns the number of bytes asset takes within the package file.
		 *         If the asset is compressed, then this is the compressed byte count.
		 */
		public long getByteCountInPackage() {
			return fByteCountInPackage;
		}

		/**
		 * Sets a flag indicating if the asset is compressed within the packgae file.
		 * @param value Set to true if the asset is compressed within the package file.
		 *              Expected to use a zip compression method if true.
		 *              <p>
		 *              Set to false if the asset is not compressed within the package file.
		 *              This means that the asset can be read without a zip decompessor for fast access.
		 */
		public void setIsCompressed(boolean value) {
			fIsCompressed = value;
		}

		/**
		 * Determines if the asset is compressed within the package file.
		 * @return Returns true if the asset is compressed within the package file.
		 *         Expected to use a zip compression method if true.
		 *         <p>
		 *         Returns false if the asset is not compressed within the package file.
		 *         This means that the asset can be read without a zip decompessor for fast access.
		 */
		public boolean isCompressed() {
			return fIsCompressed;
		}
	}
}
