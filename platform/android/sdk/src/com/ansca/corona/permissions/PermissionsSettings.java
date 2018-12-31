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

package com.ansca.corona.permissions;

/* TODO: CLEAN UP DOCS FOR GETTERS AND SETTERS. ENSURE THAT IT'S CLEAR THAT WHEN PERMISSIONS ARE REQUESTED, ANDROID PERMISSION NAMES ARE EXPECTED TO BE HERE! i.e. requestedPermission... such as.. android.permission...
 * ALSO ENSURE THAT THE PLATFORM-AGNOSTIC NAMES ARE MADE CLEAR: "These are Corona String IDs you'd use in Lua".*/

/**
 * Stores Permissions configuration options such as which permissions to request, 
 * why they're needed, how important they are to the app, etc.
 * <p>
 * <b>Added in <a href="http://developer.coronalabs.com/release-ent/2016/2828/">daily build 2016.2828</a></b>.
 */
public class PermissionsSettings {
	/** Stores a list of permissions to be requested. */
	private java.util.LinkedHashSet<String> fPermissions;

	// TODO: Refactor urgency and dialog strings out of PermissionsSettings since they're only every used by the DefaultHandler.
	// Or Alternatively, open up all the functionality here to Enterprise devs.
	/** A listener to be invoked after the popup closes. */
	private int fListener;
	
	/** Urgency of this permissions request. */
	private PermissionUrgency fUrgency;

	/** The title for the permission rationale dialog. */
	private String fRationaleTitle;

	/** A message to state why these permissions are needed. */
	private String fRationaleDescription;

	/** The title for the settings redirect dialog. */
	private String fSettingsRedirectTitle;

	/** A message to display when redirecting users to Settings. */
	private String fSettingsRedirectDescription;
	
	/** Flag to indicate whether this PermissionsSettings needs to be serviced. */
	private java.util.concurrent.atomic.AtomicBoolean fNeedsService = new java.util.concurrent.atomic.AtomicBoolean(true);

	/** 
	 * Dictionary containing all permissions settings objects that have yet to be serviced.
	 * Uses the request code assigned by registerOnRequestPermissionsResultHandler() as a key.
	 * Primarily used to tranfer permissions settings to OnRequestPermissionsResultHandlers.
	 * Can also be used to pre-determine permissions requests you app may make.
	 */
	private static java.util.HashMap<Integer, PermissionsSettings> fPermissionSettingsToBeServiced =
						new java.util.HashMap<Integer, PermissionsSettings>();
	
	/** Creates an object for storing Permissions settings. */
	public PermissionsSettings() {
		this(new java.util.LinkedHashSet<String>());
	}

	// Single Permission constructors
	/** 
	 * Creates an object for storing Permissions settings.
	 * @param permission The permission we wish to request.
	 */
	public PermissionsSettings(String permission) {
		this(permission, 0);
	}

	// TODO: PUBLICLY DOCUMENT THE PURPOSE OF THIS CONSTRUCTOR WHEN THE FULL 
	// SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	public PermissionsSettings(String permission, int listener) {
		this(permission, listener, PermissionUrgency.LOW);
	}

	// TODO: PUBLICLY DOCUMENT THE PURPOSE OF THIS CONSTRUCTOR WHEN THE FULL 
	// SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	public PermissionsSettings(String permission, int listener, PermissionUrgency urgency) {
		java.util.LinkedHashSet<String> permissions = new java.util.LinkedHashSet<String>();
		if (permission != null) {
			permissions.add(permission);
		}
		fPermissions = permissions;
		fListener = listener;
		fUrgency = urgency;
	}

	// String array constructors
	/** 
	 * Creates an object for storing Permissions settings.
	 * @param permissions An array of permissions we wish to request.
	 */
	public PermissionsSettings(String[] permissions) {
		this(permissions, 0);
	}

	// TODO: PUBLICLY DOCUMENT THE PURPOSE OF THIS CONSTRUCTOR WHEN THE FULL 
	// SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	public PermissionsSettings(String[] permissions, int listener) {
		this(permissions, listener, PermissionUrgency.LOW);
	}

	// TODO: PUBLICLY DOCUMENT THE PURPOSE OF THIS CONSTRUCTOR WHEN THE FULL 
	// SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	public PermissionsSettings(String[] permissions, int listener, PermissionUrgency urgency) {
		// Validate arguments.
		if (permissions != null && permissions.length > 0) {
			fPermissions = new java.util.LinkedHashSet<String>(java.util.Arrays.asList(permissions));
		} else {
			fPermissions = new java.util.LinkedHashSet<String>();
		}
		fListener = listener;
		fUrgency = urgency;
	}

	// LinkedHashSet constructors
	/** 
	 * Creates an object for storing Permissions settings.
	 * @param permissions A {@link java.util.LinkedHashSet LinkedHashSet} of permissions we wish to request.
	 */
	public PermissionsSettings(java.util.LinkedHashSet<String> permissions) {
		this(permissions, 0);
	}

	// TODO: PUBLICLY DOCUMENT THE PURPOSE OF THIS CONSTRUCTOR WHEN THE FULL 
	// SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	public PermissionsSettings(java.util.LinkedHashSet<String> permissions, int listener) {
		this(permissions, listener, PermissionUrgency.LOW);
	}

	// TODO: PUBLICLY DOCUMENT THE PURPOSE OF THIS CONSTRUCTOR WHEN THE FULL 
	// SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	public PermissionsSettings(java.util.LinkedHashSet<String> permissions, int listener, PermissionUrgency urgency) {
		if (permissions != null) {
			fPermissions = permissions;
		} else {
			fPermissions = new java.util.LinkedHashSet<String>();
		}
		fListener = listener;
		fUrgency = urgency;
	}
	
	/**
	 * Gets a collection of permissions that will be requested.
	 * @return Returns a modifiable collection of strings.
	 */
	public java.util.LinkedHashSet<String> getPermissions() {
		return fPermissions;
	}

	/**
	 * Sets the collection of permissions to be requested.
	 * @param newPermissions The new permissions we want this PermissionsSettings object to hold.
	 */
	public void setPermissions(java.util.LinkedHashSet<String> newPermissions) {
		fPermissions = newPermissions;
	}
	
	// TODO: PUBLICLY DOCUMENT THIS WHEN THE FULL SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	// /**
	//  * Gets the lua listener to be called after the permissions dialog has been dismissed.
	//  * @return Returns the lua registry id for the listener to be called.
	//  */
	public int getListener() {
		return fListener;
	}

	// TODO: PUBLICLY DOCUMENT THIS WHEN THE FULL SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	// /**
	//  * Sets the lua listener to be called after the permissions dialog has been dismissed.
	//  * @param listenerRegistryId The Id for this listener in the lua registry.
	//  */
	public void setListener(int listenerRegistryId) {
		fListener = listenerRegistryId;
	}

	// TODO: PUBLICLY DOCUMENT THIS WHEN THE FULL SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	// /**
	//  * Gets the urgency of this permissions request
	//  * @return Returns the {@link com.ansca.corona.permissions.PermissionUrgency PermissionUrgency} of this permissions request.
	//  */
	public PermissionUrgency getUrgency() {
		return fUrgency;
	}

	// TODO: PUBLICLY DOCUMENT THIS WHEN THE FULL SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	// /**
	//  * Sets the urgency of this permissions request.
	//  * @param urgency The {@link com.ansca.corona.permissions.PermissionUrgency PermissionUrgency} to set for this request.
	//  */
	public void setUrgency(PermissionUrgency urgency) {
		fUrgency = urgency;
	}

	// TODO: PUBLICLY DOCUMENT THIS WHEN THE FULL SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	// /**
	//  * Gets the title for the permission rationale dialog.
	//  * @return Returns the title for the permission rationale dialog.
	//  */
	public String getRationaleTitle() {
		return fRationaleTitle;
	}
	
	// TODO: PUBLICLY DOCUMENT THIS WHEN THE FULL SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	// /**
	//  * Sets the title for the permission rationale dialog.
	//  * @param text The rationale for these permissions. Setting this to null will make it an empty string.
	//  */
	public void setRationaleTitle(String rationaleTitle) {
		fRationaleTitle = (rationaleTitle != null) ? rationaleTitle : "";
	}

	// TODO: PUBLICLY DOCUMENT THIS WHEN THE FULL SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	// /**
	//  * Gets the rationale for needing these permissions.
	//  * @return Returns the rationale for these permissions. Returns an empty string if not set yet.
	//  */
	public String getRationaleDescription() {
		return fRationaleDescription;
	}
	
	// TODO: PUBLICLY DOCUMENT THIS WHEN THE FULL SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	// /**
	//  * Sets the rationale for these permissions.
	//  * @param rationaleDescription The rationale for these permissions. Setting this to null will make it an empty string.
	//  */
	public void setRationaleDescription(String rationaleDescription) {
		fRationaleDescription = (rationaleDescription != null) ? rationaleDescription : "";
	}

	// TODO: PUBLICLY DOCUMENT THIS WHEN THE FULL SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	// /**
	//  * Gets the title for the permission rationale dialog.
	//  * @return Returns the title for the permission rationale dialog.
	//  */
	public String getSettingsRedirectTitle() {
		return fSettingsRedirectTitle;
	}
	
	// TODO: PUBLICLY DOCUMENT THIS WHEN THE FULL SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	// /**
	//  * Sets the title for the permission rationale dialog.
	//  * @param text The rationale for these permissions. Setting this to null will make it an empty string.
	//  */
	public void setSettingsRedirectTitle(String settingsRedirectTitle) {
		fSettingsRedirectTitle = (settingsRedirectTitle != null) ? settingsRedirectTitle : "";
	}

	// TODO: PUBLICLY DOCUMENT THIS WHEN THE FULL SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	// /**
	//  * Gets the rationale for needing these permissions.
	//  * @return Returns the rationale for these permissions. Returns an empty string if not set yet.
	//  */
	public String getSettingsRedirectDescription() {
		return fSettingsRedirectDescription;
	}
	
	// TODO: PUBLICLY DOCUMENT THIS WHEN THE FULL SUITE OF PERMISSION REQUEST THINGS IS OPENED TO ENTERPRISE CUSTOMERS!
	// /**
	//  * Sets the rationale for these permissions.
	//  * @param rationaleDescription The rationale for these permissions. Setting this to null will make it an empty string.
	//  */
	public void setSettingsRedirectDescription(String settingsRedirectDescription) {
		fSettingsRedirectDescription = (settingsRedirectDescription != null) ? settingsRedirectDescription : "";
	}

	/**
	 * Returns whether this PermissionsSettings object needs service.
	 */
	public boolean needsService() {
		return fNeedsService.get();
	}

	/**
	 * Puts this PermissionsSettings object in the "serviced" state.
	 */
	public void markAsServiced() {
		fNeedsService.set(false);
	}

	/**
	 * Puts this PermissionsSettings object in the "needs service" state.
	 */
	public void resetServiceStatus() {
		fNeedsService.set(true);
	}

	// TODO: PUBLICLY DOCUMENT THIS WHEN THE SERVICE QUEUE CAN HANDLE MORE ROBUSTNESS WITH BATCHING PERMISSION REQUESTS!
	// /**
	//  * Fetches all the PermissionSettings to be serviced.
	//  * @return Returns a HashMap containing all PermissionsSettings objects that still need service.
	//  */
	public static java.util.HashMap<Integer, PermissionsSettings> getSettingsToBeServiced() {
		return fPermissionSettingsToBeServiced;
	}
	
	/**
	 * Extracts the PermissionsSettings from a Corona Hashtable.
	 * @param collection The {@link java.util.HashMap HashMap} to extract PermissionsSettings from.
	 * @return Returns a new PermissionsSettings object containing the information stored in the given hashtable.
	 *         If the given table is invalid, then the returned object will be set to its defaults.
	 */
	public static PermissionsSettings from(java.util.HashMap<String, Object> collection) {
		final PermissionsSettings settings = new PermissionsSettings();
		// Do not continue if given an invalid argument.
		if (collection == null) {
			return settings;
		}
		
		// Extract Permissions settings from the given collection.
		for (java.util.Map.Entry<String, Object> entry : collection.entrySet()) {
			// Fetch entry information.
			String keyName = entry.getKey();
			Object value = entry.getValue();
			if ((keyName == null) || (keyName.length() <= 0) || (value == null)) {
				continue;
			}
			
			// Extract entry's setting(s).
			keyName = keyName.toLowerCase().trim();
			if (keyName.equals("apppermission") || keyName.equals("apppermissions")) {
				if (value instanceof String) {
					settings.getPermissions().add((String)value);
				}
				else if (value instanceof String[]) {
					for (String text : (String[])value) {
						settings.getPermissions().add(text);
					}
				}
				else if (value instanceof java.util.HashMap<?,?>) {
					for (Object nextObject : ((java.util.HashMap<Object, Object>)value).values()) {
						if (nextObject instanceof String) {
							settings.getPermissions().add((String)nextObject);
						}
					}
				}
				else if (value instanceof java.util.Collection<?>) {
					try {
						settings.getPermissions().addAll((java.util.Collection<String>)value);
					}
					catch (Exception ex) { }
				}
			}
			else if (keyName.equals("listener")) {
				if (value instanceof Integer) {
					settings.setListener((Integer)value);
				}
			}
			else if (keyName.equals("urgency")) {
				if (value instanceof String) {
					settings.setUrgency(PermissionUrgency.from((String)value));
				}
			}
			else if (keyName.equals("rationaletitle")) {
				if (value instanceof String) {
					settings.setRationaleTitle((String)value);
				}
			}
			else if (keyName.equals("rationaledescription")) {
				if (value instanceof String) {
					settings.setRationaleDescription((String)value);
				}
			}
			else if (keyName.equals("settingsredirecttitle")) {
				if (value instanceof String) {
					settings.setSettingsRedirectTitle((String)value);
				}
			}
			else if (keyName.equals("settingsredirectdescription")) {
				if (value instanceof String) {
					settings.setSettingsRedirectDescription((String)value);
				}
			}
		}
		
		// settings.validate();

		// Return the settings object.
		return settings;
	}

	// * TODO: SPRINKLE THIS INTO CONSTRUCTORS IF IT'S REALLY NEEDED... 
	// HAVE THIS BE CALLED BEFORE CLONING THIS OBJECT FOR ACTIVITY.REQUESTPERMISSIONS
	//  * Checks the PermissionsSettings object for errors like not having a permission specified.
	//  * Will display a native alert detailing all errors found if any.
	 
	// private void validate() {
	// 	// Verify that we have specified some permissions.	
	// 	if (fPermissions == null || fPermissions.isEmpty()) {

	// 	}
	// }
}
