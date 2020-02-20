//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.storage;


/** Stream used to read decompressed data from one entry within a zip file. */
public class ZipFileEntryInputStream extends java.io.InputStream {
	/** Provides read access to the zip file. */
	private java.util.zip.ZipFile fZipFile;

	/** Provides information about the zip file's entry. */
	private java.util.zip.ZipEntry fZipEntry;

	/** The input stream to the zip file's entry. */
	private java.io.InputStream fInputStream;


	/**
	 * Creates a new input stream for reading decompressed data from one entry within a zip file.
	 * @param filePath The path and file name of the zip file to be accessed. Cannot be null or empty.
	 * @param entryName The unique name of the entry within the zip file. Cannot be null or empty.
	 */
	public ZipFileEntryInputStream(String filePath, String entryName)
		throws NullPointerException, IllegalArgumentException, IllegalStateException,
				java.io.IOException, java.io.FileNotFoundException, java.util.zip.ZipException
	{
		this(new java.io.File(filePath), entryName);
	}

	/**
	 * Creates a new input stream for reading decompressed data from one entry within a zip file.
	 * @param file The zip file to be accessed. Cannot be null.
	 * @param entryName The unique name of the entry within the zip file. Cannot be null or empty.
	 */
	public ZipFileEntryInputStream(java.io.File file, String entryName)
		throws NullPointerException, IllegalArgumentException, IllegalStateException,
				java.io.IOException, java.io.FileNotFoundException, java.util.zip.ZipException
	{
		// Validate arguments.
		if (file == null) {
			throw new NullPointerException("file");
		}
		if (file.exists() == false) {
			throw new java.io.FileNotFoundException(file.getAbsolutePath());
		}
		if (entryName == null) {
			throw new NullPointerException("entryName");
		}
		if (entryName.length() <= 0) {
			throw new IllegalArgumentException("'entryName' cannot be empty.");
		}

		// Attempt to open the zip file.
		fZipFile = new java.util.zip.ZipFile(file);

		// Attempt to open an input stream to the zip file entry.
		fZipEntry = null;
		fInputStream = null;
		try {
			open(fZipFile, entryName);
		}
		finally {
			if ((fInputStream == null) && (fZipFile != null)) {
				try { fZipFile.close(); }
				catch (Exception ex) { }
			}
		}
	}

	/**
	 * Creates a new input stream for reading decompressed data from one entry within a zip file.
	 * @param zipFile The zip file to be accessed. Cannot be null.
	 * @param entryName The unique name of the entry within the zip file. Cannot be null or empty.
	 */
	public ZipFileEntryInputStream(java.util.zip.ZipFile zipFile, String entryName)
		throws NullPointerException, IllegalArgumentException, IllegalStateException,
				java.io.IOException, java.io.FileNotFoundException, java.util.zip.ZipException
	{
		// Do not keep a reference to the zip file since this input stream does not own it.
		// This will prevent this object from closing the zip file.
		fZipFile = null;
		fZipEntry = null;
		fInputStream = null;

		// Attempt to open an input stream to the zip file entry.
		open(zipFile, entryName);
	}

	/**
	 * Opens an input stream to the given zip file entry.
	 * @param zipFile The zip file to be accessed. Expected to be open. Cannot be null.
	 * @param entryName The unique name of the entry within the zip file. Cannot be null or empty.
	 */
	private void open(java.util.zip.ZipFile zipFile, String entryName)
		throws NullPointerException, IllegalArgumentException, IllegalStateException,
				java.io.IOException, java.io.FileNotFoundException, java.util.zip.ZipException
	{
		// Validate arguments.
		if (zipFile == null) {
			throw new NullPointerException("zipFile");
		}
		if (entryName == null) {
			throw new NullPointerException("entryName");
		}
		if (entryName.length() <= 0) {
			throw new IllegalArgumentException("'entryName' cannot be empty.");
		}

		// Attempt to open an input stream to the zip file entry.
		fZipEntry = zipFile.getEntry(entryName);
		if (fZipEntry == null) {
			throw new java.util.zip.ZipException("Failed to find zip file entry: " + entryName);
		}
		fInputStream = zipFile.getInputStream(fZipEntry);
	}

	/** Closes the input stream to the zip file. */
	@Override
	public void close() throws java.io.IOException {
		fInputStream.close();
		if (fZipFile != null) {
			fZipFile.close();
		}
	}

	/**
	 * Gets the number of decompressed bytes of the zip file's entry.
	 * @return Returns the number of bytes the entry claims in decompressed form.
	 */
	@Override
	public int available() throws java.io.IOException {
		return (int)(fZipEntry.getSize());
	}

	/**
	 * Sets a mark position in this input stream.
	 * @param readlimit The number of bytes that can be read from this stream before the mark is invalidated.
	 */
	@Override
	public void mark(int readlimit) {
		fInputStream.mark(readlimit);
	}

	/**
	 * Determines if this input stream supports the mark() and reset() methods.
	 * @return Returns true if mark() and reset() are support. Returns false if not.
	 */
	@Override
	public boolean markSupported() {
		return fInputStream.markSupported();
	}

	/** Resets this stream to the last marked location. */
	@Override
	public void reset() throws java.io.IOException {
		fInputStream.reset();
	}

	/**
	 * Reads a single byte from this stream.
	 * <p>
	 * Blocks until one byte has been read, the end of the source stream has been reached,
	 * or when an exception has been thrown.
	 * @return Returns the read byte as an integer ranging between 0 and 255.
	 *         <p>
	 *         Returns -1 if the end of the stream has been reached.
	 */
	@Override
	public int read() throws java.io.IOException {
		return fInputStream.read();
	}

	/**
	 * Reads as many bytes from the source that will fill the given array.
	 * @param buffer The byte array which will stored the read bytes.
	 * @return Returns the number of bytes read or -1 if the end of the stream has been reached.
	 */
	@Override
	public int read(byte[] buffer) throws java.io.IOException, java.lang.IndexOutOfBoundsException {
		return fInputStream.read(buffer);
	}

	/**
	 * Reads at most the number of bytes specified by "length" from the stream to the given buffer.
	 * @param buffer The byte array which will stored the read bytes.
	 * @param offset Zero based index to the position in the "buffer" array to start storing read bytes.
	 * @param length The maximum number of bytes to read.
	 * @return Returns the number of bytes read or -1 if the end of the stream has been reached.
	 */
	@Override
	public int read(byte[] buffer, int offset, int length)
		throws java.io.IOException, java.lang.IndexOutOfBoundsException
	{
		return fInputStream.read(buffer, offset, length);
	}

	/**
	 * Skips the given number of bytes in the stream.
	 * @param byteCount The number of bytes to skip. Does nothing if less than or equal to zero.
	 * @return Returns the number of bytes skipped.
	 */
	@Override
	public long skip(long byteCount) throws java.io.IOException {
		return fInputStream.skip(byteCount);
	}

	/**
	 * Safely opens an input stream to the given zip file entry without exceptions.
	 * @param filePath The path and file name of the zip file to be accessed.
	 * @param entryName The unique name of the entry within the zip file.
	 * @return Returns an input stream to zip file entry.
	 *         <p>
	 *         Returns null if failed to find the given zip file or its zip entry.
	 */
	public static ZipFileEntryInputStream tryOpen(String filePath, String entryName) {
		// Validate arguments.
		if ((filePath == null) || (filePath.length() <= 0)) {
			return null;
		}
		
		// Open a stream to the zip file entry.
		ZipFileEntryInputStream inputStream = null;
		try {
			inputStream = ZipFileEntryInputStream.tryOpen(new java.io.File(filePath), entryName);
		}
		catch (Exception ex) { }
		return inputStream;
	}

	/**
	 * Safely opens an input stream to the given zip file entry without exceptions.
	 * @param file The zip file to be accessed.
	 * @param entryName The unique name of the entry within the zip file.
	 * @return Returns an input stream to zip file entry.
	 *         <p>
	 *         Returns null if failed to find the given zip file or its zip entry.
	 */
	public static ZipFileEntryInputStream tryOpen(java.io.File file, String entryName) {
		// Validate arguments.
		if ((file == null) || (file.exists() == false)) {
			return null;
		}
		if ((entryName == null) || (entryName.length() <= 0)) {
			return null;
		}

		// Open a stream to the zip file entry.
		ZipFileEntryInputStream inputStream = null;
		try {
			inputStream = new ZipFileEntryInputStream(file, entryName);
		}
		catch (Exception ex) { }
		return inputStream;
	}

	/**
	 * Safely opens an input stream to the given zip file entry without exceptions.
	 * @param file The zip file to be accessed.
	 * @param entryName The unique name of the entry within the zip file.
	 * @return Returns an input stream to zip file entry.
	 *         <p>
	 *         Returns null if failed to find the given zip file or its zip entry.
	 */
	public static ZipFileEntryInputStream tryOpen(java.util.zip.ZipFile file, String entryName) {
		// Validate arguments.
		if (file == null) {
			return null;
		}
		if ((entryName == null) || (entryName.length() <= 0)) {
			return null;
		}

		// Open a stream to the zip file entry.
		ZipFileEntryInputStream inputStream = null;
		try {
			inputStream = new ZipFileEntryInputStream(file, entryName);
		}
		catch (Exception ex) { }
		return inputStream;
	}
}
