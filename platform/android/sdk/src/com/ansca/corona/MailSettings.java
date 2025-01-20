//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;


/** Stores e-mail coniguration such as a To and CC list, subject, and body text. */
public class MailSettings {
	/** Stores a list of e-mail addresses to be used in the To field of the e-mail. */
	private java.util.LinkedHashSet<String> fToList;
	
	/** Stores a list of e-mail addresses to be used in the CC field of the e-mail. */
	private java.util.LinkedHashSet<String> fCcList;
	
	/** Stores a list of e-mail addresses to be used in the BCC field of the e-mail. */
	private java.util.LinkedHashSet<String> fBccList;
	
	/** Collection of URIs to files to send as attachments. */
	private java.util.LinkedHashSet<android.net.Uri> fFileAttachments;
	
	/** The e-mail's subject line. */
	private String fSubject;
	
	/** The e-mail's main body text. */
	private String fBody;
	
	/** Set true if the e-mail body is HTML. Set false if the body is plain text. */
	private boolean fIsHtml;
	
	
	/** Creates a object for storing e-mail settings. */
	public MailSettings() {
		fToList = new java.util.LinkedHashSet<String>();
		fCcList = new java.util.LinkedHashSet<String>();
		fBccList = new java.util.LinkedHashSet<String>();
		fFileAttachments = new java.util.LinkedHashSet<android.net.Uri>();
		fSubject = "";
		fBody = "";
		fIsHtml = false;
	}
	
	/**
	 * Gets a collection of e-mail addresses to be used in the e-mail's To field.
	 * @return Returns a modifiable collection of strings.
	 */
	public java.util.LinkedHashSet<String> getToList() {
		return fToList;
	}
	
	/**
	 * Gets a collection of e-mail addresses to be used in the e-mail's CC field.
	 * @return Returns a modifiable collection of strings.
	 */
	public java.util.LinkedHashSet<String> getCcList() {
		return fCcList;
	}
	
	/**
	 * Gets a collection of e-mail addresses to be used in the e-mail's BCC field.
	 * @return Returns a modifiable collection of strings.
	 */
	public java.util.LinkedHashSet<String> getBccList() {
		return fBccList;
	}
	
	/**
	 * Gets a collection of URIs to files to send as attachments.
	 * @return Returns a modifiable collection of URIs.
	 */
	public java.util.LinkedHashSet<android.net.Uri> getFileAttachments() {
		return fFileAttachments;
	}
	
	/**
	 * Gets the e-mail's subject text.
	 * @return Returns the subject text. Returns an empty string if not set yet.
	 */
	public String getSubject() {
		return fSubject;
	}
	
	/**
	 * Sets the e-mail's subject line.
	 * @param text The text to be used for the subject line. Setting this to null will make it an empty string.
	 */
	public void setSubject(String text) {
		fSubject = (text != null) ? text : "";
	}
	
	/**
	 * Gets the e-mail's body text.
	 * @return Returns the text to be used in the e-mail's body. Returns an empty string if not set yet.
	 */
	public String getBody() {
		return fBody;
	}
	
	/**
	 * Sets the e-mail's body text.
	 * @param text The text to be used by the e-mail's body. Setting this to null will make it an empty string.
	 */
	public void setBody(String text) {
		fBody = (text != null) ? text : "";
	}
	
	/**
	 * Determines if the body text is to be sent as HTML.
	 * @return Returns true if the body text is to be sent as HTML.
	 *         Returns false if the body text should be sent as plain text.
	 */
	public boolean isHtml() {
		return fIsHtml;
	}
	
	/**
	 * Flags whether or not the e-mail body should be sent as HTML or plain text.
	 * @param isHtml Set true to have the body text sent as HTML. Set false to send it as plain text.
	 */
	public void setHtmlFlag(boolean isHtml) {
		fIsHtml = isHtml;
	}
	
	/**
	 * Creates an Intent object for displaying a "Send Mail" activity initialized with this object's mail settings.
	 * @return Returns an Intent object configured to use this objects mail settings and for displaying a "Send Mail" activity.
	 */
	public android.content.Intent toIntent() {
		android.content.Intent intent;
		String actionStringKey;
		
		// Determine which action string to use based on the number of attachments.
		if (fFileAttachments.size() > 1) {
			actionStringKey = android.content.Intent.ACTION_SEND_MULTIPLE;
		}
		else {
			actionStringKey = android.content.Intent.ACTION_SEND;
		}

		// Determine what the mime type should be set to.
		String mimeType = fIsHtml ? "text/html" : "plain/text";
		if (fFileAttachments.size() > 0) {
			// The e-mail has attachments. The mime type must correspond to the attached file.
			// Note: If the files have the same mime type, then use that mime type.
			//       If the files have different mime types, then we must use a wildcard mime type.
			android.content.Context context = CoronaEnvironment.getApplicationContext();
			if (context != null) {
				com.ansca.corona.storage.FileServices fileServices;
				fileServices = new com.ansca.corona.storage.FileServices(context);
				boolean same = true;
				java.util.Iterator<android.net.Uri> i = fFileAttachments.iterator();
				mimeType = fileServices.getMimeTypeFrom(i.next());

				// Guard against the case where the first doesn't have a known MIME type.
				// In this case, we just assign a wildcard mime type, to not crash.
				if (mimeType == null) {
					mimeType = "*/*";
				}

				while (i.hasNext() && same) {
					if (mimeType != fileServices.getMimeTypeFrom(i.next())) {
						same = false;
						mimeType = "*/*";
					}
				}
			}
		}

		// Set up the intent.
		intent = new android.content.Intent(actionStringKey);
		intent.setType(mimeType);
		if (fToList.size() > 0) {
			intent.putExtra(android.content.Intent.EXTRA_EMAIL, fToList.toArray(new String[0]));
		}
		if (fCcList.size() > 0) {
			intent.putExtra(android.content.Intent.EXTRA_CC, fCcList.toArray(new String[0]));
		}
		if (fBccList.size() > 0) {
			intent.putExtra(android.content.Intent.EXTRA_BCC, fBccList.toArray(new String[0]));
		}
		if (fSubject.length() > 0) {
			intent.putExtra(android.content.Intent.EXTRA_SUBJECT, fSubject);
		}
		if (fBody.length() > 0) {
			intent.putExtra(android.content.Intent.EXTRA_TEXT, fIsHtml ? android.text.Html.fromHtml(fBody) : fBody);
		}
		if (fFileAttachments.size() > 1) {
			java.util.ArrayList<android.net.Uri> uriList = new java.util.ArrayList<android.net.Uri>(fFileAttachments.size());
			uriList.addAll(fFileAttachments);
			intent.putParcelableArrayListExtra(android.content.Intent.EXTRA_STREAM, uriList);
		}
		else if (fFileAttachments.size() == 1) {
			intent.putExtra(android.content.Intent.EXTRA_STREAM, fFileAttachments.iterator().next());
		}
		if (fFileAttachments.size() > 0) {
			intent.addFlags(android.content.Intent.FLAG_GRANT_READ_URI_PERMISSION);
		}
		return intent;
	}
	
	/**
	 * Extracts the mail settings from the given "mailto:" URL.
	 * @param url The "mailto:" URL to extract mail settings from.
	 * @return Returns a new MailSettings object containing the mail information stored in the given URL.
	 *         If the given URL is invalid, then the returned object will be set to its defaults.
	 */
	public static MailSettings fromUrl(String url) {
		MailSettings settings = new MailSettings();
		
		// Do not continue if given an invalid URL.
		if ((url == null) || (url.length() <= 0)) {
			return settings;
		}
		
		// Extract e-mail settings from the given "mailto:" URL.
		try {
			android.net.MailTo mailToConverter = android.net.MailTo.parse(url);
			if (mailToConverter.getTo() != null) {
				for (String addressName : mailToConverter.getTo().split(",")) {
					settings.getToList().add(addressName);
				}
			}
			if (mailToConverter.getCc() != null) {
				for (String addressName : mailToConverter.getCc().split(",")) {
					settings.getCcList().add(addressName);
				}
			}
			if (mailToConverter.getSubject() != null) {
				settings.setSubject(mailToConverter.getSubject());
			}
			if (mailToConverter.getBody() != null) {
				settings.setBody(mailToConverter.getBody());
			}
			for (java.util.Map.Entry<String, String> item : mailToConverter.getHeaders().entrySet()) {
				if (item.getKey().toLowerCase().equals("bcc")) {
					for (String addressName : item.getValue().split(",")) {
						settings.getBccList().add(addressName);
					}
				}
				// Custom "mailto:" argument types go here.
			}
		}
		catch (Exception ex) { }
		
		// Return the settings object.
		return settings;
	}
	
	/**
	 * Extracts the mail settings from a Corona hashtable.
	 * @param context Reference to this application's context. Cannot be null.
	 * @param collection The hashtable to extract mail settings from.
	 * @return Returns a new MailSettings object containing the mail information stored in the given hashtable.
	 *         If the given table is invalid, then the returned object will be set to its defaults.
	 */
	public static MailSettings from(android.content.Context context, java.util.HashMap<String, Object> collection) {
		MailSettings settings = new MailSettings();
		
		// Do not continue if given an invalid argument.
		if (collection == null) {
			return settings;
		}
		
		// Extract mail settings from the given collection.
		for (java.util.Map.Entry<String, Object> entry : collection.entrySet()) {
			// Fetch entry information.
			String keyName = entry.getKey();
			Object value = entry.getValue();
			if ((keyName == null) || (keyName.length() <= 0) || (value == null)) {
				continue;
			}
			
			// Extract entry's setting(s).
			keyName = keyName.toLowerCase().trim();
			if (keyName.equals("to")) {
				addStringObjectToCollection(settings.getToList(), value);
			}
			else if (keyName.equals("cc")) {
				addStringObjectToCollection(settings.getCcList(), value);
			}
			else if (keyName.equals("bcc")) {
				addStringObjectToCollection(settings.getBccList(), value);
			}
			else if (keyName.equals("subject")) {
				if (value instanceof String) {
					settings.setSubject((String)value);
				}
			}
			else if (keyName.equals("body")) {
				if (value instanceof String) {
					settings.setBody((String)value);
				}
			}
			else if (keyName.equals("isbodyhtml")) {
				if (value instanceof Boolean) {
					settings.setHtmlFlag(((Boolean)value).booleanValue());
				}
			}
			else if (keyName.equals("attachment")) {
				if (value instanceof java.util.HashMap<?,?>) {
					for (Object nextObject : ((java.util.HashMap<Object, Object>)value).values()) {
						addFileAttachmentObjectToCollection(context, settings.getFileAttachments(), nextObject);
					}
				}
				else {
					addFileAttachmentObjectToCollection(context, settings.getFileAttachments(), value);
				}
			}
		}
		
		// Return the settings object.
		return settings;
	}
	
	/**
	 * Copies the given value object to the collection.
	 * This method is only called by the static from() method.
	 * @param collection The string collection to copy the value object to.
	 * @param value The object to be copied into the string collection.
	 *              Can be of type String, String[], or Collection<String>. No other type is supported.
	 */
	private static void addStringObjectToCollection(java.util.LinkedHashSet<String> collection, Object value) {
		// Validate.
		if ((collection == null) || (value == null)) {
			return;
		}
		
		// Copy object to the given string collection, but only if the given value is a string or a collection of strings.
		try {
			if (value instanceof String) {
				collection.add((String)value);
			}
			else if (value instanceof String[]) {
				for (String text : (String[])value) {
					collection.add(text);
				}
			}
			else if (value instanceof java.util.HashMap<?,?>) {
				for (Object nextObject : ((java.util.HashMap<Object, Object>)value).values()) {
					if (nextObject instanceof String) {
						collection.add((String)nextObject);
					}
				}
			}
			else if (value instanceof java.util.Collection<?>) {
				collection.addAll((java.util.Collection<String>)value);
			}
		}
		catch (Exception ex) { }
	}
	
	/**
	 * Copies the given file attachment object to the collection.
	 * This method is only called by the static from() method.
	 * @param context Reference to this application's context. Cannot be null.
	 * @param collection The file attachment collection to copy the value object to.
	 * @param value The object to be copied into the given collection.
	 *              Expected to be a file reference of some kind such as a string path, file object, or URI.
	 */
	private static void addFileAttachmentObjectToCollection(
			android.content.Context context, java.util.LinkedHashSet<android.net.Uri> collection, Object value) {
		
		// Validate.
		if ((collection == null) || (value == null)) {
			return;
		}
		
		// Copy object to the given collection, but only if the given value is one of the below types.
		if (value instanceof String) {
			if (context != null) {
				collection.add(com.ansca.corona.storage.FileContentProvider.createContentUriForFile(context, (String)value));
			}
		}
		else if (value instanceof String[]) {
			if (context != null) {
				for (String filePath : (String[])value) {
					collection.add(com.ansca.corona.storage.FileContentProvider.createContentUriForFile(context, filePath));
				}
			}
		}
		else if (value instanceof java.io.File) {
			if (context != null) {
				String filePath = ((java.io.File)value).getPath();
				collection.add(com.ansca.corona.storage.FileContentProvider.createContentUriForFile(context, filePath));
			}
		}
		else if (value instanceof java.io.File[]) {
			if (context != null) {
				for (java.io.File file : (java.io.File[])value) {
					collection.add(com.ansca.corona.storage.FileContentProvider.createContentUriForFile(context, file.getPath()));
				}
			}
		}
		else if (value instanceof android.net.Uri) {
			collection.add((android.net.Uri)value);
		}
		else if (value instanceof android.net.Uri[]) {
			for (android.net.Uri uri : (android.net.Uri[])value) {
				collection.add(uri);
			}
		}
	}
}
