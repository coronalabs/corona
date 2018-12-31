/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.android.vending.licensing;

import com.ansca.corona.CoronaEnvironment;

import org.apache.http.NameValuePair;
import org.apache.http.client.utils.URLEncodedUtils;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Vector;
/**
 * Policy used by {@link LicenseChecker} to determine whether a user should have
 * access to the application.
 */
public abstract class Policy {

    /**
     * Change these values to make it more difficult for tools to automatically
     * strip LVL protection from your APK.
     */

    /**
     * LICENSED means that the server returned back a valid license response
     */
    public static final int LICENSED = 0x0100;
    /**
     * NOT_LICENSED means that the server returned back a valid license response
     * that indicated that the user definitively is not licensed
     */
    public static final int NOT_LICENSED = 0x0231;
    /**
     * RETRY means that the license response was unable to be determined ---
     * perhaps as a result of faulty networking
     */
    public static final int RETRY = 0x0123;

    private Vector<String> mExpansionURLs = new Vector<String>();
    private Vector<String> mExpansionFileNames = new Vector<String>();
    private Vector<Long> mExpansionFileSizes = new Vector<Long>();

    private static String EXPANSION_FILE_PREFS = "CoronaProvider.licensing.google.lualoader.EXPANSION_FILE_PREFS";

    public Policy() {
        SharedPreferences sp = CoronaEnvironment.getApplicationContext().getSharedPreferences(EXPANSION_FILE_PREFS, Context.MODE_PRIVATE);

        //Loads all the saved expansion file names.
        java.util.Map<String, ?> everything = sp.getAll();
        java.util.Set<String> keys = everything.keySet();
        java.util.Iterator<String> iterator = keys.iterator();
        while(iterator.hasNext()) {
            String expansionFileNameKey = String.valueOf(iterator.next());
            String expansionFileName = sp.getString(expansionFileNameKey, "");
            this.setExpansionFileName(Integer.parseInt(expansionFileNameKey), expansionFileName);
        }
    }

    /**
     * Provide results from contact with the license server. Retry counts are
     * incremented if the current value of response is RETRY. Results will be
     * used for any future policy decisions.
     * 
     * @param response the result from validating the server response
     * @param rawData the raw server response data, can be null for RETRY
     */
    abstract void processServerResponse(int response, ResponseData rawData);

    /**
     * Check if the user should be allowed access to the application.
     */
    abstract boolean allowAccess();

    abstract public long getValidityTimestamp();

    /**
     * Gets the count of expansion URLs. Since expansionURLs are not committed
     * to preferences, this will return zero if there has been no LVL fetch
     * in the current session.
     * 
     * @return the number of expansion URLs. (0,1,2)
     */
    public int getExpansionURLCount() {
        return mExpansionURLs.size();
    }

    /**
     * Gets the expansion URL. Since these URLs are not committed to
     * preferences, this will always return an empty string if there has not been an LVL
     * fetch in the current session.
     * 
     * @param index the index of the URL to fetch. This value will be either
     *            MAIN_FILE_URL_INDEX or PATCH_FILE_URL_INDEX
     * @param URL the URL to set
     */
    public String getExpansionURL(int index) {
        if (index < mExpansionURLs.size()) {
            return mExpansionURLs.elementAt(index);
        }
        return "";
    }

    /**
     * Sets the expansion URL. Expansion URL's are not committed to preferences,
     * but are instead intended to be stored when the license response is
     * processed by the front-end.
     * 
     * @param index the index of the expansion URL. This value will be either
     *            MAIN_FILE_URL_INDEX or PATCH_FILE_URL_INDEX
     * @param URL the URL to set
     */
    public void setExpansionURL(int index, String URL) {
        if (index >= mExpansionURLs.size()) {
            mExpansionURLs.setSize(index + 1);
        }
        mExpansionURLs.set(index, URL);
    }

    public String getExpansionFileName(int index) {
        if (index < mExpansionFileNames.size()) {
            return mExpansionFileNames.elementAt(index);
        }
        return "";
    }

    //Each time we set an expansion file we want to save it so we can be sure that they're all there
    public void setExpansionFileName(int index, String name) {
        if (index >= mExpansionFileNames.size()) {
            mExpansionFileNames.setSize(index + 1);
        }
        mExpansionFileNames.set(index, name);

        SharedPreferences sp = CoronaEnvironment.getApplicationContext().getSharedPreferences(EXPANSION_FILE_PREFS, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sp.edit();
        editor.putString(String.valueOf(index), name);
        editor.commit();
    }

    public int getExpansionFileNameCount() {
        return mExpansionFileNames.size();
    }

    public long getExpansionFileSize(int index) {
        if (index < mExpansionFileSizes.size()) {
            return mExpansionFileSizes.elementAt(index);
        }
        return -1;
    }

    public void setExpansionFileSize(int index, long size) {
        if (index >= mExpansionFileSizes.size()) {
            mExpansionFileSizes.setSize(index + 1);
        }
        mExpansionFileSizes.set(index, size);
    }

    protected Map<String, String> decodeExtras(String extras) {
        Map<String, String> results = new HashMap<String, String>();
        try {
            URI rawExtras = new URI("?" + extras);
            List<NameValuePair> extraList = URLEncodedUtils.parse(rawExtras, "UTF-8");
            for (NameValuePair item : extraList) {
                String name = item.getName();
                int i = 0;
                while (results.containsKey(name)) {
                    name = item.getName() + ++i;
                }
                results.put(name, item.getValue());
            }
        } catch (URISyntaxException e) {
            Log.w("Policy", "Invalid syntax error while decoding extras data from server.");
        }
        return results;
    }

    //This is used when we get a valid response from the server.  We will want to clear the saved expansion file names in case they're not needed anymore.
    protected void clearSavedExpansionFiles() {
        SharedPreferences sp = CoronaEnvironment.getApplicationContext().getSharedPreferences(EXPANSION_FILE_PREFS, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sp.edit();
        editor.clear();
        editor.commit();
    }
}
