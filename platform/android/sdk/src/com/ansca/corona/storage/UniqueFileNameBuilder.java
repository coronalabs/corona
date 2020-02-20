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
 * Generates a unique file name in a given directory.
 * The file name format and its extension is configurable.
 * <p>
 * Example usage:
 * <pre>
 *    // Create a unique JPEG file name in the "Pictures" directory.
 *    UniqueFileNameBuilder builder = new UniqueFileNameBuilder();
 *    builder.setDirectory(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES));
 *    builder.setFileNameFormat("Picture %d");
 *    builder.setFileExtension("jpg");
 *    java.io.File imageFile = builder.build();
 *    if (imageFile != null) {
 *       // Unique file name was generated. Create the file here.
 *    }
 * </pre>
 */
public class UniqueFileNameBuilder {
	/** Directory path to generate a unique file name in. */
	private java.io.File fDirectory;

	/** Format string used to generate the file name. */
	private String fFileNameFormat;

	/** The extension to append to the file name. */
	private String fFileExtension;


	/** Create a new file name builder object. */
	public UniqueFileNameBuilder() {
		fDirectory = null;
		fFileNameFormat = "File %d";
		fFileExtension = "";
	}

	/**
	 * Sets the directory to generate a unique file name in.
	 * @param directory The directory's path. This directory is expected to exist
	 *                  or else this object will fail to build a unique file name.
	 */
	public void setDirectory(java.io.File directory) {
		fDirectory = directory;
	}

	/**
	 * Gets the directory this builder will generate a unique file name in.
	 * @return Returns a File object containing the path to the directory.
	 *         <p>
	 *         Returns null if the directory has not been set yet.
	 */
	public java.io.File getDirectory() {
		return fDirectory;
	}

	/**
	 * Sets the file name format string to be used to genearte a unique file name.
	 * @param format The format string to be used, such as "File %d". This string is expected to have
	 *               a "%d" in it so that this builder can generate a unique number for the file name.
	 *               This string is not expected to provide a file extension.
	 */
	public void setFileNameFormat(String format) {
		if (format == null) {
			format = "";
		}
		fFileNameFormat = format;
	}

	/**
	 * Gets the file name format string that this builder will use to generate a unique file name.
	 * @return Returns the format string used to generate the file name.
	 */
	public String getFileNameFormat() {
		return fFileNameFormat;
	}

	/**
	 * Sets the file extension to be appended to the file name.
	 * @param extension The extension to be appended to the file name.
	 *                  <p>
	 *                  Set to null or empty string to not append an extension to the file name.
	 */
	public void setFileExtension(String extension) {
		// Use an empty string if given a null reference.
		if (extension == null) {
			extension = "";
		}

		// Remove the leading period if provided.
		if (extension.startsWith(".")) {
			extension = extension.substring(1);
		}

		// Store the file extension.
		fFileExtension = extension;
	}

	/**
	 * Gets the file extension to be appended to the file name.
	 * @return Returns the extension to be appended to the file name without the leading period.
	 *         <p>
	 *         Returns an empty string if an extension will not be appended.
	 */
	public String getFileExtension() {
		return fFileExtension;
	}

	/**
	 * Generates a unique file name for the directory that you provided to the setDirectory() method.
	 * <p>
	 * Note that this method does not actually create a file in the set directory.
	 * This method only generates a unique file name that you can use to create a new file with.
	 * <p>
	 * You are expected to call the setDirectory(), setFileNameFormat(), and setFileExtension() methods
	 * before calling this build() method.
	 * @return Returns a path and file name that is unique in the set directory.
	 *         <p>
	 *         Returns null if the directory does not exist.
	 */
	public java.io.File build() {
		// Do not continue if the given directory does not exist.
		if ((fDirectory == null) || (fDirectory.exists() == false)) {
			return null;
		}

		// Determines if the given file name contains a "%d" format type for inserting a number.
		// Note: This is a very simple check, but this string is expected to always contain a "%d".
		boolean hasFormatter = (fFileNameFormat.indexOf('%') >= 0);

		// Create a unique file name in the directory.
		java.io.File uniqueFile = null;
		try {
			StringBuilder fileNameBuilder = new StringBuilder();
			for (int index = 1; index <= 10000; index++) {
				if (hasFormatter) {
					fileNameBuilder.append(String.format(fFileNameFormat, index));
				}
				else {
					fileNameBuilder.append(fFileNameFormat);
					fileNameBuilder.append(Integer.toString(index));
				}
				if (fFileExtension.length() > 0) {
					fileNameBuilder.append('.');
					fileNameBuilder.append(fFileExtension);
				}
				java.io.File nextFile = new java.io.File(fDirectory, fileNameBuilder.toString());
				if (nextFile.exists() == false) {
					uniqueFile = nextFile;
					break;
				}
				fileNameBuilder.setLength(0);
			}
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return uniqueFile;
	}
}
