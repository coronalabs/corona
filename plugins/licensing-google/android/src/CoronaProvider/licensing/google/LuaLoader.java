//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package CoronaProvider.licensing.google;

import android.provider.Settings.Secure;
import android.app.Activity;
import android.util.Log;
import android.content.Context;
import android.content.SharedPreferences;

import java.util.Random;

import com.naef.jnlua.LuaState;
import com.naef.jnlua.LuaType;
import com.naef.jnlua.JavaFunction;
import com.naef.jnlua.NamedJavaFunction;

import com.ansca.corona.CoronaRuntime;
import com.ansca.corona.CoronaLua;
import com.ansca.corona.CoronaLuaEvent;
import com.ansca.corona.CoronaEnvironment;
import com.ansca.corona.CoronaRuntimeTaskDispatcher;

import com.ansca.corona.purchasing.StoreServices;
import com.ansca.corona.purchasing.StoreName;

import com.google.android.vending.licensing.AESObfuscator;
import com.google.android.vending.licensing.LicenseChecker;
import com.google.android.vending.licensing.LicenseCheckerCallback;
import com.google.android.vending.licensing.Policy;
import com.google.android.vending.licensing.APKExpansionPolicy;
import com.google.android.vending.licensing.StrictPolicy;


public class LuaLoader implements JavaFunction {
	private static byte[] SALT;
	public static final String PREFS_FILE = "CoronaProvider.licensing.google.lualoader";
	
	private static final String LAST_VERSION_CHECKED = "LAST_VERSION_CHECKED";
	private static final String SALT_STRING = "salt";

	private LicenseCheckerCallback mLicenseCheckerCallback;
	private LicenseChecker mChecker;
	private Policy licenseCheckPolicy;
	private String licenseKey;

	private com.ansca.corona.CoronaRuntimeTaskDispatcher fDispatcher;
	private int fListener;

	public LuaLoader() {
		initialize();
	}

	protected void initialize() {
		//Its less secure if we have a hardcoded salt because would be able to use that for all corona apps
		SharedPreferences sp = CoronaEnvironment.getApplicationContext().getSharedPreferences(PREFS_FILE, Context.MODE_PRIVATE);
		String salty = sp.getString(SALT_STRING, null);
		if (salty == null) {
			SALT = new byte[20];
			new Random().nextBytes(SALT);
			salty = new String(SALT);
			SharedPreferences.Editor editor = sp.edit();
			editor.putString(SALT_STRING, salty);
			editor.commit();
		}
		SALT = salty.getBytes();
	}

	/**
	 * Warning! This method is not called on the main UI thread.
	 */
	@Override
	public int invoke(LuaState L) {
		initialize();
		fDispatcher = new CoronaRuntimeTaskDispatcher( L );

		NamedJavaFunction[] luaFunctions = new NamedJavaFunction[] {
			new InitWrapper(),
			new VerifyWrapper(),
			// Undocumented beyond here. Only used for Expansion File support.
			new IsGoogleExpansionFileRequiredWrapper(),
			new GetExternalStorageStateWrapper(),
			new GetAvailableExternalSpaceWrapper(),
			new IsNewAppVersionWrapper(),
			new GetFileNamesFromPreferencesWrapper(),
			new LoadExpansionFilesWrapper()
		};

		String libName = L.toString( 1 );
		L.register(libName, luaFunctions);

		return 1;
	}

	/**
	 * Initializes the license checker object with the supplied license key
	 * pushes back true if the init was successful
	 * pushes back false if the init wasn't successful
	 */
	public int init(LuaState L) {

		int top = L.getTop();
		
		String licenseKey = "";
		String policyType = "";

		//does this call in lua require("config")
		L.getGlobal("require");
		L.pushString("config");
		L.call(1, LuaState.MULTRET);

		//gets the application table
		L.getGlobal("application");
		if (L.isTable(-1)) {
			//push the license table to the top of the stack
			L.getField(-1, "license");
			if(L.isTable(-1)) {
				//push the google table to the top of the stack
				L.getField(-1, "google");
				if(L.isTable(-1)) {
					//gets the key field from the google table
					L.getField(-1, "key");
					if(L.isString(-1)) {
						licenseKey = L.toString(-1);
					}
					
					//gets the policy field from the google table only if the key was successfully gotten
					L.getField(-2, "policy");
					if(L.isString(-1)) {
						policyType = L.toString(-1);
					}
				}	
			}
		}
		
		boolean success = !licenseKey.equals("") && (StoreServices.getTargetedAppStoreName().equals(StoreName.GOOGLE) || StoreServices.getTargetedAppStoreName().equals(StoreName.NONE));

		if (success) {
			success = this.initLicenseChecker(licenseKey, policyType);
		}
		
		L.setTop(top);

		L.pushBoolean( success );
		return 1;
	}

	private boolean initLicenseChecker(String licenseKey, String policyType) {
				
		//the object that will be called when license verification is complete
		mLicenseCheckerCallback = new MyLicenseCheckerCallback();
		String deviceId = Secure.getString(CoronaEnvironment.getApplicationContext().getContentResolver(), Secure.ANDROID_ID);
		String packageName = CoronaEnvironment.getApplicationContext().getPackageName();

		//obfuscates the cached response from the server so that it can't be easily changed to verified
		AESObfuscator aes = new AESObfuscator(SALT, packageName, deviceId);

		if (policyType.equals("strict")) {
			//this policy will check every time and not cache the result so if the server can't be reached then 
			//it will return false
			licenseCheckPolicy = new StrictPolicy();
		} else {
			//this policy will check every time and cache the result so if the server can't be reached then
			//it will use the cached value if the last check was within the grace period.
			//the grace period is determined by the server
			licenseCheckPolicy = new APKExpansionPolicy(CoronaEnvironment.getApplicationContext(), aes);
		}
		try {
			mChecker = new LicenseChecker(CoronaEnvironment.getApplicationContext(), licenseCheckPolicy, licenseKey);	
		} catch (IllegalArgumentException e) {
			Log.e("Corona", "Invalid public key", e);
			return false;
		}
		return true;
	}

	/**
	 * Gets the lua callback function and a dispatcher and then starts the check access process which is done on another thread
	 */
	public int verify(LuaState L) {
		//check for the licensing permission
		Context context = CoronaEnvironment.getApplicationContext();
		if (context != null) {
			context.enforceCallingOrSelfPermission("com.android.vending.CHECK_LICENSE", null);
		}
		
		int listenerIndex = 1;
		int forceIndex = 2;
		boolean success = false;
		if ( CoronaLua.isListener( L, listenerIndex, "licensing" ) && mChecker != null && (StoreServices.getTargetedAppStoreName().equals(StoreName.GOOGLE)) || StoreServices.getTargetedAppStoreName().equals(StoreName.NONE)) {
			fListener = CoronaLua.newRef( L, listenerIndex );
			
			boolean force = false;
			if ( L.isBoolean(forceIndex) ) {
				force = L.toBoolean(forceIndex);
			}

			mChecker.checkAccess(mLicenseCheckerCallback, force);
			success = true;
		}

		L.pushBoolean(success);
		return 1;
	}

	/**
	 * Calls back the lua function that was supplied in verify()
	 * Pushes on to the stack a boolean for if the application was verified
	 * Pushes on to the stack a string saying why it was verified or not
	 */
	private void callLuaCallback(final boolean isVerified, final String response, final boolean isError, final String errorType) {
		final CoronaRuntimeTaskDispatcher dispatcher = fDispatcher;
			
		// Create a task that will call the given Lua function.
		// This task's execute() method will be called on the Corona runtime thread, just before rendering a frame.
		com.ansca.corona.CoronaRuntimeTask task = new com.ansca.corona.CoronaRuntimeTask() {
			@Override
			public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
				// *** We are now running on the Corona runtime thread. ***
				try {
					// Fetch the Corona runtime's Lua state.
					LuaState L = runtime.getLuaState();

					CoronaLua.newEvent( L, "licensing");

					L.pushBoolean( isVerified );
					L.setField( -2, "isVerified" );

					if (isError) {
						L.pushBoolean( isError );
					} else {
						L.pushNil();
					}
					L.setField( -2, "isError" );

					if (isError) {
						L.pushString( errorType);
					} else {
						L.pushNil();
					}
					L.setField(-2, "errorType");

					L.pushString( response );
					L.setField(-2, "response");

					L.pushString( "google" );
					L.setField( -2, CoronaLuaEvent.PROVIDER_KEY );

					L.pushNumber( (double)licenseCheckPolicy.getValidityTimestamp() );
					L.setField( -2, "expiration" );
 					
 					//Creates a table to put all the expansion file info in there
 					L.newTable(licenseCheckPolicy.getExpansionURLCount(), 0);
 					
 					for (int i = 0; i < licenseCheckPolicy.getExpansionURLCount(); i++) {
 						L.newTable(0, 3);

 						L.pushString(licenseCheckPolicy.getExpansionURL(i));
 						L.setField(-2, "url");

 						L.pushString(licenseCheckPolicy.getExpansionFileName(i));
 						L.setField(-2, "fileName");

 						L.pushNumber(licenseCheckPolicy.getExpansionFileSize(i));
 						L.setField(-2, "fileSize");

 						L.setField(-2, Integer.toString(i+1));
 					}
 					
 					L.setField( -2, "expansionFiles");
 					
					// Dispatch event table at top of stack
					CoronaLua.dispatchEvent( L, fListener, 0 );
				}
				catch (Exception ex) {
					ex.printStackTrace();
				}
			}
		};
			
		// Send the above task to the Corona runtime asynchronously.
		// The send() method will do nothing if the Corona runtime is no longer available, which can
		// happen if the runtime was disposed/destroyed after the user has exited the Corona activity.
		dispatcher.send(task);

	}

	public static boolean isNewAppVersion() {
		Context context = CoronaEnvironment.getApplicationContext();

		SharedPreferences sp = context.getSharedPreferences(
			CoronaProvider.licensing.google.LuaLoader.PREFS_FILE, Context.MODE_PRIVATE);
		int lastVersion = sp.getInt(LAST_VERSION_CHECKED, 0);
		
		int currentVersion = 0;
		try {
			currentVersion = context.getPackageManager().getPackageInfo(context.getPackageName(), 0).versionCode;
		} catch (Exception ex) {}

		return currentVersion > lastVersion;
	}

	public static void setLastCheckedAppVersion() {
		Context context = CoronaEnvironment.getApplicationContext();

		SharedPreferences sp = context.getSharedPreferences(
			CoronaProvider.licensing.google.LuaLoader.PREFS_FILE, Context.MODE_PRIVATE);
		SharedPreferences.Editor editor = sp.edit();
		int currentVersion = 0;
		try {
			currentVersion = context.getPackageManager().getPackageInfo(context.getPackageName(), 0).versionCode;
		} catch (Exception ex) {}
		
		editor.putInt(LAST_VERSION_CHECKED, currentVersion);
		editor.commit();
	}


	private void setExpansionFileNames() {
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		if (context != null) {
			com.ansca.corona.storage.FileServices fileServices = new com.ansca.corona.storage.FileServices(context);
			for (int index = 0; index < licenseCheckPolicy.getExpansionFileNameCount(); index++) {
				String expansionFileName = licenseCheckPolicy.getExpansionFileName(index);
				if (expansionFileName != null) {
					if (expansionFileName.startsWith("main")) {
						fileServices.setMainExpansionFileName(expansionFileName);
					}
					else if (expansionFileName.startsWith("patch")) {
						fileServices.setPatchExpansionFileName(expansionFileName);
					}
				}
			}
			fileServices.loadExpansionFiles();
		}
	}


	private class MyLicenseCheckerCallback implements LicenseCheckerCallback{
		public void allow(int policyReason) {
			//Save the current version code
			//LicenseChecker will not use a cached verification response if there is a new version so even if
			//the new version isn't verified, we won't set this value
			setLastCheckedAppVersion();

			// Send the licensing response to Lua.
			callLuaCallback(true, this.translateResponse(policyReason), false, "");
		}

		public void dontAllow(int policyReason) {
			//google's code doesn't consider retrying an error but we want to
			boolean isError = Policy.RETRY == policyReason;
			String errorType = isError ? "network" : "";

			callLuaCallback(false, this.translateResponse(policyReason), isError, errorType);
		}

		public void applicationError(int errorCode) {
			callLuaCallback(false, this.translateResponse(errorCode), true, "configuration");
		}

		private String translateResponse(int responseCode) {
			String message = "";
			switch (responseCode) {
				//configuration error
				case LicenseCheckerCallback.ERROR_INVALID_PUBLIC_KEY:
					message = "Invalid public key";
					break;
				//configuration error
				case LicenseCheckerCallback.ERROR_MISSING_PERMISSION:
					message = "Missing permission";
					break;
				//configuration error
				case LicenseCheckerCallback.ERROR_INVALID_PACKAGE_NAME:
					message = "Invalid package name";
					break;
				//configuration error
				case LicenseCheckerCallback.ERROR_NON_MATCHING_UID:
					message = "Non matching UID";
					break;
				//configuration error
				case LicenseCheckerCallback.ERROR_NOT_MARKET_MANAGED:
					message = "Not market managed";
					break;
				//network error
				case Policy.RETRY:
					message = "Unable to establish connection to Google's Licensing servers.\n\n" + 
						"This may be the result of a poor internet connection, or no internet connection at all.\n\n" + 
						"Please ensure that your device is connected to the internet and try again.";
					break;
				case Policy.LICENSED:
					message = "Licensed";
					break;
				case Policy.NOT_LICENSED:
					message = "Not licensed";
					break;
				default:
					message = "";
			}

			return message;
		}
	}

	private class InitWrapper implements NamedJavaFunction {
		@Override
		public String getName() {
			return "init";
		}
		
		/**
		 * Warning! This method is not called on the main UI thread.
		 */
		@Override
		public int invoke(LuaState L) {
			return init(L);
		}
	}

	private class VerifyWrapper implements NamedJavaFunction{
		

		@Override
		public String getName() {
			return "verify";
		}
		
		/**
		 * Warning! This method is not called on the main UI thread.
		 */
		@Override
		public int invoke(LuaState L) {
			return  verify(L);
		}
	}

	private class IsGoogleExpansionFileRequiredWrapper implements NamedJavaFunction {
		@Override
		public String getName() {
			return "isGoogleExpansionFileRequired";
		}
		
		@Override
		public int invoke(LuaState L) {
			// Attempt to fetch the "usesExpansionFile" flag from the "AndroidManifest.xml" file.
			// Corona Simulator builds normally add it as meta-data within the application tag.
			boolean isRequired = false;
			try {
				String targetedAppStoreName = com.ansca.corona.purchasing.StoreServices.getTargetedAppStoreName();
				if (com.ansca.corona.purchasing.StoreName.GOOGLE.equals(targetedAppStoreName) ||
					com.ansca.corona.purchasing.StoreName.NONE.equals(targetedAppStoreName))
				{
					android.content.Context context = com.ansca.corona.CoronaEnvironment.getApplicationContext();
					android.content.pm.PackageManager packageManager = context.getPackageManager();
					android.content.pm.ApplicationInfo applicationInfo = packageManager.getApplicationInfo(
										context.getPackageName(), android.content.pm.PackageManager.GET_META_DATA);
					if ((applicationInfo != null) && (applicationInfo.metaData != null)) {
						isRequired = applicationInfo.metaData.getBoolean("usesExpansionFile", false);
					}
				}
			}
			catch (Exception ex) { 
				Log.e("Corona", "isGoogleExpansionFileRequired", ex);
			}

			// Return the result to Lua.
			L.pushBoolean(isRequired);
			return 1;
		}
	}

	private class GetExternalStorageStateWrapper implements NamedJavaFunction {
		@Override
		public String getName() {
			return "getExternalStorageState";
		}

		@Override
		public int invoke(LuaState L) {
			String state = android.os.Environment.getExternalStorageState();
			L.pushString(state);
			return 1;
		}
	}

	public class GetAvailableExternalSpaceWrapper implements NamedJavaFunction {
		@Override
		public String getName() {
			return "getAvailableExternalSpace";
		}

		@Override
		public int invoke(LuaState L) {
			double availableBytes;
			try {
				android.os.StatFs statFs = new android.os.StatFs(android.os.Environment.getExternalStorageDirectory().getAbsolutePath());
				availableBytes = (double)statFs.getBlockSize() * statFs.getAvailableBlocks();
			} catch (Exception e) {
				android.util.Log.w("Corona", "Could not retrieve the available free space for: " + android.os.Environment.getExternalStorageDirectory().getAbsolutePath());
				availableBytes = java.lang.Double.MAX_VALUE;
			}
			

			L.pushNumber(availableBytes);
			return 1;
		}
	}

	public class IsNewAppVersionWrapper implements NamedJavaFunction {
		@Override
		public String getName() {
			return "isNewAppVersion";
		}

		@Override
		public int invoke(LuaState L) {
			L.pushBoolean(isNewAppVersion());
			return 1;
		}
	}

	public class GetFileNamesFromPreferencesWrapper implements NamedJavaFunction {
		@Override
		public String getName() {
			return "getFileNamesFromPreferences";
		}

		@Override
		public int invoke(LuaState L) {
			L.newTable(licenseCheckPolicy.getExpansionFileNameCount(), 0);
			int luaTableStackIndex = L.getTop();
			for (int i = 0; i < licenseCheckPolicy.getExpansionFileNameCount(); i++) {
				L.pushString(licenseCheckPolicy.getExpansionFileName(i));
				L.rawSet(luaTableStackIndex, i + 1);
			}
			
			return 1;
		}
	}

	//Needs this because we have to load it after its downloaded
	public class LoadExpansionFilesWrapper implements NamedJavaFunction {
		@Override
		public String getName() {
			return "loadExpansionFiles";
		}

		@Override
		public int invoke(LuaState L) {
			setExpansionFileNames();
			return 0;
		}
	}
}
