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
 * Stores SMS coniguration such as recipients and text message.
 * Note: In the future, this can be used to configure MMS with a file attachment.
 */
public class SmsSettings {
	/** Stores a list of phone numbers in string form to be texted. */
	private java.util.LinkedHashSet<String> fRecipients;
	
	/** The text to be sent. */
	private String fText;
	
	
	/** Creates an object for storing SMS settings. */
	public SmsSettings() {
		fRecipients = new java.util.LinkedHashSet<String>();
		fText = "";
	}
	
	/**
	 * Gets a collection of phone number strings that will be sent a text message.
	 * @return Returns a modifiable collection of strings.
	 */
	public java.util.LinkedHashSet<String> getRecipients() {
		return fRecipients;
	}
	
	/**
	 * Gets the text message to be sent.
	 * @return Returns the text to be sent. Returns an empty string if not set yet.
	 */
	public String getText() {
		return fText;
	}
	
	/**
	 * Sets the text message to be sent.
	 * @param text The text to be sent. Setting this to null will make it an empty string.
	 */
	public void setText(String text) {
		fText = (text != null) ? text : "";
	}
	
	/**
	 * Creates an Intent object for displaying a "Send SMS" activity initialized with this object's settings.
	 * @return Returns an Intent object configured to use this objects SMS settings and for displaying a "Send SMS" activity.
	 */
	public android.content.Intent toIntent() {
		android.content.Intent intent;
		
		intent = new android.content.Intent(android.content.Intent.ACTION_VIEW);
		intent.setData(android.net.Uri.parse("sms:"));
		if (fRecipients.size() > 0) {
			boolean isFirstString = true;
			StringBuilder builder = new StringBuilder();
			builder.append("smsto:");
			for (String phoneNumberString : fRecipients) {
				if ((phoneNumberString != null) && (phoneNumberString.length() > 0)) {
					if (isFirstString == false) {
						builder.append(";");
					}
					builder.append(phoneNumberString);
					isFirstString = false;
				}
			}
			intent.setData(android.net.Uri.parse(builder.toString()));
		}
		if (fText.length() > 0) {
			intent.putExtra("sms_body", fText);
		}
		return intent;
	}
	
	/**
	 * Extracts the SMS settings from a Corona hashtable.
	 * @param collection The hashtable to extract SMS settings from.
	 * @return Returns a new SmsSettings object containing the information stored in the given hashtable.
	 *         If the given table is invalid, then the returned object will be set to its defaults.
	 */
	public static SmsSettings from(java.util.HashMap<String, Object> collection) {
		SmsSettings settings = new SmsSettings();
		
		// Do not continue if given an invalid argument.
		if (collection == null) {
			return settings;
		}
		
		// Extract SMS settings from the given collection.
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
				if (value instanceof String) {
					settings.getRecipients().add((String)value);
				}
				else if (value instanceof String[]) {
					for (String text : (String[])value) {
						settings.getRecipients().add(text);
					}
				}
				else if (value instanceof java.util.HashMap<?,?>) {
					for (Object nextObject : ((java.util.HashMap<Object, Object>)value).values()) {
						if (nextObject instanceof String) {
							settings.getRecipients().add((String)nextObject);
						}
					}
				}
				else if (value instanceof java.util.Collection<?>) {
					try {
						settings.getRecipients().addAll((java.util.Collection<String>)value);
					}
					catch (Exception ex) { }
				}
			}
			else if (keyName.equals("body")) {
				if (value instanceof String) {
					settings.setText((String)value);
				}
			}
		}
		
		// Return the settings object.
		return settings;
	}
}
